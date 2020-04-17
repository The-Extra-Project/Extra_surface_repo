rm -rf ./datas/eval_dir/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/*.sh /home/lcaraffa/code/ddt-wasure/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/services/ddt-spark/workflow/* /home/lcaraffa/code/ddt-wasure/services/ddt-spark/workflow/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/services/wasure/workflow/* /home/lcaraffa/code/ddt-wasure/services/wasure/workflow/

scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/build/spark/target/scala-2.11/*.jar /home/lcaraffa/code/ddt-wasure/build/spark/target/scala-2.11/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/build/build-spark-Release-D3/{bin,lib,*.so} /home/lcaraffa/code/ddt-wasure/build/build-spark-Release-D3/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/build/build-spark-Release-3/{bin,lib,*.so} /home/lcaraffa/code/ddt-wasure/build/build-spark-Release-3/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/build/build-spark-Release-D2/{bin,lib,*.so} /home/lcaraffa/code/ddt-wasure/build/build-spark-Release-D2/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/build/build-spark-Release-2/{bin,lib,*.so} /home/lcaraffa/code/ddt-wasure/build/build-spark-Release-2/
scp -r laurent@lcaraffa.ddns.net:/home/laurent/code/ddt-wasure/datas/ /home/lcaraffa/code/ddt-wasure/

KERNEL=build-spark-Release-D2
KERNEL=build-spark-Release-2
KERNEL=build-spark-Release-3
rm -rf /home/lcaraffa/tmp/*
cp -rf /home/lcaraffa/code/ddt-wasure/build/${KERNEL}/lib/* /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/code/ddt-wasure/build/${KERNEL}/*.so /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/code/ddt-wasure/build/${KERNEL}/bin/* /home/lcaraffa/tmp/
cp -rf /home/lcaraffa/lib/* /home/lcaraffa/tmp/

hadoop fs -rm -r /user/lcaraffa/tmp/
hadoop fs -put -f /home/lcaraffa/tmp/ /user/lcaraffa/
hadoop fs -rm -r /user/lcaraffa/datas_xml/*
hadoop fs -put -f /home/lcaraffa/code/ddt-wasure/datas/* /user/lcaraffa/datas_xml/
