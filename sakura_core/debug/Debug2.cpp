#include "StdAfx.h"
#include "debug/Debug2.h"

#ifdef _DEBUG
// デバッグメッセージ出力
void debug_output(const char* str, ...)
{
	static char buf[_MAX_PATH + 150];
	va_list mark;
	va_start(mark, str);
	// FILE名, LINE 式 分必要
	tchar_vsnprintf_s(buf, _countof(buf), str, mark);
	va_end(mark);

	// デバッガに出力
	OutputDebugStringA(buf);
}

// 強制終了
void debug_exit()
{
//	MessageBox(NULL, _T("assertとかに引っ掛かったぽいです"), GSTR_APPNAME, MB_OK);
	__debugbreak();
}

void warning_point()
{
	int n;
	n = 0; // ※←ここにブレークポイントを設けておくと、任意ワーニングでブレークできる
}
#endif	// _DEBUG
