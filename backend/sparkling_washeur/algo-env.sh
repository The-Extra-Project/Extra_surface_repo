## Docker 
export NAME_IMG_BASE="emr"  # ddt_img_base_multistage ddt_img_base_22_04
export CONTAINER_NAME_SHELL="emr-shell" # ddt_container_shell
export CONTAINER_NAME_COMPILE="emr" # ddt_img_base_multistage

## Shared dir
export DDT_MAIN_DIR_DOCKER=${DDT_MAIN_DIR}
export SPARK_SHARED_DIR="${DDT_MAIN_DIR_DOCKER}/storage/shared_spark/"
export TMP_DIR="${SPARK_SHARED_DIR}/tmp/"
export SPARK_TMP_DIR="${TMP_DIR}/spark/"
export SPARK_HISTORY_DIR="${SPARK_TMP_DIR}"
export SPARK_EVENTS_DIR=/tmp/spark-events

## Spark
export CURRENT_PLATEFORM="local"
export MASTER_IP_SPARK="localhost"

### Cluster configuration
## Local Cluster configuration
export SPARK_EXECUTOR_MEMORY="16G"
export SPARK_DRIVER_MEMORY="16G"
export SPARK_WORKER_MEMORY="16G"
export NUM_PROCESS="14"
