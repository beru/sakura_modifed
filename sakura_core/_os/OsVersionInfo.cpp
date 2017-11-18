#include "StdAfx.h"
#include "_os/OsVersionInfo.h"
#include "util/RegKey.h"
#include "util/window.h"

// OsVersionInfo�̓���static�ϐ��̒�`
// ��������IsValidVersion()�ōs��
BOOL	 		OsVersionInfo::bSuccess;
OSVERSIONINFO	OsVersionInfo::osVersionInfo;
#ifdef USE_SSE2
bool			COsVersionInfo::bSSE2;
#endif
bool			OsVersionInfo::bWine;

/*!
	���������s��(�����̓_�~�[)
	�ďo�͊�{1��̂�
*/
OsVersionInfo::OsVersionInfo(bool pbStart)
{
	memset_raw(&osVersionInfo, 0, sizeof(osVersionInfo));
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	bSuccess = ::GetVersionEx(&osVersionInfo);

#ifdef USE_SSE2
 		int data[4];
#ifdef __MINGW32__
		__cpuid(1, data[0], data[1], data[2], data[3]);
#else
		__cpuid(data, 1);
#endif
		bSSE2 = (data[3] & (1<<26)) != 0;
#endif

	RegKey reg;
	bWine = (reg.Open(HKEY_CURRENT_USER, _T("Software\\Wine\\Debug"), KEY_READ) == ERROR_SUCCESS);
}

