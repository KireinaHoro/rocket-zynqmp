package zynqmp

import zynqmp._
import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.debug.Debug
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.jtag.JTAGIO
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._

class LabXTop(implicit val p: Parameters) extends Module {
  val ldut = LazyModule(new RocketTop)
  val target = Module(ldut.module)

  require(ldut.mem_axi4.size == 1)
  require(ldut.mmio_axi4.size == 1)
  require(ldut.l2_frontend_bus_axi4.size == 1)
  require(target.debug.head.systemjtag.size == 1)

  val mem = ldut.mem_axi4.head
  val mmio = ldut.mmio_axi4.head
  val slave = ldut.l2_frontend_bus_axi4.head
  val dutJtag = target.debug.get.systemjtag.get

  val io = IO(new Bundle {
    val interrupts = Input(UInt(p(NExtTopInterrupts).W))
    val mem_axi4 = mem.cloneType
    val mmio_axi4 = mmio.cloneType
    val slave_axi4 = Flipped(slave.cloneType)
    val jtag = Flipped(new JTAGIO(hasTRSTn = false))
  })

  // AXI ports
  io.mem_axi4 <> mem
  io.mmio_axi4 <> mmio
  slave <> io.slave_axi4

  // JTAG
  Debug.connectDebugClockAndReset(target.debug, clock)
  target.resetctrl map { _.hartIsInReset.map(_ := reset) }
  connectJTAG(dutJtag, io.jtag)(reset)

  // interrupts
  target.interrupts := io.interrupts

  target.dontTouchPorts
}

class RocketTop(implicit p: Parameters) extends RocketSubsystem
  with HasAsyncExtInterrupts
  with CanHaveSlaveAXI4Port
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new RocketTopModuleImp(this)

  setResetVector(this)
}

class RocketTopModuleImp[+L <: RocketTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with DontTouch
