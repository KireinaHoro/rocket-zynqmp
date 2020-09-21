package zynqmp

import chisel3._
import chisel3.experimental.{ExtModule, chiselName}
import chisel3.util._
import freechips.rocketchip.devices.debug.SystemJTAGIO

class BUFGCE extends ExtModule {
  val O = IO(Output(Bool()))
  val CE = IO(Input(Bool()))
  val I = IO(Input(Bool()))
}

class BSCANE2 extends ExtModule(Map("JTAG_CHAIN" -> 4)) {
  val TDO = IO(Input(Bool()))
  val CAPTURE = IO(Output(Bool()))
  val DRCK = IO(Output(Bool()))
  val RESET = IO(Output(Bool()))
  val RUNTEST = IO(Output(Bool()))
  val SEL = IO(Output(Bool()))
  val SHIFT = IO(Output(Bool()))
  val TCK = IO(Output(Bool()))
  val TDI = IO(Output(Bool()))
  val TMS = IO(Output(Bool()))
  val UPDATE = IO(Output(Bool()))
}

object BscanJTAG {
  def connectJTAG(dutJtag: SystemJTAGIO, chainPosition: Int = 0)(implicit reset: Reset) = {
    val bscan = Module(new BscanJTAG(chainPosition))
    dutJtag.jtag.TCK := bscan.tck
    dutJtag.jtag.TDI := bscan.tdi
    dutJtag.jtag.TMS := bscan.tms
    dutJtag.reset := reset
    dutJtag.mfr_id := 0x489.U(11.W)
    dutJtag.part_number := 0.U(16.W)
    dutJtag.version := 2.U(4.W)

    bscan.tdo := dutJtag.jtag.TDO.data
    bscan.tdoEnable := dutJtag.jtag.TDO.driven

    bscan
  }
}

@chiselName
class BscanJTAG(chainPosition: Int = 0) extends MultiIOModule {
  val tck: Clock = IO(Output(Clock()))
  val tms: Bool = IO(Output(Bool()))
  val tdi: Bool = IO(Output(Bool()))
  val tdo: Bool = IO(Input(Bool()))
  val tdoEnable: Bool = IO(Input(Bool()))

  val bscane2 = Module(new BSCANE2)
  tdi := bscane2.TDI
  bscane2.TDO := Mux(tdoEnable, tdo, true.B)
  val bufgce = Module(new BUFGCE)
  bufgce.I := bscane2.TCK
  bufgce.CE := bscane2.SEL
  tck := bufgce.O.asClock

  val posClock: Clock = bscane2.TCK.asClock
  val negClock: Clock = (!bscane2.TCK).asClock
  /**
   * This two wire will cross two clock domain,
   * generated at [[posClock]], used in [[negClock]]
   **/
  val tdiRegisterWire = Wire(Bool())
  val payloadWidthWire = Wire(UInt(7.W))
  val chainPos = chainPosition.asUInt(8.W)
  withReset(!bscane2.SHIFT) {
    withClock(posClock) {
      // match tunnel payload (LSB)
      // 00 1010000 xxxxx 000 (IR, 5 bit payload)
      // 01 wwwwwww xxxx...xx 000 (DR, 0bwwwwwww bit payload)
      val posCounter = RegInit(0.U(8.W)) // cycle counter
      val payloadWidthCounter = RegInit(0.U(7.W))
      val irOrDr = RegInit(false.B)
      posCounter := posCounter + 1.U
      // capture IR or DR
      when(posCounter === chainPos) {
        irOrDr := !bscane2.TDI
      }
      // capture payload width
      when(posCounter >= 1.U + chainPos && posCounter <= 7.U + chainPos) {
        payloadWidthCounter := Cat(bscane2.TDI, payloadWidthCounter.head(6))
      }
      tdiRegisterWire := irOrDr
      payloadWidthWire := payloadWidthCounter
    }
    withClock(negClock) {
      val negCounter = RegInit(0.U(8.W))
      negCounter := negCounter + 1.U
      tms := MuxLookup(negCounter, false.B, Array(
        4.U + chainPos -> tdiRegisterWire,
        5.U + chainPos -> true.B,
        payloadWidthWire + 7.U + chainPos -> true.B,
        payloadWidthWire + 8.U + chainPos -> true.B)
      )
    }
  }
}
