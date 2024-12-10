name := "workflow_preprocess"
version := "0.1"
scalaVersion := "2.13.0"


scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")



libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",  // Ensure Hadoop version matches
  "org.scala-lang.modules" %% "scala-parallel-collections" % "0.2.0",
  "org.scala-lang.modules" % "scala-xml_2.11" % "1.0.5",
  "org.apache.commons" % "commons-io" % "2.8.0",
)

import sbtassembly.AssemblyPlugin.autoImport._

lazy val preprocessJar = taskKey[File]("preprocess-jar")


// preprocessJar := {
//   val jarFile = (Compile / packageBin).value
//   val targetFile = target.value / "preprocess.jar"
//   IO.copyFile(jarFile, targetFile)
//   targetFile
// }
// addSbtPlugin("com.eed3si9n" % "sbt-assembly" % "1.1.0")



// the following specs are for the combination of the various assembly plugins in uber jar format (by checking and removing those jars that have path conflicts)

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

// enablePlugins(AssemblyPlugin)