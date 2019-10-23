#!/bin/bash

libs=("ann_1.1.2"  "cimg"  "cut-pursuit"  "graphcut"  "pugixml"  "tinyply")
CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/"

function build {
    while getopts "t:f:j" OPTION
    do
	case $OPTION in
	    t)
		COMPILE_TYPE="${OPTARG}"
		;;
	    f)
		COMPILE_FLAG="${OPTARG}"
		;;
	    j)
		NB_PROC_FLAG="${OPTARG}"
		;;
	esac
    done
    
    for i in ${!libs[@]}; do
	${CUR_DIR}/../scripts/compile.sh build -d ${CUR_DIR}/${libs[i]} -t $COMPILE_TYPE
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    done
}

$@

