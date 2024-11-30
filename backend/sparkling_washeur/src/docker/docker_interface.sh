#!/bin/bash 

export DDT_MAIN_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)

## Check if env variable file exist
if [ -f "${DDT_MAIN_DIR}/algo-env.sh" ]; then
    source ${DDT_MAIN_DIR}/algo-env.sh
else
    echo "ERROR"
    echo "file algo-env.sh not exists, please copy algo-env.sh.conf and set your parameters"
    exit 1
fi


print_fun() {
    grep "^function" $0
}

if [ $# -eq 0 ]; then
    echo "No arguments supplied!"
    echo "=> functions available are : "
    print_fun
    exit 1
fi

MOUNT_CMD="${MOUNT_CMD} -v /tmp/:/tmp/ -v ${TMP_DIR}:${TMP_DIR} -v ${SPARK_SHARED_DIR}:${SPARK_SHARED_DIR} -v ${DDT_MAIN_DIR}:${DDT_MAIN_DIR}"
echo "MOUNT CMD ===> $MOUNT_CMD"


function compile {
    echo -e "\nStarting daemon..."
    echo -e "\nCompiling..."
    DDT_TRAITS="3"
    COMPILE_MODE="Release"
    while getopts "j:t:df" OPTION; do
        case $OPTION in
        d)
            COMPILE_MODE="Debug"
            ;;
        j)
            NB_PROC="${OPTARG}"
            ;;
        t)
            DDT_TRAITS="${OPTARG}"
            ;;
        f)
            DO_FORMAT="TRUE"
            ;;
        esac
    done

    docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null

    SPARK_BUILD_DIR="${GLOBAL_BUILD_DIR}/spark/"
    GLOBAL_BUILD_DIR="${DDT_MAIN_DIR_DOCKER}/build/"
    KERNEL_BUILD_DIR="${GLOBAL_BUILD_DIR}/build-spark-${COMPILE_MODE}-${DDT_TRAITS}/"

    # Compile ddt project at the top level:: ./CMakeLists.txt
    EXEC_FUN="mkdir -p ${KERNEL_BUILD_DIR} && cd ${KERNEL_BUILD_DIR} && \
              cmake ${DDT_MAIN_DIR_DOCKER} -DBUILD_DIR=${KERNEL_BUILD_DIR} -DCMAKE_BUILD_TYPE=${COMPILE_MODE} -DDDT_TRAITS=${DDT_TRAITS} && \
              make -j${NB_PROC} "

    # Compile formatting
    if [ -n "$DO_FORMAT" ]; then
	    EXEC_FUN="${EXEC_FUN} && apt-get install -y astyle && cd ${KERNEL_BUILD_DIR} && make format"
    fi

    # Compile services:: ./services
    EXEC_FUN="${EXEC_FUN} && echo -e '\nBUILDING SERVICES\n' && cd ${DDT_MAIN_DIR_DOCKER}/services/ && \
              ./build-unix.sh build -b ${KERNEL_BUILD_DIR} -c ${COMPILE_MODE} -t ${DDT_TRAITS} -j ${NB_PROC}"

    # Compile spark:: ./src/spark
    EXEC_FUN="${EXEC_FUN} && mkdir -p ${SPARK_BUILD_DIR} && cp -rf ${DDT_MAIN_DIR_DOCKER}/src/spark/* ${SPARK_BUILD_DIR} && \
              cd ${SPARK_BUILD_DIR} && ./build-unix.sh"

    # Eval EXEC_FUN directly or in a docker container depending on the env
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -l "${EXEC_FUN}" -m "${MOUNT_CMD}" -i "${NAME_IMG_BASE}" -c ${CONTAINER_NAME_COMPILE}
}

# Build docker container
function build {
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
        "ddt_img_base_devel")
            docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-base-Ubuntu-devel ${DDT_MAIN_DIR}
            ;;
        "ddt_img_base_multistage")
            docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/src/docker/Dockerfile-base-Ubuntu-multistage ${DDT_MAIN_DIR}
            ;;
        *)
            echo "ERROR NO IMAGE"
            ;;
      esac
}


function kill_container {
    echo "kill all container: ${CONTAINER_NAME_EXAMPLE}"
    docker rm -f ${CONTAINER_NAME_SHELL} 2>/dev/null
    docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null
}


# Run the main pipeline
function run_algo_spark {
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
              	SPARK_CONF="-s ${OPTARG}"
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

    MOUNT_CMD="${MOUNT_CMD} \
               -v ${INPUT_DATA_DIR}:${INPUT_DATA_DIR} \
               -v ${OUTPUT_DATA_DIR}:${OUTPUT_DATA_DIR} \
               -v ${SPARK_EVENTS_DIR}:${SPARK_EVENTS_DIR}"

    echo -e "\nRemoving the shell container pipeline\n"
    docker rm -f ${CONTAINER_NAME_SHELL} 2>/dev/null

    EXEC_FUN="cd ${ND_TRI_MAIN_DIR_DOCKER}; ${DDT_MAIN_DIR}/src/scala/run_algo_spark.sh \
              -i ${INPUT_DATA_DIR} -o ${OUTPUT_DATA_DIR} \
              ${FILE_SCRIPT} ${PARAM_PATH} ${SPARK_CONF} ${MASTER_IP} ${CORE_LOCAL_MACHINE} \
              -b ${GLOBAL_BUILD_DIR} ${ALGO_SEED} ${DEBUG_CMD}"

    if [ "$DEBUG_MODE" = true ]; then
        ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL} -d bash
    else
	    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
	    return 0
    fi
}


# Go inside container
function shell {
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
}


$@

exit 0
