import sbtassembly.AssemblyPlugin.autoImport._
import sbtassembly.MergeStrategy
import sbt.Keys._

lazy val commonSettings = Seq(
  scalaVersion := "2.12.18",
  scalacOptions += "-target:jvm-1.8",
  javacOptions ++= Seq("-source", "1.8"),
  resolvers ++= Seq(
    "Maven Central" at "https://repo1.maven.org/maven2/",
    "Typesafe Repository" at "https://repo.typesafe.com/typesafe/ivy-releases/",
    "Scala SBT" at "https://repo.scala-sbt.org/scalasbt/sbt-plugin-releases/",
    "Sonatype OSS Snapshots" at "https://oss.sonatype.org/content/repositories/snapshots",
  ),
)

lazy val iqlib = (project in file("iqlib"))
  .settings(commonSettings: _*)

lazy val spark = (project in file("spark"))
  .settings(commonSettings: _*)
  .dependsOn(iqlib)

lazy val workflowPreprocess = (project in file("workflow_preprocess"))
  .settings(commonSettings: _*)
  .dependsOn(spark, iqlib)


lazy val workflowWasure = (project in file("workflow_wasure"))
  .settings(commonSettings: _*)
  .dependsOn(spark, iqlib)


lazy val root = (project in file("."))
  .aggregate(iqlib, spark,
  workflowPreprocess, workflowWasure
  )
  .settings(commonSettings: _*)