package zynqmp

import freechips.rocketchip.config._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.system._
import freechips.rocketchip.tile._

object EdgeBoardParams extends Params {
  override val RAMBase = 0x40000000L // High 1G
  override val RAMSize = 0x3ff00000L // 1 GiB - PMU reserved
  override val MMIOBase = 0xe0000000L // ZynqMP Peripherals
  val NInterrupts = 4 // UART, Ethernet, Ethernet Wake, MMC
  val SystemFreq = 100000000L // 100 MHz
  val NBreakpoints = 8 // # Hardware breakpoints
  val NCores = 1 // # Big cores
}
class EdgeBoardConfig extends BoardConfig(EdgeBoardParams)

object ZCU102Params extends Params {
  override val RAMBase = 0x800000000L // High 2G of SODIMM
  override val RAMSize = 0x80000000L // 2 GiB
  val NInterrupts = 7 // Ethernet, DMA MM2S, DMA S2MM, Timer, UART, IIC, SPI (SD card)
  val SystemFreq = 50000000L // 50 MHz
  val NBreakpoints = 8 // # Hardware breakpoints
  val NCores = 1 // # Big cores
}
class ZCU102Config extends BoardConfig(ZCU102Params)

abstract class Params {
  val RAMBase: Long = 0x80000000L
  val RAMSize: Long = 0x10000000L
  val MMIOBase: Long = 0x60000000L
  val MMIOSize: Long = 0x20000000L
  val BootROMBase: Int = 0x10000
  val BootROMSize: Int = 0x2000
  val BootROMHang: Int = 0x10000
  val SystemFreq: Long
  val NInterrupts: Int
  val NBreakpoints: Int
  val NCores: Int
}

class OverridingConfig[+T <: Params](params: T) extends Config((site, here, up) => {
  // BootROM
  case BootROMParams => BootROMParams(
    address = params.BootROMBase,
    size = params.BootROMSize,
    hang = params.BootROMHang, // _start in bootrom
    contentFileName = s"./bootrom/bootrom.rv${site(XLen)}.img")
  // RAM
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = params.RAMBase, size = params.RAMSize)))
  case ExtBus => up(ExtBus, site).map(_.copy(idBits = 6, base = params.MMIOBase, size = params.MMIOSize))
  // Frequency in DTS
  case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency = Some(params.SystemFreq))
  case RocketTilesKey => up(RocketTilesKey, site) map { r =>
    r.copy(core = r.core.copy(bootFreqHz = BigInt(params.SystemFreq)))
  }
})

class BoardConfig[+T <: Params](params: T) extends Config(new WithoutTLMonitors ++
  new WithNBreakpoints(params.NBreakpoints) ++
  new WithJtagDTM ++
  new OverridingConfig(params) ++
  new WithNBigCores(params.NCores) ++
  new WithDefaultMemPort() ++
  new WithDefaultMMIOPort() ++
  new WithTimebase(params.SystemFreq) ++
  new WithDTS("freechips,rocketchip-jsteward", Nil) ++
  new WithNExtTopInterrupts(params.NInterrupts) ++
  new BaseSubsystemConfig()
)
