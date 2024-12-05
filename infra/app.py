import os
from aws_cdk import Environment, App
from stacks.ecr import ECRStack
from stacks.s3 import S3Stack
from stacks.network import NetworkStack
from stacks.emr.roles import EMRRolesStack
from stacks.emr.stack import EMRStack
#from stacks.infra_stack import InfraStack


app = App()

#InfraStack(app, "InfraStack",
    # If you don't specify 'env', this stack will be environment-agnostic.
    # Account/Region-dependent features and context lookups will not work,
    # but a single synthesized template can be deployed anywhere.

    # Uncomment the next line to specialize this stack for the AWS Account
    # and Region that are implied by the current CLI configuration.

    #env=Environment(account=os.getenv('CDK_DEFAULT_ACCOUNT'), region=os.getenv('CDK_DEFAULT_REGION')),

    # Uncomment the next line if you know exactly what Account and Region you
    # want to deploy the stack to. */

    #env=Environment(account='123456789012', region='us-east-1'),

    # For more information, see https://docs.aws.amazon.com/cdk/latest/guide/environments.html
#)

net = NetworkStack(app, "NetworkStack")
ecr = ECRStack(app, "ECRStack")
s3 = S3Stack(app, "S3Stack")
roles = EMRRolesStack(app, "EMRRolesStack")

EMRStack(app, "EMRStack",
    ecr,
    s3,
    env=Environment(
    account=os.getenv('CDK_DEFAULT_ACCOUNT'),
    region=os.getenv('CDK_DEFAULT_REGION'))
)


app.synth()
