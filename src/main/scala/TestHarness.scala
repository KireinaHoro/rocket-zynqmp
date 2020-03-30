package zynqmp

import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.debug.SimJTAG
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.system.SimAXIMem
import sifive.blocks.devices.uart.UARTAdapter

class TestHarness(implicit p: Parameters) extends Module {
  val io = IO(new Bundle {
    val success = Output(Bool())
  })

  val ldut = LazyModule(new TVMTop)
  val dut = Module(ldut.module)

  val systemjtag = dut.debug.head.systemjtag.head
  // set JTAG parameters
  systemjtag.reset := reset
  systemjtag.mfr_id := 0x489.U(11.W)
  systemjtag.part_number := 0.U(16.W)
  systemjtag.version := 2.U(4.W)

  // Remote Bitbang
  val rbbJtag = Module(new SimJTAG)
  rbbJtag.connect(systemjtag.jtag, clock, reset.asBool, true.B, io.success)

  // AXI Memory
  SimAXIMem.connectMem(ldut)

  // UART - only connect the first one
  val uartDpi = Module(new UARTAdapter(0, 115200))
  uartDpi.io.uart <> dut.uart.head

  dut.uart(1).rxd := 0.U
}
