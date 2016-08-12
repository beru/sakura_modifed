/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2002/01/14 YAZAKI PrintPreview�̕���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001-2002, YAZAKI
	Copyright (C) 2002, aroka, genta, MIK
	Copyright (C) 2003, MIK, genta, wmlhq
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#pragma once

#include <ShellAPI.h>// HDROP
#include "_main/global.h"
#include "MainToolBar.h"
#include "TabWnd.h"	//@@@ 2003.05.31 MIK
#include "func/FuncKeyWnd.h"
#include "MainStatusBar.h"
#include "view/EditView.h"
#include "window/SplitterWnd.h"
#include "dlg/DlgFind.h"
#include "dlg/DlgReplace.h"
#include "dlg/DlgJump.h"
#include "dlg/DlgGrep.h"
#include "dlg/DlgGrepReplace.h"
#include "dlg/DlgSetCharSet.h"
#include "outline/DlgFuncList.h"
#include "HokanMgr.h"
#include "util/design_template.h"
#include "doc/DocListener.h"
#include "uiparts/MenuDrawer.h"
#include "view/ViewFont.h"

static const size_t MENUBAR_MESSAGE_MAX_LEN = 30;

//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
class PrintPreview; // 2002/2/10 aroka
class DropTarget;
class Plug;
class EditDoc;
struct DllSharedData;


// ���C���E�B���h�E���R���g���[��ID
#define IDT_EDIT		455  // 20060128 aroka
#define IDT_TOOLBAR		456
#define IDT_CAPTION		457
#define IDT_FIRST_IDLE	458
#define IDT_SYSMENU		1357
#define ID_TOOLBAR		100

struct TabGroupInfo {
	HWND			hwndTop;
	WINDOWPLACEMENT	wpTop;

	TabGroupInfo() : hwndTop(NULL) { }
	bool IsValid() const { return hwndTop != NULL; }
};

// �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X
// 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
// 2007.10.30 kobake IsFuncEnable,IsFuncChecked��Funccode.h�Ɉړ�
// 2007.10.30 kobake OnHelp_MenuItem��CEditApp�Ɉړ�
class EditWnd :
	public TSingleton<EditWnd>,
	public DocListenerEx
{
	friend class TSingleton<EditWnd>;
	EditWnd();
	~EditWnd();

public:
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �쐬                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Mar. 7, 2002 genta �����^�C�v�p�����ǉ�
	// 2007.06.26 ryoji �O���[�v�w������ǉ�
	// �쐬
	HWND Create(
		EditDoc*		pEditDoc,
		ImageListMgr*	pIcons,
		int				nGroup
	);
	void _GetTabGroupInfo(TabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo);	// �E�B���h�E�����p�̋�`���擾
	HWND _CreateMainWindow(int nGroup, const TabGroupInfo& tabGroupInfo);
	void _AdjustInMonitor(const TabGroupInfo& tabGroupInfo);

	void OpenDocumentWhenStart(
		const LoadInfo& loadInfo		// [in]
	);

	void SetDocumentTypeWhenCreate(
		EncodingType	nCharCode,						// [in] �����R�[�h
		bool			bViewMode,							// [in] �r���[���[�h�ŊJ�����ǂ���
		TypeConfigNum	nDocumentType = TypeConfigNum(-1)	// [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
	);
	void UpdateCaption();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �h�L�������g�C�x���g
	void OnAfterSave(const SaveInfo& saveInfo);

	// �Ǘ�
	void MessageLoop(void);								// ���b�Z�[�W���[�v
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ���b�Z�[�W����

	// �e��C�x���g
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// �`�揈��
	LRESULT OnSize(WPARAM, LPARAM);	// WM_SIZE ����
	LRESULT OnSize2(WPARAM, LPARAM, bool);
	LRESULT OnLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDown(WPARAM, LPARAM);
	LRESULT OnMouseMove(WPARAM, LPARAM);
	LRESULT OnMouseWheel(WPARAM, LPARAM);
	bool DoMouseWheel(WPARAM wParam, LPARAM lParam);	// �}�E�X�z�C�[������	// 2007.10.16 ryoji
	LRESULT OnHScroll(WPARAM, LPARAM);
	LRESULT OnVScroll(WPARAM, LPARAM);
	int	OnClose(HWND hWndFrom, bool);	// �I�����̏���
	void OnDropFiles(HDROP);			// �t�@�C�����h���b�v���ꂽ
	bool OnPrintPageSetting(void);		// ����y�[�W�ݒ�
	LRESULT OnTimer(WPARAM, LPARAM);	// WM_TIMER ����	// 2007.04.03 ryoji
	void OnEditTimer(void);				// �^�C�}�[�̏���
	void OnCaptionTimer(void);
	void OnSysMenuTimer(void);
	void OnCommand(WORD, WORD , HWND);
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ʒm                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// �t�@�C�����ύX�ʒm
	void ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep);	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���j���[                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu(HMENU, UINT, BOOL);
	void InitMenu_Function(HMENU , EFunctionCode, const wchar_t*, const wchar_t*);
	bool InitMenu_Special(HMENU , EFunctionCode);
	void InitMenubarMessageFont(void);			//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��EditWnd���ڊ�	//	Dec. 4, 2002 genta
	LRESULT WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, size_t nRowNum, bool bFull);	// �E�B���h�E�ꗗ���j���[�쐬����		2006.03.23 fon
	LRESULT PopupWinList(bool bMousePos);		// �E�B���h�E�ꗗ�|�b�v�A�b�v�\������		2006.03.23 fon	// 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
	void RegisterPluginCommand();				// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
	void RegisterPluginCommand(int id);			// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
	void RegisterPluginCommand(Plug* id);		// �v���O�C���R�}���h���G�f�B�^�ɓo�^����

	void SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const wchar_t* sKey, bool flag);				// �\���̓��I�I��	2010/5/19 Uchi

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���`                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutMainMenu(void);		// ���C�����j���[					// 2010/5/16 Uchi
	void LayoutToolBar(void);		// �c�[���o�[�̔z�u����				// 2006.12.19 ryoji
	void LayoutFuncKey(void);		// �t�@���N�V�����L�[�̔z�u����		// 2006.12.19 ryoji
	void LayoutTabBar(void);		// �^�u�o�[�̔z�u����				// 2006.12.19 ryoji
	void LayoutStatusBar(void);		// �X�e�[�^�X�o�[�̔z�u����			// 2006.12.19 ryoji
	void LayoutMiniMap();			// �~�j�}�b�v�̔z�u����
	void EndLayoutBars(bool bAdjust = true);	// �o�[�̔z�u�I������		// 2006.12.19 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF(void);	// ���Preview���[�h�̃I��/�I�t
	
	// �A�C�R��
	void SetWindowIcon(HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel(bool bState) { bPageScrollByWheel = bState; }		// �z�C�[������ɂ��y�[�W�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.17 nasukoji
	void SetHScrollByWheel(bool bState) { bHorizontalScrollByWheel = bState; }	// �z�C�[������ɂ�鉡�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.17 nasukoji
	void ClearMouseState(void);		// 2009.01.17 nasukoji	�}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	bool IsActiveApp() const { return bIsActiveApp; }
	
	// �c�[���`�b�v�̃e�L�X�g���擾�B2007.09.08 kobake �ǉ�
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;
	
	// ���Preview�����ǂ���
	bool IsInPreviewMode() {
		return pPrintPreview != nullptr;
	}
	
	bool IsPageScrollByWheel() const { return bPageScrollByWheel; }		// �z�C�[������ɂ��y�[�W�X�N���[���L��	// 2009.01.17 nasukoji
	bool IsHScrollByWheel() const { return bHorizontalScrollByWheel; }	// �z�C�[������ɂ�鉡�X�N���[���L��		// 2009.01.17 nasukoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �\��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage(const TCHAR* msg);
	void SendStatusMessage(const TCHAR* msg);		//	Dec. 4, 2002 genta ���̂�EditView����ړ�
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �E�B���h�E����                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost(int); // 2004.09.21 Moca
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �r���[�Ǘ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane(int);	// �A�N�e�B�u�ȃy�C����ݒ�
	int GetActivePane(void) const { return nActivePaneIndex; }	// �A�N�e�B�u�ȃy�C�����擾		2007.08.26 kobake const�ǉ�
	bool SetDrawSwitchOfAllViews(bool bDraw);						// ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��	2008.06.08 ryoji
	void RedrawAllViews(EditView* pViewExclude);					// ���ׂẴy�C����Redraw����
	void Views_DisableSelectArea(bool bRedraw);
	bool DetectWidthOfLineNumberAreaAllPane(bool bRedraw);	// ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j
	bool WrapWindowWidth(int nPane);	// �E�[�Ő܂�Ԃ�			2008.06.08 ryoji
	bool UpdateTextWrap(void);		// �܂�Ԃ����@�֘A�̍X�V	2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB���Ɛ܂�Ԃ��ʒu�̍X�V
	void ChangeLayoutParam(bool bShowProgress, size_t nTabSize, size_t nMaxLineKetas);
	//	Aug. 14, 2005 genta
	PointEx* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView(PointEx* pptPosArray);
	// �݊�BMP�ɂ���ʃo�b�t�@ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); // EditView�̉�ʃo�b�t�@���폜
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �e��A�N�Z�T                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HWND			GetHwnd() const			{ return hWnd; }
	MenuDrawer&		GetMenuDrawer()			{ return menuDrawer; }
	EditDoc&		GetDocument()			{ return *pEditDoc; }
	const EditDoc&	GetDocument() const		{ return *pEditDoc; }

	// �r���[
	const EditView&		GetActiveView() const		{ return *pEditView; }
	EditView&			GetActiveView()				{ return *pEditView; }
	const EditView&		GetView(int n) const		{ return *pEditViewArr[n]; }
	EditView&			GetView(int n)				{ return *pEditViewArr[n]; }
	EditView&			GetMiniMap()				{ return *pEditViewMiniMap; }
	bool				IsEnablePane(int n) const	{ return 0 <= n && n < nEditViewCount; }
	int					GetAllViewCount() const		{ return nEditViewCount; }

	EditView*			GetDragSourceView() const	{ return pDragSourceView; }
	void				SetDragSourceView(EditView* pDragSourceView)	{ this->pDragSourceView = pDragSourceView; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// by �S
protected:
	enum class IconClickStatus {
		None,
		Down,
		Clicked,
		DoubleClicked,
	};

protected:
	// �h���b�v�_�E�����j���[
	int	CreateFileDropDownMenu(HWND);	// �J��(�h���b�v�_�E��)	//@@@ 2002.06.15 MIK

	// �^�C�}�[
	void Timer_ONOFF(bool); // �X�V�̊J�n�^��~ 20060128 aroka

	// ���j���[
	void CheckFreeSubMenu(HWND, HMENU, UINT);		// ���j���[�o�[�̖�����������	2010/6/18 Uchi
	void CheckFreeSubMenuSub(HMENU, int);			// ���j���[�o�[�̖�����������	2010/6/18 Uchi

//public:
	// ��������nTimerCount���C���N�������g
	void IncrementTimerCount(int nInterval) {
		++nTimerCount;
		if (nInterval <= nTimerCount) { // 2012.11.29 aroka �Ăяo���Ԋu�̃o�O�C��
			nTimerCount = 0;
		}
	}

	void CreateAccelTbl(void); // �E�B���h�E���̃A�N�Z�����[�^�e�[�u���쐬(Wine�p)
	void DeleteAccelTbl(void); // �E�B���h�E���̃A�N�Z�����[�^�e�[�u���j��(Wine�p)

public:
	// D&D�t���O�Ǘ�
	void SetDragPosOrg(Point ptDragPosOrg)	{ ptDragPosOrg = ptDragPosOrg; }
	void SetDragMode(bool bDragMode)		{ this->bDragMode = bDragMode; }
	bool GetDragMode() const				{ return bDragMode; }
	const Point& GetDragPosOrg() const		{ return ptDragPosOrg; }
	
	// IDropTarget����		2008.06.20 ryoji
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	
	// �t�H�[�J�X�Ǘ�
	int GetCurrentFocus() const { return nCurrentFocus; }
	void SetCurrentFocus(int n) { nCurrentFocus = n; }
	
	const LOGFONT& GetLogfont(bool bTempSetting = true);
	int GetFontPointSize(bool bTempSetting = true);
	CharWidthCacheMode GetLogfontCacheMode();

	void ClearViewCaretPosInfo();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// ���E�B���h�E
	HWND			hWnd;

	// �e�E�B���h�E
	HWND			hwndParent;

public:
	// �q�E�B���h�E
	MainToolBar		toolbar;		// �c�[���o�[
	TabWnd			tabWnd;			// �^�u�E�B���h�E	//@@@ 2003.05.31 MIK
	FuncKeyWnd		funcKeyWnd;		// �t�@���N�V�����o�[
	MainStatusBar	statusBar;		// �X�e�[�^�X�o�[
	PrintPreview*	pPrintPreview;	// ���Preview�\�����B�K�v�ɂȂ����Ƃ��̂݃C���X�^���X�𐶐�����B

	SplitterWnd		splitterWnd;		// �����t���[��
	EditView*		pDragSourceView;	// �h���b�O���̃r���[
	ViewFont*		pViewFont;			// �t�H���g
	ViewFont*		pViewFontMiniMap;	// �t�H���g

	// �_�C�A���O�B
	DlgFind			dlgFind;		//�u�����v�_�C�A���O
	DlgReplace		dlgReplace;		//�u�u���v�_�C�A���O
	DlgJump			dlgJump;		//�u�w��s�փW�����v�v�_�C�A���O
	DlgGrep			dlgGrep;		// Grep�_�C�A���O
	DlgGrepReplace	dlgGrepReplace;	// Grep�u���_�C�A���O
	DlgFuncList		dlgFuncList;	// �A�E�g���C����͌��ʃ_�C�A���O
	HokanMgr		hokanMgr;		// ���͕⊮
	DlgSetCharSet	dlgSetCharSet;	//�u�����R�[�h�Z�b�g�ݒ�v�_�C�A���O

private:
	// 2010.04.10 Moca  public -> private. �N�������[0]�̂ݗL�� 4�Ƃ͌���Ȃ��̂Œ���
	EditDoc* 		pEditDoc;
	EditView*		pEditViewArr[4];		// �r���[
	EditView*		pEditView;			// �L���ȃr���[
	EditView*		pEditViewMiniMap;		// �~�j�}�b�v
	int				nActivePaneIndex;		// �L���ȃr���[��index
	int				nEditViewCount;		// �L���ȃr���[�̐�
	const int		nEditViewMaxCount;	// �r���[�̍ő吔=4

	// ���L�f�[�^
	DllSharedData*	pShareData;

	// �w���p
	MenuDrawer		menuDrawer;

	// ���b�Z�[�WID
	UINT			uMSIMEReconvertMsg;
	UINT			uATOKReconvertMsg;

	// ���
	bool			bIsActiveApp;			// ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	LPTSTR			pszLastCaption;
	LPTSTR			pszMenubarMessage;		// ���j���[�o�[�E�[�ɕ\�����郁�b�Z�[�W
public:
	int				nTimerCount;			// OnTimer�p 2003.08.29 wmlhq
	PointEx*		posSaveAry;
private:
	int				nCurrentFocus;				// ���݂̃t�H�[�J�X���
	int				nWinSizeType;				// �T�C�Y�ύX�̃^�C�v�BSIZE_MAXIMIZED, SIZE_MINIMIZED ���B
	bool			bPageScrollByWheel;			// �z�C�[������ɂ��y�[�W�X�N���[������	// 2009.01.17 nasukoji
	bool			bHorizontalScrollByWheel;	// �z�C�[������ɂ�鉡�X�N���[������		// 2009.01.17 nasukoji
	HACCEL			hAccelWine;					// �E�B���h�E���̃A�N�Z�����[�^�e�[�u���̃n���h��(Wine�p)	// 2009.08.15 nasukoji
	HACCEL			hAccel;						// �A�N�Z�����[�^�e�[�u��(���L or �E�B���h�E��)

	// �t�H���g�E�C���[�W
	HFONT			hFontCaretPosInfo;			// �L�����b�g�̍s���ʒu�\���p�t�H���g
	int				nCaretPosInfoCharWidth;		// �L�����b�g�̍s���ʒu�\���p�t�H���g�̕�
	int				nCaretPosInfoCharHeight;	// �L�����b�g�̍s���ʒu�\���p�t�H���g�̍���

	// D&D�t���O
	bool			bDragMode;
	Point			ptDragPosOrg;
	DropTarget*		pDropTarget;

	// ���̑��t���O
	BOOL			bUIPI;		// �G�f�B�^�|�g���C�Ԃł�UI���������m�F�p�t���O	// 2007.06.07 ryoji
	IconClickStatus	iconClicked;

public:
	SelectCountMode	nSelectCountMode; // �I�𕶎��J�E���g���@

};

