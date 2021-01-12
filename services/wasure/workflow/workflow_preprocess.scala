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

import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
import org.apache.hadoop.fs.permission.FsPermission

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

// Iqlib Import
import spark_ddt.core._;
import spark_ddt.util._;
import spark_ddt.core.IQlibCore._;
import spark_ddt.ddt_algo._;
import iqlibflow._;
import spark_ddt.bp_algo._;
import tiling._;

import algo_stats._;
import xml_parsing._;
import dataset_processing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;
import mflow._;
import algo_spark_ddt.bp_algo._;
import geojson_export._;
// Belief propagation
import sparkle.graph._

import collection.mutable
import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;

//=============================================
//==== Configuration and file sysyem init  ====
val conf = new SparkConf().setAppName("DDT")
val fs = FileSystem.get(sc.hadoopConfiguration);

// Metadata extraction
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
val input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val global_build_dir = get_bash_variable("GLOBAL_BUILD_DIR");

// Check if we have
if (output_dir.isEmpty ||  input_dir.isEmpty )
{
  System.err.println("ERROR")
  System.err.println("Bash variable are empy or ")
  System.err.println("File params " + env_xml +  " does not exist")
  System.exit(1)
}

// Get Params list from xml
val param_list = parse_xml_datasets(env_xml)
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
val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "MEMORY_AND_DISK_SER"))

// General Algo params
val bbox = params_scala.get_param("bbox", "")
val do_profile = params_scala.get_param("do_profile", "false").toBoolean;
val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
val regexp_filter = params_scala.get_param("regexp_filter", "");
val max_ppt = params_scala.get_param("max_ppt", "10000").toInt
val ndtree_depth = params_scala.get_param("ndtree_depth", "4").toInt
val nbp =  params_scala.get_param("nbp", "10000").toInt
val datatype =  params_scala.get_param("datatype", "")
val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt
val algo_seed =  params_scala.get_param("algo_seed",scala.util.Random.nextInt(100000).toString);

// Surface reconstruction prarams
val wasure_mode = params_scala.get_param("mode", "surface")
val pscale = params_scala.get_param("pscale", "0").toFloat
val nb_samples = params_scala.get_param("nb_samples", "3").toFloat
val min_ppt = params_scala.get_param("min_ppt", "50").toInt

// Set the iq library on
val iq = new IQlibSched(slvl_glob,slvl_loop)

// Set the c++ command line object
val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
val params_ddt =  set_params(params_new,List(
  ("exec_path", build_dir + "/bin/ddt-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("ech_input","1"),
  ("input_dir",input_dir),
  ("output_dir",output_dir),
  ("min_ppt",params_scala("min_ppt").head),
  ("seed",algo_seed)
))

val params_wasure =  set_params(params_new,List(
  ("exec_path", build_dir + "/bin/wasure-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("lambda",params_scala("lambda").head),
  ("pscale",params_scala("pscale").head),
  ("nb_samples",params_scala("nb_samples").head),
  ("mode",params_scala("mode").head),
  ("input_dir",input_dir),
  ("output_dir",output_dir),
  ("seed",algo_seed)
))

val fmt = new java.text.DecimalFormat("##0.##############")
val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")

fs.mkdirs(new Path( output_dir),new FsPermission("777"))
val nbt_side = math.pow(2,ndtree_depth)
val tot_nbt = scala.math.pow(nbt_side,dim).toInt;
val nbp_per_tile = nbp/tot_nbt;
val rep_value = ((if((tot_nbt) < sc.defaultParallelism) sc.defaultParallelism else  (tot_nbt).toInt))
var nb_leaf = tot_nbt;

params_ddt("output_dir") = collection.mutable.Set(output_dir)
params_scala("output_dir") = collection.mutable.Set(output_dir)
params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)
params_ddt("nbt_side") =  collection.mutable.Set(nbt_side.toString)

println("")
println("=======================================================")
params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))

// General c++ commands
val ply2geojson_cmd =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
val tri2geojson_cmd =  set_params(params_ddt, List(("step","tri2geojson"))).to_command_line
val ply2dataset_cmd =  set_params(params_ddt, List(("step","ply2dataset"))).to_command_line
val extract_struct_cmd =  set_params(params_ddt, List(("step","extract_struct"))).to_command_line
val dump_ply_binary_cmd =  set_params(params_ddt, List(("step","dump_ply_binary"),("output_dir", output_dir))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");

// Wausre surface reconstruction commands
val dim_cmd =  set_params(params_wasure, List(("step","dim"))).to_command_line
val dst_cmd =  set_params(params_wasure, List(("step","dst"))).to_command_line
val extract_graph_cmd =  set_params(params_wasure, List(("step","extract_graph"))).to_command_line
val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
val ext_cmd =  set_params(params_wasure, List(("step","extract_surface"))).to_command_line
val tri2geojson_wasure_cmd =  set_params(params_wasure, List(("step","tri2geojson"))).to_command_line
val wasure_ply2geojson_cmd =  set_params(params_wasure, List(("step","ply2geojson"))).to_command_line




// Starts preprocess
val preprocess_cmd =  set_params(params_wasure, List(("step","preprocess"))).to_command_line
val fs = FileSystem.get(sc.hadoopConfiguration);
val datatype =  params_scala.get_param("datatype", "")
val regexp_filter = params_scala.get_param("regexp_filter", "");
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
var rep_value = df_par.toLong;

val generate_points_cmd =  set_params(params_ddt,  List(("step","generate_points_" + datatype))).to_command_line
val ser2datastruct_cmd =  set_params(params_ddt, List(("step","serialized2datastruct"))).to_command_line
var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
var kvrdd_inputs: RDD[KValue] = sc.parallelize(List((0L,List(""))));
var kvrdd_inputs_struct  : RDD[KValue] = sc.parallelize(List((0L,List(""))));


println("")
println("======== LOAD DATA  file =============")
val ss_reg = regexp_filter.r
val nb_ply = fs.listStatus(new Path(input_dir)).map(x => fs.listStatus(x.getPath)).reduce(_ ++ _).map(_.getPath).filter(
  x => ((x.toString endsWith ".ply")) && ((ss_reg).findFirstIn(x.toString).isDefined)
).size

val ply_input = getListOfFiles(input_dir).filter(
  x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map(
  fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p").repartition(nb_ply)


val struct_inputs = iq.run_pipe_fun_KValue(
  preprocess_cmd ++ List("--label", "struct"),
  kvrdd_inputs
    , "struct", do_dump = false)
struct_inputs.collect
// val struct_inputs = iq.run_pipe_fun_KValue(
//   preprocess_cmd ++ List("--label", "struct"),
//   kvrdd_inputs.filter( x => x._2.head contains "140904P1_000028")
//     , "struct", do_dump = false)
// struct_inputs.collect
// find ./tmp/ -name "*.ply" -exec sed -i 's/scalar_//gI' {} \;


// // =================================================
// // ============  Parsing and init data ===========
// var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
// var kvrdd_inputs = format_data(
//     params_scala,
//     params_ddt,
//     global_build_dir,
//     ddt_main_dir,
//     input_dir,
//     df_par,
//     sc ,
//     iq
//   )




