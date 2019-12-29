#!/bin/bash


### Start workflow in local mode 

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"
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
    echo ""
    echo "##  ------  ${FUNCNAME[1]}  ------" 
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c 4 -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    echo ${CMD}
    if [ "$DO_RUN" = true ] ; then
	eval ${CMD}
    fi
}


### Distributed delaunay triangulation on random datasets
function run_2d_ddt_random
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt/workflow/workflow_ddt.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/random/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/unitest_metadata_2D.xml"
    run_algo_docker
}

### Distributed delaunay triangulation on random datasets
function run_3d_ddt_random
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt/workflow/workflow_ddt.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/random/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/unitest_metadata_3D.xml"
    run_algo_docker
}


### 2D img ddt
function run_2d_img_ddt
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt/workflow/workflow_ddt.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/2d_austin/imgs/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/ddt_metadata.xml"
    run_algo_docker
}





# ========== Random ddt workflow =============
# run_2d_ddt_random
 run_3d_ddt_random

# ==== 2D surface reconstruction workflow ====
#run_2d_img_ddt

















