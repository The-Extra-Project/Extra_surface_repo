 resolvers += "Spark Package Main Repo" at "https://dl.bintray.com/spark-packages/maven"
// resolvers += "spark-packages" at "https://repos.spark-packages.org/"
// resolvers += "GitHub Packages" at "https://maven.pkg.github.com/codecommit/sbt-github-packages"
resolvers += "Maven Central" at "https://repo1.maven.org/maven2/"
// resolvers += "bintray-spark-packages" at "https://dl.bintray.com/spark-packages/maven/"

resolvers += "spark-packages" at "https://repos.spark-packages.org/"


addSbtPlugin("com.github.mpeltonen" % "sbt-idea" % "1.6.0")

addSbtPlugin("org.spark-packages" % "sbt-spark-package" % "0.2.6")

addSbtPlugin("net.virtual-void" % "sbt-dependency-graph" % "0.8.2")

addSbtPlugin("org.scalastyle" % "scalastyle-sbt-plugin" % "1.0.0")

addSbtPlugin("org.scoverage" % "sbt-scoverage" % "1.5.0")

addSbtPlugin("org.scalariform" % "sbt-scalariform" % "1.8.0")

addSbtPlugin("com.eed3si9n" % "sbt-assembly" % "0.14.3")

addSbtPlugin("org.scoverage" % "sbt-scoverage" % "1.6.1")

// addSbtPlugin("nl.gn0s1s" %% "sbt-dotenv" % "3.1.0")

// addSbtPlugin("com.codecommit" % "sbt-github-packages" % "0.5.3")
