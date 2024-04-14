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
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "DISK_ONLY"))
val max_ppt_per_tile = 500000


// General Algo params
val bbox = params_scala.get_param("bbox", "")
val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt
val algo_seed =  params_scala.get_param("algo_seed",scala.util.Random.nextInt(100000).toString);

// Wasure Algo params
val wasure_mode = params_scala.get_param("mode", "1")
val pscale = params_scala.get_param("pscale", "0.05").toFloat
val nb_samples = params_scala.get_param("nb_samples", "50").toFloat
val rat_ray_sample = params_scala.get_param("rat_ray_sample", "0").toFloat
val min_ppt = params_scala.get_param("min_ppt", "50").toInt
val main_algo_opt = params_scala.get_param("algo_opt", "seg_lagrange_weight")
val dst_scale = params_scala.get_param("dst_scale", "-1").toFloat
val lambda = params_scala.get_param("lambda", "2").toFloat
val max_opt_it = params_scala.get_param("max_opt_it", "50").toInt
val do_stats = params_scala.get_param("do_stats", "false").toBoolean
val coef_mult = params_scala.get_param("coef_mult", "5").toFloat


// Set the iq library on
val iq = new IQlibSched(slvl_glob,slvl_loop)

val current_plateform = get_bash_variable("CURRENT_PLATEFORM");

current_plateform.toLowerCase match {
  case "cnes" => {
    println("void")
    import org.apache.log4j.PropertyConfigurator
    PropertyConfigurator.configure(ddt_main_dir + "/log4j-executor.properties.v3" )
  }
  case "multivac" => {
    val hdfs_files_dir = get_bash_variable("HDFS_FILES_DIR");
    val exec_path_list = List("ddt-stream-exe","wasure-stream-exe");
    // val lib_list = List(
    //   "libboost_system.so.1.71.0",
    //   "libboost_filesystem.so.1.71.0","libc.so.6",
    //   "libddt.so","libgcc_s.so.1","libgmp.so.10","libm.so.6","libpthread.so.0","libstdc++.so.6","libtbmrf.so"
    // )

    val lib_list = List(
      "ddt-stream-exe","libboost_system.so.1.67.0",
      "libboost_filesystem.so.1.67.0","libCGAL.so.13",
      "libddt.so","libstdc++.so.6","libm.so.6","libtbmrf.so"
    )

    var inc_dir = sc.parallelize(List("")).map(
      x => (SparkFiles.get("libCGAL.so.13").split("/").dropRight(1).reduce(_ ++ "/" ++ _)).dropRight(1)
    ).collect()(0).split("_").dropRight(1).reduce(_ ++ "_" ++ _) + "_"
    var inc_dir_list = (1 to 100).map(inc_dir  + "%06d".format(_)).reduce(_ ++ ":" ++ _)

    exec_path_list.map(x => sc.addFile(hdfs_files_dir + x))
    lib_list.map(x => sc.addFile(hdfs_files_dir + x))
    println("===> SC DEFAUT PAR :" + sc.defaultParallelism)

    env_map = Map(
      "CLASSPATH" -> sys.env("CLASSPATH"),
      "LD_LIBRARY_PATH" -> (inc_dir_list + ":/usr/lib/jvm/java-8-oracle/jre/lib/amd64/server:/opt/cloudera/parcels/CDH/lib/")
    )
  }
  case "local" => {}
}

val cpp_exec_path = current_plateform.toLowerCase match {
  case "cnes"  =>     "/softs/rh7/singularity/3.5.3/bin/singularity exec " + build_dir + "/ddt_img_cnes.simg " + build_dir + "/bin/"
  case "singularity"  =>     "singularity exec " + build_dir + "/ddt_img_cnes.simg " + build_dir + "/bin/"
  case "multivac"  =>   "" // Empty string
  case _  => build_dir + "/bin/"
}


