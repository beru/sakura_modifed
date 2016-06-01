/*!	@file
	@brief �A�E�g���C����̓_�C�A���O�{�b�N�X

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani Visual Basic�̃A�E�g���C�����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, JEPRO, genta, hor
	Copyright (C) 2002, MIK, aroka, hor, genta, YAZAKI, Moca, frozen
	Copyright (C) 2003, zenryaku, Moca, naoh, little YOSHI, genta,
	Copyright (C) 2004, zenryaku, Moca, novice
	Copyright (C) 2005, genta, zenryaku, ������, D.S.Koba
	Copyright (C) 2006, genta, aroka, ryoji, Moca
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <limits.h>
#include "outline/DlgFuncList.h"
#include "outline/FuncInfo.h"
#include "outline/FuncInfoArr.h"// 2002/2/3 aroka
#include "outline/DlgFileTree.h"
#include "window/EditWnd.h"	//	2006/2/11 aroka �ǉ�
#include "doc/EditDoc.h"
#include "uiparts/Graphics.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/input.h"
#include "util/window.h"
#include "env/AppNodeManager.h"
#include "env/DocTypeManager.h"
#include "env/FileNameManager.h"
#include "env/ShareData.h"
#include "env/ShareData_IO.h"
#include "extmodule/UxTheme.h"
#include "GrepEnumKeys.h"
#include "GrepEnumFilterFiles.h"
#include "GrepEnumFilterFolders.h"
#include "DataProfile.h"
#include "dlg/DlgTagJumpList.h"
#include "typeprop/ImpExpManager.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ��ʃh�b�L���O�p�̒�`	// 2010.06.05 ryoji
#define DEFINE_SYNCCOLOR
#define DOCK_SPLITTER_WIDTH		DpiScaleX(5)
#define DOCK_MIN_SIZE			DpiScaleX(60)
#define DOCK_BUTTON_NUM			(3)

// �r���[�̎��
#define VIEWTYPE_LIST	0
#define VIEWTYPE_TREE	1

// �A�E�g���C����� CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,					HIDC_FL_BUTTON_COPY,	// �R�s�[
	IDOK,								HIDOK_FL,				// �W�����v
	IDCANCEL,							HIDCANCEL_FL,			// �L�����Z��
	IDC_BUTTON_HELP,					HIDC_FL_BUTTON_HELP,	// �w���v
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	// �����I�ɕ���
	IDC_LIST_FL,						HIDC_FL_LIST1,			// �g�s�b�N���X�g	IDC_LIST1->IDC_LIST_FL	2008/7/3 Uchi
	IDC_TREE_FL,						HIDC_FL_TREE1,			// �g�s�b�N�c���[	IDC_TREE1->IDC_TREE_FL	2008/7/3 Uchi
	IDC_CHECK_bFunclistSetFocusOnJump,	HIDC_FL_CHECK_bFunclistSetFocusOnJump,	// �W�����v�Ńt�H�[�J�X�ړ�����
	IDC_CHECK_bMarkUpBlankLineEnable,	HIDC_FL_CHECK_bMarkUpBlankLineEnable,	// ��s�𖳎�����
	IDC_COMBO_nSortType,				HIDC_COMBO_nSortType,	// ����
	IDC_BUTTON_WINSIZE,					HIDC_FL_BUTTON_WINSIZE,	// �E�B���h�E�ʒu�ۑ�	// 2006.08.06 ryoji
	IDC_BUTTON_MENU,					HIDC_FL_BUTTON_MENU,	// �E�B���h�E�̈ʒu���j���[
	IDC_BUTTON_SETTING,					HIDC_FL_BUTTON_SETTING,	// �ݒ�
//	IDC_STATIC,							-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

static const AnchorListItem anchorList[] = {
	{IDC_BUTTON_COPY,		AnchorStyle::Bottom},
	{IDOK,					AnchorStyle::Bottom},
	{IDCANCEL,				AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,		AnchorStyle::Bottom},
	{IDC_CHECK_bAutoCloseDlgFuncList, AnchorStyle::Bottom},
	{IDC_LIST_FL,			AnchorStyle::All},
	{IDC_TREE_FL,			AnchorStyle::All},
	{IDC_CHECK_bFunclistSetFocusOnJump, AnchorStyle::Bottom},
	{IDC_CHECK_bMarkUpBlankLineEnable , AnchorStyle::Bottom},
	{IDC_COMBO_nSortType,	AnchorStyle::Top},
	{IDC_BUTTON_WINSIZE,	AnchorStyle::Bottom}, // 20060201 aroka
	{IDC_BUTTON_MENU,		AnchorStyle::Bottom},
};

// �֐����X�g�̗�
enum EFuncListCol {
	FL_COL_ROW		= 0,	// �s
	FL_COL_COL		= 1,	// ��
	FL_COL_NAME		= 2,	// �֐���
	FL_COL_REMARK	= 3		// ���l
};

// �\�[�g��r�p�v���V�[�W��
int CALLBACK DlgFuncList::CompareFunc_Asc(
	LPARAM lParam1,
	LPARAM lParam2,
	LPARAM lParamSort
	)
{
	DlgFuncList* pDlgFuncList = (DlgFuncList*)lParamSort;

	FuncInfo* pFuncInfo1 = pDlgFuncList->pFuncInfoArr->GetAt(lParam1);
	if (!pFuncInfo1) {
		return -1;
	}
	FuncInfo* pFuncInfo2 = pDlgFuncList->pFuncInfoArr->GetAt(lParam2);
	if (!pFuncInfo2) {
		return -1;
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if (pDlgFuncList->nSortCol == FL_COL_NAME) {	// ���O�Ń\�[�g
		return auto_stricmp(pFuncInfo1->memFuncName.GetStringPtr(), pFuncInfo2->memFuncName.GetStringPtr());
	}
	//	Apr. 23, 2005 genta �s�ԍ������[��
	if (pDlgFuncList->nSortCol == FL_COL_ROW) {	// �s�i�{���j�Ń\�[�g
		if (pFuncInfo1->nFuncLineCRLF < pFuncInfo2->nFuncLineCRLF) {
			return -1;
		}else
		if (pFuncInfo1->nFuncLineCRLF == pFuncInfo2->nFuncLineCRLF) {
			if (pFuncInfo1->nFuncColCRLF < pFuncInfo2->nFuncColCRLF) {
				return -1;
			}else
			if (pFuncInfo1->nFuncColCRLF == pFuncInfo2->nFuncColCRLF) {
				return 0;
			}else {
				return 1;
			}
		}else {
			return 1;
		}
	}
	if (pDlgFuncList->nSortCol == FL_COL_COL) {	// ���Ń\�[�g
		if (pFuncInfo1->nFuncColCRLF < pFuncInfo2->nFuncColCRLF) {
			return -1;
		}else
		if (pFuncInfo1->nFuncColCRLF == pFuncInfo2->nFuncColCRLF) {
			return 0;
		}else {
			return 1;
		}
	}
	// From Here 2001.12.07 hor
	if (pDlgFuncList->nSortCol == FL_COL_REMARK) {	// ���l�Ń\�[�g
		if (pFuncInfo1->nInfo < pFuncInfo2->nInfo) {
			return -1;
		}else
		if (pFuncInfo1->nInfo == pFuncInfo2->nInfo) {
			return 0;
		}else {
			return 1;
		}
	}
	// To Here 2001.12.07 hor
	return -1;
}

int CALLBACK DlgFuncList::CompareFunc_Desc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return -1 * CompareFunc_Asc(lParam1, lParam2, lParamSort);
}

EFunctionCode DlgFuncList::GetFuncCodeRedraw(OutlineType outlineType)
{
	if (outlineType == OutlineType::BookMark) {
		return F_BOOKMARK_VIEW;
	}else if (outlineType == OutlineType::FileTree) {
		return F_FILETREE;
	}
	return F_OUTLINE;
}

static
OutlineType GetOutlineTypeRedraw(OutlineType outlineType)
{
	if (outlineType == OutlineType::BookMark) {
		return OutlineType::BookMark;
	}else if (outlineType == OutlineType::FileTree) {
		return OutlineType::FileTree;
	}
	return OutlineType::Default;
}

LPDLGTEMPLATE DlgFuncList::pDlgTemplate = NULL;
DWORD DlgFuncList::dwDlgTmpSize = 0;
HINSTANCE DlgFuncList::lastRcInstance = 0;

DlgFuncList::DlgFuncList() : Dialog(true)
{
	// �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[����
	assert(_countof(anchorList) == _countof(rcItems));

	pFuncInfoArr = NULL;		// �֐����z��
	nCurLine = 0;	// ���ݍs
	nOutlineType = OutlineType::Default;
	nListType = OutlineType::Default;
	//	Apr. 23, 2005 genta �s�ԍ������[��
	nSortCol = 0;				// �\�[�g�����ԍ� 2004.04.06 zenryaku �W���͍s�ԍ�(1���)
	nSortColOld = -1;
	bLineNumIsCRLF = false;		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	bWaitTreeProcess = false;	// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 2/4
	nSortType = 0;
	funcInfo = NULL;			// ���݂̊֐����
	bEditWndReady = false;		// �G�f�B�^��ʂ̏�������
	bInChangeLayout = false;
	pszTimerJumpFile = NULL;
	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
}


/*!
	�W���ȊO�̃��b�Z�[�W��ߑ�����

	@date 2007.11.07 ryoji �V�K
*/
INT_PTR DlgFuncList::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);

	switch (wMsg) {
	case WM_ACTIVATEAPP:
		if (IsDocking())
			break;

		// �������ŏ��ɃA�N�e�B�u�����ꂽ�ꍇ�͈�U�ҏW�E�B���h�E���A�N�e�B�u�����Ė߂�
		//
		// Note. ���̃_�C�A���O�͑��Ƃ͈قȂ�E�B���h�E�X�^�C���̂��ߕ����Ƃ��̋������قȂ�D
		// ���̓X���b�h���ŋ߃A�N�e�B�u�ȃE�B���h�E���A�N�e�B�u�ɂȂ邪�C���̃_�C�A���O�ł�
		// �Z�b�V�������S�̂ł̍ŋ߃A�N�e�B�u�E�B���h�E���A�N�e�B�u�ɂȂ��Ă��܂��D
		// ����ł͓s���������̂ŁC���ʂɈȉ��̏������s���đ��Ɠ��l�ȋ�����������悤�ɂ���D
		if ((BOOL)wParam) {
			EditView* pEditView = (EditView*)(this->lParam);
			auto& editWnd = pEditView->editWnd;
			if (::GetActiveWindow() == GetHwnd()) {
				::SetActiveWindow(editWnd.GetHwnd());
				BlockingHook(NULL);	// �L���[���ɗ��܂��Ă��郁�b�Z�[�W������
				::SetActiveWindow(GetHwnd());
				return 0L;
			}
		}
		break;

	case WM_NCPAINT:
		return OnNcPaint(hWnd, wMsg, wParam, lParam);
	case WM_NCCALCSIZE:
		return OnNcCalcSize(hWnd, wMsg, wParam, lParam);
	case WM_NCHITTEST:
		return OnNcHitTest(hWnd, wMsg, wParam, lParam);
	case WM_NCMOUSEMOVE:
		return OnNcMouseMove(hWnd, wMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, wMsg, wParam, lParam);
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(hWnd, wMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(hWnd, wMsg, wParam, lParam);
	case WM_NCRBUTTONUP:
		if (IsDocking() && wParam == HTCAPTION) {
			// �h�b�L���O�̂Ƃ��̓R���e�L�X�g���j���[�𖾎��I�ɌĂяo���K�v������炵��
			::SendMessage(GetHwnd(), WM_CONTEXTMENU, (WPARAM)GetHwnd(), lParam);
			return 1L;
		}
		break;
	case WM_TIMER:
		return OnTimer(hWnd, wMsg, wParam, lParam);
	case WM_GETMINMAXINFO:
		return OnMinMaxInfo(lParam);
	case WM_SETTEXT:
		if (IsDocking()) {
			// �L���v�V�������ĕ`�悷��
			// �� ���̎��_�ł͂܂��e�L�X�g�ݒ肳��Ă��Ȃ��̂� RDW_UPDATENOW �ł� NG
			::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_NOINTERNALPAINT);
		}
		break;
	case WM_MOUSEACTIVATE:
		if (IsDocking()) {
			// �����o�[�ȊO�̏ꏊ�Ȃ�t�H�[�J�X�ړ�
			if (!(HTLEFT <= LOWORD(lParam) && LOWORD(lParam) <= HTBOTTOMRIGHT)) {
				::SetFocus(GetHwnd());
			}
		}
		break;
	case WM_COMMAND:
		if (IsDocking()) {
			// �R���{�{�b�N�X�̃t�H�[�J�X���ω�������L���v�V�������ĕ`�悷��i�A�N�e�B�u�^��A�N�e�B�u�ؑցj
			if (LOWORD(wParam) == IDC_COMBO_nSortType) {
				if (HIWORD(wParam) == CBN_SETFOCUS || HIWORD(wParam) == CBN_KILLFOCUS) {
					::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);
				}
			}
		}
		break;
	case WM_NOTIFY:
		if (IsDocking()) {
			// �c���[�⃊�X�g�̃t�H�[�J�X���ω�������L���v�V�������ĕ`�悷��i�A�N�e�B�u�^��A�N�e�B�u�ؑցj
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if (pNMHDR->code == NM_SETFOCUS || pNMHDR->code == NM_KILLFOCUS) {
				::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);
			}
		}
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if (pNMHDR->code == TVN_ITEMEXPANDING) {
				NMTREEVIEW* pNMTREEVIEW = (NMTREEVIEW*)lParam;
				TVITEM* pItem = &(pNMTREEVIEW->itemNew);
				if (nListType == OutlineType::FileTree) {
					SetTreeFileSub( pItem->hItem, NULL );
				}
			}
		}
		break;
	}

	return result;
}


// ���[�h���X�_�C�A���O�̕\��
/*
 * @note 2011.06.25 syat nOutlineType��ǉ�
 *   nOutlineType��nListType�͂قƂ�ǂ̏ꍇ�����l�����A�v���O�C���̏ꍇ�͗�O�ŁA
 *   nOutlineType�̓A�E�g���C����͂�ID�AnListType�̓v���O�C�����Ŏw�肷�郊�X�g�`���ƂȂ�B
 */
HWND DlgFuncList::DoModeless(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	FuncInfoArr*	pFuncInfoArr,
	int				nCurLine,
	int				nCurCol,
	OutlineType		nOutlineType,		
	OutlineType		nListType,
	bool			bLineNumIsCRLF		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	)
{
	EditView* pEditView = (EditView*)lParam;
	if (!pEditView) {
		return NULL;
	}
	this->pFuncInfoArr = pFuncInfoArr;	// �֐����z��
	this->nCurLine = nCurLine;			// ���ݍs
	this->nCurCol = nCurCol;				// ���݌�
	this->nOutlineType = nOutlineType;	// �A�E�g���C����͂̎��
	this->nListType = nListType;			// �ꗗ�̎��
	this->bLineNumIsCRLF = bLineNumIsCRLF;	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	nDocType = pEditView->GetDocument().docType.GetDocumentType().GetIndex();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
	nSortCol = type.nOutlineSortCol;
	nSortColOld = nSortCol;
	bSortDesc = type.bOutlineSortDesc;
	nSortType = type.nOutlineSortType;

	bool bType = (ProfDockSet() != 0);
	if (bType) {
		type.nDockOutline = nOutlineType;
		SetTypeConfig(TypeConfigNum(nDocType), type);
	}else {
		CommonSet().nDockOutline = nOutlineType;
	}

	// 2007.04.18 genta : �u�t�H�[�J�X���ڂ��v�Ɓu�����I�ɕ���v���`�F�b�N����Ă���ꍇ��
	// �_�u���N���b�N���s���ƁCtrue�̂܂܎c���Ă��܂��̂ŁC�E�B���h�E���J�����Ƃ��Ƀ��Z�b�g����D
	bWaitTreeProcess = false;

	eDockSide = ProfDockSide();
	HWND hwndRet;
	if (IsDocking()) {
		// �h�b�L���O�p�Ƀ_�C�A���O�e���v���[�g�Ɏ�������Ă���\������iWS_CHILD���j
		HINSTANCE hInstance2 = SelectLang::getLangRsrcInstance();
		if (!pDlgTemplate || lastRcInstance != hInstance2) {
			HRSRC hResInfo = ::FindResource(hInstance2, MAKEINTRESOURCE(IDD_FUNCLIST), RT_DIALOG);
			if (!hResInfo) return NULL;
			HGLOBAL hResData = ::LoadResource(hInstance2, hResInfo);
			if (!hResData) return NULL;
			pDlgTemplate = (LPDLGTEMPLATE)::LockResource(hResData);
			if (!pDlgTemplate) return NULL;
			dwDlgTmpSize = ::SizeofResource(hInstance2, hResInfo);
			// ����؂�ւ��Ń��\�[�X���A�����[�h����Ă��Ȃ����m�F���邽�߃C���X�^���X���L������
			lastRcInstance = hInstance2;
		}
		LPDLGTEMPLATE pDlgTemplate = (LPDLGTEMPLATE)::GlobalAlloc(GMEM_FIXED, dwDlgTmpSize);
		if (!pDlgTemplate) return NULL;
		::CopyMemory(pDlgTemplate, pDlgTemplate, dwDlgTmpSize);
		pDlgTemplate->style = (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_SETFONT);
		hwndRet = Dialog::DoModeless(hInstance, MyGetAncestor(hwndParent, GA_ROOT), pDlgTemplate, lParam, SW_HIDE);
		::GlobalFree(pDlgTemplate);
		pEditView->editWnd.EndLayoutBars(bEditWndReady);	// ��ʂ̍ă��C�A�E�g
	}else {
		hwndRet = Dialog::DoModeless(
			hInstance,
			MyGetAncestor(hwndParent, GA_ROOT),
			IDD_FUNCLIST,
			lParam,
			SW_SHOW
		);
	}
	return hwndRet;
}

// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
void DlgFuncList::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgFuncList::SetData()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_FL);
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	// 2002.02.08 hor �B���Ƃ��ăA�C�e���폜�����Ƃŕ\��
	::ShowWindow(hwndList, SW_HIDE);
	::ShowWindow(hwndTree, SW_HIDE);
	ListView_DeleteAllItems(hwndList);
	TreeView_DeleteAllItems(hwndTree);
	::ShowWindow(GetItemHwnd(IDC_BUTTON_SETTING), SW_HIDE);

	SetDocLineFuncList();
	
	switch (nListType) {
	case OutlineType::CPP:	// C++���\�b�h���X�g
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), true);	// Jan. 04, 2002 genta Java Method Tree�ɓ���
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_CPP));
		break;
	case OutlineType::RuleFile:	//@@@ 2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C������
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_RULE));
		break;
	case OutlineType::WZText: //@@@ 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_WZ)); //	2003.06.22 Moca ���O�ύX
		break;
	case OutlineType::HTML: //@@@ 2003.05.20 zenryaku HTML�A�E�g���C�����
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T("HTML"));
		break;
	case OutlineType::TeX: //@@@ 2003.07.20 naoh TeX�A�E�g���C�����
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T("TeX"));
		break;
	case OutlineType::Text: // �e�L�X�g�E�g�s�b�N���X�g
		nViewType = VIEWTYPE_TREE;
		SetTree();	//@@@ 2002.04.01 YAZAKI �e�L�X�g�g�s�b�N�c���[���A�ėpSetTree���ĂԂ悤�ɕύX�B
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_TEXT));
		break;
	case OutlineType::Java: // Java���\�b�h�c���[
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), true);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_JAVA));
		break;
	//	2007.02.08 genta Python�ǉ�
	case OutlineType::Python: // Python ���\�b�h�c���[
		nViewType = VIEWTYPE_TREE;
		SetTree(true);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_PYTHON));
		break;
	case OutlineType::Cobol: // COBOL �A�E�g���C��
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), false);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_COBOL));
		break;
	case OutlineType::VisualBasic:	// VisualBasic �A�E�g���C��
		nViewType = VIEWTYPE_LIST;
		SetListVB();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_VB));
		break;
	case OutlineType::FileTree:
		nViewType = VIEWTYPE_TREE;
		SetTreeFile();
		::SetWindowText( GetHwnd(), LS(F_FILETREE) );	// �t�@�C���c���[
		break;
	case OutlineType::Tree: // �ėp�c���[
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T(""));
		break;
	case OutlineType::TreeTagJump: // �ėp�c���[(�^�O�W�����v�t��)
		nViewType = VIEWTYPE_TREE;
		SetTree(true);
		::SetWindowText(GetHwnd(), _T(""));
		break;
	case OutlineType::ClassTree: // �ėp�N���X�c���[
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), true);
		::SetWindowText(GetHwnd(), _T(""));
		break;
	default:
		nViewType = VIEWTYPE_LIST;
		switch (nListType) {
		case OutlineType::C:
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_C));
			break;
		case OutlineType::PLSQL:
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_PLSQL));
			break;
		case OutlineType::Asm:
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_ASM));
			break;
		case OutlineType::Perl:	//	Sep. 8, 2000 genta
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_PERL));
			break;
