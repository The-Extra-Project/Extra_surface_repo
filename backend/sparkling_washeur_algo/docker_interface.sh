#!/bin/bash 

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )/"

print_fun() 
{
    grep "^function" $0
}
if [ $# -eq 0 ]
then
    echo "No arguments supplied"
    echo "=> functions aviable are : "
    print_fun
    exit 1;
fi

MOUNT_CMD="${MOUNT_CMD} -v /tmp/:/tmp/ -v ${TMP_DIR}:${TMP_DIR}  -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR}"
echo "MOUNT CMD ===> $MOUNT_CMD"

function compile
{
    # Check if env variable file exist
    if [ -f ${DDT_MAIN_DIR}/algo-env.sh ]
    then
	source ${DDT_MAIN_DIR}/algo-env.sh
    else
	echo "ERROR"
	echo "file algo-env.sh not exists, please copy algo-env.sh.conf and set your parameters "
	exit 1
    fi

    echo "starting daemon ..."
    echo "compile"
    DDT_TRAITS="3"
    COMPILE_MODE="Release"
    while getopts "j:t:df" OPTION
    do
	case $OPTION in
	    d)
		COMPILE_MODE="Debug"
		;;
	    j)
		NB_PROC="-j${OPTARG}"
		;;
	    t)
		DDT_TRAITS="${OPTARG}"
		;;
	    f)
		DO_FORMAT="TRUE"
		;;
	esac
    done
    GLOBAL_BUILD_DIR=${DDT_MAIN_DIR_DOCKER}/build/
    KERNEL_BUILD_DIR=${DDT_MAIN_DIR_DOCKER}/build/build-spark-${COMPILE_MODE}-${DDT_TRAITS}/    
    #docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null
    SPARK_BUILD_DIR=${GLOBAL_BUILD_DIR}/spark/
    EXEC_FUN="mkdir -p ${KERNEL_BUILD_DIR}"
    EXEC_FUN="${EXEC_FUN} &&  cd ${KERNEL_BUILD_DIR} && cmake ${DDT_MAIN_DIR_DOCKER}  -DCMAKE_BUILD_TYPE=${COMPILE_MODE} -DDDT_TRAITS=${DDT_TRAITS} && make ${NB_PROC}   "
    if [ ! -z "$DO_FORMAT" ];
    then
	EXEC_FUN="${EXEC_FUN} && apt-get install astyle --assume-yes && cd ${KERNEL_BUILD_DIR} && make format"
    fi
    EXEC_FUN="${EXEC_FUN} &&  cd ${DDT_MAIN_DIR_DOCKER}/services/ && ./build-unix.sh build -b ${KERNEL_BUILD_DIR} -c ${COMPILE_MODE} -t ${DDT_TRAITS}"
    EXEC_FUN="${EXEC_FUN} && mkdir -p ${SPARK_BUILD_DIR} && cp -rf ${DDT_MAIN_DIR_DOCKER}/src/spark/* ${SPARK_BUILD_DIR}  && cd ${SPARK_BUILD_DIR} && ./build-unix.sh "

    ## If inside the docker
    # if [ -f /.dockerenv ] ;
    # then
	eval ${EXEC_FUN}
    # else
	# #${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i "${NAME_IMG_BASE}" -c ${CONTAINER_NAME_COMPILE}
    #fi
}


function run_algo_spark 
{ 
    while getopts "i:f:o:p:s:m:r:b:c:d" OPTION
    do
        case $OPTION in
            i)
                INPUT_DATA_DIR="${OPTARG}"
                ;;
            o)
                OUTPUT_DATA_DIR="${OPTARG}"
                ;;
	    f)
                FILE_SCRIPT="-f ${OPTARG}"
                ;;
            p)
                PARAM_PATH="-p ${OPTARG}"
                ;;
            s)
              	SPARK_CONF="-s ${OPTARG,,}"
		;;
            m)
                MASTER_IP="-m ${OPTARG}"
                ;;
	    c)
                CORE_LOCAL_MACHINE="-c ${OPTARG}"
                ;;
	    b)
                GLOBAL_BUILD_DIR="${OPTARG}"
                ;;	    
	    d)
                CONTAINER_NAME_SHELL="${CONTAINER_NAME_SHELL}-debug"
		DEBUG_MODE=true
		DEBUG_CMD="-d"
                ;;
	    r)
                ALGO_SEED="-r ${OPTARG}"
                ;;
	esac
    done

    MOUNT_CMD="${MOUNT_CMD} -v ${INPUT_DATA_DIR}:${INPUT_DATA_DIR}"
    MOUNT_CMD="${MOUNT_CMD} -v ${OUTPUT_DATA_DIR}:${OUTPUT_DATA_DIR}"
    docker rm -f ${CONTAINER_NAME_SHELL} 2>/dev/null
    EXEC_FUN="cd ${ND_TRI_MAIN_DIR_DOCKER}"

    EXEC_FUN="${EXEC_FUN} ; ${DDT_MAIN_DIR}/src/scala/run_algo_spark.sh  -i ${INPUT_DATA_DIR} -o ${OUTPUT_DATA_DIR}  ${FILE_SCRIPT}  ${PARAM_PATH}  ${SPARK_CONF}  ${MASTER_IP} ${CORE_LOCAL_MACHINE} -b ${GLOBAL_BUILD_DIR} ${ALGO_SEED} ${DEBUG_CMD}"
    
	## If inside the docker
    if [ -f /.dockerenv ] ;
    
	then
	eval ${EXEC_FUN}
	fi
    # else	

	# if [ "$DEBUG_MODE" = true ] ; then
    #         ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL} -d bash
	
	# else
	#     ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
	#     return 0;
	
	# fi
    
}


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

function run_full_pipeline 
{

while [[ "$#" -gt 0 ]]; do
  case $1 in
      --input_dir) INPUT_DIR="${2%/}/"; shift ;;
      --colorize) DO_COLORIZE="TRUE" ;;
      --debug) DEBUG_FLAG="-d" ;;
      --params) PARAMS="${2%/}/"; shift ;;
      --output_dir) OUTPUT_DIR="${2%/}/"; shift ;;
    *) echo "Unknown parameter passed: $1"; usage ;;
  esac
  shift
done



echo "Start processing ${INPUT_DIR} ... "
echo -e "\n-[start preprocesssing]-"

LAZ_INPUT_DIR=${INPUT_DIR}
INPUT_BASE=$(basename "${INPUT_DIR}")
FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"


run_algo_docker
    
echo -e "\n-[start reconstruction]-"
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

if [[ ${DO_LOD} == "TRUE" ]]; then
    echo -e "\n -[Create LODs from tiled mesh]-"
    mkdir -p ${OUTPUT_DIR}/LODs
    ${DDT_MAIN_DIR}/services/mesh23dtile/run.sh --input_dir ${TILE_DIR} --xml_file ${PARAMS} --output_dir ${OUTPUT_DIR}/LODs
fi
run_algo_docker
    
echo -e "\n-[start reconstruction]-"
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

if [[ ${DO_LOD} == "TRUE" ]]; then
    echo -e "\n -[Create LODs from tiled mesh]-"
    mkdir -p ${OUTPUT_DIR}/LODs
    ${DDT_MAIN_DIR}/services/mesh23dtile/run.sh --input_dir ${TILE_DIR} --xml_file ${PARAMS} --output_dir ${OUTPUT_DIR}/LODs
fi

exit 0
}







# function shell # Go inside container
# {
#     ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
# }

$@
exit 0