ThisBuild / organization := "pku"
ThisBuild / version := "1.0"
ThisBuild / scalaVersion := "2.12.10"
ThisBuild / parallelExecution := false
ThisBuild / traceLevel := 15
ThisBuild / scalacOptions := Seq("-unchecked", "-deprecation", "-Xsource:2.11", "-feature", "-language:reflectiveCalls")

lazy val rocketChip = RootProject(file("rocket-chip"))

val deps = Map(
  "chisel-iotesters" -> "1.3.3"
)
val chisel_iotesters = Seq(
  "chisel-iotesters"
).map {
  dep: String => "edu.berkeley.cs" %% dep % sys.props.getOrElse(dep + "Version", deps(dep))
}

lazy val sifiveBlocks = (project in file("sifive-blocks"))
  .dependsOn(rocketChip)

lazy val testchipip = (project in file("testchipip"))
  .dependsOn(sifiveBlocks, rocketChip)

unmanagedSourceDirectories in Compile += (baseDirectory( _ / "inclusive-cache/design/craft/inclusivecache" )).value
unmanagedSourceDirectories in Compile += (baseDirectory( _ / "nvdla" )).value

lazy val gemmini = (project in file("gemmini"))
  .dependsOn(rocketChip, testchipip)
  .settings(
    libraryDependencies ++= chisel_iotesters,
  )

lazy val root = (project in file("."))
  .dependsOn(rocketChip, gemmini, sifiveBlocks)

addCompilerPlugin("org.scalamacros" % "paradise" % "2.1.0" cross CrossVersion.full)
