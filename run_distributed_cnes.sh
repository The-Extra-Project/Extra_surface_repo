
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"

DO_RUN=true

export MULTIVAC_MEMORY_OVERHEAD="4G"
export CURRENT_PLATEFORM="cnes"

function cnes_run_algo
{
    ## WORKS!!
    # spark-shell  --master spark://${NODE_NAME}:7077 yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}"  
    echo "SPARK_TMP_DIR =>>> $SPARK_TMP_DIR"
    echo ":load /home/ad/caraffl/code/spark-ddt/services/wasure/workflow/workflow_wasure_generic.scala"
    spark-shell -i /home/ad/caraffl/code/spark-ddt/services/wasure/workflow/workflow_wasure_generic.scala --master spark://${NODE_NAME}:7077 yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --driver-memory 10G --executor-memory 15G --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}" -Dlog4j.configuration=log4j-driver.properties --files log4j-driver.properties,log4j-executor.properties  --conf spark.executor.extraJavaOptions=-Dlog4j.debug=true  --conf spark.executor.extraJavaOptions=-Dlog4j.configuration=file:./log4j-executor.properties  --conf spark.yarn.app.container.log.dir=/home/ad/caraffl/code/spark-ddt  --conf "spark.memory.offHeap.enabled=true"   --conf "spark.memory.offHeap.size=10g"  --conf spark.local.dir=${SPARK_TMP_DIR}
    
    ## GOOOD
    # spark-shell  --master spark://${NODE_NAME}:7077 yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}"  --properties-file spark-defaults.conf

    #--conf "spark.executor.extraJavaOptions='-Dlog4j.configuration=log4j.properties'" --driver-java-options "-Dlog4j.configuration=file:/home/ad/caraffl/code/spark-ddt/log4j-cnes.properties"
    #spark-shell --master spark://${NODE_NAME}:7077
}

function run_cnes_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac_generic.scala"
    export INPUT_DATA_DIR="/work/scratch/caraffl/datas/church/preprocessed_small_2/"
    export OUTPUT_DATA_DIR="/work/scratch/caraffl/output/church/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"
    export GLOBAL_BUILD_DIR="${DDT_MAIN_DIR}/build/"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_bp.xml"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_small.xml"
    cnes_run_algo
}

function run_cnes_aerial
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac_generic.scala"
    export INPUT_DATA_DIR="/work/scratch/caraffl/datas/toulouse_aerial_focal_2_ply_stream/"
    export OUTPUT_DATA_DIR="/work/scratch/caraffl/output/toulouse/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_gen.xml"
    export GLOBAL_BUILD_DIR="${DDT_MAIN_DIR}/build/"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_bp.xml"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_small.xml"
    cnes_run_algo
}

function run_cnes_toulouse_v4
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac_generic.scala"
    export INPUT_DATA_DIR="/work/scratch/caraffl/datas/toulouse_v4_pp/"
    export OUTPUT_DATA_DIR="/work/scratch/caraffl/output/toulouse_v4_pp/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"
    export GLOBAL_BUILD_DIR="${DDT_MAIN_DIR}/build/"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_bp.xml"
#    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_small.xml"
    cnes_run_algo
}

function run_cnes_toulouse_full
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_multivac_generic.scala"
    export INPUT_DATA_DIR="/work/scratch/caraffl/datas/toulouse_full/"
    export OUTPUT_DATA_DIR="/work/scratch/caraffl/output/toulouse_full/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_gen.xml"
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
    OUTPUT=$(qsub ${DDT_MAIN_DIR}/services/pbs/start_cluster_v2.sh)
    echo "cluster start => $OUTPUT"
    echo "TMPDIR => $TMPDIR"
    QSUB_ID="${OUTPUT%.*}"
    echo "QSUB ID => $QSUB_ID"
    module load spark
    sleep 3
    NODE_NAME=$(qstat -f ${QSUB_ID} | grep exec_host | grep -Po  'node[0-9]*' | head -1)
    echo $TMPDIR
    echo "Node name => $NODE_NAME"
    echo ""
    echo "/!\\ Spark context Web UI available at /!\\"
    echo "${NODE_NAME}:8080"
    echo ""
}

# Init
function cnes_init_spark_v2
{
    OUTPUT=$(qsub ${DDT_MAIN_DIR}/services/pbs/start_cluster_v2.sh)
    echo "cluster start => $OUTPUT"
    export SPARK_TMP_DIR="/tmp/pbs.${OUTPUT}"
    QSUB_ID="${OUTPUT%.*}"
    echo "QSUB ID => $QSUB_ID"
    module load spark
    queue_string='    job_state = Q'
    while [[ $queue_string == *"job_state = Q"* ]]; do
	echo "Is in queue"
	sleep 3
	queue_string=$(qstat -f ${QSUB_ID}  | grep job_state)
    done
    NODE_NAME=$(qstat -f ${QSUB_ID} | grep exec_host | grep -Po  'node[0-9]*' | head -1)
    echo "Node name => $NODE_NAME"
    echo ""
    echo "/!\\ Spark context Web UI available at /!\\"
    echo "${NODE_NAME}:8080"
    echo ""
    run_cnes_toulouse_full
}

# Start Spark

function cnes_singularity
{
    module load singularity
    singularity run ~/wasure.simg
    export LD_LIBRARY_PATH=${DDT_MAIN_DIR}/build/build-spark-Release-3/lib:$LD_LIBRARY_PATH    
}

function get_first_id
{
    qstat | grep ${USER} |  awk '{print $1;}'
}

function print_current_jobs
{
     qstat | grep $USER
}

function kill_spark
{
    qdel $QSUB_ID 
}

cnes_init_spark_v2
