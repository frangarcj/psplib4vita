TARGET_LIB = libpsplib.a
SOURCES		 = source
CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))

OBJS       = $(CFILES:.c=.o)
INCLUDES   = include


PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
AR      = $(PREFIX)-ar
CFLAGS  = -Wall -I$(INCLUDES) -O3 -ftree-vectorize -mfloat-abi=hard -ffast-math -fsingle-precision-constant -ftree-vectorizer-verbose=2 -fopt-info-vec-optimized -funroll-loops
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

install: $(TARGET_LIB) $(VITASDK)/include/psplib
	@cp $(TARGET_LIB) $(VITASDK)/lib
	@cp include/* $(VITASDK)/include/psplib
	@echo "Installed!"

$(VITASDK)/include/psplib:
	@mkdir -p $@
