#pragma once

#include <ImageHlp.h> // MakeSureDirectoryPathExists

// �f�o�b�O�p�B
// Vista����ExtTextOut�̌��ʂ������f����Ȃ��B���̊֐���p����Ƒ����f�����̂ŁA
// �f�o�b�O���X�e�b�v���s����ۂɕ֗��ɂȂ�B�������A���R�d���Ȃ�B
#ifdef _DEBUG
#define DEBUG_SETPIXEL(hdc) SetPixel(hdc, -1, -1, 0); // SetPixel������ƁA���ʂ������f�����B
#else
#define DEBUG_SETPIXEL(hdc)
#endif

namespace ApiWrap {
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          W�n�����݂��Ȃ�API�̂��߂́A�V�����֐���`         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// W�ł������̂ŁA����
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR wszDirPath);
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExistsW

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W�n�`��API (ANSI�łł����p�\)                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	/*!
		����������1024���p�����B(�����Ԋu�z���1024���p�����������p�ӂ��Ă��Ȃ�����)
	*/
	inline BOOL ExtTextOutW_AnyBuild(
		HDC				hdc,
		int				x,
		int				y,
		UINT			fuOptions,
		const RECT*		lprc,
		LPCWSTR			lpwString,
		UINT			cbCount,
		const int*		lpDx
	)
	{
		BOOL ret = ::ExtTextOut(hdc, x, y, fuOptions, lprc, lpwString, cbCount, lpDx);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}

	inline BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpwString,
		size_t	cbString
	)
	{
		BOOL ret = ::TextOut(hdc, nXStart, nYStart, lpwString, (int)cbString);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	);

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	);

	#define GetTextExtentPoint32W_AnyBuild GetTextExtentPoint32

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���̑�W�nAPI (ANSI�łł����p�\)               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	inline int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	// �o�b�t�@�̃T�C�Y�B�����P�ʁB
	)
	{
		return ::LoadStringW(hInstance, uID, lpBuffer, nBufferCount);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    �`��API �s����b�v                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Vista��SetPixel�������Ȃ����߁A��֊֐���p�ӁB
	void SetPixelSurely(HDC hdc, int x, int y, COLORREF c);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �悭�g�������l                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �悭�g��ExtTextOutW_AnyBuild�̃I�v�V����
	inline UINT ExtTextOutOption() {
		return ETO_CLIPPED | ETO_OPAQUE;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �悭�g���p�@                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// SHIFT�������Ă��邩�ǂ���
	inline bool GetKeyState_Shift() {
		return (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
	}

	// CTRL�������Ă��邩�ǂ���
	inline bool GetKeyState_Control() {
		return (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
	}

	// ALT�������Ă��邩�ǂ���
	inline bool GetKeyState_Alt() {
		return (::GetKeyState(VK_MENU) & 0x8000) != 0;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �萔                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const size_t sizeof_old_PROPSHEETHEADER = sizeof(PROPSHEETHEADER);
	#else
		static const size_t sizeof_old_PROPSHEETHEADER = 40;
	#endif

	// Win95/NT���[������sizeof(MENUITEMINFO)
	// ����ȊO�̒l��^����ƌÂ�OS�ł����Ɠ����Ă���Ȃ��D
	#if defined(_WIN64) || defined(_UNICODE)
		static const int SIZEOF_MENUITEMINFO = sizeof(MENUITEMINFO);
	#else
		static const int SIZEOF_MENUITEMINFO = 44;
	#endif

}
using namespace ApiWrap;


// �Â�SDK�΍�D�V����SDK�ł͕s�v
#ifndef _WIN64
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif
#ifndef ULONG_PTR
#define ULONG_PTR ULONG
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef UINT_PTR
#define UINT_PTR UINT
#endif
#ifndef INT_PTR
#define INT_PTR INT
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE GWL_HINSTANCE
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT DWL_MSGRESULT
#endif
#endif  //_WIN64

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif

