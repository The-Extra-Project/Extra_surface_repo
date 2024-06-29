#!/bin/bash


### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ./outputs/

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
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c ${NUM_PROCESS} -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    eval ${CMD}
    return 0
}



### Reconstruction algorithm 
function ex_run_ply_mono
{
    echo "monothread surface reconstruction on toy example..."
    OUTPUT_DIR=${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}
    docker run  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR}  -u 0  --rm -it --shm-size=12gb ${NAME_IMG_BASE} /bin/bash -c "mkdir -p ${OUTPUT_DIR} &&  ${DDT_MAIN_DIR}/build//build-spark-Release-3/bin/wasure-local-exe --output_dir ${OUTPUT_DIR} --input_dir ${DDT_MAIN_DIR}/datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.1 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 10 --step full_stack --seed 18696 --label full_small_CRO --filename ${DDT_MAIN_DIR}/datas/3d_bench_small/croco_small.ply"
    echo ""


}


### 3D Surface reconstruction 
function ex_run_ply_tiling
{
    echo "run distributed algorithm on toy example..."
    echo 
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench_small/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}



### 3D Surface reconstruction 
function run_example
{
    if [ -z "${INPUT_DIR}" ]; then
	echo "INPUT_DIR is not defined or is empty."
	return 1
    else
	echo "Start processing ${INPUT_DIR} "
    fi

    echo -e "\n\n\n ---[run distributed algorithm laz file with preprocessing]---"
    echo -e "\n -[start preprocesssing]-"
    INPUT_BASE=$(basename "${INPUT_DIR}")
    PARAMS="${INPUT_DIR}/wasure_metadata.xml"
    OUTPUT_DIR_1="${DDT_MAIN_DIR}/outputs/${INPUT_BASE}_1/"
    OUTPUT_DIR_2="${DDT_MAIN_DIR}/outputs/${INPUT_BASE}_2/"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    OUTPUT_DIR=${OUTPUT_DIR_1}
    #run_algo_docker
    echo "${OUTPUT_DIR_1}"

    for file in "${OUTPUT_DIR_1}"/*.ply; do
	if [ -e "$file" ]; then
            base_name=$(basename "$file" .ply)
            mkdir -p "${OUTPUT_DIR_2}/${base_name}"
            cp "$file" "${OUTPUT_DIR_2}/${base_name}/"
	    cp "${OUTPUT_DIR_1}/wasure_metadata_3d_gen.xml" "${OUTPUT_DIR_2}/${base_name}/"
	fi
    done

    PARAMS="${OUTPUT_DIR_1}/wasure_metadata_3d_gen.xml"
    for dd in "${OUTPUT_DIR_2}"/*; do
	echo "dd=>$dd"
	INPUT_DIR=${dd}
	OUTPUT_DIR=${dd}
	FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_step_1.scala"
	#run_algo_docker
    done


    echo -e "\n -[start reconstruction]-"
    INPUT_DIR=${OUTPUT_DIR_2}
    OUTPUT_DIR=${OUTPUT_DIR_2}
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_step_2.scala"
    run_algo_docker
    
}


INPUT_DIR="${DDT_MAIN_DIR}/datas/lidar_hd_crop_w/"
run_example




