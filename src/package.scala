package zynqmp

import chipsalliance.rocketchip.config._
import chisel3._
import freechips.rocketchip.devices.debug.SystemJTAGIO
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.jtag.JTAGIO
import freechips.rocketchip.subsystem._
import sifive.blocks.devices.spi._

package object zynqmp {
  def setResetVector(subsystem: BaseSubsystem with HasTiles)(implicit p: Parameters) {
    // assume that we only have one QSPI flash
    val resetVector = p(ResetVectorKey)
    println(f"global reset vector at ${resetVector}%#x")

    val resetVectorSourceNode = BundleBridgeSource[UInt]()
    subsystem.tileResetVectorNexusNode := resetVectorSourceNode
    InModuleBody { resetVectorSourceNode.bundle := resetVector.U }
  }

  def connectJTAG(dutJtag: SystemJTAGIO, jtag: JTAGIO)(implicit reset: Reset): Unit = {
    dutJtag.jtag <> jtag
    dutJtag.reset := reset
    dutJtag.mfr_id := 0x489.U(11.W)
    dutJtag.part_number := 0.U(16.W)
    dutJtag.version := 2.U(4.W)
  }
}
