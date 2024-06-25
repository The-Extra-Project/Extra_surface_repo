#!/bin/bash

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

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
}


function run_lidarhd
{
    INPUT_DIR="/path/to/your/lidar/hd/laz/tile/"
    OUTPUT_DIR="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    run_algo_docker

    INPUT_DIR=${OUTPUT_DIR}
    PARAMS="${DDT_MAIN_DIR}/outputs/${FUNCNAME[0]}/wasure_metadata_3d_gen.xml"
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
    run_algo_docker
}

function run_liste_dalle
{

    # Specify the path to your text file
    file_path="./datas/liste_dalle.txt"

    # Check if the file exists
    if [ ! -f "$file_path" ]; then
	echo "File not found: $file_path"
	exit 1
    fi

    # Loop through each line in the file
    while IFS= read -r line; do
	# Store the current line in a variable (e.g., 'current_line')
	filename=$(basename "${line}")
	echo "$filename"
	INPUT_DIR="${DDT_MAIN_DIR}/outputs_lidarhd/${filename}/"
	mkdir -p ${INPUT_DIR}
	wget -O ${INPUT_DIR}/${filename} ${line}

	OUTPUT_DIR="${INPUT_DIR}"
	FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
	run_algo_docker

	INPUT_DIR=${OUTPUT_DIR}
	PARAMS="${OUTPUT_DIR}/wasure_metadata_3d_gen.xml"
	FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
	run_algo_docker

	
    done < "$file_path"
}

#run_lidarhd
run_liste_dalle
