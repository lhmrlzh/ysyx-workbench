BUILD_DIR = ./build

MILL = ./mill
PRJ = playground

test:
	MILL -i $(PRJ).test

verilog:
	$(call git_commit, "generate verilog")
	mkdir -p $(BUILD_DIR)
	MILL -i $(PRJ).runMain Elaborate --target-dir $(BUILD_DIR)

help:
	MILL -i $(PRJ).runMain Elaborate --help

reformat:
	MILL -i __.reformat

checkformat:
	MILL -i __.checkFormat

bsp:
	MILL -i MILL.bsp.BSP/install

idea:
	MILL -i MILL.idea.GenIdea/idea

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: test verilog help reformat checkformat clean

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."

-include ../Makefile
