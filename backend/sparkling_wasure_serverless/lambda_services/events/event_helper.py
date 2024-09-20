from typing import Sequence
from anyio import Event
from boto3.session import Session
from mypy_boto3_sns import SNSClient
from mypy_boto3_events import EventBridgeClient
from mypy_boto3_events.type_defs import PutEventsRequestEntryTypeDef
from mypy_boto3_s3 import S3Client
from mypy_boto3_cloudtrail import CloudTrailClient
from mypy_boto3_lambda import LambdaClient
from pathlib import Path
from mypy_boto3_sqs import SQSClient
import logging
from dotenv import load_dotenv
import os
import json
import datetime
from dotenv import load_dotenv
from mypy_boto3_stepfunctions import StepFunctionsClient

def cloudtrail_client() -> CloudTrailClient:
    return Session().client("cloudtrail")

def get_event_bridge_client() -> EventBridgeClient:
    return Session().client("events")

def get_sns_client() -> SNSClient:
    return Session().client("sns")     

def get_queue_client() -> SQSClient:
    return Session().client("sqs")

def get_s3_client() -> S3Client:
    return Session().client("s3")     

def get_lambda_client() -> LambdaClient:
    return Session().client("lambda")

def get_step_functions_client() -> StepFunctionsClient:
    return Session().client("stepfunctions")


class JobQueueParams(BaseModel):
    username: str
    file_path: str
    
s3_obj = get_s3_client()
sns_obj = get_sns_client()
event_obj = get_event_bridge_client()
queue_obj = get_queue_client()
lambda_obj = get_lambda_client()
load_dotenv(dotenv_path=".env")
loggerObj = logging.getLogger("publisher")

topicArn = os.getenv("SNS_TOPIC_OUTPUT")


load_dotenv(
        Path(__file__)/ ".env"
    )
## optional: we can either filter rules later for the specific files downloaded or detect just the folder creation.
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
    } 
    
    trigger_rule_folder_created = f""
    {
        "source": ["aws.s3"],
        "detail-type": ["AWS API Call via CloudTrail"],
        "detail": {
            "eventSource": ["s3.amazonaws.com"],
            "eventName": ["CreateFolder"],
            "requestParameters": {
                "bucketName": os.getenv("S3_BUCKET"),
                "key": [
                    {
                        "prefix": "extra-protocol-lidarhd/{username}/"
                    }
                ]
            }
        }
    } 
    ""
    trigger_rule_xml_file_created = f""
    try:
        event_obj.update_event_bus(Name="trigger_rule_url_param", Description=trigger_rule_url_file_upload)
        event_obj.put_rule(Name="UrlFileUploaded", EventPattern=trigger_rule__url_file_upload)
    except Exception as e:
        print("error in setting up the rule" + str(e))

def fetch_cloudtrail_event() -> Any:
    """
    fetches the cloudtrail event based on the particular file / folder object created in the S3 bucket.
    """
    
    try:
                # Define the time range for the events to fetch
        end_time = datetime.datetime.now()
        start_time = end_time - datetime.timedelta(hours=12)  # Fetch events from the last hour (safe to assume that it doesnt overflow).

        response_events = cloudtrail_client.lookup_events(
            StartTime=start_time,
            EndTime=end_time,
            LookupAttributes=[
                {
                    'AttributeKey': 'EventName',
                    'AttributeValue': 'PutObject'  # Filter for S3 object creation events
                }
            ],
            MaxResults=1
        )
                # Check if any events were found
        if response['Events']:
            return response['Events'][0]  
        else:
            return None  
    except Exception as e:
        print("error in fetching the cloudtrail event" + str(e))
        return None

def send_to_job_quque(messageBody: JobQueueParams):
    """
    this function sends the message from the event received to the job queue.
    messageBody: the message (in json format) to be sent to the job queue.
    """
    try:
        sqs_client = get_queue_client()
        sqs_client.send_message(
            QueueUrl= str(os.getenv("JOB_QUEUE_URL")),
            MessageBody= messageBody.model_dump_json()
        )
        logger.info(f"Message sent to SQS: {response['MessageId']}")
    except Exception as e:
        print("error in sending the message to the job queue" + str(e))


