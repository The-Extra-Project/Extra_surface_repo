from constructs import Construct
from aws_cdk import (
    aws_lambda as _lambda,
    Stack

)


class LambdaStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self._lambda = _lambda.Function(self, "Lambda",
            runtime=_lambda.Runtime.PYTHON_3_10,
            code=_lambda.Code.from_asset("invoke.py"),
            handler="lambda_handler.handler"
        )
