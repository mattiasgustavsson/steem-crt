#####################################
#      Makefile for Steem           #
#####################################

# requires tools: make, gcc, g++, nasm
# requires libs: rtaudio, /*portaudio*/, zlib, minizip, /*urarlib*/, X

# symbolic links - code, inc, 3rdparty

ROOT=..

# Compiler
CC=g++

# Parameters given to the compiler
STEEMFLAGS=-DUNIX -DLINUX -DSTEVEN_SEAGAL -DNO_PORT_AUDIO
CFLAGS=-I$(ROOT)/include -I$(ROOT)/steem/code \
-I$(ROOT)/3rdparty \
-I$(ROOT)/3rdparty/pasti \
-I$(ROOT)/3rdparty/dsp \
-I$(ROOT)/3rdparty/6301 \
-I$(ROOT)/3rdparty/zlib/contrib/minizip \
-I$(ROOT)/3rdparty/zlib \
-I$(ROOT)/3rdparty/rtaudio \
-lasound -w -Wfatal-errors

RELEASE_CFLAGS=-O

# Output filename
OUTPUT="./output/steem"

    LIBS=-lX11 -lXext -lpthread -lXxf86vm \
      $(ROOT)/3rdparty/zlib/crc32.o  \
      $(ROOT)/3rdparty/zlib/inflate.o \
      $(ROOT)/3rdparty/zlib/adler32.o \
      $(ROOT)/3rdparty/zlib/trees.o \
      $(ROOT)/3rdparty/zlib/inffast.o \
      $(ROOT)/3rdparty/zlib/inftrees.o \
      $(ROOT)/3rdparty/zlib/deflate.o \
      $(ROOT)/3rdparty/zlib/zutil.o \
      $(ROOT)/3rdparty/zlib/compress.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/unzip.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/ioapi.o \
      $(ROOT)/3rdparty/rtaudio/tests/Release/RtAudio.o \
      $(ROOT)/X-build/obj/6301.o \
      $(ROOT)/X-build/obj/dsp.o \
      $(ROOT)/X-build/obj/div68kCycleAccurate.o 

#      ./obj/dsp.o ./obj/6301.o ./obj/div68kCycleAccurate.o


#      $(ROOT)/3rdparty/pasti/div68kCycleAccurate.o

#LIBTOOL=$(ROOT)/3rdparty/portaudio/libtool
LIBTOOL=libtool

# Objects
OBJS=./obj/asm_draw.o ./obj/emu.o ./obj/asm_osd.o ./obj/main.o ./obj/helper.o ./obj/resource.o ./obj/SSE.o ./obj/SSEOption.o 
#./obj/dsp.o ./obj/6301.o ./obj/div68kCycleAccurate.o
# OBJS=./code/draw_c/draw_c.o emu.o main.o urarlib.o helper.o resource.o

# Targets

all:	asm
	mkdir -p obj output
	$(MAKE) res
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) SSE
	$(MAKE) SSEOption
	$(MAKE) link

clean:
	rm -rf obj output

fresh:
	$(MAKE) clean
#	$(MAKE) urarlib
	$(MAKE) asm
	$(MAKE) all

test:
	$(MAKE) all
	$(OUTPUT)

release:
	$(MAKE) clean
	$(MAKE) res
#	$(MAKE) urarlib
	$(MAKE) asm
	$(MAKE) release_compile
	$(MAKE) link_static
	strip -s $(OUTPUT)

helper:
	$(CC) -o ./obj/helper.o -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -o ./obj/main.o -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -o ./obj/emu.o -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(STEEMFLAGS)


asm:
	mkdir -p ./obj
	nasm -felf -o./obj/asm_draw.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_draw.asm
	nasm -felf -o./obj/asm_osd.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_osd_draw.asm

res:
	nasm -felf -o./obj/resource.o -i$(ROOT)/steem/ $(ROOT)/steem/rc/resource.asm

link: 
	$(LIBTOOL) --tag=CC --mode=link $(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)
#	$(LIBTOOL) --tag=CC  $(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_static:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) -static


SSE:
	$(CC) -o ./obj/SSE.o -c $(ROOT)/steem/code/SSE/SSE.cpp $(CFLAGS) $(STEEMFLAGS)

SSEOption:
	$(CC) -o ./obj/SSEOption.o -c $(ROOT)/steem/code/SSE/SSEOption.cpp $(CFLAGS) $(STEEMFLAGS)

#3rdparty:	zlib unrarlib portaudio rtaudio div68kCycleAccurate 6301
3rdparty:	div68kCycleAccurate 6301 dsp zlib rtaudio 

div68kCycleAccurate:
	gcc -c $(ROOT)/3rdparty/pasti/div68kCycleAccurate.c $(CFLAGS) -o./obj/div68kCycleAccurate.o

6301:
	gcc -c $(ROOT)/3rdparty/6301/6301.c $(CFLAGS) -o./obj/6301.o

dsp:
	$(CC) -o ./obj/dsp.o -c $(ROOT)/3rdparty/dsp/dsp.cpp $(CFLAGS) $(STEEMFLAGS)

#unrarlib:
#	gcc -c $(ROOT)/3rdparty/unrarlib/unrarlib/unrarlib.c $(CFLAGS) -o$(ROOT)/3rdparty/unrarlib/unrarlib/unrarlib.o

zlib:
	cd $(ROOT)/3rdparty/zlib; sh configure; make test
	cd $(ROOT)/3rdparty/zlib/contrib/minizip; make all

#portaudio:
#	cd $(ROOT)/3rdparty/portaudio; \
#	sh configure; \
#	make

rtaudio:
	cd $(ROOT)/3rdparty/rtaudio; sh configure --with-alsa; cd tests; make

