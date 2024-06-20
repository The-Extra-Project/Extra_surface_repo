mkdir -p  ./output/
python3  ./mesh23dtile.py --input_dir  ${PWD}/datas/croco --output_dir ${PWD}/output --meshlab_mode python  --bbox "-40 40 -40 40 -40 40"

# Loop through all OBJ files in the input directory
for obj_file in ${PWD}/output/tiles/*.obj; do
  base_name=$(basename "$obj_file" .obj)  
  obj23dtiles -i "$obj_file"  --b3dm
done


