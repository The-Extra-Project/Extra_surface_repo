#!/bin/bash

sudo yum update -y

sudo amazon-linux-extras install docker -y

sudo usermod -a -G docker hadoop

# sudo systemctl start docker
sudo service docker start

sudo docker pull 767397971222.dkr.ecr.eu-west-1.amazonaws.com/extralabs-emr-dev:latest

docker images