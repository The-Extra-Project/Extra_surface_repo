#!/bin/bash


# Function to display usage
usage() {
  echo "Usage: $0 --list_files txt_file  --output_dir <output_dir>"
  exit 1
}

# Parse command-line options
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --list_files) LIST_FILES="$2"; shift ;;
    --output_dir) OUTPUT_DIR="$2"; shift ;;
    *) echo "Unknown parameter passed: $1"; usage ;;
  esac
  shift
done

while IFS= read -r line; do
    filename=$(basename "${line}")
    wget -O ${OUTPUT_DIR}/${filename} ${line}

    ./run_workflow.sh --input_dir ${OUTPUT_DIR} --output_dir ${OUTPUT_DIR}
	
done < "${LIST_FILES}"

return 0

 
