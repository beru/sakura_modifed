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
	// 初期化を行う(引数はダミー)
	// 呼出は基本1回のみ
	OsVersionInfo(bool pbStart);

	// 通常のコンストラクタ
	// 何もしない
	OsVersionInfo() {}

	// OsVersionが取得できたか？
	BOOL GetVersion() {
		return bSuccess;
	}

	// 使用しているOS（Windows）が、動作対象か確認する
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


	/*! NTプラットフォームかどうか調べる

		@retval true NT platform
		@retval false non-NT platform
	*/
	bool _IsWin32NT() {
		return (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	/*! Windowsプラットフォームかどうか調べる

		@retval true Windows platform
		@retval false non-Windows platform
	*/
	bool IsWin32Windows() {
		return (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

	/*	::WinHelp(hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()");
		が使用できないバージョンなら、true
		使用できるバージョンなら、false
	*/
	bool _HasWinHelpContentsProblem() {
		return (_IsWin32NT() && (osVersionInfo.dwMajorVersion <= 4));
	}

	/*	再変換がOS標準で提供されていないか。
		提供されていないなら、false。
		提供されているなら、true。

		Windows95 or WindowsNTなら、FASLE（提供されていない）
		それ以外のOSなら、true（提供されている）
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

	/*! Windows Vista以上か調べる

		@retval true Windows Vista or later
	*/
	bool _IsWinVista_or_later()
	{
		return (6 <= osVersionInfo.dwMajorVersion);
	}

	/*! Windows XP以上か調べる

		@retval true Windows XP or later
	*/
	bool _IsWinXP_or_later() {
		return (osVersionInfo.dwMajorVersion >= 6 ||	// Ver 6.0, 7.0,...も含める
			(osVersionInfo.dwMajorVersion >= 5 && osVersionInfo.dwMinorVersion >= 1));
	}

	/*! Windows 2000以上か調べる

		@retval true Windows 2000 or later
	*/
	bool _IsWin2000_or_later() {
		return (_IsWin32NT() && (5 <= osVersionInfo.dwMajorVersion));
	}

	/*! Windows Meか調べる

		@retval true Windows Me
	*/
	bool _IsWinMe() {
		return (IsWin32Windows() && (osVersionInfo.dwMajorVersion == 4) && (osVersionInfo.dwMinorVersion == 90));
	}

#ifdef USE_SSE2
	/*! SSE2サポートかを調べる

		@retval true support SSE2
	*/
	bool _SupportSSE2() {
		return bSSE2;
	}
#endif

	/*! Wine上で実行されているかを調べる

		@retval true run in Wine
	*/
	bool _IsWine() {
		return bWine;
	}

protected:
	// Classはstatic(全クラス共有)変数以外持たない
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

