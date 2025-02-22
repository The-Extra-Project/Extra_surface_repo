from constructs import Construct
from aws_cdk import (
    Stack,
    aws_emr as emr,
    Fn,
    CfnParameter,
    CfnOutput,
    CfnTag,
    aws_iam as iam,
)


ECR_ENDPOINT = "767397971222.dkr.ecr.eu-west-1.amazonaws.com"
REGISTRY = ECR_ENDPOINT + "/extralabs-emr-dev"
DOCKER_IMAGE = REGISTRY + ":latest"

SSH_KEY_NAME = "extra"

S3_PATH_BOOTSTRAP = "s3://extralabs-artifacts-dev/scripts/docker.sh"

S3_PATH_INPUT = "s3a://extralabs-artifacts-dev/data/"
S3_PATH_IQLIB = "s3://extralabs-artifacts-dev/jar/iqlib.jar"
S3_PATH_PREPROCESS = "s3://extralabs-artifacts-dev/jar/preprocess-0.4.jar"
# SSH, monitoring - logs in s3 bucket which is not autocreted

EMR_NODE_TYPE_MASTER = "m5.xlarge"
EMR_NODE_TYPE_CORE = "m5.xlarge"
EMR_NODE_TYPE_TASK = "c5.xlarge"

SPARK_HEAP_SIZE = "1g"
SPARK_MEM_FRACTION = "0.6"


