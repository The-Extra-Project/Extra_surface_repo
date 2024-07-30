from datetime import date
from subprocess import check_output, Popen, check_call
from fastapi import  FastAPI, Response, BackgroundTasks
import os
from pydantic import BaseModel
from rq import Queue
from pathlib import Path
from dotenv import load_dotenv
from supabase import client
import sys
import math
import requests
from api.email import send_job_reconstruction
from api.cache import redisObj
from redis import Redis
import uuid 
sys.path.append('.')
from api.models import ScheduleJob, UserJob
from api.cache import enqueue_job, dequeue_job, current_job_index
from datetime import datetime
import s3fs
#import ipfshttpclient2
import os

queue = Queue(connection=redisObj) # type: ignore
counter_message_redit_active = 0
#Ipfs_api = ipfshttpclient2.client.connect()
#scheduler = scheduler.RQScheduler(queues=queue,connection=redisObj) #type: ignore
s3 = s3fs.S3FileSystem(anon=False)
root_folder_path = Path(os.path.abspath(__file__)).parent.parent
load_dotenv(dotenv_path=(root_folder_path  / '.env'))
os.chmod(root_folder_path / 'storage' / 'shared_spark' , mode=0o777)

fastapi = FastAPI()
supabase_client = client.Client(supabase_key=str(os.getenv("SUPABASE_KEY")), supabase_url=str(os.getenv("SUPABASE_URL")))

os.chmod(path=root_folder_path / 'storage'/ 'shared_spark', mode=0o777)

bgtasks = BackgroundTasks()


## for the single file reconstruction
def launch_single_tile_reconstruction(filepath_url, userfile_path):
    return Popen([  root_folder_path / 'run_lidarhd.sh' , '--list_files', filepath_url, '--output_dir', userfile_path ])
## for taking multiple laz tiles for reconstruction at the point

def launch_multiple_tile_reconstruction(filepath_url:Path, aggregator_factor: int, bg: BackgroundTasks): 
    """
    filepath_url: is the url file that you want to generate the reconstruction.
    aggregator_path: defines the number of tiles that you want to reconstruction at one point of time.
    """
    
    userfile_path = root_folder_path / 'demo_storage'  / str(datetime.today().strftime('%Y-%m-%d'))
    userprams = UserJob()
    
    with open(filepath_url, 'r') as fp:
        folder_aggregated_path = ""
        for folder_iter in iter(fp):
            if  not math.fabs(int(folder_iter) / aggregator_factor ):
                folder_aggregated_path = userfile_path / ("lidarhd_mode" + "_agg_" + str(folder_iter))
                folder_output = folder_aggregated_path / "output"
                os.mkdir(folder_aggregated_path) 
                os.mkdir(folder_output)
            check_call(["wget" , "-O", folder_iter, folder_aggregated_path ])
            if len(os.listdir(folder_aggregated_path)) == aggregator_factor : 
                Popen([root_folder_path / "run_workflow.sh", "--input_dir", folder_aggregated_path, "--output_dir",folder_output ])
    fp.close()
    
@fastapi.post(path="/reconstruction/post")
def run_lidarhd_job(filepath_url:Path, bg: BackgroundTasks):
    """
    Sends the files from the lidarhd to localhost and then runs the operation
    """
    userfile_path = root_folder_path / 'demo_storage'  / str(datetime.today().strftime('%Y-%m-%d'))
    user = UserJob()
    
    if(os.path.isdir(userfile_path)):

            #output = check_output([  root_folder_path / 'run_lidarhd.sh' , '--list_files', filepath_url, '--output_dir', userfile_path ])
            output = launch_single_tile_reconstruction(filepath_url,userfile_path)
            bgtasks.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path)
    else: 
            os.mkdir(userfile_path, mode=0o777)
            output = launch_single_tile_reconstruction(filepath_url,userfile_path)
            bgtasks.add_task(launch_single_tile_reconstruction,filepath_url,userfile_path)
    
    ##storing the laz file for the given user
    num_tiles = 0
    tilenames = []
    with open(filepath_url, 'r') as fp:
        for line in fp:
            filename_params = line.strip().split("/")[-1].split(".")[0]  
            tilenames.append(filename_params)
            num_tiles += 1
            user.files_storage_tiles[filename_params] = []
            # Perform other operations for each tile
            newJobId = str(uuid.uuid1())
            supabase_client.table("extra_surface").insert({
                "email": user.email,
                "job_history": [newJobId]
            })
   
    send_job_reconstruction("malikdhruv1994@gmail.com","0101",  "")
    fp.close()
    return output


class Status(BaseModel):
    job_status: str



@fastapi.post("/reconstruction/schedule")
def schedule_reconstruction_job(data:ScheduleJob) -> Status:
    params = ""
    try:
        url_filepath = Path(data.input_url)
        job_details = queue.enqueue(run_lidarhd_job,[url_filepath])
        params = job_details.result
        return Status(job_status=str(params))
        
    except Exception as e:
        print("error on scheduling reconstruction job:" + str(e))

    return Status(job_status="None")



def concurrency_condition():
    available_messages = queue.all(connection=(redisObj)) #type: ignore
    if available_messages:
        return True
    return False

@fastapi.post("/reconstruction/storage/ipfs")
def storing_result_ipfs(dir_location: str):
    """
    stores the results from the IPFS storage
    """
    try:
       output = check_output(["w3", "up" , dir_location ])  
       print(output)
       
       #return str(response_result["hash"])  #type: ignore
    except Exception as e:
        print("exception in storing ipfs" + str(e))

       
@fastapi.get("/reconstruction/fetch_ipfs")
def fetch_ipfs_result(username:str):
    try:
        ipfs_cid = supabase_client.table("superbase").select("ipfs_dir").contains("tile_name", username)
        return str(ipfs_cid)
    except Exception as e:
        print(str(e))

