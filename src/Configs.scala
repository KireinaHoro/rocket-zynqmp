package zynqmp

import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.diplomacy.DTSTimebase
import freechips.rocketchip.subsystem._
import sifive.blocks.devices.uart._

class WithSystemMemory(base: BigInt = 0x80000000L, size: BigInt = 0x10000000L) extends Config((site, here, up) => {
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = base, size = size)))
})

class WithSystemMMIO(base: BigInt = 0x60000000L, size: BigInt = 0x20000000L) extends Config((site, here, up) => {
  case ExtBus => up(ExtBus, site).map(_.copy(idBits = 6, base = base, size = size))
})

class SystemPresets(systemFreq: BigInt = 100000000, nInterrupts: Int = 1) extends Config((site, here, up) => {
  case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency = Some(systemFreq))
  case RocketTilesKey => up(RocketTilesKey, site) map { r =>
    r.copy(core = r.core.copy(bootFreqHz = systemFreq))
  }
  case DTSTimebase => systemFreq
  case NExtTopInterrupts => nInterrupts
})

class SystemPeripherals extends Config((_, _, _) => {
  case PeripheryUARTKey => List(
    UARTParams(address = 0x10010000),
    UARTParams(address = 0x10011000),
  )
})

class BaseSystemConfig extends Config(
  new WithCoherentBusTopology ++
  new SystemPeripherals ++
  new SystemPresets(nInterrupts = 2) ++ // two AXI SPI controllers
  new WithoutTLMonitors ++
  new WithDefaultMemPort() ++
  new WithDefaultMMIOPort() ++
  new WithDTS("jsteward,uhf-rfid-ctrl", Nil) ++
  new BaseSubsystemConfig()
)

class WithBoardDebug extends Config(new WithNBreakpoints(4) ++ new WithJtagDTM)
class WithVerilatorDebug extends Config((site, here, up) => {
  case ExportDebug => up(ExportDebug, site).copy(protocols = Set(DMI))
})

class UhfRfidConfig extends Config(
  new WithBoardDebug ++
  new WithSystemMemory(0x80000000L, 0x80000000L) ++ // 2GB DRAM on board
  new WithSystemMMIO() ++
  new WithNBigCores(1) ++
  new BaseSystemConfig
)

class VerilatorConfig extends Config(
  new WithVerilatorDebug ++
  new UhfRfidConfig
)
