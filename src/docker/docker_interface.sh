#!/bin/bash 

export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/../../" && pwd )/"
## Check if env variable file exist
if [ -f ${DDT_MAIN_DIR}/algo-env.sh ]
then
    source ${DDT_MAIN_DIR}/algo-env.sh
else
    echo "ERROR"
    echo "file algo-env.sh not exists, please copy algo-env.sh.conf and set your parameters "
    exit 1
fi

#NO_CACHE="--no-cache"
print_fun() # Show a list of functions
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

MOUNT_CMD="${MOUNT_CMD} -v /tmp/:/tmp/ -v ${TMP_DIR}:${TMP_DIR} -v ${SPARK_SHARED_DIR}:${SPARK_SHARED_DIR} -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR}"
echo "MOUNT CMD ===> $MOUNT_CMD"

function compile # ./docker_interface.sh  Compile [-jx]
{
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
    docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null
    SPARK_BUILD_DIR=${GLOBAL_BUILD_DIR}/spark/
    EXEC_FUN="echo \"start compile\""
    EXEC_FUN="export http_proxy=192.168.4.9 && export http_port=3128"
    EXEC_FUN="${EXEC_FUN} && mkdir -p ${KERNEL_BUILD_DIR}"
    EXEC_FUN="${EXEC_FUN} &&  cd ${KERNEL_BUILD_DIR} && cmake ${DDT_MAIN_DIR_DOCKER}  -DCMAKE_BUILD_TYPE=${COMPILE_MODE} -DDDT_TRAITS=${DDT_TRAITS} && make ${NB_PROC}   "
    if [ ! -z "$DO_FORMAT" ];
    then
	EXEC_FUN="${EXEC_FUN} && apt-get install astyle --assume-yes && cd ${KERNEL_BUILD_DIR} && make format"
    fi
    EXEC_FUN="${EXEC_FUN} &&  cd ${DDT_MAIN_DIR_DOCKER}/services/ && ./build-unix.sh build -b ${KERNEL_BUILD_DIR} -c ${COMPILE_MODE} -t ${DDT_TRAITS}"
    EXEC_FUN="${EXEC_FUN} && mkdir -p ${SPARK_BUILD_DIR} && cp -rf ${DDT_MAIN_DIR_DOCKER}/src/spark/* ${SPARK_BUILD_DIR}  && cd ${SPARK_BUILD_DIR} && ./build-unix.sh "
    
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i "${NAME_IMG_BASE}" -c ${CONTAINER_NAME_COMPILE}
}


function build # Build docker container
{
    echo "name_img_base => ${NAME_IMG_BASE}"
    case ${NAME_IMG_BASE} in
	"ddt_img_base_18_04")
	    docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-base-Ubuntu-18-04 ${DDT_MAIN_DIR}
	    ;;

	"ddt_img_base_21_10")
	    docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-base-Ubuntu-21-10 ${DDT_MAIN_DIR}
	    ;;

	"ddt_img_base_20_04")
	    docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-base-Ubuntu-20-04 ${DDT_MAIN_DIR}
	    ;;
	*)
	    echo "ERROR NO IMAGE"
	    ;;
      esac

}


function build_cnes # Build docker container
{
    docker build ${PROXY_CMD} ${NO_CACHE} -t  ddt_img_cnes -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-cnes ${DDT_MAIN_DIR}
    rm -rf ddt_img_cnes.simg
    singularity build ddt_img_cnes.simg docker-daemon://ddt_img_cnes:latest
    scp -r /home/laurent/code/spark-ddt/ddt_img_cnes.simg caraffl@hal.cnes.fr:~/code/spark-ddt/build/build-spark-Release-3/
}


function kill_container { # ./docker_interface.sh kill_container : kill all container related to wasure
    echo "kill all container: ${CONTAINER_NAME_EXAMPLE}"
    docker rm -f ${CONTAINER_NAME_SHELL} 2>/dev/null
    docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null
}

function kill_container_all { # ./docker_interface.sh kill_container_all : kill all docker container 
    docker stop $(docker ps -a -q) 2>/dev/null
    docker rm -f $(docker ps -a -q) 2>/dev/null
}


function run_algo_spark # Run the main pipeline
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
    if [ "$DEBUG_MODE" = true ] ; then
        ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL} -d bash
    else
	${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
    fi


}


function shell # Go inside container
{
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
}




$@