// Jul 10, 2003  little YOSHI  ��Ɉړ����܂���--->>
//		case OutlineType::VisualBasic:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText(GetHwnd(), "Visual Basic �A�E�g���C��");
//			break;
// <<---�����܂�
		case OutlineType::Erlang:	//	2009.08.10 genta
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_ERLANG));
			break;
		case OutlineType::BookMark:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_TEXT));
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta �s�ԍ������[��
			ListView_SetColumn(hwndList, FL_COL_NAME, &col);
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_BOOK));
			break;
		case OutlineType::List:	// �ėp���X�g 2010.03.28 syat
			::SetWindowText(GetHwnd(), _T(""));
			break;
		}
		//	May 18, 2001 genta
		//	Window�����Ȃ��Ȃ�ƌ�œs���������̂ŁA�\�����Ȃ������ɂ��Ă���
		//::DestroyWindow(hwndTree);
//		::ShowWindow(hwndTree, SW_HIDE);
		TCHAR			szText[2048];
		const FuncInfo*	pFuncInfo;
		LV_ITEM item;
		bool bSelected;
		size_t nFuncLineOld(INT_MAX);
		size_t nFuncColOld(INT_MAX);
		size_t nFuncLineTop(INT_MAX);
		size_t nFuncColTop(INT_MAX);
		size_t nSelectedLineTop = 0;
		size_t nSelectedLine = 0;
		RECT rc;

		memClipText.SetString(L"");	// �N���b�v�{�[�h�R�s�[�p�e�L�X�g
		{
			const size_t nBuffLenTag = 13 + wcslen(to_wchar(pFuncInfoArr->szFilePath));
			const size_t nNum = pFuncInfoArr->GetNum();
			size_t nBuffLen = 0;
			for (size_t i=0; i<nNum; ++i) {
				const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
				nBuffLen += pFuncInfo->memFuncName.GetStringLength();
			}
			memClipText.AllocStringBuffer(nBuffLen + nBuffLenTag * nNum);
		}

		EnableItem(IDC_BUTTON_COPY, TRUE);
		bSelected = false;
		for (size_t i=0; i<pFuncInfoArr->GetNum(); ++i) {
			pFuncInfo = pFuncInfoArr->GetAt(i);
			if (!bSelected) {
				if (pFuncInfo->nFuncLineLAYOUT < nFuncLineTop
					|| (pFuncInfo->nFuncLineLAYOUT == nFuncLineTop && pFuncInfo->nFuncColLAYOUT <= nFuncColTop)
				) {
					nFuncLineTop = pFuncInfo->nFuncLineLAYOUT;
					nFuncColTop = pFuncInfo->nFuncColLAYOUT;
					nSelectedLineTop = i;
				}
			}
			{
				if (
					(0
						|| nFuncLineOld < pFuncInfo->nFuncLineLAYOUT
						|| (nFuncLineOld == pFuncInfo->nFuncColLAYOUT && nFuncColOld <= pFuncInfo->nFuncColLAYOUT)
					)
					&& (0
						|| pFuncInfo->nFuncLineLAYOUT < nCurLine
						|| (pFuncInfo->nFuncLineLAYOUT == nCurLine && pFuncInfo->nFuncColLAYOUT <= nCurCol)
					)
				) {
					nFuncLineOld = pFuncInfo->nFuncLineLAYOUT;
					nFuncColOld = pFuncInfo->nFuncColLAYOUT;
					bSelected = true;
					nSelectedLine = i;
				}
			}
		}
		if (0 < pFuncInfoArr->GetNum() && !bSelected) {
			bSelected = true;
			nSelectedLine = nSelectedLineTop;
		}
		for (size_t i=0; i<pFuncInfoArr->GetNum(); ++i) {
			// ���݂̉�͌��ʗv�f
			pFuncInfo = pFuncInfoArr->GetAt(i);

			//	From Here Apr. 23, 2005 genta �s�ԍ������[��
			// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
			if (bLineNumIsCRLF) {
				auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncLineCRLF);
			}else {
				auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncLineLAYOUT);
			}
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = szText;
			item.iItem = (int)i;
			item.lParam	= i;
			item.iSubItem = FL_COL_ROW;
			ListView_InsertItem(hwndList, &item);

			// 2010.03.17 syat ���ǉ�
			// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
			if (bLineNumIsCRLF) {
				auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncColCRLF);
			}else {
				auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncColLAYOUT);
			}
			item.mask = LVIF_TEXT;
			item.pszText = szText;
			item.iItem = (int)i;
			item.iSubItem = FL_COL_COL;
			ListView_SetItem(hwndList, &item);

			item.mask = LVIF_TEXT;
			item.pszText = const_cast<TCHAR*>(pFuncInfo->memFuncName.GetStringPtr());
			item.iItem = (int)i;
			item.iSubItem = FL_COL_NAME;
			ListView_SetItem(hwndList, &item);
			//	To Here Apr. 23, 2005 genta �s�ԍ������[��

			item.mask = LVIF_TEXT;
			if (pFuncInfo->nInfo == 1) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK01));}else
			if (pFuncInfo->nInfo == 10) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK02));}else
			if (pFuncInfo->nInfo == 20) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK03));}else
			if (pFuncInfo->nInfo == 11) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK04));}else
			if (pFuncInfo->nInfo == 21) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK05));}else
			if (pFuncInfo->nInfo == 31) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK06));}else
			if (pFuncInfo->nInfo == 41) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK07));}else
			if (pFuncInfo->nInfo == 50) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK08));}else
			if (pFuncInfo->nInfo == 51) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK09));}else
			if (pFuncInfo->nInfo == 52) {item.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_REMARK10));}else {
				// Jul 10, 2003  little YOSHI
				// �����ɂ�����VB�֌W�̏�����SetListVB()���\�b�h�Ɉړ����܂����B

				item.pszText = const_cast<TCHAR*>(_T(""));
			}
			item.iItem = (int)i;
			item.iSubItem = FL_COL_REMARK;
			ListView_SetItem(hwndList, &item);

			// �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW
			if (item.pszText[0] != _T('\0')) {
				// ���o���ʂ̎��(�֐�,,,)������Ƃ�
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					pFuncInfoArr->szFilePath.c_str(),	// ��͑Ώۃt�@�C����
					pFuncInfo->nFuncLineCRLF,			// ���o�s�ԍ�
					pFuncInfo->nFuncColCRLF				// ���o���ԍ�
				);
				memClipText.AppendStringT(szText);
				// "%ts(%ts)\r\n"
				memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
				memClipText.AppendStringLiteral(L"(");
				memClipText.AppendStringT(item.pszText);
				memClipText.AppendStringLiteral(L")\r\n");
			}else {
				// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					pFuncInfoArr->szFilePath.c_str(),	// ��͑Ώۃt�@�C����
					pFuncInfo->nFuncLineCRLF,			// ���o�s�ԍ�
					pFuncInfo->nFuncColCRLF				// ���o���ԍ�
				);
				memClipText.AppendStringT(szText);
				memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
				memClipText.AppendStringLiteral(L"\r\n");
			}
		}
		// 2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
		::ShowWindow(hwndList, SW_SHOW);
		// ��̕����f�[�^�ɍ��킹�Ē���
		ListView_SetColumnWidth(hwndList, FL_COL_ROW, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_COL, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_NAME, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_ROW, ListView_GetColumnWidth(hwndList, FL_COL_ROW) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_COL, ListView_GetColumnWidth(hwndList, FL_COL_COL) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_NAME, ListView_GetColumnWidth(hwndList, FL_COL_NAME) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_REMARK, ListView_GetColumnWidth(hwndList, FL_COL_REMARK) + 16);

		// 2005.07.05 ������
		DWORD dwExStyle  = ListView_GetExtendedListViewStyle(hwndList);
		dwExStyle |= LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle(hwndList, dwExStyle);

		if (bSelected) {
			ListView_GetItemRect(hwndList, 0, &rc, LVIR_BOUNDS);
			ListView_Scroll(hwndList, 0, nSelectedLine * (rc.bottom - rc.top));
			ListView_SetItemState(hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}
	// �A�E�g���C�� �_�C�A���O�������I�ɕ���
	CheckButton(IDC_CHECK_bAutoCloseDlgFuncList, pShareData->common.outline.bAutoCloseDlgFuncList);
	// �A�E�g���C�� �u�b�N�}�[�N�ꗗ�ŋ�s�𖳎�����
	CheckButton(IDC_CHECK_bMarkUpBlankLineEnable, pShareData->common.outline.bMarkUpBlankLineEnable);
	// �A�E�g���C�� �W�����v������t�H�[�J�X���ڂ�
	CheckButton(IDC_CHECK_bFunclistSetFocusOnJump, pShareData->common.outline.bFunclistSetFocusOnJump);

	// �A�E�g���C�� ���ʒu�ƃT�C�Y���L������ // 20060201 aroka
	CheckButton(IDC_BUTTON_WINSIZE, pShareData->common.outline.bRememberOutlineWindowPos);
	// �{�^����������Ă��邩�͂����肳���� 2008/6/5 Uchi
	SetItemText(IDC_BUTTON_WINSIZE, 
		pShareData->common.outline.bRememberOutlineWindowPos ? _T("��") : _T("��"));

	// �_�C�A���O�������I�ɕ���Ȃ�t�H�[�J�X�ړ��I�v�V�����͊֌W�Ȃ�
	EnableItem(IDC_CHECK_bFunclistSetFocusOnJump, !pShareData->common.outline.bAutoCloseDlgFuncList);

	// 2002.02.08 hor
	//�iIDC_LIST_FL��IDC_TREE_FL����ɑ��݂��Ă��āAnViewType�ɂ���āA�ǂ����\�����邩��I��ł���j
	HWND hwndShow = (nViewType == VIEWTYPE_LIST)? hwndList: hwndTree;
	::ShowWindow(hwndShow, SW_SHOW);
	if (::GetForegroundWindow() == MyGetAncestor(GetHwnd(), GA_ROOT) && IsChild(GetHwnd(), GetFocus())) {
		::SetFocus(hwndShow);
	}

	// 2002.02.08 hor
	// ��s���ǂ��������̃`�F�b�N�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̂Ƃ������\������
	if (nListType == OutlineType::BookMark) {
		EnableItem(IDC_CHECK_bMarkUpBlankLineEnable, true);
		if (!IsDocking()) {
			::ShowWindow(GetItemHwnd(IDC_CHECK_bMarkUpBlankLineEnable), SW_SHOW);
		}
	}else {
		::ShowWindow(GetItemHwnd(IDC_CHECK_bMarkUpBlankLineEnable), SW_HIDE);
		EnableItem(IDC_CHECK_bMarkUpBlankLineEnable, false);
	}
	// 2002/11/1 frozen ���ڂ̃\�[�g���ݒ肷��R���{�{�b�N�X�̓u�b�N�}�[�N�ꗗ�̈ȊO�̎��ɕ\������
	// Nov. 5, 2002 genta �c���[�\���̎������\�[�g��R���{�{�b�N�X��\��
	EditView* pEditView = (EditView*)(this->lParam);
	int nDocType = pEditView->GetDocument().docType.GetDocumentType().GetIndex();
	if (this->nDocType != nDocType) {
		// �ȑO�Ƃ̓h�L�������g�^�C�v���ς�����̂ŏ���������
		this->nDocType = nDocType;
		nSortCol = type.nOutlineSortCol;
		nSortColOld = nSortCol;
		bSortDesc = type.bOutlineSortDesc;
		nSortType = type.nOutlineSortType;
	}
	if (nViewType == VIEWTYPE_TREE && nListType != OutlineType::FileTree) {
		HWND hWnd_Combo_Sort = GetItemHwnd(IDC_COMBO_nSortType);
		if (nListType == OutlineType::FileTree) {
			::EnableWindow(hWnd_Combo_Sort, FALSE);
		}else {
			::EnableWindow(hWnd_Combo_Sort, TRUE);
		}
		::ShowWindow(hWnd_Combo_Sort , SW_SHOW);
		Combo_ResetContent(hWnd_Combo_Sort); // 2002.11.10 Moca �ǉ�
		Combo_AddString(hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE1));
		Combo_AddString(hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE2));
		Combo_SetCurSel(hWnd_Combo_Sort , nSortType);
		::ShowWindow(GetItemHwnd(IDC_STATIC_nSortType), SW_SHOW);
		// 2002.11.10 Moca �ǉ� �\�[�g����
		if (nSortType == 1) {
			SortTree(::GetDlgItem(GetHwnd() , IDC_TREE_FL), TVI_ROOT);
		}
	}else if (nListType == OutlineType::FileTree) {
		::ShowWindow(GetItemHwnd(IDC_COMBO_nSortType), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_STATIC_nSortType), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_SETTING), SW_SHOW);
	}else {
		EnableItem(IDC_COMBO_nSortType, false);
		::ShowWindow(GetItemHwnd(IDC_COMBO_nSortType), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_STATIC_nSortType), SW_HIDE);
		//ListView_SortItems(hwndList, CompareFunc_Asc, (LPARAM)this);  // 2005.04.05 zenryaku �\�[�g��Ԃ�ێ�
		SortListView(hwndList, nSortCol);	// 2005.04.23 genta �֐���(�w�b�_���������̂���)
	}
}


bool DlgFuncList::GetTreeFileFullName(
	HWND hwndTree,
	HTREEITEM target,
	std::tstring* pPath,
	int* pnItem
	)
{
	*pPath = _T("");
	*pnItem = -1;
	do {
		TVITEM tvItem;
		TCHAR szFileName[_MAX_PATH];
		tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
		tvItem.pszText = szFileName;
		tvItem.cchTextMax = _countof(szFileName);
		tvItem.hItem = target;
		TreeView_GetItem( hwndTree, &tvItem );
		if (((-tvItem.lParam) % 10) == 3) {
			*pnItem = (int)(-tvItem.lParam) / 10;
			*pPath = std::tstring(pFuncInfoArr->GetAt(*pnItem)->memFileName.GetStringPtr()) + _T("\\") + *pPath;
			return true;
		}
		if (tvItem.lParam != -1 && tvItem.lParam != -2) {
			return false;
		}
		if (*pPath != _T("")) {
			*pPath = std::tstring(szFileName) + _T("\\") + *pPath;
		}else {
			*pPath = szFileName;
		}
		target = TreeView_GetParent( hwndTree, target );
	}while (target);
	return false;
}


// �_�C�A���O�f�[�^�̎擾
// 0==����������   0���傫��==����   0��菬����==���̓G���[
int DlgFuncList::GetData(void)
{
	funcInfo = nullptr;
	sJumpFile = _T("");
	HWND hwndList = GetItemHwnd(IDC_LIST_FL);
	if (nViewType == VIEWTYPE_LIST) {
		//	List
 		int nItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
		if (nItem == -1) {
			return -1;
		}
		LV_ITEM item;
		item.mask = LVIF_PARAM;
		item.iItem = nItem;
		item.iSubItem = 0;
		ListView_GetItem(hwndList, &item);
		funcInfo = pFuncInfoArr->GetAt(item.lParam);
	}else {
		HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
		if (hwndTree) {
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);

			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			tvi.pszText = NULL;
			tvi.cchTextMax = 0;
			if (TreeView_GetItem(hwndTree, &tvi)) {
				// lParam��-1�ȉ��� pFuncInfoArr�ɂ͊܂܂�Ȃ�����
				if (0 <= tvi.lParam) {
					funcInfo = pFuncInfoArr->GetAt(tvi.lParam);
				}else {
					if (nListType == OutlineType::FileTree) {
						if (tvi.lParam == -1) {
							int nItem;
							if (!GetTreeFileFullName(hwndTree, htiItem, &sJumpFile, &nItem)) {
								sJumpFile = _T(""); // error
							}
						}
					}
				}
			}
		}
	}
	return 1;
}

// Java/C++���\�b�h�c���[�̍ő�l�X�g�[��
#define MAX_JAVA_TREE_NEST 16

