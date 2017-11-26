#pragma once

#include <ShellAPI.h>// HDROP
#include "_main/global.h"
#include "MainToolBar.h"
#include "TabWnd.h"
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

class PrintPreview;
class DropTarget;
class Plug;
class EditDoc;
struct DllSharedData;


// ���C���E�B���h�E���R���g���[��ID
#define IDT_EDIT		455
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
	bool DoMouseWheel(WPARAM wParam, LPARAM lParam);	// �}�E�X�z�C�[������
	LRESULT OnHScroll(WPARAM, LPARAM);
	LRESULT OnVScroll(WPARAM, LPARAM);
	int	OnClose(HWND hWndFrom, bool);	// �I�����̏���
	void OnDropFiles(HDROP);			// �t�@�C�����h���b�v���ꂽ
	bool OnPrintPageSetting(void);		// ����y�[�W�ݒ�
	LRESULT OnTimer(WPARAM, LPARAM);	// WM_TIMER ����
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
	void ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���j���[                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu(HMENU, UINT, BOOL);
	void InitMenu_Function(HMENU , EFunctionCode, const wchar_t*, const wchar_t*);
	bool InitMenu_Special(HMENU , EFunctionCode);
	void InitMenubarMessageFont(void);			//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��EditWnd���ڊ�
	LRESULT WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, size_t nRowNum, bool bFull);	// �E�B���h�E�ꗗ���j���[�쐬����
	LRESULT PopupWinList(bool bMousePos);		// �E�B���h�E�ꗗ�|�b�v�A�b�v�\������
	void RegisterPluginCommand();				// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
	void RegisterPluginCommand(int id);			// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
	void RegisterPluginCommand(Plug* id);		// �v���O�C���R�}���h���G�f�B�^�ɓo�^����

	void SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const wchar_t* sKey, bool flag);				// �\���̓��I�I��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���`                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutMainMenu(void);		// ���C�����j���[
	void LayoutToolBar(void);		// �c�[���o�[�̔z�u����
	void LayoutFuncKey(void);		// �t�@���N�V�����L�[�̔z�u����
	void LayoutTabBar(void);		// �^�u�o�[�̔z�u����
	void LayoutStatusBar(void);		// �X�e�[�^�X�o�[�̔z�u����
	void LayoutMiniMap();			// �~�j�}�b�v�̔z�u����
	void EndLayoutBars(bool bAdjust = true);	// �o�[�̔z�u�I������


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF(void);	// ���Preview���[�h�̃I��/�I�t
	
	// �A�C�R��
	void SetWindowIcon(HICON, int);	// 
	void GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const;	// 
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	// 
	void SetPageScrollByWheel(bool bState) { bPageScrollByWheel = bState; }		// �z�C�[������ɂ��y�[�W�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j
	void SetHScrollByWheel(bool bState) { bHorizontalScrollByWheel = bState; }	// �z�C�[������ɂ�鉡�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j
	void ClearMouseState(void);		// �}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ���A�v�����A�N�e�B�u���ǂ���
	bool IsActiveApp() const { return bIsActiveApp; }
	
	// �c�[���`�b�v�̃e�L�X�g���擾
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;
	
	// ���Preview�����ǂ���
	bool IsInPreviewMode() {
		return pPrintPreview != nullptr;
	}
	
	bool IsPageScrollByWheel() const { return bPageScrollByWheel; }		// �z�C�[������ɂ��y�[�W�X�N���[���L��	
	bool IsHScrollByWheel() const { return bHorizontalScrollByWheel; }	// �z�C�[������ɂ�鉡�X�N���[���L��
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �\��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage(const TCHAR* msg);
	void SendStatusMessage(const TCHAR* msg);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �E�B���h�E����                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost(int);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �r���[�Ǘ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane(int);	// �A�N�e�B�u�ȃy�C����ݒ�
	int GetActivePane(void) const { return nActivePaneIndex; }	// �A�N�e�B�u�ȃy�C�����擾
	bool SetDrawSwitchOfAllViews(bool bDraw);						// ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��
	void RedrawAllViews(EditView* pViewExclude);					// ���ׂẴy�C����Redraw����
	void Views_DisableSelectArea(bool bRedraw);
	bool DetectWidthOfLineNumberAreaAllPane(bool bRedraw);	// ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j
	bool WrapWindowWidth(int nPane);	// �E�[�Ő܂�Ԃ�
	bool UpdateTextWrap(void);		// �܂�Ԃ����@�֘A�̍X�V
	void ChangeLayoutParam(bool bShowProgress, size_t nTabSize, size_t nMaxLineKetas);
	PointEx* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView(PointEx* pptPosArray);
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
protected:
	enum class IconClickStatus {
		None,
		Down,
		Clicked,
		DoubleClicked,
	};

protected:
	// �h���b�v�_�E�����j���[
	int	CreateFileDropDownMenu(HWND);	// �J��(�h���b�v�_�E��)

	// �^�C�}�[
	void Timer_ONOFF(bool); // �X�V�̊J�n�^��~

	// ���j���[
	void CheckFreeSubMenu(HWND, HMENU, UINT);		// ���j���[�o�[�̖�����������
	void CheckFreeSubMenuSub(HMENU, int);			// ���j���[�o�[�̖�����������

//public:
	// ��������nTimerCount���C���N�������g
	void IncrementTimerCount(int nInterval) {
		++nTimerCount;
		if (nInterval <= nTimerCount) {
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
	
	// IDropTarget����
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
	TabWnd			tabWnd;			// �^�u�E�B���h�E
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
	EditDoc* 		pEditDoc;
	// �N�������[0]�̂ݗL�� 4�Ƃ͌���Ȃ��̂Œ���
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
	bool			bIsActiveApp;			// ���A�v�����A�N�e�B�u���ǂ���
	LPTSTR			pszLastCaption;
	LPTSTR			pszMenubarMessage;		// ���j���[�o�[�E�[�ɕ\�����郁�b�Z�[�W
public:
	int				nTimerCount;			// OnTimer�p
	PointEx*		posSaveAry;
private:
	int				nCurrentFocus;				// ���݂̃t�H�[�J�X���
	int				nWinSizeType;				// �T�C�Y�ύX�̃^�C�v�BSIZE_MAXIMIZED, SIZE_MINIMIZED ���B
	bool			bPageScrollByWheel;			// �z�C�[������ɂ��y�[�W�X�N���[������
	bool			bHorizontalScrollByWheel;	// �z�C�[������ɂ�鉡�X�N���[������
	HACCEL			hAccelWine;					// �E�B���h�E���̃A�N�Z�����[�^�e�[�u���̃n���h��(Wine�p)
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
	BOOL			bUIPI;		// �G�f�B�^�|�g���C�Ԃł�UI���������m�F�p�t���O
	IconClickStatus	iconClicked;

public:
	SelectCountMode	nSelectCountMode; // �I�𕶎��J�E���g���@

};

