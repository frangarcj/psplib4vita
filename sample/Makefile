#
# Copyright (c) 2015 Paco García (frangarcj)
# based on Cirne's vita-toolchain test Makefile
#

TARGET = psplib_sample
OBJS   = main.o

LIBS = -L. -lpsplib -lvita2d -lSceCommonDialog_stub -lfreetype -lpng -lz -lm -lSceKernel_stub \
	-lSceDisplay_stub -lSceGxm_stub	-lSceCtrl_stub -lSceTouch_stub -lSceRtc_stub -lScePower_stub \
	-lSceAppUtil_stub -lSceAudio_stub

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).elf $(TARGET).elf $(OBJS)

copy: $(TARGET)_fixup.elf
	@cp $(TARGET)_fixup.elf ../../compartido/$(TARGET).elf
	@echo "Copied!"
