package zynqmp

import zynqmp._
import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.debug.Debug
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._
import sifive.blocks.devices.pwm.{HasPeripheryPWM, HasPeripheryPWMModuleImp}
import sifive.blocks.devices.uart.{HasPeripheryUART, HasPeripheryUARTModuleImp}

class ZCU102Top(implicit val p: Parameters) extends Module {
  val ldut = LazyModule(new RocketTop)
  val target = Module(ldut.module)

  require(ldut.mem_axi4.size == 1)
  require(ldut.mmio_axi4.size == 1)
  require(target.debug.head.systemjtag.size == 1)

  val mem = ldut.mem_axi4.head
  val mmio = ldut.mmio_axi4.head
  val dutJtag = target.debug.get.systemjtag.get
  val uart = target.uart
  val pwm = target.pwm

  val io = IO(new Bundle {
    val interrupts = Input(UInt(p(NExtTopInterrupts).W))
    val mem_axi4 = mem.cloneType
    val mmio_axi4 = mmio.cloneType
    val jtag = Flipped(dutJtag.jtag.cloneType)
    val uart0 = uart.head.cloneType
    val uart1 = uart.head.cloneType
    val pwm0 = pwm.head.cloneType
    val pwm1 = pwm.head.cloneType
  })

  io.uart0 <> uart(0)
  io.uart1 <> uart(1)

  io.pwm0 <> pwm(0)
  io.pwm1 <> pwm(1)

  // AXI ports
  io.mem_axi4 <> mem
  io.mmio_axi4 <> mmio

  // JTAG
  Debug.connectDebugClockAndReset(target.debug, clock)
  target.resetctrl map { _.hartIsInReset.map(_ := reset) }
  io.jtag <> dutJtag.jtag
  dutJtag.reset := target.reset
  dutJtag.mfr_id := 0x489.U(11.W)
  dutJtag.part_number := 0.U(16.W)
  dutJtag.version := 2.U(4.W)

  // interrupts
  target.interrupts := io.interrupts

  target.dontTouchPorts
}

class RocketTop(implicit p: Parameters) extends RocketSubsystem
  with HasAsyncExtInterrupts
  with HasPeripheryUART
  with HasPeripheryPWM
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new RocketTopModuleImp(this)

  setResetVector(this)
}

class RocketTopModuleImp[+L <: RocketTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with HasPeripheryUARTModuleImp
  with HasPeripheryPWMModuleImp
  with DontTouch
