import os
# from dotenv import load_dotenv
from aws_cdk import Environment, App
from stacks.ecr import ECRStack
from stacks.s3 import S3Stack
from stacks.network import NetworkStack
from stacks.emr.roles import EMRRolesStack
from stacks.emr.stack import EMRStack
from stacks.invoke.lambda_stack import LambdaStack

# Refactor: add .env var loads 
# load_dotenv()
S3_PATH_INPUT = "data/"
DOCKER_IMAGE = "767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest"


app = App()

net = NetworkStack(app, "NetworkStack")
ecr = ECRStack(app, "ECRStack")
s3 = S3Stack(app, "S3Stack")
roles = EMRRolesStack(app, "EMRRolesStack")

lambda_invoke = LambdaStack(app, "LambdaStack", #env,
                            net.vpc, s3.bucket, roles,
                            S3_PATH_INPUT, DOCKER_IMAGE)

# EMRStack(app, "EMRStack",
#     ecr, s3, env=Environment(
#         account=os.getenv('CDK_DEFAULT_ACCOUNT'),
#         region=os.getenv('CDK_DEFAULT_REGION')
#     )
# )

app.synth()
