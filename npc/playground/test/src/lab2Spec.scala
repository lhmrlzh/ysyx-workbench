package lab2

import chisel3._
import chisel3.experimental.BundleLiterals._
import chisel3.simulator.EphemeralSimulator._
import org.scalatest.freespec.AnyFreeSpec
import org.scalatest.matchers.must.Matchers

/** This is a trivial example of how to run this Specification From within sbt
  * use:
  * {{{
  * testOnly gcd.GCDSpec
  * }}}
  * From a terminal shell use:
  * {{{
  * sbt 'testOnly gcd.GCDSpec'
  * }}}
  * Testing from mill:
  * {{{
  * mill %NAME%.test.testOnly gcd.GCDSpec
  * }}}
  */
class Lab2Spec extends AnyFreeSpec with Matchers {
  "Encoder8t3 should convert input into bits, with an enable bit" in {
    simulate(new Encoder(3)) { dut =>
      val testValues = for { x <- 0 to 0xff } yield (x)
      val inputSeq = testValues.map { x => (x.U, 1.B) }
      val resultSeq = testValues.map { x =>
        val bit_count = Integer.bitCount(x)
        if (bit_count == 0) (0.U, 0.B)
        else if (bit_count == 1) {
          var count: Int = 0
          var nx: Int = x >> 1
          while (nx > 0) {
            nx = nx >> 1
            count = count + 1
          }
          (count.U, 1.B)
        } else (0.U, 1.B)
      }

      for ((input, result) <- inputSeq.toSeq.zip(resultSeq.toSeq)) {
        val (x, en) = input
        val (y, in) = result

        dut.io.x.poke(x)
        dut.io.en.poke(en)
        dut.io.y.expect(y)
        dut.io.in.expect(in)
      }
    }
  }
}
