#!/bin/bash

### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/datas/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"


#DEBUG_FLAG="-d"


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
    #echo $CMD
    exec ${CMD}
}

function run_lidarhd_local
{
    INPUT_DIR="${GLOBAL_INPUT_DIR}/lidar_hd_prepro/"
    OUTPUT_DIR=${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}
    INPUT_FILE="${GLOBAL_INPUT_DIR}/lidar_hd_prepro/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.crop.ply"
    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "mkdir -p ${OUTPUT_DIR} &&  ${DDT_MAIN_DIR}/build//build-spark-Release-3/bin/wasure-local-exe --output_dir ${OUTPUT_DIR} --input_dir ${DDT_MAIN_DIR}/datas/3d_bench_small --dim 3 --bbox  -59x59:-59x59:-33x32  --pscale 0.1 --nb_samples 5 --rat_ray_sample 1 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_small_CRO --filename $INPUT_FILE "

}


### 3D Surface reconstruction 
function run_lidarhd
{

    #DEBUG_FLAG="-d"
    # FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    # INPUT_DIR="${DDT_MAIN_DIR}/datas/lidarhd/"
    # OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"    

    # run_algo_docker
    #docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "${DDT_MAIN_DIR}/build/build-spark-Release-3/bin/main_preprocess ${GLOBAL_INPUT_DIR}/lidar_hd_raw/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.laz  ${GLOBAL_INPUT_DIR}/lidar_hd_full/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.ply"
    
    
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${GLOBAL_INPUT_DIR}/lidar_hd_full/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_final.xml"
    run_algo_docker

}


### 3D Surface reconstruction 
function run_lidarhd_crop
{

    #DEBUG_FLAG="-d"
    # FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    # INPUT_DIR="${DDT_MAIN_DIR}/datas/lidarhd/"
    # OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"    

    INPUT_DIR="${DDT_MAIN_DIR}/datas/lidar_hd/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}_2/"    
    INPUT_FILE="${DDT_MAIN_DIR}/datas/lidar_hd/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.crop.ply"
    
    # run_algo_docker
    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "${DDT_MAIN_DIR}/build/build-spark-Release-3/bin/main_preprocess ${DDT_MAIN_DIR}/datas/lidar_hd/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.crop.laz ${INPUT_FILE}"

    #docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "${DDT_MAIN_DIR}/build/build-spark-Release-3/bin/main_compute_normal ${DDT_MAIN_DIR}/datas/lidar_hd/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.crop-shift.laz ${DDT_MAIN_DIR}/datas/lidar_hd/LHD_FXX_0635_6857_PTS_C_LAMB93_IGN69.copc.normal.ply "


    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "mkdir -p ${OUTPUT_DIR} &&  ${DDT_MAIN_DIR}/build//build-spark-Release-3/bin/wasure-local-exe --output_dir ${OUTPUT_DIR} --input_dir ${DDT_MAIN_DIR}/datas/3d_bench_small --dim 3 --bbox  -59x59:-59x59:-1000x1000  --pscale 0.1 --nb_samples 100 --rat_ray_sample 1 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_small_CRO --filename $INPUT_FILE "
    
    ## Run distributed 
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    # run_algo_docker
}

#run_preprocess
#run_lidarhd_local
#run_lidarhd
run_lidarhd_crop
