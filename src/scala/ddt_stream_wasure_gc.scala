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
import iqlib_algo._;
import iqlibflow._;

import algo_stats._;
import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;
import mflow._;

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
val nbt_side = get_param_from_xml(env_xml, "nbt_side", "3").toInt;
val do_tests = get_param_from_xml(env_xml, "do_tests", "true").toBoolean;
val do_plot = get_param_from_xml(env_xml, "do_plot", "true").toBoolean;
val do_generate = get_param_from_xml(env_xml, "do_generate", "false").toBoolean;
val regexp_filter = get_param_from_xml(env_xml, "regexp_filter", "");
val nbp  = get_param_from_xml(env_xml, "nbp", "10000").toInt;
val lambda_list  = get_list_param_from_xml(env_xml, "lambda", "1").map(_.toDouble);

val slvl: StorageLevel = StorageLevel.MEMORY_AND_DISK_2
val iq = new IQlibSched(slvl)

var params_ddt = Map(
  "exec_path" -> List(build_dir + "/bin/ddt-stream-exe" ),
  "step" -> List(""),
  "dim" -> List(dim.toString),
  "ech_input" -> List("1"),
  "input_dir" -> List(input_dir),
  "output_dir" -> List(output_dir),
  "seed" -> List(algo_seed),
  "nbt_side" -> List(nbt_side.toString)
);


var params_wasure = Map(
  "exec_path" -> List(build_dir  + "/bin/wasure-stream-exe"),
  "step" -> List(""),
  "dim" -> List(dim.toString),
  "lambda" -> List("1"),
  "rat_ray_sample" -> List("0.2"),
  "input_dir" -> List(input_dir),
  "output_dir" -> List(output_dir),
  "seed" -> List(algo_seed),
  "nbt_side" -> List(nbt_side.toString)
);


if(!bbox.isEmpty){
  params_wasure = params_wasure + ("bbox" -> List(bbox) )
  params_ddt = params_ddt + ("bbox" -> List(bbox) )
}

//params_ddt = params_ddt + ("exec_path" -> List("/home/laurent/code/ddt-wasure/build-spark-Release-2/bin/ddt-stream-exe") )

val generate_points_cmd =  set_params(params_ddt,  List(("step","generate_points"))).to_command_line
val get_bbox_points_cmd =  set_params(params_ddt,  List(("step","get_bbox_points"))).to_command_line
val insrt_in_tri_cmd = set_params(params_ddt, List(("step","insert_in_triangulation"))).to_command_line
val get_neighbors_cmd =  set_params(params_ddt, List(("step","get_neighbors"))).to_command_line
val extract_tri_vrt_cmd =  set_params(params_ddt, List(("step","extract_tri_vrt"))).to_command_line
val ply2geojson_cmd =  set_params(params_ddt, List(("step","ply2geojson"))).to_command_line
val tests_cmd =  set_params(params_ddt, List(("step","tests"))).to_command_line
val extract_tile_vrt_cmd =  set_params(params_ddt, List(("step","extract_tile_vrt"))).to_command_line
val extract_tile_json_cmd =  set_params(params_ddt, List(("step","extract_tile_json"))).to_command_line
val tri2geojson_cmd =  set_params(params_ddt, List(("step","tri2geojson"))).to_command_line
val tile_cmd =  set_params(params_ddt, List(("step","tile_ply"),("output_dir", output_dir))).to_command_line
val copy_cmd =  set_params(params_ddt, List(("step","copy"),("output_dir", output_dir))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");


val dim_cmd =  set_params(params_wasure, List(("step","dim"))).to_command_line
val dst_cmd =  set_params(params_wasure, List(("step","dst"))).to_command_line
val extract_graph_cmd =  set_params(params_wasure, List(("step","extract_graph"))).to_command_line
val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
val ext_cmd =  set_params(params_wasure, List(("step","extract_surface"))).to_command_line
val tri2geojson_wasure_cmd =  set_params(params_wasure, List(("step","tri2geojson"))).to_command_line
val wasure_ply2geojson_cmd =  set_params(params_wasure, List(("step","ply2geojson"))).to_command_line
val fmt = new java.text.DecimalFormat("#,##0.##############")
// ============  Init data ===========

var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));


val kvrdd_log_list : ListBuffer[RDD[KValue]] = ListBuffer()


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

dim match {
  case 2 => {
    val rdd_json_input_ply = iq.run_pipe_fun_KValue(
      ply2geojson_cmd ++ List("--label", "generated"),
      kvrdd_points, "extract_tri_vrt_final", do_dump = false)
    rdd_json_input_ply.collect()
  }
  case _ => {}
}


println("======== START ALGO =============")
val t0 = System.nanoTime()
// Dimensionnality
val res_dim = iq.run_pipe_fun_KValue(
  dim_cmd ++ List("--label", "dim"),
  kvrdd_points, "dim", do_dump = false)
