package zynqmp

import chisel3._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy.LazyModule
import freechips.rocketchip.subsystem._
import freechips.rocketchip.util._
import sifive.blocks.devices.pwm.{HasPeripheryPWM, HasPeripheryPWMModuleImp}
import sifive.blocks.devices.uart.{HasPeripheryUART, HasPeripheryUARTModuleImp}

class EdgeBoardTop(implicit p: Parameters) extends BoardTop(EdgeBoardParams)
class ZCU102Top(implicit p: Parameters) extends BoardTop(ZCU102Params)

class BoardTop(params: Params)(implicit val p: Parameters) extends Module {
  val target = Module(LazyModule(new RocketTop).module)

  require(target.mem_axi4.size == 1)
  require(target.mmio_axi4.size == 1)
  require(target.debug.head.systemjtag.size == 1)

  val memBundle = target.mem_axi4.head
  val mmioBundle = target.mmio_axi4.head
  val jtagBundle = target.debug.head.systemjtag.head
  val uartBundle = target.uart
  val pwmBundle = target.pwm

  val io = IO(new Bundle {
    val interrupts = Input(UInt(params.NInterrupts.W))
    val mem_axi4 = memBundle.cloneType
    val mmio_axi4 = mmioBundle.cloneType
    val jtag = Flipped(jtagBundle.jtag.cloneType)
    val uart0 = uartBundle.head.cloneType
    val uart1 = uartBundle.head.cloneType
    val pwm0 = pwmBundle.head.cloneType
    val pwm1 = pwmBundle.head.cloneType
  })

  io.uart0 <> uartBundle(0)
  io.uart1 <> uartBundle(1)

  io.pwm0 <> pwmBundle(0)
  io.pwm1 <> pwmBundle(1)

  // AXI ports
  io.mem_axi4 <> memBundle
  io.mmio_axi4 <> mmioBundle

  // JTAG
  io.jtag <> jtagBundle.jtag
  jtagBundle.reset := target.reset
  jtagBundle.mfr_id := 0x489.U(11.W)
  jtagBundle.part_number := 0.U(16.W)
  jtagBundle.version := 2.U(4.W)

  // interrupts
  target.interrupts := io.interrupts

  target.dontTouchPorts
}

class RocketTop(implicit p: Parameters) extends RocketSubsystem
  with HasHierarchicalBusTopology
  with HasAsyncExtInterrupts
  with HasPeripheryUART
  with HasPeripheryPWM
  with CanHaveMasterAXI4MemPort
  with CanHaveMasterAXI4MMIOPort {
  override lazy val module = new RocketTopModuleImp(this)

  def resetVector = p(BootROMParams).hang
}

class RocketTopModuleImp[+L <: RocketTop](outer: L) extends RocketSubsystemModuleImp(outer)
  with HasRTCModuleImp
  with HasExtInterruptsModuleImp
  with HasPeripheryUARTModuleImp
  with HasPeripheryPWMModuleImp
  with HasResetVectorWire
  with DontTouch {
  lazy val mem_axi4 = outer.mem_axi4
  lazy val mmio_axi4 = outer.mmio_axi4

  println(f"global reset vector at 0x${outer.resetVector}%x")
  global_reset_vector := outer.resetVector.U
}
