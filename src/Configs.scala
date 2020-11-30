package zynqmp

import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.diplomacy.DTSTimebase
import freechips.rocketchip.subsystem._

case object ResetVectorKey extends Field[BigInt]

class WithSystemMemory(base: BigInt = 0x80000000L, size: BigInt = 0x10000000L) extends Config((site, here, up) => {
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = base, size = size)))
})

class WithSystemMMIO(base: BigInt = 0x60000000L, size: BigInt = 0x20000000L) extends Config((site, here, up) => {
  case ExtBus => up(ExtBus, site).map(_.copy(idBits = 6, base = base, size = size))
})

class SystemPresets(systemFreq: BigInt = 100000000, nInterrupts: Int = 1) extends Config((site, here, up) => {
  case ResetVectorKey => BigInt(0x71000000L) // High 128Mbit of XIP SPI
  case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency = Some(systemFreq))
  case RocketTilesKey => up(RocketTilesKey, site) map { r =>
    r.copy(core = r.core.copy(bootFreqHz = systemFreq))
  }
  case DTSTimebase => systemFreq
  case NExtTopInterrupts => nInterrupts
  case DebugModuleKey => up(DebugModuleKey, site) map { _.copy(clockGate = false) }
})

class BaseSystemConfig extends Config(
  new WithCoherentBusTopology ++
  new SystemPresets(nInterrupts = 32) ++
  new WithoutTLMonitors ++
  new WithDefaultMemPort() ++
  new WithDefaultMMIOPort() ++
  new WithDTS("jsteward,compnet-lab-x", Nil) ++
  new BaseSubsystemConfig()
)

class WithBoardDebug extends Config(new WithNBreakpoints(8) ++ new WithJtagDTM)
class WithVerilatorDebug extends Config((site, here, up) => {
  case ExportDebug => up(ExportDebug, site).copy(protocols = Set(DMI))
})

class LabXConfig extends Config(
  new WithBoardDebug ++
  new WithSystemMemory(0x70000000L, 0x90000000L) ++ // XIP + DRAM on board
  new WithSystemMMIO(size = 0x10000000L) ++ // 0x60000000-0x70000000
  new WithNBigCores(1) ++
  new WithDefaultSlavePort ++
  new BaseSystemConfig
)

class VerilatorConfig extends Config(
  new WithVerilatorDebug ++
  new LabXConfig
)
