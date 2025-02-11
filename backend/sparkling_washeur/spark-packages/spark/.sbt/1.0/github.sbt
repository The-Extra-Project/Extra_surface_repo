libraryDependencies += "me.shadaj" %% "dotenv-scala" % "0.1.0"

val githubUsername = sys.env.getOrElse("GITHUB_USERNAME", "")
val githubToken = sys.env.getOrElse("GITHUB_TOKEN", "")

if (githubUsername.isEmpty || githubToken.isEmpty) {
  println("Please set GITHUB_USERNAME and GITHUB_TOKEN environment variables")
  sys.exit(1)
}
credentials += 
  Credentials(
    "GitHub Package Registry",
    "maven.pkg.github.com",
    GITHUB_USERNAME,
    GITHUB_TOKEN)
