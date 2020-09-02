RISCV ?= /opt/riscv

VERILATOR_BUILD := $(BUILD)/verilator
MODEL ?= TestHarness
VLSI_MEM_GEN := $(ROCKET_DIR)/scripts/vlsi_mem_gen

emu = $(VERILATOR_BUILD)/emulator-$(PROJECT)-$(CONFIG)
emu_debug = $(emu)-debug

.PHONY: all
all: $(emu) $(emu_debug)

verilog := \
	$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).v \
	$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).behav_srams.v

rocket_cxxsrc := SimDTM SimJTAG remote_bitbang emulator
testchipip_cxxsrc := SimUART uart

LDFLAGS := $(LDFLAGS) -L$(RISCV)/lib -Wl,-rpath,$(RISCV)/lib -L$(abspath $(VERILATOR_BUILD)) -lfesvr -lpthread
CXXFLAGS := $(CXXFLAGS) -std=c++11 -I$(RISCV)/include

rocket_csrc = $(ROCKET_DIR)/src/main/resources/csrc
rocket_vsrc = $(ROCKET_DIR)/src/main/resources/vsrc
testchipip_csrc = $(TESTCHIPIP_DIR)/src/main/resources/testchipip/csrc
testchipip_vsrc = $(TESTCHIPIP_DIR)/src/main/resources/testchipip/vsrc

VERILATOR_H = $(rocket_csrc)/verilator.h

# Build and install our own Verilator, to work around versionining issues.
VERILATOR_VERSION=4.008
VERILATOR_SRCDIR ?= verilator/src/verilator-$(VERILATOR_VERSION)
VERILATOR_TARGET := $(abspath verilator/install/bin/verilator_bin)
INSTALLED_VERILATOR ?= $(VERILATOR_TARGET)
$(VERILATOR_TARGET): $(VERILATOR_SRCDIR)/bin/verilator_bin
	$(MAKE) -C $(VERILATOR_SRCDIR) installbin installdata
	touch $@

$(VERILATOR_SRCDIR)/bin/verilator_bin: $(VERILATOR_SRCDIR)/Makefile
	$(MAKE) -C $(VERILATOR_SRCDIR) verilator_bin
	touch $@

$(VERILATOR_SRCDIR)/Makefile: $(VERILATOR_SRCDIR)/configure
	mkdir -p $(dir $@)
	cd $(dir $@) && ./configure --prefix=$(abspath verilator/install)

$(VERILATOR_SRCDIR)/configure: verilator/verilator-$(VERILATOR_VERSION).tar.gz
	rm -rf $(dir $@)
	mkdir -p $(dir $@)
	cat $^ | tar -xz --strip-components=1 -C $(dir $@)
	touch $@

verilator/verilator-$(VERILATOR_VERSION).tar.gz:
	mkdir -p $(dir $@)
	wget http://www.veripool.org/ftp/verilator-$(VERILATOR_VERSION).tgz -O $@

verilator: $(INSTALLED_VERILATOR)

# Run Verilator to produce a fast binary to emulate this circuit.
VERILATOR := $(INSTALLED_VERILATOR) --cc --exe
VERILATOR_THREADS ?= 32
VERILATOR_FLAGS := --top-module $(MODEL) \
	+define+PRINTF_COND=\$$c\(\"verbose\",\"\&\&\"\,\"done_reset\"\) \
	+define+RANDOMIZE_REG_INIT \
	+define+RANDOMIZE_GARBAGE_ASSIGN \
	+define+RANDOMIZE_INVALID_ASSIGN \
	+define+PRINT_COND=\$$c\(\"done_reset\"\) \
	+define+STOP_COND=\$$c\(\"done_reset\"\) \
	--assert \
	--output-split 20000 \
	--output-split-cfuncs 20000 \
	--threads $(VERILATOR_THREADS) -Wno-UNOPTTHREADS \
	-Wno-STMTDLY --x-assign unique \
	-I$(rocket_vsrc) -I$(testchipip_vsrc) \
	-O3 -CFLAGS "$(CXXFLAGS) -DVERILATOR -DTEST_HARNESS=V$(MODEL) \
	-include $(VERILATOR_H) \
	-include $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).plusArgs"

cppfiles = $(addprefix $(rocket_csrc)/, $(addsuffix .cc, $(rocket_cxxsrc))) \
	$(addprefix $(testchipip_csrc)/, $(addsuffix .cc, $(testchipip_cxxsrc)))
headers = $(wildcard $(rocket_csrc)/*.h) $(wildcard $(testchipip_csrc)/*.h)

model_header = $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG)/V$(MODEL).h
model_header_debug = $(VERILATOR_BUILD)/debug/$(PROJECT).$(CONFIG)/V$(MODEL).h

$(emu): $(verilog) $(cppfiles) $(headers) $(INSTALLED_VERILATOR)
	mkdir -p $(dir $(model_header))
	$(VERILATOR) $(VERILATOR_FLAGS) -Mdir $(dir $(model_header)) \
	-o $@ $(verilog) $(cppfiles) -LDFLAGS "$(LDFLAGS)" \
	-CFLAGS "-I$(VERILATOR_BUILD) -include $(model_header)"
	$(MAKE) -j$(shell nproc) VM_PARALLEL_BUILDS=1 -C $(dir $(model_header)) -f V$(MODEL).mk

$(emu_debug): $(verilog) $(cppfiles) $(headers) $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).d $(INSTALLED_VERILATOR)
	mkdir -p $(dir $(model_header_debug))
	$(VERILATOR) $(VERILATOR_FLAGS) -Mdir $(dir $(model_header_debug))  --trace \
	-o $@ $(verilog) $(cppfiles) -LDFLAGS "$(LDFLAGS)" \
	-CFLAGS "-I$(VERILATOR_BUILD)/debug -include $(model_header_debug)"
	$(MAKE) -j$(shell nproc) VM_PARALLEL_BUILDS=1 -C $(dir $(model_header_debug)) -f V$(MODEL).mk

$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).fir $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).d:
	mkdir -p $(@D)
	cd .. && \
	$(SBT) "runMain freechips.rocketchip.system.Generator $(VERILATOR_BUILD) $(PROJECT) $(MODEL) $(PROJECT) $(CONFIG)"

$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).v $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).conf : $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).fir
	cd .. && \
	$(SBT) "runMain firrtl.stage.FirrtlMain \
	-i $< -o $@ -X verilog \
	--infer-rw $(MODEL) \
	--repl-seq-mem -c:$(MODEL):-o:$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).conf \
	-faf $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).anno.json \
	-td $(VERILATOR_BUILD) \
	"

$(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).behav_srams.v : $(VERILATOR_BUILD)/$(PROJECT).$(CONFIG).conf $(VLSI_MEM_GEN)
	cd $(VERILATOR_BUILD) && \
	$(VLSI_MEM_GEN) $< > $@.tmp && \
	mv -f $@.tmp $@


