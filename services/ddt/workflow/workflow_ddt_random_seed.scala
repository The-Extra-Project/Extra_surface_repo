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
import collection.mutable
import org.apache.hadoop.fs.FileSystem
import org.apache.spark.SparkContext
import scala.collection.mutable.ListBuffer
import java.nio.file.{ Paths, Files }
import org.apache.hadoop.fs.Path

// Iqlib Import
import iqlibc._;
import iqlibu._;
import iqlibc.IQlibCore._;
import algo_stats._;
import iqlib_algo._
import tiling._

import xml_parsing._;
import bash_funcs._
import strings_opt._;
import params_parser._;
import files_opt._;


val ss ="""        
  ERROR  ERROR  ERROR  ERROR  ERROR  ERROR  ERROR  ERROR
  ERROR  ERROR  ERROR         ERROR  ERROR  ERROR  ERROR
  ERROR  ERROR        `/oso.  ERROR         ERROR  ERROR
  ERROR  ERROR      ./yyo::`                       ERROR 
  ERROR  ERROR     -+syy/ -- `.`    .``````.       ERROR      
  ERROR  ERROR    `+syyyoos:`oss/..-......-...`    ERROR   
  ERROR  ERROR    :oyyhhyo` .sssys:..........`.:   ERROR  
  ERROR  ERROR`://osyys+-  `+osyyhy/....```````.`  ERROR 
  ERROR    `/ymmsyso+:`      .oyyhhysoooo/-..``.   ERROR  
  ERROR   .+yhhhhyoo.         -yyhhsossyhyo:...:   ERROR  
  ERROR  -oosyhhhys.   +ssso+-/yhhhsoo//oys:..-.   ERROR  
      `-+o++ossss/`  :o+:+hdds/+yddhhyoooss/--. R  ERROR   
     .o+so++ossyhhhsoso:--+dh++odmdmdh++sso++/- R  ERROR   
   `:o++/+ooosshdddh+s/---:yhyhhdddddds++oo++. OR  ERROR    
   `:/://+osyyhhyhdyody/---/yhhhhhhddddsosyhyso:.` ERROR 
     `-:+osyyyyysso/yddh/...:/ssyyhhddddddhhyyhhys+:`ROR    
  ERROR.-::++ooo++-/dhs/-..--:ysyyyhhdddhhyyhhhhdddh+ OR    
  ERROR      `.-   +hho:..:--:+ossyyyhdddhhhhhhhhhyys`OR   
  ERROR  ERROR    `shddy:-:--:-:/+syshdddddhdddddddh+`OR   
  ERROR  ERROR    :shdddy++::----:ooshhddddhddddddh+.ROR    
  ERROR  ERROR    /yddhhhy/:-----+++/oyhdddhhdddhy/`RROR     
  ERROR  ERROR   `shhhyhmNo:----/+-:///ydddhdddhs: ERROR 
  ERROR  ERROR   :sys+/sNMmo/::-/-.--.``ohhhhhyo.  ERROR 
  ERROR  ERROR   :so+:/oNMmdhy/:+---::-``:oooo:`R  ERROR  
  ERROR  ERROR   ohhosoomNdddh+:s/:+oso-``.+y: OR  ERROR    
  ERROR  ERROR   oshhssohddddh+-yhshdhhs-``.o.     ERROR """

val conf = new SparkConf().setAppName("DDT")
val fs = FileSystem.get(sc.hadoopConfiguration);
// ===== Metadata extraction  ==========
// Bash varaibles
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
var input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val algo_seed_scala = get_bash_variable("ALGO_SEED",scala.util.Random.nextInt(100000).toString).toInt;
val build_dir = get_bash_variable("GLOBAL_BUILD_DIR");


if (output_dir.isEmpty ||
  input_dir.isEmpty ) {
  println("Bash variable are empy, please set it")
  System.exit(1)
}


println("===> SC DEFAUT PAR :" + sc.defaultParallelism)
val param_list = parse_xml_datasets(env_xml);//.sortWith((x,y) => x("spark_core_max").head.toInt > y("spark_core_max").head.toInt)

var conf = sc.getConf
conf.set("spark.default.parallelism",sc.defaultParallelism.toString)
sc.stop()


var iqsc = new SparkContext(conf)
println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)

