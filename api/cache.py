from upstash_redis import Redis
from redis import Redis
import logging
import sys
import os
from .models import ScheduleJob
from pathlib import Path
import s3fs
from dotenv import load_dotenv

root_folder_path = Path(os.path.abspath(__file__)).parent.parent

load_dotenv(dotenv_path=(root_folder_path  / '.env'))

redisObj = Redis()

def enqueue_job(job_params: ScheduleJob):
    """
        
    """
    try:
        # Perform the job enqueueing logic, for example, storing the message in Redis
        msgId = redisObj.set(name=job_params.username, value=job_params.input_url)
        print("The message is stored as tuple: {}: {} ".format(job_params.username,job_params.input_url))
        return msgId
    except Exception as e:
        print("Exception caught in enque: " + str(e))
        return None
    

def dequeue_job(email_params):
    try:
        values = redisObj.get(email_params)
        redisObj.delete(email_params)
        return str(values)
    except Exception as e:
        print("Exception caught deq" + str(e))

def current_job_index(key_to_check):
    """
    Get the number of messages stored in the queue before the given key.
    """
    try:
        keys = redisObj.keys(pattern='**@**')  # Get all keys in the Redis queue
          # Find the index of the given key
        return keys
    except ValueError:
        print("Key not found in the queue.")
    except Exception as e:
        print("Exception caught in current_job_index: " + str(e))
