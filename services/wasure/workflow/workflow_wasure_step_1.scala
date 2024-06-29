import sys.process._

import scala.xml._
import scala.concurrent._
import scala.collection.parallel._
import scala.collection.mutable.ListBuffer
import scala.util.Random
import collection.mutable

import java.io._
import java.lang.Double
import java.util.concurrent.Executors
import java.util.Date;
import java.util.Calendar;
import java.text.SimpleDateFormat;
import java.text.DecimalFormat
import java.io.PrintWriter
import java.nio.file.{Paths, Files}
import java.nio.charset.StandardCharsets


import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
import org.apache.hadoop.fs.permission.FsPermission


import org.apache.spark._
import org.apache.spark.graphx._
import org.apache.spark.graphx.PartitionStrategy._
import org.apache.spark.rdd.RDD;
import org.apache.spark.sql.SaveMode
import org.apache.spark.storage.StorageLevel
import org.apache.spark.SparkConf

import spark_ddt.core._
import spark_ddt.util._
import spark_ddt.core.IQlibCore._
import spark_ddt.ddt_algo._
import spark_ddt.wasure._

import sparkle.graph._
import tiling._

import algo_stats._
import dataset_processing._
import bash_funcs._
import geojson_export._
import xml_parsing._
import strings_opt._
import params_parser._
import files_opt._


//=============================================
//==== Configuration and file sysyem init  ====
val conf = new SparkConf().setAppName("DDT")
val fs = FileSystem.get(sc.hadoopConfiguration);

// Checkpoint
val do_checkpoint = false
val checkpoint_dir_string = "/home/laurent/shared_spark/checkpoint/"
val checkpoint_dir_path = new Path(checkpoint_dir_string)
if(do_checkpoint){
  sc.setCheckpointDir(checkpoint_dir_string)
  if (fs.exists(checkpoint_dir_path))
    fs.delete(checkpoint_dir_path, true)
  fs.mkdirs(checkpoint_dir_path,new FsPermission("777"))
}


// Metadata extraction
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
val input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val current_plateform = get_bash_variable("CURRENT_PLATEFORM");
val global_build_dir = get_bash_variable("GLOBAL_BUILD_DIR");
val executor_cores = get_bash_variable("MULTIVAC_EXECUTOR_CORE");
val num_executors = get_bash_variable("MULTIVAC_NUM_EXECUTORS");


// Get Params list from xml
val xml_string = sc.wholeTextFiles(env_xml).collect()(0)._2
var param_list = parse_xml_datasets_string(xml_string)
val df_par = sc.defaultParallelism;
val params_scala = param_list(0) // We only process 1 set of parameter in this workflow

// ===============================================
// ==== Scala and param initialization ===========
// Param scala is mutable, get params set the default value to the collection if it's empty
//  Se for instance the xml documentation / Algorithm params for the effect

// System params
val dim = params_scala.get_param("dim", "2").toInt
//val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-D" + dim.toString)
val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-" + dim.toString)
val build_dir = global_build_dir + "/" + ddt_kernel_dir
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "DISK_ONLY"))

// General Algo params
val bbox = params_scala.get_param("bbox", "")
val do_profile = params_scala.get_param("do_profile", "false").toBoolean;
val do_stats = params_scala.get_param("do_stats", "false").toBoolean;
val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
val regexp_filter = params_scala.get_param("regexp_filter", "");
val max_ppt = params_scala.get_param("max_ppt", "10000").toInt
val do_dump_debug = params_scala.get_param("dump_debug", "false").toBoolean
val ndtree_depth = params_scala.get_param("ndtree_depth", "4").toInt
val nbp =  params_scala.get_param("nbp", "10000").toInt
val datatype =  params_scala.get_param("datatype", "")
val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt
val algo_seed =  params_scala.get_param("algo_seed",scala.util.Random.nextInt(100000).toString);

