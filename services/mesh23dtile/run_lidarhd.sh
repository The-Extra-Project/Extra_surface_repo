input_dir=${PWD}/datas/lidar_hd_crop
output_dir=${PWD}/output_lidarhd_crop
coords="635471.0x6856430.0"
mode_proj=0
#mode_proj=1

mkdir -p  ${output_dir}
python3  ./mesh23dtile.py --input_dir  ${input_dir} --output_dir ${output_dir} --meshlab_mode python --coords ${coords} --mode_proj ${mode_proj}


for obj_file in ${output_dir}/tiles/*.obj; do
    filename_we="${obj_file%.*}"
    python3 convert_obj.py --input ${obj_file}  --output ${filename_we}_trans.obj --coords ${coords} 
    echo $filename_without_extension
    obj23dtiles -i "${filename_we}.obj"  --b3dm
    obj23dtiles -i "${filename_we}_trans.obj"  --b3dm
done
case $mode_proj in
    0)
	sed -i 's/\.obj/\.b3dm/g' ${output_dir}/tileset.json
        ;;
    1)
	sed -i 's/\.obj/\_trans.b3dm/g' ${output_dir}/tileset.json
        ;;
esac



