lazy val root = (project in file("."))
  .settings(
    name := "zcu102",
    organization := "pku",
    version := "1.0",
    scalaVersion := "2.12.10",
    parallelExecution := false,
    traceLevel := 15,
    scalacOptions := Seq("-unchecked", "-deprecation", "-Xsource:2.11")
  )
