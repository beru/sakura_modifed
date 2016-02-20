/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/06/23 �V�K�쐬
	@date 1998/12/04 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, hor
	Copyright (C) 2002, aroka, hor, YAZAKI, frozen
	Copyright (C) 2003, little YOSHI
	Copyright (C) 2005, genta
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "dlg/Dialog.h"
#include "doc/EditDoc.h"

class FuncInfo;
class FuncInfoArr; // 2002/2/10 aroka
class DataProfile;

#define OUTLINE_LAYOUT_FOREGROUND (0)
#define OUTLINE_LAYOUT_BACKGROUND (1)
#define OUTLINE_LAYOUT_FILECHANGED (2)

// �t�@�C���c���[�֘A�N���X
enum class FileTreeSettingFromType {
	Common,
	Type,
	File,
};

class FileTreeSetting {
public:
	std::vector<FileTreeItem>	items;		//!< �c���[�A�C�e��
	bool		bProject;				//!< �v���W�F�N�g�t�@�C�����[�h
	SFilePath	m_szDefaultProjectIni;	//!< �f�t�H���gini�t�@�C����
	SFilePath	m_szLoadProjectIni;		//!< ���ݓǂݍ���ł���ini�t�@�C����
	FileTreeSettingFromType	m_eFileTreeSettingOrgType;
	FileTreeSettingFromType	m_eFileTreeSettingLoadType;
};


//!	�A�E�g���C����̓_�C�A���O�{�b�N�X
class DlgFuncList : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFuncList();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless(HINSTANCE, HWND, LPARAM, FuncInfoArr*, LayoutInt, LayoutInt, int, int, bool); // ���[�h���X�_�C�A���O�̕\��
	void ChangeView(LPARAM);	// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
	bool IsDocking() { return m_eDockSide > DockSideType::Float; }
	DockSideType GetDockSide() { return m_eDockSide; }

