bootrom_elf = bootrom.rv64.elf
bootrom_img = bootrom.rv64.img
bootrom_dump = bootrom.rv64.dump

TRIPLE=riscv64-linux-gnu-
GCC=$(TRIPLE)gcc -g
CFLAGS_RV64=-mabi=lp64 -march=rv64ima -nostdlib -ffreestanding -DDEBUG -mcmodel=medany -Wl,--build-id=none
OBJCOPY=$(TRIPLE)objcopy
OBJDUMP=$(TRIPLE)objdump

.PHONY: all clean

all: $(bootrom_img) $(bootrom_elf)

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
