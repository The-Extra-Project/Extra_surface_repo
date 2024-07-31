from fastapi.testclient import TestClient
from api.server import fastapi, queue
from pathlib import Path
import os

from api.cache import redisObj
import datetime
testUser = TestClient(fastapi)



def test_schedule_queue():
    url_file_path = "test_averyon.txt"
    email = "test@dev.com"
    expected_output_key = "test@dev.com"
    # Send POST request to the endpoint
    response = testUser.post(url="/reconstruction/schedule", json={"input_url":str(url_file_path), "username" : str(email) })
    
    assert queue.count == 1
    assert response.status_code == 200




