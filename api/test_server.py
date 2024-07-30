from fastapi.testclient import TestClient
from api.server import fastapi
from pathlib import Path
import os
from api.cache import redisObj
import datetime
from pytest import mark
testUser = TestClient(fastapi)



def test_schedule_queue():
    url_file_path = "test_dalle.txt"
    email = "test@dev.com"
    expected_output_key = "test@dev.com"
    # Send POST request to the endpoint
    response = testUser.post(url="/reconstruction/schedule", json={"input_url":str(url_file_path), "username" : str(email) })
    print(response.json)

    assert redisObj.get(email) == url_file_path


