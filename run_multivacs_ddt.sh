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

    
    spark-shell -i ${FILE_SCRIPT} \
		--master yarn --deploy-mode client \
		--jars ${DDT_MAIN_DIR}/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar \
		--executor-cores ${MULTIVAC_EXECUTOR_CORE} \
		--executor-memory ${MULTIVAC_EXECUTOR_MEMORY} \
		--driver-memory ${MULTIVAC_DRIVER_MEMORY} \
		--num-executors ${MULTIVAC_NUM_EXECUTORS} \
		--conf "spark.executor.memoryOverhead=${MULTIVAC_MEMORY_OVERHEAD}" \
		--conf "spark.default.parallelism=${DEFAULT_PARALLELISM}" \
		--conf "spark.serializer=org.apache.spark.serializer.KryoSerializer"  \
		--conf "spark.dynamicAllocation.enabled=false" \
		--conf "spark.cleaner.periodicGC.interval=2min" \
		--conf "spark.memory.fraction=0.2" \
		--conf "spark.executor.extraJavaOptions=-verbose:gc -XX:+PrintGCDetails -XX:+PrintGCTimeStamps"
                #--conf "yarn.nodemanager.pmem-check-enabled=false" \
		#--conf "yarn.nodemanager.vmem-pmem-ratio=5"  
}

export INPUT_DATA_DIR="hdfs:/user/lcaraffa/datas/stereopolis_full/"
export HDFS_FILES_DIR="hdfs:/user/lcaraffa/tmp/"

function run_mutlivac_random
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export PARAM_PATH="${INPUT_DATA_DIR}/unitest_multivac.xml"    
    run_algo_multivacp
}

function run_mutlivac_stereopolis_full
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export PARAM_PATH="hdfs:/user/lcaraffa/datas/3d_stereopolis/stereopolis_metadata_full.xml"    
    run_algo_multivac
}

function run_mutlivac_stereopolis_full_1
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export PARAM_PATH="hdfs:/user/lcaraffa/datas/3d_stereopolis/stereopolis_metadata_full_1.xml"    
    run_algo_multivac
}

function run_mutlivac_stereopolis_full_3
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/output/"
    export PARAM_PATH="hdfs:/user/lcaraffa/datas/3d_stereopolis/stereopolis_metadata_full_3.xml"    
    run_algo_multivac
}



function eval_params_loop {
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
    	    for ff in ${filepath}
    	    do 
		fbasename=$(basename $ff)
    		export PARAM_PATH="hdfs:/user/lcaraffa/datas_xml/eval_dir/${EVAL_REP}/${fbasename}"
    		echo "$PARAM_PATH"
    		export_default_parallelism $ff
    		run_algo_multivac
    	    done
	done
    done
}


function run_multivac_eval
{

    eval_name="eval_loop_2019_08_10_FINISH2"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"


    
    # export EVAL_REP="eval_nbp"    
    # filepath=${DDT_MAIN_DIR}/datas/eval_dir/${EVAL_REP}/*xml
    # export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/${eval_name}/${EVAL_REP}/"
    # for ff in ${filepath}
    # do 
    #     fbasename=$(basename $ff)
    # 	export PARAM_PATH="hdfs:/user/lcaraffa/datas_xml/eval_dir/${EVAL_REP}/${fbasename}"
    # 	echo "$PARAM_PATH"
    # 	export_default_parallelism $ff
    # 	run_algo_multivac
    # done

    for ii in {1..1}
    do

	export EVAL_REP="eval_core_300M"
	filepath=${DDT_MAIN_DIR}/datas/eval_dir/${EVAL_REP}/*${ii}.xml
	export LIST_EXECUTORS=" 7"
	export LIST_CORES="1 2 3 4"
	export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/eval_church/${eval_name}/${EVAL_REP}_${ii}/"
	eval_params_loop

    done


}



function run_multivac_unitest
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt_multivacs.scala"

    export EVAL_REP="unitests"
    filepath=${DDT_MAIN_DIR}/datas/eval_dir/${EVAL_REP}/*xml
    export OUTPUT_DATA_DIR="hdfs:/user/lcaraffa/eval/${EVAL_REP}/"
    for ff in ${filepath}
    do 
        fbasename=$(basename $ff)
    	export PARAM_PATH="hdfs:/user/lcaraffa/datas_xml/eval_dir/${EVAL_REP}/${fbasename}"
    	echo "$PARAM_PATH"
	export_default_parallelism $ff
    	run_algo_multivac
    done	
}


#run_multivac_unitest
run_multivac_eval

#run_mutlivac_stereopolis_full_1


#run_mutlivac_random

#run_mutlivac_stereopolis_small
#run_mutlivac_stereopolis_full_1
#run_mutlivac_stereopolis_full_3
#run_mutlivac_eval



