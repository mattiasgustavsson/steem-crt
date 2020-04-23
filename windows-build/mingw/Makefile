#####################################
#      Makefile for Steem           #
#####################################

MINGWDIR=/usr/i586-mingw32msvc
STEEMDIR=../../steem
3RDDIR=../../3rdparty
INCDIR=../../include
RCDIR=$(STEEMDIR)/rc
ASMDIR=$(STEEMDIR)/asm

# Compiler
CCP:=i586-mingw32msvc-g++
CC:=i586-mingw32msvc-gcc
WINDRES:=i586-mingw32msvc-windres
ASM:=nasm

# Parameters given to the compiler
STEEMFLAGS=-D_MINGW_BUILD -DWIN32

CFLAGS=-Wall 
CFLAGS+=-B$(MINGWDIR)/BIN/ -I/usr/i686-w64-mingw32/include -L/usr/i686-w64-mingw32/lib 
CFLAGS+=-I$(INCDIR) -I$(3RDDIR) -I$(STEEMDIR) -I$(STEEMDIR)/code

# RELEASE_CFLAGS=-O 
# RELEASE_CFLAGS=-Wall $(RELEASE_CFLAGS)

ifeq ($(DEBUG),1)
	STEEMFLAGS+=-D_DEBUG_BUILD
endif

ifeq ($(USER),1)
	STEEMFLAGS+=-D_NO_DEBUG_BUILD
endif

ifeq ($(RELEASE),1)
	STEEMFLAGS+=$(RELEASE_CFLAGS)
endif

ifeq ($(GDB),1)
	STEEMFLAGS+= -ggdb
endif

# Output filename
OUTPUT="Steem.exe"

# Libraries
OBJS=-lwinmm -luuid -lcomctl32 -lole32 

#OBJS+=asm_draw.o emu.o asm_osd.o asm_int.o urarlib.o
OBJS+=asm_draw.o emu.o asm_osd.o asm_int.o
OBJS+=Steem.o helper.o resource.o portio.o

# Targets

all: 
	$(MAKE) clean
	$(MAKE) rc
	$(MAKE) helper
	$(MAKE) emu
	$(MAKE) main 
	$(MAKE) link

release:
	$(MAKE) clean
	$(ASM) -fcoff ../../include/asm/asm_portio.asm -o portio.o
	$(ASM) -fcoff -dMINGW -dWIN32 -i ../../steem/asm/ ../../steem/asm/asm_draw.asm -o asm_draw.o -w+macro-params -w+macro-selfref -w+orphan-labels
	$(ASM) -fcoff -dMINGW -dWIN32 -i../../include/asm/ -i../../steem/asm/ ../../steem/asm/asm_osd_draw.asm -o asm_osd.o -w+macro-params -w+macro-selfref -w+orphan-labels
	$(ASM) -fcoff ../../steem/asm/asm_int_16_2.asm -o asm_int.o -w+macro-params -w+macro-selfref -w+orphan-labels
#	$(MAKE) urarlib
	$(MAKE) rc
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link
	strip -s $(OUTPUT)

user: 
	$(MAKE) all

debug: 
	$(MAKE) all

all_gdb:
	$(MAKE) all

user_release:
	$(MAKE) release

debug_release: 
	$(MAKE) release


helper:
	$(CCP) -c $(STEEMDIR)/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CCP) -c $(STEEMDIR)/Steem.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CCP) -c $(STEEMDIR)/emu.cpp $(CFLAGS) $(STEEMFLAGS)


link: 
	$(CCP) -o $(OUTPUT) $(CFLAGS) $(OBJS) 


rc:
	$(WINDRES) --include-dir=$(MINGWDIR)/INCLUDE/ --include-dir=$(STEEMDIR) --use-temp-file -O coff -i "$(RCDIR)/resource.rc" -o "resource.o"

urarlib:
	$(CC) -c "$(3RDDIR)/urarlib/urarlib.c" $(CFLAGS) -ourarlib.o $(RELEASE_CFLAGS)

draw_c:
	$(CCP) -o draw_c.o -c "$(STEEMDIR)/code/draw_c/draw_c.cpp" $(CFLAGS) $(STEEMFLAGS) $(RELEASE_CFLAGS)

clean:
	rm -rf Steem.o
	rm -rf emu.o
	rm -rf helper.o
	rm -rf resource.o
