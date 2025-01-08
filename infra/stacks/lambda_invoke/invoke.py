import json
import boto3


REGION = "eu-west-1"

DOCKER_IMAGE = "767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest"

S3_PATH = "s3://extralabs-artifacts-dev"
S3_PATH_INPUT = f"{S3_PATH}/data"
S3_PATH_BOOTSTRAP = f"{S3_PATH}/scripts/docker.sh"
S3_PATH_IQLIB = f"{S3_PATH}/jar/iqlib.jar"
S3_PATH_PREPROCESS = f"{S3_PATH}/jar/preprocess-0.3.jar"
S3_PATH_PROCESS = f"{S3_PATH}/jar/process-0.3.jar"


def lambda_handler(event, context):
    region = event.get('region', REGION)
    emr_client = boto3.client('emr', region_name=region)

    # Cluster
    cluster_name = event.get('cluster_name', 'Lambda-Dev')
    release_label = event.get('release_label', "emr-7.3.0")
    hadoop_version = event.get('hadoop_version', '3.3.6')
    emr_service_role = event.get('emr_service_role', 'EMR_DefaultRole')
    log_uri = event.get('log_uri', f"{S3_PATH}/emr-logs")

    # Instances
    master_instance_type = event.get('master_instance_type', "m5.xlarge")
    master_instance_count = event.get('master_instance_count', 1)
    core_instance_type = event.get('core_instance_type', "m5.xlarge")
    core_instance_count = event.get('core_instance_count', 1)
    task_instance_type = event.get('task_instance_type', "m5.xlarge")
    task_instance_count = event.get('task_instance_count', 1)
    vol_size = event.get('vol_size', 30)
    vol_root_size = event.get('vol_root_size', 30)
    key_name = event.get('key_name', "extra")
    subnet_id = event.get('subnet_id', "subnet-0f85765f2c8a832cf")
    emr_ec2_role = event.get('emr_ec2_role', "EMR_EC2_DefaultRole")

    # App Configurations
    spark_heap_size = event.get('spark_heap_size', "2g")
    spark_mem_fraction = event.get('spark_mem_fraction', "0.8")

    try:
        # Create EMR cluster
        response = emr_client.run_job_flow(
            Name=cluster_name,
            ReleaseLabel=release_label,
            Applications=[
                {"Name": "Hadoop"}, {"Name": "Spark"},
            ],
            Instances={
                'Ec2KeyName': key_name,
                'HadoopVersion': hadoop_version,
                'Ec2SubnetId': subnet_id,
                # 'Ec2SubnetIds': [
                #     'string',
                # ],
                'KeepJobFlowAliveWhenNoSteps': True,
                'TerminationProtected': False,
                'UnhealthyNodeReplacement': True,
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
                                'SizeInGB': vol_size
                                # 'Iops': 123,
                                # 'Throughput': 123
                            },
                            'VolumesPerInstance': 1
                        },
                    ],
                    'EbsOptimized': True
                },
            },
            EbsRootVolumeSize=vol_root_size,
            Steps=[
                {
                    'Name': '1 - Copy data from S3 to HDFS',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'Args': [
                            "bash", "-c", f"aws s3 sync {S3_PATH_INPUT} /home/hadoop/data"
                        ]
                    }
                },
                {
                    'Name': '2 - Preprocess',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'MainClass': 'WorkflowWasure',
                        'Args': [
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', S3_PATH_IQLIB,
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={DOCKER_IMAGE}',
                            S3_PATH_PREPROCESS
                        ],
                    }
                },
                # {
                #     'Name': '3 - Process',
                #     'ActionOnFailure': 'CANCEL_AND_WAIT',
                #     'HadoopJarStep': {
                #         'Jar': 'command-runner.jar',
                #         'MainClass': 'WorkflowWasure',
                #         'Args': [
                #             'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                #             '--jars', S3_PATH_IQLIB,
                #             '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                #             '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={DOCKER_IMAGE}',
                #             S3_PATH_PROCESS
                #         ],
                #     }
                # },
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
                        "spark.memory.offHeap.size": spark_heap_size,
                        "spark.memory.storageFraction": spark_mem_fraction,
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
                            "PARAM_PATH": "/app/datas/params.xml",
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
                        'Path': S3_PATH_BOOTSTRAP,
                    }
                },
            ],
            LogUri=log_uri,
            JobFlowRole=emr_ec2_role,
            ServiceRole=emr_service_role,
            VisibleToAllUsers=True,
            scale_down_behavior="TERMINATE_AT_TASK_COMPLETION",
            Tags=[
                # mandatory tag
                {"Key": "for-use-with-amazon-emr-managed-policies", "Value": "True"},
                {"Key": "Environment", "Value": "Development"}
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