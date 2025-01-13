import sbt._
import sbt.Keys._
import sbtassembly.AssemblyPlugin.autoImport._

// Define the iqlib project with the correct path
lazy val iqlib = project.in(file("../services/iqlib"))

// Add iqlib as a dependency
lazy val root = (project in file("."))
  .dependsOn(iqlib)
  .settings(
    name := "sparkling_washeur",
    version := "1.0",
    scalaVersion := "2.12.18",
    offline := true,
    scalacOptions += "-target:jvm-1.8",
    javacOptions ++= Seq("-source", "1.8"),
    sparkVersion := "3.5.1",
    sparkComponents := Seq("core", "sql", "graphx"),
    assemblyMergeStrategy in assembly := {
      case PathList("META-INF", xs @ _*) => MergeStrategy.discard
      case _ => MergeStrategy.first
    }
  )