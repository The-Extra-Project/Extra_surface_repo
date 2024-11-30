from constructs import Construct
from aws_cdk import (
    Stack,
    aws_s3 as s3,
    RemovalPolicy
)


class S3Stack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self.bucket = s3.Bucket(self, "S3BucketEMRDev",
            bucket_name="extralabs-artifacts-dev",
            versioned=True,
            removal_policy=RemovalPolicy.DESTROY
        )
