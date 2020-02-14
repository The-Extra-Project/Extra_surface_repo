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
import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import iqlib_algo._;
import iqlibflow._;
import iqlibbp._;
import tiling._;

import algo_stats._;
import xml_parsing._;
import dataset_processing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;
import mflow._;
import algo_iqlibbp._;
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
if (output_dir.isEmpty ||  input_dir.isEmpty || !Files.exists(Paths.get(env_xml)))
{
  System.err.println("ERROR")
  System.err.println("Bash variable are empy or ")
  System.err.println("File params " + env_xml +  " does not exist")
  System.exit(1)
}

// Get Params list from xml
var param_list = parse_xml_datasets_2(env_xml)
val df_par = sc.defaultParallelism;
val params_scala = param_list(0) // We only process 1 set of parameter in this workflow



// ===============================================
// ==== Scala and param initialization ===========
// Param scala is mutable, get params set the default value to the collection if it's empty
//  Se for instance the xml documentation / Algorithm params for the effect

// System params
val dim = params_scala.get_param("dim", "2").toInt
val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-D" + dim.toString)
val build_dir = global_build_dir + "/" + ddt_kernel_dir
val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "MEMORY_AND_DISK_SER"))

// General Algo params
val bbox = params_scala.get_param("bbox", "")
val do_profile = params_scala.get_param("do_profile", "false").toBoolean;
val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
val regexp_filter = params_scala.get_param("regexp_filter", "");
val max_ppt = params_scala.get_param("max_ppt", "1000").toInt
val ndtree_depth = params_scala.get_param("ndtree_depth", "4").toInt
val nbp =  params_scala.get_param("nbp", "10000").toInt
val datatype =  params_scala.get_param("datatype", "")
val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt
val algo_seed =  params_scala.get_param("algo_seed",scala.util.Random.nextInt(100000).toString);
val dump_mode = params_scala.get_param("dump_mode", "0").toInt
val min_ppt = params_scala.get_param("min_ppt", "0").toInt



// Set the iq library on
val iq = new IQlibSched(slvl_glob,slvl_loop)

// Set the c++ command line object
val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
val params_cpp =  set_params(params_new,List(
  ("exec_path", build_dir + "/bin/ddt-stream-exe"),
  ("bbox",params_scala("bbox").head),
  ("ech_input","1"),
  ("input_dir",input_dir),
  ("output_dir",output_dir),
  ("min_ppt",params_scala("min_ppt").head),
  ("dump_mode",params_scala("dump_mode").head),
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

params_cpp("output_dir") = collection.mutable.Set(output_dir)
params_scala("output_dir") = collection.mutable.Set(output_dir)
params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)
params_cpp("nbt_side") =  collection.mutable.Set(nbt_side.toString)
params_scala("df_par") = collection.mutable.Set(4.toString);
println("")
println("=======================================================")
params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))