// Surface reconstruction prarams
val wasure_mode = params_scala.get_param("mode", "-1")
val pscale = params_scala.get_param("pscale", "0.05").toFloat
val nb_samples = params_scala.get_param("nb_samples", "3").toFloat
val rat_ray_sample = params_scala.get_param("rat_ray_sample", "1").toFloat
val min_ppt = params_scala.get_param("min_ppt", "100").toInt
val dst_scale = params_scala.get_param("dst_scale", "-1").toFloat
val lambda = params_scala.get_param("lambda", "4").toFloat
val coef_mult = params_scala.get_param("coef_mult", "1").toFloat
val max_opt_it = params_scala.get_param("max_opt_it", "30").toInt

val main_algo_opt = params_scala.get_param("algo_opt", "seg_lagrange_weight")
val stats_mod_it = params_scala.get_param("stats_mod_it", ((max_opt_it)/2).toString).toInt
val stats_mod_it = 1
// params_scala("dump_mode") = collection.mutable.Set("TRIANGLE_SOUP")
params_scala("dump_mode") = collection.mutable.Set("NONE")
val fmt = new java.text.DecimalFormat("##0.##############")
val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
val datestring = dateFormatter.format(Calendar.getInstance().getTime());
val cur_output_dir ={output_dir  + "outputs"} //sc.applicationId + "_" + datestring + "_"+ params_scala("name").head }



var env_map: scala.collection.immutable.Map[String, String] = Map("" -> "")

val cpp_exec_path = current_plateform.toLowerCase match {
  case _  => build_dir + "/bin/"
}

val python_exec_path = "python3 /home/laurent/code/spark-ddt/services/wasure/python/"

// Set the iq library on
val iq = new IQlibSched(slvl_glob,slvl_loop,env_map)




// Set the c++ command line object
val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
val params_ddt =  set_params(params_new,List(
  ("exec_path", cpp_exec_path + "ddt-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("ech_input","1"),
  ("input_dir",input_dir),
  ("output_dir",cur_output_dir),
  ("min_ppt",params_scala("min_ppt").head),
  ("dump_mode",params_scala("dump_mode").head),
  ("seed",algo_seed)
))


val params_python =  set_params(params_new,List(
  ("exec_path", python_exec_path + "python_stream.py")
))


val params_wasure =  set_params(params_new,List(
  ("exec_path", cpp_exec_path + "wasure-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("lambda",params_scala("lambda").head),
  ("dst_scale",params_scala("dst_scale").head),
  ("pscale",params_scala("pscale").head),
  ("rat_ray_sample",params_scala("rat_ray_sample").head),
  ("nb_samples",params_scala("nb_samples").head),
  ("mode",params_scala("mode").head),
  ("input_dir",input_dir),
  ("output_dir",cur_output_dir),
  ("seed",algo_seed)
))


if(do_dump_debug)
  params_wasure("dump_debug") = collection.mutable.Set("")



val floatFormat = new DecimalFormat("#.###")
val nbt_side = math.pow(2,ndtree_depth)
val tot_nbt = scala.math.pow(nbt_side,dim).toInt;
val nbp_per_tile = nbp/tot_nbt;

val rep_value = 100
var nb_leaf = tot_nbt;
params_scala("rep_value") = collection.mutable.Set(rep_value.toString)
params_ddt("output_dir") = collection.mutable.Set(cur_output_dir)
params_scala("output_dir") = collection.mutable.Set(cur_output_dir)
params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)
params_scala("executor_cores") =  collection.mutable.Set(executor_cores)
params_scala("num_executors") =  collection.mutable.Set(num_executors)
params_ddt("nbt_side") =  collection.mutable.Set(nbt_side.toString)
params_wasure("nbt_side") =  collection.mutable.Set(nbt_side.toString)


println("")
println("=======================================================")
params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))


// python
val python_cmd = params_python.to_command_line

