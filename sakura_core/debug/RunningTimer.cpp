#include "StdAfx.h"
#include <MMSystem.h>
#include "debug/RunningTimer.h"
#include "_main/global.h"

// 処理所要時間の計測クラス

#if 1 //def _DEBUG

#pragma comment(lib, "winmm.lib")

int RunningTimer::nNestCount = 0;

RunningTimer::RunningTimer(const char* pszText)
{
	Reset();
	if (pszText)
		strcpy(szText, pszText);
	else
		szText[0] = '\0';
	nDepth = nNestCount++;
	TRACE(_T("%3d:\"%hs\" : Enter \n"), nDepth, szText);
	return;
}


RunningTimer::~RunningTimer()
{
	WriteTrace("Exit Scope");
	--nNestCount;
	return;
}


void RunningTimer::Reset()
{
	nStartTime = timeGetTime();
}


DWORD RunningTimer::Read()
{
	return timeGetTime() - nStartTime;
}

void RunningTimer::WriteTrace(const char* msg) const
{
	TRACE(_T("%3d:\"%hs\", %d㍉秒 : %hs\n"), nDepth, szText, timeGetTime() - nStartTime, msg);
}

#endif // #ifdef _DEBUG


