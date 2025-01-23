### Currently not used


# from aws_cdk import (
#     # Duration,
#     Stack,
#     aws_ecs as ecs,
#     aws_ecs_patterns,
#     aws_ec2,
#     App,Stack,
#     aws_autoscaling
# )
# import os
# from constructs import Construct

# from dotenv import load_dotenv

# app = App()


# class InfraStack(Stack):

#     def __init__(self, scope: Construct, construct_id: str, **kwargs) -> None:
#         super().__init__(scope, construct_id, **kwargs)

#         load_dotenv()

#         self.stack = Stack(app, "extrasurface-full-stack")

#         self.vpc = aws_ec2.Vpc(
#             self,
#             "extrasurface",
#         )
#         self.cluster = ecs.Cluster(self, "Extrasurface_cluster")
#         self.asg = aws_autoscaling.AutoScalingGroup(
#         self.stack, "DefaultAutoScalingGroup",
#         instance_type=aws_ec2.InstanceType("c5a.4xlarge"),
#         machine_image=ecs.EcsOptimizedImage.amazon_linux2(),
#         vpc=self.vpc,
#         )

#         capacity_provider = ecs.AsgCapacityProvider(self.stack, "AsgCapacityProvider",
#             auto_scaling_group=self.asg
#             )
#         self.cluster.add_asg_capacity_provider(capacity_provider)

#         self.task_definition = ecs.Ec2TaskDefinition(
#             self.stack, "nginx-awsvpc", network_mode=ecs.NetworkMode.AWS_VPC,
#         )

#         self.api_container = self.task_definition.add_container(
#             "api",
#             image = ecs.ContainerImage.from_registry("devextralabs/distributed-delaunay-transformation:latest"),
#             environment = {**dict(os.environ)},
#             port_mappings = [ecs.PortMapping(container_port=8000)]
#         )
#         # Define the Sparkling Washeur Core service
#         self.sparkling_container = self.task_definition.add_container(
#             "sparkling_washeur_core",
#             image=ecs.ContainerImage.from_registry("devextralabs/distributed-delaunay-transformation"),
#             environment={"NB_PROC": "4", "DDT_TRAITS": "3"},
#             port_mappings=[ecs.PortMapping(container_port=8082)]
#         )
#         # Define the Redis service
#         self.redis_container = self.task_definition.add_container("redis",
#             image=ecs.ContainerImage.from_registry("redis:latest"),
#             port_mappings=[ecs.PortMapping(container_port=6379)]
#         )
#         # Define the Website service
#         self.website_container = self.task_definition.add_container("website",
#             image=ecs.ContainerImage.from_registry("your-website-image"),
#             port_mappings=[ecs.PortMapping(container_port=3000)]
#         )

#         # Create a Fargate service for the task definition
#         self.service = ecs.Ec2Service(
#             self,
#             "Fargate-Service",
#             cluster=self.cluster,
#             task_definition=self.task_definition
#         )

#         self.service.node.add_dependency(self.api_container)
#         self.service.node.add_dependency(self.sparkling_container)
#         self.service.node.add_dependency(self.redis_container)



# app.synth()
