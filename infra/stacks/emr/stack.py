from constructs import Construct
from aws_cdk import (
    Stack,
    aws_ec2 as ec2,
    aws_emr as emr,
    Fn,
    CfnParameter,
    CfnOutput,
    Aws,
    CfnTag,
    aws_iam as iam,
)

JAR_IQ = "s3://extralabs-artifacts-dev/jar/iqlib.jar"
DOCKER_IMAGE = "767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest"
BOOTSTRAP_SCRIPT_PATH = "s3://extralabs-artifacts-dev/scripts/docker.sh"
SSH_KEY_NAME="extra"

# SSH, monitroing - logs in s3 buckeet which is not autocreted
# steps fail


class EMRStack(Stack):
    def __init__(self, scope: Construct, construct_id: str, ecr: Stack, s3: Stack, **kwargs) -> None:
        super().__init__(scope, construct_id, **kwargs)

        emr_role = Fn.import_value("EMRDefaultRole")
        self.emr_role = iam.Role.from_role_arn(self, "EMRDefaultRoleImported", role_arn=emr_role)

        emr_ec2_role = Fn.import_value("EMREC2Role")
        self.emr_ec2_role = iam.Role.from_role_arn(self, "EMREC2RoleImported", role_arn=emr_ec2_role)

        emr_instance_profile = Fn.import_value("EMRInstanceProfile")
        self.emr_instance_profile = iam.Role.from_role_arn(self, "EMRInstanceProfileImported", role_arn=emr_instance_profile)

        vpc_id = Fn.import_value("EMRVpcId")
        self.vpc = ec2.Vpc.from_vpc_attributes(self, "EMRVPCImported",
            vpc_id=vpc_id,
            availability_zones=["eu-west-1a", "eu-west-1b", "eu-west-1c"],
        )

        cluster_name = CfnParameter(self, 'EMRClusterName',
            type='String',
            description='Extralabs EMR Cluster name',
            default='extralabs-dev'
        )

        emr_instances = emr.CfnCluster.JobFlowInstancesConfigProperty(
            master_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type="c5.xlarge",
                market="ON_DEMAND"
            ),
            core_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type="c5.xlarge",
                market="ON_DEMAND"
            ),
            task_instance_groups=[emr.CfnCluster.InstanceGroupConfigProperty(
                name="default",
                instance_count=1,
                instance_type="c5.xlarge",
                market="ON_DEMAND",
                # custom_ami_id="customAmiId",
                # configurations=[emr.CfnCluster.ConfigurationProperty(
                #     classification="classification",
                #     configuration_properties={
                #         "configuration_properties_key": "configurationProperties"
                #     },
                #     configurations=[configuration_property_]
                # )],
                ebs_configuration=emr.CfnCluster.EbsConfigurationProperty(
                    ebs_block_device_configs=[emr.CfnCluster.EbsBlockDeviceConfigProperty(
                        volume_specification=emr.CfnCluster.VolumeSpecificationProperty(
                            size_in_gb=50,
                            volume_type="gp3",
                            # iops=123
                        ),
                        # volumes_per_instance=123
                    )],
                    ebs_optimized=True
                ),
            )],
            # ec2_subnet_id=Fn.import_value("PublicSubnet0"),
            keep_job_flow_alive_when_no_steps=True,
            termination_protected=False,
            hadoop_version="3.3.6",
            ec2_key_name=SSH_KEY_NAME,
            # Network settings
            ec2_subnet_ids=[Fn.import_value("PublicSubnet0")],
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
                        path=f"s3://{s3.bucket.bucket_name}/scripts/docker.sh"
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
                            'aws s3 sync s3://extralabs-artifacts-dev/data/ /home/hadoop/data',
                        ],
                    ),
                    action_on_failure="CANCEL_AND_WAIT"  # optional
                ),
                emr.CfnCluster.StepConfigProperty(
                    name="2 - Preprocess",
                    hadoop_jar_step=emr.CfnCluster.HadoopJarStepConfigProperty(
                        jar="command-runner.jar",
                        # optional
                        # /app/build/target/scala-2.13/iqlib-spark_2.13-1.0.jar
                        args=[
                            'spark-submit', '--deploy-mode', 'cluster', '--master', 'yarn',
                            '--jars', 's3://extralabs-artifacts-dev/jar/iqlib.jar',
                            '--conf', 'spark.executorEnv.YARN_CONTAINER_RUNTIME_TYPE=docker',
                            '--conf', f'spark.executorEnv.YARN_CONTAINER_RUNTIME_DOCKER_IMAGE={DOCKER_IMAGE}',
                            's3://extralabs-artifacts-dev/scripts/workflow_preprocess.scala'
                        ],
                        main_class="mainClass",
                        # step_properties=[emr.CfnCluster.KeyValueProperty(
                        #     key="key",
                        #     value="value"
                        # )]
                    ),
                    action_on_failure="CANCEL_AND_WAIT"  # optional
                )
            ],
            applications=[
                emr.CfnCluster.ApplicationProperty(name="Hadoop"),
                emr.CfnCluster.ApplicationProperty(name="Spark"),
            ],
            configurations=[
                ################### GENERAL ###################
                emr.CfnCluster.ConfigurationProperty(
                    classification="delta-defaults",
                    configuration_properties={
                        "delta.enabled": "true"
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="yarn-site",
                    configuration_properties={
                        "yarn.container.runtime.type": "docker",
                        "yarn.container.docker.image": DOCKER_IMAGE,
                        "yarn.nodemanager.runtime.linux.docker.default-rw-mounts": "/home/hadoop/data:/app/datas",
                        # "yarn.nodemanager.env-whitelist": ""
                        # docker.allowed.ro-mounts
                        # docker.allowed.rw-mounts
                    }
                ),
                # access to private repos granted by default
                # emr.CfnCluster.ConfigurationProperty(
                #     classification="container-executor",
                #     configurations=[emr.CfnCluster.ConfigurationProperty(
                #         classification="docker",
                #         configuration_properties={
                #             "docker.trusted.registries": "local,centos,{ecr}".format(ecr=ecr.registry_emr.repository_uri),
                #             "docker.privileged-containers.registries": "local,centos,{ecr}".format(
                #                 ecr=ecr.registry_emr.repository_uri
                #             )
                #         }
                #     )],
                # ),

                ################### SPARK ################### 
                # https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-spark-docker.html
                # Override JVM
                emr.CfnCluster.ConfigurationProperty(
                    classification="spark-defaults",
                    configuration_properties={
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
                        # "spark.executorEnv.DDT_MAIN_DIR": "/app/",
                        # "spark.executorEnv.INPUT_DATA_DIR": "/app/datas/",
                        # "spark.executorEnv.OUTPUT_DATA_DIR": "/app/out/",
                        # "spark.executorEnv.GLOBAL_BUILD_DIR": "/app/build/",
                        # "spark.executorEnv.PARAM_PATH": "${INPUT_DATA_DIR}/params.xml",
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="spark-env",
                    configurations=[emr.CfnCluster.ConfigurationProperty(
                        classification="export",
                        configuration_properties={
                            "JAVA_HOME": "/usr/lib/jvm/java-1.8.0",
                            "DDT_MAIN_DIR": "/app/",
                            "INPUT_DATA_DIR": "/app/datas/",
                            "OUTPUT_DATA_DIR": "/app/out/",
                            "GLOBAL_BUILD_DIR": "/app/build/",
                            "PARAM_PATH": "${INPUT_DATA_DIR}/params.xml",
                        }
                    )],
                ),

                ################### HADOOP ################### 
                # https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-hadoop-config.html
                emr.CfnCluster.ConfigurationProperty(
                    classification="mapred-site",
                    configuration_properties={
                        # reuse JVM for an infinite number of tasks within a single job
                        "mapred.job.jvm.num.tasks": "-1"
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="hadoop-env",
                    configurations=[
                        # Override JVM
                        emr.CfnCluster.ConfigurationProperty(
                            classification="export",
                            configuration_properties={
                                "JAVA_HOME": "/usr/lib/jvm/java-1.8.0"
                            },
                        )
                    ]
                ),
            ],
            # custom_ami_id="customAmiId",
            ebs_root_volume_size=50,
            log_uri=f"s3://{s3.bucket.bucket_name}/emr-logs",
            release_label=self.node.try_get_context("emr_version") or "emr-7.3.0",
            scale_down_behavior="TERMINATE_AT_TASK_COMPLETION",
            visible_to_all_users=True,
            tags=[# Required
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
