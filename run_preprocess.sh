#!/bin/bash

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
DEBUG_FLAG="-d"
DO_RUN=true


function run_algo_docker
{
    echo ""
    echo "##  ------  ${FUNCNAME[1]}  ------" 
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c 2 -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    echo ${CMD}
    if [ "$DO_RUN" = true ] ; then
	eval ${CMD}
    fi
}


function run_format_3D
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="${HOME}/shared_spark/inputs/stereopolis/raw/"
    OUTPUT_DIR="${HOME}/shared_spark/inputs/stereopolis_solo/stream_pruned/"
    PARAMS="${INPUT_DIR}/stereopolis_metadata.xml"
    run_algo_docker 
}




run_format_3D




