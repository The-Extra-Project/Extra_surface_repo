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

// Checkpoint
val do_checkpoint = true
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
val pscale = params_scala.get_param("pscale", "1").toFloat
val nb_samples = params_scala.get_param("nb_samples", "3").toFloat
val rat_ray_sample = params_scala.get_param("rat_ray_sample", "1").toFloat
val min_ppt = params_scala.get_param("min_ppt", "50").toInt
val adaptative_scale = params_scala.get_param("adaptative_scale", "false").toBoolean

val fmt = new java.text.DecimalFormat("##0.##############")
val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
val datestring = dateFormatter.format(Calendar.getInstance().getTime());
val cur_output_dir ={output_dir  + sc.applicationId + "_" + datestring + "_"+ params_scala("name").head }
fs.mkdirs(new Path(cur_output_dir),new FsPermission("777"))


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
  ("output_dir",cur_output_dir),
  ("min_ppt",params_scala("min_ppt").head),
  ("seed",algo_seed)
))


val params_wasure =  set_params(params_new,List(
  ("exec_path", build_dir + "/bin/wasure-stream-exe"),
  ("dim",params_scala("dim").head),
  ("bbox",params_scala("bbox").head),
  ("lambda",params_scala("lambda").head),
  ("pscale",params_scala("pscale").head),
  ("rat_ray_sample",params_scala("rat_ray_sample").head),
  ("nb_samples",params_scala("nb_samples").head),
  ("mode",params_scala("mode").head),
  ("input_dir",input_dir),
  ("output_dir",cur_output_dir),
  ("seed",algo_seed)
))


if(false){
  params_ddt("dump_ply") = collection.mutable.Set("")
  params_wasure("dump_ply") = collection.mutable.Set("")
}
if(adaptative_scale)
  params_wasure("adaptative_scale") =  collection.mutable.Set("")

val fmt = new java.text.DecimalFormat("##0.##############")
val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
val floatFormat = new DecimalFormat("#.###")

val nbt_side = math.pow(2,ndtree_depth)
val tot_nbt = scala.math.pow(nbt_side,dim).toInt;
val nbp_per_tile = nbp/tot_nbt;
val rep_value = ((if((tot_nbt) < sc.defaultParallelism) sc.defaultParallelism else  (tot_nbt).toInt))
var nb_leaf = tot_nbt;

params_ddt("output_dir") = collection.mutable.Set(cur_output_dir)
params_scala("output_dir") = collection.mutable.Set(cur_output_dir)
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
val dump_ply_binary_cmd =  set_params(params_ddt, List(("step","dump_ply_binary"),("output_dir", cur_output_dir))).to_command_line
val id_cmd = List(build_dir + "/bin/identity-exe");

// Wausre surface reconstruction commands
val dim_cmd =  set_params(params_wasure, List(("step","dim"))).to_command_line
val dst_cmd =  set_params(params_wasure, List(("step","dst"))).to_command_line
val regularize_slave_cmd =  set_params(params_wasure, List(("step","regularize_slave"))).to_command_line
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



// =========== Start of the algorithm ==============
val t0 = System.nanoTime()
params_scala("t0") = collection.mutable.Set(t0.toString)
println("======== Tiling =============")
kvrdd_points = ddt_algo.compute_tiling_2(kvrdd_inputs,iq,params_ddt,params_scala);
nb_leaf = params_scala("nb_leaf").head.toInt;

val rep_merge = ((if((nb_leaf) < spark_core_max) spark_core_max else  nb_leaf));
var rep_loop = nb_leaf;
if(ndtree_depth > 8){
  rep_loop = spark_core_max*10;
  rep_merge = spark_core_max*10;
}
params_scala("rep_loop") = collection.mutable.Set(rep_loop.toString)
params_scala("rep_merge") = collection.mutable.Set(rep_merge.toString)
params_scala("dump_mode") = collection.mutable.Set("NONE")

println("======== Dimenstionnality =============")
val t0 = System.nanoTime()
params_scala("t0") = collection.mutable.Set(t0.toString)

val res_dim = iq.run_pipe_fun_KValue(
  dim_cmd ++ List("--label", "dim"),
  kvrdd_points, "dim", do_dump = false).persist(slvl_glob)
val kvrdd_dim = iq.get_kvrdd(res_dim,"z");
val kvrdd_simp = iq.get_kvrdd(res_dim,"x").reduceByKey((u,v) => u ::: v,rep_loop);


var input_ddt = kvrdd_points;
// If we have a simplified point cloud, do the delaunay triangulation of the simplfied point cloud
if(pscale > 0)
  input_ddt = kvrdd_simp;

