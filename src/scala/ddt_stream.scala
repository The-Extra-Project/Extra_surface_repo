import sys.process._
import scala.io.Source
import java.io._
import scala.xml._
import java.lang.Double
import scala.concurrent._
import scala.collection.parallel._
import java.nio.file.{ Paths, Files }
import scala.collection.mutable.ListBuffer

import java.util.concurrent.Executors
import java.util.Date;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import org.apache.spark._;
import org.apache.spark.graphx._;
import org.apache.spark.graphx.PartitionStrategy._
import org.apache.spark.rdd.RDD;
import org.apache.spark.sql.SaveMode
import org.apache.spark.storage.StorageLevel
import org.apache.spark.SparkConf


import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
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

if (output_dir.isEmpty ||
  input_dir.isEmpty ) {
  println("Bash variable are empy, please set it")
  System.exit(1)
}

// ===== Algo params extraction / struct def  ==========
// Xml stuff
val input_label = "pts_generated"
val dim = get_param_from_xml(env_xml, "dim", "2").toInt
val nbt_side = get_param_from_xml(env_xml, "nbt_side", "3").toInt;
val do_tests = get_param_from_xml(env_xml, "do_tests", "true").toBoolean;
val do_plot = get_param_from_xml(env_xml, "do_plot", "true").toBoolean;
val nbp_pt=100000
val nbp = 10000//(nbp_pt*Math.pow(nbt_side,dim)).toInt


val slvl: StorageLevel = StorageLevel.MEMORY_AND_DISK_2



val iq = new IQlibSched(slvl)



var params_ddt = Map(
  "exec_path" -> List(ddt_main_dir + "build-spark-Release/bin/ddt-stream-exe" ),
  "step" -> List(""),
  "dim" -> List("2"),
  "bbox" -> List("-100x5100:-100x5100"),
  "input_dir" -> List(input_dir),
  "output_dir" -> List(output_dir),
  "seed" -> List(algo_seed),
  "nbt_side" -> List(nbt_side.toString)
);



val generate_points_fun =  set_params(params_ddt,  List(("step","generate_points"))).to_command_line
val get_bbox_points_fun =  set_params(params_ddt,  List(("step","get_bbox_points"))).to_command_line
val insrt_in_tri_fun = set_params(params_ddt, List(("step","insert_in_triangulation"))).to_command_line
val get_neighbors_fun =  set_params(params_ddt, List(("step","get_neighbors"))).to_command_line
val extract_tri_vrt_fun =  set_params(params_ddt, List(("step","extract_tri_vrt"))).to_command_line
val ply2geojson_fun =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
val tests_fun =  set_params(params_ddt, List(("step","tests"))).to_command_line
val extract_tile_vrt_fun =  set_params(params_ddt, List(("step","extract_tile_vrt"))).to_command_line
val extract_tile_json_fun =  set_params(params_ddt, List(("step","extract_tile_json"))).to_command_line
val tile_cmd =  set_params(params_ddt, List(("step","tile_ply"),("output_dir", output_dir))).to_command_line
val copy_cmd =  set_params(params_ddt, List(("step","copy"),("output_dir", output_dir))).to_command_line
val id_cmd = List(ddt_main_dir + "build-spark-Release/bin/identity-exe");


// ============  Init data ===========

val do_generate = false;
var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));

