#!/usr/bin/python
import pytest
from server import upload_file_s3, run_lidarhd_job
from event_architecture import publish_sns_topic, get_sqs_event, get_event_bridge_client
from pathlib import Path
def test_file_uploads_and_generates_event():
    event_bridge_client = get_event_bridge_client()
    sqs_event = get_sqs_event()
    filepath = Path(__file__) / "datas" / "liste_dalle.txt"
    is_uploaded = upload_file_s3(filepath=filepath, email="test@demo.txt")
    assert is_uploaded == True
    
    archives =  event_bridge_client.list_archives(NamePrefix="UrlFilesUploadEvents") 
    assert archives["ResponseMetadata"]["HTTPStatusCode"] == 404
    
    retrieved_message =  get_sqs_event()
    assert str(retrieved_message) is not None
    
