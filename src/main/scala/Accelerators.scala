package zynqmp

import chipsalliance.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.diplomacy.{LazyModule, ValName}
import freechips.rocketchip.subsystem._
import freechips.rocketchip.tile.{BuildRoCC, OpcodeSet}
import gemmini.{CapacityInKilobytes, Gemmini, GemminiConfigs}
import nvidia.blocks.dla._
import sifive.blocks.devices.uart._

class AlternativeGemminiConfig extends Config((site, here, up) => {
  case BuildRoCC => Seq(
    (p: Parameters) => {
      implicit val q = p
      implicit val v = implicitly[ValName]
      LazyModule(new Gemmini(
        OpcodeSet.custom3,
        GemminiConfigs.defaultConfig.copy(
          meshRows = 8, // orig: 16
          meshColumns = 8, // orig: 16
          ld_queue_length = 4, // orig: 8
          ex_queue_length = 4, // orig: 8
          sp_capacity = CapacityInKilobytes(128), // orig: 256K
          acc_capacity = CapacityInKilobytes(32), // orig: 64K
        )))
    }
  )
  case SystemBusKey => up(SystemBusKey).copy(beatBytes = 16)
})

trait GemminiParams extends Params {
  override val CoreConfig = new WithNMedCores(1)
  override val NInterrupts = 0
  override val AuxConfig = Common.l2cache ++ Common.uart ++
    //new gemmini.DefaultGemminiConfig
    new AlternativeGemminiConfig
}


trait NVDLAParams extends Params {
  // use big cores
  override val NInterrupts = 0 // no external interrupts
  override val AuxConfig = Common.uart ++
    new WithNVDLA("small")
}

trait SHA3Params extends Params {
  override val AuxConfig = new sha3.WithSha3Accel
}

trait DMIDebug extends Params {
  override val DebugConfig = new Config((site, here, up) => {
    case ExportDebug => up(ExportDebug, site).copy(protocols = Set(DMI))
  })
}

class ZCU102GemminiParams extends ZCU102 with GemminiParams with RAMInit

class ZCU102GemminiConfig extends BoardConfig(new ZCU102GemminiParams)

class VerilatorGemminiParams extends ZCU102GemminiParams with DMIDebug

class VerilatorGemminiConfig extends BoardConfig(new VerilatorGemminiParams)

class ZCU102NVDLAParams extends ZCU102 with NVDLAParams with RAMInit {
  override val RAMBase = 0x80000000L // DDR4 SDRAM component 512M
  override val RAMSize = 0x20000000L
  override val BootROMHang = RAMBase
}

class ZCU102NVDLAConfig extends BoardConfig(new ZCU102NVDLAParams)
