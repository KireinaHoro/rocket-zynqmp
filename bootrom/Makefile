bootrom_img = bootrom.rv64.img
bootrom_dump = bootrom.rv64.dump

GCC=riscv64-unknown-elf-gcc
CFLAGS_RV64=-mabi=lp64 -march=rv64ima
OBJCOPY=riscv64-unknown-elf-objcopy
OBJDUMP=riscv64-unknown-elf-objdump

img: $(bootrom_img)

dump: $(bootrom_dump)

%.img: %.elf
	$(OBJCOPY) -O binary --change-addresses=-0x10000 $< $@

%.rv64.elf: %.S bootloader.c linker.ld
	$(GCC) $(CFLAGS_RV64) -Tlinker.ld $^ -nostdlib -ffreestanding -static -o $@

%.dump: %.elf
	$(OBJDUMP) -D $< > $@

clean:
	rm -f *.elf *.dump *.img
