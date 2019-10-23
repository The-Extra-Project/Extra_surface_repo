package iqlibflow2

import org.apache.spark._
import org.apache.spark.graphx._
import org.apache.spark.rdd.RDD
import scala.collection.mutable.HashMap
import scala.collection.mutable.Set
import sys.process._
import org.apache.spark.graphx.util.GraphGenerators
import java.io._
import scala.util.control.Breaks._


/* 
 - EdgeData - This type simply represents the flow on an edge as a tuple. The first Int
 is the active flow, and the second Int is the edge capacity. This data encapsulates all
 the information of the residual graph as the residual capacity in the opposite direction
 is simply equal to the current flow in the direction of the edge, while the capacity of
 the residual edge in the direction of the edge is the capacity minus the current flow.

 - VertexData - This type represents the data at each vertex as a tuple. The first Int
 is the excess flow and the second Int is the current height label. The third element of
 the tuple is the VertexPushMap type that was defined earlier, and is where the selected
 pushes are stored in between the Surveying and Execution steps.

 - SurveyMessage - This is the type used for the messaging in the Surveying step. The
 tuple simply includes the push information as a VertexPushMap, and the neighboring
  height label as an Int.


 vertexBuffer += ((sourceId, (0, values(1).toInt + 1, Map[(VertexId, Boolean), Int]())))
 vertexBuffer += ((sinkId, (0, 0, Map[(VertexId, Boolean), Int]())))

 edgeBuffer += Edge(sourceId, values(1).toLong, (values(2).toInt, values(2).toInt))
*/

object mflow2 {

  // Define types
  type VertexPushMap = Map[(VertexId, Boolean), Int]
  type EdgeData = (Int, Int)
  type VertexData = (Int, Int, VertexPushMap)
  type SurveyMessage = (VertexPushMap, Int)


