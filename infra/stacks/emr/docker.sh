#!/bin/bash

sudo yum update -y
sudo yum install -y docker
sudo usermod -a -G docker hadoop
sudo systemctl start docker

# That's a hacky legacy auth used in EMR pre 6.x! The modern auto ECR auth by the docs wasn't not working at all ;(
# https://docs.aws.amazon.com/emr/latest/ManagementGuide/emr-plan-docker.html#emr-docker-ECR
# Refactor
aws ecr get-login-password --region eu-west-1 | sudo docker login --username AWS --password-stdin 767397971222.dkr.ecr.eu-west-1.amazonaws.com
mkdir -p ~/.docker && sudo cp /root/.docker/config.json ~/.docker/config.json && sudo chmod 644 ~/.docker/config.json
hadoop fs -put ~/.docker/config.json /user/hadoop/
sudo docker pull 767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest
