// CpuUsageCpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "CpuUsage.h"

DWORD WINAPI EatItThreadProc(LPVOID lpParam);
DWORD WINAPI WatchItThreadProc(LPVOID lpParam);

CpuUsage usage;

int _tmain(int argc, _TCHAR* argv[])
{
	//start threads to eat the processor
	CreateThread(NULL, 0, EatItThreadProc, NULL, 0, NULL);
	CreateThread(NULL, 0, EatItThreadProc, NULL, 0, NULL);

	//start threads to watch the processor (to test thread-safety)
	CreateThread(NULL, 0, WatchItThreadProc, NULL, 0, NULL);
	CreateThread(NULL, 0, WatchItThreadProc, NULL, 0, NULL);
	
	while (true)
	{
		Sleep(1000);
	}

	return 0;
}


DWORD WINAPI WatchItThreadProc(LPVOID lpParam)
{
	while (true)
	{
		short cpuUsage = usage.GetUsage();

		printf("Thread id %d: %d%% cpu usage\n", ::GetCurrentThreadId(), cpuUsage);
		Sleep(1000);
	}
}

DWORD WINAPI EatItThreadProc(LPVOID lpParam)
{
	ULONGLONG accum = 0;
	while (true)
	{
		accum++;
	}

	printf("%64d\n", accum);
}