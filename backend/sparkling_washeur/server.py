from datetime import date
from subprocess import check_output, check_call, run
from fastapi import  APIRouter, FastAPI, Response, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from dotenv import load_dotenv
import os
from supabase import client
import sys
import shutil
import math
import boto3
import uuid 
import requests
import logging
from pathlib import Path
import supabase
import uvicorn
import re
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
sys.path.append(".")


root_folder_path = Path(os.path.abspath(__file__))

import os
import io
import zipfile
from sparkling_washeur.event_architecture import publish_sns_topic, get_sqs_event, upload_file_s3, s3Object,push_sqs_event_schedule
from sparkling_washeur.email import send_job_reconstruction, send_job_results, send_payment_notification
from sparkling_washeur.cache import redisObj, enqueue_job
from sparkling_washeur.models import JobHistory, ScheduleJob, ScheduleJobMulti, UserJob, Status, JobRuntime
from datetime import datetime
from s3path import S3Path

core_directory_path = S3Path(str(os.getenv("S3_DIR")) )

# queue = Queue(connection=redisObj) 

load_dotenv(dotenv_path=(root_folder_path/ '.env'))


fastapi = FastAPI()
callback_router = APIRouter()

# creating database management endpoints.
supabase_client = client.Client(supabase_key=str(os.getenv("SUPABASE_KEY")), supabase_url=str(os.getenv("SUPABASE_URL")))
bgtasks = BackgroundTasks()

Debug=False

## for the single file reconstruction.
origins = [
    "http://localhost",
    "http://localhost:8000",
    "127.0.0.1:8002"
    "*"
]

fastapi.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@fastapi.post("/schedule/single_tile")
def launch_single_tile_reconstruction() -> JobRuntime:
    """
    Runs the reconstruction algorithm on one tile per session, 
    TODO: its by default the implementation for reconstructing the output, but .
    """    
    params = str(get_sqs_event())
    print("the sqs event" +params)
    ## the information is in format:  "data_url: " + str(job_params.input_url) + '&username:' + str(job_params.username)
    ## need to separate the input_url along with params_username.    
    data_url = params.split('&')[0].split(':')[1]
    username_param = params.split('&')[1].split(':')[1]

    try:
        jobId = supabase_client.table("selectionjob").select("jobId").contains(column="upload_path_url", value=data_url)
        if Debug:
            print("resulting jobId:" + str(jobId))    
        
        supabase_client.table("extralabs").update({
            "email": username_param,
            "selectionJob": {
                "jobId": jobId,
                "status": True,
                "upload_url_file": data_url
            }                
        })
        
        ## first defining the various paths for getting the required parameters      
        process_id = run([
            "/bin/sh", "-c" , "./run_examples.sh" , "--input_dir" , data_url , "--output_dir", data_url + "/output"
        ])
        return_value = JobRuntime(jobId= str(jobId), container_id= str((str(process_id))))
        queue_params = ScheduleJob(input_url=data_url,username=username_param )
        enqueue_job(queue_params)
        return return_value

    except Exception as e:
        print("exception:" + str(e))
        return JobRuntime(jobId="Null: error in executing the docker container", container_id="Null")

## for taking multiple laz tiles for reconstruction at the point
def launch_multiple_tile_reconstruction(filepath_url:S3Path, aggregator_factor: int, username: str): 
    """
    filepath_url: is the url file path (from locale) you want to generate the reconstruction.
    aggregator_path: defines the number of tiles that you want to reconstruction in batches.
    """
    userfile_path = S3Path(str(os.getenv("S3_DIR"))) / str(datetime.today().strftime('%Y-%m-%d')) 
    ## read the url file --> club the various tiles into the batches of 'aggregated_factor' and then run the reconstruction as unique block
    with open(filepath_url, 'r') as fp:
        folder_aggregated_path = ""
        for folder_iter in iter(fp):
            folder_aggregated_path = (userfile_path) / ("lidarhd_mode" + "_agg_" + str(folder_iter))
            folder_output = folder_aggregated_path / "output"
            
            if len(os.listdir(folder_aggregated_path)) == aggregator_factor : 
                launch_single_tile_reconstruction(filepath_laz= folder_aggregated_path, output_path= folder_output, username=username)
            if  not math.fabs(int(folder_iter) / aggregator_factor ):
                os.mkdir(folder_aggregated_path) 
                os.mkdir(folder_output)
            check_call(["wget" , "-O", folder_iter, folder_aggregated_path ])
    fp.close()

