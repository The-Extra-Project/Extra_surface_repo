## Docker 
export NAME_IMG_BASE=ddt_img_base_devel
export CONTAINER_NAME_SHELL="ddt_container_shell"
export CONTAINER_NAME_COMPILE="ddt_container_compile"

## Shared dir
export DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR}
export SHARED_DIR="${DDT_MAIN_DIR_DOCKER}/shared_spark/"
export TMP_DIR="${SHARED_DIR}/tmp/"
export SPARK_TMP_DIR="${TMP_DIR}/spark/"
export SPARK_HISTORY_DIR="${SPARK_TMP_DIR}"


## Spark
export CURRENT_PLATEFORM="local"
export MASTER_IP_SPARK="localhost"

### Cluster configuration
## Local Cluster configuration
export SPARK_EXECUTOR_MEMORY="8G"
export SPARK_DRIVER_MEMORY="8G"
export SPARK_WORKER_MEMORY="8G"
export NUM_PROCESS="12"
