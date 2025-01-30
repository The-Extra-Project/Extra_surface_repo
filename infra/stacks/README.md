# Main Infrastructure Directory

### Description

1. EMR
EMR cluster definition.

2. Lambda
Lambda function responsible for the cluster invocation.
S3 event EMR cluster triggering via EventBridge.

3. ECR
Elastic Container Registry - for docker image storage

4. Network
VPC definition

5. S3
Buckets

6. API GW
Currently only function is generating pre-signed URL for uploading the files to the assigned bucket.
API GW proxies the requests:
1. GET to generate the link.
2. PUT via the generated link to upload the file.