// Set the c++ command line object
val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
val params_wasure =  set_params(params_new,List(
  ("exec_path", cpp_exec_path + "wasure-stream-exe"),
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

val bbox_cmd =  set_params(params_wasure, List(("step","compute_bbox"))).to_command_line
val fs = FileSystem.get(sc.hadoopConfiguration);
val regexp_filter = params_scala.get_param("regexp_filter", "");
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
var kvrdd_inputs: RDD[KValue] = sc.parallelize(List((0L,List(""))));


println("")
println("======== LOAD DATA  file =============")
val ss_reg = regexp_filter.r
// get number of ply
val nb_ply = fs.listStatus(new Path(input_dir)).map(x => fs.listStatus(x.getPath)).reduce(_ ++ _).map(_.getPath).filter(
  x => ((x.toString endsWith ".las") || (x.toString endsWith ".laz")) && ((ss_reg).findFirstIn(x.toString).isDefined)
).size

if(nb_ply == 0){
  println("!! ERROR: 0 PLY FOUND !!")
  println("!! ERROR: 0 PLY FOUND !!")
}

// List of input ply filepath
// List of input ply filepath
val ply_input = getListOfFiles(input_dir).filter(
  x => (((x.toString endsWith ".las") || (x.toString endsWith ".laz")) && ((ss_reg).findFirstIn(x.toString).isDefined))).zipWithIndex

kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map(
  xx => "p 1 " + xx._2 + " f " + xx._1)),"p").repartition(nb_ply)

// val ply_input = getListOfFiles(input_dir).filter(
//   x => (((x.toString endsWith ".las") || (x.toString endsWith ".laz")) && ((ss_reg).findFirstIn(x.toString).isDefined)))
// kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map(
//   fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p").repartition(nb_ply)

// process data
val struct_inputs_bbox = iq.run_pipe_fun_KValue(
  bbox_cmd ++ List("--label", "struct"),
  kvrdd_inputs
    , "struct", do_dump = false)
struct_inputs_bbox.collect
val kvrdd_bbox_ori = iq.get_kvrdd(struct_inputs_bbox,"s").persist(slvl_loop);
val bba_ori =   kvrdd_bbox_ori.map(x => x._2.head.split("z").tail.head.split(" ").filter(!_.isEmpty).map(x => x.toFloat)).reduce((x,y) => Array(Math.min(x(0),y(0)),Math.max(x(1),y(1)),Math.min(x(2),y(2)),Math.max(x(3),y(3)),Math.min(x(4),y(4)),Math.max(x(5),y(5)), x(6)+y(6)))


val preprocess_cmd =  set_params(params_wasure, List(
  ("step","preprocess"),
  ("bbox", bba_ori(0) + "x" + bba_ori(1) + ":" + bba_ori(2) + "x" + bba_ori(3) + ":" + bba_ori(4) + "x" + bba_ori(5))
)).to_command_line

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
val ndtree_depth = Math.max((Math.log(tot_nbp/max_ppt_per_tile)/Math.log(3)).round,0) + 1
val lambda = params_scala.get_param("lambda", "1").toFloat


// Dump xml

// params_scala("bbox") = collection.mutable.Set(bba(0) + "x" + (bba(0) + smax) + ":" + bba(2) + "x" + (bba(2) + smax) + ":" + bba(4) + "x" + (bba(4) + smax))
params_scala("bbox") = collection.mutable.Set(bba(0) + "x" + (bba(0) + smax) + ":" + bba(2) + "x" + (bba(2) + smax) + ":" + 0.0 + "x" + 1000.0)

params_scala("ndtree_depth") = collection.mutable.Set(ndtree_depth.toString)
params_scala("datatype") = collection.mutable.Set("files")
params_scala("max_ppt") = collection.mutable.Set(max_ppt_per_tile.toString)

val params_scala_dump = params_scala.filter(x => !x._2.head.isEmpty && x._1 != "do_expand" && x._1 != "output_dir" )

val xml_output_string =  ("<env>\n\t<datasets>\n\t\t<generated>\n" +
  params_scala_dump.map(x => "\t\t\t<" + x._1 + ">" + x._2.head + "</" + x._1 + ">").reduce( _ + "\n" + _ ) +
  "\n\t\t</generated>\n\t</datasets>\n</env>\n")
val path_name = new Path(output_dir + "/wasure_metadata_3d_gen.xml" );
val output = fs.create(path_name);
val os = new BufferedOutputStream(output)
os.write((xml_output_string).getBytes)
os.close()

