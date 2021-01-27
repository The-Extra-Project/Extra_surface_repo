
function run_3d_croco
{
    ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/outputs_local/run_3d_croco/ --input_dir ./datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.03 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_small_CRO --filename ./datas/3d_bench/croco.ply
}

function run_yanis
{
    ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/outputs_local/run_yanis/ --input_dir ./datas/3d_yanis --dim 3 --bbox 0000x100000:0000x100000  --pscale 0.03 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_small_CRO --filename ./datas/3d_yanis/PC3E45_3_LiDAR_sigma_0.3_OptCtr.ply
}


# OUTPUT_DIR="/home/laurent/shared_spark/tests_outputs/run_3d_croco_tessel/"
# mkdir -p $OUTPUT_DIR
# ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir $OUTPUT_DIR --input_dir ./datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 20 --nb_samples 5 --rat_ray_sample 0 --mode surface --lambda 1 --step full_stack --seed 18696 --label damping_1 --filename ./datas/3d_bench/croco.ply


# OUTPUT_DIR="/home/laurent/shared_spark/tests_outputs/run_3d_church_eval/"
# mkdir -p $OUTPUT_DIR
# ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir $OUTPUT_DIR  --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.3 --nb_samples 10 --rat_ray_sample 1 --mode surface --lambda 0 --step church --seed 18696 --label damping_05 --adaptative_scale --filename /home/laurent/shared_spark/inputs/church/preprocessed_small_2_merged/merged_01.ply


function run_aerial
{
    OUTPUT_DIR="/home/laurent/shared_spark/tests_outputs/preprocessed_small_2_merged2_new/"
    # INPUT_FILE="/home/laurent/shared_spark/inputs/church/preprocessed_small_2_merged/merged_01.ply"
    INPUT_FILE="/home/laurent/shared_spark/inputs/church/preprocessed_small_2_merged2/merged4.ply"
    #INPUT_FILE="/home/laurent//shared_spark/inputs/toulouse_pp/Toul1_3_LAMB93_000006.ply"
    mkdir -p $OUTPUT_DIR
    ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir $OUTPUT_DIR  --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.01 --nb_samples 1 --rat_ray_sample 1 --mode surface --lambda 0 --step church --seed 18696 --label confidance --adaptative_scale --filename $INPUT_FILE
}

run_yanis




