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
import java.text.DecimalFormat
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
import spark_ddt.wasure._;
import tiling._;

import algo_stats._;
import xml_parsing._;
import dataset_processing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import spark_ddt.util.params_parser.params_map
import files_opt._;
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
var params_scala = new Hash_StringSeq with mutable.MultiMap[String, String]
if (fs.exists(new Path(env_xml))){
  val param_list = parse_xml_datasets(env_xml)
  params_scala = param_list(0) // We only process 1 set of parameter in this workflow
}
val df_par = sc.defaultParallelism;

// ===============================================
// ==== Scala and param initialization ===========
// Param scala is mutable, get params set the default value to the collection if it's empty
//  Se for instance the xml documentation / Algorithm params for the effect

// System params
val dim = params_scala.get_param("dim", "3").toInt
//val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-D" + dim.toString)
val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-" + dim.toString)
val build_dir = global_build_dir + "/" + ddt_kernel_dir
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "MEMORY_ONLY"))
val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "MEMORY_AND_DISK_SER"))
val max_ppt_per_tile = 100000


// General Algo params
val bbox = params_scala.get_param("bbox", "")
val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt
val algo_seed =  params_scala.get_param("algo_seed",scala.util.Random.nextInt(100000).toString);

// Wasure Algo params
val wasure_mode = params_scala.get_param("mode", "surface")
val pscale = params_scala.get_param("pscale", "0.05").toFloat
val nb_samples = params_scala.get_param("nb_samples", "3").toFloat
val rat_ray_sample = params_scala.get_param("rat_ray_sample", "1").toFloat
val min_ppt = params_scala.get_param("min_ppt", "5").toInt
val main_algo_opt = params_scala.get_param("algo_opt", "seg_lagrange_weight")
val dst_scale = params_scala.get_param("dst_scale", "-1").toFloat
val dst_scale = params_scala.get_param("lambda", "0.1").toFloat
val max_opt_it = params_scala.get_param("max_opt_it", "10").toInt



// Set the iq library on
val iq = new IQlibSched(slvl_glob,slvl_loop)

// Set the c++ command line object
val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
val params_wasure =  set_params(params_new,List(
  ("exec_path", build_dir + "/bin/wasure-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("input_dir",input_dir),
  ("output_dir",output_dir)
))

// Create output dir
fs.mkdirs(new Path( output_dir),new FsPermission("777"))
params_scala("output_dir") = collection.mutable.Set(output_dir)
params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)



println("")
println("=======================================================")
params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))


// Starts preprocess
val preprocess_cmd =  set_params(params_wasure, List(("step","preprocess"))).to_command_line
val fs = FileSystem.get(sc.hadoopConfiguration);
val regexp_filter = params_scala.get_param("regexp_filter", "");
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
var kvrdd_inputs: RDD[KValue] = sc.parallelize(List((0L,List(""))));


println("")
println("======== LOAD DATA  file =============")
val ss_reg = regexp_filter.r
// get number of ply
val nb_ply = fs.listStatus(new Path(input_dir)).map(x => fs.listStatus(x.getPath)).reduce(_ ++ _).map(_.getPath).filter(
  x => ((x.toString endsWith ".ply")) && ((ss_reg).findFirstIn(x.toString).isDefined)
).size

if(nb_ply == 0){
  println("!! ERROR: 0 PLY FOUND !!")
  println("!! ERROR: 0 PLY FOUND !!")
}

// List of input ply filepath
val ply_input = getListOfFiles(input_dir).filter(
  x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map(
  fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p").repartition(nb_ply)

// process data
val struct_inputs = iq.run_pipe_fun_KValue(
  preprocess_cmd ++ List("--label", "struct"),
  kvrdd_inputs
    , "struct", do_dump = false)
struct_inputs.collect
val kvrdd_bbox = iq.get_kvrdd(struct_inputs,"s").persist(slvl_loop);
val bba =   kvrdd_bbox.map(x => x._2.head.split("z").tail.head.split(" ").filter(!_.isEmpty).map(x => x.toFloat)).reduce(
  (x,y) => Array(Math.min(x(0),y(0)),Math.max(x(1),y(1)),Math.min(x(2),y(2)),Math.max(x(3),y(3)),Math.min(x(4),y(4)),Math.max(x(5),y(5)), x(6)+y(6)))
val smax =   Math.max(Math.max(bba(1)-bba(0),bba(1)-bba(0)),bba(1)-bba(0))
val tot_nbp = bba(6)
val ndtree_depth = (Math.log(tot_nbp/max_ppt_per_tile)/Math.log(3)).round 

// Dump xml

params_scala("bbox") = collection.mutable.Set(bba(0) + "x" + (bba(0) + smax) + ":" + bba(2) + "x" + (bba(2) + smax) + ":" + bba(4) + "x" + (bba(4) + smax))
params_scala("ndtree_depth") = collection.mutable.Set(ndtree_depth.toString)
params_scala("datatype") = collection.mutable.Set("filestream")

val params_scala_dump = params_scala.filter(x => !x._2.head.isEmpty && x._1 != "do_expand" && x._1 != "output_dir" )

val xml_output_string =  ("<env>\n\t<datasets>\n\t\t<generated>\n" +
  params_scala_dump.map(x => "\t\t\t<" + x._1 + ">" + x._2.head + "</" + x._1 + ">").reduce( _ + "\n" + _ ) +
  "\n\t\t</generated>\n\t</datasets>\n</env>\n")
val path_name = new Path(output_dir + "/wasure_metadata_3d_gen.xml" );
val output = fs.create(path_name);
val os = new BufferedOutputStream(output)
os.write((xml_output_string).getBytes)
os.close()