println("=========== Delauay triangulatin computation ==================")
val defaultV = (List(""));
val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(
  kvrdd_points = input_ddt,
  iq = iq,
  params_cpp = params_ddt,
  params_scala = params_scala
);


val kvrdd_tri_gid = ddt_algo.update_global_ids(graph_tri.vertices,stats_tri,iq, params_ddt,sc)


val graph_tri_gid = Graph(kvrdd_tri_gid, graph_tri.edges, defaultV)
graph_tri_gid.vertices.setName("graph_tri_gid");
graph_tri_gid.edges.setName("graph_tri_gid");

val graph_pts = Graph(kvrdd_dim.reduceByKey( (u,v) => u ::: v), graph_tri.edges, List(""));
graph_pts.vertices.setName("graph_pts");
graph_pts.edges.setName("graph_pts");
val stats_kvrdd = kvrdd_simplex_id(stats_tri,sc)
val graph_stats = Graph(stats_kvrdd, graph_tri.edges, List(""));
val input_dst = (graph_tri_gid.vertices).union(iq.aggregate_value_clique(graph_pts, 1)).union(graph_stats.vertices).reduceByKey(_ ::: _).setName("input_dst");
input_dst.persist(slvl_glob);
graph_tri.vertices.unpersist()
graph_tri.edges.unpersist()
//val input_dst = (graph_tri.vertices).union(graph_pts.vertices).union(graph_stats.vertices).reduceByKey(_ ::: _).setName("input_dst");

val datastruct_identity_cmd =  set_params(params_ddt, List(("step","datastruct_identity"))).to_command_line
val struct_inputs_id = iq.run_pipe_fun_KValue(
  datastruct_identity_cmd ++ List("--label", "struct"),
  graph_pts.vertices, "struct", do_dump = false)


println("============= Simplex score computation ===============")
val res_dst = iq.run_pipe_fun_KValue(
  dst_cmd ++ List("--label", "dst"),
  input_dst, "dst", do_dump = false).persist(slvl_glob).setName("res_dst");
res_dst.count
res_dim.unpersist()
input_dst.unpersist()
kvrdd_points.unpersist();
val kvrdd_dst = iq.get_kvrdd(res_dst,"t");
val graph_dst = Graph(kvrdd_dst, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);

println("============= Regularize ===============")
val res_regularize = iq.run_pipe_fun_KValue(
  regularize_slave_cmd ++ List("--label", "regularize_slave"),
  iq.aggregate_value_clique(graph_dst, 1), "regularize", do_dump = false).persist(slvl_glob).setName("res_reg");
val kvrdd_reg = iq.get_kvrdd(res_regularize,"t");
val kvrdd_shr = iq.get_edgrdd(res_regularize,"e")
val graph_reg = Graph(kvrdd_reg, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);
 graph_reg.vertices.setName("graph_reg");
 graph_reg.edges.setName("graph_reg");


graph_pts.vertices.unpersist();
graph_tri_gid.vertices.unpersist();
graph_tri.vertices.unpersist();

// val graph_dst = Graph(kvrdd_reg, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);
// graph_dst.vertices.setName("graph_dst");
/// graph_dst.edges.setName("graph_dst");



println("============= Optimiation ===============")
val lambda_list = params_scala("lambda").map(_.toDouble).toList.sortWith(_ > _).map(fmt.format(_))

//val lambda_list = List("0.1","0.2","0.05")

var full_acc = 0;
val label = "coef_adapt"


def partition2ply(path_output : String, label : String){
  fs.listStatus(new Path(path_output)).filter(
    dd => (dd.isDirectory)).map(
    ss => fs.listStatus(ss.getPath)).reduce(_ ++ _).filter(
    xx => ((xx.getPath.toString contains "part-") && !(xx.getPath.toString contains ".ply"))).map(
    ff => fs.rename(ff.getPath, new Path(ff.getPath.toString + "_" + label + ".ply"))
  )
}

// Loop over the differents parameters


val it_list = List(20)
val lambda_list = List("0.1")
val coef_mult_list = List("110000000000".toLong)
val coef_mult_list = List("1000000".toLong)

val ll = lambda_list.head
val coef_mult = coef_mult_list.head
val max_it = it_list.head

val algo_list = List("seg_lagrange_weight","seg_lagrange_raw");
//val algo_list = List("seg_lagrange_weight");
/*
 val maxIterations = 1
 val nb_part = rep_merge
 */
//val coef_mult_list = List("5".toLong,"10".toLong,"50".toLong)


