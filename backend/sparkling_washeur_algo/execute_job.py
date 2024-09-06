import os
import sagemaker
from sagemaker import get_execution_role
from sagemaker.spark import PySparkProcessor
from pathlib import Path
from pyspark import SparkContext, SparkConf
import subprocess
import sagemaker_pyspark
from platformdirs import site_config_dir, user_config_dir


jar_deps = sagemaker_pyspark.classpath_jars()
spark_processor = PySparkProcessor(role=str(os.getenv("SAGEMAKER_EXECUTOR_ROLE")), instance_type="ml.r5.large", instance_count=2, framework_version="3.3")

def process_spark_job():
    try:
        spark_processor.run(submit_app="spark_job.py")
    except Exception as e:
        print("job execution on sagemaker failed:" + str(e))
if __name__ == "__main__":
    process_spark_job()