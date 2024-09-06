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

function build 
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

    echo "name_img_base => ${NAME_IMG_BASE}"
    if [ ! -z ${HTTP_PROXY} ] 
    then
	PROXY_CMD=" --build-arg HTTP_PROXY=${HTTP_PROXY} --build-arg HTTPS_PROXY=${HTTPS_PROXY} "
    fi
    case ${NAME_IMG_BASE} in
	"ddt_img_base_devel")
	    docker build ${PROXY_CMD} ${NO_CACHE} -t  ${NAME_IMG_BASE} -f ${DDT_MAIN_DIR}/Dockerfile.spark ${DDT_MAIN_DIR}
	    ;;
	*)
	    echo "ERROR NO IMAGE"
	    ;;
      esac
}



function run_funcs () {

    ## If inside the docker
    if [ -f /.dockerenv ] ;
    then
	eval ${BASH_CMD}
    else
	if [[ "$(docker images -q $NAME_IMG 2> /dev/null)" == "" ]]; then
	    echo "image $NAME_IMG does not exists... "
	    echo "do ./docker.sh build_compile"
	    exit 1
	else
	    if [ -z "$CONTAINER_NAME" ];
	    then
		CONTAINER_NAME=$(cat /dev/urandom | tr -cd 'a-f0-9' | head -c 8)
	    fi

	    if [ ! -z "$http_proxy" ];
	    then
	    	PROXY_LINE="-e http_proxy='$http_proxy' -e http_proxy_port='$http_proxy_port'  -e http_proxy_ip='$http_proxy_ip'"
	    fi
	    CMD="docker run  -d $MOUNT_CMD -u 0 --cap-add SYS_ADMIN  --privileged --net host $PROXY_LINE -e DDT_MAIN_DIR='$DDT_MAIN_DIR_DOCKER' -e COLUMNS="`tput cols`" -e LINES="`tput lines`" --name $CONTAINER_NAME  -ti ${NAME_IMG}"
	    eval $CMD 
	    container_ip=$(docker inspect $CONTAINER_NAME | grep IPAddress | sed 's/[^0-9.]*//g')
	    echo "ip:$container_ip"
	    echo ""
	    echo "==========================================================="
	    echo "====> DOCKER EXEC :"
	    echo "$CMD"
	    if [ -z "$BASH_CMD" ] && [ -z "${DEBUG_MODE}" ];
	    then
		CMD="docker exec -i -t -u 0 $CONTAINER_NAME /bin/bash"
		echo "==> $CMD"
		eval $CMD
	    else
		case "${DEBUG_MODE}" in
		    bash | shell)
			DOCKER_EXE="docker exec -i -t -u 0 $CONTAINER_NAME /bin/bash"
			;;
		    scala)
			DOCKER_EXE="docker exec -i -t  -u 0  $CONTAINER_NAME  bash -c \"${BASH_CMD}\""    
			;;
		    *)
			DOCKER_EXE="docker exec $CONTAINER_NAME  bash -c \"${BASH_CMD}\""
		esac

		echo "====> DOCKER RUN :"
		echo "$DOCKER_EXE"
		echo ""
		echo "## =======> TYPE THE FOLLOWING BASH CMD TO START <======="
		echo "${RED}$BASH_CMD ${NC}"
		echo ""
		eval $DOCKER_EXE
		rc=$?;
		if [[ $rc != 0 ]];
		then
		    exit $rc;
		else
		    return 0;
		fi
	    fi
	    if [ -z "$DETACHED_TRUE" ];
	    then
		docker rm -f $CONTAINER_NAME
	    else
		echo  " ======>> Container $CONTAINER_NAME detached  "
		echo  " kill it with: docker rm -f $CONTAINER_NAME"
	    fi
	fi
    fi

}





function kill_container { 
    echo "kill all container: ${CONTAINER_NAME_EXAMPLE}"
    docker rm -f ${CONTAINER_NAME_SHELL} 2>/dev/null
    docker rm -f ${CONTAINER_NAME_COMPILE} 2>/dev/null
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
    ## If inside the docker
    if [ -f /.dockerenv ] ;
    then
	eval ${EXEC_FUN}
    else
	if [ "$DEBUG_MODE" = true ] ; then
            ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL} -d bash
	else
	    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
	    return 0;
	fi
    fi
}

function shell # Go inside container
{
    ${DDT_MAIN_DIR}/src/docker/run_bash_docker.sh -m "${MOUNT_CMD}" -l "${EXEC_FUN}" -i ${NAME_IMG_BASE} -c ${CONTAINER_NAME_SHELL}
}
$@

exit 0