// General c++ commands
val ply2geojson_cmd =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
val tri2geojson_cmd =  set_params(params_ddt, List(("step","tri2geojson"))).to_command_line
val ply2dataset_cmd =  set_params(params_ddt, List(("step","ply2dataset"))).to_command_line
val extract_struct_cmd =  set_params(params_ddt, List(("step","extract_struct"))).to_command_line
val dump_ply_binary_cmd =  set_params(params_ddt, List(("step","dump_ply_binary"),("output_dir", cur_output_dir))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");

// Wausre surface reconstruction commands
val dim_cmd =  set_params(params_wasure, List(("step","dim"))).to_command_line
val dst_cmd =  set_params(params_wasure, List(("step","dst"))).to_command_line
val regularize_slave_focal_cmd =  set_params(params_wasure, List(("step","regularize_slave_focal"))).to_command_line
val regularize_slave_extract_cmd =  set_params(params_wasure, List(("step","regularize_slave_extract"))).to_command_line
val regularize_slave_insert_cmd =  set_params(params_wasure, List(("step","regularize_slave_insert"))).to_command_line
val extract_graph_cmd =  set_params(params_wasure, List(("step","extract_graph"))).to_command_line
val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
val ext_cmd =  set_params(params_wasure, List(("step","extract_surface"))).to_command_line
val tri2geojson_wasure_cmd =  set_params(params_wasure, List(("step","tri2geojson"))).to_command_line
val wasure_ply2geojson_cmd =  set_params(params_wasure, List(("step","ply2geojson"))).to_command_line

// =================================================
// ============  Parsing and init data ===========
var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
var kvrdd_inputs = format_data(
  params_scala,
  params_ddt,
  global_build_dir,
  ddt_main_dir,
  input_dir,
  df_par,
  sc ,
  iq
)


val t0 = System.nanoTime()
params_scala("t0") = collection.mutable.Set(t0.toString)
println("======== Tiling =============")
kvrdd_points = ddt_algo.compute_tiling(kvrdd_inputs,iq,params_ddt,params_scala);
ddt_algo.update_time(params_scala,"tiling");
nb_leaf = params_scala("nb_leaf").head.toInt;

var rep_merge = ((if((nb_leaf) < spark_core_max) spark_core_max else  nb_leaf));
var rep_loop = nb_leaf;
if(ndtree_depth > 8){
  rep_loop = spark_core_max*10;
  rep_merge = spark_core_max*10;
}
params_scala("rep_loop") = collection.mutable.Set(rep_loop.toString)
params_scala("rep_merge") = collection.mutable.Set(rep_merge.toString)


println("======== Dimenstionnality =============")
val res_dim = iq.run_pipe_fun_KValue(
  dim_cmd ++ List("--label", "dim"),
  kvrdd_points, "dim", do_dump = false).persist(slvl_glob)
val kvrdd_dim = iq.get_kvrdd(res_dim,"z");
val kvrdd_simp = iq.get_kvrdd(res_dim,"x").reduceByKey((u,v) => u ::: v,rep_loop);


val nbe = kvrdd_simp.count
// Create a map from 0 to 100 with each number mapping to a random 10-character string


  // Function to generate a random 10-character string
  def randomString(length: Int): String = {
    val chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    val sb = new StringBuilder(length)
    for (_ <- 1 to length) {
      val randomNum = Random.nextInt(chars.length)
      sb.append(chars.charAt(randomNum))
    }
    sb.toString()
  }


val numberToKey: Map[Long, String] = (0 to nbe.toInt).map { num =>
  num.toLong -> randomString(10)
}.toMap


// kvrdd_simp.map(x => (numberToKey(x._1),x._2)).saveAsObjectFile(output_dir + "/kvrdd_simp")
// kvrdd_dim.map(x => (numberToKey(x._1),x._2)).saveAsObjectFile(output_dir + "/kvrdd_dim")

//val hihi = sc.objectFile[spark_ddt.core.IQlibCore.KValue](output_dir + "/kvrdd_dim")
