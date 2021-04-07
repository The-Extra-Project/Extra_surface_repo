
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"



DO_RUN=true

function cnes_run_algo
{

    ## GOOOD
    # spark-shell  --master spark://${NODE_NAME}:7077 yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}"  --properties-file spark-defaults.conf
    spark-shell  --master spark://${NODE_NAME}:7077 yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}"  --files spark-defaults.conf,log4j-driver.properties,log4j-executor.properties --conf "spark.driver.extraJavaOptions=-Djava.security.auth.login.config=spark-defaults.conf -Dlog4j.configuration=file:log4j-driver.properties -Dvm.logging.level=DEBUG -Dvm.logging.name=SparkStreaming-1" --conf "spark.executor.extraJavaOptions=-Djava.security.auth.login.config=spark-defaults.conf -Dlog4j.configuration=file:log4j-executor.properties -Dvm.logging.level=DEBUG -Dvm.logging.name=SparkStreaming-1" 
    #--conf "spark.executor.extraJavaOptions='-Dlog4j.configuration=log4j.properties'" --driver-java-options "-Dlog4j.configuration=file:/home/ad/caraffl/code/spark-ddt/log4j-cnes.properties"
    #spark-shell --master spark://${NODE_NAME}:7077
}


function run_multivac_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac.scala"
    export INPUT_DATA_DIR="/work/scratch/caraffl/datas/church/preprocessed_small_2/"
    export OUTPUT_DATA_DIR="/work/scratch/caraffl/output/church/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"
    export GLOBAL_BUILD_DIR="${DDT_MAIN_DIR}/build/"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_bp.xml"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_small.xml"
    cnes_run_algo
}

function cnes_load_spark
{
    module load spark
}


# Init
function cnes_init_spark
{
    OUTPUT=$(qsub start_cluster.sh)
    echo "cluster start => $OUTPUT"
    QSUB_ID="${OUTPUT%.*}"
    echo "QSUB ID => $QSUB_ID"
    module load spark
    sleep 3
    NODE_NAME=$(qstat -f ${QSUB_ID} | grep exec_host | grep -Po  'node[0-9]*' | head -1)
    echo "Node name => $NODE_NAME"
}
# Start Spark


function kill_spark
{
    qdel $QSUB_ID 
}


# cnes_init_spark
# cnes_load_spark
# run_multivac_church


# Kill job
#qdel $QSUB_ID 
