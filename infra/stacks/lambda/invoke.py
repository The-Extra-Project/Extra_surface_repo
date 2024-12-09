import boto3
import json
import os


JAR_IQ = "s3://extralabs-artifacts-dev/jar/iqlib.jar"
DOCKER_IMAGE = "767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest"
BOOTSTRAP_SCRIPT_PATH = "s3://extralabs-artifacts-dev/scripts/docker.sh"
SSH_KEY_NAME="extra"


def lambda_handler(event, context):
    region = event.get('region', 'eu-west-1')
    emr_client = boto3.client('emr', region_name=region)

    # Cluster
    cluster_name = "Lambda-Dev"
    release_label = "emr-7.3.0"
    hadoop_version = "3.3.6"
    emr_service_role = "EMR_DefaultRole"

    log_uri = "s3://extralabs-artifacts-dev/emr-logs/"

    # Instancess
    master_instance_type = "m5.xlarge"
    core_instance_type = "m5.xlarge"
    task_instance_type = "m5.xlarge"
    master_instance_count = 1
    core_instance_count = 1
    task_instance_count = 1
    volume_size = 40
    key_name = "extra"
    subnet_id = "subnet-0f85765f2c8a832cf"
    emr_ec2_role = "EMR_EC2_DefaultRole"

    # Configurations
    s3_boostsrap_path = 's3://extralabs-artifacts-dev/scripts/docker.sh'


    try:
        # Create EMR cluster
        response = emr_client.run_job_flow(
            Name=cluster_name,
            ReleaseLabel=release_label,
            Applications=[
                {"Name": "Hadoop"},
                {"Name": "Spark"},
            ],
            Instances={
                'Ec2KeyName': key_name,
                'HadoopVersion': hadoop_version,
                'Ec2SubnetId': subnet_id,

                'KeepJobFlowAliveWhenNoSteps': True,
                'TerminationProtected': False,
                'UnhealthyNodeReplacement': True,
                # 'Ec2SubnetIds': [
                #     'string',
                # ],
                "InstanceGroups": [
                    {
                        "Name": "Master",
                        "Market": "ON_DEMAND",
                        "InstanceRole": "MASTER",
                        "InstanceType": master_instance_type,
                        "InstanceCount": master_instance_count
                    },
                    {
                        "Name": "Core",
                        "Market": "ON_DEMAND",
                        "InstanceRole": "CORE",
                        "InstanceType": core_instance_type,
                        "InstanceCount": core_instance_count
                    },
                    {
                        "Name": "Task",
                        "Market": "ON_DEMAND",
                        "InstanceRole": "TASK",
                        "InstanceType": task_instance_type,
                        "InstanceCount": task_instance_count
                    }
                ],
                'EbsConfiguration': {
                    'EbsBlockDeviceConfigs': [
                        {
                            'VolumeSpecification': {
                                'VolumeType': 'gp3',
                                #'Iops': 123,
                                'SizeInGB': volume_size
                                #'Throughput': 123
                            },
                            'VolumesPerInstance': 1
                        },
                    ],
                    'EbsOptimized': True
                },
            },
            EbsRootVolumeSize = 50,
            Steps=[
                {
                    'Name': 'I - Copy data from S3 to HDFS',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'Args': [
                            "bash", "-c", "aws s3 sync s3://extralabs-artifacts-dev/data/ /home/hadoop/data"
                        ]
                    }
                },
                {
                    'Name': 'II - Preprocess',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'MainClass': 'string',
                        'Args': [
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', 's3://extralabs-artifacts-dev/jar/iqlib.jar',
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={DOCKER_IMAGE}',
                            's3://extralabs-artifacts-dev/scripts/workflow_preprocess.scala'
                        ],
                    }
                },
                {
                    'Name': 'III - Process',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'MainClass': 'string',
                        'Args': [
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', 's3://extralabs-artifacts-dev/jar/iqlib.jar',
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={DOCKER_IMAGE}',
                            's3://extralabs-artifacts-dev/scripts/workflow_wasure.scala'
                        ],
                    }
                },
            ],
            Configurations=[
                {
                    'Classification': 'delta-defaults',
                    'Properties': {
                        "delta.enabled": "true"
                    }
                },
                {
                    'Classification': 'yarn-site',
                    'Properties': {
                        "yarn.container.runtime.type": "docker",
                        "yarn.container.docker.image": DOCKER_IMAGE,
                        "yarn.nodemanager.runtime.linux.docker.default-rw-mounts": "/home/hadoop/data:/app/datas",
                    }
                },
                {
                    'Classification': 'spark-defaults',
                    'Properties': {
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": DOCKER_IMAGE,
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_MOUNTS": "/home/hadoop/data:/app/datas:rw,/home/hadoop/out:/app/out:rw,/home/hadoop/logs:/tmp:rw",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": DOCKER_IMAGE,
                        "spark.rdd.compress": "true",
                        "spark.eventLog.enabled": "true",
                        "spark.driver.allowMultipleContexts": "true",
                        "spark.memory.offHeap.enabled": "true",
                        "spark.memory.offHeap.size": "10g",
                        "spark.memory.storageFraction": "0.8",
                        "spark.serializer": "org.apache.spark.serializer.KryoSerializer",
                    }
                },
                {
                    'Classification': 'spark-env',
                    'Configurations': {
                        'Classification': 'export',
                        'Properties': {
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0",
                            "DDT_MAIN_DIR": "/app/",
                            "INPUT_DATA_DIR": "/app/datas/",
                            "OUTPUT_DATA_DIR": "/app/out/",
                            "GLOBAL_BUILD_DIR": "/app/build/",
                            "PARAM_PATH": "${INPUT_DATA_DIR}/params.xml",
                        }
                    }
                },
                {
                    'Classification': 'mapred-site',
                    'Properties': {
                        "mapred.job.jvm.num.tasks": "-1"
                    }
                },
                {
                    'Classification': 'hadoop-env',
                    'Configurations': {
                        'Classification': 'export',
                        'Properties': {
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0",
                        }
                    }
                },
            ],
            BootstrapActions=[
                {
                    'Name': 'docker',
                    'ScriptBootstrapAction': {
                        'Path': s3_boostsrap_path,
                    }
                },
            ],
            LogUri=log_uri,
            JobFlowRole=emr_ec2_role,
            ServiceRole=emr_service_role,
            VisibleToAllUsers=True,
            scale_down_behavior="TERMINATE_AT_TASK_COMPLETION",
            Tags=[
                {"Key": "Environment", "Value": "Development"},
                {"Key": "for-use-with-amazon-emr-managed-policies", "Value": "True"}
            ]
        )

        # Return cluster ID
        return {
            'statusCode': 200,
            'body': json.dumps({
                "message": "EMR cluster created successfully",
                "ClusterId": response['JobFlowId']
            })
        }

    except Exception as e:
        # Log and return error
        print(f"Error creating EMR cluster: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps({
                "message": "Failed to create EMR cluster",
                "error": str(e)
            })
        }