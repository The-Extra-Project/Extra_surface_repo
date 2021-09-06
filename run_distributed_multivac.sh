#!/bin/bash
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
#DEBUG_FLAG="-d"
DO_RUN=true


function export_default_parallelism
{
    echo "$ff:$1"
    VV=$(cat $1 | grep nbp  | grep -o -E '[0-9]+'| head -n 1 | sed -e 's/^0\+//')
    PPT=$(cat $1 | grep max_ppt  | grep -o -E '[0-9]+'| head -n 1 | sed -e 's/^0\+//')    
    export DEFAULT_PARALLELISM=$((2*(VV/PPT)))
    echo "$VV, $PPT, $DEFAULT_PARALLELISM"
}


function run_algo_multivac
{
    echo ""
    echo "##  ------  ${FUNCNAME[1]}  ------"
    echo "##  "
    echo "spark-shell -i ${FILE_SCRIPT}"
    echo "	--master yarn --deploy-mode client "
    echo "	--jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar "
    echo "	--executor-cores ${MULTIVAC_EXECUTOR_CORE} "
    echo "	--executor-memory ${MULTIVAC_EXECUTOR_MEMORY} "
    echo "	--driver-memory ${MULTIVAC_DRIVER_MEMORY} "
    echo "	--num-executors ${MULTIVAC_NUM_EXECUTORS} "
    echo "	--conf spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}"
    echo "	--conf spark.default.parallelism=${DEFAULT_PARALLELISM}"
    echo "	--conf spark.serializer=org.apache.spark.serializer.KryoSerializer"  
    echo "	--conf spark.dynamicAllocation.enabled=false "
    echo "	--conf spark.cleaner.periodicGC.interval=2min "
    echo "	--conf spark.memory.fraction=0.2 "
    echo "	--conf spark.executor.extraJavaOptions=-verbose:gc -XX:+PrintGCDetails -XX:+PrintGCTimeStamps"

    export CURRENT_PLATEFORM="multivac"
    echo "cur plateform ==> $CURRENT_PLATEFORM"
#	    spark-shell -i  ${FILE_SCRIPT}\
	    spark-shell   --master yarn --deploy-mode client --jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar --executor-cores ${MULTIVAC_EXECUTOR_CORE} 	--executor-memory ${MULTIVAC_EXECUTOR_MEMORY} --driver-memory ${MULTIVAC_DRIVER_MEMORY} --num-executors ${MULTIVAC_NUM_EXECUTORS} --conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}" --conf "spark.serializer=org.apache.spark.serializer.KryoSerializer"  --conf "spark.dynamicAllocation.enabled=false" --conf "spark.cleaner.periodicGC.interval=2min" --conf "spark.memory.fraction=0.2" --conf "spark.executor.extraJavaOptions=-verbose:gc -XX:+PrintGCDetails -XX:+PrintGCTimeStamps" 

		#--conf "spark.default.parallelism=${DEFAULT_PARALLELISM}" \
                #--conf "yarn.nodemanager.pmem-check-enabled=false" \
		#--conf "yarn.nodemanager.vmem-pmem-ratio=5"  
}


export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"



function eval_params_loop {
    date_name="$(date +%Y_%m_%d_%H_%M)"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/church/preprocessed_small_3/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/church_eval_new_${date_name}/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_eval.xml"
    export LIST_EXECUTORS=" 7 4 2 1"
    export LIST_CORES="4 3 2 1"
    # export LIST_EXECUTORS="1"
    # export LIST_CORES="1"

    
    for ee in ${LIST_EXECUTORS}
    do
	for cc in ${LIST_CORES}
	do
    	    export MULTIVAC_NUM_EXECUTORS="$ee"
	    export MULTIVAC_EXECUTOR_CORE="$cc"
    	    if [ "$ee" == "0" ]; then
		echo "special case 0000000000000"
    		export MULTIVAC_NUM_EXECUTORS="1"
    		export MULTIVAC_EXECUTOR_CORE="1" 
    	    fi
    	    echo "num executors => $MULTIVAC_NUM_EXECUTORS"
    	    echo "num cores => $MULTIVAC_EXECUTOR_CORE"
    	    run_algo_multivac
	done
    done
}


function run_multivac_tau0
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/church/preprocessed_small_3/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/church/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_tau.xml"
    #export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_large.xml"
    run_algo_multivac
}


function run_multivac_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/church/preprocessed_small_2/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/church/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_visu.xml"
    #export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d_large.xml"
    run_algo_multivac
}

function run_multivac_aerial
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/aerial_stream/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/aerial/"
    export PARAM_PATH="${INPUT_DATA_DIR}wasure_metadata_3d.xml"    
    run_algo_multivac
}

function run_multivac_full
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/toulouse/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/toulouse/"
    export PARAM_PATH="${INPUT_DATA_DIR}/wasure_metadata_3d.xml"    
    run_algo_multivac
}


function run_multivac_full_v4
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/toulouse_v4_pp/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/toulouse_v4_pp/"
    export PARAM_PATH="${INPUT_DATA_DIR}/wasure_metadata_3d.xml"    
    run_algo_multivac
}


function run_multivac_croco
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/croco/"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/croco/"
    export PARAM_PATH="hdfs:/user/lcaraffa/datas/croco/wasure_metadata_3d.xml"    
    run_algo_multivac
}

#run_multivac_aerial
#eval_params_loop
run_multivac_church
#run_multivac_aerial
#
#run_multivac_full_v3
#run_multivac_full_v4

#run_multivac_tau0
#run_multivac_croco



