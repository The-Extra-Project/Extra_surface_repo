rm -rf ./datas/eval_dir/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/*.sh /home/lcaraffa/code/spark-ddt/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/services/{ddt,wasure} /home/lcaraffa/code/spark-ddt/services/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/services/wasure/workflow/* /home/lcaraffa/code/spark-ddt/services/wasure/workflow/

scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/build/spark/target/scala-2.11/*.jar /home/lcaraffa/code/spark-ddt/build/spark/target/scala-2.11/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/build/build-spark-Release-D3/{bin,lib,*.so} /home/lcaraffa/code/spark-ddt/build/build-spark-Release-D3/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/build/build-spark-Release-3/{bin,lib,*.so} /home/lcaraffa/code/spark-ddt/build/build-spark-Release-3/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/build/build-spark-Release-D2/{bin,lib,*.so} /home/lcaraffa/code/spark-ddt/build/build-spark-Release-D2/
scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/build/build-spark-Release-2/{bin,lib,*.so} /home/lcaraffa/code/spark-ddt/build/build-spark-Release-2/
#scp -r laurent@caraffa.ddns.net:/home/laurent/code/spark-ddt/datas/ /home/lcaraffa/code/spark-ddt/

KERNEL=build-spark-Release-D2
KERNEL=build-spark-Release-2
KERNEL=build-spark-Release-3
rm -rf /home/lcaraffa/tmp/*
cp -rf /home/lcaraffa/code/spark-ddt/build/${KERNEL}/lib/* /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/code/spark-ddt/build/${KERNEL}/*.so /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/code/spark-ddt/build/${KERNEL}/bin/* /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/lib/* /home/lcaraffa/tmp/

hadoop fs -rm -r /user/lcaraffa/tmp/
hadoop fs -put -f /home/lcaraffa/tmp/ /user/lcaraffa/
