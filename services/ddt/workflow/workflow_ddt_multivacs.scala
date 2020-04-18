import sys.process._
import scala.io.Source
import java.io._
import scala.xml._
import java.lang.Double
import scala.concurrent._
import scala.collection.parallel._
import scala.collection.mutable.ListBuffer

import java.util.concurrent.Executors
import java.util.Date;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import java.io.PrintWriter
import java.nio.file.{Paths, Files}
import java.nio.charset.StandardCharsets
// Spark Import
import org.apache.spark._;
import org.apache.spark.graphx._;
import org.apache.spark.graphx.PartitionStrategy._
import org.apache.spark.rdd.RDD;
import org.apache.spark.sql.SaveMode
import org.apache.spark.storage.StorageLevel
import org.apache.spark.SparkConf
import collection.mutable
import org.apache.hadoop.fs.FileSystem
import org.apache.spark.SparkContext
import scala.collection.mutable.ListBuffer
import java.nio.file.{ Paths, Files }
import org.apache.hadoop.fs.Path
import org.apache.hadoop.fs.permission.FsPermission
import org.apache.spark.HashPartitioner
// Iqlib Import
import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import algo_stats._;
import iqlib_algo._
import tiling._

import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;


val fs = FileSystem.get(sc.hadoopConfiguration);
val checkpoint_dir_string = "hdfs:/user/lcaraffa/checkpoint/"
val checkpoint_dir_path = new Path(checkpoint_dir_string)

// =========  Metadata extraction  ==========
// Bash varaibles
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
var input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH").replaceAll("//", "/");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val hdfs_files_dir = get_bash_variable("HDFS_FILES_DIR");
val global_build_dir = get_bash_variable("GLOBAL_BUILD_DIR");
if (env_xml.isEmpty || ddt_main_dir.isEmpty || input_dir.isEmpty) {
  println("Bash variable are empy, please set it")
  System.exit(1)
}

// ============= Parse Algo params ================
val xml_string = sc.wholeTextFiles(env_xml).collect()(0)._2
var param_list = parse_xml_datasets_string(xml_string)
if (param_list.size == 0){
  println("param_list empty")
  System.exit(1)
}

// ===== Quick and dirty hack to find where are stored on the SPARK/HADOOP cluster the library for the  c++ =====
val exec_path_list = List("ddt-stream-exe");
val lib_list = List(
  "ddt-stream-exe","libboost_system.so.1.67.0",
  "libboost_filesystem.so.1.67.0","libCGAL.so.13",
  "libddt.so","libstdc++.so.6","libdouble-conversion.a"
)
val params_scala = param_list.head

// ============= Spark context ================
// Init spark spark conext
val df_par = sc.defaultParallelism;

//conf.set("spark.default.parallelism",sc.defaultParallelism.toString)
//sc.stop()
var iqsc = sc ;//new SparkContext(conf)
var conf = iqsc.getConf
println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)
val iqsc_default_par = iqsc.defaultParallelism;

for(params_scala <- param_list){
  if( params_scala.exists(List("dim","bbox")) &&
    (params_scala.get_param("do_process", "false").toBoolean)){


        // ============  Init data ===========
    var kvrdd_points: RDD[KValue] = iqsc.parallelize(List((0L,List(""))));
    val kvrdd_log_list : ListBuffer[RDD[KValue]] = ListBuffer()


    // ========================================================= //
    // iqsc.stop()
    // iqsc = new SparkContext(conf)


    var inc_dir = iqsc.parallelize(List("")).map(
  x => (SparkFiles.get("libCGAL.so.13").split("/").dropRight(1).reduce(_ ++ "/" ++ _)).dropRight(1)
).collect()(0).split("_").dropRight(1).reduce(_ ++ "_" ++ _) + "_"
    var inc_dir_list = (1 to 100).map(inc_dir  + "%06d".format(_)).reduce(_ ++ ":" ++ _)
    exec_path_list.map(x => iqsc.addFile(hdfs_files_dir + x))
    lib_list.map(x => iqsc.addFile(hdfs_files_dir + x))
    println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)
    var env_map_multivacs = Map(
      "CLASSPATH" -> sys.env("CLASSPATH"),
      "LD_LIBRARY_PATH" -> (inc_dir_list + ":/usr/lib/jvm/java-8-oracle/jre/lib/amd64/server:/opt/cloudera/parcels/CDH/lib/")
    )


    val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
    val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "MEMORY_AND_DISK"))
    val iq = new IQlibSched(slvl_glob ,slvl_loop,env_map_multivacs)


    val (kvrdd_inputs,params_cpp) =  generate_data(
      params_scala,
      "./",
      ddt_main_dir,
      input_dir,
      df_par,
      output_dir,
      iqsc,
        iq
    )


    val datatype = params_scala("datatype").head;
    val plot_lvl = params_scala("plot_lvl").head.toInt;




    // ======================================================= //
    val t0 = System.nanoTime()
    params_scala("t0") = collection.mutable.Set(t0.toString)

    println("======== Tiling =============")
    kvrdd_points = ddt_algo.compute_tiling(kvrdd_inputs,iq,params_cpp,params_scala);


    val nb_leaf = params_scala("nb_leaf").head.toInt;
    val dump_mode = params_scala("dump_mode").head.toInt;
    val cur_output_dir = params_scala("output_dir").head;
    val rep_merge =   nb_leaf;
    var rep_loop = nb_leaf;
    val nb_executor = sc.getConf.getInt("spark.executor.instances", -1)
    params_scala("rep_loop") = collection.mutable.Set(rep_loop.toString)
    params_scala("rep_merge") = collection.mutable.Set(rep_merge.toString)
    params_scala("num-executors") = collection.mutable.Set(nb_executor.toString)

    println("======== START DDT =============")
    val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(
      kvrdd_points = kvrdd_points,
      iq = iq,
      params_cpp = params_cpp,
      params_scala = params_scala
    );
    println("======== DDT DONE =============")



    if(dump_mode > 0 && false){
      fs.listStatus(new Path(cur_output_dir)).filter(
        dd => (dd.isDirectory)).map(
        ss => fs.listStatus(ss.getPath)).reduce(_ ++ _).filter(
        xx => (xx.getPath.toString contains "part-")).map(
        ff => fs.rename(ff.getPath, new Path(ff.getPath.toString + ".ply"))
      )
    }


    graph_tri.vertices.count
    val t1 = System.nanoTime()
    println("======== algo done =============")
    val scala_time = ((t1 - t0)/1000000000.0);
    println("scala_time_full:" + scala_time.toString)

    // + iqsc.applicationId > spark_core_max

    params_scala("availableProcessors") = collection.mutable.Set(java.lang.Runtime.getRuntime.availableProcessors.toString)
    params_scala("scala_time_full") = collection.mutable.Set(scala_time.toString)
    params_scala("Parallelism_lvl") = collection.mutable.Set((iqsc.getExecutorMemoryStatus.size -1).toString)
    val kvrdd_stats = iqsc.parallelize(List(
      (0L,List("l 1 0 s tot_scala4:" + scala_time))
    )) //union iq.get_kvrdd(raw_inputs, "l",txt="pts")
      dump_stats(kvrdd_stats,cur_output_dir + ".stats.csv" ,sc);
    dump_json(params_scala,cur_output_dir + ".params.json",sc);
    System.exit(0)
  }
}
