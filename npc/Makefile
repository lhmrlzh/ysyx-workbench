all:
	verilator --cc --exe --build -j 0 -Wall --trace-fst --Mdir build csrc/main.cpp vsrc/top.v

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	build/Vtop

clean:
	rm -r build

include ../Makefile