/*! �c���[�R���g���[���̏������FJava���\�b�h�c���[

	Java Method Tree�̍\�z: �֐����X�g������TreeControl������������B

	@date 2002.01.04 genta C++�c���[�𓝍�
*/
void DlgFuncList::SetTreeJava(
	HWND hwndDlg,
	bool bAddClass
	)
{
	size_t nFuncLineTop(INT_MAX);
	size_t nFuncColTop(INT_MAX);
	TV_INSERTSTRUCT	tvis;
	const TCHAR*	pPos;
    TCHAR           szLabel[64 + 6];  // Jan. 07, 2001 genta �N���X���G���A�̊g��
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++�Ɠ���
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiSelectedTop = NULL;
	HTREEITEM		htiSelected = NULL;
	TV_ITEM			tvi;
	int				nDummylParam = -64000;	// 2002.11.10 Moca �N���X���̃_�~�[lParam �\�[�g�̂���
	TCHAR			szClassArr[MAX_JAVA_TREE_NEST][64];	// Jan. 04, 2001 genta �N���X���G���A�̊g�� // 2009.9.21 syat �l�X�g���[������ۂ�BOF�΍�

	EnableItem(IDC_BUTTON_COPY, true);

	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	memClipText.SetString(L"");
	{
		const size_t nBuffLenTag = 13 + wcslen(to_wchar(pFuncInfoArr->szFilePath));
		const size_t nNum = pFuncInfoArr->GetNum();
		size_t nBuffLen = 0;
		for (size_t i=0; i<nNum; ++i) {
			const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
			nBuffLen += pFuncInfo->memFuncName.GetStringLength();
		}
		memClipText.AllocStringBuffer(nBuffLen + nBuffLenTag * nNum);
	}
	// �ǉ�������̏������i�v���O�C���Ŏw��ς݂̏ꍇ�͏㏑�����Ȃ��j
	pFuncInfoArr->SetAppendText(FL_OBJ_DECLARE,		LSW(STR_DLGFNCLST_APND_DECLARE),	false);
	pFuncInfoArr->SetAppendText(FL_OBJ_CLASS,		LSW(STR_DLGFNCLST_APND_CLASS),		false);
	pFuncInfoArr->SetAppendText(FL_OBJ_STRUCT,		LSW(STR_DLGFNCLST_APND_STRUCT),		false);
	pFuncInfoArr->SetAppendText(FL_OBJ_ENUM,			LSW(STR_DLGFNCLST_APND_ENUM),		false);
	pFuncInfoArr->SetAppendText(FL_OBJ_UNION,		LSW(STR_DLGFNCLST_APND_UNION),		false);
	pFuncInfoArr->SetAppendText(FL_OBJ_NAMESPACE,	LSW(STR_DLGFNCLST_APND_NAMESPACE),	false);
	pFuncInfoArr->SetAppendText(FL_OBJ_INTERFACE,	LSW(STR_DLGFNCLST_APND_INTERFACE),	false);
	pFuncInfoArr->SetAppendText(FL_OBJ_GLOBAL,		LSW(STR_DLGFNCLST_APND_GLOBAL),		false);
	
	size_t nFuncLineOld = INT_MAX;
	size_t nFuncColOld = INT_MAX;
	int bSelected = FALSE;
	for (size_t i=0; i<pFuncInfoArr->GetNum(); ++i) {
		const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
		const TCHAR* pWork = pFuncInfo->memFuncName.GetStringPtr();
		size_t m = 0;
		int nClassNest = 0;
		// �N���X��::���\�b�h�̏ꍇ
		if ((pPos = _tcsstr(pWork, _T("::")))
			&& auto_strncmp(_T("operator "), pWork, 9) != 0
		) {
			// �C���i�[�N���X�̃l�X�g���x���𒲂ׂ�
			size_t k;
			size_t nWorkLen;
			size_t nCharChars;
			int	nNestTemplate = 0;
			nWorkLen = _tcslen(pWork);
			for (k=0; k<nWorkLen; ++k) {
				// 2009.9.21 syat �l�X�g���[������ۂ�BOF�΍�
				if (nClassNest == MAX_JAVA_TREE_NEST) {
					k = nWorkLen;
					break;
				}
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = NativeT::GetSizeOfChar(pWork, nWorkLen, k);
				if (nCharChars == 1 && nNestTemplate == 0 && pWork[k] == _T(':')) {
					//	Jan. 04, 2001 genta
					//	C++�̓����̂��߁A\�ɉ�����::���N���X��؂�Ƃ݂Ȃ��悤��
					if (k < nWorkLen - 1 && _T(':') == pWork[k + 1]) {
						auto_memcpy(szClassArr[nClassNest], &pWork[m], k - m);
						szClassArr[nClassNest][k - m] = _T('\0');
						++nClassNest;
						m = k + 2;
						++k;
						// Klass::operator std::string
						if (auto_strncmp(_T("operator "), pWork + m, 9) == 0) {
							break;
						}
					}else {
						break;
					}
				}else if (nCharChars == 1 && _T('\\') == pWork[k]) {
					auto_memcpy(szClassArr[nClassNest], &pWork[m], k - m);
					szClassArr[nClassNest][k - m] = _T('\0');
					++nClassNest;
					m = k + 1;
				}else if (nCharChars == 1 && _T('<') == pWork[k]) {
					// namesp::function<std::string> �̂悤�Ȃ��̂���������
					++nNestTemplate;
				}else if (nCharChars == 1 && _T('>') == pWork[k]) {
					if (0 < nNestTemplate) {
						--nNestTemplate;
					}
				}
				if (nCharChars == 2) {
					++k;
				}
			}
		}
		if (0 < nClassNest) {
			int	k;
			//	Jan. 04, 2001 genta
			//	�֐��擪�̃Z�b�g(�c���[�\�z�Ŏg��)
			pWork = pWork + m; // 2 == lstrlen("::");

			// �N���X���̃A�C�e�����o�^����Ă��邩
			htiClass = TreeView_GetFirstVisible(hwndTree);
			HTREEITEM htiParent = TVI_ROOT;
			for (k=0; k<nClassNest; ++k) {
				//	Apr. 1, 2001 genta
				//	�ǉ��������S�p�ɂ����̂Ń����������ꂾ���K�v
				//	6 == strlen("�N���X"), 1 == strlen(L'\0')

				// 2002/10/30 frozen
				// bAddClass == true �̏ꍇ�̎d�l�ύX
				// �����̍��ڂ́@�u(�N���X��)(���p�X�y�[�X���)(�ǉ�������)�v
				// �ƂȂ��Ă���Ƃ݂Ȃ��AszClassArr[k] �� �u�N���X���v�ƈ�v����΁A�����e�m�[�h�ɐݒ�B
				// �������A��v���鍀�ڂ���������ꍇ�͍ŏ��̍��ڂ�e�m�[�h�ɂ���B
				// ��v���Ȃ��ꍇ�́u(�N���X��)(���p�X�y�[�X���)�N���X�v�̃m�[�h���쐬����B
				size_t nClassNameLen = _tcslen(szClassArr[k]);
				for (; htiClass; htiClass=TreeView_GetNextSibling(hwndTree, htiClass)) {
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = _countof(szLabel);
					if (TreeView_GetItem(hwndTree, &tvi)) {
						if (_tcsncmp(szClassArr[k], szLabel, nClassNameLen) == 0) {
							if (_countof(szLabel) < (nClassNameLen +1)) {
								break;// �o�b�t�@�s���ł͖������Ƀ}�b�`����
							}else {
								if (bAddClass) {
									if (szLabel[nClassNameLen] == L' ') {
										break;
									}
								}else {
									if (szLabel[nClassNameLen] == L'\0') {
										break;
									}
								}
							}
						}
					}
				}

				// �N���X���̃A�C�e�����o�^����Ă��Ȃ��̂œo�^
				if (!htiClass) {
					// 2002/10/28 frozen �ォ�炱���ֈړ�
					// 2002/10/28 frozen +9�͒ǉ����镶����̍ő咷�i" ���O���"���ő�j// 2011.09.25 syat �v���O�C���ɂ��g���Ή�
					std::vector<TCHAR> className(_tcslen(szClassArr[k]) + 1 + pFuncInfoArr->AppendTextLenMax());
					TCHAR* pClassName = &className[0];
					_tcscpy(pClassName, szClassArr[k]);

					tvis.item.lParam = -1;
					if (bAddClass) {
						if (pFuncInfo->nInfo == FL_OBJ_NAMESPACE) {
							//_tcscat(pClassName, _T(" ���O���"));
							_tcscat(pClassName, to_tchar(pFuncInfoArr->GetAppendText(FL_OBJ_NAMESPACE).c_str()));
							tvis.item.lParam = i;
						}else {
							//_tcscat(pClassName, _T(" �N���X"));
							_tcscat(pClassName, to_tchar(pFuncInfoArr->GetAppendText(FL_OBJ_CLASS).c_str()));
							tvis.item.lParam = nDummylParam;
							++nDummylParam;
						}
					}

					tvis.hParent = htiParent;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
					tvis.item.pszText = const_cast<TCHAR*>(to_tchar(pClassName));

					htiClass = TreeView_InsertItem(hwndTree, &tvis);
				}else {
					// none
				}
				htiParent = htiClass;
				//if (k + 1 >= nClassNest) {
				//	break;
				//}
				htiClass = TreeView_GetChild(hwndTree, htiClass);
			}
			htiClass = htiParent;
		}else {
			//	Jan. 04, 2001 genta
			//	Global��Ԃ̏ꍇ (C++�̂�)

			// 2002/10/27 frozen ��������
			// 2007.05.26 genta "__interface" ���N���X�ɗނ��鈵���ɂ���
			// 2011.09.25 syat �v���O�C���Œǉ����ꂽ�v�f���N���X�ɗނ��鈵���ɂ���
			if (FL_OBJ_CLASS <= pFuncInfo->nInfo  && pFuncInfo->nInfo <= FL_OBJ_ELEMENT_MAX) {
				htiClass = TVI_ROOT;
			}else {
			// 2002/10/27 frozen �����܂�
				if (!htiGlobal) {
					TV_INSERTSTRUCT	tvg = {0};
					std::tstring sGlobal = to_tchar(pFuncInfoArr->GetAppendText(FL_OBJ_GLOBAL).c_str());
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					//tvg.item.pszText = const_cast<TCHAR*>(_T("�O���[�o��"));
					tvg.item.pszText = const_cast<TCHAR*>(sGlobal.c_str());
					tvg.item.lParam = nDummylParam;
					htiGlobal = TreeView_InsertItem(hwndTree, &tvg);
					++nDummylParam;
				}
				htiClass = htiGlobal;
			}
		}
		std::vector<TCHAR> funcName(_tcslen(pWork) + pFuncInfoArr->AppendTextLenMax());	// ���Œǉ����镶���񂪎��܂邾���m��
		TCHAR* pFuncName = &funcName[0];
		_tcscpy(pFuncName, pWork);

		// 2002/10/27 frozen �ǉ�������̎�ނ𑝂₵��
		switch (pFuncInfo->nInfo) {
		case FL_OBJ_DEFINITION:		//�u��`�ʒu�v�ɒǉ�������͕s�v�Ȃ��ߏ��O
		case FL_OBJ_NAMESPACE:		//�u���O��ԁv�͕ʂ̏ꏊ�ŏ������Ă�̂ŏ��O
		case FL_OBJ_GLOBAL:			//�u�O���[�o���v�͕ʂ̏ꏊ�ŏ������Ă�̂ŏ��O
			break;
		default:
			_tcscat(pFuncName, to_tchar(pFuncInfoArr->GetAppendText(pFuncInfo->nInfo).c_str()));
		}

		// �Y���N���X���̃A�C�e���̎q�Ƃ��āA���\�b�h�̃A�C�e����o�^
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = pFuncName;
		tvis.item.lParam = i;
		htiItem = TreeView_InsertItem(hwndTree, &tvis);

		// �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW
		wchar_t szText[2048];
		auto_sprintf(
			szText,
			L"%ts(%d,%d): ",
			pFuncInfoArr->szFilePath.c_str(),	// ��͑Ώۃt�@�C����
			pFuncInfo->nFuncLineCRLF,			// ���o�s�ԍ�
			pFuncInfo->nFuncColCRLF				// ���o���ԍ�
		);
		memClipText.AppendString(szText);		// �N���b�v�{�[�h�R�s�[�p�e�L�X�g
		// "%ts%ls\r\n"
		memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
		memClipText.AppendString(pFuncInfo->nInfo == FL_OBJ_DECLARE ? pFuncInfoArr->GetAppendText(FL_OBJ_DECLARE).c_str() : L""); 	//	Jan. 04, 2001 genta C++�Ŏg�p
		memClipText.AppendStringLiteral(L"\r\n");

		// ���݃J�[�\���ʒu�̃��\�b�h���ǂ������ׂ�
		if (!bSelected) {
			if (pFuncInfo->nFuncLineLAYOUT < nFuncLineTop
				|| (pFuncInfo->nFuncLineLAYOUT == nFuncLineTop && pFuncInfo->nFuncColLAYOUT <= nFuncColTop)
			) {
				nFuncLineTop = pFuncInfo->nFuncLineLAYOUT;
				nFuncColTop = pFuncInfo->nFuncColLAYOUT;
				htiSelectedTop = htiItem;
			}
		}
		{
			if ((nFuncLineOld < pFuncInfo->nFuncLineLAYOUT
				|| (nFuncLineOld == pFuncInfo->nFuncColLAYOUT && nFuncColOld <= pFuncInfo->nFuncColLAYOUT))
			  && (pFuncInfo->nFuncLineLAYOUT < nCurLine
				|| (pFuncInfo->nFuncLineLAYOUT == nCurLine && pFuncInfo->nFuncColLAYOUT <= nCurCol))
			) {
				nFuncLineOld = pFuncInfo->nFuncLineLAYOUT;
				nFuncColOld = pFuncInfo->nFuncColLAYOUT;
				bSelected = TRUE;
				htiSelected = htiItem;
			}
		}
		//	Jan. 04, 2001 genta
		//	delete�͂��̓s�x�s���̂ł����ł͕s�v
	}
	// �\�[�g�A�m�[�h�̓W�J������
//	TreeView_SortChildren(hwndTree, TVI_ROOT, 0);
	htiClass = TreeView_GetFirstVisible(hwndTree);
	while (htiClass) {
//		TreeView_SortChildren(hwndTree, htiClass, 0);
		TreeView_Expand(hwndTree, htiClass, TVE_EXPAND);
		htiClass = TreeView_GetNextSibling(hwndTree, htiClass);
	}
	// ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ���
	if (bSelected) {
		TreeView_SelectItem(hwndTree, htiSelected);
	}else {
		TreeView_SelectItem(hwndTree, htiSelectedTop);
	}
//	GetTreeTextNext(hwndTree, NULL, 0);
	return;
}


/*! ���X�g�r���[�R���g���[���̏������FVisualBasic

  �����Ȃ����̂œƗ������܂����B

  @date Jul 10, 2003  little YOSHI
*/
void DlgFuncList::SetListVB(void)
{
	TCHAR	szType[64];
	TCHAR	szOption[64];
	LV_ITEM	item;
	HWND	hwndList;
	size_t	nFuncLineOld;
	size_t	nFuncColOld;
	size_t	nSelectedLine = 0;
	RECT	rc;

	EnableItem(IDC_BUTTON_COPY, true);

	hwndList = GetItemHwnd(IDC_LIST_FL);

	memClipText.SetString(L"");
	{
		const size_t nBuffLenTag = 17 + wcslen(to_wchar(pFuncInfoArr->szFilePath));
		const size_t nNum = pFuncInfoArr->GetNum();
		size_t nBuffLen = 0;
		for (size_t i=0; i<nNum; ++i) {
			const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
			nBuffLen += pFuncInfo->memFuncName.GetStringLength();
		}
		memClipText.AllocStringBuffer(nBuffLen + nBuffLenTag * nNum);
	}

	nFuncLineOld = -1;
	nFuncColOld = -1;
	size_t nFuncLineTop(INT_MAX);
	size_t nFuncColTop(INT_MAX);
	size_t nSelectedLineTop = 0;
	bool bSelected = false;
	for (size_t i=0; i<pFuncInfoArr->GetNum(); ++i) {
		const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
		if (!bSelected) {
			if (pFuncInfo->nFuncLineLAYOUT < nFuncLineTop
				|| (pFuncInfo->nFuncLineLAYOUT == nFuncLineTop && pFuncInfo->nFuncColLAYOUT <= nFuncColTop)
			) {
				nFuncLineTop = pFuncInfo->nFuncLineLAYOUT;
				nFuncColTop = pFuncInfo->nFuncColLAYOUT;
				nSelectedLineTop = i;
			}
		}
		{
			if ((nFuncLineOld < pFuncInfo->nFuncLineLAYOUT
				|| (nFuncLineOld == pFuncInfo->nFuncColLAYOUT && nFuncColOld <= pFuncInfo->nFuncColLAYOUT))
			  && (pFuncInfo->nFuncLineLAYOUT < nCurLine
				|| (pFuncInfo->nFuncLineLAYOUT == nCurLine && pFuncInfo->nFuncColLAYOUT <= nCurCol))
			) {
				nFuncLineOld = pFuncInfo->nFuncLineLAYOUT;
				nFuncColOld = pFuncInfo->nFuncColLAYOUT;
				bSelected = true;
				nSelectedLine = i;
			}
		}
	}
	if (0 < pFuncInfoArr->GetNum() && !bSelected) {
		bSelected = true;
		nSelectedLine = nSelectedLineTop;
	}

	TCHAR szText[2048];
	for (size_t i=0; i<pFuncInfoArr->GetNum(); ++i) {
		// ���݂̉�͌��ʗv�f
		const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);

		//	From Here Apr. 23, 2005 genta �s�ԍ������[��
		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		if (bLineNumIsCRLF) {
			auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncLineCRLF);
		}else {
			auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncLineLAYOUT);
		}
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = szText;
		item.iItem = (int)i;
		item.iSubItem = FL_COL_ROW;
		item.lParam	= i;
		ListView_InsertItem(hwndList, &item);

		// 2010.03.17 syat ���ǉ�
		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		if (bLineNumIsCRLF) {
			auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncColCRLF);
		}else {
			auto_sprintf(szText, _T("%d"), pFuncInfo->nFuncColLAYOUT);
		}
		item.mask = LVIF_TEXT;
		item.pszText = szText;
		item.iItem = (int)i;
		item.iSubItem = FL_COL_COL;
		ListView_SetItem(hwndList, &item);

		item.mask = LVIF_TEXT;
		item.pszText = const_cast<TCHAR*>(pFuncInfo->memFuncName.GetStringPtr());
		item.iItem = (int)i;
		item.iSubItem = FL_COL_NAME;
		ListView_SetItem(hwndList, &item);
		//	To Here Apr. 23, 2005 genta �s�ԍ������[��

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta ���p���ȁ��S�p��
		auto_memset(szText, _T('\0'), _countof(szText));
		auto_memset(szType, _T('\0'), _countof(szType));
		auto_memset(szOption, _T('\0'), _countof(szOption));
		if (((pFuncInfo->nInfo >> 8) & 0x01) == 1) {
			// �X�^�e�B�b�N�錾(Static)
			// 2006.12.12 Moca �����ɃX�y�[�X�ǉ�
			_tcscpy(szOption, LS(STR_DLGFNCLST_VB_STATIC));
		}
		switch ((pFuncInfo->nInfo >> 4) & 0x0f) {
			case 2  :	// �v���C�x�[�g(Private)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PRIVATE), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
				break;

			case 3  :	// �t�����h(Friend)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_FRIEND), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
				break;

			default :	// �p�u���b�N(Public)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PUBLIC), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta �T�C�Y���C��
		}
		int nInfo = pFuncInfo->nInfo;
		switch (nInfo & 0x0f) {
			case 1:		// �֐�(Function)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_FUNCTION));
				break;

			// 2006.12.12 Moca �X�e�[�^�X���v���V�[�W���ɕύX
			case 2:		// �v���V�[�W��(Sub)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROC));
				break;

			case 3:		// �v���p�e�B �擾(Property Get)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPGET));
				break;

			case 4:		// �v���p�e�B �ݒ�(Property Let)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPLET));
				break;

			case 5:		// �v���p�e�B �Q��(Property Set)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPSET));
				break;

			case 6:		// �萔(Const)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_CONST));
				break;

			case 7:		// �񋓌^(Enum)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_ENUM));
				break;

			case 8:		// ���[�U��`�^(Type)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_TYPE));
				break;

			case 9:		// �C�x���g(Event)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_EVENT));
				break;

			default:	// ����`�Ȃ̂ŃN���A
				nInfo	= 0;

		}
		if (((nInfo >> 8) & 0x02) == 2) {
			// �錾(Declare�Ȃ�)
			_tcsncat(szType, LS(STR_DLGFNCLST_VB_DECL), _countof(szType) - _tcslen(szType));
		}

		TCHAR szTypeOption[256]; // 2006.12.12 Moca auto_sprintf�̓��o�͂œ���ϐ����g��Ȃ����߂̍�Ɨ̈�ǉ�
		if (nInfo == 0) {
			szTypeOption[0] = _T('\0');	//	2006.12.17 genta �S�̂�0�Ŗ��߂�K�v�͂Ȃ�
		}else
		if (szOption[0] == _T('\0')) {
			auto_sprintf(szTypeOption, _T("%ts"), szType);
		}else {
			auto_sprintf(szTypeOption, _T("%ts�i%ts�j"), szType, szOption);
		}
		item.pszText = szTypeOption;
		item.iItem = (int)i;
		item.iSubItem = FL_COL_REMARK;
		ListView_SetItem(hwndList, &item);

		// �N���b�v�{�[�h�ɃR�s�[����e�L�X�g��ҏW
		if (item.pszText[0] != _T('\0')) {
			// ���o���ʂ̎��(�֐�,,,)������Ƃ�
			// 2006.12.12 Moca szText ���������g�ɃR�s�[���Ă����o�O���C��
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				pFuncInfoArr->szFilePath.c_str(),	// ��͑Ώۃt�@�C����
				pFuncInfo->nFuncLineCRLF,			// ���o�s�ԍ�
				pFuncInfo->nFuncColCRLF				// ���o���ԍ�
			);
			memClipText.AppendStringT(szText);
			// "%ts(%ts)\r\n"
			memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
			memClipText.AppendStringLiteral(L"(");
			memClipText.AppendStringT(item.pszText);
			memClipText.AppendStringLiteral(L")\r\n");
		}else {
			// ���o���ʂ̎��(�֐�,,,)���Ȃ��Ƃ�
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				pFuncInfoArr->szFilePath.c_str(),	// ��͑Ώۃt�@�C����
				pFuncInfo->nFuncLineCRLF,			// ���o�s�ԍ�
				pFuncInfo->nFuncColCRLF				// ���o���ԍ�
			);
			memClipText.AppendStringT(szText);
			// "%ts\r\n"
			memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
			memClipText.AppendStringLiteral(L"\r\n");
		}
	}

	// 2002.02.08 hor List�͗񕝒����Ƃ������s����O�ɕ\�����Ƃ��Ȃ��ƕςɂȂ�
	::ShowWindow(hwndList, SW_SHOW);
	// ��̕����f�[�^�ɍ��킹�Ē���
	ListView_SetColumnWidth(hwndList, FL_COL_ROW, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hwndList, FL_COL_COL, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hwndList, FL_COL_NAME, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE);
	ListView_SetColumnWidth(hwndList, FL_COL_ROW, ListView_GetColumnWidth(hwndList, FL_COL_ROW) + 16);
	ListView_SetColumnWidth(hwndList, FL_COL_COL, ListView_GetColumnWidth(hwndList, FL_COL_COL) + 16);
	ListView_SetColumnWidth(hwndList, FL_COL_NAME, ListView_GetColumnWidth(hwndList, FL_COL_NAME) + 16);
	ListView_SetColumnWidth(hwndList, FL_COL_REMARK, ListView_GetColumnWidth(hwndList, FL_COL_REMARK) + 16);
	if (bSelected) {
		ListView_GetItemRect(hwndList, 0, &rc, LVIR_BOUNDS);
		ListView_Scroll(hwndList, 0, nSelectedLine * (rc.bottom - rc.top));
		ListView_SetItemState(hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}

	return;
}

