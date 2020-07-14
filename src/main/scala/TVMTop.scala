package zynqmp

import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._
import sifive.blocks.devices.uart.{HasPeripheryUART, HasPeripheryUARTModuleImp}
import nvidia.blocks.dla.CanHavePeripheryNVDLA

class TVMEvaluator(implicit p: Parameters) extends MultiIOModule {
  val inst = Module(LazyModule(new TVMTop).module)

  require(inst.mem_axi4.size == 1)
  require(inst.debug.head.systemjtag.size == 1)

  val _mem = inst.mem_axi4.head
  val mem = IO(_mem.cloneType)
  mem <> _mem

  val _jtag = inst.debug.head.systemjtag.head
  val jtag = IO(Flipped(_jtag.jtag.cloneType))
  jtag <> _jtag.jtag

  val _uart = inst.uart
  val uart0 = IO(_uart.head.cloneType)
  val uart1 = IO(_uart.head.cloneType)
  uart0 <> _uart(0)
  uart1 <> _uart(1)

  // set JTAG parameters
  _jtag.reset := inst.reset
  _jtag.mfr_id := 0x489.U(11.W)
  _jtag.part_number := 0.U(16.W)
  _jtag.version := 2.U(4.W)
}

class TVMTop(implicit p: Parameters) extends RocketSubsystem
  with HasHierarchicalBusTopology
  with HasPeripheryUART
  with CanHavePeripheryNVDLA
  with CanHaveMasterAXI4MemPort {
  override lazy val module = new TVMTopModuleImp(this)

  def resetVector = p(BootROMParams).hang
}

class TVMTopModuleImp[+L <: TVMTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasPeripheryUARTModuleImp
  with HasResetVectorWire
  with DontTouch {
  lazy val mem_axi4 = outer.mem_axi4

  println(f"global reset vector at 0x${outer.resetVector}%x")
  global_reset_vector := outer.resetVector.U
}
