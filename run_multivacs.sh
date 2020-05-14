#!/bin/bash

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
#DEBUG_FLAG="-d"
DO_RUN=true


function run_algo_multivac
{
    echo ""
    echo "##  ------  ${FUNCNAME[1]}  ------"
    spark-shell -i  \ #${FILE_SCRIPT} \
		--master yarn --deploy-mode client \
		--jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar \
		--executor-cores ${MULTIVAC_EXECUTOR_CORE} \
		--executor-memory ${MULTIVAC_EXECUTOR_MEMORY} \
		--driver-memory ${MULTIVAC_DRIVER_MEMORY} \
		--num-executors ${MULTIVAC_NUM_EXECUTORS} \
		--conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}" \
		--conf "spark.dynamicAllocation.enabled=false" \
                --conf "yarn.nodemanager.pmem-check-enabled=false" \
		--conf "yarn.nodemanager.vmem-pmem-ratio=5"  
}


function run_mutlivac_random
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/random/"
    export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"
    export PARAM_PATH="${INPUT_DATA_DIR}/unitest_multivac.xml"    
    run_algo_multivac
}

function run_mutlivac_stereopolis
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/stereopolis/"
    export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"
    export PARAM_PATH="hdfs:/user/lcaraffa/datas/3d_stereopolis/stereopolis_metadata_small.xml"    
    run_algo_multivac
}

function run_multivac_toulouse
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/preprocessed_toulouse_full/"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/toulouse/"
    export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"    
    run_algo_multivac
}


function run_multivac_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/preprocessed_small/"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/church/preprocessed_small/"
    export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"    
    run_algo_multivac
}


#run_mutlivac_random

#run_mutlivac_stereopolis

run_multivac_church





