/*!	@file
	@brief Dialog Box���N���X�w�b�_�t�@�C��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

class Dialog;

struct DllSharedData;
class Recent;

enum class AnchorStyle {
	None			= 0,
	Left			= 1,
	Right			= 2,
	LeftRight		= 3,
	Top				= 4,
	TopLeft			= 5,
	TopRight		= 6,
	TopLeftRight	= 7,
	Bottom			= 8,
	BottomLeft		= 9,
	BottomRight		= 10,
	BottomLeftRight	= 11,
	TopBottom		= 12,
	TopBottomLeft	= 13,
	TopBottomRight	= 14,
	All				= 15
};

struct AnchorListItem
{
	int id;
	AnchorStyle anchor;
};

struct ComboBoxItemDeleter
{
	Recent*	pRecent;
	HWND		hwndCombo;
	WNDPROC		pComboBoxWndProc;
	WNDPROC		pEditWndProc;
	WNDPROC		pListBoxWndProc;
	ComboBoxItemDeleter(): pRecent(NULL), hwndCombo(NULL), pComboBoxWndProc(NULL), pEditWndProc(NULL), pListBoxWndProc(NULL) {}
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �_�C�A���O�E�B���h�E�������N���X

	�_�C�A���O�{�b�N�X�����Ƃ��ɂ͂�������p��������D

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
*/
class Dialog {
public:
	/*
	||  Constructors
	*/
	Dialog( bool bSizable = false, bool bCheckShareData = true );
	virtual ~Dialog();
	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);		// �_�C�A���O�̃��b�Z�[�W����
	INT_PTR DoModal(HINSTANCE, HWND, int, LPARAM);					// ���[�_���_�C�A���O�̕\��
	HWND DoModeless(HINSTANCE, HWND, int, LPARAM, int);				// ���[�h���X�_�C�A���O�̕\��
	HWND DoModeless(HINSTANCE, HWND, LPCDLGTEMPLATE, LPARAM, int);	// ���[�h���X�_�C�A���O�̕\��
	void CloseDialog(int);
	
	virtual BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	virtual void SetDialogPosSize();
	virtual BOOL OnDestroy(void);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam) {return FALSE;}
	BOOL OnSize();
	virtual BOOL OnSize(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnMove(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnDrawItem(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual BOOL OnTimer(WPARAM wParam) {return TRUE;}
	virtual BOOL OnKeyDown(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual BOOL OnDeviceChange(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual int GetData(void) {return 1;}	// �_�C�A���O�f�[�^�̎擾
	virtual void SetData(void) {return;}	// �_�C�A���O�f�[�^�̐ݒ�
	virtual BOOL OnBnClicked(int);
	virtual BOOL OnStnClicked(int) {return FALSE;}
	virtual BOOL OnEnChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnEnKillFocus(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnLbnSelChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnLbnDblclk(int wID) {return FALSE;}
	virtual BOOL OnCbnSelChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnCbnEditChange(HWND hwndCtl, int wID) {return FALSE;} // @@2005.03.31 MIK �^�O�W�����vDialog
	virtual BOOL OnCbnDropDown(HWND hwndCtl, int wID);
	static BOOL OnCbnDropDown( HWND hwndCtl, bool scrollBar );
//	virtual BOOL OnCbnCloseUp(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnCbnSelEndOk(HWND hwndCtl, int wID);

	virtual BOOL OnKillFocus(WPARAM wParam, LPARAM lParam) {return FALSE;}
	virtual BOOL OnActivate(WPARAM wParam, LPARAM lParam) {return FALSE;}	//@@@ 2003.04.08 MIK
	virtual int OnVKeyToItem(WPARAM wParam, LPARAM lParam) { return -1; }
	virtual LRESULT OnCharToItem(WPARAM wParam, LPARAM lParam) { return -1; }
	virtual BOOL OnPopupHelp(WPARAM, LPARAM);	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu(WPARAM, LPARAM);	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void ResizeItem(HWND hTarget, const POINT& ptDlgDefalut, const POINT& ptDlgNew, const RECT& rcItemDefault, AnchorStyle anchor, bool bUpdate = true);
	void GetItemClientRect(int wID, RECT& rc);
	static void SetComboBoxDeleter(HWND hwndCtl, ComboBoxItemDeleter* data);
public:
	// �ݒ�t�H���_���΃t�@�C���I��(���L�f�[�^,ini�ʒu�ˑ�)
	static BOOL SelectFile(HWND parent, HWND hwndCtl, const TCHAR* filter, bool resolvePath);
	static bool DirectoryUp(TCHAR*);

public:
	HWND GetHwnd() const { return m_hWnd; }
	// ����C���^�[�t�F�[�X (�g�p�͍D�܂����Ȃ�)
	void _SetHwnd(HWND hwnd) { m_hWnd = hwnd; }
	bool IsButtonChecked(int id) { return DlgButton_IsChecked(m_hWnd, id); }
	bool CheckButton(int id, bool bCheck) { return ::CheckDlgButton(m_hWnd, id, bCheck ? BST_CHECKED : BST_UNCHECKED) != 0; }
	UINT GetItemText(int nIDDlgItem, TCHAR* str, int nMaxCount) { return ::GetDlgItemText(m_hWnd, nIDDlgItem, str, nMaxCount); }
	BOOL SetItemText(int nIDDlgItem, const TCHAR* str) { return ::SetDlgItemText(m_hWnd, nIDDlgItem, str); }
	UINT GetItemInt(int nIDDlgItem, BOOL *lpTranslated, BOOL bSigned) { return ::GetDlgItemInt(m_hWnd, nIDDlgItem, lpTranslated, bSigned); }
	bool SetItemInt(int nIDDlgItem, UINT uValue, BOOL bSigned) { return ::SetDlgItemInt(m_hWnd, nIDDlgItem, uValue, bSigned) != 0; }

	HWND GetItemHwnd(int nID) { return ::GetDlgItem(m_hWnd, nID); }
	bool EnableItem(int nID, bool bEnable) { return ::EnableWindow(GetItemHwnd(nID), bEnable ? TRUE : FALSE) != 0; }

	bool GetWindowRect(LPRECT lpRect) { return ::GetWindowRect(m_hWnd, lpRect) != 0; }

public:
	HINSTANCE		m_hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND			m_hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
private:
	HWND			m_hWnd;			// ���̃_�C�A���O�̃n���h��
public:
	HWND			m_hwndSizeBox;
	LPARAM			m_lParam;
	bool			m_bModal;		// ���[�_�� �_�C�A���O��
	bool			m_bSizable;		// �σ_�C�A���O���ǂ���
	int				m_nShowCmd;		// �ő剻/�ŏ���
	int				m_nWidth;
	int				m_nHeight;
	int				m_xPos;
	int				m_yPos;
//	void*			m_pcEditView;
	DllSharedData*	m_pShareData;
	bool			m_bInited;
	HINSTANCE		m_hLangRsrcInstance;		// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��	// 2011.04.10 nasukoji

protected:
	void CreateSizeBox(void);
	BOOL OnCommand(WPARAM, LPARAM);

	// �R���g���[���ɉ�ʂ̃t�H���g��ݒ�	2012/11/27 Uchi
	HFONT SetMainFont(HWND hTarget);
};

