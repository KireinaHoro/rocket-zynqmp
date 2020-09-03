package zynqmp

import chisel3._
import chipsalliance.rocketchip.config._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._

package object zynqmp {
  def setResetVector(subsystem: BaseSubsystem with HasTiles)(implicit p: Parameters) {
    def resetVector = p(ResetKey)
    println(f"global reset vector at ${resetVector}%#x")

    val resetVectorSourceNode = BundleBridgeSource[UInt]()
    subsystem.tileResetVectorNexusNode := resetVectorSourceNode
    InModuleBody { resetVectorSourceNode.bundle := resetVector.U }
  }
}
