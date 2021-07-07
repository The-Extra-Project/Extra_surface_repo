#!/bin/bash

### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/inputs/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
DEBUG_FLAG="-d"
DO_RUN=true


### Run spark-shell with a given script,params and input dir.
# INPUT_DIR  : The directory with ply file
# OUTPUT     : The output directcory
# PARAMS     : Xml file with algo prameters
# FILESCRIPT : Scala algorithm
function run_algo_docker
{

    if [ -z "$PARAMS" ]; then PARAMS="void.xml"
    fi
    
    echo ""
    echo "##  ------  ${FUNCNAME[1]}  ------" 
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c 4 -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    echo ${CMD}
    if [ "$DO_RUN" = true ] ; then
	eval ${CMD}
    fi
}

### Evaluation
function run_3d_evaluation_tiling
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"

    FILES="${INPUT_DIR}/wasure_metadata_3d_octree*.xml"
    for f in $FILES
    do
	PARAMS="${f}"
	run_algo_docker
    done
    
 #   run_algo_docker
}

### 3D Surface reconstruction 
function run_3d_evaluation_coef_mult
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_eval_1.xml"
    run_algo_docker
}



### 3D Surface reconstruction 
function run_3d_bench_raw
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_ori_do.xml"
    run_algo_docker
}

function run_3d_bench_preprocess
{
    DEBUG_FLAG=""
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    run_algo_docker

    DEBUG_FLAG="-d"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${OUTPUT_DIR}"
    OUTPUT_DIR="${OUTPUT_DIR}"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_gen.xml"
    run_algo_docker
}


### 3D Surface reconstruction 
function run_3d_yanis
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_yanis/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

function run_3d_yanis_2
{
    DEBUG_FLAG=""
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_yanis_2/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # run_algo_docker

    DEBUG_FLAG="-d"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${OUTPUT_DIR}"
    OUTPUT_DIR="${OUTPUT_DIR}"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_gen.xml"
    run_algo_docker
}


### 3D Surface reconstruction 
function run_3d_bench_small
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench_small/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

 ### 3D Surface reconstruction                                                                                                                                                                                                                
 function run_3d_toulouse
 {
     FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
     INPUT_DIR="${GLOBAL_INPUT_DIR}/toulouse_pp/"
     OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
     PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
     run_algo_docker
 }

 ### 3D Surface reconstruction                                                                                                                                                                                                                
 function run_aerial
 {
     FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
     INPUT_DIR="${GLOBAL_INPUT_DIR}/aerial/aerial_small_pp/"
     OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
     PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
     run_algo_docker
 }

 
function run_3d_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="~/shared_spark/inputs/church/preprocessed_small_2/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_buggy.xml"
    run_algo_docker
}


 
# format data
function preprocess_data
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_yanis/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/datas/3d_yanis_processed/"
    run_algo_docker    
}

# format data
function preprocess_toulouse
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"

    INPUT_DIR="/mnt/ssd/las_focal_0_cc/"
    OUTPUT_DIR="/mnt/ssd/las_focal_0_pp/"

    run_algo_docker
}


# ==== surface reconstruction workflow ====
## Preprocess data
#preprocess_toulouse

# Evaluation
#run_3d_evaluation_tiling
#run_3d_evaluation_coef_mult


### 3D
#run_3d_bench_small
run_3d_bench_raw
#run_3d_bench_preprocess
#run_3d_yanis
#run_3d_yanis_2

#run_aerial
#run_3d_church
#run_3d_bench_preprocessed
#run_3d_bench_small
#run_3d_toulouse






















