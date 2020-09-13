import mill._
import mill.modules.Util
import scalalib._
import scalafmt._

import $file.`rocket-chip`.firrtl.{build => Firrtl}
import $file.`rocket-chip`.chisel3.{build => Chisel3}

object Common { def scalaVersion = "2.12.10" }
import Common._

object myfirrtl extends Firrtl.firrtlCrossModule(scalaVersion) {
  override def millSourcePath = super.millSourcePath / 'firrtl

  // fix stupid JAR download
  override def downloadAntlr4Jar = T.persistent {
    Util.download(s"https://www.antlr.org/download/antlr-$antlr4Version-complete.jar")
  }
  override def downloadProtocJar = T.persistent {
    Util.download(s"https://repo.maven.apache.org/maven2/com/github/os72/protoc-jar/$protocVersion/protoc-jar-$protocVersion.jar")
  }
}
object mychisel3 extends Chisel3.chisel3CrossModule(scalaVersion) {
  override def millSourcePath = super.millSourcePath / 'chisel3
  def firrtlModule: Option[PublishModule] = Some(myfirrtl)
}

trait CommonModule extends ScalaModule with ScalafmtModule {
  def scalaVersion = Common.scalaVersion
  override def scalacOptions = Seq("-Xsource:2.11")
  def moduleDeps: Seq[ScalaModule] = Seq(mychisel3)

  private val macroParadise = ivy"org.scalamacros:::paradise:2.1.0"
  override def compileIvyDeps = Agg(macroParadise)
  override def scalacPluginIvyDeps = Agg(macroParadise)
}

// use alternative name to avoid name conflict with $file import
object rocketchip extends CommonModule with SbtModule {
  override def millSourcePath = super.millSourcePath / os.up / "rocket-chip"

  object `api-config-chipsalliance` extends CommonModule {
    override def millSourcePath = super.millSourcePath / 'design / 'craft
  }
  object macros extends CommonModule with SbtModule
  object hardfloat extends CommonModule with SbtModule

  override def ivyDeps = super.ivyDeps() ++ Agg(
    ivy"${scalaOrganization()}:scala-reflect:${scalaVersion()}",
    ivy"org.json4s::json4s-jackson:3.6.1"
  )
  override def moduleDeps = super.moduleDeps ++
    Seq(`api-config-chipsalliance`, macros, hardfloat)
}
trait RocketModule extends CommonModule {
  override def moduleDeps = super.moduleDeps ++ Seq(rocketchip)
}

object `sifive-blocks` extends RocketModule with SbtModule
object testchipip extends RocketModule with SbtModule {
  override def moduleDeps = super.moduleDeps ++ Seq(`sifive-blocks`)
}
object `inclusive-cache` extends RocketModule {
  override def millSourcePath = super.millSourcePath / 'design / 'craft / 'inclusivecache
}
object gemmini_conv extends RocketModule {
  override def millSourcePath = super.millSourcePath / 'src
}

object system extends RocketModule {
  // use project root as source root
  override def millSourcePath = super.millSourcePath / os.up
  override def moduleDeps = super.moduleDeps ++ Seq(testchipip, `sifive-blocks`, `inclusive-cache`, gemmini_conv)

  val project = "zynqmp"
  val buildDir = "build"
  val boardConfig = "EdgeBoardConfig"
  val boardTop = "EdgeBoardTop"
  val simConfig = "MidgardVerilatorConfig"
  val simTop = "TestHarness"

  def genFirrtl(top: String = boardTop, config: String = boardConfig) = T.command {
    os.makeDir.all(os.pwd / buildDir)
    runMain("freechips.rocketchip.system.Generator",
      "-td", buildDir,
      "-T", s"$project.$top",
      "-C", s"$project.$config")
  }
  def genVerilog(top: String = boardTop, config: String = boardConfig) = T.command {
    runMain("firrtl.stage.FirrtlMain",
      "-i", s"$buildDir/$project.$config.fir",
      "-o", s"$buildDir/$project.$config.v",
      "-X", "verilog")
  }
}
