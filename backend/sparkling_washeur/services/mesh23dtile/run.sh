#!/bin/bash
export CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")/" && pwd )"
# Function to display usage
usage() {
  echo "Usage: $0 --input_dir <input_dir> --xml_file <xml_file> --output_dir <output_dir>"
  exit 1
}

source ${CUR_DIR}/../../algo-env.sh

# Initialize parameters
input_dir=""
xml_file=""
output_dir=""

# Parse command-line options
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --input_dir) input_dir="$2"; shift ;;
    --xml_file) xml_file="$2"; shift ;;
    --output_dir) output_dir="$2"; shift ;;
    *) echo "Unknown parameter passed: $1"; usage ;;
  esac
  shift
done

# Check if all parameters are provided
if [ -z "$input_dir" ] || [ -z "$xml_file" ] || [ -z "$output_dir" ]; then
  echo "Error: Missing required parameters."
  usage
fi

# Check if input_dir is empty
if [ -z "$input_dir" ]; then
  echo "Error: input_dir is empty"
  exit 1
fi

# Check if output_dir is empty
if [ -z "$output_dir" ]; then
  echo "Error: output_dir is empty"
  exit 1
fi

offset=$(cat ${xml_file} | sed -n 's/.*<bbox_ori>\([0-9.]*\)x[0-9.]*:\([0-9.]*\)x[0-9.]*:.*/\1 \2/p')
coords=${offset// /x}
input_crs="2154"
output_crs="4978"

mkdir -p "${output_dir}"

echo "Starting mesh23tile..."
python ${CUR_DIR}/mesh23dtile.py --input_dir ${input_dir} --output_dir ${output_dir} --meshlab_mode python --coords ${coords} --mode_proj 0
echo "Finished mesh23dtile.py"

count=0


### Sequential job 
# for obj_file in "${output_dir}/tiles/"*.obj; do
#   count=$((count + 1))
#   output_tile=${output_dir}/$(basename ${obj_file%.*})
#   echo ${offset}
#   obj-tiler -i "$obj_file" --offset ${offset} --crs_in EPSG:${input_crs} --crs_out EPSG:${output_crs} -o ${output_tile}
# done

process_obj(){
    objf=${1}
    output_tile=${output_dir}/$(basename ${objf%.*})
    echo ${offset}
    obj-tiler -i ${objf} --offset ${offset} --crs_in EPSG:${input_crs} --crs_out EPSG:${output_crs} -o ${output_tile}
}


echo ${NUM_PROCESS}
num_jobs="\j" 
for obj_file in "${output_dir}/tiles/"*.obj; do
    while (( ${num_jobs@P} >= ${NUM_PROCESS} )); do
	wait -n
    done
     process_obj "${obj_file}" & 
done

wait
echo "finalize!"
python3 ${CUR_DIR}/finalize.py ${output_dir}
