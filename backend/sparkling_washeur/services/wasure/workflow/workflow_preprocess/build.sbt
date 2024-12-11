name := "workflow_preprocess"
version := "0.1"
scalaVersion := "2.13.0"


scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

resolvers +=  "commons-io" at "https://mvnrepository.com/artifact"




libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",  // Ensure Hadoop version matches
  "org.scala-lang.modules" %% "scala-parallel-collections" % "1.0.4",
  "org.scala-lang.modules" %% "scala-xml" % "2.1.0",
  "commons-io" % "commons-io" % "2.11.0",
  
)

//import sbtassembly.AssemblyPlugin.autoImport._

enablePlugins(AssemblyPlugin)

lazy val preprocessJar = taskKey[File]("preprocess-jar")


// preprocessJar := {
//   val jarFile = (Compile / packageBin).value / "src" / "main" / "scala"
//   val targetFile = target.value / "preprocess.jar"
//   IO.copyFile(jarFile, targetFile)
//   targetFile
// }



// the following specs are for the combination of the various assembly plugins in uber jar format (by checking and removing those jars that have path conflicts)

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

