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


import xyz._
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
import fr.ign.spark.iqmulus.xyz._

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
val do_plot = get_param_from_xml(env_xml, "do_plot", "false").toBoolean;
val do_generate = get_param_from_xml(env_xml, "do_generate", "false").toBoolean;
val do_stream = get_param_from_xml(env_xml, "do_stream", "false").toBoolean;
val regexp_filter = get_param_from_xml(env_xml, "regexp_filter", "");

val nbp_list  = get_list_param_from_xml(env_xml, "nbp", "10000").map(_.toInt);
val nbt_list = get_list_param_from_xml(env_xml, "nbt_side", "3").map(_.toInt);

val conf_list = get_list_param_from_xml(env_xml, "conf", "10,1000000").map(x => x.split(",").map(_.toInt))


//val slvl: StorageLevel = StorageLevel.MEMORY_AND_DISK

//val slvl: StorageLevel = StorageLevel.OFF_HEAP

//val slvl : StorageLevel = StorageLevel.MEMORY_AND_DISK_SER
val slvl: StorageLevel = StorageLevel.DISK_ONLY
//val slvl: StorageLevel = StorageLevel.MEMORY_ONLY
val iq = new IQlibSched(slvl)
 sc.setCheckpointDir("/home/laurent/shared_spark/checkpoint/")

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


val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")

// nbp_list.foreach{ nbp =>
//   nbt_list.foreach{ nbt_side =>

if(false){
  val nbt_side = conf_list.head(0);
  val nbp = conf_list.head(1);
}


conf_list.foreach{ cc =>
  val nbt_side = cc(0);
  val nbp = cc(1);
  val tot_nbt = scala.math.pow(nbt_side,dim);
  val nbp_per_tile = nbp/tot_nbt;
  val rep_value = ((if((tot_nbt) < sc.defaultParallelism) sc.defaultParallelism else  (tot_nbt).toInt))

  val test_dir = output_dir // + dateFormatter.format(Calendar.getInstance().getTime()) + "_nbt" + nbt_side + "_nbp" + nbp
  ("mkdir -p " + test_dir).!!
  params_ddt = params_ddt + ("output_dir" -> List(test_dir) )

  params_ddt = params_ddt + ("nbt_side" -> List(nbt_side.toString) )
  val generate_points_fun =  set_params(params_ddt,  List(("step","generate_points"))).to_command_line
  val get_bbox_points_fun =  set_params(params_ddt,  List(("step","get_bbox_points"))).to_command_line
  val extract_tri_vrt_fun =  set_params(params_ddt, List(("step","extract_tri_vrt"))).to_command_line
  val ply2geojson_fun =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
  val tests_fun =  set_params(params_ddt, List(("step","tests"))).to_command_line
  val extract_tile_vrt_fun =  set_params(params_ddt, List(("step","extract_tile_vrt"))).to_command_line
  val extract_tile_json_fun =  set_params(params_ddt, List(("step","extract_tile_json"))).to_command_line
  val tile_cmd =  set_params(params_ddt, List(("step","tile_ply"),("output_dir", test_dir))).to_command_line
  val id_cmd = List(build_dir + "/bin/identity-exe");

  val tt000 = System.nanoTime()

  // ============  Init data ===========

  var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
  val kvrdd_log_list : ListBuffer[RDD[KValue]] = ListBuffer()

  if(!do_generate){
    println("")
    println("======== LOAD DATA =============")
    val ss_reg = regexp_filter.r
    val ply_input = getListOfFiles(input_dir).filter(
      x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
    var kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map(
      fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p")
    val res_tiles = iq.run_pipe_fun_KValue(
      tile_cmd ++ List("--label", "tile_pp"),
      kvrdd_inputs, "tile", do_dump = false)
    kvrdd_points = iq.get_kvrdd(res_tiles,"p");
    kvrdd_points = kvrdd_points.reduceByKey( (u,v) => u ::: v);
  }else{
    println("")
    println("======== GENERATE DATA =============")
    val input_rdd_raw: RDD[KValue] = sc.parallelize(List.range(0, math.pow(nbt_side, dim).toInt).map(
      x => (x.toLong, List("")))).repartition(rep_value)
    val inputs_points = iq.run_pipe_fun_KValue(
      generate_points_fun ++ List("--label","lab2","--nbp",nbp.toString),
      input_rdd_raw, "generate_points", do_dump = false)
    kvrdd_points = iq.get_kvrdd(inputs_points, "p",txt="pts").reduceByKey(_ ++ _).persist(slvl);
    if(nbp <= 10000){
      val rdd_json_input_ply = iq.run_pipe_fun_KValue(
        ply2geojson_fun ++ List("--label", "generated"),
        kvrdd_points, "extract_tri_vrt_final", do_dump = false)
      rdd_json_input_ply.collect()
    }
  }

  println("======== START DDT =============")
  val t0 = System.nanoTime()
  val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(
    kvrdd_points = kvrdd_points,
    iq = iq,
    params_ddt = params_ddt,
    do_plot = false,
    do_simple_output = false);

  graph_tri.vertices.count
  // graph_tri.vertices.unpersist()
  // graph_tri.edges.unpersist()
  println("======== algo done =============")
  val t1 = System.nanoTime()
  val scala_time = ((t1 - t0)/1000000000.0);
  println("scala time:" + scala_time.toString)
  dump_stats(nbp,nbt_side,"full_"+t0,scala_time.toLong,ddt_main_dir +"/" + sc.applicationId ,sc);
  //dump_stats(log_tri,nbp,nbt_side,"full_"+t0,scala_time.toLong,test_dir ,sc);
  if(nbp <= 20000){
    val rdd_json_res = iq.run_pipe_fun_KValue(
      ply2geojson_fun ++ List("--label", "res_merged","--style","tri_main.qml"),
      graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)
    rdd_json_res.collect()
  }
}


// graph_tri.edges.collect().map(e => (e.srcId,e.dstId)).filter(x => (x._1 == 0 || x._2 ==0)).size
