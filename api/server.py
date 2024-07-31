from datetime import date
from subprocess import check_output, Popen, check_call
from fastapi import  APIRouter, FastAPI, Response, BackgroundTasks
import os
from pydantic import BaseModel
from rq import Queue
from pathlib import Path
from dotenv import load_dotenv
from supabase import client
import sys
import math
import requests
from api.email import send_job_reconstruction, send_job_results, send_payment_notification
from api.cache import redisObj
from redis import Redis
import uuid 
sys.path.append('.')
from api.models import ScheduleJob, UserJob
from api.cache import enqueue_job, dequeue_job, current_job_index
from datetime import datetime
import s3fs
# import ipfshttpclient2
import os

import logging


queue = Queue(connection=redisObj) # type: ignore

#Ipfs_api = ipfshttpclient2.client.connect()
#scheduler = scheduler.RQScheduler(queues=queue,connection=redisObj) #type: ignore
## providing necessary permissions to avoid the error:
root_folder_path = Path(os.path.abspath(__file__)).parent.parent
# spark_dir = root_folder_path / 'storage'/ 'shared_spark'
#load_dotenv(dotenv_path=(root_folder_path  / '.env'))
current_dir = Path(os.path.abspath(__file__)).parent
load_dotenv(current_dir / '.env')

fastapi = FastAPI()
callback_router = APIRouter()

supabase_client = client.Client(supabase_key=str(os.getenv("SUPABASE_KEY")), supabase_url=str(os.getenv("SUPABASE_URL")))

bgtasks = BackgroundTasks()

## for the single file reconstruction
def launch_single_tile_reconstruction(filepath_url, userfile_path):
    return Popen([  root_folder_path / 'run_lidarhd.sh' , '--list_files', filepath_url, '--output_dir', userfile_path ])
## for taking multiple laz tiles for reconstruction at the point

def launch_multiple_tile_reconstruction(filepath_url:Path, aggregator_factor: int): 
    """
    filepath_url: is the url file that you want to generate the reconstruction.
    aggregator_path: defines the number of tiles that you want to reconstruction at one point of time.
    """
    
    userfile_path = root_folder_path / 'demo_storage'  / str(datetime.today().strftime('%Y-%m-%d'))
    userprams = UserJob()
    
    with open(filepath_url, 'r') as fp:
        folder_aggregated_path = ""
        for folder_iter in iter(fp):
            folder_aggregated_path = userfile_path / ("lidarhd_mode" + "_agg_" + str(folder_iter))
            folder_output = folder_aggregated_path / "output"
            if len(os.listdir(folder_aggregated_path)) == aggregator_factor : 
                Popen([root_folder_path / "run_workflow.sh", "--input_dir", folder_aggregated_path, "--output_dir",folder_output ])
            if  not math.fabs(int(folder_iter) / aggregator_factor ):
                os.mkdir(folder_aggregated_path) 
                os.mkdir(folder_output)
            check_call(["wget" , "-O", folder_iter, folder_aggregated_path ])
    fp.close()


@fastapi.post(path="/reconstruction/post", callbacks= callback_router.routes)
def run_lidarhd_job(filepath_url:Path, email: str,  bg: BackgroundTasks):
    """
    Sends the files from the lidarhd to localhost and then runs the operation
    """
    userfile_path = root_folder_path / 'demo_storage'  / str(datetime.today().strftime('%Y-%m-%d'))
    user = UserJob()
    
    if(os.path.isdir(userfile_path)):
            output = launch_single_tile_reconstruction(filepath_url,userfile_path)
            bg.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path)
    else: 
            os.mkdir(userfile_path, mode=0o777)
            output = launch_single_tile_reconstruction(filepath_url,userfile_path)
            bg.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path)
            
    ##storing the laz file for the given user
    num_tiles = 0
    tilenames = []
    jobIds = []
    with open(filepath_url, 'r') as fp:
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

class Status(BaseModel):
    job_status: str
@fastapi.post("/reconstruction/schedule")
def schedule_reconstruction_job(data:ScheduleJob) -> Status:
    params = ""
    try:
        url_filepath = Path(data.input_url)
        with open(url_filepath, '+rt'):
            job_details = queue.enqueue_call(run_lidarhd_job,[url_filepath, data.username], timeout=10000)
            params = job_details.result   
        return Status(job_status=str(params))
        
    except Exception as e:
        print("error on scheduling reconstruction job:" + str(e))

    return Status(job_status="error scheduling job")

def concurrency_condition():
    available_messages = queue.all(connection=(redisObj)) #type: ignore
    if available_messages:
        return True
    return False

@fastapi.post("/reconstruction/storage/s3")
def storing_files_s3(dir_location_output: str):
    """
    stores the generated output from the localhost to the given S3 bucket storage.
    
    dir_location_output: is the relative location from the root folder od the generated output folder with the information 
    """
    try:
         check_output(["S3", "cp", str(os.getenv("S3_DIR")),  dir_location_output ]) 
    except Exception as e:
        print("s3 file is not able to be stored" + str(e))


#@fastapi.post("/reconstruction/email/send_result")
@callback_router.post("/reconstruction/email/send_result")
def send_final_result(email, jobId):
    fullpaths = []
    folder_paths = []
    signed_urls = []
    try:
        folder_paths = supabase_client.table("selectionJob").select("final_results").contains("job_id", email)
        for i in folder_paths:
            fullpaths.append(str(os.getenv("S3_DIR")) + '/' + folder_paths[i])
            signed_urls.append(check_output(["aws", "s3", "presign", folder_paths[i]]).decode().strip())
        send_job_results(receiver_email=email, job_id= jobId, ipfs_url=signed_urls)
    except Exception as e:
        print("not able to send email with results")
