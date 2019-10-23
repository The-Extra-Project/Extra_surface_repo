#!/bin/bash


### Start workflow in local mode 

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${HOME}/shared_spark/tests_outputs/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
#DEBUG_FLAG="-d"
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
function run_ddt_random
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/random/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/unitest_metadata.xml"
    run_algo_docker
}

### 2D Surface reconstruction 
function run_2d_wasure
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/2d_austin/imgs/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_surface.xml"
    run_algo_docker
}

### 3D Surface reconstruction 
function run_3d_wasure
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_egouts/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

function run_ddt_benchmark
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/ddt-spark/workflow/workflow_ddt.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/random/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/benchmark_metadata.xml"
    run_algo_docker
}

### Create 2D ply from images 
function run_img2ply
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/create_datas/scripts/aerial_raytracing.py"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/2d_austin/imgs/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/datas/2d_austin/imgs/"
    CMD="python3  $FILE_SCRIPT  --img_dir $INPUT_DIR  --output_dir $OUTPUT_DIR --bin_dir ${DDT_MAIN_DIR}/build/build-spark-Release-D2/bin/"
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m " -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR}" -l "${CMD}" -i "${NAME_IMG_BASE}"  -c "${CONTAINER_NAME_SHELL}"
}



# ========== Random ddt workflow =============
#run_ddt_random

# ==== 2D surface reconstruction workflow ====
#run_img2ply
run_2d_wasure

# ==== 3D surface reconstruction workflow ====
#run_3d_wasure
















