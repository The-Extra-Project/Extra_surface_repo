#!/bin/bash

sudo yum update -y
sudo yum install -y docker
sudo usermod -a -G docker hadoop
sudo systemctl start docker

#sudo amazon-linux-extras install docker -y
#sudo service docker start