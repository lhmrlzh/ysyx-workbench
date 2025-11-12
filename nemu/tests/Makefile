CC ?= gcc
TEST_SRCS := $(shell find -L "tests" -name "*.c")
TEST_CXXSRCS := $(shell find -L "tests" -name "*.cc")
TEST_CXXSRCS += $(shell find -L "tests" -name "*.cpp")

EXCLUDE := $(OBJ_DIR)/src/monitor/sdb/expr.o \
		   $(OBJ_DIR)/src/nemu-main.o
TEST_OBJS = $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o) $(TEST_CXXSRCS:%.cc=$(OBJ_DIR)/%.o)
OBJS_FOR_TEST := $(filter-out $(EXCLUDE), $(OBJS))
LIBS ?= 
LIBS += -lcriterion
CFLAGS += -g -O0

expr_test: $(OBJS_FOR_TEST) $(TEST_OBJS) $(ARCHIVES)
	@echo + LD $@
	@$(LD) -o $@ $(OBJS_FOR_TEST) $(TEST_OBJS) $(LDFLAGS) $(ARCHIVES) $(LIBS)

.PHONY: expr_test