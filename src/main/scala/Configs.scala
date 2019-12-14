package zcu102

import chisel3._

import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink.BootROMParams
import freechips.rocketchip.system._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.subsystem.MemoryPortParams
import freechips.rocketchip.rocket.{RocketCoreParams, MulDivParams, DCacheParams, ICacheParams}
import freechips.rocketchip.tile.{RocketTileParams, XLen}

class WithBootROM extends Config((site, here, up) => {
  case BootROMParams => BootROMParams(
    hang = 0x10000,  // _start in bootrom
    contentFileName = s"./bootrom/bootrom.rv${site(XLen)}.img")
})

class WithMemMMIO extends Config((site, here, up) => {
  case ExtMem => up(ExtMem, site).map(x => x.copy(
    master = x.master.copy(idBits = 6, base = 0x800000000L, size = 0x80000000L)))
  case ExtBus => up(ExtBus, site).map(x => x.copy(idBits = 6))
  case NExtTopInterrupts => 3
})

class MyConfig extends Config(
  new WithMemMMIO ++ new WithBootROM ++ new WithNBigCores(4) ++ new DefaultConfig)
class ZCU102Config extends Config(new WithoutTLMonitors ++ new MyConfig)
