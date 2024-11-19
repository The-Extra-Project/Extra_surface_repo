# Extralabs Infrastructure

## Prerequisites
- Python 3.10.11+
- Node v18+

### Install Python dependencies
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```
### Install AWS CDK
```bash
nvm use v20.11.0 #or another version
npm i -g aws-cdk-lib
```

### Configure AWS
Download and install aws-cli.
Run `aws configure --profile <profile_name>`, set your AWS_ACCESS_KEY and AWS_SECRET_ACCESS_KEY values.

```bash
export AWS_PROFILE=`profile_name`
```

#### Console Access
```
profile_id=test
https://${profile_id}.signin.aws.amazon.com/console
```

## Workflow

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
```


## Other CDK commands

 * `cdk ls`          list all stacks in the app
 * `cdk diff`        compare deployed stack with current state
 * `cdk docs`        open CDK documentation