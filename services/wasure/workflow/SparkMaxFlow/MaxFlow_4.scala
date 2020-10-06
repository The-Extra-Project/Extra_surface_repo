import org.apache.spark._
import org.apache.spark.graphx._
import org.apache.spark.rdd.RDD
import scala.collection.mutable.HashMap
import scala.collection.mutable.Set
import sys.process._



import org.apache.spark._
import org.apache.spark.graphx._
import org.apache.spark.rdd.RDD
import scala.collection.mutable.HashMap
import scala.collection.mutable.Set
import org.apache.spark.graphx.util.GraphGenerators
import java.io._


// Iqlib Import
import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import iqlib_algo._;
import iqlibflow._;
import iqlibflow2._;

import algo_stats._;
import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;
import mflow2._;
import mflow._;


import sparkle.graph._

def export_graph(
  gg : org.apache.spark.graphx.Graph[Long,Int],
  ff : org.apache.spark.rdd.RDD[((org.apache.spark.graphx.VertexId, org.apache.spark.graphx.VertexId), Int)],
  sid: VertexId,
  tid: VertexId
){
  val pw = new PrintWriter(new File("/home/laurent/shared_spark/outputs/out.dot"))
  val str = gg.edges.collect().map(
    ee => ee.srcId + " -> " + ee.dstId + " [ " +
      " label=\"" + ee.attr + "(" +  ff.filter(x => x._1._1 == ee.srcId && x._1._2 == ee.dstId).map(x => x._2).reduce(_ + _) + ")\"" +
      " color=" + ( if( Math.abs(ee.attr -  ff.filter(x => x._1._1 == ee.srcId && x._1._2 == ee.dstId).map(x => x._2).reduce(_ + _) ) == 0 ) "red" else "black") + " ]"
  ).reduce(_ ++ " ;\n" ++ _) + "; \n"
  pw.write("digraph mon_graphe {\n")
  pw.write(str)
//  pw.write("{ rank=same " + gg.vertices.collect().map(x => x._1).filter( x => (x != sid && x != tid)).map(_.toString).reduce(_ ++ " " ++ _) + " }")
  pw.write("\n }")
  pw.close()   // "dot -Tpdf out.dot -o out.pdf".!!
}


// == Init  part ==
val r = scala.util.Random
var n: Int = 20 // Number of vertices
val verts_ex = sc.parallelize(Range(0,n).map(x => (x.toLong,0.toLong)))
val sourceId_ex: VertexId = 0L
val targetId_ex: VertexId = 1L

// == Cpp part ==
val stc = (("/home/laurent/code/spark-ddt/build/build-spark-Release-3/bin/test_graph-exe " + n).!!).split(";");
// val res_cpp =sc.parallelize(
//   stc(0).split(",").map( x=> x.split(" ")).filter(_.size > 1).map(x => x.map(_.toInt)).map(
//     x => ( if(x(0) >1) (x(0),x(3)) else (x(1),x(3))))
// ).reduceByKey((u,v) => v)

val res_cpp = sc.parallelize(stc(3).split(",").map(x => x.split(" ")).map(x => (x(0).toInt,x(1).toInt)))

val edges_ex_1 =sc.parallelize(
  stc(0).split(",").map( x=> x.split(" ")).filter(_.size > 1).map( x => org.apache.spark.graphx.Edge(x(0).toLong,x(1).toLong,x(2).toInt))
)
val edges_ex_2 =sc.parallelize(
  stc(1).split(",").map( x=> x.split(" ")).filter(_.size > 1).map( x => org.apache.spark.graphx.Edge(x(0).toLong,x(1).toLong,x(2).toInt))
)


// val edges_res_ex =sc.parallelize(
//   stc(0).split(",").map( x=> x.split(" ")).filter(_.size > 1).map( x => org.apache.spark.graphx.Edge(x(0).toLong,x(1).toLong,x(4).toInt))
// )

// // ===== Algo 1 =====
var vertexBuffer = scala.collection.mutable.ArrayBuffer[(Long, VertexData)]()
var edgeBuffer = scala.collection.mutable.ArrayBuffer[Edge[EdgeData]]()

var edgeBuffer = stc(0).split(",").map( x=> x.split(" ")).filter(_.size > 1).map( x=> Edge(x(0).toLong,x(1).toLong,(0,x(2).toInt)))
var vertexBuffer = Range(0,n+2).map(x=> (x.toLong,(0,0, Map[(VertexId, Boolean), Int]()))).toArray
var res_gg2 = maxflow2(sourceId_ex,targetId_ex,vertexBuffer,edgeBuffer,sc)
var flows_ex_2 = res_gg2.edges.map(x => ((x.srcId,x.dstId),x.attr._2))


// == Algo 2 part ==
val gg_ex_1 : org.apache.spark.graphx.Graph[Long,Int] = Graph(verts_ex,edges_ex_1)
val gg_ex_2 : org.apache.spark.graphx.Graph[Long,Int] = Graph(verts_ex,edges_ex_2)


val flows_ex_1 = maxFlow(sourceId_ex, targetId_ex, gg_ex_1,sc)
val flows_ex_2 = maxFlow(sourceId_ex, targetId_ex, gg_ex_2,sc)


val edg_flow_1 = gg_ex_1.edges.map(x => ((x.srcId,x.dstId),x.attr))
val emanating_1 = flows_ex_1.filter(e => e._1._1 == sourceId_ex).map(e => (e._1._1,e._2)).reduceByKey(_ + _).collect


val res_score_ex_1 = (gg_ex_1.edges.map(x => ((x.srcId,x.dstId),x.attr)) union flows_ex_1).reduceByKey((u,v) => Math.abs(u - v))
val res_score_ex_2 = (gg_ex_2.edges.map(x => ((x.srcId,x.dstId),x.attr)) union flows_ex_2).reduceByKey((u,v) => Math.abs(u - v))

val res_spark = res_score_ex_2.filter(
  {case ((u,v),w) => u <2 || v < 2 } // Keep only edges connected to source or target
).map(
  {case ((u,v),w) => if(u >1) (u,(v,w)) else (v,(u,w))} // Put the source target id 
).reduceByKey(
  (x1,x2) => if(x1._2 < x2._2) x2 else x1
).map( x=> (x._1.toInt,(x._2._1.toInt + 1) % 2))

val merge_res =  (res_cpp union res_spark).reduceByKey((u,v) => Math.abs(u-v)).collect
println("\n================================")
println("=========== Results ============")
println("Diff error:")
println(merge_res.map(_._2).foldLeft(0)((u,v) => u + v))
println("===== Max flow =====")
print("Max Flow scala : ")
println(emanating_1(0)._2)
print("Max Flow cpp : ")
println(stc(4))
// print("Max Flow cpp + scala : ")
// println(emanating_res(0)._2)
// print("Max Flow cpp + scala mix : ")
// println(emanating_mix(0)._2)

gg_ex_1.edges.collect
export_graph(gg_ex_1,flows_ex_1,sourceId_ex,targetId_ex)
//}






