#
# Copyright (c) 2015 Paco García (frangarcj)
# based on Cirne's vita-toolchain test Makefile
#

TARGET = psplib
OBJS   = main.o

LIBS = -L. -lUVL_stub -lpsplib -lvita2d -lfreetype -lpng -lz -lm -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceCtrl_stub -lSceTouch_stub -lSceRtc_stub -lScePower_stub -lSceAppUtil_stub -lSceAudio_stub

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3
ASFLAGS = $(CFLAGS)

all: $(TARGET)_fixup.elf

%_fixup.elf: %.elf
	#advice from xyzz strip before create elf
		$(PREFIX)-strip -g $<
	#i put db.json there use your location
		vita-elf-create  $< $@ db.json

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET)_fixup.elf $(TARGET).elf $(OBJS)

copy: $(TARGET)_fixup.elf
	@cp $(TARGET)_fixup.elf ../../compartido/$(TARGET).elf
	@echo "Copied!"