  def maxflow2(
    sourceId: VertexId,
    sinkId: VertexId,
    vertexBuffer: Array[(Long, VertexData)],
    edgeBuffer : Array[Edge[EdgeData]],
    sc : SparkContext
  ) : Graph[iqlibflow2.mflow2.VertexData,iqlibflow2.mflow2.EdgeData] = {

    // Initialize the graph
    var activeMessages = 1
    var iteration = 1


    val vertexArray = vertexBuffer
    val edgeArray = edgeBuffer
    val vertexRDD: RDD[(VertexId, VertexData)] = sc.parallelize(vertexArray)
    val edgeRDD: RDD[Edge[EdgeData]] = sc.parallelize(edgeArray)
    val graph_init = Graph(vertexRDD, edgeRDD)


    // "Surveying" MapReduce step
    val eligiblePushesRDD = graph_init.aggregateMessages[SurveyMessage] (
      // Map: Send message if vertex has excess
      edgeContext => {
        // Make sure not to push from sink or source
        if (edgeContext.srcId != sinkId && edgeContext.srcId != sourceId) {
          // If a residual edge exists from source to destination
          if (edgeContext.attr._2 > edgeContext.attr._1) {
            // If source has an excess
            if (edgeContext.srcAttr._1 > 0) {
              // If source has height one greater than destination
              if (edgeContext.srcAttr._2 == (edgeContext.dstAttr._2 + 1)) {
                // Push is possible, send message to source containing push information
                val pushAmount = math.min(edgeContext.attr._2 - edgeContext.attr._1, edgeContext.srcAttr._1)
                edgeContext.sendToSrc((Map((edgeContext.dstId, true) -> pushAmount), edgeContext.dstAttr._2))
              } else {
                edgeContext.sendToSrc((Map(), edgeContext.dstAttr._2))
              }
            }
          }
        }
      },
      // Reduce: Concatenate into map of all possible pushes, keep track of relabel eligibility
      (a, b) => {
        (a._1 ++ b._1, math.min(a._2, b._2))
      }
    )

    // "Surveying" MapReduce step
    val eligiblePushesRDD2 = graph_init.aggregateMessages[SurveyMessage] (
      // Map: Send message if vertex has excess
      edgeContext => {
        // Make sure not to push from sink or source
        if (edgeContext.srcId != sinkId && edgeContext.srcId != sourceId) {
          // If a residual edge exists from source to destination
          if (edgeContext.attr._2 > edgeContext.attr._1) {
            // If source has an excess
            if (edgeContext.srcAttr._1 > 0) {
              // If source has height one greater than destination
              if (edgeContext.srcAttr._2 == (edgeContext.dstAttr._2 + 1)) {
                // Push is possible, send message to source containing push information
                val pushAmount = math.min(edgeContext.attr._2 - edgeContext.attr._1, edgeContext.srcAttr._1)
                edgeContext.sendToSrc((Map((edgeContext.dstId, true) -> pushAmount), edgeContext.dstAttr._2))
              } else {
                edgeContext.sendToSrc((Map(), edgeContext.dstAttr._2))
              }
            }
          }
        }
      },
      // Reduce: Concatenate into map of all possible pushes, keep track of relabel eligibility
      (a, b) => {
        (a._1 ++ b._1, math.min(a._2, b._2))
      }
    )

    val graph_2 = graph_init.outerJoinVertices(eligiblePushesRDD2) {
      (id: VertexId, data: VertexData, msg: Option[SurveyMessage]) => {
        // Store empty map if no messages
        if (msg.isEmpty) {
          (data._1, data._2, Map[(VertexId, Boolean), Int]())
        } else if (msg.get._2 >= data._2) {
          // Eligible for relabel
          (data._1, msg.get._2 + 1, Map[(VertexId, Boolean), Int]())
        } else {
          // Add pushes until no excess remains or pushes are exhausted
          var excess = data._1
          val selectedPushes = scala.collection.mutable.Map[(VertexId, Boolean), Int]()
          // Select pushes until flow is gone, break once no flow is remaining.
          breakable {
            msg.get._1.foreach(pushData => {
              val dstId = pushData._1._1
              val forwardPush = pushData._1._2
              val pushAmount = pushData._2
              if (excess > 0) {
                val selectedPushAmount = math.min(pushAmount, excess)
                excess -= selectedPushAmount
                selectedPushes((dstId, forwardPush)) = selectedPushAmount
              } else {
                break
              }
            })
          }
          (excess, data._2, selectedPushes.toMap)
        }
      }
    }



    val graph_3 = graph_2.outerJoinVertices(eligiblePushesRDD2) {
      (id: VertexId, data: VertexData, msg: Option[SurveyMessage]) => {
        // Store empty map if no messages
        if (msg.isEmpty) {
          (data._1, data._2, Map[(VertexId, Boolean), Int]())
        } else if (msg.get._2 >= data._2) {
          // Eligible for relabel
          (data._1, msg.get._2 + 1, Map[(VertexId, Boolean), Int]())
        } else {
          // Add pushes until no excess remains or pushes are exhausted
          var excess = data._1
          val selectedPushes = scala.collection.mutable.Map[(VertexId, Boolean), Int]()
          // Select pushes until flow is gone, break once no flow is remaining.
          breakable {
            msg.get._1.foreach(pushData => {
              val dstId = pushData._1._1
              val forwardPush = pushData._1._2
              val pushAmount = pushData._2
              if (excess > 0) {
                val selectedPushAmount = math.min(pushAmount, excess)
                excess -= selectedPushAmount
                selectedPushes((dstId, forwardPush)) = selectedPushAmount
              } else {
                break
              }
            })
          }
          (excess, data._2, selectedPushes.toMap)
        }
      }
    }




    val executedPushesRDD = graph_3.aggregateMessages[Int] (
      // Map: Send push information to vertices that received flow
      edgeContext => {

        // Check if destination vertex id is in the source's push map
        if (edgeContext.srcAttr._3.contains((edgeContext.dstId, true))) {
          val pushAmount: Int = edgeContext.srcAttr._3((edgeContext.dstId, true))
          edgeContext.sendToDst(pushAmount)
        }
        // Check if source vertex id is in the destinations's push map
        if (edgeContext.dstAttr._3.contains((edgeContext.srcId, false))) {
          val pushAmount: Int = edgeContext.dstAttr._3((edgeContext.srcId, false))
          edgeContext.sendToSrc(pushAmount)
        }
      },
      // Reduce: Combine all incoming flow into a single total
      (a, b) => {
        a + b
      }
    )


    val executedPushesRDD2 = graph_3.aggregateMessages[Int] (
      // Map: Send push information to vertices that received flow
      edgeContext => {

        // Check if destination vertex id is in the source's push map
        if (edgeContext.srcAttr._3.contains((edgeContext.dstId, true))) {
          val pushAmount: Int = edgeContext.srcAttr._3((edgeContext.dstId, true))
          edgeContext.sendToDst(pushAmount)
        }
        // Check if source vertex id is in the destinations's push map
        if (edgeContext.dstAttr._3.contains((edgeContext.srcId, false))) {
          val pushAmount: Int = edgeContext.dstAttr._3((edgeContext.srcId, false))
          edgeContext.sendToSrc(pushAmount)
        }
      },
      // Reduce: Combine all incoming flow into a single total
      (a, b) => {
        a + b
      }
    )




    // Update excess values
    val graph_res = graph_3.outerJoinVertices(executedPushesRDD2) {
      (id: VertexId, data: VertexData, msg: Option[Int]) => {
        // Add pushed flow to vertex
        (data._1 + msg.getOrElse(0), data._2, data._3)
      }
    }

    return graph_res
  }

}



