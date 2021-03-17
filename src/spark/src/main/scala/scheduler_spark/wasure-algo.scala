package spark_ddt.wasure

import sys.process._
import scala.io.Source
import java.io._
import scala.xml._
import java.lang.Double
import scala.concurrent._
import scala.collection.parallel._
import java.nio.file.{ Paths, Files }
import scala.collection.mutable.ListBuffer

import java.text.DecimalFormat
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
import org.apache.spark.HashPartitioner


import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
import org.apache.hadoop.fs.permission.FsPermission

import spark_ddt.core._;
import spark_ddt.util._;
import spark_ddt.bp_algo._;
import spark_ddt.ddt_algo._;
import spark_ddt.bp_algo._;
import algo_stats._;
import spark_ddt.core.IQlibCore._;
import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;
import tiling._;
import geojson_export._;


object wasure_algo {



def dump_it_stats(stats_filename : String, stats_list : ListBuffer[(Int, (Float, Float))], sc : SparkContext){
  val fs = FileSystem.get(sc.hadoopConfiguration);
  val path_name = new Path(stats_filename);
  if (fs.exists(path_name))
    fs.delete(path_name, true)
  val output = fs.create(path_name);
  val os = new BufferedOutputStream(output)
  os.write((stats_list.map( x => x._1 + "," + x._2._1 + "," + x._2._2 + "," + x._2._1   /  (if(x._2._2 != 0) x._2._2.toFloat else 1)).reduce(_ + "\n" + _) + "\n").getBytes)
  os.close()
}


  def partition2ply(path_output : String, label : String, sc : SparkContext){
    val fs = FileSystem.get(sc.hadoopConfiguration);
    fs.listStatus(new Path(path_output)).filter(
      dd => (dd.isDirectory)).map(
      ss => fs.listStatus(ss.getPath)).reduce(_ ++ _).filter(
      xx => ((xx.getPath.toString contains "part-") && !(xx.getPath.toString contains ".ply"))).map(
      ff => fs.rename(ff.getPath, new Path(ff.getPath.toString + "_" + label + ".ply"))
    )
  }


//   val algo_list = List("seg_lagrange_weight","seg_lagrange_raw");
// algo_list.foreach{ cur_algo =>
//   wasure_algo.compute_optimization(res_regularize,graph_tri,graph_stats, stats_tri, ll, coef_mult, max_it, cur_algo,label, iq,params_wasure,params_scala,sc)
// }


  def compute_optimization(res_regularize : RDD[VData] ,graph_tri : TGraph ,graph_stats : TGraph, stats_tri:RDD[KValue],
    ll : String, coef_mult : Float, max_it : Int,cur_algo : String, label : String,
    iq : IQlibSched,params_wasure : params_map,params_scala : params_map,
    sc : SparkContext){

    val floatFormat = new DecimalFormat("#.###")
    val ext_cmd =  set_params(params_wasure, List(("step","extract_surface"))).to_command_line
    val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
    val cur_output_dir = params_scala("output_dir").head;
    val slvl_loop = iq.get_storage_level_loop();
    val rep_loop = params_scala("rep_loop").head.toInt;
    val rep_merge = params_scala("rep_merge").head.toInt;
    val kvrdd_reg = iq.get_kvrdd(res_regularize,"t");
    val kvrdd_shr = iq.get_edgrdd(res_regularize,"e")
    val graph_reg = Graph(kvrdd_reg, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);
 graph_reg.vertices.setName("graph_reg");
 graph_reg.edges.setName("graph_reg");

    var full_acc = 0;
    /// Start
    params_wasure("lambda") = collection.mutable.Set(ll)
    params_wasure("coef_mult") = collection.mutable.Set(coef_mult.toString)
    val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
    val fmt = new java.text.DecimalFormat("##0.##############")

    val datestring = dateFormatter.format(Calendar.getInstance().getTime());
    val ext_name = label + "_" + full_acc + "_ll_" + ll + "_cm_" + fmt.format(coef_mult) + "_" + fmt.format(max_it) + "_" + cur_algo + "_"  + datestring;
    full_acc+=1;

    var stats_list_1 = new ListBuffer[(Int,(Float,Float))]()
var stats_list_2 = new ListBuffer[(Int,(Float,Float))]()


    if(false){
      // wasure_algo.run_belief_prop
    }
    if(true){
      //val seg_cmd =  set_params(params_wasure, List(("step","seg"))).to_command_line
      val seg_cmd =  set_params(params_wasure, List(("step",cur_algo))).to_command_line
      var input_seg2 = //(iq.aggregate_value_clique(graph_reg, 1)
        (kvrdd_reg union kvrdd_shr.map(e => (e.srcId, e.attr))).reduceByKey(_ ::: _)
      input_seg2.persist(slvl_loop)
      input_seg2.count()
      var acc_loop = 0;
      val nb_it = 30;
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
          partition2ply(cur_output_dir,full_acc.toString,sc);
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
            partition2ply(cur_output_dir,full_acc.toString,sc);
          }
        }
        if( acc_loop % mod_val == 0 || acc_loop == 1){
          val t1 = ( acc_loop,stats_1)
          val t2 = ( acc_loop,stats_2)
          stats_list_1 += t1
          stats_list_2 += t2
          println("[it " + acc_loop_str + "] " + floatFormat.format(100*stats_1._1/stats_1._2.toFloat) + "% "
            + stats_1 + " \t --- " + floatFormat.format(100*stats_2._1/stats_2._2.toFloat) + "% " + stats_2)
          dump_it_stats(cur_output_dir + "/" + cur_algo + "_stats_conv_1.txt",stats_list_1,sc)
          dump_it_stats(cur_output_dir + "/" + cur_algo + "_stats_gtdiff_2.txt",stats_list_2,sc)
        }
        res_seg.unpersist()
        acc_loop = acc_loop + 1;
      }

      stats_list_1.clear()
      stats_list_2.clear()
    }
  }




  // def run_belief_prop(graph_reg : TGraph,graph_stats : TGraph, stats_tri:RDD[KValue],
  //   ll : Int, coef_mult : Float, max_it : Int, ext_name : String,
  //   iq : IQlibSched,params_wasure : params_map,params_scala : params_map,
  //   sc : SparkContext){



