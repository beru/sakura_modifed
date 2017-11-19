#pragma once

#include <ObjIdl.h> // LPDATAOBJECT

// �V�X�e������
BOOL GetSystemResources(int*, int*, int*);	// �V�X�e�����\�[�X�𒲂ׂ�
BOOL CheckSystemResources(const TCHAR*);		// �V�X�e�����\�[�X�̃`�F�b�N

// �N���b�v�{�[�h
bool SetClipboardText(HWND hwnd, const char* pszText, size_t nLength);	// �N���[�v�{�[�h��Text�`���ŃR�s�[����BANSI�ŁBnLength�͕����P�ʁB
bool SetClipboardText(HWND hwnd, const wchar_t* pszText, size_t nLength);	// �N���[�v�{�[�h��Text�`���ŃR�s�[����BUNICODE�ŁBnLength�͕����P�ʁB
bool IsDataAvailable(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat);
HGLOBAL GetGlobalData(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat);

bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount);

//	�}���`���j�^�Ή��̃f�X�N�g�b�v�̈�擾
bool GetMonitorWorkRect(HWND     hWnd, LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(LPCRECT  prc,  LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(POINT    pt,   LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = NULL);

#define PACKVERSION(major, minor) MAKELONG(minor, major)
DWORD GetComctl32Version();					// Comctl32.dll �̃o�[�W�����ԍ����擾
bool IsVisualStyle();						// ���������݃r�W���A���X�^�C���\����Ԃ��ǂ���������
void PreventVisualStyle(HWND hWnd);		// �w��E�B���h�E�Ńr�W���A���X�^�C�����g��Ȃ��悤�ɂ���
void MyInitCommonControls();				// �R�����R���g���[��������������

// �J�����g�f�B���N�g�����[�e�B���e�B�B
// �R���X�g���N�^�ŃJ�����g�f�B���N�g����ۑ����A�f�X�g���N�^�ŃJ�����g�f�B���N�g���𕜌����郂�m�B
class CurrentDirectoryBackupPoint {
public:
	CurrentDirectoryBackupPoint();
	~CurrentDirectoryBackupPoint();
private:
	TCHAR szCurDir[_MAX_PATH];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���b�Z�[�W�萔                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- �}�E�X -- -- //

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif
// �}�E�X�T�C�h�{�^���Ή�
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif

// -- -- �e�[�} -- -- //

#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

// -- -- IME (imm.h) -- -- //

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING
