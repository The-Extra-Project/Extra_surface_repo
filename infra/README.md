# Extralabs Infrastructure

## Workflow

![Flow](./flow.svg)

## Prerequisites
- Python 3.10.11+
- Node v18+

#### Install Python dependencies
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```
#### Install AWS CDK
```bash
nvm use v20.11.0 #or another version
npm i -g aws-cdk-lib
```

#### Configure AWS
Download and install aws-cli.
Run `aws configure --profile <profile_name>`, set your AWS_ACCESS_KEY and AWS_SECRET_ACCESS_KEY values.

```bash
export AWS_PROFILE=`profile_name`
```

#### AWS Console Access
```
profile_id=test
https://${profile_id}.signin.aws.amazon.com/console
```


## Project structure
- [app.py](./app.py) - highest level aggregation file with stacks instantiation.
- [stacks/](./stacks/) - folder with the definition of stacks (logical collections of AWS resources).
    - [stacks/emr](./stacks/emr/) - EMR stack for development purposes. You can test your EMR cluster changes here and run `cdk deploy EMRStack` to get the real cluster. You can then use AWS console / SSH into primary/core instances to troubleshoot the Spark app, see logs, try different configurations, etc.
    - [stacks/invoke](./stacks/invoke/) - stack with the invocation Lambda function. It's what actually used for the production workflow to invoke the EMR cluster in response to an http request coming from the API GW.
        - [stacks/invoke/handler/handler.py](./stacks/invoke/handler/handler.py) - the body of the lambda function, its main logic with parsing the input parameters, EMR cluster creation.
        - [stacks/invoke/lambda_stack.py](./stacks/invoke/lambda_stack.py) - the CDK definition of the lambda function. Contains resource constraints, environment variables passed to the function runtime.
    - [stacks/api_gw.py](./stacks/api_gw.py) - API GW definition, serves as the REST bridge between the frontend and cloud backend (EMR). You can define various HTTP endpoints here, e.g. `POST $domain/invoke` => incoming request data passed to the invoke lambda function, which parses input parameters (such as S3 path with the input data) and instantiates an EMR cluster with that params. Current setup: generating pre-signed URL to upload the files to an assigned bucket:
        1. `GET` - generate the link.
        2. `PUT` - upload the file using the generated previously link.
    - [stacks/ecr.py](./stacks/ecr.py) - elastic container registry definition, serves as a storage for docker images alike Docker Hub. The registry is private, accessible only to AWS account resources by default.
    - [stacks/network.py](./stacks/network.py) - VPC definition, network rules, security groups and other network related components to be defined here.
    - [stacks/s3.py](./stacks/s3.py) - S3 buckets.



## CDK Commands

### I. Configure AWS Account
```bash
cdk bootstrap  # only run on the first cdk usage
cdk doctor  # check the env for validity
```

### II. Run cdk commands
```bash
cdk synth  # generate CloudFormation templates == validate code
cdk deploy  # rollout the infrastructure 
cdk destroy  # use with caution only in case of need
cdk ls  # list all stacks in the app
cdk diff  # compare deployed stack with current state
cdk docs  # open CDK documentation
```