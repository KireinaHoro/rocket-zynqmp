package zynqmp

import chisel3._
import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.debug.SystemJTAGIO
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._
import sifive.blocks.devices.uart.{HasPeripheryUART, HasPeripheryUARTModuleImp, UARTPortIO}

class TVMEvaluator(implicit p: Parameters) extends MultiIOModule {
  val inst = Module(LazyModule(new TVMTop).module)

  require(inst.mem_axi4.size == 1)
  require(inst.mmio_axi4.size == 1)
  require(inst.debug.head.systemjtag.size == 1)

  val _mem = inst.mem_axi4.head
  val mem = IO(_mem.cloneType)
  mem <> _mem

  val _mmio = inst.mmio_axi4.head
  val mmio = IO(_mmio.cloneType)
  mmio <> _mmio

  val _jtag = inst.debug.head.systemjtag.head
  val jtag = IO(Flipped(_jtag.jtag.cloneType))
  jtag <> _jtag.jtag

  val _uart = inst.uart
  val uart0 = IO(_uart.head.cloneType)
  val uart1 = IO(_uart.head.cloneType)
  uart0 <> _uart(0)
  uart1 <> _uart(1)

  val _interrupts = inst.interrupts
  val interrupts = IO(Input(UInt(EdgeBoardSmallParams.NInterrupts.W)))
  _interrupts := interrupts

  // set JTAG parameters
  _jtag.reset := inst.reset
  _jtag.mfr_id := 0x489.U(11.W)
  _jtag.part_number := 0.U(16.W)
  _jtag.version := 2.U(4.W)
}

class TVMTop(implicit p: Parameters) extends RocketSubsystem
  with HasHierarchicalBusTopology
  with HasPeripheryUART
  with HasPeripheryBootROM
  with HasAsyncExtInterrupts
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new TVMTopModuleImp(this)
}

class TVMTopModuleImp[+L <: TVMTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with HasPeripheryUARTModuleImp
  with HasPeripheryBootROMModuleImp
  with DontTouch {
  lazy val mem_axi4 = outer.mem_axi4
  lazy val mmio_axi4 = outer.mmio_axi4
}
