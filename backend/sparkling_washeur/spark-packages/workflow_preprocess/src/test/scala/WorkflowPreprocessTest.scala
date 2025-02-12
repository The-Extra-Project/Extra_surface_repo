import org.apache.spark.{SparkConf, SparkContext}
import org.apache.hadoop.fs.{FileSystem, Path}
import org.scalatest.funsuite.AnyFunSuite
import org.scalatest.BeforeAndAfterAll
import org.apache.hadoop.fs.permission.FsPermission
import scala.collection.mutable

import iqlib.util.params_parser._
class WorkflowPreprocessUnitTest extends AnyFunSuite with BeforeAndAfterAll {

  var sc: SparkContext = _
  var fs: FileSystem = _

  override def beforeAll(): Unit = {
    val conf = new SparkConf().setAppName("WorkflowPreprocessUnitTest").setMaster("local[*]")
    sc = new SparkContext(conf)
    fs = FileSystem.get(sc.hadoopConfiguration)
  }

  override def afterAll(): Unit = {
    sc.stop()
  }

  test("Test get input and output directories from parameters") {
    val basePath = "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/"
    val args = Array(
      s"INPUT_DATA_DIR=${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/",
      s"OUTPUT_DATA_DIR=${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/",
      s"PARAM_PATH=${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/wasure_metadata.xml",
      s"DDT_MAIN_DIR=${basePath}backend/",
      s"GLOBAL_BUILD_DIR=${basePath}backend/build/"
    )
    val params = args.map(_.split("=")).collect {
      case Array(k, v) => k -> v
    }.toMap

    assert(params("INPUT_DATA_DIR") == s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/")
    assert(params("OUTPUT_DATA_DIR") == s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/")
    assert(params("PARAM_PATH") == s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/wasure_metadata.xml")
    assert(params("DDT_MAIN_DIR") == s"${basePath}backend/")
    assert(params("GLOBAL_BUILD_DIR") == s"${basePath}backend/build/")
  }

  test("Test file system operations") {
    val basePath = "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/"
    val outputDir = new Path(s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/")
    fs.mkdirs(outputDir, new FsPermission("777"))
    assert(fs.exists(outputDir))
    fs.delete(outputDir, true)
    assert(!fs.exists(outputDir))
  }

  test("Test XML parsing") {
    val basePath = "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/"
    val envXml = s"${basePath}/path/to/env.xml"
    if (fs.exists(new Path(envXml))) {
      val paramList = parse_xml_datasets(envXml)
      val paramsScala = paramList(0)
      assert(paramsScala.nonEmpty)
    }
  }

  test("Test Spark operations") {
    val dfPar = sc.defaultParallelism
    assert(dfPar > 0)
  }

  test("Test command line generation") {
    val basePath = "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/"
    val inputPathDir = s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/input/"
    val outputPathDir = s"${basePath}backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/"
    val execPath = s"${basePath}backend/sparkling_washeur/spark-packages/workflow_preprocess/wasure-stream-exe"

    val paramsNew = new Hash_StringSeq with mutable.MultiMap[String, String]
    val paramsWasure = set_params(paramsNew, List(
      ("exec_path", execPath),
      ("dim", "3"),
      ("input_dir", inputPathDir),
      ("output_dir", outputPathDir)
    ))
    val commandLine = paramsWasure.to_command_line
    assert(commandLine.contains(s"--exec_path $execPath"))
    assert(commandLine.contains("--dim 3"))
    assert(commandLine.contains(s"--input_dir $inputPathDir"))
    assert(commandLine.contains(s"--output_dir $outputPathDir"))
  }

  // Add more tests for other functions as needed
}