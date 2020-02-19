package zynqmp

import chisel3._

import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._

class EdgeBoardTop(implicit p: Parameters) extends BoardTop(EdgeBoardParams)
class ZCU102Top(implicit p: Parameters) extends BoardTop(ZCU102Params)

class BoardTop(params: Params)(implicit val p: Parameters) extends Module {
  val config = p(ExtIn)
  val target = Module(LazyModule(new RocketTop).module)

  require(target.mem_axi4.size == 1)
  require(target.mmio_axi4.size == 1)
  require(target.debug.head.systemjtag.size == 1)

  val memBundle = target.mem_axi4.head
  val mmioBundle = target.mmio_axi4.head
  val jtagBundle = target.debug.head.systemjtag.head

  val io = IO(new Bundle {
    val interrupts = Input(UInt(params.NInterrupts.W))
    val mem_axi4 = memBundle.cloneType
    val mmio_axi4 = mmioBundle.cloneType
  })

  io.mem_axi4 <> memBundle
  io.mmio_axi4 <> mmioBundle

  // connect JTAG - PL TAP is at position 1
  val boardJTAG = Module(new BscanJTAG(1))
  // set JTAG parameters
  jtagBundle.reset := reset
  jtagBundle.mfr_id := 0x489.U(11.W)
  jtagBundle.part_number := 0.U(16.W)
  jtagBundle.version := 2.U(4.W)
  // connect to BSCAN
  jtagBundle.jtag.TCK := boardJTAG.tck
  jtagBundle.jtag.TMS := boardJTAG.tms
  jtagBundle.jtag.TDI := boardJTAG.tdi
  boardJTAG.tdo := jtagBundle.jtag.TDO.data
  boardJTAG.tdoEnable := jtagBundle.jtag.TDO.driven

  target.interrupts := io.interrupts

  target.dontTouchPorts
}

class RocketTop(implicit p: Parameters) extends RocketSubsystem
  with HasHierarchicalBusTopology
  with HasPeripheryBootROM
  with HasPeripheryBRAM
  with HasAsyncExtInterrupts
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new RocketTopModuleImp(this)
}

class RocketTopModuleImp[+L <: RocketTop](_outer: L) extends RocketSubsystemModuleImp(_outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with HasPeripheryBootROMModuleImp
  with HasPeripheryBRAMModuleImp
  with DontTouch {
  lazy val mem_axi4 = _outer.mem_axi4
  lazy val mmio_axi4 = _outer.mmio_axi4
}
