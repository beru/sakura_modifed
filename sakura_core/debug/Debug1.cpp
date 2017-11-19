#include "StdAfx.h"
#include <stdarg.h>
#include <tchar.h>
#include "debug/Debug1.h"
#include "debug/Debug3.h"

// �f�o�b�O�p�֐�

#if 0
// �f�o�b�O�E�H�b�`�p�̌^
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
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief �����t���f�o�b�K�o��

	@param[in] lpFmt printf�̏����t��������

	�����ŗ^����ꂽ����DebugString�Ƃ��ďo�͂���D
*/
void DebugOutW(LPCWSTR lpFmt, ...)
{
	// ���`
	static wchar_t szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(szText, _countof(szText), lpFmt, argList);

	// �o��
	::OutputDebugStringW(szText);
	if (ret == -1) {
		::OutputDebugStringW(L"(�؂�̂Ă܂���...)\n");
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	// �E�F�C�g
	::Sleep(1);	// ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}

void DebugOutA(LPCSTR lpFmt, ...)
{
	// ���`
	static CHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(szText, _countof(szText), lpFmt, argList);

	// �o��
	::OutputDebugStringA(szText);
	if (ret == -1) {
		::OutputDebugStringA("(�؂�̂Ă܂���...)\n");
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	// �E�F�C�g
	::Sleep(1);	// ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}

#endif	// _DEBUG || USE_RELPRINT

