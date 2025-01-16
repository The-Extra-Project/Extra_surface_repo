name := "sparkling_wasure_core"
version := "0.1"
scalaVersion := "2.12.18"

import sbtassembly.AssemblyPlugin.autoImport._
import sbt.Keys._

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

resolvers +=  "commons-io" at "https://mvnrepository.com/artifact"

// unmanagedJars in Compile += file(baseDirectory.value / "services/iqlib/target/scala-2.12/iqlib-assembly-0.1.jar")

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

ThisBuild / packageTimestamp := Package.keepTimestamps
//unmanagedJars in Compile += file(baseDirectory.value + "/src/spark/target/scala-2.12/iqlib-spark-assembly-1.0.jar")
libraryDependencies ++= Seq(
 "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",
  "org.scala-lang.modules" %% "scala-xml" % "2.1.0",
  "commons-io" % "commons-io" % "2.11.0"
)


import sbtassembly.AssemblyPlugin.autoImport._
import sbt.Keys._

lazy val wasureJar = taskKey[File]("wasure-jar")

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

wasureJar := {
  val jarFile = (Compile / packageBin).value
  val targetFile = target.value / "preprocess.jar"
  IO.copyFile(jarFile, targetFile)
  targetFile
}

  


