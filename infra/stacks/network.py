from constructs import Construct
from aws_cdk import (
    aws_ec2 as ec2,
    Stack,
    CfnOutput
)
from aws_cdk import Tags



class NetworkStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> Construct:

        super().__init__(scope, construct_id, **kwargs)

        self.vpc = ec2.Vpc(self, "EMRStackVPC",
            vpc_name="EMR-Dev",
            ip_addresses=ec2.IpAddresses.cidr("10.100.0.0/16"),
            subnet_configuration=[
                ec2.SubnetConfiguration(
                    subnet_type=ec2.SubnetType.PUBLIC,
                    name="Public",
                    cidr_mask=24
                ),
                ec2.SubnetConfiguration(
                    subnet_type=ec2.SubnetType.PRIVATE_ISOLATED,  # refactor
                    name="Private",
                    cidr_mask=24
                ),
                ec2.SubnetConfiguration(
                    subnet_type=ec2.SubnetType.PRIVATE_ISOLATED,
                    name="DB",
                    cidr_mask=24
                )
            ],
            max_azs=3,
            # nat_gateways=0
            # gateway_endpoints={
            #     "S3": ec2.GatewayVpcEndpointOptions(service=ec2.GatewayVpcEndpointAwsService.S3)
            # }
        )

        CfnOutput(self, "EMRVpcId", value=self.vpc.vpc_id, export_name="EMRVpcId")

        for i, subnet in enumerate(self.vpc.public_subnets):
            CfnOutput(
                self, f"PublicSubnet{i}",
                value=subnet.subnet_id,
                export_name=f"PublicSubnet{i}"
            )

        for i, subnet in enumerate(self.vpc.private_subnets):
            CfnOutput(
                self, f"PrivateSubnet{i}",
                value=subnet.subnet_id,
                export_name=f"PrivateSubnet{i}"
            )

        Tags.of(self.vpc).add("for-use-with-amazon-emr-managed-policies", "true")
