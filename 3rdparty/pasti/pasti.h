//
// PASTI DLL
// FDC Emulator
//	external interface definitions
//

// SSE modifications
// long replaced with LONG_PTR where appropriate (for a future x64 pasti.dll)
// C calling convention specified to be compatible with a host using Pascal
// calling convention

#pragma once
#ifndef PASTI_DLL_H
#define PASTI_DLL_H

#define PASTI_CALLCONV __cdecl

#ifndef DllExport
#define DllExport __declspec( dllimport )
#endif

#ifndef WINVER
typedef void *HWND;
#endif

#define PASTI_VERSION		0x0002

// Dll flags

#define PASTI_DFDEBUG		0x01
#define PASTI_DFBETA		0x8000

// Application flags

#define PASTI_AFUSEHBL		0x01
#define PASTI_AFUPDONLY		0x02
#define PASTI_AFRDMA		0x04

// I/O mode

#define PASTI_IOUPD			0x00
#define PASTI_IOREAD		0x01
#define PASTI_IOWRITE		0x02

// Image load/save modes

#define PASTI_LDQUERYSIZE	0x00
#define PASTI_LDFNAME		0x01
#define PASTI_LDMEM			0x02

#define PASTI_LDUSEGEOMETRY	0x100

// Image types

#define PASTI_ITNODISK		0x00
#define PASTI_ITST			0x01
#define PASTI_ITMSA			0x02
#define PASTI_ITPROT		0x03

#define PASTI_ITUNPROT		0x01

// Configuration flags

#define PASTI_CFDRIVES		0x01
#define PASTI_CFOPTIONS		0x02
#define PASTI_CFDETAILSPEED	0x04
#define PASTI_CFSIMPLESPEED	0x08

#define PASTI_CFAPPOPTIONS	(PAST_CFDRIVES)
#define PASTI_CFUSROPTIONS	(PASTI_CFOPTIONS | PASTI_CFDETAILSPEED)
#define PASTI_CFALL			(PASTI_CFAPPOPTIONS | PASTI_CFUSROPTIONS)

// Options setting

#define PASTI_OPWPOFF		0x100
#define PASTI_OPNORAND		0x200
#define PASTI_OPFORMAT		0x400

// Warnings

#define PASTI_WNUNIMG		0x01
#define PASTI_WNRDTRK		0x02
#define PASTI_WNWRITEPROT	0x04
#define PASTI_WNUNIMPLWRT	0x08
#define PASTI_WNONCEPTRK	0x10


// Dialogs flags

#define PASTI_DFNOINIT		0x01
#define PASTI_DFNOAPPLY		0x02

// Load Save config modes

#define PASTI_LCSTRINGS		0

// pastiBreakpoint subfunc codes

#define PASTI_BRK_SET		1
#define PASTI_BRK_GET		2
#define PASTI_BRK_DEL		3
#define PASTI_BRK_KILL		4

// Error values

#define pastiErrNoerr		0
#define pastiErrNotInited	-1
#define pastiErrGeneric		-2
#define pastiErrOsErr		-3
#define pastiErrNoMem		-4
#define pastiErrFileFmt		-5
#define pastiErrUnimpl		-6
#define pastiErrInvalidDrv	-7
#define pastiErrNoDisk		-8
#define pastiErrInvParam	-9
#define pastiErrIncompatVersion	-10

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct pastiFUNCS
{
	BOOL (PASTI_CALLCONV *Io) ( int mode, struct pastiIOINFO *);
#ifdef SSE_X64
  BOOL (PASTI_CALLCONV *WritePorta) ( unsigned data, LONG_PTR cycles);
#else
	BOOL (PASTI_CALLCONV *WritePorta) ( unsigned data, long cycles);
#endif
	BOOL (PASTI_CALLCONV *Config) ( struct pastiCONFIGINFO *);
	BOOL (PASTI_CALLCONV *GetConfig) ( struct pastiCONFIGINFO *pInfo);
	BOOL (PASTI_CALLCONV *HwReset) ( BOOL bPowerUp);
	int (PASTI_CALLCONV *GetLastError) ( void);
#ifdef SSE_X64
	BOOL ( *ImgLoad) ( int drive, BOOL bWprot, BOOL bDelay, LONG_PTR cycles,
		struct pastiDISKIMGINFO *);
#else
	BOOL (PASTI_CALLCONV *ImgLoad) ( int drive, BOOL bWprot, BOOL bDelay, long cycles,
		struct pastiDISKIMGINFO *);
#endif

	BOOL (PASTI_CALLCONV *SaveImg) ( int drive, BOOL bAlways, struct pastiDISKIMGINFO *);
#ifdef SSE_X64
  BOOL ( *Eject) ( int drive, LONG_PTR cycles);
#else
	BOOL (PASTI_CALLCONV *Eject) ( int drive, long cycles);
#endif
	BOOL (PASTI_CALLCONV *GetBootSector) ( int drive, struct pastiBOOTSECTINFO *);
	int (PASTI_CALLCONV *GetFileExtensions) ( char *buf, int bufSize, BOOL bAll);

	BOOL (PASTI_CALLCONV *SaveState) ( struct pastiSTATEINFO *);
	BOOL (PASTI_CALLCONV *LoadState) ( struct pastiSTATEINFO *);

	BOOL (PASTI_CALLCONV *LoadConfig )( struct pastiLOADINI *, pastiCONFIGINFO *);
	BOOL (PASTI_CALLCONV *SaveConfig) ( struct pastiLOADINI *, const pastiCONFIGINFO *);

	BOOL (PASTI_CALLCONV *Peek) ( struct pastiPEEKINFO *);
	BOOL (PASTI_CALLCONV *Breakpoint) ( unsigned subfunc, int n, struct pastiBREAKINFO *);

	BOOL (PASTI_CALLCONV *DlgConfig) ( HWND hWnd, unsigned flags, struct pastiCONFIGINFO *);
	BOOL (PASTI_CALLCONV *DlgBreakpoint) ( HWND hWnd);
	BOOL (PASTI_CALLCONV *DlgStatus) ( HWND hWnd);
	BOOL (PASTI_CALLCONV *DlgFileProps) ( HWND hWnd, const char *fileName);

	BOOL (PASTI_CALLCONV *Extra) ( unsigned code, void *ptr);
};

