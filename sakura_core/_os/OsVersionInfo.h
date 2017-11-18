#pragma once

#ifndef _WIN32_WINNT_WIN2K
#define _WIN32_WINNT_WIN2K	0x0500
#endif
#ifndef _WIN32_WINNT_WINXP
#define _WIN32_WINNT_WINXP	0x0501
#endif
#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA	0x0600
#endif
#ifndef _WIN32_WINNT_WIN7
#define _WIN32_WINNT_WIN7	0x0601
#endif

#ifdef USE_SSE2
#ifdef __MINGW32__
#include <cpuid.h>
#else
#include <intrin.h>
#endif
#endif

class OsVersionInfo {
public:
	// ���������s��(�����̓_�~�[)
	// �ďo�͊�{1��̂�
	OsVersionInfo(bool pbStart);

	// �ʏ�̃R���X�g���N�^
	// �������Ȃ�
	OsVersionInfo() {}

	// OsVersion���擾�ł������H
	BOOL GetVersion() {
		return bSuccess;
	}

	// �g�p���Ă���OS�iWindows�j���A����Ώۂ��m�F����
	bool OsIsEnableVersion() {
#if (WINVER >= _WIN32_WINNT_WIN7)
		return (_IsWin32NT() &&
			(osVersionInfo.dwMajorVersion >= 7 ||
			(osVersionInfo.dwMajorVersion == 6 && osVersionInfo.dwMinorVersion >= 1)));
#elif (WINVER >= _WIN32_WINNT_VISTA)
		return (_IsWin32NT() && (osVersionInfo.dwMajorVersion >= 6));
#elif (WINVER >= _WIN32_WINNT_WIN2K)
		return (_IsWin32NT() && (osVersionInfo.dwMajorVersion >= 5));
#else
		return (osVersionInfo.dwMajorVersion >= 4);
#endif
	}


	/*! NT�v���b�g�t�H�[�����ǂ������ׂ�

		@retval true NT platform
		@retval false non-NT platform
	*/
	bool _IsWin32NT() {
		return (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	/*! Windows�v���b�g�t�H�[�����ǂ������ׂ�

		@retval true Windows platform
		@retval false non-Windows platform
	*/
	bool IsWin32Windows() {
		return (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

	/*	::WinHelp(hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()");
		���g�p�ł��Ȃ��o�[�W�����Ȃ�Atrue
		�g�p�ł���o�[�W�����Ȃ�Afalse
	*/
	bool _HasWinHelpContentsProblem() {
		return (_IsWin32NT() && (osVersionInfo.dwMajorVersion <= 4));
	}

	/*	�ĕϊ���OS�W���Œ񋟂���Ă��Ȃ����B
		�񋟂���Ă��Ȃ��Ȃ�Afalse�B
		�񋟂���Ă���Ȃ�Atrue�B

		Windows95 or WindowsNT�Ȃ�AFASLE�i�񋟂���Ă��Ȃ��j
		����ȊO��OS�Ȃ�Atrue�i�񋟂���Ă���j
	*/
	bool _OsSupportReconvert() {
		return !((osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 0));
	}

	// Windows 2000 version of OPENFILENAME.
	// The new version has three extra members.
	// See CommDlg.h
	bool _IsWinV5forOfn() {
		return (_IsWin2000_or_later() || _IsWinMe()); 
	}

	/*! Windows Vista�ȏォ���ׂ�

		@retval true Windows Vista or later
	*/
	bool _IsWinVista_or_later()
	{
		return (6 <= osVersionInfo.dwMajorVersion);
	}

	/*! Windows XP�ȏォ���ׂ�

		@retval true Windows XP or later
	*/
	bool _IsWinXP_or_later() {
		return (osVersionInfo.dwMajorVersion >= 6 ||	// 2006.06.17 ryoji Ver 6.0, 7.0,...���܂߂�
			(osVersionInfo.dwMajorVersion >= 5 && osVersionInfo.dwMinorVersion >= 1));
	}

	/*! Windows 2000�ȏォ���ׂ�

		@retval true Windows 2000 or later
	*/
	bool _IsWin2000_or_later() {
		return (_IsWin32NT() && (5 <= osVersionInfo.dwMajorVersion));
	}

	/*! Windows Me�����ׂ�

		@retval true Windows Me
	*/
	bool _IsWinMe() {
		return (IsWin32Windows() && (osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 90));
	}

#ifdef USE_SSE2
	/*! SSE2�T�|�[�g���𒲂ׂ�

		@retval true support SSE2
	*/
	bool _SupportSSE2() {
		return bSSE2;
	}
#endif

	/*! Wine��Ŏ��s����Ă��邩�𒲂ׂ�

		@retval true run in Wine
	*/
	bool _IsWine() {
		return bWine;
	}

protected:
	// Class��static(�S�N���X���L)�ϐ��ȊO�����Ȃ�
	static BOOL bSuccess;
	static OSVERSIONINFO osVersionInfo;
#ifdef USE_SSE2
	static bool bSSE2;
#endif
	static bool bWine;
};

inline bool IsWin32NT() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return OsVersionInfo()._IsWin32NT();
#endif
}

inline bool HasWinHelpContentsProblem() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return false;
#else
	return OsVersionInfo()._HasWinHelpContentsProblem();
#endif
}

inline bool OsSupportReconvert() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return OsVersionInfo()._OsSupportReconvert();
#endif
}

inline bool IsWinV5forOfn() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return OsVersionInfo()._IsWinV5forOfn();
#endif
}

inline bool IsWinVista_or_later() {
#if (WINVER >= _WIN32_WINNT_VISTA)
	return true;
#else
	return OsVersionInfo()._IsWinVista_or_later();
#endif
}

inline bool IsWinXP_or_later() {
#if (WINVER >= _WIN32_WINNT_WINXP)
	return true;
#else
	return OsVersionInfo()._IsWinXP_or_later();
#endif
}

inline bool IsWin2000_or_later() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return true;
#else
	return OsVersionInfo()._IsWin2000_or_later();
#endif
}

inline bool IsWinMe() {
#if (WINVER >= _WIN32_WINNT_WIN2K)
	return false;
#else
	return OsVersionInfo()._IsWinMe();
#endif
}

inline bool IsWine() {
	return OsVersionInfo()._IsWine();
}

