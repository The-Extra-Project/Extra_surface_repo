import sbt._
import sbt.Keys._

object CommonSettings extends AutoPlugin {
  override def trigger = allRequirements

  override def projectSettings: Seq[Def.Setting[_]] = Seq(
    scalaVersion := "2.12.18",
    scalacOptions += "-target:jvm-1.8",
    javacOptions ++= Seq("-source", "1.8"),
    resolvers ++= Seq(
      "Maven Central" at "https://repo1.maven.org/maven2/",
      "Typesafe Repository" at "https://repo.typesafe.com/typesafe/ivy-releases/",
      "Scala SBT" at "https://repo.scala-sbt.org/scalasbt/sbt-plugin-releases/",
      "Sonatype OSS Snapshots" at "https://oss.sonatype.org/content/repositories/snapshots"
    )
  )
}