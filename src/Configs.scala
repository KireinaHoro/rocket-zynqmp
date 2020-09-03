package zynqmp

import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy.DTSTimebase
import freechips.rocketchip.subsystem._
import freechips.rocketchip.tile._
import sifive.blocks.devices.pwm.{PWMParams, PeripheryPWMKey}
import sifive.blocks.devices.uart.{PeripheryUARTKey, UARTParams}

case object MemBaseKey extends Field[Long]
case object ResetKey extends Field[Long]

class WithSystemMemory(base: Long = 0x80000000L, size: Long = 0x10000000L) extends Config((site, here, up) => {
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = base, size = size)))
  case MemBaseKey => base
})

class WithSystemMMIO(base: Long = 0x60000000L, size: Long = 0x20000000L) extends Config((site, here, up) => {
  case ExtBus => up(ExtBus, site).map(_.copy(idBits = 6, base = base, size = size))
})

class WithMemReset extends Config((site, here, up) => {
  case ResetKey => up(MemBaseKey, site)
})

class SystemPresets(systemFreq: Long = 100000000, nInterrupts: Int = 1) extends Config((site, here, up) => {
  case PeripheryBusKey => up(PeripheryBusKey, site).copy(dtsFrequency = Some(systemFreq))
  case RocketTilesKey => up(RocketTilesKey, site) map { r =>
    r.copy(core = r.core.copy(bootFreqHz = BigInt(systemFreq)))
  }
  case DTSTimebase => systemFreq
  case NExtTopInterrupts => nInterrupts
})

class SystemPeripherals extends Config((_, _, _) => {
  case PeripheryUARTKey => List(
    UARTParams(address = 0x10010000),
    UARTParams(address = 0x10011000),
  )
  case PeripheryPWMKey => List(
    PWMParams(address = 0x10020000),
    PWMParams(address = 0x10021000),
  )
})

class BaseSystemConfig extends Config(
  new WithMemReset ++
  new SystemPeripherals ++
  new SystemPresets() ++
  new WithoutTLMonitors ++
  new WithDefaultMemPort() ++
  new WithDefaultMMIOPort() ++
  new WithDTS("freechips,rocketchip-jsteward", Nil) ++
  new BaseSubsystemConfig()
)

class WithBoardDebug extends Config(new WithNBreakpoints(4) ++ new WithJtagDTM)
class WithVerilatorDebug extends Config((site, here, up) => {
  case ExportDebug => up(ExportDebug, site).copy(protocols = Set(JTAG))
})

class EdgeBoardConfig extends Config(
  new WithBoardDebug ++
  new WithSystemMemory(0x40000000L, 0x3ff00000L) ++ // high 1G of PS DDR
  new WithSystemMMIO(base = 0xe0000000L) ++ // ZynqMP peripherals
  new WithNBigCores(1) ++
  new BaseSystemConfig
)

class MidgardVerilatorConfig extends Config(
  new WithVerilatorDebug ++
  new EdgeBoardConfig
)