def s3_directory_exists(path_params):
    """
    Checks whether the given filepath (in the S3_DIR) exists.    
    """
    return S3Path(path_params).exists()

def store_laz_files_s3(filepath_url: S3Path) -> list:
    ## takes in the laz texturl and then downloads and fetches the result.
    tiles_stored = []
    try:
        folder_path = core_directory_path / str(datetime.today().strftime('%Y-%m-%d'))        
        folder_path.mkdir()
        with filepath_url.open() as read_urls:
            for url_file in read_urls:
                filename_params = url_file.strip().split("/")[-1].split(".")[0]  
                tiles_stored.append(filename_params)
                check_call(["wget" , "-O", filename_params, folder_path.as_uri() ])
            read_urls.close()
        
    except Exception as e:
        print("facing the issue on  "+ str(e))
                
    return tiles_stored
            
@fastapi.post(path="/reconstruction/post", callbacks= callback_router.routes)
def run_lidarhd_job(filepath_url:Path, email: str,  bg: BackgroundTasks):
    """
    Sends the files from the lidarhd to localhost and then runs the operation
    """
    userfile_path_dir = filepath_url
    # userfile_path = root_folder_path / 'demo_storage'  / str(datetime.today().strftime('%Y-%m-%d'))
    user = UserJob()
    if(s3_directory_exists(userfile_path_dir)):
        output = launch_single_tile_reconstruction(filepath_url,userfile_path_dir, username=email)
        bg.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path_dir, email)
    else: 
        userfile_path_dir.mkdir(mode=0o777)
        output = launch_single_tile_reconstruction(filepath_url,userfile_path_dir, username=email)
        bg.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path_dir, username=email)
    num_tiles = 0
    tilenames = []
    jobIds = []
    with userfile_path_dir.open() as fp:
        print("successful read of the file, now getting the internal values")
        for line in fp:
            filename_params = line.strip().split("/")[-1].split(".")[0]  
            tilenames.append(filename_params)
            num_tiles += 1
            user.files_storage_tiles[filename_params] = []
            # Perform other operations for each tile
            newJobId = str(uuid.uuid1())
            ## useremail
            supabase_client.table("extra_surface").insert({
                "email": user.email,
                "job_history": [newJobId]
            })    
            jobIds.append(newJobId)
        user.email = email
   ## now sending the email 
    send_job_reconstruction(receiver_email=user.email, job_id=jobIds)
    fp.close()
    return output

@fastapi.post("/reconstruction/schedule_multi")
def schedule_multi_reconstruction_job(data:ScheduleJobMulti,bg:BackgroundTasks):
    try:
        params = ""
        url_file_filepath = S3Path(data.input_url_file)
        bg.add_task(launch_multiple_tile_reconstruction,url_file_filepath,data.aggregator,data.username)     
    except Exception as e:
        print("error in launching multi reconstruction" + str(e))


@fastapi.post("/reconstruction/schedule")
def schedule_reconstruction_job(data:ScheduleJob) -> Status:
    try:
        params = str(uuid.uuid4())
        url_filepath = S3Path(data.input_url)
        #job_id = str(job_params)
        
        # supabase_client.table("extra_surface").insert(json={
        #     "email": data.username,
        #     "job_history": [].append(JobHistory(
        #         jobId= job_id,
        #         status=True,
        #         upload_url_file=url_filepath.as_uri()
        #     ).model_dump()
        #                              )
        # })  
        
        # supabase_client.table("reconstructed_tiles").insert(
        #     json={
        #         "job_id": params
        #     }
        # )
        #redisObj.set(value=data.input_url,name=data.username)
        job_params = ScheduleJob(input_url=str(url_filepath.as_uri), username=data.username)
        id_values = push_sqs_event_schedule(job_params=job_params, queue_name="Extrasurface.fifo")
        return Status(job_status=str(id_values)) 
    except Exception as e:
        print("error on scheduling reconstruction job:" + str(e))
    return Status(job_status="error scheduling job")

