from constructs import Construct
import aws_cdk as cdk
from aws_cdk import aws_iam as iam


class EMRRoles(Construct):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self.emr_role = iam.Role(self, "RoleEMRDefault",
            role_name="EMRRoleDefault",
            managed_policies=[iam.ManagedPolicy.from_aws_managed_policy_name("service-role/AmazonEMRServicePolicy_v2")],
            assumed_by=iam.ServicePrincipal("elasticmapreduce.amazonaws.com"),
            inline_policies={
                "EMRAssumeRolePolicy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=["sts:AssumeRole"],
                            # principals=[iam.ServicePrincipal("elasticmapreduce.amazonaws.com")],
                            conditions={
                                "StringEquals": {
                                    "aws:SourceAccount": cdk.Aws.ACCOUNT_ID
                                },
                                "ArnLike": {
                                    "aws:SourceArn": "arn:aws:elasticmapreduce:{region}:{account}:*".format(
                                        region=cdk.Aws.REGION, account=cdk.Aws.ACCOUNT_ID
                                    )
                                }
                            },
                            resources=["*"]
                        )
                    ]
                )
            }
        )

        self.emr_ec2_role = iam.Role(self, "RoleEMREC2",
            role_name="EMRRoleEC2",
            assumed_by=iam.ServicePrincipal("ec2.amazonaws.com"),
            managed_policies=[iam.ManagedPolicy.from_aws_managed_policy_name("service-role/AmazonEC2ContainerRegistryReadOnly")],
            inline_policies={
                "EMREC2Policy": iam.PolicyDocument(
                    statements=[
                        iam.PolicyStatement(
                            actions=[
                                "cloudwatch:*",
                                "dynamodb:*",
                                "ec2:Describe*",
                                "elasticmapreduce:Describe*",
                                "elasticmapreduce:ListBootstrapActions",
                                "elasticmapreduce:ListClusters",
                                "elasticmapreduce:ListInstanceGroups",
                                "elasticmapreduce:ListInstances",
                                "elasticmapreduce:ListSteps",
                                "kinesis:CreateStream",
                                "kinesis:DeleteStream",
                                "kinesis:DescribeStream",
                                "kinesis:GetRecords",
                                "kinesis:GetShardIterator",
                                "kinesis:MergeShards",
                                "kinesis:PutRecord",
                                "kinesis:SplitShard",
                                "rds:Describe*",
                                "s3:*",
                                "sdb:*",
                                "sns:*",
                                "sqs:*",
                            ],
                            resources=["*"]
                        )
                    ]
                )
            }
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
                                "arn:aws:s3:::{region}-emr-data-inputs".format(
                                    region=cdk.Aws.REGION
                                ),
                                "arn:aws:s3:::{region}-emr-data-inputs/*".format(
                                    region=cdk.Aws.REGION
                                ),
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
