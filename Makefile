JOBS = 16
ROCKET_DIR ?= $(BASE_DIR)/rocket-chip
TOP_MODULE_PROJECT ?= zcu102
TOP_MODULE ?= RocketChip
CONFIG ?= ZCU102Config

BASE_DIR = $(abspath .)
BUILD = $(BASE_DIR)/build
SRC = $(BASE_DIR)/src
BOOTROM = $(BASE_DIR)/bootrom
IP = $(BASE_DIR)/vivado/ip/rocket
OUT_VERILOG = $(IP)/$(TOP_MODULE_PROJECT).$(CONFIG).v
ROCKETCHIP_STAMP = $(BASE_DIR)/lib/rocketchip.stamp

SHELL := /bin/bash

SBT ?= java -Xmx2G -Xss8M -jar $(ROCKET_DIR)/sbt-launch.jar

FIRRTL_JAR ?= $(ROCKET_DIR)/firrtl/utils/bin/firrtl.jar
FIRRTL ?= java -Xmx2G -Xss8M -cp $(FIRRTL_JAR) firrtl.stage.FirrtlMain

all: $(OUT_VERILOG)

$(FIRRTL_JAR): $(shell find $(ROCKET_DIR)/firrtl/src/main/scala -iname "*.scala" 2> /dev/null)
	$(MAKE) -C $(ROCKET_DIR)/firrtl SBT="$(SBT)" root_dir=$(ROCKET_DIR)/firrtl build-scala

CHISEL_ARGS := $(BUILD)

LOOKUP_SCALA_SRCS = $(shell find $(1)/. -iname "*.scala" 2> /dev/null)
BOOTROM_IMG := $(BOOTROM)/bootrom.rv64.img

$(BOOTROM_IMG): $(BOOTROM) $(shell find $(BOOTROM))
	$(MAKE) -C $(BOOTROM)

$(ROCKETCHIP_STAMP): $(call LOOKUP_SCALA_SRCS, $(ROCKET_DIR)) $(FIRRTL_JAR)
	cd $(ROCKET_DIR) && $(SBT) pack
	mkdir -p $(BASE_DIR)/lib
	cp $(ROCKET_DIR)/target/pack/lib/* $(BASE_DIR)/lib
	touch $(ROCKETCHIP_STAMP)

$(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir: $(ROCKETCHIP_STAMP) $(call LOOKUP_SCALA_SRCS,$(SRC)) $(BOOTROM_IMG)
	mkdir -p $(@D)
	$(SBT) "runMain freechips.rocketchip.system.Generator $(CHISEL_ARGS) $(TOP_MODULE_PROJECT) $(TOP_MODULE) $(TOP_MODULE_PROJECT) $(CONFIG)"

$(OUT_VERILOG): $(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir $(FIRRTL_JAR)
	$(FIRRTL) -i $< -o $@ -X verilog

clean:
	$(MAKE) -C $(BOOTROM) clean
	rm -rf build/*
	rm $(OUT_VERILOG) -f

.PHONY:  all clean
