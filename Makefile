JOBS = 16
ROCKET_DIR ?= $(BASE_DIR)/rocket-chip
TOP_MODULE_PROJECT ?= zynqmp
TOP_MODULE ?= RocketChip
CONFIG ?= EdgeBoardConfig

BASE_DIR = $(abspath .)
BUILD = $(BASE_DIR)/build
BOOTROM = $(BASE_DIR)/bootrom
IP = $(BASE_DIR)/vivado/ip/rocket
OUT_VERILOG = $(IP)/$(TOP_MODULE_PROJECT).$(CONFIG).v

SHELL := /bin/bash

JAVA ?= java
SBT ?= $(JAVA) -Xmx2G -Xss8M -jar $(ROCKET_DIR)/sbt-launch.jar

ROCKET ?= $(SBT) runMain freechips.rocketchip.system.Generator
FIRRTL ?= $(SBT) runMain firrtl.stage.FirrtlMain

all: $(OUT_VERILOG)

BOOTROM_IMG := $(BOOTROM)/bootrom.rv64.img

.PHONY: $(BOOTROM_IMG)
$(BOOTROM_IMG):
	$(MAKE) -C $(BOOTROM)

$(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir: $(BOOTROM_IMG)
	mkdir -p $(@D)
	$(ROCKET) $(BUILD) $(TOP_MODULE_PROJECT) $(TOP_MODULE) $(TOP_MODULE_PROJECT) $(CONFIG)

$(OUT_VERILOG): $(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir
	$(FIRRTL) -i $< -o $@ -X verilog

clean:
	$(MAKE) -C $(BOOTROM) clean
	rm -rf build/*
	rm $(OUT_VERILOG) -f

.PHONY:  all clean