/*! �ėp�c���[�R���g���[���̏������FFuncInfo::nDepth�𗘗p���Đe�q��ݒ�

	@param[in] tagjump �^�O�W�����v�`���ŏo�͂���

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca �K�w�̐������Ȃ�����
	@date 2007.02.25 genta �N���b�v�{�[�h�o�͂��^�u�W�����v�\�ȏ����ɕύX
	@date 2007.03.04 genta �^�u�W�����v�\�ȏ����ɕύX���邩�ǂ����̃t���O��ǉ�
	@date 2014.06.06 Moca ���t�@�C���ւ̃^�O�W�����v�@�\��ǉ�
*/
void DlgFuncList::SetTree(bool tagjump, bool nolabel)
{
	HTREEITEM hItemSelected = NULL;
	HTREEITEM hItemSelectedTop = NULL;
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	size_t nFuncInfoArrNum = pFuncInfoArr->GetNum();
	size_t nStackPointer = 0;
	size_t nStackDepth = 32; // phParentStack �̊m�ۂ��Ă��鐔
	HTREEITEM* phParentStack;
	phParentStack = (HTREEITEM*)malloc(nStackDepth * sizeof(HTREEITEM));
	phParentStack[nStackPointer] = TVI_ROOT;
	size_t nFuncLineOld(INT_MAX);
	size_t nFuncColOld(INT_MAX);
	size_t nFuncLineTop(INT_MAX);
	size_t nFuncColTop(INT_MAX);
	bool bSelected = false;

	memClipText.SetString(L"");
	{
		int nCount = 0;
		size_t nBuffLen = 0;
		size_t nBuffLenTag = 3; // " \r\n"
		if (tagjump) {
			nBuffLenTag = 10 + wcslen(to_wchar(pFuncInfoArr->szFilePath));
		}
		for (size_t i=0; i<nFuncInfoArrNum; ++i) {
			const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
			if (pFuncInfo->IsAddClipText()) {
				nBuffLen += pFuncInfo->memFuncName.GetStringLength() + pFuncInfo->nDepth * 2;
				++nCount;
			}
		}
		memClipText.AllocStringBuffer(nBuffLen + nBuffLenTag * nCount);
	}

	for (size_t i=0; i<nFuncInfoArrNum; ++i) {
		FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);

		/*	�V�����A�C�e�����쐬
			���݂̐e�̉��ɂԂ牺����`�ŁA�Ō�ɒǉ�����B
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[nStackPointer];
		cTVInsertStruct.hInsertAfter = TVI_LAST;	//	�K���Ō�ɒǉ��B
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pFuncInfo->memFuncName.GetStringPtr();
		cTVInsertStruct.item.lParam = i;	//	���Ƃł��̐��l�i��pcFuncInfoArr�̉��Ԗڂ̃A�C�e�����j�����āA�ړI�n�ɃW�����v���邺!!�B

		// �e�q�֌W���`�F�b�N
		if (nStackPointer != pFuncInfo->nDepth) {
			//	���x�����ς��܂���!!
			//	�����A2�i�K�[���Ȃ邱�Ƃ͍l�����Ă��Ȃ��̂Œ��ӁB
			//	�@�������A2�i�K�ȏ�󂭂Ȃ邱�Ƃ͍l���ς݁B

			// 2002.11.10 Moca �ǉ� �m�ۂ����T�C�Y�ł͑���Ȃ��Ȃ����B�Ċm��
			if (nStackDepth <= pFuncInfo->nDepth + 1) {
				nStackDepth = pFuncInfo->nDepth + 4; // ���߂Ɋm�ۂ��Ă���
				HTREEITEM* phTi;
				phTi = (HTREEITEM*)realloc(phParentStack, nStackDepth * sizeof(HTREEITEM));
				if (phTi) {
					phParentStack = phTi;
				}else {
					goto end_of_func;
				}
			}
			nStackPointer = pFuncInfo->nDepth;
			cTVInsertStruct.hParent = phParentStack[nStackPointer];
		}
		hItem = TreeView_InsertItem(hwndTree, &cTVInsertStruct);
		phParentStack[nStackPointer + 1] = hItem;

		// pFuncInfo�ɓo�^����Ă���s���A�����m�F���āA�I������A�C�e�����l����
		bool bFileSelect = false;
		if (pFuncInfo->memFileName.GetStringPtr() && pFuncInfoArr->szFilePath[0]) {
			if (auto_stricmp(pFuncInfo->memFileName.GetStringPtr(), pFuncInfoArr->szFilePath.c_str()) == 0) {
				bFileSelect = true;
			}
		}else {
			bFileSelect = true;
		}
		if (bFileSelect) {
			if (!bSelected) {
				if (pFuncInfo->nFuncLineLAYOUT < nFuncLineTop
					|| (pFuncInfo->nFuncLineLAYOUT == nFuncLineTop && pFuncInfo->nFuncColLAYOUT <= nFuncColTop)
				) {
					nFuncLineTop = pFuncInfo->nFuncLineLAYOUT;
					nFuncColTop = pFuncInfo->nFuncColLAYOUT;
					hItemSelectedTop = hItem;
				}
			}
			if ((nFuncLineOld < pFuncInfo->nFuncLineLAYOUT
				|| (nFuncLineOld == pFuncInfo->nFuncColLAYOUT && nFuncColOld <= pFuncInfo->nFuncColLAYOUT))
			  && (pFuncInfo->nFuncLineLAYOUT < nCurLine
				|| (pFuncInfo->nFuncLineLAYOUT == nCurLine && pFuncInfo->nFuncColLAYOUT <= nCurCol))
			) {
				nFuncLineOld = pFuncInfo->nFuncLineLAYOUT;
				nFuncColOld = pFuncInfo->nFuncColLAYOUT;
				hItemSelected = hItem;
				bSelected = true;
			}
		}

		// �N���b�v�{�[�h�R�s�[�p�e�L�X�g���쐬����
		//	2003.06.22 Moca dummy�v�f�̓c���[�ɓ���邪TAGJUMP�ɂ͉����Ȃ�
		if (pFuncInfo->IsAddClipText()) {
			NativeT text;
			if (tagjump) {
				const TCHAR* pszFileName = pFuncInfo->memFileName.GetStringPtr();
				if (!pszFileName) {
					pszFileName = pFuncInfoArr->szFilePath;
				}
				text.AllocStringBuffer(
					  pFuncInfo->memFuncName.GetStringLength()
					+ nStackPointer * 2 + 1
					+ _tcslen(pszFileName)
					+ 20
				);
				//	2007.03.04 genta �^�O�W�����v�ł���`���ŏ�������
				text.AppendString(pszFileName);
				
				if (0 < pFuncInfo->nFuncLineCRLF) {
					TCHAR linenum[32];
					int len = auto_sprintf(linenum, _T("(%d,%d): "),
						pFuncInfo->nFuncLineCRLF,				// ���o�s�ԍ�
						pFuncInfo->nFuncColCRLF					// ���o���ԍ�
					);
					text.AppendString(linenum);
				}
			}

			if (!nolabel) {
				for (int cnt=0; cnt<nStackPointer; ++cnt) {
					text.AppendStringLiteral(_T("  "));
				}
				text.AppendStringLiteral(_T(" "));
				
				text.AppendNativeData(pFuncInfo->memFuncName);
			}
			text.AppendStringLiteral(_T("\r\n"));
			memClipText.AppendNativeDataT(text);	// �N���b�v�{�[�h�R�s�[�p�e�L�X�g
		}
	}

end_of_func:;

	EnableItem(IDC_BUTTON_COPY, true);

	if (hItemSelected) {
		// ���݃J�[�\���ʒu�̃��\�b�h��I����Ԃɂ���
		TreeView_SelectItem(hwndTree, hItemSelected);
	}else if (hItemSelectedTop) {
		TreeView_SelectItem(hwndTree, hItemSelectedTop);
	}

	free(phParentStack);
}


void DlgFuncList::SetDocLineFuncList()
{
	if (nOutlineType == OutlineType::BookMark) {
		return;
	}
	if (nOutlineType == OutlineType::FileTree) {
		return;
	}
	EditView* pEditView = (EditView*)(this->lParam);
	auto& docLineMgr = pEditView->GetDocument().docLineMgr;
	
	FuncListManager().ResetAllFucListMark(docLineMgr, false);
	size_t num = pFuncInfoArr->GetNum();
	for (size_t i=0; i<num; ++i) {
		const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);
		if (0 < pFuncInfo->nFuncLineCRLF) {
			DocLine* pDocLine = docLineMgr.GetLine( pFuncInfo->nFuncLineCRLF - 1 );
			if (pDocLine) {
				FuncListManager().SetLineFuncList( pDocLine, true );
			}
		}
	}
}


/*! �t�@�C���c���[�쐬
	@note pFuncInfoArr�Ƀt���p�X�����������݂c���[���쐬
*/
void DlgFuncList::SetTreeFile()
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	memClipText.SetString(L"");
	SFilePath iniDirPath;
	LoadFileTreeSetting( fileTreeSetting, iniDirPath );
	pFuncInfoArr->Empty();
	int nFuncInfo = 0;
	std::vector<HTREEITEM> hParentTree;
	hParentTree.push_back(TVI_ROOT);
	for (int i=0; i<(int)fileTreeSetting.items.size(); ++i) {
		TCHAR szPath[_MAX_PATH];
		TCHAR szPath2[_MAX_PATH];
		const FileTreeItem& item = fileTreeSetting.items[i];
		// item.szTargetPath => szPath ���^�����̓W�J
		if (!FileNameManager::ExpandMetaToFolder(item.szTargetPath, szPath, _countof(szPath))) {
			auto_strcpy_s(szPath, _countof(szPath), _T("<Error:Long Path>"));
		}
		// szPath => szPath2 <iniroot>�W�J
		const TCHAR* pszFrom = szPath;
		if (fileTreeSetting.szLoadProjectIni[0] != _T('\0')) {
			NativeT strTemp(pszFrom);
			strTemp.Replace(_T("<iniroot>"), iniDirPath);
			if (_countof(szPath2) <= strTemp.GetStringLength()) {
				auto_strcpy_s(szPath2, _countof(szPath), _T("<Error:Long Path>"));
			}else {
				auto_strcpy_s(szPath2, _countof(szPath), strTemp.GetStringPtr());
			}
		}else {
			auto_strcpy(szPath2, pszFrom);
		}
		// szPath2 => szPath �u.�v��V���[�g�p�X���̓W�J
		pszFrom = szPath2;
		if (::GetLongFileName(pszFrom, szPath)) {
		}else {
			auto_strcpy(szPath, pszFrom);
		}
		while (item.nDepth < (int)hParentTree.size() - 1) {
			hParentTree.resize(hParentTree.size() - 1);
		}
		const TCHAR* pszLabel = szPath;
		if (item.szLabelName[0] != _T('\0')) {
			pszLabel = item.szLabelName;
		}
		// lvis.item.lParam
		// 0 �ȉ�(nFuncInfo): pFuncInfoArr->At(nFuncInfo)�Ƀt�@�C����
		// -1: Grep�̃t�@�C�����v�f
		// -2: Grep�̃T�u�t�H���_�v�f
		// -(nFuncInfo * 10 + 3): Grep���[�g�t�H���_�v�f
		// -4: �f�[�^�E�ǉ�����Ȃ�
		TVINSERTSTRUCT tvis;
		tvis.hParent = hParentTree.back();
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		if (item.eFileTreeItemType == FileTreeItemType::Grep) {
			pFuncInfoArr->AppendData( -1, -1, -1, -1, _T(""), szPath, 0, 0 );
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = -(nFuncInfo * 10 + 3);
			HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
			++nFuncInfo;
			SetTreeFileSub( hParent, NULL );
		}else if (item.eFileTreeItemType == FileTreeItemType::File) {
			pFuncInfoArr->AppendData( -1, -1, -1, -1, _T(""), szPath, 0, 0 );
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = nFuncInfo;
			TreeView_InsertItem(hwndTree, &tvis);
			++nFuncInfo;
		}else if (item.eFileTreeItemType == FileTreeItemType::Folder) {
			pszLabel = item.szLabelName;
			if (pszLabel[0] == _T('\0')) {
				pszLabel = _T("Folder");
			}
			tvis.item.pszText = const_cast<TCHAR*>(pszLabel);
			tvis.item.lParam  = -4;
			HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
			hParentTree.push_back(hParent);
		}
	}
}


void DlgFuncList::SetTreeFileSub(
	HTREEITEM hParent,
	const TCHAR* pszFile
	)
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	if (TreeView_GetChild(hwndTree, hParent)) {
		return;
	}

	HTREEITEM hItemSelected = NULL;

	std::tstring basePath;
	int nItem = 0; // �ݒ�Item�ԍ�
	if (!GetTreeFileFullName( hwndTree, hParent, &basePath, &nItem )) {
		return; // error
	}

	size_t count = 0;
	GrepEnumKeys grepEnumKeys;
	int errNo = grepEnumKeys.SetFileKeys( fileTreeSetting.items[nItem].szTargetFile );
	if (errNo != 0) {
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.pszText = const_cast<TCHAR*>(_T("<Wild Card Error>"));
		tvis.item.lParam = -4;
		TreeView_InsertItem(hwndTree, &tvis);
		return;
	}
	GrepEnumOptions grepEnumOptions;
	grepEnumOptions.bIgnoreHidden   = fileTreeSetting.items[nItem].bIgnoreHidden;
	grepEnumOptions.bIgnoreReadOnly = fileTreeSetting.items[nItem].bIgnoreReadOnly;
	grepEnumOptions.bIgnoreSystem   = fileTreeSetting.items[nItem].bIgnoreSystem;
	GrepEnumFiles grepExceptAbsFiles;
	grepExceptAbsFiles.Enumerates(_T(""), grepEnumKeys.vecExceptAbsFileKeys, grepEnumOptions);
	GrepEnumFolders grepExceptAbsFolders;
	grepExceptAbsFolders.Enumerates(_T(""), grepEnumKeys.vecExceptAbsFolderKeys, grepEnumOptions);

	//�t�H���_�ꗗ�쐬
	GrepEnumFilterFolders grepEnumFilterFolders;
	grepEnumFilterFolders.Enumerates( basePath.c_str(), grepEnumKeys, grepEnumOptions, grepExceptAbsFolders );
	size_t nItemCount = grepEnumFilterFolders.GetCount();
	count = nItemCount;
	for (size_t i=0; i<nItemCount; ++i) {
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.pszText = const_cast<TCHAR*>(grepEnumFilterFolders.GetFileName(i));
		tvis.item.lParam  = -2;
		tvis.item.cChildren = 1; // �_�~�[�̎q�v�f����������[+]��\��
		TreeView_InsertItem(hwndTree, &tvis);
	}

	//�t�@�C���ꗗ�쐬
	GrepEnumFilterFiles grepEnumFilterFiles;
	grepEnumFilterFiles.Enumerates( basePath.c_str(), grepEnumKeys, grepEnumOptions, grepExceptAbsFiles );
	nItemCount = grepEnumFilterFiles.GetCount();
	count += nItemCount;
	for (size_t i=0; i<nItemCount; ++i) {
		const TCHAR* pFile = grepEnumFilterFiles.GetFileName(i);
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParent;
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = const_cast<TCHAR*>(pFile);
		tvis.item.lParam  = -1;
		HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvis);
		if (pszFile && auto_stricmp(pszFile, pFile) == 0) {
			hItemSelected = hItem;
		}
	}
	if (hItemSelected) {
		TreeView_SelectItem( hwndTree, hItemSelected );
	}
	if (count == 0) {
		// [+]�L���폜
		TVITEM item;
		item.mask  = TVIF_HANDLE | TVIF_CHILDREN;
		item.cChildren = 0;
		item.hItem = hParent;
		TreeView_SetItem(hwndTree, &item);
	}
}


BOOL DlgFuncList::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	bStretching = false;
	bHovering = false;
	nHilightedBtn = -1;
	nCapturingBtn = -1;

	_SetHwnd(hwndDlg);

	int			nCxVScroll;
	int			nColWidthArr[] = { 0, 10, 46, 80 };
	RECT		rc;
	LV_COLUMN	col;
	HWND hwndList = ::GetDlgItem(hwndDlg, IDC_LIST_FL);
	::SetWindowLongPtr(hwndList, GWL_STYLE, ::GetWindowLongPtr(hwndList, GWL_STYLE) | LVS_SHOWSELALWAYS);
	// 2005.10.21 zenryaku 1�s�I��
	ListView_SetExtendedListViewStyle(hwndList,
		ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	::GetWindowRect(hwndList, &rc);
	nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - (nColWidthArr[1] + nColWidthArr[2] + nColWidthArr[3]) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_LINE_M));
	col.iSubItem = FL_COL_ROW;
	ListView_InsertColumn(hwndList, FL_COL_ROW, &col);

	// 2010.03.17 syat ���ǉ�
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_COL];
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_COL));
	col.iSubItem = FL_COL_COL;
	ListView_InsertColumn(hwndList, FL_COL_COL, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_NAME];
	//	Apr. 23, 2005 genta �s�ԍ������[��
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_FUNC));
	col.iSubItem = FL_COL_NAME;
	ListView_InsertColumn(hwndList, FL_COL_NAME, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_REMARK];
	col.pszText = const_cast<TCHAR*>(_T(" "));
	col.iSubItem = FL_COL_REMARK;
	ListView_InsertColumn(hwndList, FL_COL_REMARK, &col);

	// �A�E�g���C���ʒu�ƃT�C�Y������������ // 20060201 aroka
	EditView* pEditView = (EditView*)(this->lParam);
	if (pEditView) {
		if (!IsDocking() && pShareData->common.outline.bRememberOutlineWindowPos) {
			WINDOWPLACEMENT windowPlacement;
			windowPlacement.length = sizeof(windowPlacement);
			if (::GetWindowPlacement(pEditView->editWnd.GetHwnd(), &windowPlacement)) {
				// �E�B���h�E�ʒu�E�T�C�Y��-1�ȊO�̒l�ɂ��Ă����ƁADialog�Ŏg�p�����D
				xPos = pShareData->common.outline.xOutlineWindowPos + windowPlacement.rcNormalPosition.left;
				yPos = pShareData->common.outline.yOutlineWindowPos + windowPlacement.rcNormalPosition.top;
				nWidth =  pShareData->common.outline.widthOutlineWindow;
				nHeight = pShareData->common.outline.heightOutlineWindow;
			}
		}else if (IsDocking()) {
			xPos = 0;
			yPos = 0;
			nShowCmd = SW_HIDE;
			::GetWindowRect(::GetParent(pEditView->GetHwnd()), &rc);	// �����ł͂܂� GetDockSpaceRect() �͎g���Ȃ�
			DockSideType dockSideType = GetDockSide();
			switch (dockSideType) {
			case DockSideType::Left:	nWidth = ProfDockLeft();		break;
			case DockSideType::Top:		nHeight = ProfDockTop();		break;
			case DockSideType::Right:	nWidth = ProfDockRight();		break;
			case DockSideType::Bottom:	nHeight = ProfDockBottom();	break;
			}
			if (dockSideType == DockSideType::Left || dockSideType == DockSideType::Right) {
				if (nWidth == 0) { // ����
					nWidth = (rc.right - rc.left) / 3;
				}
				if (nWidth > rc.right - rc.left - DOCK_MIN_SIZE) {
					nWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				}
				if (nWidth < DOCK_MIN_SIZE) {
					nWidth = DOCK_MIN_SIZE;
				}
			}else {
				if (nHeight == 0) { // ����
					nHeight = (rc.bottom - rc.top) / 3;
				}
				if (nHeight > rc.bottom - rc.top - DOCK_MIN_SIZE) {
					nHeight = rc.bottom - rc.top - DOCK_MIN_SIZE;
				}
				if (nHeight < DOCK_MIN_SIZE) {
					nHeight = DOCK_MIN_SIZE;
				}
			}
		}
	}

	if (!bInChangeLayout) {	// ChangeLayout() �������͐ݒ�ύX���Ȃ�
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ProfDockDisp() = TRUE;
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);

		}
		// ���E�B���h�E�ɕύX��ʒm����
		if (ProfDockSync()) {
			HWND hwndEdit = pEditView->editWnd.GetHwnd();
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);
		}
	}

	if (!IsDocking()) {
		// ���N���X�����o
		CreateSizeBox();

		LONG_PTR lStyle = ::GetWindowLongPtr(GetHwnd(), GWL_STYLE);
		::SetWindowLongPtr(GetHwnd(), GWL_STYLE, lStyle | WS_THICKFRAME);
		::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	hwndToolTip = NULL;
	if (IsDocking()) {
		// �c�[���`�b�v���쐬����B�i�u����v�Ȃǂ̃{�^���p�j
		hwndToolTip = ::CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GetHwnd(),
			NULL,
			hInstance,
			NULL
			);

		// �c�[���`�b�v���}���`���C���\�ɂ���iSHRT_MAX: Win95��INT_MAX���ƕ\������Ȃ��j
		Tooltip_SetMaxTipWidth(hwndToolTip, SHRT_MAX);

		// �A�E�g���C���Ƀc�[���`�b�v��ǉ�����
		TOOLINFO	ti;
		ti.cbSize      = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId �� HWND �� rect �͖����iHWND �S�́j
		ti.hwnd        = GetHwnd();
		ti.hinst       = hInstance;
		ti.uId         = (UINT_PTR)GetHwnd();
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		Tooltip_AddTool(hwndToolTip, &ti);

		// �s�v�ȃR���g���[�����B��
		HWND hwndPrev;
		HWND hwnd = ::GetWindow(GetHwnd(), GW_CHILD);
		while (hwnd) {
			int nId = ::GetDlgCtrlID(hwnd);
			hwndPrev = hwnd;
			hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
			switch (nId) {
			case IDC_STATIC_nSortType:
			case IDC_COMBO_nSortType:
			case IDC_LIST_FL:
			case IDC_TREE_FL:
				continue;
			}
			ShowWindow(hwndPrev, SW_HIDE);
		}
	}

	SyncColor();

	::GetWindowRect(hwndDlg, &rc);
	ptDefaultSize.x = rc.right - rc.left;
	ptDefaultSize.y = rc.bottom - rc.top;
	
	::GetClientRect(hwndDlg, &rc);
	ptDefaultSizeClient.x = rc.right;
	ptDefaultSizeClient.y = rc.bottom;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, rcItems[i]);
		// �h�b�L���O���̓E�B���h�E�������ς��܂ŐL�΂�
		if (IsDocking()) {
			if (anchorList[i].anchor == AnchorStyle::All) {
				::GetClientRect(hwndDlg, &rc);
				rcItems[i].right = rc.right;
				rcItems[i].bottom = rc.bottom;
			}
		}
	}
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgFuncList::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_MENU:
		RECT rcMenu;
		::GetWindowRect(GetItemHwnd(IDC_BUTTON_MENU), &rcMenu);
		POINT ptMenu;
		ptMenu.x = rcMenu.left;
		ptMenu.y = rcMenu.bottom;
		DoMenu(ptMenu, GetHwnd());
		return TRUE;
	case IDC_BUTTON_HELP:
		//�u�A�E�g���C����́v�̃w���v
		// Apr. 5, 2001 JEPRO �C���R���ǉ� (Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���)
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if (bModal) {		// ���[�_�� �_�C�A���O��
			::EndDialog(GetHwnd(), 0);
		}else {
			if (IsDocking()) {
				::SetFocus(((EditView*)lParam)->GetHwnd());
			}else {
				::DestroyWindow(GetHwnd());
			}
		}
		return TRUE;
	case IDC_BUTTON_COPY:
		// Windows�N���b�v�{�[�h�ɃR�s�[ 
		// 2004.02.17 Moca �֐���
		SetClipboardText(GetHwnd(), memClipText.GetStringPtr(), memClipText.GetStringLength());
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// �E�B���h�E�̈ʒu�ƃT�C�Y���L�� // 20060201 aroka
			pShareData->common.outline.bRememberOutlineWindowPos = IsButtonChecked(IDC_BUTTON_WINSIZE);
		}
		// �{�^����������Ă��邩�͂����肳���� 2008/6/5 Uchi
		SetItemText(IDC_BUTTON_WINSIZE,
			pShareData->common.outline.bRememberOutlineWindowPos ? _T("��") : _T("��"));
		return TRUE;
	// 2002.02.08 �I�v�V�����ؑ֌�List/Tree�Ƀt�H�[�J�X�ړ�
	case IDC_CHECK_bAutoCloseDlgFuncList:
	case IDC_CHECK_bMarkUpBlankLineEnable:
	case IDC_CHECK_bFunclistSetFocusOnJump:
		pShareData->common.outline.bAutoCloseDlgFuncList = IsButtonChecked(IDC_CHECK_bAutoCloseDlgFuncList);
		pShareData->common.outline.bMarkUpBlankLineEnable = IsButtonChecked(IDC_CHECK_bMarkUpBlankLineEnable);
		pShareData->common.outline.bFunclistSetFocusOnJump = IsButtonChecked(IDC_CHECK_bFunclistSetFocusOnJump);
		EnableItem(IDC_CHECK_bFunclistSetFocusOnJump, !pShareData->common.outline.bAutoCloseDlgFuncList);
		if (wID == IDC_CHECK_bMarkUpBlankLineEnable&&nListType == OutlineType::BookMark) {
			EditView* pEditView = (EditView*)lParam;
			pEditView->GetCommander().HandleCommand(F_BOOKMARK_VIEW, true, TRUE, 0, 0, 0);
			nCurLine = pEditView->GetCaret().GetCaretLayoutPos().y + 1;
			DocTypeManager().GetTypeConfig(pEditView->GetDocument().docType.GetDocumentType(), type);
			SetData();
		}else
		if (nViewType == VIEWTYPE_TREE) {
			::SetFocus(GetItemHwnd(IDC_TREE_FL));
		}else {
			::SetFocus(GetItemHwnd(IDC_LIST_FL));
		}
		return TRUE;
	case IDC_BUTTON_SETTING:
		{
			DlgFileTree dlgFileTree;
			INT_PTR nRet = dlgFileTree.DoModal(G_AppInstance(), GetHwnd(), (LPARAM)this);
			if (nRet == TRUE) {
				EFunctionCode nFuncCode = GetFuncCodeRedraw(nOutlineType);
				EditView* pEditView = (EditView*)lParam;
				pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0);
			}
		}
	}
	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}

