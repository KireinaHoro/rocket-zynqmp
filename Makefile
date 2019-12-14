JOBS = 16
ROCKET_DIR ?= $(BASE_DIR)/rocket-chip
TOP_MODULE_PROJECT ?= zcu102
TOP_MODULE ?= RocketChip
CONFIG ?= ZCU102Config

BASE_DIR = $(abspath .)
BUILD = $(BASE_DIR)/build
SRC = $(BASE_DIR)/src
ROCKETCHIP_STAMP = $(BASE_DIR)/lib/rocketchip.stamp

SHELL := /bin/bash

SBT ?= java -Xmx2G -Xss8M -jar $(ROCKET_DIR)/sbt-launch.jar

FIRRTL_JAR ?= $(ROCKET_DIR)/firrtl/utils/bin/firrtl.jar
FIRRTL ?= java -Xmx2G -Xss8M -cp $(FIRRTL_JAR) firrtl.stage.FirrtlMain

all: $(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).v

$(FIRRTL_JAR): $(shell find $(ROCKET_DIR)/firrtl/src/main/scala -iname "*.scala" 2> /dev/null)
	$(MAKE) -C $(ROCKET_DIR)/firrtl SBT="$(SBT)" root_dir=$(ROCKET_DIR)/firrtl build-scala

CHISEL_ARGS := $(BUILD)

LOOKUP_SCALA_SRCS = $(shell find $(1)/. -iname "*.scala" 2> /dev/null)
BOOTROM := $(shell find bootrom -iname "*.img" 2> /dev/null)

$(ROCKETCHIP_STAMP): $(call LOOKUP_SCALA_SRCS, $(ROCKET_DIR)) $(FIRRTL_JAR)
	cd $(ROCKET_DIR) && $(SBT) pack
	mkdir -p $(BASE_DIR)/lib
	cp $(ROCKET_DIR)/target/pack/lib/* $(BASE_DIR)/lib
	touch $(ROCKETCHIP_STAMP)

$(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir: $(ROCKETCHIP_STAMP) $(call LOOKUP_SCALA_SRCS,$(SRC)) $(BOOTROM)
	mkdir -p $(@D)
	$(SBT) "runMain freechips.rocketchip.system.Generator $(CHISEL_ARGS) $(TOP_MODULE_PROJECT) $(TOP_MODULE) $(TOP_MODULE_PROJECT) $(CONFIG)"

$(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).v: $(BUILD)/$(TOP_MODULE_PROJECT).$(CONFIG).fir $(FIRRTL_JAR)
	$(FIRRTL) -i $< -o $@ -X verilog
	cp $@ $@.bak
	sed 's/wire \[63:0\] coreMonitorBundle/(* mark_debug="true" *) \0/g' $@.bak > $@

clean:
	rm -rf build/*

.PHONY:  all clean
