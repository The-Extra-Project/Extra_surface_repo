import json
import boto3
import os


# Cluster
region = os.environ.get('region', "eu-west-1")
release_label = os.environ.get('release_label', 'emr-7.3.0')
hadoop_version = os.environ.get('hadoop_version', '3.3.6')
emr_service_role = os.environ.get('emr_service_role')
instance_profile = os.environ.get('instance_profile')
subnet_id = os.environ.get('subnet_id', "subnet-0f85765f2c8a832cf")
key_name = os.environ.get('key_name', "extra")
vol_root_size = os.environ.get('vol_root_size', 30)

# App
docker_image = os.environ.get('docker_image', None)
s3_path = os.environ.get('s3_path', None)
s3_path_input = os.environ.get('s3_path_input')
s3_path_bootstrap = os.environ.get('s3_path_bootstrap')
s3_path_iqlib = os.environ.get('s3_path_iqlib')
s3_path_preprocess = os.environ.get('s3_path_preprocess')
s3_path_process = os.environ.get('s3_path_process')
log_uri = os.environ.get('log_uri')
main_class = os.environ.get('main_class', "WorkflowWasure")
emr_client = boto3.client('emr', region_name=region)


def lambda_handler(event, context):
    # Cluster
    cluster_name = event.get('cluster_name', 'Dev')

    # Instances
    master_instance_type = event.get('master_instance_type', "m5.xlarge")
    master_instance_count = event.get('master_instance_count', 1)
    core_instance_type = event.get('core_instance_type', "m5.xlarge")
    core_instance_count = event.get('core_instance_count', 1)
    task_instance_type = event.get('task_instance_type', "m5.xlarge")
    task_instance_count = event.get('task_instance_count', 1)
    vol_size = event.get('vol_size', 30)

    # App Configurations
    spark_heap_size = event.get('spark_heap_size', "2g")
    spark_mem_fraction = event.get('spark_mem_fraction', "0.8")

    # Create EMR cluster
    try:
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
                        "InstanceCount": master_instance_count,
                        'EbsConfiguration': {
                            'EbsBlockDeviceConfigs': [
                                {
                                    'VolumeSpecification': {
                                        'VolumeType': 'gp3',
                                        'SizeInGB': vol_size,
                                        # 'Iops': 123,
                                        # 'Throughput': 123
                                    },
                                    'VolumesPerInstance': 1
                                },
                            ],
                            'EbsOptimized': True
                        },
                    },
                    {
                        "Name": "Core",
                        "Market": "ON_DEMAND",
                        "InstanceRole": "CORE",
                        "InstanceType": core_instance_type,
                        "InstanceCount": core_instance_count,
                        'EbsConfiguration': {
                            'EbsBlockDeviceConfigs': [
                                {
                                    'VolumeSpecification': {
                                        'VolumeType': 'gp3',
                                        'SizeInGB': vol_size,
                                        # 'Iops': 123,
                                        # 'Throughput': 123
                                    },
                                    'VolumesPerInstance': 1
                                },
                            ],
                            'EbsOptimized': True
                        },
                    },
                    {
                        "Name": "Task",
                        "Market": "ON_DEMAND",
                        "InstanceRole": "TASK",
                        "InstanceType": task_instance_type,
                        "InstanceCount": task_instance_count,
                        'EbsConfiguration': {
                            'EbsBlockDeviceConfigs': [
                                {
                                    'VolumeSpecification': {
                                        'VolumeType': 'gp3',
                                        'SizeInGB': vol_size,
                                        # 'Iops': 123,
                                        # 'Throughput': 123
                                    },
                                    'VolumesPerInstance': 1
                                },
                            ],
                            'EbsOptimized': True
                        },
                    }
                ],
            },
            EbsRootVolumeSize=int(vol_root_size),
            Steps=[
                {
                    'Name': '1 - Copy data from S3 to HDFS',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'Args': [
                            "bash", "-c", f"aws s3 sync {s3_path_input} /home/hadoop/data"
                        ]
                    }
                },
                {
                    'Name': '2 - Preprocess',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'MainClass': main_class,
                        'Args': [
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', s3_path_iqlib,
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={docker_image}',
                            s3_path_preprocess
                        ],
                    }
                },
                {
                    'Name': '3 - Process',
                    'ActionOnFailure': 'CANCEL_AND_WAIT',
                    'HadoopJarStep': {
                        'Jar': 'command-runner.jar',
                        'MainClass': main_class,
                        'Args': [
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', s3_path_iqlib,
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={docker_image}',
                            s3_path_process
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
                        "yarn.container.docker.image": docker_image,
                        "yarn.nodemanager.runtime.linux.docker.default-rw-mounts": "/home/hadoop/data:/app/datas",
                    }
                },
                {
                    'Classification': 'spark-defaults',
                    'Properties': {
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": docker_image,
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_MOUNTS": "/home/hadoop/data:/app/datas:rw,/home/hadoop/out:/app/out:rw,/home/hadoop/logs:/tmp:rw",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": docker_image,
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
                    'Configurations': [{
                        'Classification': 'export',
                        'Properties': {
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0",
                            "DDT_MAIN_DIR": "/app/",
                            "INPUT_DATA_DIR": "/app/datas/",
                            "OUTPUT_DATA_DIR": "/app/out/",
                            "GLOBAL_BUILD_DIR": "/app/build/",
                            "PARAM_PATH": "/app/datas/params.xml",
                        }
                    }]
                },
                {
                    'Classification': 'mapred-site',
                    'Properties': {
                        "mapred.job.jvm.num.tasks": "-1"
                    }
                },
                {
                    'Classification': 'hadoop-env',
                    'Configurations': [{
                        'Classification': 'export',
                        'Properties': {
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0",
                        }
                    }]
                },
            ],
            BootstrapActions=[
                {
                    'Name': 'docker',
                    'ScriptBootstrapAction': {
                        'Path': s3_path_bootstrap,
                    }
                },
            ],
            LogUri=log_uri,
            JobFlowRole=instance_profile,
            ServiceRole=emr_service_role,
            VisibleToAllUsers=True,
            ScaleDownBehavior="TERMINATE_AT_TASK_COMPLETION",
            Tags=[
                # mandatory tag
                {"Key": "for-use-with-amazon-emr-managed-policies", "Value": "True"},
                {"Key": "Environment", "Value": "Development"}
            ]
        )

        print(f"EMR cluster created! clusterId: {response['JobFlowId']}")
        # Return cluster ID
        return {
            'statusCode': 200,
            'body': json.dumps({
                "message": "EMR cluster created successfully",
                "clusterId": response['JobFlowId']
            })
        }

    except Exception as e:
        # Log and return error
        print(f"Error creating EMR cluster: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps({
                "error": str(e)
            })
        }