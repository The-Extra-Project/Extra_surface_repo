// name := "preprocess"
// version := "1.0"
scalaVersion := "2.13.0"
// offline := true

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

sparkComponents := Seq("core", "sql", "graphx")
sparkVersion := "3.5.1"

// libraryDependencies ++= Seq(
//   "org.scala-lang.modules" % "scala-xml_2.11" % "1.0.5",
//   "org.apache.spark" %% "spark-core" % sparkVersion.value,
//   "org.apache.spark" %% "spark-sql" % sparkVersion.value,
//   "org.apache.spark" %% "spark-graphx" % sparkVersion.value
// )

libraryDependencies ++= Seq(
  // "org.scala-lang.modules" %% "scala-parser-combinators" % "1.1.0",
  // "org.scala-lang.modules" % "scala-xml_2.11" % "1.0.5",
  "org.apache.spark" %% "spark-core" % sparkVersion.value,
  "org.apache.spark" %% "spark-sql" % sparkVersion.value,
  "org.apache.spark" %% "spark-graphx" % sparkVersion.value,
  // "org.locationtech.geotrellis" %% "geotrellis-raster" % "1.0.0",
  // "org.locationtech.geotrellis" %% "geotrellis-spark" % "1.0.0",
  // "org.vegas-viz" %% "vegas-spark" % "0.3.11"
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",  // Ensure Hadoop version matches
  "org.scala-lang.modules" %% "scala-parallel-collections" % "0.2.0",
  "org.scala-lang" % "scala-xml" % "2.1.0",
  "org.apache.commons" % "commons-io" % "2.8.0"
)

spIgnoreProvided := true

enablePlugins(AssemblyPlugin)

/* Assembly settings
assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case x => MergeStrategy.first
} */

lazy val preprocessJar = taskKey[File]("preprocess-jar")

lazy val wasureJar = taskKey[File]("wasure-jar")

preprocessJar := {
  val preprocessSource = (sourceDirectory in Compile).value / "scala" / "preprocess"
  val target = target.value / "preprocess.jar"
  // Do the assembly of the JAR for the preprocess code
  sbt.Compile / packageBin toTask (preprocessSource, target)
}

wasureJar := {
  val wasureSource = (sourceDirectory in Compile).value / "scala" / "wasure"
  val target = target.value / "wasure.jar"
  // Do the assembly of the JAR for the wasure code
  sbt.Compile / packageBin toTask (wasureSource, target)
}