package zynqmp

import chipsalliance.rocketchip.config._
import chisel3._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._
import sifive.blocks.devices.spi._

package object zynqmp {
  def setResetVector(subsystem: BaseSubsystem with HasTiles)(implicit p: Parameters) {
    // assume that we only have one QSPI flash
    val resetVector = p(PeripherySPIFlashKey).head.fAddress
    println(f"global reset vector at ${resetVector}%#x")

    val resetVectorSourceNode = BundleBridgeSource[UInt]()
    subsystem.tileResetVectorNexusNode := resetVectorSourceNode
    InModuleBody { resetVectorSourceNode.bundle := resetVector.U }
  }
}