val kvrdd_dim = iq.get_kvrdd(res_dim,"p");
kvrdd_log_list += iq.get_kvrdd(res_dim,"l");

// Delauay triangulatin computation
val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(kvrdd_points,iq,params_ddt,false);
val graph_pts = Graph(kvrdd_dim, graph_tri.edges, List(""));
val input_dst = (graph_tri.vertices).union(iq.aggregate_value_clique(graph_pts, 1)).reduceByKey(_ ::: _).setName("input_dst");
kvrdd_log_list ++=  log_tri

// Simplex score computation
val res_dst = iq.run_pipe_fun_KValue(
  dst_cmd ++ List("--label", "dst"),
  input_dst, "dst", do_dump = false)
val kvrdd_dst = iq.get_kvrdd(res_dst,"t");
kvrdd_log_list += iq.get_kvrdd(res_dst,"l");
val graph_dst = Graph(kvrdd_dst, graph_tri.edges, List(""));
val input_seg =  iq.aggregate_value_clique(graph_dst, 1);




// ============ Spark graphcut =======
// Get global id by tile for each simplex
val summed_id = sum_simplex_id(stats_tri);

val tot_simlex = summed_id.last._2
val kvrdd_tile_id : RDD[KValue] = sc.parallelize(
  summed_id.dropRight(1).map(x =>
    (x._1.toLong, List("s 1 " + x._1.toString +  " s " + x._2.map(_.toString).reduce((y,z) => y + " " + z)))
  )
)

val input_extract_simplex = (graph_dst.vertices).union(kvrdd_tile_id).reduceByKey(_ ::: _);
val extract_graph_cmd =  set_params(params_wasure, List(("step","extract_graph"),("lambda","1"))).to_command_line
val full_graph = iq.run_pipe_fun_KValue(
  extract_graph_cmd,
  input_extract_simplex, "ext_gr", do_dump = false)


val edges_graphx = full_graph.filter(x => !x.isEmpty).map(x => x.split(" ")).map(x => org.apache.spark.graphx.Edge(x(0).toLong,x(1).toLong,x(2).toDouble.toInt))
val nodes_graphx = sc.parallelize(List.range(0, tot_simlex(2)+2).map(x => (x.toLong,x.toLong)))
val gg1 : org.apache.spark.graphx.Graph[Long,Int] = Graph(nodes_graphx,edges_graphx).groupEdges( (u,v) => u+v)
val flows = maxFlow(0L, 1L, gg1,sc,100)
val res_score = (gg1.edges.map(x => ((x.srcId,x.dstId),x.attr)) union flows).reduceByKey((u,v) => Math.abs(u - v))
val res_select = res_score.filter(
  {case ((u,v),w) => u <2 || v < 2 } // Keep only edges connected to source or target
).map(
  {case ((u,v),w) => if(u >1) (u,(v,w)) else (v,(u,w))}
).reduceByKey(
 (x1,x2) => if(x1._2 < x2._2) x1 else x2
)


// (u-2) for the padding according to 
val rdd_resmaxflow =  res_select.map(
  {case (u,(v,w)) => ((summed_id.indexWhere(x => x._2(2) > (u-2) )) - 1,(1,u.toString + " " + v.toString))  }
).reduceByKey(
  (v1,v2) => ((v1._1 + v2._1), v1._2 + " " + v2._2)
).map( x => (x._1.toLong, List("l 1 " + x._1.toString + " s " + x._2._1 + " " + x._2._2)) )

  

// val rdd_resmaxflow =  res_select.map(
//   {case (u,(v,w)) => ((summed_id.indexWhere(x => x._2(2) >= u )) - 1,u.toString + " " + v.toString)  }
// ).reduceByKey(
//   (v1,v2) => v1 + " " + v2
// ).map( x => (x._1.toLong, List("l 1 " + x._1.toString + " s " + x._2)) )

val input_extract_graph = (input_extract_simplex).union(rdd_resmaxflow).reduceByKey(_ ::: _);


val res_seg_mf = iq.run_pipe_fun_KValue(
  fill_graph_cmd ++ List("--label", "sparkcuted"),
   input_extract_graph, "ext_gr", do_dump = false)

val kvrdd_seg = iq.get_kvrdd(res_seg_mf,"t");
val graph_seg = Graph(kvrdd_seg, graph_tri.edges, List(""));
  if (dim == 2)  {
    iq.run_pipe_fun_KValue(
      ply2geojson_cmd ++ List("--label","ply2geojson_ll_" + "1","--style","tri_seg.qml"),
      kvrdd_seg, "seg", do_dump = false).collect()
    iq.run_pipe_fun_KValue(
      tri2geojson_wasure_cmd ++ List("--label","tri2geojson_ll_" + "1","--style","tri_seg.qml"),
      iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false).collect()
  }