// General c++ commands
val ply2geojson_cmd =  set_params(params_cpp, List(("step","ply2geojson"))).to_command_line
val ply2dataset_cmd =  set_params(params_cpp, List(("step","ply2dataset"))).to_command_line
val extract_graph_local_cmd =  set_params(params_cpp, List(("step","extract_graph"),("area_processed","1"))).to_command_line
val extract_graph_shared_cmd =  set_params(params_cpp, List(("step","extract_graph"),("area_processed","2"))).to_command_line
val dump_ply_binary_cmd =  set_params(params_cpp, List(("step","dump_ply_binary"),("output_dir", output_dir))).to_command_line
val tri2geojson_cmd =  set_params(params_cpp, List(("step","tri2geojson"))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");
val update_global_id_cmd =  set_params(params_cpp, List(("step","update_global_id"))).to_command_line


// =================================================
// ============  Parsing and init data ===========
var kvrdd_points: RDD[KValue] = sc.parallelize(List((0L,List(""))));
var kvrdd_inputs = format_data(
    params_scala,
    params_cpp,
    global_build_dir,
    ddt_main_dir,
    input_dir,
    df_par,
    sc ,
    iq
  )

if(plot_lvl >= 3){
  // if(dim == 2){
  //   iq.run_pipe_fun_KValue(
  //     tri2geojson_cmd ++ List("--label", "kvrdd_input"),
  //     kvrdd_inputs, "kvrdd_input", do_dump = false).collect()
  // }
  if(dim == 3){
    iq.run_pipe_fun_KValue(
      dump_ply_binary_cmd ++ List("--label", "tile_pts"),
      kvrdd_inputs, "tile_pts", do_dump = false).collect()
  }
}



// =========== Start of the algorithm ==============
println("======== Tiling =============")
val t0 = System.nanoTime()
params_scala("t0") = collection.mutable.Set(t0.toString)
kvrdd_points = ddt_algo.compute_tiling_2(kvrdd_inputs,iq,params_cpp,params_scala);
nb_leaf = params_scala("nb_leaf").head.toInt;

val rep_merge = ((if((nb_leaf) < spark_core_max) spark_core_max else  nb_leaf));
var rep_loop = nb_leaf;
if(ndtree_depth == 8)
  rep_loop = spark_core_max*10;
params_scala("rep_loop") = collection.mutable.Set(rep_loop.toString)
params_scala("rep_merge") = collection.mutable.Set(rep_merge.toString)




var input_ddt = kvrdd_points;
println("=========== Delauay triangulatin computation ==================")
val (graph_tri,log_tri,kvrdd_stats)  = ddt_algo.compute_ddt(
  kvrdd_points = input_ddt,
  iq = iq,
  params_cpp = params_cpp,
  params_scala = params_scala
);


println("========= PLY extraction =============")
if(dump_mode > 0){
  fs.listStatus(new Path(output_dir)).filter(
    dd => (dd.isDirectory)).map(
    ss => fs.listStatus(ss.getPath)).reduce(_ ++ _).filter(
    xx => (xx.getPath.toString contains "part-")).map(
    ff => fs.rename(ff.getPath, new Path(ff.getPath.toString + ".ply"))
  )
}


if(dim == 2){
  val stats_cum = kvrdd_simplex_id(kvrdd_stats,sc)

  val res_tri_gid = iq.run_pipe_fun_KValue(
    update_global_id_cmd,
    (graph_tri.vertices union stats_cum).reduceByKey(_ ::: _), "ext_gr", do_dump = false).filter(!_.isEmpty())
  val kvrdd_gid_tri = iq.get_kvrdd(res_tri_gid)
  val graph_full = Graph((kvrdd_gid_tri union stats_cum).reduceByKey(_ ::: _) , graph_tri.edges, List(""))
  val input_vertex : RDD[KValue] =  graph_full.vertices
  val input_edges : RDD[KValue] =  graph_full.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr))

  val full_graph_local = iq.run_pipe_fun_KValue(
    extract_graph_local_cmd,
    input_vertex, "ext_gr", do_dump = false).filter(!_.isEmpty())

  val full_graph_shared = iq.run_pipe_fun_KValue(
    extract_graph_shared_cmd,
    input_edges, "ext_gr", do_dump = false).filter(!_.isEmpty())

  val tri_vertex = full_graph_local.filter(x => x(0) == 'v').map(
    x => x.split(" ")).map(x => x.splitAt(2)).map(cc => (cc._1(1).toLong, cc._2.map(_.toDouble)))
  val tri_simplex = full_graph_local.filter(x => x(0) == 's').filter(x => x.count(_ == 's') == 1).map(
    x => x.split(" ")).map(x => x.splitAt(2)).map(cc => (cc._1(1).toLong, cc._2.map(_.toDouble)))
  val tri_edges = (full_graph_local.filter(x => x(0) == 'e') union full_graph_shared.filter(x => x(0) == 'e')).map(
    x => x.split(" ")).map(cc => Edge(cc(1).toLong, cc(2).toLong,""))

  val g_voronoi = Graph(tri_simplex,tri_edges)

  val geojson_header= raw"""{
    "type": "FeatureCollection",
    "features": [
    """
  val geojson_footer = raw"""
     ]
     }
  """

  val geojson_fh = raw"""
     {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [
  """
  val geojson_fb =   raw"""
        ]
      }
    }
  """
  val vr_str = geojson_header + g_voronoi.triplets.filter(tt => tt.dstAttr != null &&  tt.srcAttr != null).map(
    tt => geojson_fh + "[" + tt.srcAttr(0) + "," + tt.srcAttr(1) + "],[" + tt.dstAttr(0) + "," + tt.dstAttr(1) + "]" + geojson_fb ).reduce(_ + ","+_) + geojson_footer

  val bname =  params_scala("output_dir").head +"/voronoi";
  val pw1 = new PrintWriter(new File(bname + ".geojson" ))
  pw1.write(vr_str)
  pw1.close
}
// {
//   "type": "FeatureCollection",
//   "features": [
//      {
//       "type": "Feature",
//       "geometry": {
//         "type": "LineString",
//         "coordinates": [
//           [102.0, 0.0], [103.0, 1.0], [104.0, 0.0], [105.0, 1.0]
//         ]
//       },
//       "properties": {
//         "prop0": "value0",
//         "prop1": 0.0
//       }
//      },
//   ]
// }
