#./build//build-spark-Release-D3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco/ --input_dir ./datas/3d_bench/ --dim 3 --bbox -50x50:-50x50:-50x50  --pscale 2 --nb_samples 5 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_stack --filename ./datas/3d_bench_small/croco_small.ply

# ./build//build-spark-Release-D2/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_2d_wasure/ --input_dir ./datas/2d_austin/imgs2/ --dim 2 --bbox 0000x10000:0000x10000  --pscale 20 --nb_samples 5 --mode surface --lambda 0 --step full_stack --seed 18696 --label full_stack --filename ./datas/2d_austin/imgs2/austin2_res.ply

# ./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco/ --input_dir ./datas/3d_bench_small --dim 3 --bbox 0000x10000:0000x10000  --pscale 20 --nb_samples 5 --mode surface --lambda 1 --step full_stack --seed 18696 --label full_stack --filename ./datas/3d_bench/croco.ply



./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco/ --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.05 --nb_samples 10 --rat_ray_sample 1 --mode surface --lambda 1 --step full_stack --seed 18696 --label a1 --filename /home/laurent/shared_spark/inputs/aerial_crop/crop_pro_02.ply
./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco/ --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.05 --nb_samples 10 --rat_ray_sample 1 --mode surface --lambda 0.5 --step full_stack --seed 18696 --label a05 --filename /home/laurent/shared_spark/inputs/aerial_crop/crop_pro_02.ply
./build//build-spark-Release-3/bin/wasure-local-exe --output_dir /home/laurent/shared_spark/tests_outputs/run_3d_croco/ --input_dir /home/laurent/shared_spark/inputs/ --dim 3 --bbox 0000x10000:0000x10000  --pscale 0.05 --nb_samples 10 --rat_ray_sample 1 --mode surface --lambda 2 --step full_stack --seed 18696 --label a2 --filename /home/laurent/shared_spark/inputs/aerial_crop/crop_pro_02.ply






