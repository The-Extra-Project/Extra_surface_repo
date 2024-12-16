import sbt._
import sbt.Keys._
import sbtassembly.AssemblyPlugin.autoImport._


name := "iqlib-spark"
version := "1.0"
scalaVersion := "2.13.10"
offline := true


scalacOptions += "-target:jvm-1.8"
javacOptions ++= Seq("-source", "1.8")

sparkVersion := "3.5.1"

val githubOwner = settingKey[String]("GitHub owner")
val githubRepository = settingKey[String]("GitHub repository")

githubOwner := "The-Extra-Project"
githubRepository := "Extra_surface_repo"


publishTo := {
  val githubRepo = s"https://maven.pkg.github.com/${githubOwner.value}/${githubRepository.value}"
  if (isSnapshot.value)
    Some("GitHub Package Registry Snapshots" at githubRepo)
  else
    Some("GitHub Package Registry Releases" at githubRepo)
}

credentials += Credentials(
  "GitHub Package Registry",
  "maven.pkg.github.com",
  sys.env.getOrElse("GITHUB_USERNAME", ""
  
  ),
  sys.env.getOrElse("GITHUB_TOKEN", "")
)

sparkVersion := "3.5.1"
sparkComponents := Seq("core", "sql", "graphx")



assemblyMergeStrategy in assembly := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _ => MergeStrategy.first
}

// lazy val sparkJar = taskKey[File]("spark-jar")

// sparkJar := {
//   val jarFile = (Compile / packageBin).value
//   val targetFile = target.value / "spark.jar"
//   IO.copyFile(jarFile, targetFile)
//   targetFile
// }