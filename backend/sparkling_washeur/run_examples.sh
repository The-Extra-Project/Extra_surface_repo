#!/bin/bash

DATA_PATH="${PWD}/"
source ./algo-env.sh

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"

APP_DIR=/app/wasure

DO_USE_LOCAL_BUILD="TRUE"
if [[ ${DO_USE_LOCAL_BUILD} == "TRUE" ]]; then
    APP_DIR="${PWD}"
    MOUNT_LOCAL=" -v ${APP_DIR}/:${APP_DIR}"
fi

function run_cmd_container
{   
    OUTPUT_ROOT=$(dirname ${OUTPUT_DIR})/
    CMD_DOCKER="docker run  \
       -u 0 \
       -v ${INPUT_DIR}:${INPUT_DIR} -v ${OUTPUT_ROOT}:${OUTPUT_ROOT} ${MOUNT_LOCAL} \
       -v ${TMP_DIR}:${TMP_DIR} \
       --rm \
       -it \
       -e NAME_IMG_BASE=${NAME_IMG_BASE} -e DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR_DOCKER} \
       -e CONTAINER_NAME_SHELL=${CONTAINER_NAME_SHELL} -e CONTAINER_NAME_COMPILE=${CONTAINER_NAME_COMPILE} \
       -e TMP_DIR=${TMP_DIR} -e SPARK_TMP_DIR=${SPARK_TMP_DIR} -e SPARK_HISTORY_DIR=${SPARK_HISTORY_DIR} \
       -e CURRENT_PLATEFORM=${CURRENT_PLATEFORM} -e MASTER_IP_SPARK=${MASTER_IP_SPARK} \
       -e SPARK_EXECUTOR_MEMORY=${SPARK_EXECUTOR_MEMORY} -e SPARK_DRIVER_MEMORY=${SPARK_DRIVER_MEMORY} -e SPARK_WORKER_MEMORY=${SPARK_WORKER_MEMORY} -e NUM_PROCESS=${NUM_PROCESS} \
       ${NAME_IMG_BASE} /bin/bash -c \"${CMD}\""
    eval $CMD_DOCKER
}

$@



INPUT_DIR=${PWD}/datas/demo_lhd/
OUTPUT_DIR=${PWD}/datas/output_examples/demo_lhd

CMD="${APP_DIR}/run_workflow.sh --input_dir ${INPUT_DIR} --output_dir ${OUTPUT_DIR} --colorize"
run_cmd_container 

