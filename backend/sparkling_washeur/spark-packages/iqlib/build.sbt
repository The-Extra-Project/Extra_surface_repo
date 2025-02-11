

name := "iqlib"
version := "0.1"
scalaVersion := "2.12.18"

scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

resolvers +=  "commons-io" at "https://mvnrepository.com/artifact"

libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % "3.5.1",
  "org.apache.spark" %% "spark-sql" % "3.5.1",
  "org.apache.spark" %% "spark-graphx" % "3.5.1",
  "org.apache.hadoop" % "hadoop-common" % "3.3.6",
  "org.scala-lang.modules" %% "scala-xml" % "2.1.0",
  "commons-io" % "commons-io" % "2.11.0"
)

ThisBuild / packageTimestamp := Package.keepTimestamps

lazy val iqlibJar = taskKey[File]("iqlib-jar")

assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

iqlibJar := {
  val jarFile = (Compile / packageBin).value
  val targetFile = target.value / "iqlib.jar"
  IO.copyFile(jarFile, targetFile)
  targetFile
}