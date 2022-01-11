#./src/docker/docker_interface.sh build_cnes 
scp -r *.sh   caraffl@hal.cnes.fr:~/code/spark-ddt/
scp -r /home/laurent/code/spark-ddt/services/wasure/workflow/* caraffl@hal.cnes.fr:~/code/spark-ddt/services/wasure/workflow/
scp -r /home/laurent/code/spark-ddt/services/pbs/* caraffl@hal.cnes.fr:~/code/spark-ddt/services/pbs/
scp -r /home/laurent/code/spark-ddt/build/build-spark-Release-3/{bin,lib}/ caraffl@hal.cnes.fr:~/code/spark-ddt/build/build-spark-Release-3/
scp -r /home/laurent/code/spark-ddt/build/build-spark-Release-3/*.so caraffl@hal.cnes.fr:~/code/spark-ddt/build/build-spark-Release-3/
scp -r /home/laurent/code/spark-ddt/build/spark/target/scala-2.11/iqlib-spark_2.11-1.0.jar caraffl@hal.cnes.fr:~/code/spark-ddt/build/spark/target/scala-2.11/
#scp -r /home/laurent/code/spark-ddt/ddt_img_cnes.simg caraffl@hal.cnes.fr:~/code/spark-ddt/build/build-spark-Release-3/