class EMRStack(Stack):
    def __init__(self, scope: Construct, construct_id: str, vpc: Construct, ecr: Stack, s3: Stack, **kwargs) -> None:
        super().__init__(scope, construct_id, **kwargs)

        emr_role = Fn.import_value("EMRDefaultRole")
        self.emr_role = iam.Role.from_role_arn(self, "EMRDefaultRoleImported", role_arn=emr_role)

        emr_ec2_role = Fn.import_value("EMREC2Role")
        self.emr_ec2_role = iam.Role.from_role_arn(self, "EMREC2RoleImported", role_arn=emr_ec2_role)

        emr_instance_profile = Fn.import_value("EMRInstanceProfile")
        self.emr_instance_profile = iam.Role.from_role_arn(self, "EMRInstanceProfileImported", role_arn=emr_instance_profile)

        self.vpc = vpc

        cluster_name = CfnParameter(self, 'EMRClusterName',
            type='String',
            description='Extralabs EMR Cluster name',
            default='extralabs-dev'
        )

        emr_instances = emr.CfnCluster.JobFlowInstancesConfigProperty(
            master_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type=EMR_NODE_TYPE_MASTER,
                market="ON_DEMAND"
            ),
            core_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type=EMR_NODE_TYPE_CORE,
                market="ON_DEMAND"
            ),
            # For cost savings, uncomment for the prod
            # task_instance_groups=[emr.CfnCluster.InstanceGroupConfigProperty(
            #     name="default",
            #     instance_count=1,
            #     instance_type=EMR_NODE_TYPE_TASK,
            #     market="ON_DEMAND",
            #     # custom_ami_id="customAmiId",
            #     # configurations=[emr.CfnCluster.ConfigurationProperty(
            #     #     classification="classification",
            #     #     configuration_properties={
            #     #         "configuration_properties_key": "configurationProperties"
            #     #     },
            #     #     configurations=[configuration_property_]
            #     # )],
            #     ebs_configuration=emr.CfnCluster.EbsConfigurationProperty(
            #         ebs_block_device_configs=[emr.CfnCluster.EbsBlockDeviceConfigProperty(
            #             volume_specification=emr.CfnCluster.VolumeSpecificationProperty(
            #                 size_in_gb=50,
            #                 volume_type="gp3",
            #                 # iops=123
            #             ),
            #             # volumes_per_instance=123
            #         )],
            #         ebs_optimized=True
            #     ),
            # )],
            keep_job_flow_alive_when_no_steps=True,
            termination_protected=False,
            hadoop_version="3.3.6",
            ec2_key_name=SSH_KEY_NAME,
            # Network settings
            ec2_subnet_id=Fn.import_value("PublicSubnet0"),
            # ec2_subnet_ids=[],
            # emr_managed_master_security_group="emrManagedMasterSecurityGroup",
            # emr_managed_slave_security_group="emrManagedSlaveSecurityGroup",
            # service_access_security_group="serviceAccessSecurityGroup",
        )

        emr_cfn_cluster = emr.CfnCluster(self, "EMRClusterDev",
            name=cluster_name.value_as_string,
            service_role=self.emr_role.role_arn,
            job_flow_role=self.emr_instance_profile.role_arn,
            instances=emr_instances,
            bootstrap_actions=[
                emr.CfnCluster.BootstrapActionConfigProperty(
                    name="docker",
                    script_bootstrap_action=emr.CfnCluster.ScriptBootstrapActionConfigProperty(
                        path=S3_PATH_BOOTSTRAP
                        # args=["args"]
                    )
                ),
            ],
            steps=[
                emr.CfnCluster.StepConfigProperty(
                    name="1 - Copy data from S3 to HDFS",
                    hadoop_jar_step=emr.CfnCluster.HadoopJarStepConfigProperty(
                        jar="command-runner.jar",
                        args=[
                            "bash", "-c",
                            f"hdfs dfs -mkdir -p /user/hadoop/{{input,output}} && hdfs dfs -cp {S3_PATH_INPUT}/* /user/hadoop/input"
                        ],
                    ),
                    action_on_failure="CANCEL_AND_WAIT"  # optional
                ),
                emr.CfnCluster.StepConfigProperty(
                    name="2 - Preprocess",
                    hadoop_jar_step=emr.CfnCluster.HadoopJarStepConfigProperty(
                        jar="command-runner.jar",
                        args=[
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--class', 'WorkflowWasure', '--jars', S3_PATH_IQLIB, S3_PATH_PREPROCESS,

                            'DDT_MAIN_DIR=/app/',
                            'GLOBAL_BUILD_DIR=/app/build/',
                            # hdfs path
                            'INPUT_DATA_DIR=/user/hadoop/input',
                            'OUTPUT_DATA_DIR=/user/hadoop/input',
                            'PARAM_PATH=/user/hadoop/input/params.xml',

                        ],
                        # Pass params as env vars
                        # step_properties=[
                        #     emr.CfnCluster.KeyValueProperty(
                        #         key="DDT_MAIN_DIR",
                        #         value="/app/",
                        #     ),
                        #     emr.CfnCluster.KeyValueProperty(
                        #         key="GLOBAL_BUILD_DIR",
                        #         value="/app/build/",
                        #     ),
                        #     # HDFS paths
                        #     emr.CfnCluster.KeyValueProperty(
                        #         key="INPUT_DATA_DIR",
                        #         value="/user/hadoop/input",
                        #     ),
                        #     emr.CfnCluster.KeyValueProperty(
                        #         key="PARAM_PATH",
                        #         value="/user/hadoop/input/params.xml",
                        #     ),
                        #     emr.CfnCluster.KeyValueProperty(
                        #         key="OUTPUT_DATA_DIR",
                        #         value="/user/hadoop/input",
                        #     ),
                        # ]
                    ),
                    action_on_failure="CANCEL_AND_WAIT"  # optional
                )
            ],
            applications=[
                emr.CfnCluster.ApplicationProperty(name="Hadoop"),
                emr.CfnCluster.ApplicationProperty(name="Spark"),
            ],
            configurations=[
                # GENERAL
                emr.CfnCluster.ConfigurationProperty(
                    classification="delta-defaults",
                    configuration_properties={
                        "delta.enabled": "true"
                    }
                ),

                # SPARK
                # https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-spark-docker.html
                emr.CfnCluster.ConfigurationProperty(
                    classification="spark-defaults",
                    configuration_properties={
                        # Override JVM
                        "spark.executorEnv.JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        "spark.yarn.appMasterEnv.JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        # Docker settings
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": DOCKER_IMAGE,
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_TYPE": "docker",
                        "spark.yarn.appMasterEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE": DOCKER_IMAGE,
                        # From Waseur repo
                        "spark.rdd.compress": "true",
                        "spark.eventLog.enabled": "true",
                        "spark.memory.offHeap.enabled": "true",
                        "spark.driver.allowMultipleContexts": "true",
                        "spark.memory.offHeap.size": SPARK_HEAP_SIZE,
                        "spark.memory.storageFraction": SPARK_MEM_FRACTION,
                        "spark.serializer": "org.apache.spark.serializer.KryoSerializer",
                        # "spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_MOUNTS": "/home/hadoop/mnt:/app/mnt:rw",
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="spark-env",
                    configurations=[emr.CfnCluster.ConfigurationProperty(
                        classification="export",
                        configuration_properties={
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto",
                        }
                    )],
                ),

                # HADOOP
                emr.CfnCluster.ConfigurationProperty(
                    classification="hadoop-env",
                    configurations=[
                        # Override JVM
                        emr.CfnCluster.ConfigurationProperty(
                            classification="export",
                            configuration_properties={
                                "JAVA_HOME": "/usr/lib/jvm/java-1.8.0-amazon-corretto"
                            },
                        )
                    ]
                ),
                # https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-hadoop-config.html
                emr.CfnCluster.ConfigurationProperty(
                    classification="mapred-site",
                    configuration_properties={
                        # reuse JVM for an infinite number of tasks within a single job
                        "mapred.job.jvm.num.tasks": "-1"
                    }
                ),

                # YARN
                emr.CfnCluster.ConfigurationProperty(
                    classification="yarn-site",
                    configuration_properties={
                        # Container runtime configuration
                        # https://hadoop.apache.org/docs/r3.3.6/hadoop-yarn/hadoop-yarn-site/DockerContainers.html
                        # Node Labels
                        "yarn.node-labels.enabled": "true",
                        "yarn.node-labels.am.default-node-label-expression": "ON_DEMAND",
                        "yarn.node-labels.configuration-type": "distributed",

                        # https://repost.aws/questions/QUPohFO5BKTrqZxhybxAG8IA/emr-ecr-auto-login-failing-starting-with-emr-6-9-0
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
                        "yarn.nodemanager.runtime.linux.docker.image-name": DOCKER_IMAGE,
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
                        # Mounts (optional)
                        #"yarn.nodemanager.runtime.linux.docker.default-rw-mounts": "/home/hadoop/mnt:/app/mnt",
                        #"yarn.nodemanager.local-dirs": "/mnt/yarn-local",
                        #"yarn.nodemanager.log-dirs": "/mnt/yarn-logs",
                        #"yarn.nodemanager.runtime.linux.docker.default-ro-mounts": "",
                    }
                ),
                # access to private repos granted by default
                emr.CfnCluster.ConfigurationProperty(
                    classification="container-executor",
                    configurations=[emr.CfnCluster.ConfigurationProperty(
                        classification="docker",
                        configuration_properties={
                            "module.enabled": "true",
                            "docker.binary": "/usr/bin/docker",
                            # ECR
                            "docker.trusted.registries": f"local,centos,{ECR_ENDPOINT},{REGISTRY}",
                            "docker.privileged-containers.registries": f"local,centos,{ECR_ENDPOINT},{REGISTRY}",

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
                    )],
                ),

            ],
            # custom_ami_id="customAmiId",
            ebs_root_volume_size=50,
            log_uri=f"s3://{s3.bucket.bucket_name}/emr-logs",
            release_label=self.node.try_get_context("emr_version") or "emr-7.3.0",
            scale_down_behavior="TERMINATE_AT_TASK_COMPLETION",
            visible_to_all_users=True,
            tags=[  # Required
                CfnTag(
                    key="for-use-with-amazon-emr-managed-policies",
                    value="True"
                )
            ],
            # scale_down_behavior="scaleDownBehavior",
            # # managed_scaling_policy=emr.CfnCluster.ManagedScalingPolicyProperty(
            #     compute_limits=emr.CfnCluster.ComputeLimitsProperty(
            #         maximum_capacity_units=123,
            #         minimum_capacity_units=123,
            #         unit_type="unitType",
            #         maximum_core_capacity_units=123,
            #         maximum_on_demand_capacity_units=123
            #     )
            # ),
            # auto_scaling_role="autoScalingRole",
            # auto_termination_policy=emr.CfnCluster.AutoTerminationPolicyProperty(
            #     idle_timeout=123
            # ),
            # security_configuration="securityConfiguration",
            # step_concurrency_level=123,
        )

        CfnOutput(self, 'OutputEMRClusterName', value=emr_cfn_cluster.name)
        CfnOutput(self, 'OutputEMRClusterMasterDNS', value=emr_cfn_cluster.attr_master_public_dns)
        CfnOutput(self, 'OutputEMRClusterLogURI', value=emr_cfn_cluster.log_uri)
        CfnOutput(self, 'OutputEMRVersion', value=emr_cfn_cluster.release_label)
        CfnOutput(self, 'OutputEMRHadoopVersion', value=emr_cfn_cluster.instances.hadoop_version)
