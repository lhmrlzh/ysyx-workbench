#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../build/Vtop.h"
#include "verilated.h"
#include "verilated_fst_c.h"

int main(int argc, char** argv) {
  VerilatedContext* contextp = new VerilatedContext;
  contextp->traceEverOn(true);
  contextp->commandArgs(argc, argv);
  Vtop* top = new Vtop{ contextp };

  VerilatedFstC* tfp = new VerilatedFstC;
  top->trace(tfp, 99);
  tfp->open("out/wave.fst");

  int step = 100, i = 0;
  while (!contextp->gotFinish() && i++ < step) {
    int a = rand() & 1;
    int b = rand() & 1;
    top->a = a;
    top->b = b;
    top->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);
    printf("a = %d, b = %d, f = %d\n", a, b, top->f);
    assert(top->f == (a ^ b));
  }

  tfp->close();
  delete tfp;
  delete top;
  delete contextp;
  return 0;
}