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
import org.apache.hadoop.fs.permission.FsPermission

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


val fs = FileSystem.get(sc.hadoopConfiguration);
// =========  Metadata extraction  ==========
// Bash varaibles
val output_dir = get_bash_variable("OUTPUT_DATA_DIR").replaceAll("//", "/");
var input_dir = get_bash_variable("INPUT_DATA_DIR").replaceAll("//", "/");
val env_xml = get_bash_variable("PARAM_PATH");
val ddt_main_dir = get_bash_variable("DDT_MAIN_DIR");
val algo_seed_scala = get_bash_variable("ALGO_SEED",scala.util.Random.nextInt(100000).toString).toInt;
val global_build_dir = get_bash_variable("GLOBAL_BUILD_DIR");
if (env_xml.isEmpty || ddt_main_dir.isEmpty || input_dir.isEmpty) {
  println("Bash variable are empy, please set it")
  System.exit(1)
}

// ============= Spark context ================
// Init spark spark conext
val df_par = sc.defaultParallelism;
var conf = sc.getConf
conf.set("spark.default.parallelism",sc.defaultParallelism.toString)
sc.stop()
var iqsc = new SparkContext(conf)
println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)


// ============= Parse Algo params ================
var param_list = parse_xml_datasets_2(env_xml)
if (param_list.size == 0){
  println("param_list empty")
  System.exit(1)
}

val params_scala = param_list.head

