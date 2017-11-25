/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X
*/
#pragma once

#include <Windows.h>
#include "dlg/Dialog.h"
#include "doc/EditDoc.h"

class FuncInfo;
class FuncInfoArr;
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
	std::vector<FileTreeItem>	items;	// �c���[�A�C�e��
	bool		bProject;				// �v���W�F�N�g�t�@�C�����[�h
	SFilePath	szDefaultProjectIni;	// �f�t�H���gini�t�@�C����
	SFilePath	szLoadProjectIni;		// ���ݓǂݍ���ł���ini�t�@�C����
	FileTreeSettingFromType	eFileTreeSettingOrgType;
	FileTreeSettingFromType	eFileTreeSettingLoadType;
};


//	�A�E�g���C����̓_�C�A���O�{�b�N�X
class DlgFuncList : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFuncList();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless(HINSTANCE, HWND, LPARAM, FuncInfoArr*, int, int, OutlineType, OutlineType, bool); // ���[�h���X�_�C�A���O�̕\��
	void ChangeView(LPARAM);	// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
	bool IsDocking() { return eDockSide > DockSideType::Float; }
	DockSideType GetDockSide() { return eDockSide; }

protected:
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// �W���ȊO�̃��b�Z�[�W��ߑ�����

	CommonSetting_OutLine& CommonSet(void) { return pShareData->common.outline; }
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
	// ���݂̎�ʂƓ����Ȃ�
	bool CheckListType(OutlineType nOutLineType) const { return nOutLineType == nOutlineType; }
	void Redraw(OutlineType nOutLineType, OutlineType nListType, FuncInfoArr*, int nCurLine, int nCurCol);
	void Refresh(void);
	bool ChangeLayout(int nId);
	void OnOutlineNotify(WPARAM wParam, LPARAM lParam);
	void SyncColor(void);
	void SetWindowText(const TCHAR* szTitle);		// �_�C�A���O�^�C�g���̐ݒ�
	EFunctionCode GetFuncCodeRedraw(OutlineType outlineType);
	void LoadFileTreeSetting( FileTreeSetting&, SFilePath& );
	static void ReadFileTreeIni( DataProfile&, FileTreeSetting& );

protected:
	bool bInChangeLayout;

	FuncInfoArr*	pFuncInfoArr;	// �֐����z��
	size_t			nCurLine;		// ���ݍs
	size_t			nCurCol;		// ���݌�
	int				nSortCol;		// �\�[�g�����ԍ�
	int				nSortColOld;	// �\�[�g�����ԍ�(OLD)
	bool			bSortDesc;		// �~��
	NativeW			memClipText;	// �N���b�v�{�[�h�R�s�[�p�e�L�X�g
	bool			bLineNumIsCRLF;	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	OutlineType		nListType;		// �ꗗ�̎��
public:
	int				nDocType;		// �h�L�������g�̎��
	OutlineType		nOutlineType;	// �A�E�g���C����͂̎��
	bool			bEditWndReady;	// �G�f�B�^��ʂ̏�������
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnNotify(WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	BOOL OnDestroy(void);
	BOOL OnCbnSelChange(HWND hwndCtl, int wID);
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
	void SortTree(HWND hWndTree, HTREEITEM htiParent);	// �c���[�r���[�̍��ڂ��\�[�g����i�\�[�g���nSortType���g�p�j

	void SortListView(HWND hwndList, int sortcol);
	static int CALLBACK CompareFunc_Asc(LPARAM, LPARAM, LPARAM);
	static int CALLBACK CompareFunc_Desc(LPARAM, LPARAM, LPARAM);

	LPVOID GetHelpIdTable(void);
	void Key2Command(WORD);			//	�L�[���쁨�R�}���h�ϊ�
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
	int	nViewType;

	// Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 1/4
	// (������Ȃ̂łǂȂ����C�����肢���܂�)
	bool bWaitTreeProcess;

	// �c���[�r���[���\�[�g����
	// 0 �f�t�H���g(�m�[�h�Ɋ֘A�Â����ꂽ�l��)
	// 1 �A���t�@�x�b�g��
	int nSortType;

	// �I�𒆂̊֐����
	FuncInfo* funcInfo;
	std::tstring sJumpFile;

	const TCHAR* pszTimerJumpFile;
	Point	pointTimerJump;
	bool	bTimerJumpAutoClose;

	DockSideType	eDockSide;	// ���݂̉�ʂ̕\���ʒu
	HWND	hwndToolTip;		// �c�[���`�b�v�i�{�^���p�j
	bool	bStretching;
	bool	bHovering;
	int		nHilightedBtn;
	int		nCapturingBtn;
	
	TypeConfig type;
	FileTreeSetting	fileTreeSetting;

	static LPDLGTEMPLATE pDlgTemplate;
	static DWORD dwDlgTmpSize;
	static HINSTANCE lastRcInstance;		// ���\�[�X�����`�F�b�N�p

	POINT	ptDefaultSize;
	POINT	ptDefaultSizeClient;
	RECT	rcItems[12];
};