// gg1.edges.map(x => ((x.srcId,x.dstId),x.attr)) union
// kvrdd_tile_id
// Graph cut segementation
lambda_list.foreach{ ll =>
  println("==== Segmentation with lambda " + fmt.format(ll) + "  ====")
  val seg_cmd =  set_params(params_wasure, List(("step","seg"),("lambda",fmt.format(ll)))).to_command_line
  val res_seg = iq.run_pipe_fun_KValue(
    seg_cmd ++ List("--label", "seg_ll_" + fmt.format(ll)),
    input_seg, "seg", do_dump = false)
  val kvrdd_seg = iq.get_kvrdd(res_seg,"t");
  kvrdd_log_list += iq.get_kvrdd(res_seg,"l");
  val graph_seg = Graph(kvrdd_seg, graph_tri.edges, List(""));
  if (dim == 2)  {
    iq.run_pipe_fun_KValue(
      ply2geojson_cmd ++ List("--label","ply2geojson_ll_" + fmt.format(ll),"--style","tri_seg.qml"),
      kvrdd_seg, "seg", do_dump = false).collect()
    iq.run_pipe_fun_KValue(
      tri2geojson_wasure_cmd ++ List("--label","tri2geojson_ll_" + fmt.format(ll),"--style","tri_seg.qml"),
      iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false).collect()
  }
  val rdd_ply_surface = iq.run_pipe_fun_KValue(
    ext_cmd ++ List("--label","ext_ll_" + fmt.format(ll)),
    iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false)
  rdd_ply_surface.collect()
}

// stats_tri.last.map(x => (x.key,x.content.split(" ").last.toInt)).sortByKey().fold((0,0))( (x,y) => (x._1 , y._2 + y._2) )
// val stats_tri_sorted = stats_tri.last.map(x => (x.key,x.content.split(" ").map(_.toInt))).sortByKey().collect.map(x => (x._1.toInt,x._2))


// val stats_tri_sorted.map(x => List(x)).foldLeft(List((0,0)))((res,y) => res ::: List((y.last._1 + 1,res.last._2 +  y.head._2)))



kvrdd_log_list.map( x => x.map(y => y.data.split(" ", 5).last))

val time_array = kvrdd_log_list.map( x => x.map(y => y.data.split(" ", 5).last.split(";"))).reduce(_ ++ _).reduce(_ ++ _)
val time_couple = time_array.map(x => x.split(":")).map(x => (x(0),x(1)))
val time_full = kvrdd_log_list.map( x => x.map(y => y.data.split(" ", 5).last.split(";").map(x => x.split(":")).map(x => (x(0),x(1))))).reduce(_ ++ _).reduce(_ ++ _);

dump_stats(log_tri,nbp,nbt_side,"full",t0,output_dir,sc);



if(false){
  val rdd_json_tri = iq.run_pipe_fun_KValue(
    ply2geojson_cmd ++ List("--label", "tri","--style","tri_dst.qml"),
    kvrdd_dst, "ply2geo", do_dump = false)
  rdd_json_tri.collect()
  val res_pts_json = iq.run_pipe_fun_KValue(
    wasure_ply2geojson_cmd ++ List("--label", "wasure_dim"),
    kvrdd_dim, "ply2geo", do_dump = false)
  res_pts_json.collect()
  val res_input_json = iq.run_pipe_fun_KValue(
    ply2geojson_cmd ++ List("--label", "input_pts"),
    kvrdd_points, "ply2geo", do_dump = false)

  res_input_json.collect()
}

if(true){
  // val res_dst_ech = iq.run_pipe_fun_KValue(
  //   dst_cmd ++ List("--label", "dst"),
  //   input_dst.filter(x => x._1 >= 0), "dst", do_dump = false)
  // val kvrdd_dst = iq.get_kvrdd(res_dst_ech,"t");

  // val print = iq.run_pipe_fun_KValue(
  //   id_cmd ++ List("--label", "dst"),
  //   input_dst.filter(x => x._1 >= 0), "dst", do_dump = false)

  dim match {
    case 2 => {
      val rdd_json_dst = iq.run_pipe_fun_KValue(
        ply2geojson_cmd ++ List("--label", "dst","--style","tri_dst.qml"),
        kvrdd_dst, "dst", do_dump = false)
      rdd_json_dst.collect()
    }
    case _ => {}
  }



  // val rdd_json_dst = iq.run_pipe_fun_KValue(
  //    id_cmd ++ List("--label", "dst","--style","tri_dst.qml"),
  // kvrdd_dst, "dst", do_dump = false)
  // rdd_json_dst.collect()
}


// graph_tri.edges.collect().map(e => (e.srcId,e.dstId)).filter(x => (x._1 == 0 || x._2 ==0)).size




