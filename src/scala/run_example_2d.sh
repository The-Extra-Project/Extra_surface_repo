if ! [ -f /.dockerenv ]; then
    echo "Error : should be run inside docker";
    exit 1
fi
cd ${DDT_MAIN_DIR}

INPUT_SCRIPT=${DDT_MAIN_DIR}/src/scala/ddt_stream.scala

export INPUT_DATA_DIR="${DDT_MAIN_DIR}/build-spark/results/set_img1_inputs_2"
export OUTPUT_DATA_DIR="${DDT_MAIN_DIR}/build-spark/results/set_img1_outputs"
export PARAM_PATH="${INPUT_DATA_DIR}/metadata.xml"

## Clear or not dir
if [ -d "${OUTPUT_DATA_DIR}" ]; then
    echo "===================="
    echo "${OUTPUT_DATA_DIR} exists  "
    read -p "do yo want to clear it? [y/n] " -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
	rm -rf ${OUTPUT_DATA_DIR}
    fi
fi

## Clear or not dir
if [ -d "${INPUT_DATA_DIR}" ]; then
    echo "===================="
    echo "${INPUT_DATA_DIR} exists  "
    read -p "do yo want to clear it? [y/n] " -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
	rm -rf ${INPUT_DATA_DIR}/
    fi
fi


## Create dirs
mkdir -p  ${OUTPUT_DATA_DIR} ${OUTPUT_DATA_DIR} ${INPUT_DATA_DIR} 
cd ${OUTPUT_DATA_DIR}


# Run 
${DDT_MAIN_DIR}/src/spark/spark.sh start_all
/usr/local/bin/spark-2.2.0-bin-hadoop2.7/bin/spark-shell \
    -i ${INPUT_SCRIPT}  \
    --jars ${DDT_MAIN_DIR}/build-spark/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar  \
    --master spark://localhost:7077  -Dspark.executor.memory=12g -Dspark.driver.memory=12g


