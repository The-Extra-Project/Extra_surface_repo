import sbtassembly.AssemblyPlugin.autoImport._
import sbtassembly.MergeStrategy
import sbt.Keys._
import scala.sys.process._


lazy val preprocessJar = taskKey[File]("Preprocess the JAR file")

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
  .settings(
    name := "workflow_preprocess",
    version := "0.1",
    libraryDependencies ++= Seq(
      "org.apache.spark" %% "spark-core" % "3.5.1",
      "org.apache.spark" %% "spark-sql" % "3.5.1",
      "org.apache.spark" %% "spark-graphx" % "3.5.1",
      "org.apache.hadoop" % "hadoop-common" % "3.3.6",
      "org.scala-lang.modules" %% "scala-xml" % "2.1.0",
      "commons-io" % "commons-io" % "2.11.0",
      "org.scalatest" %% "scalatest" % "3.2.19" % "test"
    ),
      Test / classLoaderLayeringStrategy := ClassLoaderLayeringStrategy.ScalaLibrary,
          // Add these lines
     // Add these lines
    fork in Test := true,
    javaOptions in Test ++= Seq(
      "--add-opens=java.base/sun.nio.ch=ALL-UNNAMED",
      "--add-opens=java.base/java.lang=ALL-UNNAMED",
      "--add-opens=java.base/java.lang.invoke=ALL-UNNAMED",
      "--add-opens=java.base/java.io=ALL-UNNAMED",
      "--add-opens=java.base/java.net=ALL-UNNAMED",
      "--add-opens=java.base/java.nio=ALL-UNNAMED",
      "--add-opens=java.base/java.util=ALL-UNNAMED",
      "--add-opens=java.base/java.util.concurrent=ALL-UNNAMED",
      "--add-opens=java.base/java.util.concurrent.atomic=ALL-UNNAMED",
      "--add-opens=java.base/sun.nio.ch=ALL-UNNAMED",
      "--add-opens=java.base/sun.nio.cs=ALL-UNNAMED",
      "--add-opens=java.base/sun.security.action=ALL-UNNAMED",
      "--add-opens=java.base/sun.util.calendar=ALL-UNNAMED",
      "-Xmx2g"
    ),
    
    // Optional: If you want to apply these options to run tasks as well
    fork in run := true,
    javaOptions in run ++= (javaOptions in Test).value,
    

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
    ),
    unmanagedJars in Compile += baseDirectory.value.getParentFile / "iqlib-spark-assembly-1.0.jar",
    testOptions in Test += Tests.Argument(TestFrameworks.ScalaTest, "-oD"),
    assemblyMergeStrategy in assembly := {
      case PathList("META-INF", xs @ _*) => MergeStrategy.discard
      case _ => MergeStrategy.first
    },
    preprocessJar := {
      val jarFile = (Compile / packageBin).value
      val targetFile = target.value / "preprocess.jar"
      IO.copyFile(jarFile, targetFile)
      targetFile
    }
  )
  .dependsOn(spark, iqlib)

lazy val workflowWasure = (project in file("workflow_wasure"))
  .settings(commonSettings: _*)
  .dependsOn(spark, iqlib)


lazy val root = (project in file("."))
  .aggregate(iqlib, spark,
  workflowPreprocess, workflowWasure
  )
  .settings(commonSettings: _*)