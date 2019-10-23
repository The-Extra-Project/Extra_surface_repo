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

// Iqlib Import
import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import algo_stats._;
import iqlib_algo._

import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;


val conf = new SparkConf().setAppName("DDT")

// ===== Metadata extraction  ==========
// Bash varaibles
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
val input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val algo_seed = get_bash_variable("ALGO_SEED",scala.util.Random.nextInt(100000).toString);
val build_dir = get_bash_variable("GLOBAL_BUILD_DIR");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");

if (output_dir.isEmpty ||
  input_dir.isEmpty ) {
  println("Bash variable are empy, please set it")
  System.exit(1)
}

// ===== Algo params extraction / struct def  ==========
// Xml stuff
val input_label = "pts_generated"
val dim = get_param_from_xml(env_xml, "dim", "2").toInt
val bbox = get_param_from_xml(env_xml, "bbox", "")
val do_tests = get_param_from_xml(env_xml, "do_tests", "true").toBoolean;
val do_plot = get_param_from_xml(env_xml, "do_plot", "true").toBoolean;
val do_generate = get_param_from_xml(env_xml, "do_generate", "false").toBoolean;
val do_stream = get_param_from_xml(env_xml, "do_stream", "false").toBoolean;
val regexp_filter = get_param_from_xml(env_xml, "regexp_filter", "");

val nbp_list  = get_list_param_from_xml(env_xml, "nbp", "10000").map(_.toInt);
val nbt_list = get_list_param_from_xml(env_xml, "nbt_side", "3").map(_.toInt);


//val slvl: StorageLevel = StorageLevel.MEMORY_AND_DISK_
//val slvl: StorageLevel = StorageLevel.OFF_HEAP
val slvl: StorageLevel = StorageLevel.DISK_ONLY
val iq = new IQlibSched(slvl)


var params_ddt = Map(
  "exec_path" -> List(build_dir + "/bin/ddt-stream-exe" ),
  "step" -> List(""),
  "dim" -> List(dim.toString),
  "ech_input" -> List("1"),
  "input_dir" -> List(input_dir),
  "output_dir" -> List(output_dir),
  "seed" -> List(algo_seed)
);


if(!bbox.isEmpty){
  params_ddt = params_ddt + ("bbox" -> List(bbox) )
}

if(do_stream){
  params_ddt = params_ddt + ("do_stream" -> List("") )
}


val nbp = nbp_list.head
val nbt_side = nbt_list.head

params_ddt = params_ddt + ("nbt_side" -> List(nbt_side.toString) )
val generate_points_fun =  set_params(params_ddt,  List(("step","generate_points"))).to_command_line
val get_bbox_points_fun =  set_params(params_ddt,  List(("step","get_bbox_points"))).to_command_line
val extract_tri_vrt_fun =  set_params(params_ddt, List(("step","extract_tri_vrt"))).to_command_line
val ply2geojson_fun =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
val tests_fun =  set_params(params_ddt, List(("step","tests"))).to_command_line
val extract_tile_vrt_fun =  set_params(params_ddt, List(("step","extract_tile_vrt"))).to_command_line
val extract_tile_json_fun =  set_params(params_ddt, List(("step","extract_tile_json"))).to_command_line
val tile_cmd =  set_params(params_ddt, List(("step","tile_ply"),("output_dir", output_dir))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");


// ============  Init data ===========

var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
val kvrdd_log_list : ListBuffer[RDD[KValue]] = ListBuffer()



println("")
println("======== GENERATE DATA =============")
val input_rdd_raw: RDD[KValue] = sc.parallelize(List.range(0, math.pow(nbt_side, dim).toInt).map(
  x => (x.toLong, List("")))).repartition(30)
val inputs_points = iq.run_pipe_fun_KValue(
  generate_points_fun ++ List("--label","lab2","--nbp",nbp.toString),
  input_rdd_raw, "generate_points", do_dump = false)
kvrdd_points = iq.get_kvrdd(inputs_points, "p").reduceByKey(_ ++ _)
if(nbp <= 10000){
  val rdd_json_input_ply = iq.run_pipe_fun_KValue(
    ply2geojson_fun ++ List("--label", "generated"),
    kvrdd_points, "extract_tri_vrt_final", do_dump = false)
  rdd_json_input_ply.collect()
}

inputs_points.count

// graph_tri.edges.collect().map(e => (e.srcId,e.dstId)).filter(x => (x._1 == 0 || x._2 ==0)).size

