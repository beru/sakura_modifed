#pragma once

#include <Windows.h>
#include "_main/global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// �E�B���h�E�̊�{�N���X
/*!
	@par Wnd�N���X�̊�{�I�ȋ@�\
	@li �E�B���h�E�쐬
	@li �E�B���h�E���b�Z�[�W�z��

	@par ����?�̃E�B���h�E�̎g�p���@�͈ȉ��̎菇
	@li RegisterWC()	�E�B���h�E�N���X�o�^
	@li Create()		�E�B���h�E�쐬
*/
class Wnd {
protected:
	friend LRESULT CALLBACK CWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	// Constructors
	Wnd(const TCHAR* pszInheritanceAppend = _T(""));
	virtual ~Wnd();
	/*
	||  Attributes & Operations
	*/

	// �E�B���h�E�N���X�o�^
	ATOM RegisterWC(
		HINSTANCE	hInstance,
		HICON		hIcon,			// Handle to the class icon.
		HICON		hIconSm,		// Handle to a small icon
		HCURSOR		hCursor,		// Handle to the class cursor.
		HBRUSH		hbrBackground,	// Handle to the class background brush.
		LPCTSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		LPCTSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
	);

	// �E�B���h�E�쐬
	HWND Create(
		HWND		hwndParent,
		DWORD		dwExStyle,		// extended window style
		LPCTSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
		LPCTSTR		lpWindowName,	// pointer to window name
		DWORD		dwStyle,		// window style
		int			x,				// horizontal position of window
		int			y,				// vertical position of window
		int			nWidth,			// window width
		int			nHeight,		// window height
		HMENU		hMenu			// handle to menu, or child-window identifier
	);

	virtual LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM); // ���b�Z�[�W�z��
protected:
	// ���z�֐�
	virtual LRESULT DispatchEvent_WM_APP(HWND, UINT, WPARAM, LPARAM);	// �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF)
	virtual void PreviCreateWindow(void) { return; } // �E�B���h�E�쐬�O�̏���(�N���X�o�^�O) (virtual)
	virtual void AfterCreateWindow(void) { ::ShowWindow(hWnd, SW_SHOW); } // �E�B���h�E�쐬��̏��� (virtual)

	// ���z�֐� ���b�Z�[�W����(�f�t�H���g����)
	#define DECLH(method) LRESULT method(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {return CallDefWndProc(hwnd, msg, wp, lp);}
	virtual DECLH(OnCreate			);	// WM_CREATE
	virtual DECLH(OnCommand			);	// WM_COMMAND
	virtual DECLH(OnPaint			);	// WM_PAINT
	virtual DECLH(OnLButtonDown		);	// WM_LBUTTONDOWN
	virtual DECLH(OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH(OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH(OnRButtonDown		);	// WM_RBUTTONDOWN
	virtual DECLH(OnRButtonUp		);	// WM_RBUTTONUP
	virtual DECLH(OnRButtonDblClk	);	// WM_RBUTTONDBLCLK
	virtual DECLH(OnMButtonDown		);	// WM_MBUTTONDOWN
	virtual DECLH(OnMButtonUp		);	// WM_MBUTTONUP
	virtual DECLH(OnMButtonDblClk	);	// WM_MBUTTONDBLCLK
	virtual DECLH(OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH(OnMouseWheel		);	// WM_MOUSEWHEEL
	virtual DECLH(OnMouseHWheel		);	// WM_MOUSEHWHEEL
	virtual DECLH(OnTimer			);	// WM_TIMER
	virtual DECLH(OnSize			);	// WM_SIZE
	virtual DECLH(OnMove			);	// WM_MOVE
	virtual DECLH(OnClose			);	// WM_CLOSE
	virtual DECLH(OnDestroy			);	// WM_DSESTROY
	virtual DECLH(OnQueryEndSession);	// WM_QUERYENDSESSION

	virtual DECLH(OnMeasureItem		);	// WM_MEASUREITEM
	virtual DECLH(OnMenuChar		);	// WM_MENUCHAR
	virtual DECLH(OnNotify			);	// WM_NOTIFY
	virtual DECLH(OnDrawItem		);	// WM_DRAWITEM
	virtual DECLH(OnCaptureChanged	);	// WM_CAPTURECHANGED

	// �f�t�H���g���b�Z�[�W����
	virtual LRESULT CallDefWndProc(HWND, UINT, WPARAM, LPARAM);

public:
	// �C���^�[�t�F�[�X
	HWND GetHwnd() const { return hWnd; }
	HWND GetParentHwnd() const { return hwndParent; }
	HINSTANCE GetAppInstance() const { return hInstance; }
	bool GetWindowRect(LPRECT lpRect) { return ::GetWindowRect(hWnd, lpRect) != 0; }

	// ����C���^�[�t�F�[�X (�g�p�͍D�܂����Ȃ�)
	void _SetHwnd(HWND hwnd) { hWnd = hwnd; }

	// �E�B���h�E�W������
	void DestroyWindow();

private: // 2002/2/10 aroka �A�N�Z�X���ύX
	HINSTANCE	hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND		hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	HWND		hWnd;			// ���̃_�C�A���O�̃n���h��
#ifdef _DEBUG
	TCHAR		szClassInheritances[1024];
#endif
};

