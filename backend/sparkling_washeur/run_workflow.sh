#!/bin/bash

#source ~/anaconda3/etc/profile.d/conda.sh
#conda activate mesh23Dtile
### Start workflow in local mode

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
#source ${DDT_MAIN_DIR}/algo-env.sh
BUILDS_DIR="${DDT_MAIN_DIR}/build/"
DO_COLORIZE="TRUE"
mkdir -p  /tmp/spark-events

# Function to display usage
usage() {
  echo "Usage: $0 --input_dir <input_dir>  --output_dir <output_dir>"
  exit 1
}

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
    mkdir -p ${OUTPUT_DIR}
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c ${NUM_PROCESS} -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    eval ${CMD}
    return 0
}

# Parse command-line options
while [[ "$#" -gt 0 ]]; do
  case $1 in
      --input_dir) INPUT_DIR="${2%/}/"; shift ;;
      --colorize) DO_COLORIZE="TRUE" ;;
      --debug) DEBUG_FLAG="-d" ;;
      --output_dir) OUTPUT_DIR="${2%/}/"; shift ;;
    *) echo "Unknown parameter passed: $1"; usage ;;
  esac
  shift
done

echo "do_colorize:${DO_COLORIZE}"

echo "Start processing ${INPUT_DIR} "
echo -e "\n\n\n ---[run distributed algorithm laz file with preprocessing]---"
echo -e "\n -[start preprocesssing]-"


LAZ_INPUT_DIR=${INPUT_DIR}
INPUT_BASE=$(basename "${INPUT_DIR}")
PARAMS="${INPUT_DIR}/wasure_metadata.xml"
FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"

run_algo_docker
    
echo -e "\n -[start reconstruction]-"
INPUT_DIR=${OUTPUT_DIR}
PARAMS="${OUTPUT_DIR}/wasure_metadata_3d_gen.xml"
FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure.scala"
run_algo_docker

CURRENT_CONDA_ENV=$(conda info --envs | grep '*' | awk '{print $1}') 


TILE_DIR=${OUTPUT_DIR}/outputs/tiles/
if [[ ${DO_COLORIZE} == "TRUE" ]]; then
    TILE_DIR=${OUTPUT_DIR}/colorized_tiles
    ${DDT_MAIN_DIR}/services/colorize/colorize.sh --input_dir ${LAZ_INPUT_DIR} --output_dir ${OUTPUT_DIR}
fi

echo -e "\n -[Create LODs from tiled mesh]-"
mkdir -p ${OUTPUT_DIR}/LODs
${DDT_MAIN_DIR}/services/mesh23dtile/run.sh --input_dir ${TILE_DIR} --xml_file ${PARAMS} --output_dir ${OUTPUT_DIR}/LODs

exit 0