BOOL DlgFuncList::OnNotify(WPARAM wParam, LPARAM lParam)
{
//	int				idCtrl;
	LPNMHDR			pnmh;
	NM_LISTVIEW*	pnlv;
	HWND			hwndList;
	HWND			hwndTree;
	NM_TREEVIEW*	pnmtv;
	pnmh = (LPNMHDR) lParam;
	pnlv = (NM_LISTVIEW*)lParam;

	EditView* pEditView = (EditView*)(this->lParam);
	hwndList = GetItemHwnd(IDC_LIST_FL);
	hwndTree = GetItemHwnd(IDC_TREE_FL);

	if (hwndTree == pnmh->hwndFrom) {
		pnmtv = (NM_TREEVIEW *) lParam;
		switch (pnmtv->hdr.code) {
		case NM_CLICK:
			if (IsDocking()) {
				// ���̎��_�ł͂܂��I��ύX����Ă��Ȃ��� OnJump() �̗\������Ƃ��Đ�ɑI��ύX���Ă���
				TVHITTESTINFO tvht = {0};
				::GetCursorPos(&tvht.pt);
				::ScreenToClient(hwndTree, &tvht.pt);
				TreeView_HitTest(hwndTree, &tvht);
				if ((tvht.flags & TVHT_ONITEM) && tvht.hItem) {
					TreeView_SelectItem(hwndTree, tvht.hItem);
					OnJump(false);
					return TRUE;
				}
			}
			break;
		case NM_DBLCLK:
			// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 3/4
			OnJump();
			bWaitTreeProcess=true;
			::SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, TRUE);	// �c���[�̓W�J�^�k�������Ȃ�
			return TRUE;
			//return OnJump();
		case TVN_KEYDOWN:
			if (((TV_KEYDOWN *)lParam)->wVKey == VK_SPACE) {
				OnJump(false);
				return TRUE;
			}
			Key2Command(((TV_KEYDOWN *)lParam)->wVKey);
			return TRUE;
		case NM_KILLFOCUS:
			// 2002.02.16 hor Tree�̃_�u���N���b�N�Ńt�H�[�J�X�ړ��ł���悤�� 4/4
			if (bWaitTreeProcess) {
				if (pShareData->common.outline.bFunclistSetFocusOnJump) {
					::SetFocus(pEditView->GetHwnd());
				}
				bWaitTreeProcess=false;
			}
			return TRUE;
		}
	}else
	if (hwndList == pnmh->hwndFrom) {
		switch (pnmh->code) {
		case LVN_COLUMNCLICK:
//			MYTRACE(_T("LVN_COLUMNCLICK\n"));
			nSortCol =  pnlv->iSubItem;
			if (nSortCol == nSortColOld) {
				bSortDesc = !bSortDesc;
			}
			nSortColOld = nSortCol;
			{
				auto type = std::make_unique<TypeConfig>();
				DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), *type);
				type->nOutlineSortCol = nSortCol;
				type->bOutlineSortDesc = bSortDesc;
				SetTypeConfig(TypeConfigNum(nDocType), *type);
			}
			//	Apr. 23, 2005 genta �֐��Ƃ��ēƗ�������
			SortListView(hwndList, nSortCol);
			return TRUE;
		case NM_CLICK:
			if (IsDocking()) {
				OnJump(false, false);
				return TRUE;
			}
			break;
		case NM_DBLCLK:
				OnJump();
			return TRUE;
		case LVN_KEYDOWN:
			if (((LV_KEYDOWN *)lParam)->wVKey == VK_SPACE) {
				OnJump(false);
				return TRUE;
			}
			Key2Command(((LV_KEYDOWN *)lParam)->wVKey);
			return TRUE;
		}
	}

#ifdef DEFINE_SYNCCOLOR
	if (IsDocking()) {
		if (hwndList == pnmh->hwndFrom || hwndTree == pnmh->hwndFrom) {
			if (pnmh->code == NM_CUSTOMDRAW) {
				LPNMCUSTOMDRAW lpnmcd = (LPNMCUSTOMDRAW)lParam;
				switch (lpnmcd->dwDrawStage) {
				case CDDS_PREPAINT:
					::SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					break;
				case CDDS_ITEMPREPAINT:
					{	// �I���A�C�e���𔽓]�\���ɂ���
						const TypeConfig* typeDataPtr = &(pEditView->pEditDoc->docType.GetDocumentAttribute());
						COLORREF clrText = typeDataPtr->colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
						COLORREF clrTextBk = typeDataPtr->colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;
						if (hwndList == pnmh->hwndFrom) {
							//if (lpnmcd->uItemState & CDIS_SELECTED) {	// ��I���̃A�C�e�������ׂ� CDIS_SELECTED �ŗ���H
							if (ListView_GetItemState(hwndList, lpnmcd->dwItemSpec, LVIS_SELECTED)) {
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrText = clrText ^ RGB(255, 255, 255);
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrTextBk = clrTextBk ^ RGB(255, 255, 255);
								lpnmcd->uItemState = 0;	// ���X�g�r���[�ɂ͑I���Ƃ��Ă̕`��������Ȃ��悤�ɂ���H
							}
						}else {
							if (lpnmcd->uItemState & CDIS_SELECTED) {
								((LPNMTVCUSTOMDRAW)lpnmcd)->clrText = clrText ^ RGB(255, 255, 255);
								((LPNMTVCUSTOMDRAW)lpnmcd)->clrTextBk = clrTextBk ^ RGB(255, 255, 255);
							}
						}
					}
					::SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, CDRF_DODEFAULT);
					break;
				}

				return TRUE;
			}
		}
	}
#endif

	return FALSE;
}
/*!
	�w�肳�ꂽ�J�����Ń��X�g�r���[���\�[�g����D
	�����Ƀw�b�_������������D

	�\�[�g��̓t�H�[�J�X����ʓ��Ɍ����悤�ɕ\���ʒu�𒲐�����D

	@par �\���ʒu�����̏��Z
	EnsureVisible�̌��ʂ́C��X�N���[���̏ꍇ�͏�[�ɁC���X�N���[���̏ꍇ��
	���[�ɖړI�̍��ڂ������D�[���班�����������ꍇ�̓I�t�Z�b�g��^����K�v��
	���邪�C�X�N���[���������킩��Ȃ��Ɓ}���킩��Ȃ�
	���̂��ߍŏ��Ɉ�ԉ��Ɉ��X�N���[�������邱�Ƃ�EnsureVisible�ł�
	���Ȃ炸��X�N���[���ɂȂ�悤�ɂ��邱�ƂŁC�\�[�g��̕\���ʒu��
	�Œ肷��

	@param[in] hwndList	���X�g�r���[�̃E�B���h�E�n���h��
	@param[in] sortcol	�\�[�g����J�����ԍ�(0-2)

	@date 2005.04.23 genta �֐��Ƃ��ēƗ�������
	@date 2005.04.29 genta �\�[�g��̕\���ʒu����
	@date 2010.03.17 syat ���ǉ�
*/
void DlgFuncList::SortListView(
	HWND hwndList,
	int sortcol
	)
{
	LV_COLUMN col;
	int col_no;

	//	Apr. 23, 2005 genta �s�ԍ������[��

//	if (sortcol == 1) {
	{
		col_no = FL_COL_NAME;
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = _T("�֐��� *");
		if (nListType == OutlineType::BookMark) {
			col.pszText = const_cast<TCHAR*>(sortcol == col_no ? LS(STR_DLGFNCLST_LIST_TEXT_M) : LS(STR_DLGFNCLST_LIST_TEXT));
		}else {
			col.pszText = const_cast<TCHAR*>(sortcol == col_no ? LS(STR_DLGFNCLST_LIST_FUNC_M) : LS(STR_DLGFNCLST_LIST_FUNC));
		}
	// To Here 2001.12.03 hor
		col.iSubItem = 0;
		ListView_SetColumn(hwndList, col_no, &col);

		col_no = FL_COL_ROW;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>(sortcol == col_no ? LS(STR_DLGFNCLST_LIST_LINE_M) : LS(STR_DLGFNCLST_LIST_LINE));
		col.iSubItem = 0;
		ListView_SetColumn(hwndList, col_no, &col);

		// 2010.03.17 syat ���ǉ�
		col_no = FL_COL_COL;
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>(sortcol == col_no ? LS(STR_DLGFNCLST_LIST_COL_M) : LS(STR_DLGFNCLST_LIST_COL));
		col.iSubItem = 0;
		ListView_SetColumn(hwndList, col_no, &col);

		col_no = FL_COL_REMARK;
	// From Here 2001.12.07 hor
		col.mask = LVCF_TEXT;
		col.pszText = const_cast<TCHAR*>(sortcol == col_no ? LS(STR_DLGFNCLST_LIST_M) : _T(""));
		col.iSubItem = 0;
		ListView_SetColumn(hwndList, col_no, &col);
	// To Here 2001.12.07 hor

		ListView_SortItems(hwndList, (bSortDesc ? CompareFunc_Desc : CompareFunc_Asc), (LPARAM)this);
	}
	//	2005.04.23 zenryaku �I�����ꂽ���ڂ�������悤�ɂ���

	//	Apr. 29, 2005 genta ��U��ԉ��ɃX�N���[��������
	ListView_EnsureVisible(
		hwndList,
		ListView_GetItemCount(hwndList) - 1,
		FALSE
	);
	
	//	Jan.  9, 2006 genta �擪����1�ڂ�2�ڂ̊֐���
	//	�I�����ꂽ�ꍇ�ɃX�N���[������Ȃ�����
	int keypos = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED) - 2;
	ListView_EnsureVisible(
		hwndList,
		keypos >= 0 ? keypos : 0,
		FALSE
	);
}

/*!	�E�B���h�E�T�C�Y���ύX���ꂽ

	@date 2003.06.22 Moca �R�[�h�̐���(�R���g���[���̏������@���e�[�u���Ɏ�������)
	@date 2003.08.16 genta �z���static��(���ʂȏ��������s��Ȃ�����)
*/
BOOL DlgFuncList::OnSize(WPARAM wParam, LPARAM lParam)
{
	// ���̂Ƃ��� EditWnd::OnSize() ����̌Ăяo���ł� lParam �� EditWnd �� �� lParam �̂܂ܓn�����	// 2010.06.05 ryoji
	RECT rcDlg;
	::GetClientRect(GetHwnd(), &rcDlg);
	lParam = MAKELONG(rcDlg.right - rcDlg.left, rcDlg.bottom -  rcDlg.top);	// ���O�ŕ␳

	// ���N���X�����o
	Dialog::OnSize(wParam, lParam);

	RECT  rc;
	POINT ptNew;
	ptNew.x = rcDlg.right - rcDlg.left;
	ptNew.y = rcDlg.bottom - rcDlg.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		HWND hwndCtrl = GetItemHwnd(anchorList[i].id);
		ResizeItem(hwndCtrl, ptDefaultSizeClient, ptNew, rcItems[i], anchorList[i].anchor, (anchorList[i].anchor != AnchorStyle::All));
//	2013.2.6 aroka ������h�~�p�̎��s����
		if (anchorList[i].anchor == AnchorStyle::All) {
			::UpdateWindow(hwndCtrl);
		}
	}

//	if (IsDocking())
	{
		// �_�C�A���O�������ĕ`��i�c���[�^���X�g�͈̔͂͂�����Ȃ��悤�ɏ��O�j
		::InvalidateRect(GetHwnd(), NULL, FALSE);
		POINT pt;
		::GetWindowRect(GetItemHwnd(IDC_TREE_FL), &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		::ScreenToClient(GetHwnd(), &pt);
		::OffsetRect(&rc, pt.x - rc.left, pt.y - rc.top);
		::ValidateRect(GetHwnd(), &rc);
	}
	return TRUE;
}

BOOL DlgFuncList::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = ptDefaultSize.x/2;
	lpmmi->ptMinTrackSize.y = ptDefaultSize.y/3;
	lpmmi->ptMaxTrackSize.x = ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = ptDefaultSize.y*2;
	return 0;
}
int CALLBACK Compare_by_ItemData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1< lParam2) {
		return -1;
	}
	if (lParam1 > lParam2) {
		return 1;
	}else {
		return 0;
	}
}

BOOL DlgFuncList::OnDestroy(void)
{
	Dialog::OnDestroy();

	// �A�E�g���C�� ���ʒu�ƃT�C�Y���L������	// 20060201 aroka
	// �O������FlParam �� Dialog::OnDestroy �ŃN���A����Ȃ�����
	EditView* pEditView = (EditView*)lParam;
	HWND hwndEdit = pEditView->editWnd.GetHwnd();
	if (!IsDocking() && pShareData->common.outline.bRememberOutlineWindowPos) {
		// �e�̃E�B���h�E�ʒu�E�T�C�Y���L��
		WINDOWPLACEMENT windowPlacement;
		windowPlacement.length = sizeof(windowPlacement);
		if (::GetWindowPlacement(hwndEdit, &windowPlacement)) {
			// �E�B���h�E�ʒu�E�T�C�Y���L��
			pShareData->common.outline.xOutlineWindowPos = xPos - windowPlacement.rcNormalPosition.left;
			pShareData->common.outline.yOutlineWindowPos = yPos - windowPlacement.rcNormalPosition.top;
			pShareData->common.outline.widthOutlineWindow = nWidth;
			pShareData->common.outline.heightOutlineWindow = nHeight;
		}
	}

	// �h�b�L���O��ʂ����Ƃ��͉�ʂ��ă��C�A�E�g����
	// �h�b�L���O�ŃA�v���I�����ɂ� hwndEdit �� NULL �ɂȂ��Ă���i�e�ɐ�� WM_DESTROY �������邽�߁j
	if (IsDocking() && hwndEdit) {
		pEditView->editWnd.EndLayoutBars();
	}

	// �����I�ɃA�E�g���C����ʂ�����Ƃ������A�E�g���C���\���t���O�� OFF �ɂ���
	// �t���[�e�B���O�ŃA�v���I������^�u���[�h�ŗ��ɂ���ꍇ�� ::IsWindowVisible(hwndEdit) �� FALSE ��Ԃ�
	if (hwndEdit && ::IsWindowVisible(hwndEdit) && !bInChangeLayout) {	// ChangeLayout() �������͐ݒ�ύX���Ȃ�
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ProfDockDisp() = FALSE;
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);
		}
		// ���E�B���h�E�ɕύX��ʒm����
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);
		}
	}

	if (hwndToolTip) {
		::DestroyWindow(hwndToolTip);
		hwndToolTip = NULL;
	}
	::KillTimer(GetHwnd(), 1);

	return TRUE;
}


BOOL DlgFuncList::OnCbnSelChange(HWND hwndCtl, int wID)
{
	int nSelect = Combo_GetCurSel(hwndCtl);
	switch (wID) {
	case IDC_COMBO_nSortType:
		if (nSortType != nSelect) {
			nSortType = nSelect;
			auto type = std::make_unique<TypeConfig>();
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), *type);
			type->nOutlineSortType = nSortType;
			SetTypeConfig(TypeConfigNum(nDocType), *type);
			SortTree(GetItemHwnd(IDC_TREE_FL), TVI_ROOT);
		}
		return TRUE;
	}
	return FALSE;

}
void  DlgFuncList::SortTree(HWND hWndTree, HTREEITEM htiParent)
{
	if (nSortType == 1) {
		TreeView_SortChildren(hWndTree, htiParent, TRUE);
	}else {
		TVSORTCB sort;
		sort.hParent =  htiParent;
		sort.lpfnCompare = Compare_by_ItemData;
		sort.lParam = 0;
		TreeView_SortChildrenCB(hWndTree , &sort , TRUE);
	}
	for (HTREEITEM htiItem = TreeView_GetChild(hWndTree, htiParent); htiItem; htiItem = TreeView_GetNextSibling(hWndTree, htiItem)) {
		SortTree(hWndTree, htiItem);
	}
}


bool DlgFuncList::TagJumpTimer(
	const TCHAR* pFile,
	Point point,
	bool bCheckAutoClose
	)
{
	EditView* pView = reinterpret_cast<EditView*>(lParam);

	// �t�@�C�����J���Ă��Ȃ��ꍇ�͎����ŊJ��
	if (pView->GetDocument().IsAcceptLoad()) {
		std::wstring strFile = to_wchar(pFile);
		pView->GetCommander().Command_FileOpen( strFile.c_str(), CODE_AUTODETECT, AppMode::getInstance().IsViewMode(), NULL );
		if (point.y != -1) {
			if (pView->GetDocument().docFile.GetFilePathClass().IsValidPath()) {
				Point pt;
				pt.x = point.GetX() - 1;
				pt.y = point.GetY() - 1;
				if (pt.x < 0) {
					pt.x = 0;
				}
				pView->GetCommander().Command_MoveCursor( pt, 0 );
			}
		}
		return true;
	}
	pszTimerJumpFile = pFile;
	pointTimerJump = point;
	bTimerJumpAutoClose = bCheckAutoClose;
	::SetTimer( GetHwnd(), 2, 200, NULL ); // id == 2
	return false;
}