def read_queue_and_execute_job():
    """
    Gets the message (1 per time) from the FIFO bridge.
    And then schedules the job by invoking the Step Function for the EMR job.
    """
    try:
        loggerObj.info("Getting the job queue message from the SQS")
        queueObj = get_queue_client()
        s3_result = queueObj.receive_message(QueueUrl=str(os.getenv("SNS_TOPIC")), MaxNumberOfMessages=1, WaitTimeSeconds=0)
        message = s3_result.get("Messages", [])[0]
        
        if not message:
            loggerObj.info("No message in the queue")
            return
        
        message_body = json.loads(message["Body"])

        # Now invoke the Step Function for the EMR job
        step_function_client = get_step_functions_client()
        response = step_function_client.start_execution(
            stateMachineArn=os.getenv("EMR_JOB_STATE_MACHINE_ARN"),
            input=json.dumps(message_body)
        )
        
        loggerObj.info(f"Step Function invoked for EMR job with response: {response}")
        
        # Optionally, you can check the status of the Step Function execution here
        execution_arn = response['executionArn']
        check_step_function_status(execution_arn)

        return message        
    except Exception as e:
        print("Error on event bridge consumer: " + str(e))

def check_step_function_status(execution_arn: str):
    """
    Checks the status of the Step Function execution.
    """
    step_function_client = get_step_functions_client()
    while True:
        response = step_function_client.describe_execution(executionArn=execution_arn)
        status = response['status']
        
        if status in ['SUCCEEDED', 'FAILED', 'TIMED_OUT', 'ABORTED']:
            print(f"Step Function execution finished with status: {status}")
            break
        
        print(f"Step Function execution is still running...")
        time.sleep(3600)  
        # Wait the polling interval around 1 hr before checking again.



## reference taken from the SNS examples: https://github.com/awsdocs/aws-doc-sdk-examples/blob/main/python/example_code/sns/sns_basics.py .
# def publish_sns_topic(message, attributes):
#     """
#     Publishes the notification for the user notifications for the various changes like:
#     - uploading of the final result of the ply/mesh files generated.
#     - message: this corresponds to the given user email whose files are generated 
#     - Attributes: corresponds to the additional metadata of the generated files.
#     """
#     sns_client = get_sns_client()
#     try:        
#         sns_client.subscribe(TopicArn=str(os.getenv("TOPIC_ARN")), Protocol="HTTP/HTTPS")
#         attribute_file_output = {}
#         sns_client.publish(
#             Message= message,
#             TopicArn= str(os.getenv("SNS")),
#         )
#         loggerObj.info("Successfully sent message %s ")
#     except Exception as e:
#             print("error in publishing SNS topic:" + str(e))

# ## for the S3 bridge events.
# def publish_event_bridge(file_extension, data):
#     """
#     fetches the events related to the file operation on AWS like:
#     - put object in the S3. 
#     - (for the parsing of the laz files for the results).
    
#     Input params:
#     1. file_extension is the file object stored on the S3 bucket whose changes is to be monitored (.txt and .copc.laz).
#     """ 
#     try:    
#         loggerObj.info(msg=f"event being published for :{file_extension}")

#         event_metadata : Sequence[PutEventsRequestEntryTypeDef]= [{
#         'Time': datetime.datetime.now(),
#         'Source': 'frontend-extralabs',
#         'Detail': data,
#         'DetailType': 'upload_txt_files',
#         'Resources': []
#         }                                                          
#         ]
#         eventObj = get_event_bridge_client()
#         eventObj.put_events(
#             Entries= event_metadata
#         )
#     except Exception as e:
#         print("unable to fetch the events for files due to error:+ ", str(e))



    def notify_email_service(email_category: str, *params):
        """
        this function is used to notify the email service for the various events like:
        - job reconstruction
        - job results
        - payment notifications.
        and call the lambda function for the email sender microservice.
        """
        try:
            payload = {
                "email_category": email_category,
                "details": list(params)
            }
            response = lambda_client.invoke(
            FunctionName='email_sending_serverless',  # Replace with the actual function name
            InvocationType='Event',  # Use 'RequestResponse' for synchronous invocation
            Payload=json.dumps(payload)
            )
            loggerObj.info(f"email notification parameters send to the email lambda function with category: {email_category} and params: {params}")
            return response
        except Exception as e:
            print("error in  sending the notification email" + str(e))




def eventhandler(event, context):
    """
    AWS Lambda handler function to process incoming events, trigger the EMR job,
    and send email notifications upon completion.
    """
    try:
        # Extract necessary information from the event
        username = event.get("username")
        email = event.get("email")
        file_path = event.get("file_path")

        # Create JobQueueParams object
        job_params = JobQueueParams(username=username, file_path=file_path)

        # Send message to job queue
        send_to_job_quque(job_params)

        # Read from the queue and execute the job
        message = read_queue_and_execute_job()

        if message:
            # Notify email service upon job completion
            notify_email_service("Job Completed", username, file_path)
        else:
            loggerObj.info("No message received from the job queue.")

    except Exception as e:
        print("Error in handler function: " + str(e))
        loggerObj.error(f"Error processing event: {str(e)}")