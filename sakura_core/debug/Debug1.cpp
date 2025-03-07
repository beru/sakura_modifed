#include "StdAfx.h"
#include <stdarg.h>
#include <tchar.h>
#include "debug/Debug1.h"
#include "debug/Debug3.h"

// デバッグ用関数

#if 0
// デバッグウォッチ用の型
struct TestArrayA { char    a[100]; };
struct TestArrayW { wchar_t a[100]; };
struct TestArrayI { int     a[100]; };
void Test()
{
	TestArrayA a; a.a[0]=0;
	TestArrayW w; w.a[0]=0;
	TestArrayI i; i.a[0]=0;
}
#endif

#if defined(_DEBUG) || defined(USE_RELPRINT)

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief 書式付きデバッガ出力

	@param[in] lpFmt printfの書式付き文字列

	引数で与えられた情報をDebugStringとして出力する．
*/
void DebugOutW(LPCWSTR lpFmt, ...)
{
	// 整形
	static wchar_t szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(szText, _countof(szText), lpFmt, argList);

	// 出力
	::OutputDebugStringW(szText);
	if (ret == -1) {
		::OutputDebugStringW(L"(切り捨てました...)\n");
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	// ウェイト
	::Sleep(1);	// 大量にトレースするときのために

	va_end(argList);
	return;
}

void DebugOutA(LPCSTR lpFmt, ...)
{
	// 整形
	static CHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(szText, _countof(szText), lpFmt, argList);

	// 出力
	::OutputDebugStringA(szText);
	if (ret == -1) {
		::OutputDebugStringA("(切り捨てました...)\n");
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	// ウェイト
	::Sleep(1);	// 大量にトレースするときのために

	va_end(argList);
	return;
}

#endif	// _DEBUG || USE_RELPRINT