if(false){ 
  val params_scala = param_list(0)
}



for(params_scala <- param_list){
  if( params_scala.exists(List("dim","bbox")) &&
     (params_scala.get_param("do_process", "false").toBoolean)){


    val rr = new scala.util.Random(algo_seed_scala)
    val algo_seed_cpp = rr.nextInt(rr.nextInt(10000))
    val input_label = "pts_generated"
    val dim = params_scala.get_param("dim", "2").toInt
    val bbox = params_scala.get_param("bbox", "")
    val do_tests = params_scala.get_param("do_tests", "true").toBoolean;
    val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
    val do_generate = params_scala.get_param("do_generate", "false").toBoolean;
    val do_stream = params_scala.get_param("do_stream", "true").toBoolean;
    val regexp_filter = params_scala.get_param("regexp_filter", "");
    val max_ppp = params_scala.get_param("max_ppp", "10000").toInt
    val algo_version = params_scala.get_param("algo_version", "1").toInt
    val slvl = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))


    val nbp_and_depth = params_scala.get_param("nbp_and_depth", "")
    if(nbp_and_depth contains ":"){
      params_scala("nbp") =  collection.mutable.Set(nbp_and_depth.split(":")(0))
      params_scala("ndtree_depth") = collection.mutable.Set(nbp_and_depth.split(":")(1))
    }

    val nbp =  params_scala.get_param("nbp", "10000").toInt
    val ndtree_depth = params_scala.get_param("ndtree_depth", "4").toInt
    var cur_input_dir = input_dir;
    val datatype =  params_scala.get_param("datatype", "")
    var spark_core_max = params_scala.get_param("spark_core_max", "-1").toInt

    println("")
    println("=======================================================")
    println("=======================================================")
    params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))

    if(datatype == "files"){
      cur_input_dir = params_scala.get_param("filepath", "")
    }

    if(spark_core_max >0){
      iqsc.stop()
      conf.set("spark.cores.max",spark_core_max.toString)
      iqsc = new SparkContext(conf)
    }else{
      spark_core_max = iqsc.defaultParallelism;
    }
   println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)


    val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
    val datestring = dateFormatter.format(Calendar.getInstance().getTime());
    val cur_output_dir ={output_dir  + iqsc.applicationId + "_" + datestring + "_"+ params_scala("name").head }    
    fs.mkdirs(new Path(cur_output_dir))


    params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)
    params_scala("output_dir") = collection.mutable.Set(cur_output_dir)

    //val slvl: StorageLevel = StorageLevel.MEMORY_AND_DISK
    //val slvl: StorageLevel = StorageLevel.OFF_HEAP
    //val slvl : StorageLevel = StorageLevel.MEMORY_AND_DISK_SER
    //val slvl: StorageLevel = StorageLevel.MEMORY_ONLY
    //val slvl: StorageLevel = StorageLevel.DISK_ONLY


    val iq = new IQlibSched(slvl)


    //iqsc.setCheckpointDir("/home/laurent/shared_spark/checkpoint/")
    
    val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
    val params_cpp =  set_params(params_new,List(
      ("exec_path", build_dir + "/bin/ddt-stream-exe"),
      ("dim",params_scala("dim").head),
      ("bbox",params_scala("bbox").head),
      ("ech_input","1"),
      ("input_dir",cur_input_dir),
      ("output_dir",cur_output_dir),
      ("seed",algo_seed_cpp.toString)
    ))

    val nbt_side = math.pow(2,ndtree_depth)
    val tot_nbt = scala.math.pow(nbt_side,dim).toInt;
    val nbp_per_tile = nbp/tot_nbt;
    var rep_value = spark_core_max 
    var nb_leaf = tot_nbt;


    params_cpp("output_dir") = collection.mutable.Set(cur_output_dir)
    params_cpp("nbt_side") =  collection.mutable.Set(nbt_side.toString)

    if(do_stream)
      params_cpp("do_stream") =  collection.mutable.Set("")

    val generate_points_cmd =  set_params(params_cpp,  List(("step","generate_points_" + datatype))).to_command_line
    val extract_tri_vrt_cmd =  set_params(params_cpp, List(("step","extract_tri_vrt"))).to_command_line
    val ply2geojson_cmd =  set_params(params_cpp, List(("step","ply2geojson"))).to_command_line
    val extract_tile_vrt_cmd =  set_params(params_cpp, List(("step","extract_tile_vrt"))).to_command_line
    val extract_tile_json_cmd =  set_params(params_cpp, List(("step","extract_tile_json"))).to_command_line
    val tile_cmd =  set_params(params_cpp, List(("step","tile_ply"),("output_dir", cur_output_dir))).to_command_line
    val id_cmd = List(build_dir + "/bin/identity-exe");

    val tt000 = System.nanoTime()

    // ============  Init data ===========

    var kvrdd_points: RDD[KValue] = iqsc.parallelize(List((0L,List(""))));
    var kvrdd_inputs: RDD[KValue] = iqsc.parallelize(List((0L,List(""))));

    val kvrdd_log_list : ListBuffer[RDD[KValue]] = ListBuffer()


    println("")
    println("======== GENERATE DATA =============")
    println("datatype : " + datatype)
    datatype match {
      case "random_normal" => {
        val nb_kernel = tot_nbt/10;
        rep_value = ((if((nb_kernel) < spark_core_max) spark_core_max else  (nb_kernel).toInt))
        val nbp_per_kernel = nbp/nb_kernel;
        val input_rdd_raw: RDD[KValue] = iqsc.parallelize(List.range(0,nb_kernel)).map(
          x => (x.toLong, List(""))).repartition(rep_value)
        val raw_inputs = iq.run_pipe_fun_KValue(
          generate_points_cmd ++ List("--label","lab2","--nbp",nbp_per_kernel.toString),
          input_rdd_raw, "generate_points", do_dump = false)
        kvrdd_inputs = iq.get_kvrdd(raw_inputs, "g",txt="pts").persist(slvl);
        // kvrdd_points = iq.get_kvrdd(res_tiles,"p");
        // kvrdd_points = kvrdd_points.reduceByKey( (u,v) => u ::: v);

      }
      case "random_uniform" => {
        rep_value = ((if((tot_nbt/10) < spark_core_max) spark_core_max else  (tot_nbt/10).toInt))
        val input_rdd_raw: RDD[KValue] = iqsc.parallelize(List.range(0,tot_nbt)).map(
          x => (x.toLong, List(""))).repartition(rep_value)
        val kvrdd_inputs = iq.run_pipe_fun_KValue(
          generate_points_cmd ++ List("--label","lab2","--nbp",nbp.toString),
          input_rdd_raw, "generate_points", do_dump = false)
        kvrdd_points = iq.get_kvrdd(kvrdd_inputs, "p",txt="pts").reduceByKey(_ ++ _).persist(slvl);
      }
      case "files" => {
        println("")
        println("======== LOAD DATA =============")
        val ss_reg = regexp_filter.r
        val ply_input = getListOfFiles(cur_input_dir).filter(
          x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
        kvrdd_inputs = iq.get_kvrdd(iqsc.parallelize(ply_input.map(
          fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p")
      }
    }

    // if(false){
    //   val nbt = new nd_tree(2,10)
    //   val ndtree_depth = 8
    //   val id_pad = nbt.nb_nodes(ndtree_depth-1);
    //   val input_rdd_raw: RDD[(Int,Int)] = sc.parallelize(List.range(0,math.pow(math.pow(2,ndtree_depth),2).toInt)).map(x => (x.toInt, rr.nextInt(10000))).repartition(1000)
    //   val rddc = input_rdd_raw.map(x => (x._1.toInt + id_pad,x._2))
    //   val id_map = nbt.compute_id_map_fast(rddc,100000);
    //   val nb_leaf = id_map.values.toArray.distinct.size
    //   kvrdd_points = iq.get_kvrdd(res_tiles,"p").map(x => (id_map(x._1.toInt + id_pad).toLong,x._2)).reduceByKey( (u,v) => u ::: v).repartition(nb_leaf)
    // }

    println("======== Tiling =============")
    if(datatype != "random_uniform"){
      val nbt = new nd_tree(2,10)
      val id_pad = nbt.nb_nodes(ndtree_depth-1);
      val res_tiles = iq.run_pipe_fun_KValue(
        tile_cmd ++ List("--label", "tile_pp"),
        kvrdd_inputs, "tile", do_dump = false)
      res_tiles.persist(slvl);

      if(true){
        val defaultParallelism_val = spark_core_max;
        val kvrdd_count: RDD[KValue] = iq.get_kvrdd(res_tiles, "c",txt="count").repartition(defaultParallelism_val)
        val rddc = kvrdd_count.map(x => (x._1.toInt + id_pad,x.value(0).split(" ").last.toInt))
        rddc.cache()
        rddc.collect()

        val id_map = nbt.compute_id_map_fast(rddc,max_ppp);


        println("== new map ==")
        println(id_map)

        //        iq.get_kvrdd(res_tiles,"p").map(x => (id_map(x._1.toInt + id_pad),x._2)).collect
        println("Aggregate points set per leaf (can take a while ...)")
        nb_leaf = id_map.values.toArray.distinct.size
        params_scala("nb_leaf") =  collection.mutable.Set(nb_leaf.toString)
        println("Number of leaf :" + nb_leaf)
        rep_value = ((if((nb_leaf) < spark_core_max) spark_core_max else  nb_leaf))
        kvrdd_points = iq.get_kvrdd(res_tiles,"p").map(x => (id_map(x._1.toInt + id_pad).toLong,x._2)).reduceByKey( (u,v) => u ::: v).repartition(rep_value)
        println("Aggregate done")
      }else{
        nb_leaf = tot_nbt;
        kvrdd_points = iq.get_kvrdd(res_tiles,"p");
        kvrdd_points = kvrdd_points.reduceByKey( (u,v) => u ::: v);
      }
    }

    // ====== Algo param according to tiling ==========
    val rep_loop = ((if((nb_leaf/10) < spark_core_max) spark_core_max else  nb_leaf/10));
    params_scala("repartition_loop") = collection.mutable.Set(rep_loop.toString)

    if(plot_lvl >= 3){
      val rdd_json_input_ply = iq.run_pipe_fun_KValue(
        ply2geojson_cmd ++ List("--label", "kvrdd_points"),
        kvrdd_points, "extract_tri_vrt_final", do_dump = false)
      rdd_json_input_ply.collect()
    }


    println("======== START DDT =============")
    val t0 = System.nanoTime()
    val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(
      kvrdd_points = kvrdd_points,
      iq = iq,
      params_cpp = params_cpp,
      params_scala = params_scala,
      plot_lvl = plot_lvl,
      algo_version = algo_version
    );

    graph_tri.vertices.count
    val t1 = System.nanoTime()
    println("======== algo done =============")
    val scala_time = ((t1 - t0)/1000000000.0);
    println("scala time:" + scala_time.toString)

// + iqsc.applicationId > spark_core_max
    params_scala("algo_seed_scala") = collection.mutable.Set(algo_seed_scala.toString)
    params_scala("nb_leaf") = collection.mutable.Set(nb_leaf.toString)
    params_scala("availableProcessors") = collection.mutable.Set(java.lang.Runtime.getRuntime.availableProcessors.toString)
    params_scala("scala_time") = collection.mutable.Set(scala_time.toString)
    params_scala("Parallelism_lvl") = collection.mutable.Set((iqsc.getExecutorMemoryStatus.size -1).toString)
    val kvrdd_stats = iqsc.parallelize(List(
      (0L,List("l 1 0 s tot_scala4:" + scala_time))
    )) //union iq.get_kvrdd(raw_inputs, "l",txt="pts")
    dump_stats(kvrdd_stats,cur_output_dir + ".stats.csv" ,sc);
    dump_json(params_scala,cur_output_dir + ".params.json",sc);
  }
}
//dump_stats(log_tri,nbp,nbt_side,"full_"+t0,scala_time.toLong,cur_output_dir ,sc);
// if(nbp <= 20000){
//   val rdd_json_res = iq.run_pipe_fun_KValue(
//     ply2geojson_cmd ++ List("--label", "res_merged","--style","tri_main.qml"),
//     graph_tri.vertices, "extract_tri_vrt_final", do_dump = false)
//   rdd_json_res.collect()
// }



// graph_tri.edges.collect().map(e => (e.srcId,e.dstId)).filter(x => (x._1 == 0 || x._2 ==0)).size
