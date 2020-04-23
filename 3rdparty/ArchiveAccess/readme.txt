ArchiveAccess
=============

This is ArchiveAccess, a portable library for access to archive files.
It is meant to support a wide range of archives through a simple C interface.
At the moment, it supports zip, rar, bz2, gz, tar, arj and 7z archive formats.
It only supports decompression, compression is not yet supported.
Decryption should work for zip 2.0 and rar encryption, if the filenames are
not encrypted. Also, the only platform ArchiveAccess has 
been tested on is Windows. Please post bugs and change requests to the public 
forums on 

www.sourceforge.net/projects/archiveaccess


Buidling ArchiveAccess
======================

Just start a build of ArchiveAccess/ArchiveAccess.dsw using VC.
Pay close attention to the definition of CodecsSubDir in Common\Defs.h. It 
controls where your codecs need to be. Please copy the dlls from the codecs
directory into that directory.


Using ArchiveAccess
===================

You can start familiarizing yourself with ArchiveAccess by reading 
ArchiveAccess.h, it should be well-enough documented. There are 
two C++ examples, ArchiveAccessTest and UnpackAll. A Delphi 
example can be found in ArchiveExploder.


Acknowledgements
================

The main code is from 7-Zip, www.7-zip.com, currently based on version 9.20.
The code for recognizing tar archives was written by Ian F. Darwin.


I hope you find this software useful. Have fun!


Björn Ganster
