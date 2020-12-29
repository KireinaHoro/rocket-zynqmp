JOBS = 20
ROCKET_DIR ?= $(BASE_DIR)/rocket-chip
TESTCHIPIP_DIR = $(BASE_DIR)/testchipip
PROJECT ?= zynqmp
TOP_MODULE ?= RfidTop
CONFIG ?= UhfRfidConfig

BASE_DIR = $(abspath .)
BUILD = $(BASE_DIR)/build-060
BOOTROM = $(BASE_DIR)/bootrom
OUT_VERILOG = $(BUILD)/$(PROJECT).$(CONFIG).v

SHELL := /bin/bash

export

MILL ?= mill -j 0

.PHONY: all
all: verilog verilator

BOOTROM_IMG := $(BOOTROM)/bootrom.rv64.img

.PHONY: $(BOOTROM_IMG)
$(BOOTROM_IMG):
	$(MAKE) -C $(BOOTROM)

$(BUILD)/$(PROJECT).$(CONFIG).fir:
	mkdir -p $(@D)
	$(MILL) system.genFirrtl $(TOP_MODULE) $(CONFIG)

.PHONY: verilog
verilog: $(OUT_VERILOG)

.PHONY: verilator
verilator:
	$(MAKE) -C verilator/

%.v: %.fir
	$(MILL) system.genVerilog $(TOP_MODULE) $(CONFIG)

clean:
	rm -rf build-060/*

.PHONY:  all clean

bitstream:
	cd vivado
	vivado -mode tcl -nolog -nojournal -source src/create_bitstream.tcl
