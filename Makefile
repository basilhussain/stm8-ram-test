################################################################################
# BUILD OPTIONS
################################################################################

# These options can be set by overriding their values by giving them on the make
# command line. For example: "make MODEL=large RAM_END=0x17FF".

# Address of the last position in RAM. For example, for STM8S003 0x03FF; for
# STM8S208 0x17FF. Check datasheet for proper device-specific value.
RAM_END ?= 0x03FF

# Memory model of the target device ('medium' or 'large'). Devices with >32 KB
# of flash should typically use 'large'.
MODEL ?= medium

################################################################################

CC = sdcc
CFLAGS = -mstm8 -DRAM_END=$(RAM_END)
ifeq ($(MODEL),large)
	CFLAGS += --model-large
endif

AR = sdar
AFLAGS = -c

ifeq ($(OS),Windows_NT)
	RM = cmd.exe /C del /Q
	MKDIR = mkdir
else
	RM = rm -fr
	MKDIR = mkdir -p
endif

SRCHEAD = $(wildcard *.h)
SRCLIB = $(wildcard ram_test_*.c)
SRCTEST = test.c

OBJDIR = obj
OBJTEST = $(patsubst %.c,$(OBJDIR)/%.rel,$(SRCTEST))
OBJLIB = $(patsubst %.c,$(OBJDIR)/%.rel,$(SRCLIB))

LIBDIR = lib
LIBRARY = $(LIBDIR)/stm8-ram-test.lib

BINDIR = bin
BINARY = $(BINDIR)/test.ihx

.PHONY: all clean sim

all: $(BINARY)

$(BINARY): $(OBJTEST) $(LIBRARY) $(BINDIR)
	$(CC) $(CFLAGS) --out-fmt-ihx -o $@ -l $(LIBRARY) $(OBJTEST)

$(LIBRARY): $(OBJLIB) $(LIBDIR)
	$(AR) $(AFLAGS) -r $@ $(OBJLIB)

$(OBJDIR)/%.rel: %.c $(SRCHEAD) $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(LIBDIR):
	$(MKDIR) $(LIBDIR)

$(BINDIR):
	$(MKDIR) $(BINDIR)

clean:
	$(RM) $(OBJDIR)
	$(RM) $(LIBDIR)
	$(RM) $(BINDIR)

sim:
	ucsim_stm8 -t STM8S208 -X 16M -I if=rom[0x5800] $(BINARY)
