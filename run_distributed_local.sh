#!/bin/bash


### Start workflow in local mode 
export DDT_MAIN_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
source ${DDT_MAIN_DIR}/algo-env.sh
GLOBAL_OUTPUT_DIR="${SPARK_SHARED_DIR}/outputs/"
GLOBAL_INPUT_DIR="${SPARK_SHARED_DIR}/datas/"
BUILDS_DIR="${DDT_MAIN_DIR}/build/"

mkdir -p ${GLOBAL_OUTPUT_DIR}
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
    #    export CURRENT_PLATEFORM="singularity"
    CMD="${DDT_MAIN_DIR}/src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c 4 -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
    #echo $CMD
    exec ${CMD}
}



### EVAL BENCHMARK
function run_full_eval
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    FILES="/mnt/samsung_T5b/bench_pts_processed/*"
    OUTPUT="/mnt/samsung_T5b/bench_pts_meshed/"
    PARAMS="${DDT_MAIN_DIR}/datas/3d_bench/wasure_bench.xml"
    for ff in $FILES
    do
	bname="$(basename -- $ff)"
	INPUT_DIR="${ff}/"
	OUTPUT_DIR="${OUTPUT}${bname}/"
	echo $OUTPUT_DIR
	run_algo_docker
	#mkdir -p ${OUTPUT_DIR}
    done
}

### Evaluation FIG 1 
function run_3d_evaluation_tiling
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"

    FILES="${INPUT_DIR}/wasure_metadata_3d_octree*.xml"
    for f in $FILES
    do
	PARAMS="${f}"
	run_algo_docker
    done
    
 #   run_algo_docker
}
function run_3d_evaluation_coef_mult
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_eval_1.xml"
    run_algo_docker
}



### 3D Surface reconstruction 
function run_3d_bench_raw
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"

    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}-files/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_files.xml"
    run_algo_docker
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}-stream/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_plystream.xml"
    #run_algo_docker

}

function run_3d_bench_preprocess
{
    DEBUG_FLAG=""
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    #run_algo_docker

    DEBUG_FLAG=""
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${OUTPUT_DIR}"
    OUTPUT_DIR="${OUTPUT_DIR}"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d_gen.xml"
    run_algo_docker
}


### 3D Surface reconstruction 
function run_3d_yanis
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench/pts1/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}





### 3D Surface reconstruction 
function run_3d_yanis_bench
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"

    # INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench_full/Strasbourg/PC3E44_3/OC/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker

    # INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench_full/Strasbourg/PC3E45_3/OC/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker

    # INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench_full/Strasbourg/PC3E47_3/OC/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker

    # INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench_full/garage/OC/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker


    
    ## Train
    # INPUT_DIR="${GLOBAL_INPUT_DIR}/yanis_bench_full/Evaluation/train/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker


    # INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/Strasbourg/PC3E45_3/OC/"
    # OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/yanis_bench_full/PC3E45_3/"
    # PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    # run_algo_docker

    OUTPUT_DIR_BENCH="${GLOBAL_OUTPUT_DIR}/yanis_bench_2022_05_19/"
    
    echo "$1"
    case $1 in
	1)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/Strasbourg/PC3E44_3/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/PC3E44_3/"
	    ;;
	2)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/Strasbourg/PC3E47_3/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/PC3E47_3/"
	    ;;
	3)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/atelier/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/atelier/"
	    ;;
	4)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/exterieur/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/exterieur/"
	    ;;
	5)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/garage/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/garage/"
	    ;;
	6)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/ETH3D/pipes/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/pipe/"
	    ;;
	7)
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/ETH3D/courtyard/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/courtyard/"
	    ;;
	8)    
	    INPUT_DIR="/home/laurent/shared_spark/datas/yanis_bench_full/ETH3D/terrace/OC"
	    OUTPUT_DIR="${OUTPUT_DIR_BENCH}/terrace/"
	    ;;
    esac
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}


### 3D Surface reconstruction 
function run_3d_lidarhd
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${GLOBAL_INPUT_DIR}/lidarHD/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}



### 3D Surface reconstruction 
function run_3d_bench_small
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${DDT_MAIN_DIR}/datas/3d_bench_small/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}

 ### 3D Surface reconstruction                                                                                                                                                                                                                
 function run_3d_toulouse
 {
     FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
     INPUT_DIR="${GLOBAL_INPUT_DIR}/toulouse_pp/"
     OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
     PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
     run_algo_docker
 }

 ### 3D Surface reconstruction                                                                                                                                                                                                                
 function run_aerial_las
 {
     FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
     INPUT_DIR="/home/laurent/shared_spark/datas/toulouse_aerial_focal_0_ply_stream/"
     OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
     PARAMS="${INPUT_DIR}/wasure_metadata_3d_gen.xml"
     run_algo_docker
 }

 
function run_3d_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${GLOBAL_INPUT_DIR}/toulouse_church/preprocessed_small_2/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}


 
# format data
function preprocess_data
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="/home/laurent/shared_spark/datas/toulouse_aerial_focal_2_ply_bin/"
    OUTPUT_DIR="/home/laurent/shared_spark/datas/toulouse_aerial_focal_2_ply_stream/"
    run_algo_docker    
}

# format data
function preprocess_toulouse
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
    INPUT_DIR="/mnt/samsung_T5b/Toulouse_cc_3"
    OUTPUT_DIR="/mnt/samsung_T5b/Toulouse_pp_3"

    run_algo_docker
}


function run_3d_church
{
    FILE_SCRIPT="${DDT_MAIN_DIR}/services/wasure/workflow/workflow_wasure_generic.scala"
    INPUT_DIR="${GLOBAL_INPUT_DIR}/toulouse_church/preprocessed_small_2/"
    OUTPUT_DIR="${GLOBAL_OUTPUT_DIR}/${FUNCNAME[0]}/"
    PARAMS="${INPUT_DIR}/wasure_metadata_3d.xml"
    run_algo_docker
}



#run_3d_lidarhd

#run_3d_bench_raw
# run_3d_yanis_bench $1

# ==== surface reconstruction workflow ====
#preprocess_data
#preprocess_toulouse

# Evaluation
#run_3d_evaluation_tiling
#run_3d_evaluation_coef_mult
#run_full_eval

### 3D
run_3d_bench_small
#run_3d_bench_preprocess

#run_3d_yanis_2

#run_aerial
#run_aerial_las
#run_3d_church
#run_3d_bench_preprocessed
#run_3d_bench_small
#run_3d_toulouse






















