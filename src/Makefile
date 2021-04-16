##############################################################################
# Title        : AVR Makefile for Windows
#
# Created      : Matthew Millman 2018-05-29
#                http://tech.mattmillman.com/
#
##############################################################################

# Fixes clash between windows and coreutils mkdir. Comment out the below line to compile on Linux
COREUTILS  = C:/Projects/coreutils/bin/

CLOCK      = 16000000
SRCS       = main.c onewire.c ds18b20.c veml7700.c mcp9808.c ds28e17.c ds2482.c ow_bitbang.c i2c.c util.c crc8.c crc16_arc.c usart_buffered.c
OBJS       = $(SRCS:.c=.o)
DEPDIR     = deps
DEPFLAGS   = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
RM         = rm
MV         = mv
MKDIR      = $(COREUTILS)mkdir
LDFLAGS    = -Wl,-u,vfprintf -lprintf_flt -lm

ifeq ($(ARDUINO), LEONARDO)
DEVICE     = atmega32u4
CFLAGS     = -D_LEONARDO_
PROGRAMMER = -c arduino -P COM6 -c avr109 -b 57600 
else
DEVICE     = atmega328p
CFLAGS     = -D_UNO_
PROGRAMMER = -P COM3 -c arduino -b 115200  
endif

POSTCOMPILE = $(MV) $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os $(DEPFLAGS) $(CFLAGS) -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

all:	owdemo.hex

.c.o:
	@$(MKDIR) -p $(DEPDIR)
	$(COMPILE) -c $< -o $@
	@$(POSTCOMPILE)

.S.o:
	@$(MKDIR) -p $(DEPDIR)
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
	@$(POSTCOMPILE)

.c.s:
	@$(MKDIR) -p $(DEPDIR)
	$(COMPILE) -S $< -o $@
	@$(POSTCOMPILE)

flash:	all
	$(AVRDUDE) -U flash:w:owdemo.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

install: flash

clean:
	$(RM) -f owdemo.hex owdemo.elf $(OBJS)

owdemo.elf: $(OBJS)
	$(COMPILE) -o owdemo.elf $(OBJS) $(LDFLAGS)

owdemo.hex: owdemo.elf
	avr-objcopy -j .text -j .data -O ihex owdemo.elf owdemo.hex

disasm:	owdemo.elf
	avr-objdump -d owdemo.elf

cpp:
	$(COMPILE) -E $(SRCS)

$(DEPDIR)/%.d:
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))