if(!do_generate){
  println("")
  println("======== LOAD DATA =============")
  val ss_reg = "austin3[0]".r
  val ply_input = getListOfFiles(input_dir).filter(
    x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
  var kvrdd_inputs = iq.get_kvrdd(sc.parallelize(ply_input.map( 
       fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p")
  val res_tiles = iq.run_pipe_fun_KValue(
  tile_cmd ++ List("--label", "tile_pp"),
  kvrdd_inputs, "tile", do_dump = false)
  kvrdd_points = iq.get_kvrdd(res_tiles,"p");
}else{
  println("")
  println("======== GENERATE DATA =============")

  val input_rdd_raw: RDD[KValue] = sc.parallelize(List.range(0, math.pow(nbt_side, dim).toInt).map(
    x => (x.toLong, List(""))))

  val inputs_points = iq.run_pipe_fun_KValue(
    generate_points_fun ++ List("--label", input_label,"--nbp",nbp.toString),
    input_rdd_raw, "generate_points", do_dump = false)
  kvrdd_points = iq.get_kvrdd(inputs_points, "p").reduceByKey(_ ++ _)
} 



// ============== Start Algo =======================
val t0 = System.nanoTime()



// val ddt_aglo = new DDT_algo_spark(iq,sc,params_ddt,exec_path)

// Insert in tri1
val res_tri_local = iq.run_pipe_fun_KValue(
  insrt_in_tri_fun ++ List("--label", "tri_tile"),
  kvrdd_points, "insert_in_triangulation", do_dump = false)
val kvrdd_tri_local: RDD[KValue] = iq.get_kvrdd(res_tri_local, "t");
val kvrdd_count: RDD[KValue] = iq.get_kvrdd(res_tri_local, "c");

// Get bbox points
val res_bbox = iq.run_pipe_fun_KValue(
  get_bbox_points_fun,
  kvrdd_tri_local, "get_bbox_points", do_dump = false)

val nb_pts_bbox = res_bbox.count;
val list_bbox_pts = res_bbox.reduce(_ ++ _);
val stream_pts = "q 1 0 s " ++ dim.toString ++ " " ++ nb_pts_bbox.toString ++ " " ++ list_bbox_pts
val rdd_input_tri = kvrdd_tri_local.map(x => (x.key, (x.value ++ List(stream_pts))))

// Inset bbox points
val res_tri_bbox = iq.run_pipe_fun_KValue(
  insrt_in_tri_fun ++ List("--label", "tri_bbox"),
  rdd_input_tri, "insert_in_triangulation_bbox", do_dump = false)
val kvrdd_tri_bbox: RDD[KValue] = iq.get_kvrdd(res_tri_bbox, "t");

val res_nbrs: RDD[VData] = iq.run_pipe_fun_KValue(
  get_neighbors_fun ++ List("--label", "nbrs_init"),
  kvrdd_tri_bbox, "get_neighbors", do_dump = false);

//Extract the edge of the merging graph
val rdd_list_edges:  ListBuffer[RDD[TEdge]] = ListBuffer(iq.get_edgrdd(res_nbrs, "e"));
val kvrdd_list_tri : ListBuffer[RDD[KValue]] = ListBuffer(kvrdd_tri_bbox)

var acc = 0
var nb_insert = 0

val defaultV = (List(""));
while (nb_insert != 0 || acc == 0) {
  //Create the merging graph (in each node : triangulation and sent point from source)

  val graph_tri: TGraph = Graph(kvrdd_list_tri.last, rdd_list_edges.last, defaultV);
  val kvrdd_insert = iq.send_edge_attr_to_dst(graph_tri)

  // Inset bbox points
  val res_tri_insert_nbr = iq.run_pipe_fun_KValue(
    insrt_in_tri_fun ++ List("--label", "tri_loop_" + acc.toString),
    kvrdd_insert, "insert_in_triangulation_nbr", do_dump = false)

  val kvrdd_cur_tri : RDD[KValue] = iq.get_kvrdd(res_tri_insert_nbr, "t");
  kvrdd_list_tri += kvrdd_cur_tri

  val kvrdd_count3: RDD[KValue] = iq.get_kvrdd(res_tri_insert_nbr, "c");
  nb_insert = kvrdd_count3.map(x => x.value(0).split(" ").last.toInt).reduce(_ + _)
  acc = acc + 1;

  val res_nbrs_cur = iq.run_pipe_fun_KValue(
    get_neighbors_fun ++ List("--label", "nbrs"),
    kvrdd_list_tri.last, "get_neighbors", do_dump = false);

  //Extract the edge of the merging graph
  val rdd_cur_edges = iq.get_edgrdd(res_nbrs_cur, "e");
  rdd_list_edges += rdd_cur_edges
  println("==== Stats ====")
  //println("Nb edges:" + graph_tri.edges.count)
  println("NB insert :" + nb_insert)
}
val t1 = System.nanoTime()
println("")
println("======== Triangulation computed ============")
println("nbp:" + nbp);
println("Elapsed time: " + (t1 - t0)/1000000000.0 + "s")


if(do_tests){
  val graph_tri: TGraph = Graph(kvrdd_list_tri.last, rdd_list_edges.last, defaultV);

    val rdd_id_res = iq.run_pipe_fun_KValue(
     id_cmd ++ List("--label", "funtests"),
  graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)
  rdd_id_res.collect()

  val rdd_json_res = iq.run_pipe_fun_KValue(
     ply2geojson_fun ++ List("--label", "ply2geojson"),
  graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)
  rdd_json_res.collect()

  
  val rdd_test_tile = iq.run_pipe_fun_KValue(
     tests_fun ++ List("--label", "funtests"),
    graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)

  rdd_test_tile.collect();


  val res_copy = iq.run_pipe_fun_KValue(
    copy_cmd ++ List("--label", "copy"),
    kvrdd_points, "copy", do_dump = false)
  res_copy.collect();
}

if(do_plot){
  val graph_tri: TGraph = Graph(kvrdd_list_tri.last, rdd_list_edges.last, defaultV);
// Tri
  val rdd_json_tile = iq.run_pipe_fun_KValue(
     extract_tri_vrt_fun ++ List("--label", "tile"),
  graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)
  rdd_json_tile.collect()

// Full tri
  val rdd_full_tri = kvrdd_list_tri.last.map(x => (0L,x.value)).reduceByKey(_++ _)
  val rdd_json_name = iq.run_pipe_fun_KValue(
     extract_tri_vrt_fun ++ List("--label", "output1"),
  rdd_full_tri, "extract_tri_vrt_final", do_dump = false)
  rdd_json_name.collect()

// Clique

  val kvrdd_clique_1 =  iq.aggregate_value_clique(graph_tri, 1);
  val rdd_json_clique = iq.run_pipe_fun_KValue(
     extract_tri_vrt_fun ++ List("--label", "output_clique_1"),
  kvrdd_clique_1, "extract_tri_vrt_clique1", do_dump = false)
  rdd_json_clique.collect()
}



print("2D json viewer => https://mapshaper.org/")

// graph_tri.edges.collect().map(e => (e.srcId,e.dstId)).filter(x => (x._1 == 0 || x._2 ==0)).size
