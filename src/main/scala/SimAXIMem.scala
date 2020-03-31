package zynqmp

import chisel3._
import freechips.rocketchip.amba.axi4._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem.{CanHaveMasterAXI4MMIOPort, CanHaveMasterAXI4MemPort, ExtMem}

/** Memory with AXI port for use in elaboratable test harnesses. */
class SimAXIMem(edge: AXI4EdgeParameters, base: BigInt, size: BigInt)(implicit p: Parameters) extends SimpleLazyModule {
  val node = AXI4MasterNode(List(edge.master))
  val srams = AddressSet.misaligned(base, size).map { aSet => LazyModule(new AXI4RAM(aSet, beatBytes = edge.bundle.dataBits / 8)) }
  val xbar = AXI4Xbar()
  srams.foreach { s => s.node := AXI4Buffer() := AXI4Fragmenter() := xbar }
  xbar := node
  val io_axi4 = InModuleBody {
    node.makeIOs()
  }
}

object SimAXIMem {
  def connectMem(dut: CanHaveMasterAXI4MemPort)(implicit p: Parameters): Seq[SimAXIMem] = {
    dut.mem_axi4.zip(dut.memAXI4Node.in).map { case (io, (_, edge)) =>
      val memMaster = p(ExtMem).get.master
      val mem = LazyModule(new SimAXIMem(edge, base = memMaster.base,
        size = memMaster.size))
      Module(mem.module).suggestName("mem")
      mem.io_axi4.head <> io
      mem
    }
  }
}
