#!/bin/bash

### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"


DEBUG_FLAG="-d"


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
    #    export CURRENT_PLATEFORM="singularity"
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c 4 -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    #echo $CMD
    exec ${CMD}
}



### 3D Surface reconstruction 
function run_lidarhd
{
    # DEBUG_FLAG=""
    # FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    # INPUT_DIR="${DDT_MAIN_DIR}/datas/lidarhd/"
    # OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"    

    # run_algo_docker
    echo "${DDT_MAIN_DIR}/build/build-spark-Release-3/bin/main_preprocess ${GLOBAL_INPUT_DIR}/Semis_2021_1226_6202_LA93_IGN78.laz ${GLOBAL_INPUT_DIR}/Semis_2021_1226_6202_LA93_IGN78.ply "
    
    
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/lidarhd_ply/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

#run_preprocess
run_lidarhd
