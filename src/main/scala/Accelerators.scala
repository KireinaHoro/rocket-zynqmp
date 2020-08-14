package zynqmp

import chipsalliance.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.diplomacy.{LazyModule, ValName}
import freechips.rocketchip.subsystem._
import freechips.rocketchip.tile.{BuildRoCC, OpcodeSet}
import sifive.blocks.devices.uart._

trait DMIDebug extends Params {
  override val DebugConfig = new Config((site, here, up) => {
    case ExportDebug => up(ExportDebug, site).copy(protocols = Set(DMI))
  })
}
