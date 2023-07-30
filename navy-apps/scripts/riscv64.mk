CROSS_COMPILE = riscv64-linux-gnu-
LNK_ADDR = $(if $(VME), 0x40000000, 0x83000000)
CFLAGS  += -fno-pic -march=rv64g -mcmodel=medany
LDFLAGS += --no-relax -Ttext-segment $(LNK_ADDR)
ifdef VME
	LDFLAGS += --no-relax -T $(NAVY_HOME)/scripts/user.ld
else
	LDFLAGS += --no-relax -Ttext-segment $(LNK_ADDR)
endif
