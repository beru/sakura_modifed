/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <MMSystem.h>
#include "debug/RunningTimer.h"
#include "_main/global.h"

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

/*!
	@date 2002.10.15 genta
*/
void RunningTimer::WriteTrace(const char* msg) const
{
	TRACE(_T("%3d:\"%hs\", %d㍉秒 : %hs\n"), nDepth, szText, timeGetTime() - nStartTime, msg);
}

#endif // #ifdef _DEBUG