// val kvrdd_reg = iq.get_kvrdd(res_regularize,"t");
// val kvrdd_shr = iq.get_edgrdd(res_regularize,"e")
// val graph_reg = Graph(kvrdd_reg, graph_tri.edges, List("")).partitionBy(EdgePartition1D,rep_merge);
//  graph_reg.vertices.setName("graph_reg");
//  graph_reg.edges.setName("graph_reg");



  //   val rep_merge = params_scala("rep_merge").head.toInt;
  //   val cur_output_dir = params_scala("output_dir").head;

  //     println("==== Segmentation with lambda:" + ll + " coef_mult:" + coef_mult +  "  ====")
  //     val ext_cmd_vertex =  set_params(params_wasure, List(("step","extract_surface"),("area_processed","1"))).to_command_line
  //     val ext_cmd_edges =  set_params(params_wasure, List(("step","extract_surface"),("area_processed","2"))).to_command_line
  //     val graph_bp = Graph((graph_reg.vertices union graph_stats.vertices).reduceByKey(_ ::: _ ), graph_reg.edges, List(""))
  //     graph_bp.vertices.setName("graph_bp");
  //     graph_bp.edges.setName("graph_bp");
  //     val epsilon = 0.00000001;
  //     val kvrdd_seg = compute_belief_prop_v2(
  //       graph_bp,
  //       max_it,epsilon,
  //       stats_tri, params_wasure, iq, sc,rep_merge);
  //     val graph_seg = Graph(kvrdd_seg, graph_reg.edges, List(""));
  //     graph_seg.vertices.setName("graph_seg");
  //     graph_seg.edges.setName("graph_seg");
  //     // if (dim == 2)  {
  //     //   iq.run_pipe_fun_KValue(
  //     //     tri2geojson_cmd ++ List("--label","sparkcuted_v2_ll_" + ll,"--style","tri_seg.qml"),
  //     //     kvrdd_seg, "seg", do_dump = false).collect()
  //     // }
  //     if(false){
  //       val rdd_ply_surface_edges = iq.run_pipe_fun_KValue(
  //         ext_cmd_edges ++ List("--label","ext_spark_ll_v2_edge" + ext_name),
  //         graph_seg.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr)), "seg", do_dump = false)
  //       val rdd_ply_surface_vertex = iq.run_pipe_fun_KValue(
  //         ext_cmd_vertex ++ List("--label","ext_spark_ll_v2_tile" + ext_name),
  //         graph_seg.vertices, "seg", do_dump = false)
  //       val ply_dir_edges = cur_output_dir + "/ply" + ext_name + "_edges"
  //       val ply_dir_vertex = cur_output_dir + "/ply" + ext_name + "_vertex"
  //       ddt_algo.saveAsPly(rdd_ply_surface_edges,ply_dir_edges,plot_lvl)
  //       ddt_algo.saveAsPly(rdd_ply_surface_vertex,ply_dir_vertex,plot_lvl)
  //     }else{
  //       val rdd_ply_surface = iq.run_pipe_fun_KValue(
  //         ext_cmd ++ List("--label","ext_spark"  +  ext_name),
  //         iq.aggregate_value_clique(graph_seg, 1), "seg", do_dump = false)
  //       val ply_dir = cur_output_dir + "/ply" + ext_name + "_edges_" + full_acc.toString
  //       ddt_algo.saveAsPly(rdd_ply_surface,ply_dir,plot_lvl)
  //       dump_json(params_wasure,ply_dir + "/params_wasure.json",sc);
  //       dump_json(params_scala,ply_dir + "/params_scala.json",sc);
  //     }
  //     partition2ply(cur_output_dir, full_acc.toString);
  // }
}




//

