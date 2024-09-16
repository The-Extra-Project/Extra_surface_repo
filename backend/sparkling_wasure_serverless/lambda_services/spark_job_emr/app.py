from typing import Sequence
from mypy_boto3_emr import EMRClient
from mypy_boto3_emr.type_defs import  StepConfigUnionTypeDef
from boto3.session import Session
import os

class SparkInputContext():
    inputS3Uri: str
    OutputS3Uri: str
    

emr_obj: EMRClient = Session().client("emr")

cluster_params = {
            "MasterInstanceType": "m5.xlarge",
            "Ec2SubnetIds": ["i-06a4c608effe8a5cd", "i-0dc516dff1f2e718c"],
            "SlaveInstanceType": "m5.xlarge"
        }

def get_cluster_info(cluster_name: str):
    try:
        cluster_metadata = emr_obj.describe_cluster(ClusterId=str(os.getenv("S3_CLUSTER_NAME")))
        return cluster_metadata["Cluster"]    
    except Exception as e:
        print("job didnt execute: " + str(e))


def define_job_flow(job_flow_id: str, steps: Sequence[StepConfigUnionTypeDef]):
    """
    defines the command to be executed for the given cluster container.
    in our case it will be just running the sparkling_washeur container with the S3 directories as params.
    """
    try:
        emr_obj.add_job_flow_steps(
            JobFlowId= job_flow_id,   
            Steps=steps)
        
    except Exception as e:
        print("Not able to define the job flow")
    

def run_compute_job(event, context): 
    try:
        params = event