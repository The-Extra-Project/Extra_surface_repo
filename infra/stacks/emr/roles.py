from constructs import Construct
from aws_cdk import (
    Aws,
    Stack,
    CfnOutput,
    aws_iam as iam,
)


class EMRRolesStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        # https://docs.aws.amazon.com/emr/latest/ManagementGuide/emr-iam-role.html
        self.emr_default_role = iam.Role(self, "EMRDefaultRole",
            role_name="EMR_DefaultRole",
            assumed_by=iam.ServicePrincipal("elasticmapreduce.amazonaws.com"),
            # managed AmazonEMRServicePolicy_v2 not working, falling back to the inline
            inline_policies={
                "EMRPolicy": iam.PolicyDocument(
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
                                "ssm:*"
                            ],
                            resources=["*"]
                        ),
                        iam.PolicyStatement(
                            actions=["iam:PassRole"],
                            resources=["arn:aws:iam::*:role/EMR_AutoScaling_DefaultRole"],
                            conditions={
                                "StringLike": {
                                    "iam:PassedToService": "application-autoscaling.amazonaws.com*"
                                }
                            }
                        ),
                        iam.PolicyStatement(
                            actions=["iam:PassRole"],
                            resources=["arn:aws:iam::*:role/EMR_EC2_DefaultRole"],
                            conditions={
                                "StringLike": {
                                    "iam:PassedToService": "ec2.amazonaws.com*"
                                }
                            }
                        )
                    ]
                )
            }
        )

        # https://docs.aws.amazon.com/emr/latest/ManagementGuide/emr-iam-role-for-ec2.html
        self.emr_ec2_role = iam.Role(self, "RoleEMREC2",
            role_name="EMR_EC2_DefaultRole",
            assumed_by=iam.ServicePrincipal("ec2.amazonaws.com"),
            managed_policies=[iam.ManagedPolicy.from_aws_managed_policy_name("AmazonSSMManagedInstanceCore")],
            inline_policies={
                "EMREC2Policy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=[
                                "cloudwatch:*",
                                "dynamodb:*",
                                "ec2:*",
                                "elasticmapreduce:*",
                                "kinesis:*",
                                "rds:Describe*",
                                "s3:*",
                                "sdb:*",
                                "sns:*",
                                "sqs:*",
                                "glue:*",
                            ],
                            resources=["*"]
                        )
                    ]
                ),
                "ECRPolicy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=[
                                "ecr:GetAuthorizationToken",
                                "ecr:BatchCheckLayerAvailability",
                                "ecr:GetDownloadUrlForLayer",
                                "ecr:GetRepositoryPolicy",
                                "ecr:DescribeRepositories",
                                "ecr:ListImages",
                                "ecr:DescribeImages",
                                "ecr:BatchGetImage",
                                "ecr:GetLifecyclePolicy",
                                "ecr:GetLifecyclePolicyPreview",
                                "ecr:ListTagsForResource",
                                "ecr:DescribeImageScanFindings"
                            ],
                            resources=["*"]
                        )
                    ]
                )
            }
        )

        self.emr_instance_profile = iam.CfnInstanceProfile(
            self, "EMREC2InstanceProfile",
            roles=[self.emr_ec2_role.role_name],
            instance_profile_name="EMR_Instance_Profile"
        )

        self.emrfs_role = iam.Role(self, "RoleEMRFS",
            role_name="EMRRoleEMRFS",
            assumed_by=iam.ServicePrincipal("ec2.amazonaws.com"),
            inline_policies={
                "EMRFSS3Policy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=[
                                "s3:AbortMultipartUpload",
                                "s3:CreateBucket",
                                "s3:DeleteObject",
                                "s3:GetBucketVersioning",
                                "s3:GetObject",
                                "s3:GetObjectTagging",
                                "s3:GetObjectVersion",
                                "s3:ListBucket",
                                "s3:ListBucketMultipartUploads",
                                "s3:ListBucketVersions",
                                "s3:ListMultipartUploadParts",
                                "s3:PutBucketVersioning",
                                "s3:PutObject",
                                "s3:PutObjectTagging",
                            ],
                            resources=[
                                f"arn:aws:s3:::{Aws.REGION}-emr-data-inputs",
                                f"arn:aws:s3:::{Aws.REGION}-emr-data-inputs/*"
                            ],
                        ),
                    ]
                ),
                "EMRFSDynamodbPolicy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            effect=iam.Effect.ALLOW,
                            actions=[
                                "dynamodb:CreateTable",
                                "dynamodb:BatchGetItem",
                                "dynamodb:BatchWriteItem",
                                "dynamodb:PutItem",
                                "dynamodb:DescribeTable",
                                "dynamodb:DeleteItem",
                                "dynamodb:GetItem",
                                "dynamodb:Scan",
                                "dynamodb:Query",
                                "dynamodb:UpdateItem",
                                "dynamodb:DeleteTable",
                                "dynamodb:UpdateTable",
                            ],
                            resources=[
                                "*",  # refactor
                            ],
                        )
                    ]
                ),
                "EMRFSLogPolicy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            effect=iam.Effect.ALLOW,
                            actions=[
                                "cloudwatch:PutMetricData",
                                "dynamodb:ListTables",
                                "s3:ListBucket",
                            ],
                            resources=["*"],
                        ),
                    ]
                ),
                "EMRFSSqsPolicy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            effect=iam.Effect.ALLOW,
                            actions=[
                                "sqs:GetQueueUrl",
                                "sqs:ReceiveMessage",
                                "sqs:DeleteQueue",
                                "sqs:SendMessage",
                                "sqs:CreateQueue",
                            ],
                            resources=[
                                "*",  # refactor
                            ],
                        )
                    ]
                )
            }
        )

        CfnOutput(
            self, "OutputEMREC2Role",
            value=self.emr_ec2_role.role_arn,
            export_name="EMREC2Role"
        )
        CfnOutput(
            self, "OutputEMRInstanceProfile",
            value=self.emr_instance_profile.ref,
            export_name="EMRInstanceProfile"
        )
        CfnOutput(
            self, "OutputEMRRole",
            value=self.emr_default_role.role_arn,
            export_name="EMRDefaultRole"
        )