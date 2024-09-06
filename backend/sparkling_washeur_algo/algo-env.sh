## Docker 
export NAME_IMG_BASE=ddt_img_base_devel
export CONTAINER_NAME_SHELL="ddt_container_shell"
export CONTAINER_NAME_COMPILE="ddt_container_compile"
export DO_USE_LOCAL_BUILD="FALSE"
export DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR} ## Used when called inside docker


## Algo params
# Number of parallel process
export NUM_PROCESS="4"

## Apache/Spark
# WARNING : temporary data from Apache Spark are stored into the SHARED_DIR directory when used locally. The default value is the current directory, you need a quick access and a huge amont of free space so change it if it's not the case.
export SHARED_DIR="${DDT_MAIN_DIR_DOCKER}/shared_spark/"
export TMP_DIR="${SHARED_DIR}/tmp/"
export SPARK_TMP_DIR="${TMP_DIR}/spark/"
export SPARK_HISTORY_DIR="${SPARK_TMP_DIR}"
export APP_DIR="/app/wasure/"
export CURRENT_PLATEFORM="local"

export MASTER_IP_SPARK="localhost"
export CORE_IP_SPARK=""
export SPARK_EXECUTOR_MEMORY="16G"
export SPARK_DRIVER_MEMORY="16G"
export SPARK_WORKER_MEMORY="16G"


if [[ ${DO_USE_LOCAL_BUILD} == "TRUE" ]]; then
    export APP_DIR="${PWD}"
    export MOUNT_LOCAL=" -v ${APP_DIR}/:${APP_DIR} "
else
    export APP_DIR="/app/wasure/"
fi
