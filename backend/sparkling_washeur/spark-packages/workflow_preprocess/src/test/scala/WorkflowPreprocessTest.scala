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
    val args = Array(
      "INPUT_DATA_DIR=/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/",
      "OUTPUT_DATA_DIR=/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/",
      "PARAM_PATH=/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/wasure_metadata.xml",
      "DDT_MAIN_DIR=/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/",
      "GLOBAL_BUILD_DIR=/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/build/"
    )
    val params = args.map(_.split("=")).collect {
      case Array(k, v) => k -> v
    }.toMap

    assert(params("INPUT_DATA_DIR") == "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/")
    assert(params("OUTPUT_DATA_DIR") == "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/")
    assert(params("PARAM_PATH") == "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/wasure_metadata.xml")
    assert(params("DDT_MAIN_DIR") == "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/")
    assert(params("GLOBAL_BUILD_DIR") == "/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/build/")
    }

  test("Test file system operations") {
    val outputDir = new Path("/home/ubuntu/app/frontiertech/extralabs_finam/Extra_surface_repo/backend/sparkling_washeur/outputs_examples/lidar_hd_crop_2/output/")
    fs.mkdirs(outputDir, new FsPermission("777"))
    assert(fs.exists(outputDir))
    fs.delete(outputDir, true)
    assert(!fs.exists(outputDir))
  }

  test("Test XML parsing") {
    val envXml = "/path/to/env.xml"
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
    val paramsNew = new Hash_StringSeq with mutable.MultiMap[String, String]
    val paramsWasure = set_params(paramsNew, List(
      ("exec_path", "/path/to/wasure-stream-exe"),
      ("dim", "3"),
      ("input_dir", "/path/to/input"),
      ("output_dir", "/path/to/output")
    ))
    val commandLine = paramsWasure.to_command_line
    assert(commandLine.contains("--exec_path /path/to/wasure-stream-exe"))
    assert(commandLine.contains("--dim 3"))
    assert(commandLine.contains("--input_dir /path/to/input"))
    assert(commandLine.contains("--output_dir /path/to/output"))
  }

  // Add more tests for other functions as needed
}