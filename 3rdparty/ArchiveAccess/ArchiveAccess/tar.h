/*
 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Ian F. Darwin and others.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Header file for public domain tar (tape archive) program.
 *
 * @(#)tar.h 1.20 86/10/29	Public Domain.
 *
 * Created 25 August 1985 by John Gilmore, ihnp4!hoptoad!gnu.
 *
 * $Id: tar.h,v 1.7 2003/10/14 19:29:56 christos Exp $ # checkin only
 */

#ifndef TAR__H
#define TAR__H

extern "C" {

const int TarRecordSize	  = 512;
const int TarNameSizeLen  = 100;
const int TarBigSizeLen   = 12;
const int TarSmallSizeLen = 8;
const int TarMagicLen     = 8;
const int TarUNameLen     = 32;
const int TarGNameLen     = 32;

union TarHeader {
	char		charptr[TarRecordSize];
	struct header {
		char	name[TarNameSizeLen];
		char	mode[TarSmallSizeLen];
		char	uid[TarSmallSizeLen];
		char	gid[TarSmallSizeLen];
		char	size[TarBigSizeLen];
		char	mtime[TarBigSizeLen];
		char	checksum[TarSmallSizeLen];
		char	linkflag;
		char	linkname[TarNameSizeLen];
		char	magic[TarMagicLen];
		char	uname[TarUNameLen];
		char	gname[TarGNameLen];
		char	devmajor[TarSmallSizeLen];
		char	devminor[TarSmallSizeLen];
	} header;
};

// The checksum field is filled with this while the checksum is computed.
const char TarChecksumBlanks[] = "        "; // 8 blanks, no null

// The magic field is filled with this if uname and gname are valid. 
const char TarMagic[] = "ustar  "; // 7 chars and a null 

// Modes for DOS files
#define TarDOSModeRW "0000777"
#define TarDOSModeR  "0000555"

// The linkflag defines the type of file
#define	LF_OLDNORMAL	'\0'	// Normal disk file, Unix compat
#define	LF_NORMAL	'0'		// Normal disk file
#define	LF_LINK		'1'		// Link to previously dumped file
#define	LF_SYMLINK	'2'		// Symbolic link
#define	LF_CHR		'3'		// Character special file
#define	LF_BLK		'4'		// Block special file
#define	LF_DIR		'5'		// Directory
#define	LF_FIFO		'6'		// FIFO special file
#define	LF_CONTIG	'7'		// Contiguous file

int __stdcall is_tar(const unsigned char *buf, size_t nbytes);

}

#endif