struct pastiCALLBACKS
{
	BOOL (PASTI_CALLCONV *DmaXfer) ( const struct pastiDMAXFERINFO *);
	void (PASTI_CALLCONV *MotorOn) ( BOOL bOn);
	void (PASTI_CALLCONV *IntrqChg) ( BOOL bOn);

	void (PASTI_CALLCONV *loadDelay) ( int drive, BOOL bStart);

	void (PASTI_CALLCONV *LogMsg) ( const char *msg);
	void (PASTI_CALLCONV *WarnMsg) ( const char *msg);

	void (PASTI_CALLCONV *BreakHit) ( int n);
};

struct pastiINITINFO
{
	unsigned dwSize;

	unsigned applFlags;
	unsigned applVersion;

	const struct pastiCALLBACKS *cBacks;

	unsigned dllFlags;
	unsigned dllVersion;

	// array of dll public func pointers
	const struct pastiFUNCS *funcs;
};

struct pastiSPEEDINFO
{
	BOOL slowSectRead;
	BOOL slowTrackRead;

	BOOL slowRotation;
	BOOL slowSpinup;
	BOOL slowSettle;

	BOOL slowSeek;
	BOOL slowVerify;
	BOOL slowSnf;
};

struct pastiCONFIGINFO
{
	unsigned flags;						// Which options to configure and which not

	int ndrives;
	unsigned drvFlags;					// drivesides & tracks

	int logLevel;
	unsigned options;
	unsigned warnings;

	unsigned reserved;


	BOOL slowSpeed;						// Simplified speed settings

	BOOL fastUnprotDisk;				// Not copy-protected disks
	BOOL fastUnprotTracks;				// Not copy-protected tracks

	struct pastiSPEEDINFO SpeedInfo;	// Detailed speed settings

};

struct pastiGEOMETRY
{
	unsigned nSides;
	unsigned sectsPerTrack;
	unsigned nTracks;
	unsigned bytesPerSector;
};

struct pastiDISKIMGINFO
{
	unsigned mode;

	unsigned imgType;
	const char *fileName;
	void *fileBuf;

	long fileLength;
	long bufSize;

	struct pastiGEOMETRY geometry;

	BOOL bDirty;
};

struct pastiDMAXFERINFO
{
	BOOL memToDisk;
	unsigned xferLen;
	void *xferBuf;
	unsigned xferSTaddr;
};

struct pastiIOINFO
{
	unsigned addr;
	unsigned data;

	long stPC;				// Only for debugging
#ifdef SSE_X64
	LONG_PTR cycles;			// Current ST main clock cycles counter
	LONG_PTR updateCycles;		// Need update after these cycles
#else
	long cycles;			// Current ST main clock cycles counter
	long updateCycles;		// Need update after these cycles
#endif
	BOOL intrqState;
	BOOL haveXfer;
	BOOL brkHit;

	struct pastiDMAXFERINFO xferInfo;
};

struct pastiLOADINI
{
	int mode;
	const char *name;
	void *buffer;
	unsigned bufSize;
};

struct pastiSTATEINFO
{
	void *buffer;
	unsigned long bufSize;
#ifdef SSE_X64
  LONG_PTR cycles;
#else
	unsigned long cycles;
#endif
};

struct pastiBOOTSECTINFO
{
	unsigned nBoots;
	unsigned readSize;
	void *buffer;
};

