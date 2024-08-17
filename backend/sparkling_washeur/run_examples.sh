#!/bin/bash

DATA_PATH="${PWD}/"
source ./algo-env.sh

function run_cmd_container
{   
    CMD_DOCKER="docker run  \
       -u 0 \
       -v ${DATA_PATH}:${DATA_PATH} \
       -v ${TMP_DIR}:${TMP_DIR} \
       --rm \
       -it \
       -e NAME_IMG_BASE=${NAME_IMG_BASE} -e DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR_DOCKER} \
       -e CONTAINER_NAME_SHELL=${CONTAINER_NAME_SHELL} -e CONTAINER_NAME_COMPILE=${CONTAINER_NAME_COMPILE} \
       -e TMP_DIR=${TMP_DIR} -e SPARK_TMP_DIR=${SPARK_TMP_DIR} -e SPARK_HISTORY_DIR=${SPARK_HISTORY_DIR} \
       -e CURRENT_PLATEFORM=${CURRENT_PLATEFORM} -e MASTER_IP_SPARK=${MASTER_IP_SPARK} \
       -e SPARK_EXECUTOR_MEMORY=${SPARK_EXECUTOR_MEMORY} -e SPARK_DRIVER_MEMORY=${SPARK_DRIVER_MEMORY} -e SPARK_WORKER_MEMORY=${SPARK_WORKER_MEMORY} -e NUM_PROCESS=${NUM_PROCESS} \
       ${NAME_IMG_BASE} /bin/bash -c  "${PWD}/run_lidarhd.sh --input_dir ${2} --output_dir ${3} --colorize"
       "
    eval $CMD_DOCKER
}

$@

# CMD="/app/wasure/run_workflow.sh --input_dir ${PWD}/datas/lidar_hd_crop_1/ --output_dir ${PWD}/outputs_examples/lidar_hd_crop_1_v2/ --colorize"
# run_cmd_container "$CMD"