BOOL DlgFuncList::OnJump(
	bool bCheckAutoClose,
	bool bFileJump
	)	// 2002.02.08 hor �����ǉ�
{
	size_t nLineTo;
	size_t nColTo;
	// �_�C�A���O�f�[�^�̎擾
	if (0 < GetData() && (funcInfo || 0 < sJumpFile.size() )) {
		if (bModal) {		// ���[�_�� �_�C�A���O��
			// ���[�_���\������ꍇ�́AfuncInfo���擾����A�N�Z�T���������Č��ʎ擾���邱�ƁB
			::EndDialog(GetHwnd(), 1);
		}else {
			bool bFileJumpSelf = true;
			if (0 < sJumpFile.size()) {
				if (bFileJump) {
					// �t�@�C���c���[�̏ꍇ
					if (bModal) {		// ���[�_�� �_�C�A���O��
						// ���[�_���\������ꍇ�́AfuncInfo���擾����A�N�Z�T���������Č��ʎ擾���邱�ƁB
						::EndDialog(GetHwnd(), 1);
					}
					Point poCaret;
					poCaret.x = -1;
					poCaret.y = -1;
					bFileJumpSelf = TagJumpTimer(sJumpFile.c_str(), poCaret, bCheckAutoClose);
				}
			}else
			if (funcInfo && 0 < funcInfo->memFileName.GetStringLength()) {
				if (bFileJump) {
					nLineTo = funcInfo->nFuncLineCRLF;
					nColTo = funcInfo->nFuncColCRLF;
					// �ʂ̃t�@�C���փW�����v
					Point poCaret; // TagJumpSub��1�J�n
					poCaret.x = (int)nColTo;
					poCaret.y = (int)nLineTo;
					bFileJumpSelf = TagJumpTimer(funcInfo->memFileName.GetStringPtr(), poCaret, bCheckAutoClose);
				}
			}else {
				nLineTo = funcInfo->nFuncLineCRLF;
				nColTo = funcInfo->nFuncColCRLF;
				// �J�[�\�����ړ�������
				Point	poCaret;
				poCaret.x = (int)nColTo - 1;
				poCaret.y = (int)nLineTo - 1;

				pShareData->workBuffer.logicPoint = poCaret;

				//	2006.07.09 genta �ړ����ɑI����Ԃ�ێ�����悤��
				::SendMessage(((EditView*)lParam)->editWnd.GetHwnd(),
					MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			}
			if (bCheckAutoClose && bFileJumpSelf) {
				// �A�E�g���C�� �_�C�A���O�������I�ɕ���
				if (IsDocking()) {
					::PostMessage( ((EditView*)lParam)->GetHwnd(), MYWM_SETACTIVEPANE, 0, 0 );
				}else if (pShareData->common.outline.bAutoCloseDlgFuncList) {
					::DestroyWindow( GetHwnd() );
				}else if (pShareData->common.outline.bFunclistSetFocusOnJump) {
					::SetFocus( ((EditView*)lParam)->GetHwnd() );
				}
			}
		}
	}
	return TRUE;
}


//@@@ 2002.01.18 add start
LPVOID DlgFuncList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


// �L�[������R�}���h�ɕϊ�����w���p�[�֐�
void DlgFuncList::Key2Command(WORD KeyCode)
{
	EditView*	pEditView;
// novice 2004/10/10
	// Shift,Ctrl,Alt�L�[��������Ă�����
	int nIdx = GetCtrlKeyState();
	auto& csKeyBind = pShareData->common.keyBind;
	EFunctionCode nFuncCode = KeyBind::GetFuncCode(
		((WORD)(((BYTE)(KeyCode)) | ((WORD)((BYTE)(nIdx))) << 8)),
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr
	);
	switch (nFuncCode) {
	case F_REDRAW:
		nFuncCode=GetFuncCodeRedraw(nOutlineType);
		// FALLTHROUGH
	case F_OUTLINE:
	case F_OUTLINE_TOGGLE: // 20060201 aroka �t�H�[�J�X������Ƃ��̓����[�h
	case F_BOOKMARK_VIEW:
	case F_FILETREE:
		pEditView=(EditView*)lParam;
		pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0); // �����̕ύX 20060201 aroka

		break;
	case F_BOOKMARK_SET:
		OnJump(false);
		pEditView=(EditView*)lParam;
		pEditView->GetCommander().HandleCommand(nFuncCode, true, 0, 0, 0, 0);

		break;
	case F_COPY:
	case F_CUT:
		OnBnClicked(IDC_BUTTON_COPY);
		break;
	}
}

/*!
	@date 2002.10.05 genta
*/
void DlgFuncList::Redraw(
	OutlineType nOutLineType,
	OutlineType nListType,
	FuncInfoArr* pFuncInfoArr,
	int nCurLine,
	int nCurCol
	)
{
	EditView* pEditView = (EditView*)lParam;
	nDocType = pEditView->GetDocument().docType.GetDocumentType().GetIndex();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
	SyncColor();

	nOutlineType = nOutLineType;
	nListType = nListType;
	pFuncInfoArr = pFuncInfoArr;	// �֐����z��
	nCurLine = nCurLine;				// ���ݍs
	nCurCol = nCurCol;				// ���݌�

	bool bType = (ProfDockSet() != 0);
	if (bType) {
		type.nDockOutline = nOutlineType;
		SetTypeConfig( TypeConfigNum(nDocType), type );
	}else {
		CommonSet().nDockOutline = nOutlineType;
	}

	SetData();
}

// �_�C�A���O�^�C�g���̐ݒ�
void DlgFuncList::SetWindowText(const TCHAR* szTitle)
{
	::SetWindowText(GetHwnd(), szTitle);
}

/** �z�F�K�p����
	@date 2010.06.05 ryoji �V�K�쐬
*/
void DlgFuncList::SyncColor(void)
{
	if (!IsDocking()) {
		return;
	}
#ifdef DEFINE_SYNCCOLOR
	// �e�L�X�g�F�E�w�i�F���r���[�Ɠ��F�ɂ���
	EditView* pEditView = (EditView*)lParam;
	const TypeConfig* pTypeData = &(pEditView->pEditDoc->docType.GetDocumentAttribute());
	COLORREF clrText = pTypeData->colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
	COLORREF clrBack = pTypeData->colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	TreeView_SetTextColor(hwndTree, clrText);
	TreeView_SetBkColor(hwndTree, clrBack);
	{
		// WinNT4.0 ������ł̓E�B���h�E�X�^�C���������I�ɍĐݒ肵�Ȃ���
		// �c���[�A�C�e���̍������^�����ɂȂ�
		LONG lStyle = (LONG)GetWindowLongPtr(hwndTree, GWL_STYLE);
		SetWindowLongPtr(hwndTree, GWL_STYLE, lStyle & ~(TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT));
		SetWindowLongPtr(hwndTree, GWL_STYLE, lStyle);
	}
	::SetWindowPos(hwndTree, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);	// �Ȃ����������Ȃ��Ǝl�ӂP�h�b�g���������F�ύX�������K�p����Ȃ��i���X�^�C���Đݒ�Ƃ͖��֌W�j
	::InvalidateRect(hwndTree, NULL, TRUE);

	HWND hwndList = GetItemHwnd(IDC_LIST_FL);
	ListView_SetTextColor(hwndList, clrText);
	ListView_SetTextBkColor(hwndList, clrBack);
	ListView_SetBkColor(hwndList, clrBack);
	::InvalidateRect(hwndList, NULL, TRUE);
#endif
}

/** �h�b�L���O�Ώۋ�`�̎擾�i�X�N���[�����W�j
	@date 2010.06.05 ryoji �V�K�쐬
*/
void DlgFuncList::GetDockSpaceRect(LPRECT pRect)
{
	EditView* pEditView = (EditView*)lParam;
	// DlgFuncList �� CSplitterWnd �̊O�ڋ�`
	// 2014.12.02 �~�j�}�b�v�Ή�
	HWND hwnd[3];
	RECT rc[3];
	hwnd[0] = ::GetParent( pEditView->GetHwnd() );	// CSplitterWnd
	int nCount = 1;
	if (IsDocking()) {
		hwnd[nCount] = GetHwnd();
		++nCount;
	}
	hwnd[nCount] = pEditView->editWnd.GetMiniMap().GetHwnd();
	if (hwnd[nCount]) {
		++nCount;
	}
	for (int i=0; i<nCount; ++i) {
		::GetWindowRect(hwnd[i], &rc[i]);
	}
	if (nCount == 1) {
		*pRect = rc[0];
	}else if (nCount == 2) {
		::UnionRect(pRect, &rc[0], &rc[1]);
	}else {
		RECT rcTemp;
		::UnionRect(&rcTemp, &rc[0], &rc[1]);
		::UnionRect(pRect, &rcTemp, &rc[2]);
	}
}

/**�L���v�V������`�擾�i�X�N���[�����W�j
	@date 2010.06.05 ryoji �V�K�쐬
*/
void DlgFuncList::GetCaptionRect(LPRECT pRect)
{
	RECT rc;
	GetWindowRect(&rc);
	DockSideType dockSideType = GetDockSide();
	pRect->left = rc.left + ((dockSideType == DockSideType::Right)? DOCK_SPLITTER_WIDTH: 0);
	pRect->top = rc.top + ((dockSideType == DockSideType::Bottom)? DOCK_SPLITTER_WIDTH: 0);
	pRect->right = rc.right - ((dockSideType == DockSideType::Left)? DOCK_SPLITTER_WIDTH: 0);
	pRect->bottom = pRect->top + (::GetSystemMetrics(SM_CYSMCAPTION) + 1);
}

/** �L���v�V������̃{�^����`�擾�i�X�N���[�����W�j
	@date 2010.06.05 ryoji �V�K�쐬
*/
bool DlgFuncList::GetCaptionButtonRect(int nButton, LPRECT pRect)
{
	if (!IsDocking()) {
		return false;
	}
	if (nButton >= DOCK_BUTTON_NUM) {
		return false;
	}
	GetCaptionRect(pRect);
	::OffsetRect(pRect, 0, 1);
	int cx = ::GetSystemMetrics(SM_CXSMSIZE);
	pRect->left = pRect->right - cx * (nButton + 1);
	pRect->right = pRect->left + cx;
	pRect->bottom = pRect->top + ::GetSystemMetrics(SM_CYSMSIZE);
	return true;
}

/** �����o�[�ւ̃q�b�g�e�X�g�i�X�N���[�����W�j
	@date 2010.06.05 ryoji �V�K�쐬
*/
bool DlgFuncList::HitTestSplitter(int xPos, int yPos)
{
	if (!IsDocking()) {
		return false;
	}
	bool bRet = false;
	RECT rc;
	GetWindowRect(&rc);

	DockSideType dockSideType = GetDockSide();
	switch (dockSideType) {
	case DockSideType::Left:	bRet = (rc.right - xPos < DOCK_SPLITTER_WIDTH);		break;
	case DockSideType::Top:		bRet = (rc.bottom - yPos < DOCK_SPLITTER_WIDTH);	break;
	case DockSideType::Right:	bRet = (xPos - rc.left< DOCK_SPLITTER_WIDTH);		break;
	case DockSideType::Bottom:	bRet = (yPos - rc.top < DOCK_SPLITTER_WIDTH);		break;
	}

	return bRet;
}

/** �L���v�V������̃{�^���ւ̃q�b�g�e�X�g�i�X�N���[�����W�j
	@date 2010.06.05 ryoji �V�K�쐬
*/
int DlgFuncList::HitTestCaptionButton(int xPos, int yPos)
{
	if (!IsDocking()) {
		return -1;
	}

	POINT pt;
	pt.x = xPos;
	pt.y = yPos;

	RECT rcBtn;
	GetCaptionRect(&rcBtn);
	::OffsetRect(&rcBtn, 0, 1);
	rcBtn.left = rcBtn.right - ::GetSystemMetrics(SM_CXSMSIZE);
	rcBtn.bottom = rcBtn.top + ::GetSystemMetrics(SM_CYSMSIZE);
	int nBtn = -1;
	for (int i=0; i<DOCK_BUTTON_NUM; ++i) {
		if (::PtInRect(&rcBtn, pt)) {
			nBtn = i;	// �E�[���� i �Ԗڂ̃{�^����
			break;
		}
		::OffsetRect(&rcBtn, -(rcBtn.right - rcBtn.left), 0);
	}

	return nBtn;
}

/** WM_NCCALCSIZE ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!IsDocking()) {
		return 0L;
	}

	// ���E�B���h�E�̃N���C�A���g�̈���`����
	// ����ŃL���v�V�����╪���o�[���N���C�A���g�̈�ɂ��邱�Ƃ��ł���
	NCCALCSIZE_PARAMS* pNCS = (NCCALCSIZE_PARAMS*)lParam;
	pNCS->rgrc[0].top += (::GetSystemMetrics(SM_CYSMCAPTION) + 1);
	switch (GetDockSide()) {
	case DockSideType::Left:	pNCS->rgrc[0].right -= DOCK_SPLITTER_WIDTH;		break;
	case DockSideType::Top:		pNCS->rgrc[0].bottom -= DOCK_SPLITTER_WIDTH;	break;
	case DockSideType::Right:	pNCS->rgrc[0].left += DOCK_SPLITTER_WIDTH;		break;
	case DockSideType::Bottom:	pNCS->rgrc[0].top += DOCK_SPLITTER_WIDTH;		break;
	}
	return 1L;
}

/** WM_NCHITTEST ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnNcHitTest(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (!IsDocking()) {
		return 0L;
	}

	INT_PTR nRet = HTERROR;
	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;
	if (HitTestSplitter(pt.x, pt.y)) {
		switch (GetDockSide()) {
		case DockSideType::Left:	nRet = HTRIGHT;		break;
		case DockSideType::Top:		nRet = HTBOTTOM;	break;
		case DockSideType::Right:	nRet = HTLEFT;		break;
		case DockSideType::Bottom:	nRet = HTTOP;		break;
		}
	}else {
		RECT rc;
		GetCaptionRect(&rc);
		nRet = ::PtInRect(&rc, pt) ? HTCAPTION: HTCLIENT;
	}
	::SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, nRet);

	return nRet;
}

/** WM_TIMER ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnTimer(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (wParam == 2) {
		EditView* pView = reinterpret_cast<EditView*>(this->lParam);
		if (pszTimerJumpFile) {
			const TCHAR* pszFile = pszTimerJumpFile;
			pszTimerJumpFile = NULL;
			bool bSelf = false;
			pView->TagJumpSub( pszFile, pointTimerJump, false, false, &bSelf );
			if (bTimerJumpAutoClose) {
				if (IsDocking()) {
					if (bSelf) {
						::PostMessage( pView->GetHwnd(), MYWM_SETACTIVEPANE, 0, 0 );
					}
				}else if (pShareData->common.outline.bAutoCloseDlgFuncList) {
					::DestroyWindow( GetHwnd() );
				}else if (pShareData->common.outline.bFunclistSetFocusOnJump) {
					if (bSelf) {
						::SetFocus( pView->GetHwnd() );
					}
				}
			}
		}
		::KillTimer(hwnd, 2);
		return 0L;
	}
	if (!IsDocking()) {
		return 0L;
	}
	if (wParam == 1) {
		// �J�[�\�����E�B���h�E�O�ɂ���ꍇ�ɂ� WM_NCMOUSEMOVE �𑗂�
		POINT pt;
		RECT rc;
		::GetCursorPos(&pt);
		::GetWindowRect(hwnd, &rc);
		if (!::PtInRect(&rc, pt)) {
			::SendMessage(hwnd, WM_NCMOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
		}
	}

	return 0L;
}

/** WM_NCMOUSEMOVE ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnNcMouseMove(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (!IsDocking()) {
		return 0L;
	}

	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;

	// �J�[�\�����E�B���h�E���ɓ�������^�C�}�[�N��
	// �E�B���h�E�O�ɏo����^�C�}�[�폜
	RECT rc;
	GetWindowRect(&rc);
	bool bHovering = ::PtInRect(&rc, pt) ? true: false;
	if (this->bHovering != bHovering) {
		this->bHovering = bHovering;
		if (bHovering) {
			::SetTimer(hwnd, 1, 200, NULL);
		}else {
			::KillTimer(hwnd, 1);
		}
	}

	// �}�E�X�J�[�\�����{�^����ɂ���΃n�C���C�g
	int nHilightedBtn = HitTestCaptionButton(pt.x, pt.y);
	if (this->nHilightedBtn != nHilightedBtn) {
		// �n�C���C�g��Ԃ̕ύX�𔽉f���邽�߂ɍĕ`�悷��
		this->nHilightedBtn = nHilightedBtn;
		::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);

		// �c�[���`�b�v�X�V
		TOOLINFO ti = {0};
		ti.cbSize	= CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.hwnd		= GetHwnd();
		ti.hinst	= hInstance;
		ti.uId		= (UINT_PTR)GetHwnd();
		switch (nHilightedBtn) {
		case 0: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_CLOSE)); break;
		case 1: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_WIN)); break;
		case 2: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_UPDATE)); break;
		default: ti.lpszText = NULL;	// ����
		}
		Tooltip_UpdateTipText(hwndToolTip, &ti);
	}

	return 0L;
}

/** WM_MOUSEMOVE ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnMouseMove(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (!IsDocking()) {
		return 0L;
	}

	if (bStretching) {	// �}�E�X�̃h���b�O�ʒu�ɂ��킹�ăT�C�Y��ύX����
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		::ClientToScreen(GetHwnd(), &pt);

		RECT rc;
		GetDockSpaceRect(&rc);

		// ��ʃT�C�Y������������Ƃ��͉������Ȃ�
		DockSideType dockSideType = GetDockSide();
		if (dockSideType == DockSideType::Left || dockSideType == DockSideType::Right) {
			if (rc.right - rc.left < DOCK_MIN_SIZE) {
				return 0L;
			}
		}else {
			if (rc.bottom - rc.top < DOCK_MIN_SIZE) {
				return 0L;
			}
		}

		// �}�E�X���㉺���E�ɍs���߂��Ȃ�͈͓��ɒ�������
		if (pt.x > rc.right - DOCK_MIN_SIZE)	pt.x = rc.right - DOCK_MIN_SIZE;
		if (pt.x < rc.left + DOCK_MIN_SIZE)	pt.x = rc.left + DOCK_MIN_SIZE;
		if (pt.y > rc.bottom - DOCK_MIN_SIZE)	pt.y = rc.bottom - DOCK_MIN_SIZE;
		if (pt.y < rc.top + DOCK_MIN_SIZE)		pt.y = rc.top + DOCK_MIN_SIZE;

		// �N���C�A���g���W�n�ɕϊ����ĐV�����ʒu�ƃT�C�Y���v�Z����
		POINT ptLT;
		ptLT.x = rc.left;
		ptLT.y = rc.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rc, ptLT.x - rc.left, ptLT.y - rc.top);
		::ScreenToClient(hwndParent, &pt);
		switch (dockSideType) {
		case DockSideType::Left:	rc.right = pt.x - DOCK_SPLITTER_WIDTH / 2 + DOCK_SPLITTER_WIDTH;	break;
		case DockSideType::Top:		rc.bottom = pt.y - DOCK_SPLITTER_WIDTH / 2 + DOCK_SPLITTER_WIDTH;	break;
		case DockSideType::Right:	rc.left = pt.x - DOCK_SPLITTER_WIDTH / 2;	break;
		case DockSideType::Bottom:	rc.top = pt.y - DOCK_SPLITTER_WIDTH / 2;	break;
		}

		// �ȑO�Ɠ����z�u�Ȃ疳�ʂɈړ����Ȃ�
		RECT rcOld;
		GetWindowRect(&rcOld);
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top);
		if (::EqualRect(&rcOld, &rc)) {
			return 0L;
		}

		// �ړ�����
		::SetWindowPos(GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		((EditView*)lParam)->editWnd.EndLayoutBars(bEditWndReady);

		// �ړ���̔z�u�����L������
		GetWindowRect(&rc);
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		switch (GetDockSide()) {
		case DockSideType::Left:	ProfDockLeft() = rc.right - rc.left;	break;
		case DockSideType::Top:		ProfDockTop() = rc.bottom - rc.top;		break;
		case DockSideType::Right:	ProfDockRight() = rc.right - rc.left;	break;
		case DockSideType::Bottom:	ProfDockBottom() = rc.bottom - rc.top;	break;
		}
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);
		}
		return 1L;
	}

	return 0L;
}

/** WM_NCLBUTTONDOWN ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnNcLButtonDown(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	POINT pt;
	pt.x = MAKEPOINTS(lParam).x;
	pt.y = MAKEPOINTS(lParam).y;

	if (!IsDocking()) {
		if (GetDockSide() == DockSideType::Float) {
			if (wParam == HTCAPTION  && !::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd())) {
				::SetActiveWindow(GetHwnd());
				// ��� SetActiveWindow() �� WM_ACTIVATEAPP �֍s���P�[�X�ł́AWM_ACTIVATEAPP �ɓ��ꂽ���ꏈ���i�G�f�B�^�{�̂��ꎞ�I�ɃA�N�e�B�u�����Ė߂��j
				// �ɗ]�v�Ɏ��Ԃ������邽�߁A��� SetActiveWindow() ��ɂ̓{�^����������Ă��邱�Ƃ�����B���̏ꍇ�� Track() ���J�n�����ɔ�����B
				if ((::GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON)? VK_RBUTTON: VK_LBUTTON) & 0x8000) == 0) {
					return 1L;	// �{�^���͊��ɗ�����Ă���
				}
				Track(pt);	// �^�C�g���o�[�̃h���b�O���h���b�v�ɂ��h�b�L���O�z�u�ύX
				return 1L;
			}
		}
		return 0L;
	}

	int nBtn;
	if (HitTestSplitter(pt.x, pt.y)) {	// �����o�[
		bStretching = true;
		::SetCapture(GetHwnd());	// OnMouseMove�ł̃T�C�Y�����̂��߂Ɏ��O�̃L���v�`�����K�v
	}else {
		if ((nBtn = HitTestCaptionButton(pt.x, pt.y)) >= 0) {	// �L���v�V������̃{�^��
			if (nBtn == 1) {	// ���j���[
				RECT rcBtn;
				GetCaptionButtonRect(nBtn, &rcBtn);
				pt.x = rcBtn.left;
				pt.y = rcBtn.bottom;
				DoMenu(pt, GetHwnd());
				// ���j���[�I�������Ƀ��X�g��c���[���N���b�N������{�^�����n�C���C�g�̂܂܂ɂȂ�̂ōX�V
				::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);
			}else {
				nCapturingBtn = nBtn;
				::SetCapture(GetHwnd());
			}
		}else {	// �c��̓^�C�g���o�[�̂�
			Track(pt);	// �^�C�g���o�[�̃h���b�O���h���b�v�ɂ��h�b�L���O�z�u�ύX
		}
	}

	return 1L;
}

/** WM_LBUTTONUP ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnLButtonUp(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (!IsDocking()) {
		return 0L;
	}

	if (bStretching) {
		::ReleaseCapture();
		bStretching = false;

		if (ProfDockSync()) {
			// ���E�B���h�E�ɕύX��ʒm����
			HWND hwndEdit = ((EditView*)lParam)->editWnd.GetHwnd();
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);
		}
		return 1L;
	}

	if (nCapturingBtn >= 0) {
		::ReleaseCapture();
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		::ClientToScreen(GetHwnd(), &pt);
		int nBtn = HitTestCaptionButton(pt.x, pt.y);
		if (nBtn == nCapturingBtn) {
			if (nBtn == 0) {	// ����
				::DestroyWindow(GetHwnd());
			}else if (nCapturingBtn == 2) {	// �X�V
				EFunctionCode nFuncCode = GetFuncCodeRedraw(nOutlineType);
				EditView* pEditView = (EditView*)(this->lParam);
				pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0);
			}
		}
		nCapturingBtn = -1;
		return 1L;
	}

	return 0L;
}

/** WM_NCPAINT ����
	@date 2010.06.05 ryoji �V�K�쐬
*/
INT_PTR DlgFuncList::OnNcPaint(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	if (!IsDocking()) {
		return 0L;
	}

	DockSideType dockSideType = GetDockSide();

	HDC hdc;
	RECT rc, rcScr, rcWk;

	// �`��Ώ�
	hdc = ::GetWindowDC(hwnd);
	Graphics gr(hdc);
	::GetWindowRect(hwnd, &rcScr);
	rc = rcScr;
	::OffsetRect(&rc, -rcScr.left, -rcScr.top);

	// �w�i��`�悷��
	//::FillRect(gr, &rc, (HBRUSH)(COLOR_3DFACE + 1));

	// ��������`�悷��
	rcWk = rc;
	switch (dockSideType) {
	case DockSideType::Left:	rcWk.left = rcWk.right - DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Top:		rcWk.top = rcWk.bottom - DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Right:	rcWk.right = rcWk.left + DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Bottom:	rcWk.bottom = rcWk.top + DOCK_SPLITTER_WIDTH; break;
	}
	::FillRect(gr, &rcWk, (HBRUSH)(COLOR_3DFACE + 1));
	::DrawEdge(gr, &rcWk, EDGE_ETCHED, BF_TOPLEFT);

	// �^�C�g����`�悷��
	BOOL bThemeActive = UxTheme::getInstance().IsThemeActive();
	BOOL bGradient = FALSE;
	::SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &bGradient, 0);
	if (!bThemeActive) {
		bGradient = FALSE;	// �K���ɒ���
	}
	HWND hwndFocus = ::GetFocus();
	BOOL bActive = (GetHwnd() == hwndFocus || ::IsChild(GetHwnd(), hwndFocus));
	RECT rcCaption;
	GetCaptionRect(&rcCaption);
	::OffsetRect(&rcCaption, -rcScr.left, -rcScr.top);
	rcWk = rcCaption;
	rcWk.top += 1;
	rcWk.right -= DOCK_BUTTON_NUM * (::GetSystemMetrics(SM_CXSMSIZE));
	// ��DrawCaption() �� DC_SMALLCAP ���w�肵�Ă͂����Ȃ����ۂ�
	// ��DC_SMALLCAP �w��̂��̂� Win7(64bit��) �œ������Ă݂���`��ʒu�����ɂ���ď㔼�����������Ȃ������ix86�r���h/x64�r���h�̂ǂ���� NG�j
	::DrawCaption(hwnd, gr, &rcWk, DC_TEXT | (bGradient? DC_GRADIENT: 0) /*| DC_SMALLCAP*/ | (bActive? DC_ACTIVE: 0));
	rcWk.left = rcCaption.right;
	int nClrCaption;
	if (bGradient) {
		nClrCaption = (bActive? COLOR_GRADIENTACTIVECAPTION: COLOR_GRADIENTINACTIVECAPTION);
	}else {
		nClrCaption = (bActive? COLOR_ACTIVECAPTION: COLOR_INACTIVECAPTION);
	}
	::FillRect(gr, &rcWk, ::GetSysColorBrush(nClrCaption));
	::DrawEdge(gr, &rcCaption, BDR_SUNKENOUTER, BF_TOP);

	// �^�C�g����̃{�^����`�悷��
	NONCLIENTMETRICS ncm;
	ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);	// �ȑO�̃v���b�g�t�H�[���� WINVER >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0);
	LOGFONT lf = {0};
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = ncm.lfCaptionFont.lfHeight;
	::lstrcpy(lf.lfFaceName, _T("Marlett"));
	HFONT hFont = ::CreateFontIndirect(&lf);
	::lstrcpy(lf.lfFaceName, _T("Wingdings"));
	HFONT hFont2 = ::CreateFontIndirect(&lf);
	gr.SetTextBackTransparent(true);

	static const TCHAR szBtn[DOCK_BUTTON_NUM] = { (TCHAR)0x72/* ���� */, (TCHAR)0x36/* ���j���[ */, (TCHAR)0xFF/* �X�V */ };
	HFONT hFontBtn[DOCK_BUTTON_NUM] = { hFont/* ���� */, hFont/* ���j���[ */, hFont2/* �X�V */ };
	POINT pt;
	::GetCursorPos(&pt);
	pt.x -= rcScr.left;
	pt.y -= rcScr.top;
	RECT rcBtn = rcCaption;
	::OffsetRect(&rcBtn, 0, 1);
	rcBtn.left = rcBtn.right - ::GetSystemMetrics(SM_CXSMSIZE);
	rcBtn.bottom = rcBtn.top + ::GetSystemMetrics(SM_CYSMSIZE);
	for (int i=0; i<DOCK_BUTTON_NUM; ++i) {
		int nClrCaptionText;
		// �}�E�X�J�[�\�����{�^����ɂ���΃n�C���C�g
		if (::PtInRect(&rcBtn, pt)) {
			::FillRect(gr, &rcBtn, ::GetSysColorBrush((bGradient && !bActive)? COLOR_INACTIVECAPTION: COLOR_ACTIVECAPTION));
			nClrCaptionText = ((bGradient && !bActive)? COLOR_INACTIVECAPTIONTEXT: COLOR_CAPTIONTEXT);
		}else {
			nClrCaptionText = (bActive? COLOR_CAPTIONTEXT: COLOR_INACTIVECAPTIONTEXT);
		}
		gr.PushMyFont(hFontBtn[i]);
		::SetTextColor(gr, ::GetSysColor(nClrCaptionText));
		::DrawText(gr, &szBtn[i], 1, &rcBtn, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		::OffsetRect(&rcBtn, -(rcBtn.right - rcBtn.left), 0);
		gr.PopMyFont();
	}

	::DeleteObject(hFont);
	::DeleteObject(hFont2);

	::ReleaseDC(hwnd, hdc);
	return 1L;
}

