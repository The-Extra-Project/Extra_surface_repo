from constructs import Construct
import aws_cdk as cdk
from aws_cdk import (
    Stack,
    aws_ec2,
    aws_emr as emr
)
from .roles import EMRRoles


class EMRStack(Stack):
    def __init__(self, scope: Construct, construct_id: str, ecr: Stack, **kwargs) -> None:
        super().__init__(scope, construct_id, **kwargs)

        self.roles = EMRRoles(self, "EMRRoles")

        EMR_CLUSTER_NAME = cdk.CfnParameter(self, 'EMRClusterName',
            type='String',
            description='Extralabs EMR Cluster name',
            default='extralabs-dev'
        )

        vpc = aws_ec2.Vpc(self, "EMRStackVPC",
            max_azs=3,
            gateway_endpoints={
                "S3": aws_ec2.GatewayVpcEndpointOptions(service=aws_ec2.GatewayVpcEndpointAwsService.S3)
            }
        )

        emr_instances = emr.CfnCluster.JobFlowInstancesConfigProperty(
            master_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type="c5.2xlarge",
                market="ON_DEMAND"
            ),
            core_instance_group=emr.CfnCluster.InstanceGroupConfigProperty(
                instance_count=1,
                instance_type="c5.2xlarge",
                market="ON_DEMAND"
            ),
            task_instance_groups=[emr.CfnCluster.InstanceGroupConfigProperty(
                name="default",
                instance_count=1,
                instance_type="c5.4xlarge",
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
            ec2_subnet_id=vpc.public_subnets[0].subnet_id,
            keep_job_flow_alive_when_no_steps=True,
            termination_protected=False,
            hadoop_version="2.7.7",  # 3.3.6 default

            # ec2_key_name="ec2KeyName",

            # Network settings
            # ec2_subnet_ids=["ec2SubnetIds"],
            # emr_managed_master_security_group="emrManagedMasterSecurityGroup",
            # emr_managed_slave_security_group="emrManagedSlaveSecurityGroup",
            # service_access_security_group="serviceAccessSecurityGroup",
        )

        emr_version = self.node.try_get_context("emr_version") or "emr-7.3.0"
        emr_cfn_cluster = emr.CfnCluster(self, "EMRClusterDev",
            instances=emr_instances,
            job_flow_role=self.roles.emr_ec2_role.role_name,
            name=EMR_CLUSTER_NAME.value_as_string,
            service_role=self.roles.emr_role.role_name,
            # bootstrap_actions=[emr.CfnCluster.BootstrapActionConfigProperty(
            #     name="name",
            #     script_bootstrap_action=emr.CfnCluster.ScriptBootstrapActionConfigProperty(
            #         path="path",
            #         args=["args"]
            #     )
            # )],
            # auto_scaling_role="autoScalingRole",
            # auto_termination_policy=emr.CfnCluster.AutoTerminationPolicyProperty(
            #     idle_timeout=123
            # ),
            steps=[emr.CfnCluster.StepConfigProperty(
                hadoop_jar_step=emr.CfnCluster.HadoopJarStepConfigProperty(
                    jar="jar",
                    # optional
                    args=["args"],
                    main_class="mainClass",
                    step_properties=[emr.CfnCluster.KeyValueProperty(
                        key="key",
                        value="value"
                    )]
                ),
                name="name",
                action_on_failure="actionOnFailure"  # optional
            )],
            applications=[
                emr.CfnCluster.ApplicationProperty(name="Hadoop"),
                emr.CfnCluster.ApplicationProperty(name="Spark"),
            ],
            configurations=[
                emr.CfnCluster.ConfigurationProperty(
                    classification="delta-defaults",
                    configuration_properties={
                        "delta.enabled": "true"
                    }
                ),

                # Spark: https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-spark-docker.html
                # emr.CfnCluster.ConfigurationProperty(
                #     classification="container-executor",
                #     configurations=[emr.CfnCluster.ConfigurationProperty(
                #         classification="docker",
                #         configuration_properties={
                #             "docker.privileged-containers.registries": "local,centos,{ecr}".format(ecr=ecr.registry_emr.repository_name),
                #             "docker.trusted.registries": "local,centos,{ecr}".format(ecr=ecr.registry_emr.repository_name),
                #             # "yarn.nodemanager.runtime.linux.docker.ecr-auto-authentication.enabled": "true"

                #         }
                #     )],
                #     configuration_properties=[]
                # ),

                # Hadoop: https://docs.aws.amazon.com/emr/latest/ReleaseGuide/emr-hadoop-config.html
                emr.CfnCluster.ConfigurationProperty(
                    classification="mapred-site",
                    configuration_properties={
                        # reuse JVM for an infinite number of tasks within a single job
                        "mapred.job.jvm.num.tasks": "-1"
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="hadoop-env",
                    configuration_properties={
                        "hive.metastore.client.factory.class": "com.amazonaws.glue.catalog.metastore.AWSGlueDataCatalogHiveClientFactory"
                    }
                ),
                emr.CfnCluster.ConfigurationProperty(
                    classification="spark-hive-site",
                    configuration_properties={
                        "hive.metastore.client.factory.class": "com.amazonaws.glue.catalog.metastore.AWSGlueDataCatalogHiveClientFactory"
                    }
                )
            ],
            # custom_ami_id="customAmiId",
            ebs_root_volume_size=50,
            log_uri="s3://aws-logs-{account}-{region}/elasticmapreduce/".format(account=cdk.Aws.ACCOUNT_ID, region=cdk.Aws.REGION),
            release_label=emr_version,
            scale_down_behavior="TERMINATE_AT_TASK_COMPLETION",
            visible_to_all_users=True,
            tags=[cdk.CfnTag( 
                # required tag
                key="for-use-with-amazon-emr-managed-policies",
                value="True"
            )],
            # managed_scaling_policy=emr.CfnCluster.ManagedScalingPolicyProperty(
            #     compute_limits=emr.CfnCluster.ComputeLimitsProperty(
            #         maximum_capacity_units=123,
            #         minimum_capacity_units=123,
            #         unit_type="unitType",
            #         maximum_core_capacity_units=123,
            #         maximum_on_demand_capacity_units=123
            #     )
            # ),
            # os_release_label="osReleaseLabel",
            # release_label="releaseLabel",
            # scale_down_behavior="scaleDownBehavior",
            # security_configuration="securityConfiguration",
            # step_concurrency_level=123,
        )

        cdk.CfnOutput(self, 'OutputEMRClusterName', value=emr_cfn_cluster.name)
        cdk.CfnOutput(self, 'OutputEMRClusterMasterDNS', value=emr_cfn_cluster.attr_master_public_dns)
        cdk.CfnOutput(self, 'OutputEMRClusterLogURI', value=emr_cfn_cluster.log_uri)
        cdk.CfnOutput(self, 'OutputEMRVersion', value=emr_cfn_cluster.release_label)
        cdk.CfnOutput(self, 'OutputEMRHadoopVersion', value=emr_cfn_cluster.instances.hadoop_version)
