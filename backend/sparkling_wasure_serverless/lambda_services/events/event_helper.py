from typing import Sequence
from anyio import Event
from boto3.session import Session
from mypy_boto3_sns import SNSClient
from mypy_boto3_events import EventBridgeClient
from mypy_boto3_events.type_defs import PutEventsRequestEntryTypeDef
from mypy_boto3_s3 import S3Client
from mypy_boto3_cloudtrail import CloudTrailClient
from pathlib import Path
from mypy_boto3_sqs import SQSClient
import logging
from dotenv import load_dotenv
import os
import json
import datetime
from dotenv import load_dotenv



def cloudtrail_client() -> CloudTrailClient:
    return Session().client("cloudtrail")

def get_event_bridge_client() -> EventBridgeClient:
    return Session().client("events")


event_obj = get_event_bridge_client()
rule_url_file = "UrlTextStoredExtraSurface"



## optional: we can either filter rules later for the specific files
def activate_url_upload_rule(email, username):

    trigger_rule__url_file_upload = f""
    {
        "source": ["aws.s3"],
        "detail-type": ["Object Created"],
        "detail": {
            "bucket": {
            "name": [os.getenv("S3_BUCKET")]
            },
            "object": {
            "key": [
                {
                "prefix": "extra-protocol-lidarhd/{username}"
            }
            ]
            }
        }
    } # type: ignore
    ""
    try:
        event_obj.update_event_bus(Name="trigger_rule_url_param", Description=trigger_rule__url_file_upload)
        event_obj.put_rule(Name="UrlFileUploaded", EventPattern=trigger_rule__url_file_upload)
    except Exception as e:
        print("error in setting up downloading rule" + str(e))

load_dotenv(
        Path(__file__)/ ".env"
    )


load_dotenv(dotenv_path=".env")
loggerObj = logging.getLogger("publisher")

def get_s3_client() -> S3Client:
    return Session().client("s3")     

def get_sns_client() -> SNSClient:
    return Session().client("sns")     


def get_queue_client() -> SQSClient:
    return Session().client("sqs")

s3Object = get_s3_client()
topicArn = "arn:aws:sns:us-east-1:573622188359:Extrasurface_job_execution.fifo"

def upload_file_s3(filepath, email):
    try:
        s3Object.upload_file(Filename=filepath,Bucket=str(os.getenv("S3_BUCKET")), Key=email)
        upload_status = s3Object.get_object(Bucket=str(os.getenv("S3_BUCKET")),Key=email)
        if upload_status["ResponseMetadata"]["HTTPStatusCode"] == "404":
            return False
        return True
    except Exception as e:
        print("error in uploading the file" + str(e))

## reference taken from the SNS examples: https://github.com/awsdocs/aws-doc-sdk-examples/blob/main/python/example_code/sns/sns_basics.py .
def publish_sns_topic(message, attributes):
    """
    Publishes the notification for the user notifications for the various changes like:
    - uploading of the final result of the plyy/mesh files generated.
    message: this corresponds to the given user email whose files are generated 
    attributes: corresponds to the additional metadata of the generated files.
    """
    sns_client = get_sns_client()
    try:        
        sns_client.subscribe(TopicArn=topicArn, Protocol="HTTP/HTTPS")
        attribute_file_output = {}
        sns_client.publish(
            Message= message,
            TopicArn= str(os.getenv("SNS")),
        )
        loggerObj.info("Successfully sent message %s ")
    except Exception as e:
            print("error in publishing SNS topic:" + str(e))

## for the S3 bridge events.
def event_bridge_producer(file_extension, data):
    """
    fetches the events related to the file operation on AWS like:
    - put object in the S3. 
    - (for the parsing of the laz files for the results).
    
    Input params:
    1. file_extension is the file object stored on the S3 bucket whose changes is to be monitored (.txt and .copc.laz).
    """ 
    try:    
        loggerObj.info(msg=f"event being published for :{file_extension}")

        event_metadata : Sequence[PutEventsRequestEntryTypeDef]= [{
        'Time': datetime.datetime.now(),
        'Source': 'frontend-extralabs',
        'Detail': data,
        'DetailType': 'upload_txt_files',
        'Resources': []
        }                                                          
        ]
        eventObj = get_event_bridge_client()
        eventObj.put_events(
            Entries= event_metadata
        )
    except Exception as e:
        print("unable to fetch the events for files due to error:+ ", str(e))

def get_sqs_event():
    """
    gets the message (1 per time) from the FIFO bridge.
    
    """
    try:
        loggerObj.info("waiting for getting the events of S3 object addition from the bridge changes")
        queueObj = get_queue_client()
        ## while the messages are not being acknowledged , you should store the messages locally and get the content from the message
        s3_result = queueObj.receive_message(QueueUrl=str(os.getenv("SNS_TOPIC")), MaxNumberOfMessages=1, WaitTimeSeconds=0)
        message = s3_result.get("Messages", [])[0]
        return message
        
        
    except Exception as e:
        print("error on event bridge consumer" + str(e))
        

def reconstruction_result_notification():
    """
    reads the notification of completed apache job and fetches the url of the signed download of output files and result.
    
    """
    
    try:
        loggerObj.info("fetch the SNS event for result updates")
        sns_obj = get_sns_client()
        sns_message = sns_obj.subscribe(TopicArn="arn:aws:sns:us-east-1:573622188359:Extrasurface_job_execution.fifo", Protocol="HTTP/HTTPS")
            
    except Exception as e:
        print("error in getting the messages")
    
    
    