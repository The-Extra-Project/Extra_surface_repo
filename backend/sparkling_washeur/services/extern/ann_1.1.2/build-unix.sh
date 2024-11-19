#!/bin/bash

function build {
    while getopts "b:" OPTION
    do
    	case $OPTION in
	    b)
		CURRENT_PROJECT_BUILD_DIR="${OPTARG}"
		;;
    	esac
    done

    mkdir -p ${CURRENT_PROJECT_BUILD_DIR}/lib/
    make linux-g++
    cp -rf lib/* ${CURRENT_PROJECT_BUILD_DIR}/lib/
    cp -rf include/ ${CURRENT_PROJECT_BUILD_DIR}/
}

$@
