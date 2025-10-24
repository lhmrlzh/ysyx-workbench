package lab2

import chisel3._
import chisel3.util.PriorityEncoder
import java.awt.MouseInfo

class Encoder8t3 extends Module {
  val io = IO(new Bundle {
    val x = Input(UInt(8.W))
    val en = Input(Bool())
    val y = Output(UInt(3.W))
    val in = Output(Bool())
  })

  io.y := Mux(io.en, PriorityEncoder(io.x), 0.U(3.W))
  io.in := io.en && io.x.orR
}

class Decoder7Seg extends Module {
  val io = IO(new Bundle {
    val x = Input(UInt(3.W))
    val in = Input(Bool())
    val y = Output(UInt(7.W))
  })

  var nums = VecInit(
    Seq(
      "b0111111".U(7.W), // 0
      "b0000110".U(7.W), // 1
      "b1011011".U(7.W), // 2
      "b1001111".U(7.W), // 3
      "b1100110".U(7.W), // 4
      "b1101101".U(7.W), // 5
      "b1111101".U(7.W), // 6
      "b0000111".U(7.W) // 7
    )
  )
  var invalid = "b0000000".U(7.W)

  io.y := Mux(io.in, nums(io.x), invalid)
}
