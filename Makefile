JOBS = 20
ROCKET_DIR ?= $(BASE_DIR)/rocket-chip
TESTCHIPIP_DIR = $(BASE_DIR)/testchipip
PROJECT ?= zynqmp
TOP_MODULE ?= TVMEvaluator
CONFIG ?= ZCU102NVDLAConfig

BASE_DIR = $(abspath .)
BUILD = $(BASE_DIR)/build
BOOTROM = $(BASE_DIR)/bootrom
OUT_VERILOG = $(BUILD)/$(PROJECT).$(CONFIG).v

SHELL := /bin/bash

export

JAVA ?= java
#SBT ?= $(JAVA) -Xmx2G -Xss8M -jar $(ROCKET_DIR)/sbt-launch.jar
SBT ?= sbt
export SBT_OPTS = -Xmx2G -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -XX:MaxPermSize=2G -Xss2M  -Duser.timezone=GMT

.PHONY: all
all: verilog verilator

BOOTROM_IMG := $(BOOTROM)/bootrom.rv64.img

.PHONY: $(BOOTROM_IMG)
$(BOOTROM_IMG):
	$(MAKE) -C $(BOOTROM)

$(BUILD)/$(PROJECT).$(CONFIG).fir:
	mkdir -p $(@D)
	$(SBT) "runMain freechips.rocketchip.system.Generator $(BUILD) $(PROJECT) $(TOP_MODULE) $(PROJECT) $(CONFIG)"

.PHONY: verilog
verilog: $(OUT_VERILOG)

.PHONY: verilator
verilator:
	$(MAKE) -C verilator/

%.v: %.fir
	$(SBT) "runMain firrtl.stage.FirrtlMain -i $< -o $@ -X verilog"

clean:
	$(MAKE) -C $(BOOTROM) clean
	rm -rf build/*

.PHONY:  all clean

bitstream:
	cd vivado
	vivado -mode tcl -nolog -nojournal -source src/create_bitstream.tcl
