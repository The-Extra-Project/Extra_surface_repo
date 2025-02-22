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
ecr_endpoint = os.environ.get('ecr_endpoint')
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
                    # Refactor: Uncomment for production
                    # {
                    #     "Name": "Task",
                    #     "Market": "ON_DEMAND",
                    #     "InstanceRole": "TASK",
                    #     "InstanceType": task_instance_type,
                    #     "InstanceCount": task_instance_count,
                    #     'EbsConfiguration': {
                    #         'EbsBlockDeviceConfigs': [
                    #             {
                    #                 'VolumeSpecification': {
                    #                     'VolumeType': 'gp3',
                    #                     'SizeInGB': vol_size,
                    #                     # 'Iops': 123,
                    #                     # 'Throughput': 123
                    #                 },
                    #                 'VolumesPerInstance': 1
                    #             },
                    #         ],
                    #         'EbsOptimized': True
                    #     },
                    # }
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
                            "bash", "-c", f"hdfs dfs -mkdir -p /user/hadoop/{{input,output}} && hdfs dfs -cp {s3_path_input}/* /user/hadoop/input"
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
                            '--class', 'WorkflowWasure', '--jars', s3_path_iqlib,
                            s3_path_preprocess,
                            # Docker image paths
                            'DDT_MAIN_DIR=/app/',
                            'GLOBAL_BUILD_DIR=/app/build/',
                            # Cluster hdfs path
                            'INPUT_DATA_DIR=/user/hadoop/input',
                            'OUTPUT_DATA_DIR=/user/hadoop/input',
                            'PARAM_PATH=/user/hadoop/input/params.xml',
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
                            '--class', 'WorkflowWasure', '--jars', s3_path_iqlib,
                            s3_path_process,
                            # Refactor: add args, see job 2 above
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
                    'Classification': 'spark-defaults',
                    'Properties': {
                        # Override JVM
                        "spark.executorEnv.JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        "spark.yarn.appMasterEnv.JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        # Docker settings
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": docker_image,
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": docker_image,
                        # From Waseur repo
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
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        }
                    }]
                },
                {
                    'Classification': 'hadoop-env',
                    'Configurations': [{
                        'Classification': 'export',
                        'Properties': {
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
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
                    'Classification': 'yarn-site',
                    'Properties': {
                        # Node Labels
                        "yarn.node-labels.enabled": "true",
                        "yarn.node-labels.am.default-node-label-expression": "ON_DEMAND",
                        "yarn.node-labels.configuration-type": "distributed",

                        "yarn.nodemanager.runtime.linux.docker.ecr-auto-authentication.enabled": "true",
                        "yarn.nodemanager.runtime.linux.docker.docker-client-credential-provider.class": "org.apache.hadoop.security.authentication.EcrDockerClientCredentialProvider",

                        "yarn.nodemanager.container-executor.class": "org.apache.hadoop.yarn.server.nodemanager.LinuxContainerExecutor",
                        "yarn.nodemanager.linux-container-executor.path": "/usr/lib/hadoop-yarn/bin/container-executor",
                        "yarn.nodemanager.linux-container-executor.group": "hadoop",
                        "yarn.nodemanager.linux-container-executor.nonsecure-mode.limit-users": "false",
                        "yarn.nodemanager.runtime.linux.docker.allowed-container-runtimes": "docker",
                        "yarn.nodemanager.runtime.linux.type": "docker",
                        "yarn.nodemanager.runtime.linux.allowed-runtimes": "docker",
                        "yarn.nodemanager.runtime.linux.docker.image-update": "true",
                        "yarn.nodemanager.runtime.linux.docker.image-name": docker_image,
                        "yarn.nodemanager.runtime.linux.docker.allowed-container-networks": "host,none,bridge",
                        "yarn.nodemanager.runtime.linux.docker.default-container-network": "host",
                        "yarn.nodemanager.runtime.linux.docker.host-pid-namespace.allowed": "false",
                        "yarn.nodemanager.runtime.linux.docker.privileged-containers.allowed": "false",
                        "yarn.nodemanager.runtime.linux.docker.delayed-removal.allowed": "false",
                        "yarn.nodemanager.runtime.linux.docker.stop.grace-period": "10",
                        "yarn.nodemanager.runtime.linux.docker.privileged-containers.acl": "",
                        "yarn.nodemanager.runtime.linux.docker.capabilities": "CHOWN,DAC_OVERRIDE,FSETID,FOWNER,MKNOD,NET_RAW,SETGID,SETUID,SETFCAP,SETPCAP,NET_BIND_SERVICE,SYS_CHROOT,KILL,AUDIT_WRITE",
                        "yarn.nodemanager.runtime.linux.docker.enable-userremapping.allowed": "true",
                        "yarn.nodemanager.runtime.linux.docker.userremapping-uid-threshold": "1",
                        "yarn.nodemanager.runtime.linux.docker.userremapping-gid-threshold": "1",

                        # Worker nodes init connection to the master node
                        "yarn.resourcemanager.nodemanager-connect-retries": "55",
                        "yarn.client.nodemanager-connect.retry-interval-ms": "3000",
                        "yarn.client.nodemanager-connect.max-wait-ms": "600000",
                        "yarn.resourcemanager.connect.max-wait.ms": "600000",
                        "yarn.resourcemanager.connect.retry-interval.ms": "10000",
                        #"yarn.resourcemanager.nodemanagers.heartbeat-interval-ms": "3000",

                        # Resource constraints
                        "yarn.scheduler.maximum-allocation-mb": "8192",

                        "yarn.nodemanager.env-whitelist": "JAVA_HOME,HADOOP_COMMON_HOME,HADOOP_HDFS_HOME,HADOOP_CONF_DIR,CLASSPATH_PREPEND_DISTCACHE,HADOOP_YARN_HOME,HADOOP_HOME,PATH,LANG,TZ",
                    }
                },
                {
                    'Classification': 'container-executor',
                    'Configurations': [{
                        'Classification': 'docker',
                        'Properties': {
                            "module.enabled": "true",
                            "docker.binary": "/usr/bin/docker",
                            # ECR
                            "docker.trusted.registries": f"local,centos,{ecr_endpoint}",
                            "docker.privileged-containers.registries": f"local,centos,{ecr_endpoint}",

                            "docker.allowed.capabilities": "CHOWN,DAC_OVERRIDE,FSETID,FOWNER,MKNOD,NET_RAW,SETGID,SETUID,SETFCAP,SETPCAP,NET_BIND_SERVICE,SYS_CHROOT,KILL,AUDIT_WRITE",
                            "docker.allowed.devices": "",
                            "docker.allowed.networks": "host",
                            "docker.inspect.max.retries": "5",
                            "docker.no-new-privileges.enabled": "false",
                            "docker.allowed.runtimes": "docker",
                            "docker.service-mode.enabled": "false",
                            "docker.allowed.rw-mounts": "/mnt,/mnt1",  #/hdfs /etc,/var/hadoop/yarn/local-dir,/var/hadoop/yarn/log-dir,
                            "docker.allowed.ro-mounts": "/sys/fs/cgroup,/etc/passwd,/etc/group,/usr",
                            "docker.allowed.volume-drivers": "local",
                            "docker.host-pid-namespace.enabled": "false",
                            "docker.privileged-containers.enabled": "false",
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