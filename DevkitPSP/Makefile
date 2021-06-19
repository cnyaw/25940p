TARGET = 25940p
OBJS = font.h script.h main.o

CFLAGS = -O2 -G0 -Wall -DDEVKIT_PSP
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 25940p

LIBS= -lpspgum -lpspgu -lm -lstdc++
INCDIR = ../../smallworld2/include ../../stge ../../../open/yard-1.5 ../

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

font.h : font.raw
	bin2c font.raw font.h font

script.h : ../25940.stge
	bin2c ../25940.stge script.h script
