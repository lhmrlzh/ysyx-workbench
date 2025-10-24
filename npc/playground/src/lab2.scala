package lab2

import chisel3._

class Encoder8t3 extends Module{
    val io = IO(new Bundle{
        val x = Input(UInt(8.W))
        val en = Input(Bool())
        val y = Output(UInt(3.W))
    })
}