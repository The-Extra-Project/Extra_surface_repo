from fastapi.testclient import TestClient
from api.server import fastapi, queue
from pathlib import Path
import os
from rq.job import Job

import resend

from api.cache import redisObj
import datetime

testUser = TestClient(fastapi)



def test_schedule_reconstruction():
    url_file_path = "test_averyon.txt"
    email = "test@gmail.com"
    # Send POST request to the endpoint
    response =  testUser.post(url="/reconstruction/schedule", json={"input_url":str(url_file_path), "username" : str(email) })
    assert response.json()["job_status"] == True
    assert response.status_code == 200
    assert redisObj.get(email) =="test@gmail.com"
    ## also the email has to be send to the given client for notification
    assert resend.Emails.get("1") is not None

def test_reconstruction_multiple_pipeline():
    response = testUser.post(url="/reconstruction/multi", json={"filepath_url": str("/home/ubuntu/app/spark-ddt-laurent/datas/list_averyon.txt"), "aggregator_factor": 4})
    assert response.json()[""]
    #assert 

    