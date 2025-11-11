CC ?= gcc
TEST_SRCS := $(shell find '$(NEMU_HOME)/tests' -type f -name '*.c')
TEST_OBJS = $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o) $(TEST_SRCS:%.cc=$(OBJ_DIR)/%.o)
LIBS ?= 
LIBS += -lcriterion

expr_test: $(OBJS) $(TEST_OBJS) $(ARCHIVES)
	@echo + LD $@
	@$(LD) -o $@ $(OBJS) $(TEST_OBJS) $(LDFLAGS) $(ARCHIVES) $(LIBS)

.PHONY: expr_test