/** ���j���[����
	@date 2010.06.05 ryoji �V�K�쐬
*/
void DlgFuncList::DoMenu(POINT pt, HWND hwndFrom)
{
	// ���j���[���쐬����
	EditView* pEditView = &EditDoc::GetInstance(0)->pEditWnd->GetActiveView();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
	DockSideType dockSideType = ProfDockSide();	// �ݒ��̔z�u
	UINT uFlags = MF_BYPOSITION | MF_STRING;
	HMENU hMenu = ::CreatePopupMenu();
	HMENU hMenuSub = ::CreatePopupMenu();
	int iPos = 0;
	int iPosSub = 0;
	HMENU& hMenuRef = (hwndFrom == GetHwnd())? hMenu: hMenuSub;
	int& iPosRef = (hwndFrom == GetHwnd())? iPos: iPosSub;

	if (hwndFrom != GetHwnd()) {
		// �����A������ hwndFrom �ɉ������󋵈ˑ����j���[��ǉ�����Ƃ�������
		// �i�c���[�Ȃ�u���ׂēW�J�v�^�u���ׂďk���v�Ƃ��A���������́j
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING, 450, LS(STR_DLGFNCLST_MENU_UPDATE));
		int flag = 0;
		if (!::IsWindowEnabled( GetItemHwnd(IDC_BUTTON_COPY) )) {
			flag |= MF_GRAYED;
		}
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | flag, 451, LS(STR_DLGFNCLST_MENU_COPY));
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuSub,	LS(STR_DLGFNCLST_MENU_WINPOS));
	}

	int iFrom = iPosRef;
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Left,       LS(STR_DLGFNCLST_MENU_LEFTDOC));
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Right,      LS(STR_DLGFNCLST_MENU_RIGHTDOC));
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Top,        LS(STR_DLGFNCLST_MENU_TOPDOC));
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Bottom,     LS(STR_DLGFNCLST_MENU_BOTDOC));
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Float,      LS(STR_DLGFNCLST_MENU_FLOATING));
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 100 + (int)DockSideType::Undockable, LS(STR_DLGFNCLST_MENU_NODOCK));
	int iTo = iPosRef - 1;
	for (int i=iFrom; i<=iTo; ++i) {
		if (::GetMenuItemID(hMenuRef, i) == (100 + (int)dockSideType)) {
			::CheckMenuRadioItem(hMenuRef, iFrom, iTo, i, MF_BYPOSITION);
			break;
		}
	}
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
	::InsertMenu(hMenuRef, iPosRef++, uFlags, 200, LS(STR_DLGFNCLST_MENU_SYNC));
	::CheckMenuItem(hMenuRef, 200, (MF_BYCOMMAND | ProfDockSync()) ? MF_CHECKED: MF_UNCHECKED);
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 300, LS(STR_DLGFNCLST_MENU_INHERIT));
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 301, LS(STR_DLGFNCLST_MENU_TYPE));
	::CheckMenuRadioItem(hMenuRef, 300, 301, (ProfDockSet() == 0)? 300: 301, MF_BYCOMMAND);
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
	::InsertMenu(hMenuRef, iPosRef++, MF_BYPOSITION | MF_STRING, 305, LS(STR_DLGFNCLST_MENU_UNIFY));

	if (hwndFrom != GetHwnd()) {
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING, 452, LS(STR_DLGFNCLST_MENU_CLOSE));
	}

	// ���j���[��\������
	RECT rcWork;
	GetMonitorWorkRect(pt, &rcWork);	// ���j�^�̃��[�N�G���A
	int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
								(pt.x > rcWork.left)? pt.x: rcWork.left,
								(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
								0, GetHwnd(), NULL);
	::DestroyMenu(hMenu);	// �T�u���j���[�͍ċA�I�ɔj�������

	// ���j���[�I�����ꂽ��Ԃɐ؂�ւ���
	HWND hwndEdit = pEditView->editWnd.GetHwnd();
	if (nId == 450) {	// �X�V
		EFunctionCode nFuncCode = GetFuncCodeRedraw(nOutlineType);
		EditView* pEditView = (EditView*)(this->lParam);
		pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0);
	}else if (nId == 451) {	// �R�s�[
		// Windows�N���b�v�{�[�h�ɃR�s�[ 
		SetClipboardText(GetHwnd(), memClipText.GetStringPtr(), memClipText.GetStringLength());
	}else if (nId == 452) {	// ����
		::DestroyWindow(GetHwnd());
	}else if (nId == 300 || nId == 301) {	// �h�b�L���O�z�u�̌p�����@
		ProfDockSet() = nId - 300;
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// �������g�ւ̋����ύX
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// ���E�B���h�E�Ƀh�b�L���O�z�u�ύX��ʒm����
		}
	}else if (nId == 305) {	// �ݒ�R�s�[
		if (::MYMESSAGEBOX(
				hwndEdit,
				MB_OKCANCEL | MB_ICONINFORMATION, GSTR_APPNAME,
				LS(STR_DLGFNCLST_UNIFY)
			) == IDOK
		) {
			CommonSet().bOutlineDockDisp = (GetHwnd() != NULL);
			CommonSet().eOutlineDockSide = GetDockSide();
			if (GetHwnd()) {
				RECT rc;
				GetWindowRect(&rc);
				switch (GetDockSide()) {	// ���݂̃h�b�L���O���[�h
				case DockSideType::Left:	CommonSet().cxOutlineDockLeft = rc.right - rc.left;	break;
				case DockSideType::Top:		CommonSet().cyOutlineDockTop = rc.bottom - rc.top;	break;
				case DockSideType::Right:	CommonSet().cxOutlineDockRight = rc.right - rc.left;	break;
				case DockSideType::Bottom:	CommonSet().cyOutlineDockBottom = rc.bottom - rc.top;	break;
				}
			}
			auto type = std::make_unique<TypeConfig>();
			for (int i=0; i<GetDllShareData().nTypesCount; ++i) {
				DocTypeManager().GetTypeConfig(TypeConfigNum(i), *type);
				type->bOutlineDockDisp = CommonSet().bOutlineDockDisp;
				type->eOutlineDockSide = CommonSet().eOutlineDockSide;
				type->cxOutlineDockLeft = CommonSet().cxOutlineDockLeft;
				type->cyOutlineDockTop = CommonSet().cyOutlineDockTop;
				type->cxOutlineDockRight = CommonSet().cxOutlineDockRight;
				type->cyOutlineDockBottom = CommonSet().cyOutlineDockBottom;
				DocTypeManager().SetTypeConfig(TypeConfigNum(i), *type);
			}
			ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// �������g�ւ̋����ύX
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// ���E�B���h�E�Ƀh�b�L���O�z�u�ύX��ʒm����
		}
	}else if (nId == 200) {	// �h�b�L���O�z�u�̓������Ƃ�
		ProfDockSync() = !ProfDockSync();
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// �������g�ւ̋����ύX
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// ���E�B���h�E�Ƀh�b�L���O�z�u�ύX��ʒm����
		}
	}else if (nId >= 100 - 1) {	// �h�b�L���O���[�h �i�� DockSideType::Undockable �� -1 �ł��j */
		int* pnWidth = NULL;
		int* pnHeight = NULL;
		RECT rc;
		GetDockSpaceRect(&rc);
		dockSideType = DockSideType(nId - 100);	// �V�����h�b�L���O���[�h
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		if (dockSideType > DockSideType::Float) {
			switch (dockSideType) {
			case DockSideType::Left:	pnWidth = &ProfDockLeft();		break;
			case DockSideType::Top:		pnHeight = &ProfDockTop();		break;
			case DockSideType::Right:	pnWidth = &ProfDockRight();		break;
			case DockSideType::Bottom:	pnHeight = &ProfDockBottom();	break;
			}
			if (dockSideType == DockSideType::Left || dockSideType == DockSideType::Right) {
				if (*pnWidth == 0) {	// ����
					*pnWidth = (rc.right - rc.left) / 3;
				}
				if (*pnWidth > rc.right - rc.left - DOCK_MIN_SIZE) {
					*pnWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				}
				if (*pnWidth < DOCK_MIN_SIZE) {
					*pnWidth = DOCK_MIN_SIZE;
				}
			}else {
				if (*pnHeight == 0) {	// ����
					*pnHeight = (rc.bottom - rc.top) / 3;
				}
				if (*pnHeight > rc.bottom - rc.top - DOCK_MIN_SIZE) {
					*pnHeight = rc.bottom - rc.top - DOCK_MIN_SIZE;
				}
				if (*pnHeight < DOCK_MIN_SIZE) {
					*pnHeight = DOCK_MIN_SIZE;
				}
			}
		}

		// �h�b�L���O�z�u�ύX
		ProfDockDisp() = GetHwnd()? TRUE: FALSE;
		ProfDockSide() = dockSideType;	// �V�����h�b�L���O���[�h��K�p
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// �������g�ւ̋����ύX
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// ���E�B���h�E�Ƀh�b�L���O�z�u�ύX��ʒm����
		}
	}
}

/** ���݂̐ݒ�ɉ����ĕ\�������V����
	@date 2010.06.05 ryoji �V�K�쐬
*/
void DlgFuncList::Refresh(void)
{
	EditWnd* pEditWnd = EditDoc::GetInstance(0)->pEditWnd;
	BOOL bReloaded = ChangeLayout(OUTLINE_LAYOUT_FILECHANGED);	// ���ݐݒ�ɏ]���ăA�E�g���C����ʂ��Ĕz�u����
	if (!bReloaded && pEditWnd->dlgFuncList.GetHwnd()) {
		OutlineType nOutlineType = GetOutlineTypeRedraw(this->nOutlineType);
		pEditWnd->GetActiveView().GetCommander().Command_FuncList(ShowDialogType::Reload, nOutlineType);	// �J��	�� HandleCommand(F_OUTLINE,...) ���ƈ��Preview��ԂŎ��s����Ȃ��̂� Command_FUNCLIST()
	}
	if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == pEditWnd->GetHwnd()) {
		::SetFocus(pEditWnd->GetActiveView().GetHwnd());	// �t�H�[�J�X��߂�
	}
}