var stats_list_1 = new ListBuffer[(Int,(Float,Float))]()
var stats_list_2 = new ListBuffer[(Int,(Float,Float))]()


def dump_it_stats(stats_filename : String, stats_list : ListBuffer[(Int, (Float, Float))]){
  val fs = FileSystem.get(sc.hadoopConfiguration);
  val path_name = new Path(stats_filename);
  if (fs.exists(path_name))
    fs.delete(path_name, true)
  val output = fs.create(path_name);
  val os = new BufferedOutputStream(output)
  os.write((stats_list.map( x => x._1 + "," + x._2._1 + "," + x._2._2 + "," + x._2._1   /  (if(x._2._2 != 0) x._2._2.toFloat else 1)).reduce(_ + "\n" + _) + "\n").getBytes)
  os.close()
}


val algo_list = List("seg_lagrange_weight","seg_lagrange_raw");

if(true){
  algo_list.foreach{ cur_algo =>
    /// Start
    params_wasure("lambda") = collection.mutable.Set(ll)
    params_wasure("coef_mult") = collection.mutable.Set(coef_mult.toString)
    val datestring = dateFormatter.format(Calendar.getInstance().getTime());
    val ext_name = label + "_" + full_acc + "_ll_" + ll + "_cm_" + fmt.format(coef_mult) + "_" + fmt.format(max_it) + "_" + cur_algo + "_"  + datestring;
    full_acc+=1;
    if(false){
      println("==== Segmentation with lambda:" + ll + " coef_mult:" + coef_mult +  "  ====")
      val ext_cmd_vertex =  set_params(params_wasure, List(("step","extract_surface"),("area_processed","1"))).to_command_line
      val ext_cmd_edges =  set_params(params_wasure, List(("step","extract_surface"),("area_processed","2"))).to_command_line
      val graph_bp = Graph((graph_reg.vertices union graph_stats.vertices).reduceByKey(_ ::: _ ), graph_tri.edges, List(""))
      graph_bp.vertices.setName("graph_bp");
      graph_bp.edges.setName("graph_bp");
      val epsilon = 0.00000001;
      val kvrdd_seg = compute_belief_prop_v2(
        graph_bp,
        max_it,epsilon,
        stats_tri, params_wasure, iq, sc,rep_merge);
      val graph_seg = Graph(kvrdd_seg, graph_reg.edges, List(""));
      graph_seg.vertices.setName("graph_seg");
      graph_seg.edges.setName("graph_seg");
      // if (dim == 2)  {
      //   iq.run_pipe_fun_KValue(
      //     tri2geojson_cmd ++ List("--label","sparkcuted_v2_ll_" + ll,"--style","tri_seg.qml"),
      //     kvrdd_seg, "seg", do_dump = false).collect()
      // }
      if(false){
        val rdd_ply_surface_edges = iq.run_pipe_fun_KValue(
          ext_cmd_edges ++ List("--label","ext_spark_ll_v2_edge" + ext_name),
          graph_seg.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr)), "seg", do_dump = false)
        val rdd_ply_surface_vertex = iq.run_pipe_fun_KValue(
          ext_cmd_vertex ++ List("--label","ext_spark_ll_v2_tile" + ext_name),
          graph_seg.vertices, "seg", do_dump = false)
        val ply_dir_edges = cur_output_dir + "/ply" + ext_name + "_edges"
        val ply_dir_vertex = cur_output_dir + "/ply" + ext_name + "_vertex"
        ddt_algo.saveAsPly(rdd_ply_surface_edges,ply_dir_edges,plot_lvl)
        ddt_algo.saveAsPly(rdd_ply_surface_vertex,ply_dir_vertex,plot_lvl)
      }else{
        val rdd_ply_surface = iq.run_pipe_fun_KValue(
          ext_cmd ++ List("--label","ext_spark"  +  ext_name),
          iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false)
        val ply_dir = cur_output_dir + "/ply" + ext_name + "_edges_" + full_acc.toString
        ddt_algo.saveAsPly(rdd_ply_surface,ply_dir,plot_lvl)
        dump_json(params_wasure,ply_dir + "/params_wasure.json",sc);
        dump_json(params_scala,ply_dir + "/params_scala.json",sc);
      }
      partition2ply(cur_output_dir, full_acc.toString);
    }

    if(true){
      //val seg_cmd =  set_params(params_wasure, List(("step","seg"))).to_command_line
      val seg_cmd =  set_params(params_wasure, List(("step",cur_algo))).to_command_line
      var input_seg2 = //(iq.aggregate_value_clique(graph_reg, 1)
        (kvrdd_reg union kvrdd_shr.map(e => (e.srcId, e.attr))).reduceByKey(_ ::: _)
      input_seg2.persist(slvl_loop)
      input_seg2.count()
      var acc_loop = 0;
      val nb_it = 150;
      val mod_val = 10;
      while (acc_loop < nb_it) {
        val acc_loop_str = "%03d".format(acc_loop)
        val res_seg = iq.run_pipe_fun_KValue(
          seg_cmd ++ List("--label", "seg" + acc_loop_str),
          input_seg2, cur_algo, do_dump = false).persist(slvl_loop)
        res_seg.count()
        val kvrdd_seg = iq.get_kvrdd(res_seg,"t");
        input_seg2.unpersist()
        if(((acc_loop == nb_it -1) || acc_loop % mod_val == 0 || acc_loop == 1) ){
          val graph_seg = Graph(kvrdd_seg, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);
          val rdd_ply_surface = iq.run_pipe_fun_KValue(
            ext_cmd ++ List("--label","ext_seg" + ext_name + "_" + acc_loop_str),
            iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false)

          val ply_dir = cur_output_dir + "/plydist_" + ext_name + "_gc_" + full_acc.toString + "_" + acc_loop_str
          ddt_algo.saveAsPly(rdd_ply_surface,ply_dir,plot_lvl)
          partition2ply(cur_output_dir,full_acc.toString);
        }
        val rdd_local_edges = iq.get_edgrdd(res_seg,"e")
        val rdd_shared_edges = iq.get_edgrdd(res_seg,"f")
        val rdd_stats  = iq.get_kvrdd(res_seg,"s")
        var stats_1 = rdd_stats.map(x => x._2(0).split(" ").takeRight(2)).map(x => (x(0).toFloat,x(1).toFloat)).reduce( (x,y) => (x._1+y._1,x._2+y._2))
        var stats_2 = stats_1;

        input_seg2 = (
          rdd_local_edges.map(e => (e.srcId, e.attr)) union
            rdd_shared_edges.map(e => (e.dstId, e.attr)) union  kvrdd_seg
        ).reduceByKey(_ ::: _,rep_loop).persist(slvl_loop).setName("NEW_KVRDD_WITH_EDGES_" + acc_loop_str)
        input_seg2.count()

        if( acc_loop % mod_val == 0 || acc_loop == 1 ){
          val seg_cmd_full =  set_params(params_wasure, List(("step","seg_global_extract"))).to_command_line
          val input_extract = kvrdd_seg.map(x => (0L,x._2)).reduceByKey(_ ::: _)
          val res_surface = iq.run_pipe_fun_KValue(
            seg_cmd_full ++ List("--label", "seg"),
            input_extract , "seg_lagrange", do_dump = false)
          val rdd_ply  = res_surface.filter(_(0) == 'p');
          val rdd_stats  = iq.get_kvrdd(res_surface,"s")
          stats_2 = rdd_stats.map(x => x._2(0).split(" ").takeRight(2)).map(x => (x(0).toFloat,x(1).toFloat)).reduce( (x,y) => (x._1+y._1,x._2+y._2))
          rdd_stats.collect()
          if(acc_loop == 0){
            val ply_dir = cur_output_dir + "/plyglob_" + ext_name + "_gc_" + full_acc.toString + "_" + acc_loop_str + "_global3"
            ddt_algo.saveAsPly(rdd_ply,ply_dir,plot_lvl)
            partition2ply(cur_output_dir,full_acc.toString);
          }
        }
        if( acc_loop % mod_val == 0 || acc_loop == 1){
          val t1 = ( acc_loop,stats_1)
          val t2 = ( acc_loop,stats_2)
          stats_list_1 += t1
          stats_list_2 += t2
          println("[it " + acc_loop_str + "] " + floatFormat.format(100*stats_1._1/stats_1._2.toFloat) + "% "
            + stats_1 + " \t --- " + floatFormat.format(100*stats_2._1/stats_2._2.toFloat) + "% " + stats_2)
          dump_it_stats(cur_output_dir + "/" + cur_algo + "_stats_conv_1.txt",stats_list_1)
          dump_it_stats(cur_output_dir + "/" + cur_algo + "_stats_gtdiff_2.txt",stats_list_2)
        }
        res_seg.unpersist()
        acc_loop = acc_loop + 1;
      }

      stats_list_1.clear()
      stats_list_2.clear()
    }
  }
}



