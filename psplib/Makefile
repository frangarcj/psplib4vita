TARGET_LIB = libpsplib.a
SOURCES		 = source
CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))

OBJS       = $(CFILES:.c=.o)
INCLUDES   = include


PREFIX  = arm-none-eabi
CC      = $(PREFIX)-gcc
AR      = $(PREFIX)-ar
CFLAGS  = -Wall -specs=psp2.specs -I$(INCLUDES)
ASFLAGS = $(CFLAGS)

all: $(TARGET_LIB)

debug: CFLAGS += -DDEBUG_BUILD
debug: all

$(TARGET_LIB): $(OBJS)
	$(AR) -rc $@ $^

$(SOURCES)/font.o: $(INCLUDES)/stockfont.h

$(INCLUDES)/stockfont.h: genfont/genfont genfont/stockfont.fd
	$< < $(word 2,$^) > $@
genfont/genfont: genfont/genfont.c
	cc $< -o $@
#%.o: %.gxp
#	bin2s $^ > $(^:.gxp=.s)
#	$(CC) $(CFLAGS) -c $(^:.gxp=.s) -o $@

#%.frag.o: %.frag.cg
#	cgc -profile ps_3_0 $^ -o $(@:.o=.gxp)
#	bin2s $(@:.o=.gxp) > $(@:.o=.s)
#	$(CC) $(CFLAGS) -c $(@:.o=.s) -o $@

#%.vert.o: %.vert.cg
#	cgc -profile vs_3_0 $^ -o $(@:.o=.gxp)
#	bin2s $(@:.o=.gxp) > $(@:.o=.s)
#	$(CC) $(CFLAGS) -c $(@:.o=.s) -o $@

clean:
	@rm -rf $(TARGET_LIB) $(OBJS)

install: $(TARGET_LIB)
	@cp $(TARGET_LIB) $(PSP2SDK)/lib
	@cp include/* $(PSP2SDK)/include/psplib
	@echo "Installed!"