def concurrency_condition():
    available_messages =   True#redisObj.all(connection=(redisObj)) 
    if available_messages:
        return True
    return False

#@fastapi.post("/reconstruction/email/send_result")
@callback_router.post("/reconstruction/email/send_result")
def send_final_result(email, jobId):
    fullpaths = []
    folder_paths = []
    signed_urls = []
    try:
        #folder_paths.append(supabase_client.table("selectionJob").select("final_results").contains("job_id", email))
        for i in folder_paths:
            supabase_client.table("selectionJob").update(
            {
                "jobId": jobId,
                "upload_url_file": fullpaths[i]
            }
            )
            fullpaths.append(str(os.getenv("S3_DIR")) + '/' + folder_paths[i])
            signed_urls.append(check_output(["aws", "s3", "presign", folder_paths[i]]).decode().strip())
        send_job_results(receiver_email=email, job_id= jobId, ipfs_url=signed_urls)
        
        for folder_path in iter(folder_paths):
            zipfile_path = folder_path + "/output"
            zip_s3_upload(folder_path=folder_path,zip_filename=zipfile_path)
    except Exception as e:
        print("not able to send email with results")

## additional functions for listing the files and then uploading (in case they are stored in folders)
## credits to the stackoverflow Q: https://stackoverflow.com/questions/16368119/zip-an-entire-directory-on-s3Object
def ListDir(bucket_name, prefix, file_type='.obj'):
    #file_type can be set to anything you need
    files = []    
    paginator = s3Object.get_paginator('list_objects_v2')
    pages = paginator.paginate(Bucket=bucket_name, Prefix=prefix)
    for page in pages:
        for obj in page['Contents']:
            files.append(obj['Key'])
    if files:
        files = [f for f in files if file_type in f]
    return files

def ZipFolderS3(bucket_name,prefix, zip_filename):
    files = ListDir(bucket_name,prefix)
    zip_buffer = io.BytesIO()
    for ind,file in enumerate(files):
        file = file.split("/")[-1]
        print(f"Processing file {ind} : {file}")
        object_key = prefix+file
        print(object_key)
        
        with zipfile.ZipFile(zip_buffer, "a", zipfile.ZIP_DEFLATED, False) as zipper:
            infile_object = s3Object.get_object(Bucket=bucket_name, Key=object_key) 
            infile_content = infile_object['Body'].read()
            zipper.writestr(file, infile_content)
    s3Object.put_object(Bucket=bucket_name, Key=prefix + zip_filename, Body=zip_buffer.getvalue()) 

def zip_s3_upload(folder_path: str, zip_filename:str):
    """
    Uploads the output file as the zip(in order to later send it as later as single downloadable S3 / IPFS file)
    """
    try:
        shutil.make_archive(folder_path,format='zip',root_dir=zip_filename)
        s3_dir = S3Path(folder_path).as_uri()
        s3Object.put_object(Bucket= s3_dir, Key= zip_filename, Body= zip_filename)        
    except Exception as e:
        print("while fn zip_s3_upload", e)
        
        

@fastapi.post("/email/send_mail_paid")
def send_confirmation_email(receiever_email: str, payment_reference: str, file_details):
    """
    sending the mail regarding successful submission of the compute job.
    """
    
    try:
       result =  send_payment_notification(receiver_email=receiever_email, payment_intent_id=payment_reference, file_details=file_details)
       return result
    except Exception as e:
        print("send_confirmation_error" + str(e))



if __name__ == "__main__":
    uvicorn.run(fastapi, host="ec2-3-80-167-195.compute-1.amazonaws.com", port=8000)