protected:
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 2007.11.07 ryoji �W���ȊO�̃��b�Z�[�W��ߑ�����

	CommonSetting_OutLine& CommonSet(void) { return m_pShareData->m_common.outline; }
	TypeConfig& TypeSet(void) { return type; }
	int& ProfDockSet() { return CommonSet().nOutlineDockSet; }
	bool& ProfDockSync() { return CommonSet().bOutlineDockSync; }
	bool& ProfDockDisp() { return (ProfDockSet() == 0)? CommonSet().bOutlineDockDisp: TypeSet().bOutlineDockDisp; }
	DockSideType& ProfDockSide() { return (ProfDockSet() == 0)? CommonSet().eOutlineDockSide: TypeSet().eOutlineDockSide; }
	int& ProfDockLeft() { return (ProfDockSet() == 0)? CommonSet().cxOutlineDockLeft: TypeSet().cxOutlineDockLeft; }
	int& ProfDockTop() { return (ProfDockSet() == 0)? CommonSet().cyOutlineDockTop: TypeSet().cyOutlineDockTop; }
	int& ProfDockRight() { return (ProfDockSet() == 0)? CommonSet().cxOutlineDockRight: TypeSet().cxOutlineDockRight; }
	int& ProfDockBottom() { return (ProfDockSet() == 0)? CommonSet().cyOutlineDockBottom: TypeSet().cyOutlineDockBottom; }
	void SetTypeConfig(TypeConfigNum, const TypeConfig&);

public:
	//! ���݂̎�ʂƓ����Ȃ�
	bool CheckListType(int nOutLineType) const { return nOutLineType == m_nOutlineType; }
	void Redraw(int nOutLineType, int nListType, FuncInfoArr*, LayoutInt nCurLine, LayoutInt nCurCol);
	void Refresh(void);
	bool ChangeLayout(int nId);
	void OnOutlineNotify(WPARAM wParam, LPARAM lParam);
	void SyncColor(void);
	void SetWindowText(const TCHAR* szTitle);		// �_�C�A���O�^�C�g���̐ݒ�
	EFunctionCode GetFuncCodeRedraw(int outlineType);
	void LoadFileTreeSetting( FileTreeSetting&, SFilePath& );
	static void ReadFileTreeIni( DataProfile&, FileTreeSetting& );

protected:
	bool m_bInChangeLayout;

	FuncInfoArr*	m_pFuncInfoArr;	// �֐����z��
	LayoutInt		m_nCurLine;			// ���ݍs
	LayoutInt		m_nCurCol;			// ���݌�
	int				m_nSortCol;			// �\�[�g�����ԍ�
	int				m_nSortColOld;		//!< �\�[�g�����ԍ�(OLD)
	bool			m_bSortDesc;		//!< �~��
	NativeW			m_memClipText;		// �N���b�v�{�[�h�R�s�[�p�e�L�X�g
	bool			m_bLineNumIsCRLF;	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	int				m_nListType;		// �ꗗ�̎��
public:
	int				m_nDocType;			// �h�L�������g�̎��
	int				m_nOutlineType;		// �A�E�g���C����͂̎��
	bool			m_bEditWndReady;	// �G�f�B�^��ʂ̏�������
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnNotify(WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	BOOL OnDestroy(void); // 20060201 aroka
	BOOL OnCbnSelChange(HWND hwndCtl, int wID); // 2002/11/1 frozen
	BOOL OnContextMenu(WPARAM, LPARAM);
	void SetData();	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾

	/*
	||  �����w���p�֐�
	*/
	BOOL OnJump( bool bCheckAutoClose = true, bool bFileJump = true );	//	bCheckAutoClose�F�u���̃_�C�A���O�������I�ɕ���v���`�F�b�N���邩�ǂ���
	void SetTreeCpp(HWND);			// �c���[�R���g���[���̏������FC++���\�b�h�c���[
	void SetTreeJava(HWND, bool);	// �c���[�R���g���[���̏������FJava���\�b�h�c���[
	void SetTree(bool tagjump = false, bool nolabel = false);		// �c���[�R���g���[���̏������F�ėp�i
	void SetTreeFile();				// �c���[�R���g���[���̏������F�t�@�C���c���[
	void SetListVB( void );			// ���X�g�r���[�R���g���[���̏������FVisualBasic		// Jul 10, 2003  little YOSHI
	void SetDocLineFuncList();

	void SetTreeFileSub( HTREEITEM, const TCHAR* );
	// 2002/11/1 frozen 
	void SortTree(HWND hWndTree, HTREEITEM htiParent);//!< �c���[�r���[�̍��ڂ��\�[�g����i�\�[�g���m_nSortType���g�p�j
#if 0
2002.04.01 YAZAKI SetTreeTxt()�ASetTreeTxtNest()�͔p�~�BGetTreeTextNext�͂��Ƃ��Ǝg�p����Ă��Ȃ������B
	void SetTreeTxt(HWND);	// �c���[�R���g���[���̏������F�e�L�X�g�g�s�b�N�c���[
	int SetTreeTxtNest(HWND, HTREEITEM, int, int, HTREEITEM*, int);
	void GetTreeTextNext(HWND, HTREEITEM, int);
#endif

	//	Apr. 23, 2005 genta ���X�g�r���[�̃\�[�g���֐��Ƃ��ēƗ�������
	void SortListView(HWND hwndList, int sortcol);
	static int CALLBACK CompareFunc_Asc(LPARAM, LPARAM, LPARAM);
	static int CALLBACK CompareFunc_Desc(LPARAM, LPARAM, LPARAM);

	// 2001.12.03 hor
//	void SetTreeBookMark(HWND);		// �c���[�R���g���[���̏������F�u�b�N�}�[�N
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void Key2Command(WORD);		//	�L�[���쁨�R�}���h�ϊ�
	bool HitTestSplitter(int xPos, int yPos);
	int HitTestCaptionButton(int xPos, int yPos);
	INT_PTR OnNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void GetDockSpaceRect(LPRECT pRect);
	void GetCaptionRect(LPRECT pRect);
	bool GetCaptionButtonRect(int nButton, LPRECT pRect);
	void DoMenu(POINT pt, HWND hwndFrom);
	BOOL PostOutlineNotifyToAllEditors(WPARAM wParam, LPARAM lParam);
	DockSideType GetDropRect(POINT ptDrag, POINT ptDrop, LPRECT pRect, bool bForceFloat);
	BOOL Track(POINT ptDrag);
	bool GetTreeFileFullName(HWND, HTREEITEM, std::tstring*, int*);
	bool TagJumpTimer(const TCHAR*, Point, bool);

private:
	//	May 18, 2001 genta
	/*!
		@brief �A�E�g���C����͎��

		0: List, 1: Tree
	*/
	int	m_nViewType;

	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 1/4
	// (������Ȃ̂łǂȂ����C�����肢���܂�)
	bool m_bWaitTreeProcess;

	// 2002/11/1 frozen
	//! �c���[�r���[���\�[�g����
	// 0 �f�t�H���g(�m�[�h�Ɋ֘A�Â����ꂽ�l��)
	// 1 �A���t�@�x�b�g��
	int m_nSortType;

	// �I�𒆂̊֐����
	FuncInfo* m_funcInfo;
	std::tstring m_sJumpFile;

	const TCHAR* m_pszTimerJumpFile;
	Point		m_pointTimerJump;
	bool		m_bTimerJumpAutoClose;

	DockSideType	m_eDockSide;	// ���݂̉�ʂ̕\���ʒu
	HWND		m_hwndToolTip;	//!< �c�[���`�b�v�i�{�^���p�j
	bool		m_bStretching;
	bool		m_bHovering;
	int			m_nHilightedBtn;
	int			m_nCapturingBtn;
	
	TypeConfig type;
	FileTreeSetting	m_fileTreeSetting;

	static LPDLGTEMPLATE m_pDlgTemplate;
	static DWORD m_dwDlgTmpSize;
	static HINSTANCE m_lastRcInstance;		// ���\�[�X�����`�F�b�N�p

	POINT				m_ptDefaultSize;
	POINT				m_ptDefaultSizeClient;
	RECT				m_rcItems[12];
};

