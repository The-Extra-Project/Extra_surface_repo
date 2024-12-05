#!/bin/bash
DDT_MAIN_DIR="/app/"
PARAM_PATH=$INPUT_DATA_DIR/params.xml

export INPUT_DATA_DIR="/app/datas/"
export OUTPUT_DATA_DIR="/app/out/"
export GLOBAL_BUILD_DIR="/app/build/"

export SCRIPT_DIR="/app/services/wasure/workflow"

export TMP_DIR="/tmp/"
export SPARK_TMP_DIR="/tmp/spark/"
export SPARK_LOG_DIR="/tmp/spark/logs/"
export SPARK_EVENTS_DIR="/tmp/spark/events/"
export SPARK_SHARED_DIR="/app/storage/shared_spark/"
export SPARK_SHELL="/usr/local/bin/spark-shell"

# mkdir -p $SPARK_EVENTS_DIR; chmod 777 /tmp/spark-events

### Run spark-shell with a given script,params and input dir.
# INPUT_DIR  : The directory with ply file
# OUTPUT     : The output directcory
# PARAMS     : Xml file with algo parameters
# FILESCRIPT : Scala algorithm
function process {
    if [ -z "$PARAMS" ]; then PARAMS="void.xml"; fi

    MOUNT="-v ${INPUT_DIR}:${INPUT_DIR} \
           -v ${OUTPUT_DIR}:${OUTPUT_DIR} \
           -v ${SPARK_EVENTS_DIR}:${SPARK_EVENTS_DIR}"

    $SPARK_SHELL -i $SCRIPT --jars ${BUILD_DIR}/spark/target/scala-2.13/iqlib-spark_2.13-1.0.jar
      #--conf \"spark.local.dirs=${SPARK_TMP_DIR}\"  \
      #--conf \"spark.history.fs.logDirectory=${SPARK_LOG_DIR}\" \
      #--conf \"yarn.nodemanager.log-dirs=${SPARK_LOG_DIR}\"
      #-Dspark.executor.memory=1g -Dspark.driver.memory=1g

    docker run --memory=16g --cpus="2.0" -u $(id -u):$(id -g) --privileged -d -it --name $CONTAINER_NAME $MOUNT_CMD \
				-e COLUMNS="$(tput cols)" -e LINES="$(tput lines)" -e DDT_MAIN_DIR="$DDT_MAIN_DIR_DOCKER" \
				${NAME_IMG}

    RUN="${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh \
            -m "${MOUNT}" -l \"${CMD}\" \
            -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}"

    if [ "$DEBUG_MODE" == "true" ]; then
        eval "$RUN -d bash"
    else
        eval "$RUN"
        return 0
    fi

    return 0
}


# Parse command-line options
while getopts "j:s:i:d" OPTION; do
  case $OPTION in
    j)
        JOB="${OPTARG}"
        ;;
    s)
        INPUT_S3="${OPTARG}"
        ;;
    i)
        IMAGE="${OPTARG}"
        ;;
    d)
        DEBUG="True"
        ;;
    r)
        export ALGO_SEED="${OPTARG}"
        ;;
  esac
done


case $JOB in
  "pre")
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    SCRIPT="${SCRIPT_DIR}/workflow_preprocess.scala"
    download_inputs
    echo -e "\Parameter file: \n\t${PARAMS} \nScript:\n\t${FILE_SCRIPT}"
    docker run -d $IMAGE
    echo "[PREPROCESSING INPUTS]"
    process
  ;;

  "pro")
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    SCRIPT="${SCRIPT_DIR}/workflow_preprocess.scala"
    echo -e "\Parameter file: \n\t${PARAMS} \nScript:\n\t${FILE_SCRIPT}"
    echo "[PREPROCESSING INPUTS]"
    process
  ;;

  "lod")
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    SCRIPT="${SCRIPT_DIR}/workflow_preprocess.scala"
    echo -e "\Parameter file: \n\t${PARAMS} \nScript:\n\t${FILE_SCRIPT}"
    echo "[PREPROCESSING INPUTS]"
    process
  ;;

  *)
    echo "Invalid job type: $JOB"
    exit 1
  ;;

esac



echo -e "\n\n---[Reconstruction]---"
INPUT_DIR=${OUTPUT_DIR}
PARAMS="${OUTPUT_DIR}/wasure_metadata_3d_gen.xml"
FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
echo -e "\nParameters: \n\t${PARAMS} \nScript:\n\t${FILE_SCRIPT}"
process

echo -e "\n\n---[Creating LODs]---"
mkdir -p ${OUTPUT_DIR}/LODs
source ${DDT_MAIN_DIR}/services/mesh23dtile/run.sh --input_dir ${OUTPUT_DIR}/outputs/tiles/ --xml_file ${PARAMS} --output_dir ${OUTPUT_DIR}/LODs

exit 0
