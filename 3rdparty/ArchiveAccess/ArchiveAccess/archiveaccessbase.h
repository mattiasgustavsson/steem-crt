///////////////////////////////////////////////////////////////////////////////
// ArchiveAccess.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file defines basic functions and settings for ArchiveAccess
///////////////////////////////////////////////////////////////////////////////

#ifndef ArchiveAccessConfig__H
#define ArchiveAccessConfig__H

#ifdef _WIN32
const char OSPathSep = '\\';
#else
const char OSPathSep = '/';
#endif

#include "../CPP/Common/MyCom.h"
//#include "7zip/archive/common/CoderLoader.h"

// Get a reference to codecs
//CCoderLibraries& getCoders ();

#endif
