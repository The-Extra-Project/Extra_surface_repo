from constructs import Construct
from aws_cdk import (
    aws_ecr as ecr,
    Stack
)


class ECRStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self.registry_emr = ecr.Repository(self, "ECRRepositoryEMR",
            repository_name="extralabs-emr-dev",
            empty_on_delete=False,
            image_scan_on_push=True,
            image_tag_mutability=ecr.TagMutability.MUTABLE,
        )
