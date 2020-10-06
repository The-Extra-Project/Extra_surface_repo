package iqlibbp

import org.apache.spark._
import org.apache.spark.graphx._
import org.apache.spark.rdd.RDD
import scala.collection.mutable.HashMap
import scala.collection.mutable.Set
import sys.process._
import org.apache.spark.graphx.util.GraphGenerators
import java.io._
import sparkle.graph._
import org.apache.spark.storage.StorageLevel

import org.apache.spark.graphx._;
import org.apache.spark.graphx.PartitionStrategy._

import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;

import algo_stats._;


import iqlibflow.mflow._;
object algo_iqlibbp {
  /*
   maxFlow function

   Arguments: sourceId (type: VertexId)
   targetId (type: VertexId)*
   graph (type: Graph[Long,Int])
   Returns: the maximum flows as RDD[(VertexId,VertexId),Int]

   Note that the maxFlow function is defined below the shortestPath function
   */


      def compute_graph_cut(graph_bp: TGraph , maxIterations : Int, epsilon : Double, stats_tri : RDD[KValue], params_wasure : params_map, iq : IQlibSched, sc : SparkContext,nb_part : Int): RDD[KValue]  = {

      val input_vertex : RDD[KValue] =  graph_bp.vertices
//      input_vertex.persist(iq.get_storage_level())
      val input_edges : RDD[KValue] =  graph_bp.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr))

      val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
      val extract_graph_cmd_local =  set_params(params_wasure, List(("step","extract_graph"),("graph_type","1"),("area_processed","1"))).to_command_line
      val extract_graph_cmd_shared =  set_params(params_wasure, List(("step","extract_graph"),("graph_type","1"),("area_processed","2"))).to_command_line


    val full_graph_local = iq.run_pipe_fun_KValue(
      extract_graph_cmd_local,
      input_vertex, "ext_gr", do_dump = false)


    val full_graph_shared = iq.run_pipe_fun_KValue(
      extract_graph_cmd_shared, 
      input_edges, "ext_gr", do_dump = false)

    val full_graph_merged =  (full_graph_local union full_graph_shared);

    val beliefs = graph_cut(full_graph_merged,maxIterations,epsilon,iq,sc,nb_part)

    val summed_id = sum_simplex_id(stats_tri);
    val res_belief_str = graphcut_2_kvrdd(beliefs,summed_id);
      res_belief_str.persist(iq.get_storage_level()).setName("res_belief_str")
      res_belief_str.count()


    val input_extract_graph = (input_vertex).union(res_belief_str).reduceByKey(_ ::: _,nb_part)

    val res_seg_mf = iq.run_pipe_fun_KValue(
      fill_graph_cmd ++ List("--label", "sparkcuted_full"),
      input_extract_graph, "ext_gr", do_dump = false)

      res_seg_mf.persist(iq.get_storage_level()).setName("res_seg_mf");
      res_seg_mf.count()
      res_belief_str.unpersist();
      input_vertex.unpersist();

    val kvrdd_seg = iq.get_kvrdd(res_seg_mf,"t");
    return kvrdd_seg;

  } 

    def compute_belief_prop_v2(graph_bp: TGraph , maxIterations : Int, epsilon : Double, stats_tri : RDD[KValue], params_wasure : params_map, iq : IQlibSched, sc : SparkContext,nb_part : Int): RDD[KValue]  = {

      val input_vertex : RDD[KValue] =  graph_bp.vertices
//      input_vertex.persist(iq.get_storage_level())
      val input_edges : RDD[KValue] =  graph_bp.convertToCanonicalEdges().triplets.map(ee => (ee.srcId,ee.srcAttr ++ ee.dstAttr))

      val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
      val extract_graph_cmd_local =  set_params(params_wasure, List(("step","extract_graph"),("graph_type","0"),("area_processed","1"))).to_command_line
      val extract_graph_cmd_shared =  set_params(params_wasure, List(("step","extract_graph"),("graph_type","0"),("area_processed","2"))).to_command_line
    // ============ Spark Belied propagation =======
    // Get global id by tile for each simplex
    // val tot_simlex = summed_id.last._2
    // val kvrdd_tile_id : RDD[KValue] = sc.parallelize(
    //   summed_id.dropRight(1).map(x =>
    //     (x._1.toLong, List("s 1 " + x._1.toString +  " s " + x._2.map(_.toString).reduce((y,z) => y + " " + z)))
    //   )
    // )
    // val input_extract_simplex = (graph_dst.vertices).union(kvrdd_tile_id).reduceByKey(_ ::: _);


    val full_graph_local = iq.run_pipe_fun_KValue(
      extract_graph_cmd_local,
      input_vertex, "ext_gr", do_dump = false)


    val full_graph_shared = iq.run_pipe_fun_KValue(
      extract_graph_cmd_shared, 
      input_edges, "ext_gr", do_dump = false)

    val full_graph_merged =  (full_graph_local union full_graph_shared);

    val beliefs = belief_prop(full_graph_merged,maxIterations,epsilon,iq,nb_part)

    val summed_id = sum_simplex_id(stats_tri);
    val res_belief_str = belief_2_kvrdd(beliefs,summed_id);
      res_belief_str.persist(iq.get_storage_level()).setName("res_belief_str")
      res_belief_str.count()


    val input_extract_graph = (input_vertex).union(res_belief_str).reduceByKey(_ ::: _,nb_part)

    val res_seg_mf = iq.run_pipe_fun_KValue(
      fill_graph_cmd ++ List("--label", "sparkcuted_full"),
      input_extract_graph, "ext_gr", do_dump = false)

      res_seg_mf.persist(iq.get_storage_level()).setName("res_seg_mf");
      res_seg_mf.count()
      res_belief_str.unpersist();
      input_vertex.unpersist();
    val kvrdd_seg = iq.get_kvrdd(res_seg_mf,"t");
    return kvrdd_seg;

  }


  // def compute_belief_prop_v1(input_extract_simplex: RDD[KValue] , maxIterations : Int, epsilon : Double, stats_tri : RDD[KValue], params_wasure : params_map, iq : IQlibSched, sc : SparkContext, nb_part : Int): RDD[KValue]  = {

  //   val fill_graph_cmd =  set_params(params_wasure, List(("step","fill_graph"))).to_command_line
  //   val extract_graph_cmd =  set_params(params_wasure, List(("step","extract_graph"),("graph_type","0"))).to_command_line
  //   // ============ Spark Belied propagation =======
  //   // Get global id by tile for each simplex


  //   // val tot_simlex = summed_id.last._2
  //   // val kvrdd_tile_id : RDD[KValue] = sc.parallelize(
  //   //   summed_id.dropRight(1).map(x =>
  //   //     (x._1.toLong, List("s 1 " + x._1.toString +  " s " + x._2.map(_.toString).reduce((y,z) => y + " " + z)))
  //   //   )
  //   // )

  //   // val input_extract_simplex = (graph_dst.vertices).union(kvrdd_tile_id).reduceByKey(_ ::: _);


  //   val full_graph = iq.run_pipe_fun_KValue(
  //     extract_graph_cmd,
  //     input_extract_simplex, "ext_gr", do_dump = false)


  //   val beliefs = belief_prop(full_graph,maxIterations,epsilon,nb_part)

  //   val summed_id = sum_simplex_id(stats_tri);
  //   val res_belief_str = belief_2_kvrdd(beliefs,summed_id);

  //   val input_extract_graph = (input_extract_simplex).union(res_belief_str).reduceByKey(_ ::: _,16).persist(StorageLevel.MEMORY_AND_DISK);

  //   val res_seg_mf = iq.run_pipe_fun_KValue(
  //     fill_graph_cmd ++ List("--label", "sparkcuted_full"),
  //     input_extract_graph, "ext_gr", do_dump = false)

  //   val kvrdd_seg = iq.get_kvrdd(res_seg_mf,"t");
  //   return kvrdd_seg;


  // }

  def belief_prop(full_graph_merged: RDD[VData] , maxIterations: Int, epsilon : Double,iq : IQlibSched,nb_part : Int):  Graph[PVertex,PEdge]  = {
    println(" belief prop : Init")
    val edges_cpp = full_graph_merged.filter(x => !x.isEmpty).filter(_(0) == 'e').map(_.substring(2))
    val nodes_cpp = full_graph_merged.filter(x => !x.isEmpty).filter(_(0) == 'v').map(_.substring(2))
    val vertexRDD: RDD[(Long, PVertex)] = nodes_cpp.map { line =>
      val fields = line.split(' ')
      val id = fields(0).toLong
      val prior = Variable(fields.slice(1,3).map(x =>  x.toDouble))
      val belief = Variable(prior.cloneValues) // Variable(fields.tail.slice(3,5).map(_.toDouble)) //
      (id, PVertex(belief, prior))
    }
    val edgeRDD = edges_cpp.map { line =>
      val fields = line.split(' ')
      val srcId = fields(0).toLong
      val dstId = fields(1).toLong
      val factor = fields.tail.tail.map(x => x.toDouble)
      Edge(srcId, dstId, factor)
    }

    edgeRDD.setName("INPUT_BG")
    vertexRDD.setName("INPUT_BG")

    val tmpGraph = Graph(vertexRDD, edgeRDD)

    // fixing the dimensions of factor based on vertexes

    val graph = tmpGraph.mapTriplets { triplet =>
      val srcSize = triplet.srcAttr.prior.size
      val dstSize = triplet.dstAttr.prior.size
      PEdge(Factor(Array(srcSize, dstSize), triplet.attr), Variable.fill(srcSize)(0.0), Variable.fill(dstSize)(0.0))
    }.partitionBy(EdgePartition1D,nb_part)

    graph.vertices.setName("INPUT_BG_2")
    graph.edges.setName("INPUT_BG_2")

    println(" belief prop : start belief")
    //val beliefs = PairwiseBP(graph, maxIterations, epsilon,iq.get_storage_level())
    val beliefs = PairwiseBP2(graph, maxIterations, epsilon)
    graph.unpersist()
    graph.unpersist()
    return beliefs
  }


  def graph_cut(full_graph_merged: RDD[VData] , maxIterations: Int, epsilon : Double,iq : IQlibSched,sc : SparkContext,nb_part : Int):  RDD[(Int, Int)]  = {
    println(" belief prop : Init")
    val edges_cpp = full_graph_merged.filter(x => !x.isEmpty).filter(_(0) == 'e').map(_.substring(2))
    val nodes_cpp = full_graph_merged.filter(x => !x.isEmpty).filter(_(0) == 'v').map(_.substring(2))
    val vertexRDD = nodes_cpp.map(x => x.split(" ")).map(x => (x(0).toLong,x(1).toLong));
    val edgesRDD = edges_cpp.map( x=> x.split(" ")).filter(_.size > 1).map( x => org.apache.spark.graphx.Edge(x(0).toLong,x(1).toLong,x(2).toFloat.toInt))

    val graph = Graph(vertexRDD, edgesRDD)
    val graph_2 = graph.convertToCanonicalEdges((ee1, ee2) => ee1 + ee2)
    edgesRDD.setName("INPUT_BG")
    vertexRDD.setName("INPUT_BG")
    val sourceId_ex: VertexId = 0L
    val targetId_ex: VertexId = 1L
    //    val graph : org.apache.spark.graphx.Graph[Long,Int] = Graph(vertexRDD,edgesRDD)
    // fixing the dimensions of factor based on vertexes
    val flows_ex = maxFlow(sourceId_ex, targetId_ex, graph,sc,maxIterations)

    val res_score_ex = (graph_2.edges.map(x => ((x.srcId,x.dstId),x.attr)) union flows_ex).reduceByKey((u,v) => Math.abs(u - v))
    val res_spark = res_score_ex.filter(
      {case ((u,v),w) => u <2 || v < 2 } // Keep only edges connected to source or target
    ).map(
      {case ((u,v),w) => if(u >1) (u,(v,w)) else (v,(u,w))}
    ).reduceByKey(
      (x1,x2) => if(x1._2 < x2._2) x1 else x2
    ).map( x=> (x._1.toInt,x._2._1.toInt))

    return res_spark
    }


  def graphcut_2_kvrdd(res_spark : RDD[(Int, Int)], summed_id : List[(Int, List[Int])]) : RDD[KValue] = {
    //val res_belief_test = nodes_cpp.map( x => x.split(" ")).map(x => (x(0).toInt,x(1).toInt))
    val res_belief_str = res_spark.map(
      x => (x._1 -2, x._2)).map(
      {case (u,v) => ( summed_id(summed_id.indexWhere(x => x._2(2) > (u) )-1)._1,(1,u.toString + " " + v.toString))  }
    ).reduceByKey(
      (v1,v2) => ((v1._1 + v2._1), v1._2 + " " + v2._2)
    ).map( x => (x._1.toLong, List("l 1 " + x._1.toString + " s " + x._2._1 + " " + x._2._2)) )
    return res_belief_str
  }

  def belief_2_kvrdd(beliefs : Graph[PVertex,PEdge], summed_id : List[(Int, List[Int])]) : RDD[KValue] = {
    val res_belief = beliefs.vertices.map(
      x => (x._1,x._2.belief.cloneValues)
    ).map( {case (id,bl) => (id,if(bl(0) > bl(1)) 0 else 1)} )

    //val res_belief_test = nodes_cpp.map( x => x.split(" ")).map(x => (x(0).toInt,x(1).toInt))

    val res_belief_str = res_belief.map(
      {case (u,v) => ( summed_id(summed_id.indexWhere(x => x._2(2) > (u) )-1)._1,(1,u.toString + " " + v.toString))  }
    ).reduceByKey(
      (v1,v2) => ((v1._1 + v2._1), v1._2 + " " + v2._2)
    ).map( x => (x._1.toLong, List("l 1 " + x._1.toString + " s " + x._2._1 + " " + x._2._2)) )
    return res_belief_str
  }

}



