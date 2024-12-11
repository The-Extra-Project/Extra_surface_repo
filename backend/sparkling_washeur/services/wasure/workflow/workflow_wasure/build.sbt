

name := "workflow_preprocess"
version := "0.1"
scalaVersion := "2.13.0"


resolvers += "spark-packages" at "https://dl.bintray.com/spark-packages/maven/"

sbt.version=1.0.0

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

sparkComponents ++= Seq("core", "sql", "graphx")
sparkVersion := "3.5.1"


libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",  // Ensure Hadoop version matches
  "org.scala-lang.modules" %% "scala-parallel-collections" % "0.2.0",
  "com.eed3si9n" % "sbt-assembly_2.12_1.0" % "2.1.3",
  "org.scala-lang" % "scala-xml" % "2.1.0",
  "commons-io" % "commons-io" % "2.18.0"

)


lazy val wasureJar = taskKey[File]("wasure-jar")


// wasureJar := {
//   val wasureSource = (sourceDirectory in Compile).value / "src" / "main" / "scala"
//   val target = target.value / "wasure.jar"
//   // Do the assembly of the JAR for the wasure code
//   sbt.Compile / packageBin toTask (wasureSource, target)
// }


// the following specs are for the combination of the various assembly plugins in uber jar format (by checking and removing those jars that have path conflicts)
// reference by copilot recommendation to resolve the errors:

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case x => MergeStrategy.first
}

enablePlugins(AssemblyPlugin)
