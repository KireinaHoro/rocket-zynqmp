package zcu102

import chisel3._

import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._

class RocketChip(implicit val p: Parameters) extends Module {
  val config = p(ExtIn)
  val target = Module(LazyModule(new RocketTop).module)

  require(target.mem_axi4.size == 1)
  require(target.mmio_axi4.size == 1)
  require(target.debug.head.systemjtag.size == 1)

  val memBundle = target.mem_axi4.head
  val mmioBundle = target.mmio_axi4.head
  val jtagBundle = target.debug.head.systemjtag.head

  val io = IO(new Bundle {
    val interrupts = Input(UInt(7.W))
    val mem_axi4 = memBundle.cloneType
    val mmio_axi4 = mmioBundle.cloneType
  })

  io.mem_axi4 <> memBundle
  io.mmio_axi4 <> mmioBundle

  // connect JTAG
  val boardJtag = Module(new BscanJTAG(1))
  // set JTAG parameters
  jtagBundle.reset := reset
  jtagBundle.mfr_id := 0x489.U(11.W)
  jtagBundle.part_number := 0.U(16.W)
  jtagBundle.version := 2.U(4.W)
  // connect to BSCAN
  jtagBundle.jtag.TCK := boardJtag.tck
  jtagBundle.jtag.TMS := boardJtag.tms
  jtagBundle.jtag.TDI := boardJtag.tdi
  boardJtag.tdo := jtagBundle.jtag.TDO.data
  boardJtag.tdoEnable := jtagBundle.jtag.TDO.driven

  target.interrupts := io.interrupts

  target.dontTouchPorts
}

class RocketTop(implicit p: Parameters) extends RocketSubsystem
  with HasHierarchicalBusTopology
  with HasPeripheryBootROM
  with HasAsyncExtInterrupts
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new RocketTopModuleImp(this)
}

class RocketTopModuleImp[+L <: RocketTop](_outer: L) extends RocketSubsystemModuleImp(_outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with HasPeripheryBootROMModuleImp
  with DontTouch {
  lazy val mem_axi4 = _outer.mem_axi4
  lazy val mmio_axi4 = _outer.mmio_axi4
}
