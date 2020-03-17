package zynqmp

import chipsalliance.rocketchip.config._
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

trait SHA3Params extends Params {
  override val AuxConfig = new sha3.WithSha3Accel
}