for(params_scala <- param_list){
  if( params_scala.exists(List("dim","bbox")) &&
    (params_scala.get_param("do_process", "false").toBoolean)){

    val rr = new scala.util.Random(algo_seed_scala)
    val input_label = "pts_generated"
    val dim = params_scala.get_param("dim", "2").toInt
    val bbox = params_scala.get_param("bbox", "")
    val do_tests = params_scala.get_param("do_tests", "true").toBoolean;
    val plot_lvl = params_scala.get_param("plot_lvl", "1").toInt;
    val do_generate = params_scala.get_param("do_generate", "false").toBoolean;
    val do_stream = params_scala.get_param("do_stream", "true").toBoolean;
    val do_checkpoint = params_scala.get_param("do_checkpoint", "false").toBoolean;
    val regexp_filter = params_scala.get_param("regexp_filter", "");
    val max_ppt = params_scala.get_param("max_ppt", "10000").toInt
    val min_ppt = params_scala.get_param("min_ppt", "20").toInt
    val algo_seed_cpp = params_scala.get_param("algo_seed_cpp", rr.nextInt(rr.nextInt(10000)).toString).toInt 
    val ddt_kernel_dir = params_scala.get_param("ddt_kernel", "build-spark-Release-D" + dim.toString)
    val slvl_glob = StorageLevel.fromString(params_scala.get_param("StorageLevel", "DISK_ONLY"))
    val slvl_loop = StorageLevel.fromString(params_scala.get_param("StorageLevelLoop", "DISK_ONLY"))
    val build_dir = global_build_dir + "/" + ddt_kernel_dir
    params_scala("ddt_main_dir") = collection.mutable.Set(ddt_main_dir)
    params_scala("build_dir") = collection.mutable.Set(build_dir)

    val nbp_and_depth = params_scala.get_param("nbp_and_depth", "")
    if(nbp_and_depth contains ":"){
      params_scala("nbp") =  collection.mutable.Set(nbp_and_depth.split(":")(0))
      params_scala("ndtree_depth") = collection.mutable.Set(nbp_and_depth.split(":")(1))
    }

    val nbp =  params_scala.get_param("nbp", "10000").toInt
    val ndtree_depth = params_scala.get_param("ndtree_depth", "4").toInt
    var cur_input_dir = input_dir;
    val datatype =  params_scala.get_param("datatype", "")
    val spark_core_max = params_scala.get_param("spark_core_max", df_par.toString).toInt

    println("")
    println("=======================================================")
    println("=======================================================")
    params_scala.map(x => println((x._1 + " ").padTo(15, '-') + "->  " + x._2.head))

    // if(datatype == "files"){
    //   cur_input_dir = params_scala.get_param("filepath", "")
    // }

    if(spark_core_max >0){
      iqsc.stop()
      conf.set("spark.cores.max",spark_core_max.toString)
      iqsc = new SparkContext(conf)
    }
    println("===> IQSC DEFAUT PAR :" + iqsc.defaultParallelism)


    val dateFormatter = new SimpleDateFormat("dd_MM_yyyy_hh_mm_ss")
    val datestring = dateFormatter.format(Calendar.getInstance().getTime());
    val cur_output_dir ={output_dir  + iqsc.applicationId + "_" + datestring + "_"+ params_scala("name").head }
    params_scala("output_dir") = collection.mutable.Set(cur_output_dir)
    fs.mkdirs(new Path(cur_output_dir),new FsPermission("777"))

    //val slvl_glob: StorageLevel = StorageLevel.MEMORY_AND_DISK
    //val slvl_glob: StorageLevel = StorageLevel.OFF_HEAP
    //val slvl_glob : StorageLevel = StorageLevel.MEMORY_AND_DISK_SER
    //val slvl_glob: StorageLevel = StorageLevel.MEMORY_ONLY
    //val slvl_glob: StorageLevel = StorageLevel.DISK_ONLY


    val iq = new IQlibSched(slvl_glob,slvl_loop)

    //iqsc.setCheckpointDir("/home/laurent/shared_spark/checkpoint/")
    
    val params_new = new Hash_StringSeq with mutable.MultiMap[String, String]
    val params_cpp =  set_params(params_new,List(
      ("exec_path", build_dir + "/bin/ddt-stream-exe"),
      ("dim",params_scala("dim").head),
      ("bbox",params_scala("bbox").head),
      ("ech_input","1"),
      ("input_dir",cur_input_dir),
      ("output_dir",cur_output_dir),
      ("min_ppt",params_scala("min_ppt").head),
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
    val dump_ply_binary_cmd =  set_params(params_cpp, List(("step","dump_ply_binary"))).to_command_line
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
        var nb_kernel = params_scala.get_param("nb_kernel", (nbp/max_ppt).toString).toInt

        println("nb kernels : " +nb_kernel)
        if(nb_kernel == 0){
          println(" ====== WARNING =====")
          println(" NB_kernel == 0, set to 1")
          nb_kernel = 1
        }
        rep_value = ((if((nb_kernel) < spark_core_max) spark_core_max else  (nb_kernel).toInt))
        val nbp_per_kernel = nbp/nb_kernel;
        val input_rdd_raw: RDD[KValue] = iqsc.parallelize(List.range(0,nb_kernel)).map(
          x => (x.toLong, List(""))).repartition(rep_value)
        val raw_inputs = iq.run_pipe_fun_KValue(
          generate_points_cmd ++ List("--label","lab2","--nbp",nbp_per_kernel.toString),
          input_rdd_raw, "generate_points", do_dump = false)
        kvrdd_inputs = iq.get_kvrdd(raw_inputs, "g",txt="pts").persist(slvl_glob).setName("KVRDD_INPUT");
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
        kvrdd_points = iq.get_kvrdd(kvrdd_inputs, "p",txt="pts").reduceByKey(_ ++ _).persist(slvl_glob).setName("KVRDD_INPUT");
      }
      case "files" => {
        println("")
        println("======== LOAD DATA  file =============")
        val ss_reg = regexp_filter.r
        val ply_input = getListOfFiles(cur_input_dir).filter(
          x => ((x.toString endsWith ".ply") && ((ss_reg).findFirstIn(x.toString).isDefined)))
        kvrdd_inputs = iq.get_kvrdd(iqsc.parallelize(ply_input.map(
          fname => "p 1 " + ("([0-9]+)".r).findAllIn(fname.toString).toArray.last + " f " + fname.toString)),"p")
      }
      case "filestream" => {
        println("")
        println("======== LOAD DATA filestream =============")
        val nb_ply = fs.listStatus(new Path(cur_input_dir)).map(_.getPath).filter(
          x => ((x.toString endsWith ".ply"))
        ).size
        kvrdd_inputs = iqsc.textFile(cur_input_dir + "*.ply").zipWithIndex.map(
          e => (e._2.toLong,List("g 1 " + e._2.toString + " s " +  e._1.toString))
        ).repartition(nb_ply).persist(slvl_glob).setName("KVRDD_INPUT")
      }
    }

    if(plot_lvl >= 3){
      iq.run_pipe_fun_KValue(
        dump_ply_binary_cmd ++ List("--label", "raw_pts"),
        kvrdd_inputs, "raw_pts", do_dump = false).collect()
    }

    println("======== Tiling =============")
    kvrdd_points = ddt_algo.compute_tiling(kvrdd_inputs,iq,params_cpp,params_scala);

    if(plot_lvl >= 3){
      iq.run_pipe_fun_KValue(
        dump_ply_binary_cmd ++ List("--label", "tiled_pts"),
        kvrdd_points, "octree_pts", do_dump = false).collect()
    }

    nb_leaf = params_scala("nb_leaf").head.toInt;

    // ====== Algo param according to tiling ==========
    val rep_merge = ((if((nb_leaf) < spark_core_max) spark_core_max else  nb_leaf));
    var rep_loop = nb_leaf;
    if(ndtree_depth == 8)
      rep_loop = spark_core_max*10;
    params_scala("rep_loop") = collection.mutable.Set(rep_loop.toString)
    params_scala("rep_merge") = collection.mutable.Set(rep_merge.toString)

    if(plot_lvl >= 3 && dim == 2){
      val rdd_json_input_ply = iq.run_pipe_fun_KValue(
        ply2geojson_cmd ++ List("--label", "kvrdd_points"),
        kvrdd_points, "extract_tri_vrt_final", do_dump = false)
      rdd_json_input_ply.collect()
    }


    //    kvrdd_points.persist(slvl_glob)
    println("======== START DDT =============")
    val t0 = System.nanoTime()
    val (graph_tri,log_tri,stats_tri)  = ddt_algo.compute_ddt(
      kvrdd_points = kvrdd_points,
      iq = iq,
      params_cpp = params_cpp,
      params_scala = params_scala
    );

    graph_tri.vertices.count
    val t1 = System.nanoTime()
    println("======== algo done =============")
    val scala_time = ((t1 - t0)/1000000000.0);
    println("scala_time_full:" + scala_time.toString)

    // + iqsc.applicationId > spark_core_max
    params_scala("algo_seed_scala") = collection.mutable.Set(algo_seed_scala.toString)
    params_scala("availableProcessors") = collection.mutable.Set(java.lang.Runtime.getRuntime.availableProcessors.toString)
    params_scala("scala_time_full") = collection.mutable.Set(scala_time.toString)
    params_scala("Parallelism_lvl") = collection.mutable.Set((iqsc.getExecutorMemoryStatus.size -1).toString)
    val kvrdd_stats = iqsc.parallelize(List(
      (0L,List("l 1 0 s tot_scala4:" + scala_time))
    )) //union iq.get_kvrdd(raw_inputs, "l",txt="pts")
      dump_stats(kvrdd_stats,cur_output_dir + ".stats.csv" ,sc);
    dump_json(params_scala,cur_output_dir + ".params.json",sc);
  }
}