struct pastiPEEKINFO
{
	BOOL motorOn;
	BOOL intrqState;

	unsigned char commandReg;
	unsigned char statusReg;
	unsigned char sectorReg;
	unsigned char trackReg;
	unsigned char dataReg;

	unsigned char drvaTrack;
	unsigned char drvbTrack;

	unsigned char drvSelect;

	unsigned long dmaBase;
	unsigned dmaControl;
	unsigned dmaStatus;
	unsigned dmaCount;
};

struct pastiBREAKINFO
{
	BOOL enabled;

	// fdc registers
	unsigned char cmdRegValue;
	unsigned char cmdRegMask;
	unsigned char trackRegMin;
	unsigned char trackRegMax;
	unsigned char sectRegMin;
	unsigned char sectRegMax;
	unsigned char statusRegValue;
	unsigned char statusRegMask;

	unsigned char dataRegMin;
	unsigned char dataRegMax;

	// select bits
	unsigned char drvSelValue;
	unsigned char drvSelMask;

	// track head position
	unsigned char trkHeadMin;
	unsigned char trkHeadMax;

	// PC
	unsigned long pcMin;
	unsigned long pcMax;

	// dmabase
	unsigned long dmaAddrMin;
	unsigned long dmaAddrMax;
};


// typedef prototype for casting GetProcAddress pointer

typedef BOOL PASTIINITPROC( struct pastiINITINFO *);
typedef PASTIINITPROC *LPPASTIINITPROC;

DllExport BOOL PASTI_CALLCONV pastiInit( struct pastiINITINFO *);

DllExport BOOL PASTI_CALLCONV pastiConfig( struct pastiCONFIGINFO *);
DllExport BOOL PASTI_CALLCONV pastiGetConfig( struct pastiCONFIGINFO *);
DllExport BOOL PASTI_CALLCONV pastiHwReset( BOOL bPowerUp);
DllExport int PASTI_CALLCONV pastiGetLastError( void);

DllExport BOOL PASTI_CALLCONV pastiIo( int mode, struct pastiIOINFO *);
#ifdef SSE_X64
DllExport BOOL PASTI_CALLCONV pastiWritePorta( unsigned data, LONG_PTR cycles);

DllExport BOOL PASTI_CALLCONV pastiImgLoad( int drive, BOOL bWprot, BOOL bDelay, LONG_PTR cycles,
				struct pastiDISKIMGINFO *);
#else
DllExport BOOL PASTI_CALLCONV pastiWritePorta( unsigned data, long cycles);

DllExport BOOL PASTI_CALLCONV pastiImgLoad( int drive, BOOL bWprot, BOOL bDelay, long cycles,
				struct pastiDISKIMGINFO *);
#endif
DllExport BOOL PASTI_CALLCONV pastiSaveImg( int drive, BOOL bAlways, pastiDISKIMGINFO *);
#ifdef SSE_X64
DllExport BOOL PASTI_CALLCONV pastiEject( int drive, LONG_PTR cycles);
#else
DllExport BOOL PASTI_CALLCONV pastiEject( int drive, long cycles);
#endif
DllExport int PASTI_CALLCONV pastiGetFileExtensions( char *newExts, int bufSize, BOOL bAll);
DllExport BOOL PASTI_CALLCONV pastiGetBootSector( int drive, struct pastiBOOTSECTINFO *);

DllExport BOOL PASTI_CALLCONV pastiPeek( struct pastiPEEKINFO *);
DllExport BOOL PASTI_CALLCONV pastiBreakpoint( unsigned subfunc, int n, struct pastiBREAKINFO *);

DllExport BOOL PASTI_CALLCONV pastiSaveState( struct pastiSTATEINFO *);
DllExport BOOL PASTI_CALLCONV pastiLoadState( struct pastiSTATEINFO *);

DllExport BOOL PASTI_CALLCONV pastiLoadConfig( struct pastiLOADINI *, struct pastiCONFIGINFO *);
DllExport BOOL PASTI_CALLCONV pastiSaveConfig( struct pastiLOADINI *, const struct pastiCONFIGINFO *);

DllExport BOOL PASTI_CALLCONV pastiDlgConfig( HWND hWnd, unsigned flags,
											struct pastiCONFIGINFO *);
DllExport BOOL PASTI_CALLCONV pastiDlgBreakpoint( HWND hWnd);
DllExport BOOL PASTI_CALLCONV pastiDlgStatus( HWND hWnd);
DllExport BOOL PASTI_CALLCONV pastiDlgFileProps( HWND hWnd, const char *fileName);

DllExport BOOL PASTI_CALLCONV pastiExtra( unsigned code, void *ptr);


#ifdef __cplusplus
}				/* End of extern "C" { */
#endif

#endif//#ifndef PASTI_DLL_H
