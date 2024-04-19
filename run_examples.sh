#!/bin/bash

### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"


# DEBUG_FLAG="-d"

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
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c ${NUM_PROCESS} -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"

    eval ${CMD}
    echo "run_algo_docker done"
    return 0
}



### Reconstruction algorithm 
function ex_run_ply_mono
{
    OUTPUT_DIR=${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}
    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "mkdir -p ${OUTPUT_DIR} &&  ${DDT_MAIN_DIR}/build//build-spark-Release-3/bin/wasure-local-exe --output_dir ${OUTPUT_DIR} --input_dir ${DDT_MAIN_DIR}/datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.1 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 10 --step full_stack --seed 18696 --label full_small_CRO --filename ${DDT_MAIN_DIR}/datas/3d_bench_small/croco_small.ply"

}

### 3D Surface reconstruction 
function ex_run_ply_tiling
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench_small/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}






### 3D Surface reconstruction 
function ex_run_lidarhd_crop
{


    INPUT_DIR="${DDT_MAIN_DIR}/datas/lidar_hd_crop/"
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    run_algo_docker

    INPUT_DIR=${OUTPUT_DIR}
    PARAMS="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/wasure_metadata_3d_gen.xml"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    run_algo_docker
}


### 3D Surface reconstruction 
function run_lidarhd_tiles
{

    
    INPUT_DIR="${DDT_MAIN_DIR}/datas/lidar_hd_tiles/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    run_algo_docker

    INPUT_DIR=${OUTPUT_DIR}
    PARAMS="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/wasure_metadata_3d_gen.xml"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    run_algo_docker
}

### 3D Surface reconstruction
#ex_run_ply_mono
ex_run_ply_tiling
#ex_run_lidarhd_crop
#run_lidarhd_tiles

