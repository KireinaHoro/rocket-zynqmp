lazy val rocketChip = RootProject(file("rocket-chip/"))

lazy val commonSettings = Seq(
  organization := "pku",
  version := "1.0",
  scalaVersion := "2.12.10",
  parallelExecution := false,
  traceLevel := 15,
  scalacOptions := Seq("-unchecked", "-deprecation", "-Xsource:2.11", "-feature", "-language:reflectiveCalls")
)

val deps = Map(
  "chisel-iotesters" -> "1.3.3"
)
val chisel_iotesters = Seq(
  "chisel-iotesters"
).map {
  dep: String => "edu.berkeley.cs" %% dep % sys.props.getOrElse(dep + "Version", deps(dep))
}

lazy val sha3 = (project in file("sha3"))
  .dependsOn(rocketChip)
  .settings(
    commonSettings,
    libraryDependencies ++= chisel_iotesters
  )

lazy val root = (project in file("."))
  .dependsOn(rocketChip, sha3)
  .settings(commonSettings)

addCompilerPlugin("org.scalamacros" % "paradise" % "2.1.0" cross CrossVersion.full)
