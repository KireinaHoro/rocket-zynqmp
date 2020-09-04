package zynqmp

import zynqmp._

import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.devices.debug.Debug
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._
import sifive.blocks.devices.uart._

class TestHarness(implicit p: Parameters) extends Module {
  val io = IO(new Bundle {
    val success = Output(Bool())
  })

  val ldut = LazyModule(new VerilatorTop)
  val dut = Module(ldut.module)

  // Allow the debug ndreset to reset the dut, but not until the initial reset has completed
  dut.reset := reset.asBool | dut.debug.map { debug => AsyncResetReg(debug.ndreset) }.getOrElse(false.B)

  // Connect debug
  Debug.connectDebug(dut.debug, dut.resetctrl, dut.psd, clock, reset.asBool, io.success)

  // AXI Memory
  SimAXIMem.connectMem(ldut)

  // UART - only connect the first one
  // baud rate 15M, frequency 100
  UARTAdapter.connect(dut.uart, BigInt(15000000))
}

class VerilatorTop(implicit p: Parameters) extends RocketSubsystem
  with HasPeripheryUART
  with CanHaveMasterAXI4MemPort {
  override lazy val module = new VerilatorTopModuleImp(this)

  setResetVector(this)
}

class VerilatorTopModuleImp[+L <: VerilatorTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasPeripheryUARTModuleImp
  with DontTouch
