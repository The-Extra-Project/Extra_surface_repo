from constructs import Construct
from aws_cdk import (
    Stack,
    CfnOutput,
    aws_s3 as s3,
    aws_lambda as _lambda,
    aws_apigateway as apigateway
)


class ApiGateway(Stack):

    def __init__(self, scope: Construct, id: str, bucket: s3.Bucket, **kwargs) -> None:
        super().__init__(scope, id, **kwargs)

        self.bucket = bucket

        # Lambda function to generate pre-signed URL
        presigned_url_lambda = _lambda.Function(
            self, "PresignedUrlLambda",
            runtime=_lambda.Runtime.PYTHON_3_12,
            handler="index.handler",
            code=_lambda.Code.from_inline(
"""
import os
import json
import boto3

def handler(event, context):
    s3_client = boto3.client('s3')
    bucket_name = os.environ.get('BUCKET_NAME')

    filename = event['queryStringParameters'].get('filename', 'default.txt')
    params = {
        'Bucket': bucket_name,
        'Key': filename,
        'Expires': 3600  # timer for pre-signed URL
    }

    try:
        upload_url = s3_client.generate_presigned_url('put_object', Params=params)
        return {
            'statusCode': 200,
            'body': json.dumps({'uploadUrl': upload_url})
        }
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }
"""
            ),
            environment={
                'BUCKET_NAME': bucket.bucket_name
            }
        )

        # Grant S3 permissions to Lambda
        bucket.grant_put(presigned_url_lambda)

        # API Gateway setup
        api = apigateway.RestApi(self, "ApiGatewayDev", rest_api_name="Dev API GW")

        # Integrate backend lambda to API GW
        presigned_url_integration = apigateway.LambdaIntegration(presigned_url_lambda)

        api.root.add_method(
            "GET",
            presigned_url_integration,
            request_parameters={"method.request.querystring.filename": True})

        CfnOutput(
            self, "APIEndpoint",
            value=api.url,
            description="API Gateway endpoint"
        )
