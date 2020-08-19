package zynqmp

import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.debug.Debug
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.util.AsyncResetReg
import sifive.blocks.devices.uart.UARTAdapter

class TestHarness(implicit p: Parameters) extends Module {
  val io = IO(new Bundle {
    val success = Output(Bool())
  })

  val ldut = LazyModule(new TVMTop)
  val dut = Module(ldut.module)

  // Allow the debug ndreset to reset the dut, but not until the initial reset has completed
  dut.reset := reset.asBool | dut.debug.map { debug => AsyncResetReg(debug.ndreset) }.getOrElse(false.B)

  // Connect debug
  Debug.connectDebug(dut.debug, dut.psd, clock, reset.asBool, io.success)

  // AXI Memory
  SimAXIMem.connectMem(ldut)

  // UART - only connect the first one
  val uartDpi = Module(new UARTAdapter(0, 15000000))
  uartDpi.io.uart <> dut.uart.head
  dut.uart(1).rxd := 0.U
}
