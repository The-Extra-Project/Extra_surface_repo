import os
from constructs import Construct
from aws_cdk import (
    Aws,
    Stack,
    Duration,
    aws_iam as iam,
    aws_logs as logs,
    aws_lambda as lambda_,
    aws_events as events,
    aws_events_targets as targets
)


class LambdaStack(Stack):

    def __init__(self, scope: Construct, construct_id: str,
                 vpc: Construct, s3_bucket: Construct, ecr: Construct, roles: Construct,
                 input_prefix: str, docker_image: str,
                 **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self.invoke_role = iam.Role(self, "IAMRoleLambdaInvokeEMR",
            role_name="Lambda_Invoke_EMR",
            assumed_by=iam.ServicePrincipal("lambda.amazonaws.com"),
            inline_policies={
                "EMR": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=[
                                "ec2:*",
                                "application-autoscaling:*",
                                "resource-groups:ListGroupResources",
                                "cloudwatch:*",
                                "dynamodb:*",
                                "elasticmapreduce:*",
                                "s3:*",
                                "ssm:*",
                                "logs:CreateLogStream",
                                "logs:PutLogEvents"
                            ],
                            resources=["*"]
                        ),
                        iam.PolicyStatement(
                            actions=["iam:PassRole"],
                            resources=[
                                "arn:aws:iam::*:role/EMR_EC2_DefaultRole",
                                "arn:aws:iam::*:role/EMR_DefaultRole",
                                "arn:aws:iam::*:role/EMR_AutoScaling_DefaultRole"
                            ],
                        ),
                    ]
                )
            }
        )

        self.s3_bucket_name = f"s3://{s3_bucket.bucket_name}"

        self.log_group = logs.LogGroup(
                self, "LambdaLogGroup",
                log_group_name="/aws/lambda/emr_invoke",
                retention=logs.RetentionDays.ONE_MONTH
        )

        self.function_invoke_emr = lambda_.Function(self, "Lambda",
            function_name="EMR-Invoke",
            description="Invokes EMR cluster on launch. Triggered by S3 event on a file upload",
            runtime=lambda_.Runtime.PYTHON_3_12,
            code=lambda_.Code.from_asset(os.path.join(os.curdir, "stacks", "invoke", "handler")),
            handler="handler.lambda_handler",
            role=self.invoke_role,
            timeout=Duration.seconds(100),
            dead_letter_queue_enabled=True,
            environment={
                # Cluster
                "region": Aws.REGION,
                "release_label": 'emr-7.3.0',
                "hadoop_version": '3.3.6',
                "emr_service_role": roles.emr_default_role.role_name,
                "instance_profile": roles.emr_instance_profile.instance_profile_name,
                "subnet_id": vpc.public_subnets[0].subnet_id,
                "key_name": "extra",
                "vol_root_size": '30',
                # App
                "docker_image": docker_image,
                "ecr_endpoint": ecr.registry_emr.repository_uri,
                "s3_path": self.s3_bucket_name,
                "s3_path_input": f"{self.s3_bucket_name}/{input_prefix}",
                "s3_path_bootstrap": f"{self.s3_bucket_name}/scripts/docker.sh",
                "s3_path_iqlib": f"{self.s3_bucket_name}/jar/iqlib.jar",
                "s3_path_preprocess": f"{self.s3_bucket_name}/jar/preprocess-0.3.jar",
                "s3_path_process": f"{self.s3_bucket_name}/jar/process-0.3.jar",
                "log_uri": f"{self.s3_bucket_name}/emr-logs",
                "main_class": "WorkflowWasure",
            },
            log_group=self.log_group
        )

        # Define the EventBridge rule
        self.rule_s3_trigger = events.Rule(self, "S3UploadRule",
            event_pattern=events.EventPattern(
                source=["aws.s3"],
                detail_type=["Object Created"],
                detail={
                    "bucket": {
                        "name": [s3_bucket.bucket_name]
                    },
                    "object": {
                        "key": [
                            {
                                "prefix": input_prefix
                            },
                        ]
                    }
                }
            )
        )

        # Add the Lambda function as the target of the rule
        self.rule_s3_trigger.add_target(targets.LambdaFunction(self.function_invoke_emr))

        # Grant permissions for the Lambda function to be triggered by EventBridge
        self.function_invoke_emr.add_permission(
            "EventBridgePermissionS3Triggered",
            principal=iam.ServicePrincipal("events.amazonaws.com"),
            source_arn=self.rule_s3_trigger.rule_arn
        )

        # Grant the S3 bucket permission to invoke the rule
        s3_bucket.grant_read(self.function_invoke_emr)
