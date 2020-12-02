debug_img = debug.img
deploy_img = deploy.img

driver_subdir	:= driver
driver_sources	:= $(wildcard $(driver_subdir)/*.c)
driver_include	:= -I$(driver_subdir)
driver_objects	:= $(driver_sources:c=o)
driver_library	:= driver.a

TRIPLE ?= /opt/riscv/rv64imac/bin/riscv64-unknown-elf-
CC=$(TRIPLE)gcc
CPP=$(TRIPLE)cpp
CFLAGS_RV64=-g -Wall -Werror -flto -nostartfiles -fno-strict-aliasing \
	-Os -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	-mno-save-restore -mstrict-align -mabi=lp64d -march=rv64gc \
	-mcmodel=medany -ffreestanding -Wl,--build-id=none
CFLAGS=$(CFLAGS_RV64) -DDEBUG
OBJCOPY=$(TRIPLE)objcopy
STRIP=$(TRIPLE)strip

.PHONY: all clean

.PRECIOUS: %.elf

all: $(deploy_img) $(debug_img)

$(driver_objects): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.img: %.elf
	$(OBJCOPY) -O binary $< $@

%.ld: %.ldS
	$(CPP) -P $^ -o $@

components = adc.c synth.c mixer.c clkbuf.c spi371109.c

%.elf: %.ld bootrom.S main.c $(components) syscalls.c $(driver_objects)
	$(CC) $(CFLAGS) $(driver_include) -T$^ -static -o $@

clean:
	rm -f *.img *.elf $(driver_objects) $(driver_library)
