

#./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco_small/ --input_dir ./datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.3 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_small_CRO --filename ./datas/3d_bench/croco.ply

# OUTPUT_DIR="/home/laurent/shared_spark/tests_outputs/run_3d_croco_eval/"
# mkdir -p $OUTPUT_DIR
# ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir $OUTPUT_DIR --input_dir ./datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 1 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 1 --step full_stack --seed 18696 --label damping_1 --filename ./datas/3d_bench/croco.ply

OUTPUT_DIR="/home/laurent/shared_spark/tests_outputs/run_3d_church_eval/"
mkdir -p $OUTPUT_DIR
./build//build-spark-Release-3/bin/wasure-local-exe --output_dir $OUTPUT_DIR  --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 1 --nb_samples 10 --rat_ray_sample 1 --mode surface --lambda 0 --step church --seed 18696 --label damping_05 --adaptative_scale --filename /home/laurent/shared_spark/inputs/church/preprocessed_vsmall_merged/full_center_01.ply