/** ���݂̐ݒ�ɉ����Ĕz�u��ύX����i�ł������ĉ�͂��Ȃ��j

	@param nId [in] ����w��DOUTLINE_LAYOUT_FOREGROUND: �O�ʗp�̓��� / OUTLINE_LAYOUT_BACKGROUND: �w��p�̓��� / OUTLINE_LAYOUT_FILECHANGED: �t�@�C���ؑ֗p�̓���i�O�ʂ�������j
	@retval ��͂����s�������ǂ����Dtrue: ���s���� / false: ���s���Ȃ�����

	@date 2010.06.10 ryoji �V�K�쐬
*/
bool DlgFuncList::ChangeLayout(int nId)
{
	struct AutoSwitch {
		AutoSwitch(bool* pbSwitch): pbSwitch(pbSwitch) { *pbSwitch = true; }
		~AutoSwitch() { *pbSwitch = false; }
		bool* pbSwitch;
	} autoSwitch(&bInChangeLayout);	// �������� InChangeLayout �t���O�� ON �ɂ��Ă���

	EditDoc* pDoc = EditDoc::GetInstance(0);	// ���͔�\����������Ȃ��̂� (EditView*)lParam �͎g���Ȃ�
	nDocType = pDoc->docType.GetDocumentType().GetIndex();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);

	BOOL bDockDisp = ProfDockDisp();
	DockSideType eDockSideNew = ProfDockSide();

	if (!GetHwnd()) {	// ���݂͔�\��
		if (bDockDisp) {	// �V�ݒ�͕\��
			if (eDockSideNew <= DockSideType::Float) {
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// ���ł̓t���[�e�B���O�͊J���Ȃ��i�]���݊��j�������ɊJ���ƃ^�u���[�h���͉�ʂ��؂�ւ���Ă��܂�
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED) {
					return false;	// �t�@�C���ؑւł̓t���[�e�B���O�͊J���Ȃ��i�]���݊��j
				}
			}
			// �� ���ł͈ꎞ�I�� Disable �����Ă����ĊJ���i�^�u���[�h�ł̕s���ȉ�ʐ؂�ւ��}�~�j
			EditView* pEditView = &pDoc->pEditWnd->GetActiveView();
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), FALSE);
			}
			if (nOutlineType == OutlineType::Default) {
				bool bType = (ProfDockSet() != 0);
				if (bType) {
					nOutlineType = type.nDockOutline;
					SetTypeConfig(TypeConfigNum(nDocType), type);
				}else {
					nOutlineType = CommonSet().nDockOutline;
				}
			}
			OutlineType nOutlineType = GetOutlineTypeRedraw(this->nOutlineType);	// �u�b�N�}�[�N���A�E�g���C����͂��͍Ō�ɊJ���Ă������̏�Ԃ������p���i������Ԃ̓A�E�g���C����́j
			pEditView->GetCommander().Command_FuncList(ShowDialogType::Normal, nOutlineType);	// �J��	�� HandleCommand(F_OUTLINE,...) ���ƈ��Preview��ԂŎ��s����Ȃ��̂� Command_FUNCLIST()
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), TRUE);
			}
			return true;	// ��͂���
		}
	}else {	// ���݂͕\��
		DockSideType eDockSideOld = GetDockSide();

		EditView* pEditView = (EditView*)(this->lParam);
		if (!bDockDisp) {	// �V�ݒ�͔�\��
			if (eDockSideOld <= DockSideType::Float) {	// ���݂̓t���[�e�B���O
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// ���ł̓t���[�e�B���O�͕��Ȃ��i�]���݊��j
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED && eDockSideNew <= DockSideType::Float) {
					return false;	// �t�@�C���ؑւł͐V�ݒ���t���[�e�B���O�Ȃ�ė��p�i�]���݊��j
				}
			}
			::DestroyWindow(GetHwnd());	// ����
			return false;
		}

		// �h�b�L���O�̃t���[�e�B���O�ؑւł͕��ĊJ��
		if ((eDockSideOld <= DockSideType::Float) != (eDockSideNew <= DockSideType::Float)) {
			::DestroyWindow(GetHwnd());	// ����
			if (eDockSideNew <= DockSideType::Float) {	// �V�ݒ�̓t���[�e�B���O
				xPos = yPos = -1;	// ��ʈʒu������������
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// ���ł̓t���[�e�B���O�͊J���Ȃ��i�]���݊��j�������ɊJ���ƃ^�u���[�h���͉�ʂ��؂�ւ���Ă��܂�
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED) {
					return false;	// �t�@�C���ؑւł̓t���[�e�B���O�͊J���Ȃ��i�]���݊��j
				}
			}
			// �� ���ł͈ꎞ�I�� Disable �����Ă����ĊJ���i�^�u���[�h�ł̕s���ȉ�ʐ؂�ւ��}�~�j
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), FALSE);
			}
			if (nOutlineType == OutlineType::Default) {
				bool bType = (ProfDockSet() != 0);
				if (bType) {
					nOutlineType = type.nDockOutline;
					SetTypeConfig(TypeConfigNum(nDocType), type);
				}else {
					nOutlineType = CommonSet().nDockOutline;
				}
			}
			OutlineType nOutlineType = GetOutlineTypeRedraw(this->nOutlineType);
			pEditView->GetCommander().Command_FuncList(ShowDialogType::Normal, nOutlineType);	// �J��	�� HandleCommand(F_OUTLINE,...) ���ƈ��Preview��ԂŎ��s����Ȃ��̂� Command_FUNCLIST()
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), TRUE);
			}
			return true;	// ��͂���
		}

		// �t���[�e�B���O���t���[�e�B���O�ł͔z�u���������Ɍ���ێ�
		if (eDockSideOld <= DockSideType::Float) {
			eDockSide = eDockSideNew;
			return false;
		}

		// �h�b�L���O���h�b�L���O�ł͔z�u����
		RECT rc;
		POINT ptLT;
		GetDockSpaceRect(&rc);
		ptLT.x = rc.left;
		ptLT.y = rc.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rc, ptLT.x - rc.left, ptLT.y - rc.top);

		switch (eDockSideNew) {
		case DockSideType::Left:	rc.right = rc.left + ProfDockLeft();	break;
		case DockSideType::Top:		rc.bottom = rc.top + ProfDockTop();		break;
		case DockSideType::Right:	rc.left = rc.right - ProfDockRight();	break;
		case DockSideType::Bottom:	rc.top = rc.bottom - ProfDockBottom();	break;
		}

		// �ȑO�Ɠ����z�u�Ȃ疳�ʂɈړ����Ȃ�
		RECT rcOld;
		GetWindowRect(&rcOld);
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top);
		if (eDockSideOld == eDockSideNew && ::EqualRect(&rcOld, &rc)) {
			::InvalidateRect(GetHwnd(), NULL, TRUE);	// ���������ĕ`�悾��
			return false;	// �z�u�ύX�s�v�i��F�ʂ̃t�@�C���^�C�v����̒ʒm�j
		}

		// �ړ�����
		eDockSide = eDockSideNew;	// ���g�̃h�b�L���O�z�u�̋L�����X�V
		::SetWindowPos(
			GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | ((eDockSideOld == eDockSideNew)? 0: SWP_FRAMECHANGED)
		);	// SWP_FRAMECHANGED �w��� WM_NCCALCSIZE�i��N���C�A���g�̈�̍Čv�Z�j�ɗU������
		pEditView->editWnd.EndLayoutBars(bEditWndReady);
	}
	return false;
}

/** �A�E�g���C���ʒm(MYWM_OUTLINE_NOTIFY)����

	wParam: �ʒm���
	lParam: ��ʖ��̃p�����[�^

	@date 2010.06.07 ryoji �V�K�쐬
*/
void DlgFuncList::OnOutlineNotify(WPARAM wParam, LPARAM lParam)
{
	EditDoc* pDoc = EditDoc::GetInstance(0);	// ���͔�\����������Ȃ��̂� (EditView*)lParam �͎g���Ȃ�
	switch (wParam) {
	case 0:	// �ݒ�ύX�ʒm�i�h�b�L���O���[�h or �T�C�Y�j, lParam: �ʒm���� HWND
		if ((HWND)lParam == pDoc->pEditWnd->GetHwnd()) {
			return;	// ��������̒ʒm�͖���
		}
		ChangeLayout(OUTLINE_LAYOUT_BACKGROUND);	// �A�E�g���C����ʂ��Ĕz�u
		break;
	}
	return;
}

/** ���E�B���h�E�ɃA�E�g���C���ʒm���|�X�g����
	@date 2010.06.10 ryoji �V�K�쐬
*/
BOOL DlgFuncList::PostOutlineNotifyToAllEditors(WPARAM wParam, LPARAM lParam)
{
	return AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_OUTLINE_NOTIFY, (WPARAM)wParam, (LPARAM)lParam, GetHwnd());
}

void DlgFuncList::SetTypeConfig(TypeConfigNum docType, const TypeConfig& type)
{
	DocTypeManager().SetTypeConfig(docType, type);
}

/** �R���e�L�X�g���j���[����
	@date 2010.06.07 ryoji �V�K�쐬
*/
BOOL DlgFuncList::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	// �L���v�V���������X�g�^�c���[��Ȃ烁�j���[��\������
	HWND hwndFrom = (HWND)wParam;
	if (::SendMessage(GetHwnd(), WM_NCHITTEST, 0, lParam) == HTCAPTION
			|| hwndFrom == GetItemHwnd(IDC_LIST_FL)
			|| hwndFrom == GetItemHwnd(IDC_TREE_FL)
	) {
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		// �L�[�{�[�h�i���j���[�L�[ �� Shift F10�j����̌Ăяo��
		if (pt.x == -1 && pt.y == -1) {
			RECT rc;
			::GetWindowRect(hwndFrom, &rc);
			pt.x = rc.left;
			pt.y = rc.top;
		}
		DoMenu(pt, hwndFrom);
		return TRUE;
	}

	return Dialog::OnContextMenu(wParam, lParam);	// ���̑��̃R���g���[����ł̓|�b�v�A�b�v�w���v��\������
}

/** �^�C�g���o�[�̃h���b�O���h���b�v�Ńh�b�L���O�z�u����ۂ̈ړ����`�����߂�
	@date 2010.06.17 ryoji �V�K�쐬
*/
DockSideType DlgFuncList::GetDropRect(
	POINT ptDrag,
	POINT ptDrop,
	LPRECT pRect,
	bool bForceFloat
	)
{
	struct DockStretch {
		static int GetIdealStretch(int nStretch, int nMaxStretch)
		{
			if (nStretch == 0) {
				nStretch = nMaxStretch / 3;
			}
			if (nStretch > nMaxStretch - DOCK_MIN_SIZE) {
				nStretch = nMaxStretch - DOCK_MIN_SIZE;
			}
			if (nStretch < DOCK_MIN_SIZE) {
				nStretch = DOCK_MIN_SIZE;
			}
			return nStretch;
		}
	};

	// �ړ����Ȃ���`���擾����
	RECT rcWnd;
	GetWindowRect(&rcWnd);
	if (IsDocking() && !bForceFloat) {
		if (::PtInRect(&rcWnd, ptDrop)) {
			*pRect = rcWnd;
			return GetDockSide();	// �ړ����Ȃ��ʒu������
		}
	}

	// �h�b�L���O�p�̋�`���擾����
	DockSideType dockSideType = DockSideType::Float;	// �t���[�e�B���O�ɉ�����
	RECT rcDock;
	GetDockSpaceRect(&rcDock);
	if (!bForceFloat && ::PtInRect(&rcDock, ptDrop)) {
		int cxLeft		= DockStretch::GetIdealStretch(ProfDockLeft(), rcDock.right - rcDock.left);
		int cyTop		= DockStretch::GetIdealStretch(ProfDockTop(), rcDock.bottom - rcDock.top);
		int cxRight		= DockStretch::GetIdealStretch(ProfDockRight(), rcDock.right - rcDock.left);
		int cyBottom	= DockStretch::GetIdealStretch(ProfDockBottom(), rcDock.bottom - rcDock.top);

		int nDock = ::GetSystemMetrics(SM_CXCURSOR);
		if (ptDrop.x - rcDock.left < nDock) {
			dockSideType = DockSideType::Left;
			rcDock.right = rcDock.left + cxLeft;
		}else if (rcDock.right - ptDrop.x < nDock) {
			dockSideType = DockSideType::Right;
			rcDock.left = rcDock.right - cxRight;
		}else if (ptDrop.y - rcDock.top < nDock) {
			dockSideType = DockSideType::Top;
			rcDock.bottom = rcDock.top + cyTop;
		}else if (rcDock.bottom - ptDrop.y < nDock) {
			dockSideType = DockSideType::Bottom;
			rcDock.top = rcDock.bottom - cyBottom;
		}
		if (dockSideType != DockSideType::Float) {
			*pRect = rcDock;
			return dockSideType;	// �h�b�L���O�ʒu������
		}
	}

	// �t���[�e�B���O�p�̋�`���擾����
	if (!IsDocking()) {	// �t���[�e�B���O �� �t���[�e�B���O
		::OffsetRect(&rcWnd, ptDrop.x - ptDrag.x, ptDrop.y - ptDrag.y);
		*pRect = rcWnd;
	}else {	// �h�b�L���O �� �t���[�e�B���O
		int cx, cy;
		RECT rcFloat;
		rcFloat.left = 0;
		rcFloat.top = 0;
		if (pShareData->common.outline.bRememberOutlineWindowPos
				&& pShareData->common.outline.widthOutlineWindow	// �����l���� 0 �ɂȂ��Ă���
				&& pShareData->common.outline.heightOutlineWindow	// �����l���� 0 �ɂȂ��Ă���
		) {
			// �L�����Ă���T�C�Y
			rcFloat.right = pShareData->common.outline.widthOutlineWindow;
			rcFloat.bottom = pShareData->common.outline.heightOutlineWindow;
			cx = ::GetSystemMetrics(SM_CXMIN);
			cy = ::GetSystemMetrics(SM_CYMIN);
			if (rcFloat.right < cx) rcFloat.right = cx;
			if (rcFloat.bottom < cy) rcFloat.bottom = cy;
		}else {
			HINSTANCE hInstance2 = SelectLang::getLangRsrcInstance();
			if (lastRcInstance != hInstance2) {
				HRSRC hResInfo = ::FindResource(hInstance2, MAKEINTRESOURCE(IDD_FUNCLIST), RT_DIALOG);
				if (!hResInfo) return dockSideType;
				HGLOBAL hResData = ::LoadResource(hInstance2, hResInfo);
				if (!hResData) return dockSideType;
				pDlgTemplate = (LPDLGTEMPLATE)::LockResource(hResData);
				if (!pDlgTemplate) return dockSideType;
				dwDlgTmpSize = ::SizeofResource(hInstance2, hResInfo);
				// ����؂�ւ��Ń��\�[�X���A�����[�h����Ă��Ȃ����m�F���邽�߃C���X�^���X���L������
				lastRcInstance = hInstance2;
			}
			// �f�t�H���g�̃T�C�Y�i�_�C�A���O�e���v���[�g�Ō��܂�T�C�Y�j
			rcFloat.right = pDlgTemplate->cx;
			rcFloat.bottom = pDlgTemplate->cy;
			::MapDialogRect(GetHwnd(), &rcFloat);
			rcFloat.right += ::GetSystemMetrics(SM_CXDLGFRAME) * 2;	// �� Create ���̃X�^�C���ύX�ŃT�C�Y�ύX�s����T�C�Y�ύX�\�ɂ��Ă���
			rcFloat.bottom += ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYDLGFRAME) * 2;
		}
		cy = ::GetSystemMetrics(SM_CYCAPTION);
		::OffsetRect(&rcFloat, ptDrop.x - cy * 2, ptDrop.y - cy / 2);
		*pRect = rcFloat;
	}

	return DockSideType::Float;	// �t���[�e�B���O�ʒu������
}

/** �^�C�g���o�[�̃h���b�O���h���b�v�Ńh�b�L���O�z�u��ύX����
	@date 2010.06.17 ryoji �V�K�쐬
*/
BOOL DlgFuncList::Track(POINT ptDrag)
{
	if (::GetCapture()) {
		return FALSE;
	}

	// ��ʂɃS�~���c��Ȃ��悤��
	struct LockWindowUpdate {
		LockWindowUpdate() { ::LockWindowUpdate(::GetDesktopWindow()); }
		~LockWindowUpdate() { ::LockWindowUpdate(NULL); }
	} lockWindowUpdate;

	const SIZE sizeFull = {8, 8};	// �t���[�e�B���O�z�u�p�̘g���̑���
	const SIZE sizeHalf = {4, 4};	// �h�b�L���O�z�u�p�̘g���̑���
	const SIZE sizeClear = {0, 0};	// �g���`�悵�Ȃ�

	POINT pt;
	RECT rc;
	RECT rcDragLast;
	SIZE sizeLast = sizeClear;
	bool bDragging = false;	// �܂��{�i�J�n���Ȃ�
	int cxDragSm = ::GetSystemMetrics(SM_CXDRAG);
	int cyDragSm = ::GetSystemMetrics(SM_CYDRAG);

	::SetCapture(GetHwnd());	// �L���v�`���J�n

	while (::GetCapture() == GetHwnd()) {
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0)) {
			::PostQuitMessage((int)msg.wParam);
			break;
		}

		switch (msg.message) {
		case WM_MOUSEMOVE:
			::GetCursorPos(&pt);

			bool bStart;
			bStart = false;
			if (!bDragging) {
				// �������ʒu���炢���炩�����Ă���h���b�O�J�n�ɂ���
				if (abs(pt.x - ptDrag.x) >= cxDragSm || abs(pt.y - ptDrag.y) >= cyDragSm) {
					bDragging = bStart = true;	// ��������J�n
				}
			}
			if (bDragging) {	// �h���b�O��
				// �h���b�v���`��`�悷��
				DockSideType dockSideType = GetDropRect(ptDrag, pt, &rc, GetKeyState_Control());
				SIZE sizeNew = (dockSideType <= DockSideType::Float)? sizeFull: sizeHalf;
				Graphics::DrawDropRect(&rc, sizeNew, bStart? NULL : &rcDragLast, sizeLast);
				rcDragLast = rc;
				sizeLast = sizeNew;
			}
			break;

		case WM_LBUTTONUP:
			::GetCursorPos(&pt);

			::ReleaseCapture();
			if (bDragging) {
				// �h�b�L���O�z�u��ύX����
				DockSideType dockSideType = GetDropRect(ptDrag, pt, &rc, GetKeyState_Control());
				Graphics::DrawDropRect(NULL, sizeClear, &rcDragLast, sizeLast);

				bool bType = (ProfDockSet() != 0);
				if (bType) {
					DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
				}
				ProfDockDisp() = GetHwnd()? TRUE: FALSE;
				ProfDockSide() = dockSideType;	// �V�����h�b�L���O���[�h��K�p
				switch (dockSideType) {
				case DockSideType::Left:	ProfDockLeft() = rc.right - rc.left;	break;
				case DockSideType::Top:		ProfDockTop() = rc.bottom - rc.top;		break;
				case DockSideType::Right:	ProfDockRight() = rc.right - rc.left;	break;
				case DockSideType::Bottom:	ProfDockBottom() = rc.bottom - rc.top;	break;
				}
				if (bType) {
					SetTypeConfig(TypeConfigNum(nDocType), type);
				}
				ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// �������g�ւ̋����ύX
				if (!IsDocking()) {
					::MoveWindow(GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				}
				if (ProfDockSync()) {
					PostOutlineNotifyToAllEditors(
						(WPARAM)0,
						(LPARAM)((EditView*)(this->lParam))->editWnd.GetHwnd()
					);	// ���E�B���h�E�Ƀh�b�L���O�z�u�ύX��ʒm����
				}
				return TRUE;
			}
			return FALSE;

		case WM_KEYUP:
			if (bDragging) {
				if (msg.wParam == VK_CONTROL) {
					// �t���[�e�B���O���������郂�[�h�𔲂���
					::GetCursorPos(&pt);
					DockSideType dockSideType = GetDropRect(ptDrag, pt, &rc, false);
					SIZE sizeNew = (dockSideType <= DockSideType::Float)? sizeFull: sizeHalf;
					Graphics::DrawDropRect(&rc, sizeNew, &rcDragLast, sizeLast);
					rcDragLast = rc;
					sizeLast = sizeNew;
				}
			}
			break;

		case WM_KEYDOWN:
			if (bDragging) {
				if (msg.wParam == VK_CONTROL) {
					// �t���[�e�B���O���������郂�[�h�ɓ���
					::GetCursorPos(&pt);
					GetDropRect(ptDrag, pt, &rc, true);
					Graphics::DrawDropRect(&rc, sizeFull, &rcDragLast, sizeLast);
					sizeLast = sizeFull;
					rcDragLast = rc;
				}
			}
			if (msg.wParam == VK_ESCAPE) {
				// �L�����Z��
				::ReleaseCapture();
				if (bDragging) {
					Graphics::DrawDropRect(NULL, sizeClear, &rcDragLast, sizeLast);
				}
				return FALSE;
			}
			break;

		case WM_RBUTTONDOWN:
			// �L�����Z��
			::ReleaseCapture();
			if (bDragging) {
				Graphics::DrawDropRect(NULL, sizeClear, &rcDragLast, sizeLast);
			}
			return FALSE;

		default:
			::DispatchMessage(&msg);
			break;
		}
	}

	::ReleaseCapture();
	return FALSE;
}

void DlgFuncList::LoadFileTreeSetting(
	FileTreeSetting& data,
	SFilePath& iniDirPath
	)
{
	const FileTree* pFileTree;
	if (ProfDockSet() == 0) {
		pFileTree = &(CommonSet().fileTree);
		data.eFileTreeSettingOrgType = FileTreeSettingFromType::Common;
	}else {
		DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		pFileTree = &(TypeSet().fileTree);
		data.eFileTreeSettingOrgType = FileTreeSettingFromType::Type;
	}
	data.eFileTreeSettingLoadType = data.eFileTreeSettingOrgType;
	data.bProject = pFileTree->bProject;
	data.szDefaultProjectIni = pFileTree->szProjectIni;
	data.szLoadProjectIni = _T("");
	if (data.bProject) {
		// �e�t�H���_�̃v���W�F�N�g�t�@�C���ǂݍ���
		TCHAR szPath[_MAX_PATH];
		::GetLongFileName( _T("."), szPath );
		auto_strcat( szPath, _T("\\") );
		int maxDir = DlgTagJumpList::CalcMaxUpDirectory( szPath );
		for (int i=0; i<=maxDir; ++i) {
			DataProfile profile;
			profile.SetReadingMode();
			std::tstring strIniFileName;
			strIniFileName += szPath;
			strIniFileName += CommonSet().fileTreeDefIniName;
			if (profile.ReadProfile(strIniFileName.c_str())) {
				ImpExpFileTree::IO_FileTreeIni(profile, data.items);
				data.eFileTreeSettingLoadType = FileTreeSettingFromType::File;
				iniDirPath = szPath;
				CutLastYenFromDirectoryPath( iniDirPath );
				data.szLoadProjectIni = strIniFileName.c_str();
				break;
			}
			DlgTagJumpList::DirUp( szPath );
		}
	}
	if (data.szLoadProjectIni[0] == _T('\0')) {
		// �f�t�H���g�v���W�F�N�g�t�@�C���ǂݍ���
		bool bReadIni = false;
		if (pFileTree->szProjectIni[0] != _T('\0')) {
			DataProfile profile;
			profile.SetReadingMode();
			const TCHAR* pszIniFileName;
			TCHAR szDir[_MAX_PATH * 2];
			if (_IS_REL_PATH( pFileTree->szProjectIni )) {
				// sakura.ini����̑��΃p�X
				GetInidirOrExedir( szDir, pFileTree->szProjectIni );
				pszIniFileName = szDir;
			}else {
				pszIniFileName = pFileTree->szProjectIni;
			}
			if (profile.ReadProfile(pszIniFileName)) {
				ImpExpFileTree::IO_FileTreeIni(profile, data.items);
				data.szLoadProjectIni = pszIniFileName;
				bReadIni = true;
			}
		}
		if (!bReadIni) {
			// ���ʐݒ�or�^�C�v�ʐݒ肩��ǂݍ���
			//fileTreeSetting = *pFileTree;
			data.items.resize( pFileTree->nItemCount );
			for (int i=0; i<pFileTree->nItemCount; ++i) {
				data.items[i] = pFileTree->items[i];
			}
		}
	}
}

