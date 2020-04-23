// Common/Defs.h

#ifndef __COMMON_DEFS_H
#define __COMMON_DEFS_H

#define EXCLUDE_COM
#define COMPRESS_DEFLATE
#define COMPRESS_DEFLATE64
#define COMPRESS_BZIP2
#define _7ZipSupport
#define ZipSupport
#define RarSupport
//#define RpmSupport
//#define DebSupport
#define TarSupport
#define GZSupport
#define BZip2Support
//#define CabSupport
#define ArjSupport
//#define CPIOSupport

//#define RAR_LINKED_STATIC

#define COMPRESS_LZMA

#define COMPRESS_PPMD
#define COMPRESS_BCJ_X86
#define COMPRESS_BCJ2
#define COMPRESS_COPY

#define CodecsSubDir "Codecs\\"
//#define CodecsSubDir ""

template <class T> inline T MyMin(T a, T b)
  {  return a < b ? a : b; }
template <class T> inline T MyMax(T a, T b)
  {  return a > b ? a : b; }

template <class T> inline int MyCompare(T a, T b)
  {  return a < b ? -1 : (a == b ? 0 : 1); }

inline int BoolToInt(bool value)
  { return (value ? 1: 0); }

inline bool IntToBool(int value)
  { return (value != 0); }

#endif
