distdir		= i686-linux-gnu-capsimage
prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

CC		= gcc
CXX		= g++
INSTALL		= /usr/bin/install -c
TAR		= tar

CFLAGS		= -Wall -Wno-sign-compare -Wno-missing-braces -Wno-parentheses -g -O2 -fomit-frame-pointer  -I./../LibIPF -I./../Codec -I./../Core -I./../Device -I.
CXXFLAGS	= -Wall -Wno-sign-compare -Wno-missing-braces -Wno-parentheses -g -O2 -fomit-frame-pointer -fconserve-space -fno-exceptions -fno-rtti -std=c++11  -I./../LibIPF -I./../Codec -I./../Core -I./../Device -I.
PICFLAGS	= -fPIC
LDFLAGS		=  -shared -Wl,-soname,libcapsimage.so.5 -Wl,--version-script,libcapsimage.map
LIBS		= 

LIBRARY		= libcapsimage.so.5.1
OBJECTS		=  ../Codec/CTRawCodec.o ../Codec/CTRawCodecDecompressor.o ../Codec/DiskEncoding.o ../Core/BaseFile.o ../Core/BitBuffer.o ../Core/CRC.o ../Core/DiskFile.o ../Core/MemoryFile.o CapsAPI.o CapsFDCEmulator.o CapsFile.o CapsFormatMFM.o CapsImage.o CapsImageStd.o CapsLoader.o DiskImage.o DiskImageFactory.o stdafx.o StreamCueImage.o StreamImage.o


.PHONY: all clean distclean dist

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(PICFLAGS) $^ $(LIBS) -o $@

capsimage.o: capsimage.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PICFLAGS) -c $< -o $@

clean:
	$(RM) $(LIBRARY) $(OBJECTS)

distclean:
	$(RM) $(LIBRARY) $(OBJECTS) Makefile config.status config.log config.cache config.h

dist: $(LIBRARY)
	$(RM) -r $(distdir)
	$(INSTALL) -d $(distdir)
	$(INSTALL) -s $(LIBRARY) $(distdir)
	$(INSTALL) HISTORY LICENSE README $(distdir)
	$(TAR) czf $(distdir).tar.gz $(distdir)

install:
	install -d $(DESTDIR)$(libdir)
	install $(LIBRARY) $(DESTDIR)$(libdir)/$(LIBRARY)