// iq.run_pipe_fun_KValue(
//   seg_cmd ++ List("--label", "seg"),
//   iq.aggregate_value_clique(graph_seg, 1),
//   "dst", do_dump = false).take(1)

// if(false){
//   acc += 1;
//   val datestring = dateFormatter.format(Calendar.getInstance().getTime());
//   val ext_name = label + "_" + acc + "_ll_" + ll + "_cm_" + fmt.format(coef_mult) + "_it_" + fmt.format(max_it) + "_" +  datestring;
//   val rdd_ply_surface_edges = iq.run_pipe_fun_KValue(
//     ext_cmd_edges ++ List("--label","ext_spark_ll_v2_edge" + ext_name),
//     graph_seg.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr)), "seg", do_dump = false)
//   val rdd_ply_surface_vertex = iq.run_pipe_fun_KValue(
//     ext_cmd_vertex ++ List("--label","ext_spark_ll_v2_tile" + ext_name),
//     graph_seg.vertices, "seg", do_dump = false)
//   val ply_dir_edges = cur_output_dir + "/ply" + ext_name + "_edges"
//   val ply_dir_vertex = cur_output_dir + "/ply" + ext_name + "_vertex"
//   ddt_algo.saveAsPly(rdd_ply_surface_edges,ply_dir_edges,plot_lvl)
//   ddt_algo.saveAsPly(rdd_ply_surface_vertex,ply_dir_vertex,plot_lvl)
//   partition2ply(cur_output_dir, acc.toString);
// }

