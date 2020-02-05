package zcu102

import chisel3._

import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.rocket._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.system._
import freechips.rocketchip.tile._

object Params {
  val RAMBase = 0x800000000L // Hi-2G base on PS DIMM
  val RAMSize = 0x80000000L // 2 GiB
  val NInterrupts = 7 // Ethernet, DMA MM2S, DMA S2MM, Timer, UART, IIC, SPI (SD card)
  val SystemFreq = 50000000L // 50 MHz
  val NBreakpoints = 8 // # Hardware breakpoints
  val NCores = 2 // # Big cores
}

class OverridingConfig extends Config((site, here, up) => {
  // BootROM
  case BootROMParams => BootROMParams(
    hang = 0x10000, // _start in bootrom
    contentFileName = s"./bootrom/bootrom.rv${site(XLen)}.img")
  // RAM
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = Params.RAMBase, size = Params.RAMSize)))
  case ExtBus => up(ExtBus, site).map(_.copy(idBits = 6))
  // Interrupts
  case NExtTopInterrupts => Params.NInterrupts
  // Frequency in DTS
  case DTSTimebase => BigInt(Params.SystemFreq)
  case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency = Some(Params.SystemFreq))
  case RocketTilesKey => up(RocketTilesKey, site) map { r =>
    r.copy(core = r.core.copy(bootFreqHz = BigInt(Params.SystemFreq)))
  }
})

class ZCU102Config extends Config(new WithoutTLMonitors ++
  new WithNBreakpoints(Params.NBreakpoints) ++ new WithJtagDTM ++
  new OverridingConfig ++
  new WithNBigCores(Params.NCores) ++ new DefaultConfig)
