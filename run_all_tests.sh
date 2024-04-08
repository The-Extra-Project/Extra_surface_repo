#!/bin/bash

### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
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



### 3D Surface reconstruction 
function run_3d_bench_small
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench_small/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

function run_local
{
    OUTPUT_DIR=${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}
    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "mkdir -p ${OUTPUT_DIR} &&  ${DDT_MAIN_DIR}/build//build-spark-Release-3/bin/wasure-local-exe --output_dir ${OUTPUT_DIR} --input_dir ${DDT_MAIN_DIR}/datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.1 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 10 --step full_stack --seed 18696 --label full_small_CRO --filename ${DDT_MAIN_DIR}/datas/3d_bench_small/croco_small.ply"

}

### 3D Surface reconstruction 
function run_lidarhd_crop
{
    
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR_RAW="${DDT_MAIN_DIR}/datas/lidar_hd_crop/"
    INPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    OUTPUT_DIR="${INPUT_DIR}"
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    FILES="${INPUT_DIR_RAW}/*.laz"

    mkdir -p ${OUTPUT_DIR}
    for ff in ${FILES}
    do
	echo "Processing $ff file..."
	bname=$(basename "$ff")
	PLYFILE="${OUTPUT_DIR}/${bname%.laz}.ply"
	#docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR} -v ${GLOBAL_INPUT_DIR}:${GLOBAL_INPUT_DIR} --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "${DDT_MAIN_DIR}/build/build-spark-Release-3/bin/main_preprocess ${ff} ${PLYFILE} "
	# take action on each file. $f store current file name
    done

    run_algo_docker

}


### 3D Surface reconstruction
#run_local
#run_3d_bench_small
run_lidarhd_crop