// // ====== Dump geojson ======
// if(false){
//   dim match {
//     case 2 => {
//       val rdd_json_dst = iq.run_pipe_fun_KValue(
//         tri2geojson_cmd ++ List("--label", "dst","--style","tri_dst.qml"),
//         kvrdd_dst, "dst", do_dump = false)
//       rdd_json_dst.collect()

//       val res_pts_json = iq.run_pipe_fun_KValue(
//         wasure_ply2geojson_cmd ++ List("--label", "wasure_dim"),
//         kvrdd_dim, "ply2geo", do_dump = false)
//       res_pts_json.collect()
//       val rdd_json_input_ply = iq.run_pipe_fun_KValue(
//         ply2geojson_cmd ++ List("--label", "generated"),
//         kvrdd_points, "extract_tri_vrt_final", do_dump = false)
//       rdd_json_input_ply.collect()

//       // // Export Graph
//       // val rdd_graph = iq.run_pipe_fun_KValue(
//       //   extract_struct_cmd ++ List("--label", "extrac_struct"),
//       //   kvrdd_dst, "dst", do_dump = false)
//       // rdd_graph.collect()
//       // val exp = export_graph(iq.get_kvrdd(rdd_graph.filter(!_.isEmpty) ,"b"), graph_tri)
//       // val pw = new PrintWriter(new File(cur_output_dirx +"/graph.geojson" ))
//       // pw.write(exp)
//       // pw.close


//       val res_input_json = iq.run_pipe_fun_KValue(
//         ply2geojson_cmd ++ List("--label", "input_pts"),
//         kvrdd_points, "ply2geo", do_dump = false)
//       res_input_json.collect()

//     }
//     case _ => {}
//   }

//   // ======= Convert Raw ply to dataset example ==========
//   val dataset_raw = iq.run_pipe_fun_KValue(
//     ply2dataset_cmd ,
//     kvrdd_dst, "dst", do_dump = false).persist(slvl_glob)

//   val raw_header = dataset_raw.filter(x => x(0) == 'h').collect()(0)
//   val rdd_vert = dataset_raw.filter(x => x(0) == 'v').map(x => x.tail.tail).setName("VERTS_RDD_filter").persist(slvl_glob)
//   val rdd_simp = dataset_raw.filter(x => x(0) == 's').map(x => x.tail.tail).setName("Simplex_RDD_filter").persist(slvl_glob)
//   val schema_pts = get_header_schema(raw_header,1)
//   val schema_simplex = get_header_schema(raw_header,2)
//   val frame_pts = spark.read.option("delimeter", ",").schema(schema_pts).csv(rdd_vert.toDS).persist(slvl_glob)
//   val frame_simplex = spark.read.option("delimeter", ",").schema(schema_simplex).csv(rdd_simp.toDS).persist(slvl_glob)
//   frame_pts.show
//   frame_simplex.show
// }


// val rdd_graph = iq.run_pipe_fun_KValue(
//   extract_struct_cmd ++ List("--label", "extrac_struct"),
//   kvrdd_dst, "dst", do_dump = false)
//   rdd_graph.collect()
