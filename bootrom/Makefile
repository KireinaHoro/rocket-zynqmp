bootrom_img = bootrom.rv64.img
bootrom_dump = bootrom.rv64.dump

GCC=riscv64-unknown-elf-gcc
CFLAGS_RV64=-mabi=lp64 -march=rv64ima -nostdlib -ffreestanding -DDEBUG
OBJCOPY=riscv64-unknown-elf-objcopy
OBJDUMP=riscv64-unknown-elf-objdump

.PHONY: all clean

all: img

img: $(bootrom_img)

dump: $(bootrom_dump)

driver_subdir := driver
driver_sources := $(wildcard $(driver_subdir)/*.c)
driver_include := -I$(driver_subdir)
driver_objects := $(driver_sources:c=o)

$(driver_objects): %.o: %.c
	$(GCC) $(CFLAGS_RV64) -c $< -o $@

%.img: %.elf
	$(OBJCOPY) -O binary --change-addresses=-0x10000 $< $@

%.rv64.elf: %.S bootloader.c linker.ld $(driver_objects)
	$(GCC) $(CFLAGS_RV64) $(driver_include) -Tlinker.ld $^ -static -o $@

%.dump: %.elf
	$(OBJDUMP) -D $< > $@

clean:
	rm -f $(bootrom_img) $(bootrom_dump) *.elf $(driver_objects)
