name := "workflow_preprocess"
version := "0.1"
scalaVersion := "2.13.0"

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

resolvers +=  "commons-io" at "https://mvnrepository.com/artifact"


unmanagedJars in Compile += file(baseDirectory.value + "/src/spark/target/scala-2.13/iqlib-spark-assembly-1.0.jar")


libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",  // Ensure Hadoop version matches
  "org.scala-lang.modules" %% "scala-parallel-collections" % "1.0.4",
  "org.scala-lang.modules" %% "scala-xml" % "2.1.0",
  "commons-io" % "commons-io" % "2.11.0",
)

dependencyOverrides ++= Seq(
  "org.apache.curator" % "curator-recipes" % "5.2.0",
  "org.apache.curator" % "curator-framework" % "5.2.0",
  "org.apache.curator" % "curator-client" % "5.2.0",
  "com.nimbusds" % "nimbus-jose-jwt" % "9.8.1",
  "com.google.guava" % "guava" % "27.0-jre",
  "org.apache.yetus" % "audience-annotations" % "0.13.0",
  "io.airlift" % "aircompressor" % "0.25",
  "com.google.protobuf" % "protobuf-java" % "3.19.6",
  "io.netty" % "netty-transport-native-epoll" % "4.1.96.Final",
  "io.netty" % "netty-handler" % "4.1.96.Final",
  "io.dropwizard.metrics" % "metrics-core" % "4.2.19",
  "org.slf4j" % "slf4j-api" % "2.0.7",
)

import sbtassembly.AssemblyPlugin.autoImport._
import sbt.Keys._

lazy val preprocessJar = taskKey[File]("preprocess-jar")

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

preprocessJar := {
  val jarFile = (Compile / packageBin).value
  val targetFile = target.value / "preprocess.jar"
  IO.copyFile(jarFile, targetFile)
  targetFile
}


