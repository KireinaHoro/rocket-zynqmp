bootrom_elf = bootrom.elf
bootrom_img = bootrom.img

driver_subdir	:= driver
driver_sources	:= $(wildcard $(driver_subdir)/*.c)
driver_include	:= -I$(driver_subdir)
driver_objects	:= $(driver_sources:c=o)
driver_library	:= driver.a

HOST_CXX=g++
TRIPLE=riscv64-unknown-elf-
GCC=$(TRIPLE)gcc
CFLAGS_RV64=-g -Wall -Werror -nostartfiles -fno-strict-aliasing \
	-Os -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	-mno-save-restore -mstrict-align -mabi=lp64d -march=rv64gc \
	-mcmodel=medany -ffreestanding -Wl,--build-id=none
CFLAGS=$(CFLAGS_RV64) -DDEBUG
OBJCOPY=$(TRIPLE)objcopy
STRIP=$(TRIPLE)strip

.PHONY: all clean

all: $(bootrom_img) $(bootrom_elf)

$(driver_objects): %.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@

%.img: %.elf
	$(OBJCOPY) -O binary $< $@

$(driver_library): $(driver_objects)
	ar rcs $@ $^

%.elf: %.S bootloader.c linker.ld $(driver_library)
	$(GCC) $(CFLAGS) $(driver_include) -Tlinker.ld bootrom.S bootloader.c $(driver_library) -static -o $@

clean:
	rm -f $(bootrom_img) *.elf $(driver_objects) $(driver_library)
