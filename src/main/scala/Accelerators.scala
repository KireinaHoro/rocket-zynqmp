package zynqmp

import chipsalliance.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.subsystem._
import sifive.blocks.devices.uart._

trait GemminiParams extends Params {
  override val CoreConfig = new WithNMedCores(1)
  override val NInterrupts = 0
  override val AuxConfig = new gemmini.DefaultGemminiConfig ++
    new Config((_, _, _) => {
      case PeripheryUARTKey => List(
        UARTParams(address = 0x10010000),
        UARTParams(address = 0x10020000),
      )
    }) ++
    new WithInclusiveCache
}

object VerilatorGemminiParams extends ZCU102 with GemminiParams {
  override val BootROMHang = RAMBase
  override val DebugConfig = new Config((site, here, up) => {
    case ExportDebug => up(ExportDebug, site).copy(protocols = Set(DMI))
  })
}
class VerilatorGemminiConfig extends BoardConfig(VerilatorGemminiParams)

object ZCU102GemminiParams extends ZCU102 with GemminiParams {
  override val BootROMHang = RAMBase
}
class ZCU102GemminiConfig extends BoardConfig(ZCU102GemminiParams)

trait SHA3Params extends Params {
  override val AuxConfig = new sha3.WithSha3Accel
}
