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
    defines the command to be executed for the given running cluster.
    in our case it will be just running the sparkling_washeur container with the S3 directories as params.
    """
    try:
        steps = emr_obj.add_job_flow_steps(
            JobFlowId= job_flow_id,   
            Steps=steps)
        return steps
    except Exception as e:
        print("Not able to define the job flow")
    
def run_compute_job_handler(event, context): 
    try:
        steps = [{
              'Name': 'Run Spark Job',
                'ActionOnFailure': 'CONTINUE',
                'HadoopJarStep': {
                    'Jar': 'command-runner.jar',
                    'Args': [
                        'spark-submit',
                        '--deploy-mode', 'cluster',
                        '--master', 'yarn',
                        f'/app/wasure/spark_job_reconstruction.py',
                        username,
                        file_path
                    ]
                }
        }]
        
        job_flow = define_job_flow(os.getenv("S3_CLUSTER_NAME"), steps)
        ## replace with your actual cluster ID.
        cluster_id = os.getenv("S3_CLUSTER_NAME")
        
        ## now execute the job flow
        response = emr_obj.run_job_flow(
            JobFlowId=cluster_id,
            Steps=steps
        )
        print(f"Scheduled Spark job with steps: {response['StepIds']}")
        return response
    except Exception as e:
        print("Error scheduling Spark job: " + str(e))