from datetime import date
from subprocess import check_output, Popen, check_call
from fastapi import  APIRouter, FastAPI, Response, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from dotenv import load_dotenv
import os
from rq import Queue
from rq.job import Job
from pathlib import Path
from supabase import client
import sys
import shutil
import math
import boto3
import uuid 

import logging

import docker
from pathlib import Path

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

sys.path.append(".")

from api.email import send_job_reconstruction, send_job_results, send_payment_notification
from api.cache import redisObj, enqueue_job, dequeue_job, current_job_index
from api.models import ScheduleJob, ScheduleJobMulti, UserJob, Status, JobRuntime
from datetime import datetime
import requests

from s3path import S3Path

import os
import boto3
import io
import zipfile

queue = Queue(connection=redisObj) 
root_folder_path = Path(os.path.abspath(__file__)).parent

load_dotenv(dotenv_path=root_folder_path)
client_docker = docker.from_env()


s3 = boto3.client("s3")
fastapi = FastAPI()
callback_router = APIRouter()

supabase_client = client.Client(supabase_key=str(os.getenv("SUPABASE_KEY")), supabase_url=str(os.getenv("SUPABASE_URL")))

bgtasks = BackgroundTasks()

Debug=False

## for the single file reconstruction.
origins = [
    "http://localhost",
    "http://localhost:8000",
    "*"
]

fastapi.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

## function to determine the container id of the recent execution .
## this is based on the parameter: https://stackoverflow.com/questions/63534480/how-to-get-the-container-id-from-docker-py.
## 
def check_docker_container_status(container_name):
    client = docker.from_env()
    #cli = docker.APIClient()
    if client.containers.list(filters={'name': container_name}):
        response = client.containers.list(filters={'name': container_name})
        return str(response[0].id)[:12]
    else:
        return None

def launch_single_tile_reconstruction(filepath_url, output_path, username: str) -> JobRuntime:
    """
    Runs the reconstruction algorithm on one tile per session, 
    TODO: its by default the implementation for reconstructing the output, but .
    """
    try:
        jobId = supabase_client.table("selectionjob").select("jobId").contains(column="upload_path_url", value=filepath_url)
        if Debug:
            print("resulting jobId:" + str(jobId))    
        
        supabase_client.table("extralabs").update({
            "email": username,
            "selectionJob": {
                "jobId": jobId,
                "status": True,
                "upload_url_file": filepath_url
            }                
        })
        
        client_docker.containers.run(str(os.getenv("NAME_IMG_BASE")),detach=True,
                                    environment={
                                        "NAME_IMG_BASE": str(os.getenv("NAME_IMG_BASE")),
                                        "DDT_MAIN_DIR_DOCKER": str(os.getenv("DDT_MAIN_DIR_DOCKER")),
                                        "CONTAINER_NAME_SHELL": str(os.getenv("CONTAINER_NAME_SHELL")),
                                        "CONTAINER_NAME_COMPILE": str(os.getenv("CONTAINER_NAME_COMPILE")),
                                        "TMP_DIR": str(os.getenv("TMP_DIR")),
                                        "SPARK_TMP_DIR": str(os.getenv("SPARK_TMP_DIR")),
                                        "SPARK_HISTORY_DIR": str(os.getenv("SPARK_HISTORY_DIR")),
                                        "CURRENT_PLATEFORM": str("CURRENT_PLATEFORM"),
                                        "MASTER_IP_SPARK": str("MASTER_IP_SPARK"),    
                                        "SPARK_EXECUTOR_MEMORY": str("SPARK_EXECUTOR_MEMORY"),
                                        "SPARK_DRIVER_MEMORY":str("SPARK_DRIVER_MEMORY"), 
                                        "SPARK_WORKER_MEMORY": str("SPARK_WORKER_MEMORY"),
                                        "NUM_PROCESS": str("NUM_PROCESS")
                                     },
                                    command=["/app/wasure/run_lidarhd.sh", "--input_dir", filepath_url, "--output_dir", output_path, "--colorize"]
                                    
                                    )
        return_value = JobRuntime(jobId= str(jobId), container_id= str(check_docker_container_status(str(os.getenv("NAME_IMG_BASE")))))
        return return_value

    except Exception as e:
        print("exception:" + str(e))
        return JobRuntime(jobId="Null: error in executing the docker container", container_id="Null")

## for taking multiple laz tiles for reconstruction at the point
def launch_multiple_tile_reconstruction(filepath_url:Path, aggregator_factor: int, username: str): 
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
                launch_single_tile_reconstruction(filepath_url= folder_aggregated_path, output_path= folder_output, username=username)
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

@fastapi.post(path="/reconstruction/post", callbacks= callback_router.routes)
def run_lidarhd_job(filepath_url:Path, email: str,  bg: BackgroundTasks):
    """
    Sends the files from the lidarhd to localhost and then runs the operation
    """
    userfile_path_dir = S3Path(str("s3://arn:aws:s3:us-east-1:573622188359:accesspoint/extra-protocol-lidarhd")) / filepath_url
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
        url_file_filepath = Path(data.input_url_file)
        bg.add_task(launch_multiple_tile_reconstruction,url_file_filepath,data.aggregator,data.username)     
    except Exception as e:
        print("error in launching multi reconstruction" + str(e))


@fastapi.post("/reconstruction/schedule")
def schedule_reconstruction_job(data:ScheduleJob) -> Status:
    try:
        params = ""
        url_filepath = Path(data.input_url)
        job = Job.create(run_lidarhd_job,[url_filepath, data.username])
        with open(url_filepath, '+rt'):
            job_details = queue.enqueue_job(job)
            params = job_details.id   
            supabase_client.table("extra_surface").insert(json={
                "email": data.username,
                "job_history": [params]
            })  
            supabase_client.table("selectionjob").insert(
                json={
                 "job_id" : params,
                 "job_created_at": datetime.today(),
                 "status": "false"  
                }
            )
            supabase_client.table("reconstructed_tiles").insert(
                json={
                    "job_id": params
                }
            )
        redisObj.set(value=data.input_url,name=data.username)
        return Status(job_status=str(redisObj.get(data.username))) 
    except Exception as e:
        print("error on scheduling reconstruction job:" + str(e))
    return Status(job_status="error scheduling job")

def concurrency_condition():
    available_messages = queue.all(connection=(redisObj)) #type: ignore
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
## credits to the stackoverflow Q: https://stackoverflow.com/questions/16368119/zip-an-entire-directory-on-s3
def ListDir(bucket_name, prefix, file_type='.obj'):
    #file_type can be set to anything you need
    files = []    
    paginator = s3.get_paginator('list_objects_v2')
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
            infile_object = s3.get_object(Bucket=bucket_name, Key=object_key) 
            infile_content = infile_object['Body'].read()
            zipper.writestr(file, infile_content)
    s3.put_object(Bucket=bucket_name, Key=prefix + zip_filename, Body=zip_buffer.getvalue()) 

def zip_s3_upload(folder_path: str, zip_filename:str):
    """
    Uploads the output file as the zip(in order to later send it as later as single downloadable S3 / IPFS file)
    """
    try:
        shutil.make_archive(folder_path,format='zip',root_dir=zip_filename)
        s3_dir = S3Path(folder_path).as_uri()
        s3.put_object(Bucket= s3_dir, Key= zip_filename, Body= zip_filename)        
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