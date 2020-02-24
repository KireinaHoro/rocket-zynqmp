bootrom_elf = bootrom.rv64.elf
bootrom_img = bootrom.rv64.img
bootrom_dump = bootrom.rv64.dump

driver_subdir	:= driver
driver_sources	:= $(wildcard $(driver_subdir)/*.c)
driver_include	:= -I$(driver_subdir)
driver_objects	:= $(driver_sources:c=o)
driver_library	:= driver.a

sbi_platform	:= edgeboard
sbi_root		:= ../opensbi
sbi_fw_dir		:= $(sbi_root)/build/platform/$(sbi_platform)/firmware
sbi_elf			:= fw_payload.elf
sbi_elf_strip	:= fw_payload.elf.strip
sbi_coe = $(sbi_elf_strip).coe

HOST_CXX=g++
TRIPLE=riscv64-linux-gnu-
GCC=$(TRIPLE)gcc
CFLAGS_RV64=-g -Wall -Werror -nostdlib -fno-strict-aliasing \
	-Os -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	-mno-save-restore -mstrict-align -mabi=lp64 -march=rv64imafdc \
	-mcmodel=medany -ffreestanding -Wl,--build-id=none
CFLAGS=$(CFLAGS_RV64) -DDEBUG
OBJCOPY=$(TRIPLE)objcopy
OBJDUMP=$(TRIPLE)objdump
STRIP=$(TRIPLE)strip
COEGEN=./coegen

.PHONY: all clean

all: $(bootrom_img) $(bootrom_elf) $(sbi_coe)

$(driver_objects): %.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@

%.img: %.elf
	$(OBJCOPY) -O binary $< $@

$(sbi_coe): $(sbi_elf_strip) $(COEGEN)
	$(COEGEN) --file $< --width 32 --depth $$(($$(stat -L -c %s $<) / 4))

$(COEGEN): coegen.cpp
	$(HOST_CXX) -o $@ $< -lboost_program_options

$(driver_library): $(driver_objects)
	ar rcs $@ $^

.PHONY: $(sbi_elf_strip)
$(sbi_elf_strip):
	$(MAKE) -C $(sbi_root) CROSS_COMPILE=$(TRIPLE) PLATFORM=$(sbi_platform)
	cp $(sbi_fw_dir)/$(sbi_elf) .
	$(STRIP) $(sbi_elf) -o $@

%.rv64.elf: %.S bootloader.c linker.ld $(driver_library)
	$(GCC) $(CFLAGS) $(driver_include) -Tlinker.ld bootrom.S bootloader.c $(driver_library) -static -o $@

clean:
	$(MAKE) -C $(sbi_root) clean
	rm -f $(bootrom_img) $(bootrom_dump) *.elf $(sbi_elf_strip) $(driver_objects) $(driver_library) $(sbi_coe) $(COEGEN)
