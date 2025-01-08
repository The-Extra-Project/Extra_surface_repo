import os
from constructs import Construct
from aws_cdk import (
    aws_lambda as lambda_,
    Stack,
    aws_iam as iam,
    Duration
)


class LambdaStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

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
                                "ssm:*"
                            ],
                            resources=["*"]
                        ),
                        iam.PolicyStatement(
                            actions=["iam:PassRole"],
                            resources=["arn:aws:iam::*:role/EMR_EC2_DefaultRole", "arn:aws:iam::767397971222:role/EMR_DefaultRole_V2"],
                            # conditions={
                            #     "StringLike": {
                            #         "iam:PassedToService": "ec2.amazonaws.com*"
                            #     }
                            # }
                        ),
                        iam.PolicyStatement(
                            actions=["iam:PassRole"],
                            resources=["arn:aws:iam::*:role/EMR_AutoScaling_DefaultRole"],
                            # conditions={
                            #     "StringLike": {
                            #         "iam:PassedToService": "application-autoscaling.amazonaws.com*"
                            #     }
                            # }
                        ),
                    ]
                )
            }
        )

        self.lambda_ = lambda_.Function(self, "Lambda",
            runtime=lambda_.Runtime.PYTHON_3_12,
            code=lambda_.Code.from_asset(os.path.join(os.curdir, "stacks", "invoke", "handler")),
            handler="handler.lambda_handler",
            role=self.invoke_role,
            timeout=Duration.seconds(100)
        )
