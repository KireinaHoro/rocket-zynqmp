package zynqmp

import chisel3._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.tilelink._

trait HasPeripheryBRAM {
  this: BaseSubsystem =>
  val dtb: DTB
  private val params = p(BootROMParams)

  val bram = LazyModule(new TLRAM(AddressSet(0x40000, 0x1ffff), None, false, true, false, cbus.beatBytes))

  bram.node := cbus.coupleTo("bram") {
    TLFragmenter(cbus) := _
  }
}

trait HasPeripheryBRAMModuleImp extends LazyModuleImp {
  val outer: HasPeripheryBRAM
}