import os
import  pyspark 
import pyspark.sql.functions as F
from pyspark.sql import SQLContext
from pyspark.sql.session import SparkSession

MASTER_IP = "localhost"

#  CMD_DOCKER="docker run  \
#        -u 0 \
#        -v ${INPUT_DIR}:${INPUT_DIR} -v ${OUTPUT_ROOT}:${OUTPUT_ROOT} ${MOUNT_LOCAL} \
#        -v ${TMP_DIR}:${TMP_DIR} \
#        --rm \
#        -it \
#        -e NAME_IMG_BASE=${NAME_IMG_BASE} -e DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR_DOCKER} \
#        -e CONTAINER_NAME_SHELL=${CONTAINER_NAME_SHELL} -e CONTAINER_NAME_COMPILE=${CONTAINER_NAME_COMPILE} \
#        -e TMP_DIR=${TMP_DIR} -e SPARK_TMP_DIR=${SPARK_TMP_DIR} -e SPARK_HISTORY_DIR=${SPARK_HISTORY_DIR} \
#        -e CURRENT_PLATEFORM=${CURRENT_PLATEFORM} -e MASTER_IP_SPARK=${MASTER_IP_SPARK} \
#        -e SPARK_EXECUTOR_MEMORY=${SPARK_EXECUTOR_MEMORY} -e SPARK_DRIVER_MEMORY=${SPARK_DRIVER_MEMORY} -e SPARK_WORKER_MEMORY=${SPARK_WORKER_MEMORY} -e NUM_PROCESS=${NUM_PROCESS} \
#        ${NAME_IMG_BASE} /bin/bash -c \"${CMD}\""


if __name__ == "__main__":
    spark_session = SparkSession.builder.appName("Spark_extrasurface").config("spark.jars", "./build/spark/target/scala-2.13/iqlib-spark_2.13-1.0.jar").config("master", "http://${MASTER_IP}:7077") \
    .config("spark.driver.memory", "16G") \
    .config("spark.executor.memory", "16G") \
    .config("spark.worker.memory", "16G") \
    .config("spark.history.dir", "./tmp/spark/") \
    .getOrCreate() 
    
    
    context = spark_session.sparkContext
    context.setLogLevel("ERROR")
    sqlContext = SQLContext(context)
    print(sqlContext.cacheTable)