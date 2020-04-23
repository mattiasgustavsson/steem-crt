#pragma once
// requires  WINVER 0x0501
#include <windows.h>
#include <SSE.h>

class CpuUsage
{
public:
	CpuUsage(void);
	
	short  GetUsage();
private:
	ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);
	bool EnoughTimePassed();
	inline bool IsFirstRun() const { return (m_dwLastRun == 0); }
	
	//system total times
	FILETIME m_ftPrevSysKernel;
	FILETIME m_ftPrevSysUser;

	//process times
	FILETIME m_ftPrevProcKernel;
	FILETIME m_ftPrevProcUser;

	short m_nCpuUsage;
#if defined(SSE_WINDOWS_XP)
  DWORD m_dwLastRun;
#else
	ULONGLONG m_dwLastRun;
#endif
	
	volatile LONG m_lRunCount;
};
