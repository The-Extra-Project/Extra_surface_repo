#!/bin/bash

function run_fun () {
    ## If inside the docker
    if [ -f /.dockerenv ] || [ -n "$DOCKER_BUILD_ENV" ]; then
		eval ${BASH_CMD}
    else
		if [[ "$(docker images -q $NAME_IMG 2> /dev/null)" == "" ]]; then
			echo "image $NAME_IMG does not exist... "
			echo "run ./docker_interface.sh build"
			exit 1
		else
			# Ensure container name is set 
			if [ -z "$CONTAINER_NAME" ]; then
				CONTAINER_NAME=$(cat /dev/urandom | tr -cd 'a-f0-9' | head -c 8)
			fi

			# if [ ! -z "$http_proxy" ];
			# then
			# 	PROXY_LINE="-e http_proxy='$http_proxy' -e http_proxy_port='$http_proxy_port' -e http_proxy_ip='$http_proxy_ip'"
			# fi

			# Run the container in the detached mode
			docker run --memory=16g --cpus="2.0" -u $(id -u):$(id -g) --privileged -d -it --name $CONTAINER_NAME $MOUNT_CMD \
				-e COLUMNS="$(tput cols)" -e LINES="$(tput lines)" -e DDT_MAIN_DIR="$DDT_MAIN_DIR_DOCKER" \
				${NAME_IMG}

			# Show the container IP address
			container_ip=$(docker inspect $CONTAINER_NAME | grep IPAddress | sed 's/[^0-9.]*//g')
			echo -e "\nContainer IP: $container_ip"

			echo -e "\n==========================================================="
			echo "****** DOCKER EXEC *******"
			echo "$BASH_CMD"

			# Debug mode configuration
			if [ -z "$BASH_CMD" ] && [ -z "${DEBUG_MODE}" ]; then
				CMD="docker exec -it -u $(id -u):$(id -g) $CONTAINER_NAME /bin/bash"
				echo "==> $CMD"
				eval $CMD
			else
				case "${DEBUG_MODE}" in
					bash | shell)
						DOCKER_EXE="docker exec -it -u $(id -u):$(id -g) $CONTAINER_NAME /bin/bash"
						;;
					scala)
						DOCKER_EXE="docker exec -it -u $(id -u):$(id -g) $CONTAINER_NAME bash -c \"${BASH_CMD}\""
						;;
					*)
						DOCKER_EXE="docker exec $CONTAINER_NAME bash -c \"${BASH_CMD}\""
				esac

				# Run the build command inside the container
				echo -e "\n****** DOCKER RUN ******"
				echo "$DOCKER_EXE"
				echo -e "\n****** TYPE THE FOLLOWING BASH CMD TO START ******"
				echo -e "\n${RED}$BASH_CMD${NC}\n"
				eval $DOCKER_EXE
				rc=$?;
				if [[ $rc != 0 ]]; then
					exit $rc
				else
					return 0
				fi
			fi

			if [ -z "$DETACHED_TRUE" ]; then
				docker rm -f $CONTAINER_NAME
			else
				echo  " ======>> Container $CONTAINER_NAME detached  "
				echo  " kill it with: docker rm -f $CONTAINER_NAME"
			fi
		fi
    fi
}


while getopts "l:m:i:c:d:z" OPTION
do
    case $OPTION in
	l)
	    BASH_CMD="${OPTARG}"
	    ;;
	m)
	    MOUNT_CMD="${OPTARG}"
	    ;;
	i)
	    NAME_IMG="${OPTARG}"
	    ;;
	c)
	    CONTAINER_NAME="${OPTARG}"
	    ;;
	z)
	    DETACHED_TRUE="TRUE"
	    ;;
	d)
	    DEBUG_MODE="${OPTARG}"
	    ;;
    esac
done


if [[ -z ${NAME_IMG} ]]; then
    echo "---- Err : bad args -----"
    echo "$0 -i name_img [-l bash_cmd -m mount_cmd]"
    exit 1
fi

run_fun

exit 0