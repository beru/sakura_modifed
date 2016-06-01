/*!	@file
	@brief アウトライン解析ダイアログボックス

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani Visual Basicのアウトライン解析
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, JEPRO, genta, hor
	Copyright (C) 2002, MIK, aroka, hor, genta, YAZAKI, Moca, frozen
	Copyright (C) 2003, zenryaku, Moca, naoh, little YOSHI, genta,
	Copyright (C) 2004, zenryaku, Moca, novice
	Copyright (C) 2005, genta, zenryaku, ぜっと, D.S.Koba
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
#include "window/EditWnd.h"	//	2006/2/11 aroka 追加
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

// 画面ドッキング用の定義	// 2010.06.05 ryoji
#define DEFINE_SYNCCOLOR
#define DOCK_SPLITTER_WIDTH		DpiScaleX(5)
#define DOCK_MIN_SIZE			DpiScaleX(60)
#define DOCK_BUTTON_NUM			(3)

// ビューの種別
#define VIEWTYPE_LIST	0
#define VIEWTYPE_TREE	1

// アウトライン解析 CDlgFuncList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12200
	IDC_BUTTON_COPY,					HIDC_FL_BUTTON_COPY,	// コピー
	IDOK,								HIDOK_FL,				// ジャンプ
	IDCANCEL,							HIDCANCEL_FL,			// キャンセル
	IDC_BUTTON_HELP,					HIDC_FL_BUTTON_HELP,	// ヘルプ
	IDC_CHECK_bAutoCloseDlgFuncList,	HIDC_FL_CHECK_bAutoCloseDlgFuncList,	// 自動的に閉じる
	IDC_LIST_FL,						HIDC_FL_LIST1,			// トピックリスト	IDC_LIST1->IDC_LIST_FL	2008/7/3 Uchi
	IDC_TREE_FL,						HIDC_FL_TREE1,			// トピックツリー	IDC_TREE1->IDC_TREE_FL	2008/7/3 Uchi
	IDC_CHECK_bFunclistSetFocusOnJump,	HIDC_FL_CHECK_bFunclistSetFocusOnJump,	// ジャンプでフォーカス移動する
	IDC_CHECK_bMarkUpBlankLineEnable,	HIDC_FL_CHECK_bMarkUpBlankLineEnable,	// 空行を無視する
	IDC_COMBO_nSortType,				HIDC_COMBO_nSortType,	// 順序
	IDC_BUTTON_WINSIZE,					HIDC_FL_BUTTON_WINSIZE,	// ウィンドウ位置保存	// 2006.08.06 ryoji
	IDC_BUTTON_MENU,					HIDC_FL_BUTTON_MENU,	// ウィンドウの位置メニュー
	IDC_BUTTON_SETTING,					HIDC_FL_BUTTON_SETTING,	// 設定
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

// 関数リストの列
enum EFuncListCol {
	FL_COL_ROW		= 0,	// 行
	FL_COL_COL		= 1,	// 桁
	FL_COL_NAME		= 2,	// 関数名
	FL_COL_REMARK	= 3		// 備考
};

// ソート比較用プロシージャ
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
	//	Apr. 23, 2005 genta 行番号を左端へ
	if (pDlgFuncList->nSortCol == FL_COL_NAME) {	// 名前でソート
		return auto_stricmp(pFuncInfo1->memFuncName.GetStringPtr(), pFuncInfo2->memFuncName.GetStringPtr());
	}
	//	Apr. 23, 2005 genta 行番号を左端へ
	if (pDlgFuncList->nSortCol == FL_COL_ROW) {	// 行（＋桁）でソート
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
	if (pDlgFuncList->nSortCol == FL_COL_COL) {	// 桁でソート
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
	if (pDlgFuncList->nSortCol == FL_COL_REMARK) {	// 備考でソート
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
	// サイズ変更時に位置を制御するコントロール数
	assert(_countof(anchorList) == _countof(rcItems));

	pFuncInfoArr = NULL;		// 関数情報配列
	nCurLine = 0;	// 現在行
	nOutlineType = OutlineType::Default;
	nListType = OutlineType::Default;
	//	Apr. 23, 2005 genta 行番号を左端へ
	nSortCol = 0;				// ソートする列番号 2004.04.06 zenryaku 標準は行番号(1列目)
	nSortColOld = -1;
	bLineNumIsCRLF = false;		// 行番号の表示 false=折り返し単位／true=改行単位
	bWaitTreeProcess = false;	// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 2/4
	nSortType = 0;
	funcInfo = NULL;			// 現在の関数情報
	bEditWndReady = false;		// エディタ画面の準備完了
	bInChangeLayout = false;
	pszTimerJumpFile = NULL;
	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
}


/*!
	標準以外のメッセージを捕捉する

	@date 2007.11.07 ryoji 新規
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

		// 自分が最初にアクティブ化された場合は一旦編集ウィンドウをアクティブ化して戻す
		//
		// Note. このダイアログは他とは異なるウィンドウスタイルのため閉じたときの挙動が異なる．
		// 他はスレッド内最近アクティブなウィンドウがアクティブになるが，このダイアログでは
		// セッション内全体での最近アクティブウィンドウがアクティブになってしまう．
		// それでは都合が悪いので，特別に以下の処理を行って他と同様な挙動が得られるようにする．
		if ((BOOL)wParam) {
			EditView* pEditView = (EditView*)(this->lParam);
			auto& editWnd = pEditView->editWnd;
			if (::GetActiveWindow() == GetHwnd()) {
				::SetActiveWindow(editWnd.GetHwnd());
				BlockingHook(NULL);	// キュー内に溜まっているメッセージを処理
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
			// ドッキングのときはコンテキストメニューを明示的に呼び出す必要があるらしい
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
			// キャプションを再描画する
			// ※ この時点ではまだテキスト設定されていないので RDW_UPDATENOW では NG
			::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_NOINTERNALPAINT);
		}
		break;
	case WM_MOUSEACTIVATE:
		if (IsDocking()) {
			// 分割バー以外の場所ならフォーカス移動
			if (!(HTLEFT <= LOWORD(lParam) && LOWORD(lParam) <= HTBOTTOMRIGHT)) {
				::SetFocus(GetHwnd());
			}
		}
		break;
	case WM_COMMAND:
		if (IsDocking()) {
			// コンボボックスのフォーカスが変化したらキャプションを再描画する（アクティブ／非アクティブ切替）
			if (LOWORD(wParam) == IDC_COMBO_nSortType) {
				if (HIWORD(wParam) == CBN_SETFOCUS || HIWORD(wParam) == CBN_KILLFOCUS) {
					::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);
				}
			}
		}
		break;
	case WM_NOTIFY:
		if (IsDocking()) {
			// ツリーやリストのフォーカスが変化したらキャプションを再描画する（アクティブ／非アクティブ切替）
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


// モードレスダイアログの表示
/*
 * @note 2011.06.25 syat nOutlineTypeを追加
 *   nOutlineTypeとnListTypeはほとんどの場合同じ値だが、プラグインの場合は例外で、
 *   nOutlineTypeはアウトライン解析のID、nListTypeはプラグイン内で指定するリスト形式となる。
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
	bool			bLineNumIsCRLF		// 行番号の表示 false=折り返し単位／true=改行単位
	)
{
	EditView* pEditView = (EditView*)lParam;
	if (!pEditView) {
		return NULL;
	}
	this->pFuncInfoArr = pFuncInfoArr;	// 関数情報配列
	this->nCurLine = nCurLine;			// 現在行
	this->nCurCol = nCurCol;				// 現在桁
	this->nOutlineType = nOutlineType;	// アウトライン解析の種別
	this->nListType = nListType;			// 一覧の種類
	this->bLineNumIsCRLF = bLineNumIsCRLF;	// 行番号の表示 false=折り返し単位／true=改行単位
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

	// 2007.04.18 genta : 「フォーカスを移す」と「自動的に閉じる」がチェックされている場合に
	// ダブルクリックを行うと，trueのまま残ってしまうので，ウィンドウを開いたときにリセットする．
	bWaitTreeProcess = false;

	eDockSide = ProfDockSide();
	HWND hwndRet;
	if (IsDocking()) {
		// ドッキング用にダイアログテンプレートに手を加えてから表示する（WS_CHILD化）
		HINSTANCE hInstance2 = SelectLang::getLangRsrcInstance();
		if (!pDlgTemplate || lastRcInstance != hInstance2) {
			HRSRC hResInfo = ::FindResource(hInstance2, MAKEINTRESOURCE(IDD_FUNCLIST), RT_DIALOG);
			if (!hResInfo) return NULL;
			HGLOBAL hResData = ::LoadResource(hInstance2, hResInfo);
			if (!hResData) return NULL;
			pDlgTemplate = (LPDLGTEMPLATE)::LockResource(hResData);
			if (!pDlgTemplate) return NULL;
			dwDlgTmpSize = ::SizeofResource(hInstance2, hResInfo);
			// 言語切り替えでリソースがアンロードされていないか確認するためインスタンスを記憶する
			lastRcInstance = hInstance2;
		}
		LPDLGTEMPLATE pDlgTemplate = (LPDLGTEMPLATE)::GlobalAlloc(GMEM_FIXED, dwDlgTmpSize);
		if (!pDlgTemplate) return NULL;
		::CopyMemory(pDlgTemplate, pDlgTemplate, dwDlgTmpSize);
		pDlgTemplate->style = (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_SETFONT);
		hwndRet = Dialog::DoModeless(hInstance, MyGetAncestor(hwndParent, GA_ROOT), pDlgTemplate, lParam, SW_HIDE);
		::GlobalFree(pDlgTemplate);
		pEditView->editWnd.EndLayoutBars(bEditWndReady);	// 画面の再レイアウト
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

// モードレス時：検索対象となるビューの変更
void DlgFuncList::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}

// ダイアログデータの設定
void DlgFuncList::SetData()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_FL);
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	// 2002.02.08 hor 隠しといてアイテム削除→あとで表示
	::ShowWindow(hwndList, SW_HIDE);
	::ShowWindow(hwndTree, SW_HIDE);
	ListView_DeleteAllItems(hwndList);
	TreeView_DeleteAllItems(hwndTree);
	::ShowWindow(GetItemHwnd(IDC_BUTTON_SETTING), SW_HIDE);

	SetDocLineFuncList();
	
	switch (nListType) {
	case OutlineType::CPP:	// C++メソッドリスト
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), true);	// Jan. 04, 2002 genta Java Method Treeに統合
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_CPP));
		break;
	case OutlineType::RuleFile:	//@@@ 2002.04.01 YAZAKI アウトライン解析にルールファイル導入
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_RULE));
		break;
	case OutlineType::WZText: //@@@ 2003.05.20 zenryaku 階層付テキストアウトライン解析
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_WZ)); //	2003.06.22 Moca 名前変更
		break;
	case OutlineType::HTML: //@@@ 2003.05.20 zenryaku HTMLアウトライン解析
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T("HTML"));
		break;
	case OutlineType::TeX: //@@@ 2003.07.20 naoh TeXアウトライン解析
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T("TeX"));
		break;
	case OutlineType::Text: // テキスト・トピックリスト
		nViewType = VIEWTYPE_TREE;
		SetTree();	//@@@ 2002.04.01 YAZAKI テキストトピックツリーも、汎用SetTreeを呼ぶように変更。
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_TEXT));
		break;
	case OutlineType::Java: // Javaメソッドツリー
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), true);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_JAVA));
		break;
	//	2007.02.08 genta Python追加
	case OutlineType::Python: // Python メソッドツリー
		nViewType = VIEWTYPE_TREE;
		SetTree(true);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_PYTHON));
		break;
	case OutlineType::Cobol: // COBOL アウトライン
		nViewType = VIEWTYPE_TREE;
		SetTreeJava(GetHwnd(), false);
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_COBOL));
		break;
	case OutlineType::VisualBasic:	// VisualBasic アウトライン
		nViewType = VIEWTYPE_LIST;
		SetListVB();
		::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_VB));
		break;
	case OutlineType::FileTree:
		nViewType = VIEWTYPE_TREE;
		SetTreeFile();
		::SetWindowText( GetHwnd(), LS(F_FILETREE) );	// ファイルツリー
		break;
	case OutlineType::Tree: // 汎用ツリー
		nViewType = VIEWTYPE_TREE;
		SetTree();
		::SetWindowText(GetHwnd(), _T(""));
		break;
	case OutlineType::TreeTagJump: // 汎用ツリー(タグジャンプ付き)
		nViewType = VIEWTYPE_TREE;
		SetTree(true);
		::SetWindowText(GetHwnd(), _T(""));
		break;
	case OutlineType::ClassTree: // 汎用クラスツリー
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
// Jul 10, 2003  little YOSHI  上に移動しました--->>
//		case OutlineType::VisualBasic:	// 2001/06/23 N.Nakatani for Visual Basic
//			::SetWindowText(GetHwnd(), "Visual Basic アウトライン");
//			break;
// <<---ここまで
		case OutlineType::Erlang:	//	2009.08.10 genta
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_ERLANG));
			break;
		case OutlineType::BookMark:
			LV_COLUMN col;
			col.mask = LVCF_TEXT;
			col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_TEXT));
			col.iSubItem = 0;
			//	Apr. 23, 2005 genta 行番号を左端へ
			ListView_SetColumn(hwndList, FL_COL_NAME, &col);
			::SetWindowText(GetHwnd(), LS(STR_DLGFNCLST_TITLE_BOOK));
			break;
		case OutlineType::List:	// 汎用リスト 2010.03.28 syat
			::SetWindowText(GetHwnd(), _T(""));
			break;
		}
		//	May 18, 2001 genta
		//	Windowがいなくなると後で都合が悪いので、表示しないだけにしておく
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

		memClipText.SetString(L"");	// クリップボードコピー用テキスト
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
			// 現在の解析結果要素
			pFuncInfo = pFuncInfoArr->GetAt(i);

			//	From Here Apr. 23, 2005 genta 行番号を左端へ
			// 行番号の表示 false=折り返し単位／true=改行単位
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

			// 2010.03.17 syat 桁追加
			// 行番号の表示 false=折り返し単位／true=改行単位
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
			//	To Here Apr. 23, 2005 genta 行番号を左端へ

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
				// ここにあったVB関係の処理はSetListVB()メソッドに移動しました。

				item.pszText = const_cast<TCHAR*>(_T(""));
			}
			item.iItem = (int)i;
			item.iSubItem = FL_COL_REMARK;
			ListView_SetItem(hwndList, &item);

			// クリップボードにコピーするテキストを編集
			if (item.pszText[0] != _T('\0')) {
				// 検出結果の種類(関数,,,)があるとき
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					pFuncInfoArr->szFilePath.c_str(),	// 解析対象ファイル名
					pFuncInfo->nFuncLineCRLF,			// 検出行番号
					pFuncInfo->nFuncColCRLF				// 検出桁番号
				);
				memClipText.AppendStringT(szText);
				// "%ts(%ts)\r\n"
				memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
				memClipText.AppendStringLiteral(L"(");
				memClipText.AppendStringT(item.pszText);
				memClipText.AppendStringLiteral(L")\r\n");
			}else {
				// 検出結果の種類(関数,,,)がないとき
				auto_sprintf(
					szText,
					_T("%ts(%d,%d): "),
					pFuncInfoArr->szFilePath.c_str(),	// 解析対象ファイル名
					pFuncInfo->nFuncLineCRLF,			// 検出行番号
					pFuncInfo->nFuncColCRLF				// 検出桁番号
				);
				memClipText.AppendStringT(szText);
				memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
				memClipText.AppendStringLiteral(L"\r\n");
			}
		}
		// 2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
		::ShowWindow(hwndList, SW_SHOW);
		// 列の幅をデータに合わせて調整
		ListView_SetColumnWidth(hwndList, FL_COL_ROW, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_COL, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_NAME, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_REMARK, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, FL_COL_ROW, ListView_GetColumnWidth(hwndList, FL_COL_ROW) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_COL, ListView_GetColumnWidth(hwndList, FL_COL_COL) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_NAME, ListView_GetColumnWidth(hwndList, FL_COL_NAME) + 16);
		ListView_SetColumnWidth(hwndList, FL_COL_REMARK, ListView_GetColumnWidth(hwndList, FL_COL_REMARK) + 16);

		// 2005.07.05 ぜっと
		DWORD dwExStyle  = ListView_GetExtendedListViewStyle(hwndList);
		dwExStyle |= LVS_EX_FULLROWSELECT;
		ListView_SetExtendedListViewStyle(hwndList, dwExStyle);

		if (bSelected) {
			ListView_GetItemRect(hwndList, 0, &rc, LVIR_BOUNDS);
			ListView_Scroll(hwndList, 0, nSelectedLine * (rc.bottom - rc.top));
			ListView_SetItemState(hwndList, nSelectedLine, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}
	// アウトライン ダイアログを自動的に閉じる
	CheckButton(IDC_CHECK_bAutoCloseDlgFuncList, pShareData->common.outline.bAutoCloseDlgFuncList);
	// アウトライン ブックマーク一覧で空行を無視する
	CheckButton(IDC_CHECK_bMarkUpBlankLineEnable, pShareData->common.outline.bMarkUpBlankLineEnable);
	// アウトライン ジャンプしたらフォーカスを移す
	CheckButton(IDC_CHECK_bFunclistSetFocusOnJump, pShareData->common.outline.bFunclistSetFocusOnJump);

	// アウトライン ■位置とサイズを記憶する // 20060201 aroka
	CheckButton(IDC_BUTTON_WINSIZE, pShareData->common.outline.bRememberOutlineWindowPos);
	// ボタンが押されているかはっきりさせる 2008/6/5 Uchi
	SetItemText(IDC_BUTTON_WINSIZE, 
		pShareData->common.outline.bRememberOutlineWindowPos ? _T("■") : _T("□"));

	// ダイアログを自動的に閉じるならフォーカス移動オプションは関係ない
	EnableItem(IDC_CHECK_bFunclistSetFocusOnJump, !pShareData->common.outline.bAutoCloseDlgFuncList);

	// 2002.02.08 hor
	//（IDC_LIST_FLもIDC_TREE_FLも常に存在していて、nViewTypeによって、どちらを表示するかを選んでいる）
	HWND hwndShow = (nViewType == VIEWTYPE_LIST)? hwndList: hwndTree;
	::ShowWindow(hwndShow, SW_SHOW);
	if (::GetForegroundWindow() == MyGetAncestor(GetHwnd(), GA_ROOT) && IsChild(GetHwnd(), GetFocus())) {
		::SetFocus(hwndShow);
	}

	// 2002.02.08 hor
	// 空行をどう扱うかのチェックボックスはブックマーク一覧のときだけ表示する
	if (nListType == OutlineType::BookMark) {
		EnableItem(IDC_CHECK_bMarkUpBlankLineEnable, true);
		if (!IsDocking()) {
			::ShowWindow(GetItemHwnd(IDC_CHECK_bMarkUpBlankLineEnable), SW_SHOW);
		}
	}else {
		::ShowWindow(GetItemHwnd(IDC_CHECK_bMarkUpBlankLineEnable), SW_HIDE);
		EnableItem(IDC_CHECK_bMarkUpBlankLineEnable, false);
	}
	// 2002/11/1 frozen 項目のソート基準を設定するコンボボックスはブックマーク一覧の以外の時に表示する
	// Nov. 5, 2002 genta ツリー表示の時だけソート基準コンボボックスを表示
	EditView* pEditView = (EditView*)(this->lParam);
	int nDocType = pEditView->GetDocument().docType.GetDocumentType().GetIndex();
	if (this->nDocType != nDocType) {
		// 以前とはドキュメントタイプが変わったので初期化する
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
		Combo_ResetContent(hWnd_Combo_Sort); // 2002.11.10 Moca 追加
		Combo_AddString(hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE1));
		Combo_AddString(hWnd_Combo_Sort , LS(STR_DLGFNCLST_SORTTYPE2));
		Combo_SetCurSel(hWnd_Combo_Sort , nSortType);
		::ShowWindow(GetItemHwnd(IDC_STATIC_nSortType), SW_SHOW);
		// 2002.11.10 Moca 追加 ソートする
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
		//ListView_SortItems(hwndList, CompareFunc_Asc, (LPARAM)this);  // 2005.04.05 zenryaku ソート状態を保持
		SortListView(hwndList, nSortCol);	// 2005.04.23 genta 関数化(ヘッダ書き換えのため)
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


// ダイアログデータの取得
// 0==条件未入力   0より大きい==正常   0より小さい==入力エラー
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
				// lParamが-1以下は pFuncInfoArrには含まれない項目
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

// Java/C++メソッドツリーの最大ネスト深さ
#define MAX_JAVA_TREE_NEST 16

/*! ツリーコントロールの初期化：Javaメソッドツリー

	Java Method Treeの構築: 関数リストを元にTreeControlを初期化する。

	@date 2002.01.04 genta C++ツリーを統合
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
    TCHAR           szLabel[64 + 6];  // Jan. 07, 2001 genta クラス名エリアの拡大
	HTREEITEM		htiGlobal = NULL;	// Jan. 04, 2001 genta C++と統合
	HTREEITEM		htiClass;
	HTREEITEM		htiItem;
	HTREEITEM		htiSelectedTop = NULL;
	HTREEITEM		htiSelected = NULL;
	TV_ITEM			tvi;
	int				nDummylParam = -64000;	// 2002.11.10 Moca クラス名のダミーlParam ソートのため
	TCHAR			szClassArr[MAX_JAVA_TREE_NEST][64];	// Jan. 04, 2001 genta クラス名エリアの拡大 // 2009.9.21 syat ネストが深すぎる際のBOF対策

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
	// 追加文字列の初期化（プラグインで指定済みの場合は上書きしない）
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
		// クラス名::メソッドの場合
		if ((pPos = _tcsstr(pWork, _T("::")))
			&& auto_strncmp(_T("operator "), pWork, 9) != 0
		) {
			// インナークラスのネストレベルを調べる
			size_t k;
			size_t nWorkLen;
			size_t nCharChars;
			int	nNestTemplate = 0;
			nWorkLen = _tcslen(pWork);
			for (k=0; k<nWorkLen; ++k) {
				// 2009.9.21 syat ネストが深すぎる際のBOF対策
				if (nClassNest == MAX_JAVA_TREE_NEST) {
					k = nWorkLen;
					break;
				}
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = NativeT::GetSizeOfChar(pWork, nWorkLen, k);
				if (nCharChars == 1 && nNestTemplate == 0 && pWork[k] == _T(':')) {
					//	Jan. 04, 2001 genta
					//	C++の統合のため、\に加えて::をクラス区切りとみなすように
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
					// namesp::function<std::string> のようなものを処理する
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
			//	関数先頭のセット(ツリー構築で使う)
			pWork = pWork + m; // 2 == lstrlen("::");

			// クラス名のアイテムが登録されているか
			htiClass = TreeView_GetFirstVisible(hwndTree);
			HTREEITEM htiParent = TVI_ROOT;
			for (k=0; k<nClassNest; ++k) {
				//	Apr. 1, 2001 genta
				//	追加文字列を全角にしたのでメモリもそれだけ必要
				//	6 == strlen("クラス"), 1 == strlen(L'\0')

				// 2002/10/30 frozen
				// bAddClass == true の場合の仕様変更
				// 既存の項目は　「(クラス名)(半角スペース一個)(追加文字列)」
				// となっているとみなし、szClassArr[k] が 「クラス名」と一致すれば、それを親ノードに設定。
				// ただし、一致する項目が複数ある場合は最初の項目を親ノードにする。
				// 一致しない場合は「(クラス名)(半角スペース一個)クラス」のノードを作成する。
				size_t nClassNameLen = _tcslen(szClassArr[k]);
				for (; htiClass; htiClass=TreeView_GetNextSibling(hwndTree, htiClass)) {
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = htiClass;
					tvi.pszText = szLabel;
					tvi.cchTextMax = _countof(szLabel);
					if (TreeView_GetItem(hwndTree, &tvi)) {
						if (_tcsncmp(szClassArr[k], szLabel, nClassNameLen) == 0) {
							if (_countof(szLabel) < (nClassNameLen +1)) {
								break;// バッファ不足では無条件にマッチする
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

				// クラス名のアイテムが登録されていないので登録
				if (!htiClass) {
					// 2002/10/28 frozen 上からここへ移動
					// 2002/10/28 frozen +9は追加する文字列の最大長（" 名前空間"が最大）// 2011.09.25 syat プラグインによる拡張対応
					std::vector<TCHAR> className(_tcslen(szClassArr[k]) + 1 + pFuncInfoArr->AppendTextLenMax());
					TCHAR* pClassName = &className[0];
					_tcscpy(pClassName, szClassArr[k]);

					tvis.item.lParam = -1;
					if (bAddClass) {
						if (pFuncInfo->nInfo == FL_OBJ_NAMESPACE) {
							//_tcscat(pClassName, _T(" 名前空間"));
							_tcscat(pClassName, to_tchar(pFuncInfoArr->GetAppendText(FL_OBJ_NAMESPACE).c_str()));
							tvis.item.lParam = i;
						}else {
							//_tcscat(pClassName, _T(" クラス"));
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
			//	Global空間の場合 (C++のみ)

			// 2002/10/27 frozen ここから
			// 2007.05.26 genta "__interface" をクラスに類する扱いにする
			// 2011.09.25 syat プラグインで追加された要素をクラスに類する扱いにする
			if (FL_OBJ_CLASS <= pFuncInfo->nInfo  && pFuncInfo->nInfo <= FL_OBJ_ELEMENT_MAX) {
				htiClass = TVI_ROOT;
			}else {
			// 2002/10/27 frozen ここまで
				if (!htiGlobal) {
					TV_INSERTSTRUCT	tvg = {0};
					std::tstring sGlobal = to_tchar(pFuncInfoArr->GetAppendText(FL_OBJ_GLOBAL).c_str());
					tvg.hParent = TVI_ROOT;
					tvg.hInsertAfter = TVI_LAST;
					tvg.item.mask = TVIF_TEXT | TVIF_PARAM;
					//tvg.item.pszText = const_cast<TCHAR*>(_T("グローバル"));
					tvg.item.pszText = const_cast<TCHAR*>(sGlobal.c_str());
					tvg.item.lParam = nDummylParam;
					htiGlobal = TreeView_InsertItem(hwndTree, &tvg);
					++nDummylParam;
				}
				htiClass = htiGlobal;
			}
		}
		std::vector<TCHAR> funcName(_tcslen(pWork) + pFuncInfoArr->AppendTextLenMax());	// ↓で追加する文字列が収まるだけ確保
		TCHAR* pFuncName = &funcName[0];
		_tcscpy(pFuncName, pWork);

		// 2002/10/27 frozen 追加文字列の種類を増やした
		switch (pFuncInfo->nInfo) {
		case FL_OBJ_DEFINITION:		//「定義位置」に追加文字列は不要なため除外
		case FL_OBJ_NAMESPACE:		//「名前空間」は別の場所で処理してるので除外
		case FL_OBJ_GLOBAL:			//「グローバル」は別の場所で処理してるので除外
			break;
		default:
			_tcscat(pFuncName, to_tchar(pFuncInfoArr->GetAppendText(pFuncInfo->nInfo).c_str()));
		}

		// 該当クラス名のアイテムの子として、メソッドのアイテムを登録
		tvis.hParent = htiClass;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = pFuncName;
		tvis.item.lParam = i;
		htiItem = TreeView_InsertItem(hwndTree, &tvis);

		// クリップボードにコピーするテキストを編集
		wchar_t szText[2048];
		auto_sprintf(
			szText,
			L"%ts(%d,%d): ",
			pFuncInfoArr->szFilePath.c_str(),	// 解析対象ファイル名
			pFuncInfo->nFuncLineCRLF,			// 検出行番号
			pFuncInfo->nFuncColCRLF				// 検出桁番号
		);
		memClipText.AppendString(szText);		// クリップボードコピー用テキスト
		// "%ts%ls\r\n"
		memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
		memClipText.AppendString(pFuncInfo->nInfo == FL_OBJ_DECLARE ? pFuncInfoArr->GetAppendText(FL_OBJ_DECLARE).c_str() : L""); 	//	Jan. 04, 2001 genta C++で使用
		memClipText.AppendStringLiteral(L"\r\n");

		// 現在カーソル位置のメソッドかどうか調べる
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
		//	deleteはその都度行うのでここでは不要
	}
	// ソート、ノードの展開をする
//	TreeView_SortChildren(hwndTree, TVI_ROOT, 0);
	htiClass = TreeView_GetFirstVisible(hwndTree);
	while (htiClass) {
//		TreeView_SortChildren(hwndTree, htiClass, 0);
		TreeView_Expand(hwndTree, htiClass, TVE_EXPAND);
		htiClass = TreeView_GetNextSibling(hwndTree, htiClass);
	}
	// 現在カーソル位置のメソッドを選択状態にする
	if (bSelected) {
		TreeView_SelectItem(hwndTree, htiSelected);
	}else {
		TreeView_SelectItem(hwndTree, htiSelectedTop);
	}
//	GetTreeTextNext(hwndTree, NULL, 0);
	return;
}


/*! リストビューコントロールの初期化：VisualBasic

  長くなったので独立させました。

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
		// 現在の解析結果要素
		const FuncInfo* pFuncInfo = pFuncInfoArr->GetAt(i);

		//	From Here Apr. 23, 2005 genta 行番号を左端へ
		// 行番号の表示 false=折り返し単位／true=改行単位
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

		// 2010.03.17 syat 桁追加
		// 行番号の表示 false=折り返し単位／true=改行単位
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
		//	To Here Apr. 23, 2005 genta 行番号を左端へ

		item.mask = LVIF_TEXT;

		// 2001/06/23 N.Nakatani for Visual Basic
		//	Jun. 26, 2001 genta 半角かな→全角に
		auto_memset(szText, _T('\0'), _countof(szText));
		auto_memset(szType, _T('\0'), _countof(szType));
		auto_memset(szOption, _T('\0'), _countof(szOption));
		if (((pFuncInfo->nInfo >> 8) & 0x01) == 1) {
			// スタティック宣言(Static)
			// 2006.12.12 Moca 末尾にスペース追加
			_tcscpy(szOption, LS(STR_DLGFNCLST_VB_STATIC));
		}
		switch ((pFuncInfo->nInfo >> 4) & 0x0f) {
			case 2  :	// プライベート(Private)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PRIVATE), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
				break;

			case 3  :	// フレンド(Friend)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_FRIEND), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
				break;

			default :	// パブリック(Public)
				_tcsncat(szOption, LS(STR_DLGFNCLST_VB_PUBLIC), _countof(szOption) - _tcslen(szOption)); //	2006.12.17 genta サイズ誤り修正
		}
		int nInfo = pFuncInfo->nInfo;
		switch (nInfo & 0x0f) {
			case 1:		// 関数(Function)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_FUNCTION));
				break;

			// 2006.12.12 Moca ステータス→プロシージャに変更
			case 2:		// プロシージャ(Sub)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROC));
				break;

			case 3:		// プロパティ 取得(Property Get)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPGET));
				break;

			case 4:		// プロパティ 設定(Property Let)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPLET));
				break;

			case 5:		// プロパティ 参照(Property Set)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_PROPSET));
				break;

			case 6:		// 定数(Const)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_CONST));
				break;

			case 7:		// 列挙型(Enum)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_ENUM));
				break;

			case 8:		// ユーザ定義型(Type)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_TYPE));
				break;

			case 9:		// イベント(Event)
				_tcscpy(szType, LS(STR_DLGFNCLST_VB_EVENT));
				break;

			default:	// 未定義なのでクリア
				nInfo	= 0;

		}
		if (((nInfo >> 8) & 0x02) == 2) {
			// 宣言(Declareなど)
			_tcsncat(szType, LS(STR_DLGFNCLST_VB_DECL), _countof(szType) - _tcslen(szType));
		}

		TCHAR szTypeOption[256]; // 2006.12.12 Moca auto_sprintfの入出力で同一変数を使わないための作業領域追加
		if (nInfo == 0) {
			szTypeOption[0] = _T('\0');	//	2006.12.17 genta 全体を0で埋める必要はない
		}else
		if (szOption[0] == _T('\0')) {
			auto_sprintf(szTypeOption, _T("%ts"), szType);
		}else {
			auto_sprintf(szTypeOption, _T("%ts（%ts）"), szType, szOption);
		}
		item.pszText = szTypeOption;
		item.iItem = (int)i;
		item.iSubItem = FL_COL_REMARK;
		ListView_SetItem(hwndList, &item);

		// クリップボードにコピーするテキストを編集
		if (item.pszText[0] != _T('\0')) {
			// 検出結果の種類(関数,,,)があるとき
			// 2006.12.12 Moca szText を自分自身にコピーしていたバグを修正
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				pFuncInfoArr->szFilePath.c_str(),	// 解析対象ファイル名
				pFuncInfo->nFuncLineCRLF,			// 検出行番号
				pFuncInfo->nFuncColCRLF				// 検出桁番号
			);
			memClipText.AppendStringT(szText);
			// "%ts(%ts)\r\n"
			memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
			memClipText.AppendStringLiteral(L"(");
			memClipText.AppendStringT(item.pszText);
			memClipText.AppendStringLiteral(L")\r\n");
		}else {
			// 検出結果の種類(関数,,,)がないとき
			auto_sprintf(
				szText,
				_T("%ts(%d,%d): "),
				pFuncInfoArr->szFilePath.c_str(),	// 解析対象ファイル名
				pFuncInfo->nFuncLineCRLF,			// 検出行番号
				pFuncInfo->nFuncColCRLF				// 検出桁番号
			);
			memClipText.AppendStringT(szText);
			// "%ts\r\n"
			memClipText.AppendNativeDataT(pFuncInfo->memFuncName);
			memClipText.AppendStringLiteral(L"\r\n");
		}
	}

	// 2002.02.08 hor Listは列幅調整とかを実行する前に表示しとかないと変になる
	::ShowWindow(hwndList, SW_SHOW);
	// 列の幅をデータに合わせて調整
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

/*! 汎用ツリーコントロールの初期化：FuncInfo::nDepthを利用して親子を設定

	@param[in] tagjump タグジャンプ形式で出力する

	@date 2002.04.01 YAZAKI
	@date 2002.11.10 Moca 階層の制限をなくした
	@date 2007.02.25 genta クリップボード出力をタブジャンプ可能な書式に変更
	@date 2007.03.04 genta タブジャンプ可能な書式に変更するかどうかのフラグを追加
	@date 2014.06.06 Moca 他ファイルへのタグジャンプ機能を追加
*/
void DlgFuncList::SetTree(bool tagjump, bool nolabel)
{
	HTREEITEM hItemSelected = NULL;
	HTREEITEM hItemSelectedTop = NULL;
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);

	size_t nFuncInfoArrNum = pFuncInfoArr->GetNum();
	size_t nStackPointer = 0;
	size_t nStackDepth = 32; // phParentStack の確保している数
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

		/*	新しいアイテムを作成
			現在の親の下にぶら下げる形で、最後に追加する。
		*/
		HTREEITEM hItem;
		TV_INSERTSTRUCT cTVInsertStruct;
		cTVInsertStruct.hParent = phParentStack[nStackPointer];
		cTVInsertStruct.hInsertAfter = TVI_LAST;	//	必ず最後に追加。
		cTVInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		cTVInsertStruct.item.pszText = pFuncInfo->memFuncName.GetStringPtr();
		cTVInsertStruct.item.lParam = i;	//	あとでこの数値（＝pcFuncInfoArrの何番目のアイテムか）を見て、目的地にジャンプするぜ!!。

		// 親子関係をチェック
		if (nStackPointer != pFuncInfo->nDepth) {
			//	レベルが変わりました!!
			//	※が、2段階深くなることは考慮していないので注意。
			//	　もちろん、2段階以上浅くなることは考慮済み。

			// 2002.11.10 Moca 追加 確保したサイズでは足りなくなった。再確保
			if (nStackDepth <= pFuncInfo->nDepth + 1) {
				nStackDepth = pFuncInfo->nDepth + 4; // 多めに確保しておく
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

		// pFuncInfoに登録されている行数、桁を確認して、選択するアイテムを考える
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

		// クリップボードコピー用テキストを作成する
		//	2003.06.22 Moca dummy要素はツリーに入れるがTAGJUMPには加えない
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
				//	2007.03.04 genta タグジャンプできる形式で書き込む
				text.AppendString(pszFileName);
				
				if (0 < pFuncInfo->nFuncLineCRLF) {
					TCHAR linenum[32];
					int len = auto_sprintf(linenum, _T("(%d,%d): "),
						pFuncInfo->nFuncLineCRLF,				// 検出行番号
						pFuncInfo->nFuncColCRLF					// 検出桁番号
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
			memClipText.AppendNativeDataT(text);	// クリップボードコピー用テキスト
		}
	}

end_of_func:;

	EnableItem(IDC_BUTTON_COPY, true);

	if (hItemSelected) {
		// 現在カーソル位置のメソッドを選択状態にする
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


/*! ファイルツリー作成
	@note pFuncInfoArrにフルパス情報を書き込みつつツリーを作成
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
		// item.szTargetPath => szPath メタ文字の展開
		if (!FileNameManager::ExpandMetaToFolder(item.szTargetPath, szPath, _countof(szPath))) {
			auto_strcpy_s(szPath, _countof(szPath), _T("<Error:Long Path>"));
		}
		// szPath => szPath2 <iniroot>展開
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
		// szPath2 => szPath 「.」やショートパス等の展開
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
		// 0 以下(nFuncInfo): pFuncInfoArr->At(nFuncInfo)にファイル名
		// -1: Grepのファイル名要素
		// -2: Grepのサブフォルダ要素
		// -(nFuncInfo * 10 + 3): Grepルートフォルダ要素
		// -4: データ・追加操作なし
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
	int nItem = 0; // 設定Item番号
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

	//フォルダ一覧作成
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
		tvis.item.cChildren = 1; // ダミーの子要素を持たせて[+]を表示
		TreeView_InsertItem(hwndTree, &tvis);
	}

	//ファイル一覧作成
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
		// [+]記号削除
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
	// 2005.10.21 zenryaku 1行選択
	ListView_SetExtendedListViewStyle(hwndList,
		ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	::GetWindowRect(hwndList, &rc);
	nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = rc.right - rc.left - (nColWidthArr[1] + nColWidthArr[2] + nColWidthArr[3]) - nCxVScroll - 8;
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_LINE_M));
	col.iSubItem = FL_COL_ROW;
	ListView_InsertColumn(hwndList, FL_COL_ROW, &col);

	// 2010.03.17 syat 桁追加
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_COL];
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_COL));
	col.iSubItem = FL_COL_COL;
	ListView_InsertColumn(hwndList, FL_COL_COL, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_NAME];
	//	Apr. 23, 2005 genta 行番号を左端へ
	col.pszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_LIST_FUNC));
	col.iSubItem = FL_COL_NAME;
	ListView_InsertColumn(hwndList, FL_COL_NAME, &col);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nColWidthArr[FL_COL_REMARK];
	col.pszText = const_cast<TCHAR*>(_T(" "));
	col.iSubItem = FL_COL_REMARK;
	ListView_InsertColumn(hwndList, FL_COL_REMARK, &col);

	// アウトライン位置とサイズを初期化する // 20060201 aroka
	EditView* pEditView = (EditView*)(this->lParam);
	if (pEditView) {
		if (!IsDocking() && pShareData->common.outline.bRememberOutlineWindowPos) {
			WINDOWPLACEMENT windowPlacement;
			windowPlacement.length = sizeof(windowPlacement);
			if (::GetWindowPlacement(pEditView->editWnd.GetHwnd(), &windowPlacement)) {
				// ウィンドウ位置・サイズを-1以外の値にしておくと、Dialogで使用される．
				xPos = pShareData->common.outline.xOutlineWindowPos + windowPlacement.rcNormalPosition.left;
				yPos = pShareData->common.outline.yOutlineWindowPos + windowPlacement.rcNormalPosition.top;
				nWidth =  pShareData->common.outline.widthOutlineWindow;
				nHeight = pShareData->common.outline.heightOutlineWindow;
			}
		}else if (IsDocking()) {
			xPos = 0;
			yPos = 0;
			nShowCmd = SW_HIDE;
			::GetWindowRect(::GetParent(pEditView->GetHwnd()), &rc);	// ここではまだ GetDockSpaceRect() は使えない
			DockSideType dockSideType = GetDockSide();
			switch (dockSideType) {
			case DockSideType::Left:	nWidth = ProfDockLeft();		break;
			case DockSideType::Top:		nHeight = ProfDockTop();		break;
			case DockSideType::Right:	nWidth = ProfDockRight();		break;
			case DockSideType::Bottom:	nHeight = ProfDockBottom();	break;
			}
			if (dockSideType == DockSideType::Left || dockSideType == DockSideType::Right) {
				if (nWidth == 0) { // 初回
					nWidth = (rc.right - rc.left) / 3;
				}
				if (nWidth > rc.right - rc.left - DOCK_MIN_SIZE) {
					nWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				}
				if (nWidth < DOCK_MIN_SIZE) {
					nWidth = DOCK_MIN_SIZE;
				}
			}else {
				if (nHeight == 0) { // 初回
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

	if (!bInChangeLayout) {	// ChangeLayout() 処理中は設定変更しない
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ProfDockDisp() = TRUE;
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);

		}
		// 他ウィンドウに変更を通知する
		if (ProfDockSync()) {
			HWND hwndEdit = pEditView->editWnd.GetHwnd();
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);
		}
	}

	if (!IsDocking()) {
		// 基底クラスメンバ
		CreateSizeBox();

		LONG_PTR lStyle = ::GetWindowLongPtr(GetHwnd(), GWL_STYLE);
		::SetWindowLongPtr(GetHwnd(), GWL_STYLE, lStyle | WS_THICKFRAME);
		::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	hwndToolTip = NULL;
	if (IsDocking()) {
		// ツールチップを作成する。（「閉じる」などのボタン用）
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

		// ツールチップをマルチライン可能にする（SHRT_MAX: Win95でINT_MAXだと表示されない）
		Tooltip_SetMaxTipWidth(hwndToolTip, SHRT_MAX);

		// アウトラインにツールチップを追加する
		TOOLINFO	ti;
		ti.cbSize      = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId は HWND で rect は無視（HWND 全体）
		ti.hwnd        = GetHwnd();
		ti.hinst       = hInstance;
		ti.uId         = (UINT_PTR)GetHwnd();
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		Tooltip_AddTool(hwndToolTip, &ti);

		// 不要なコントロールを隠す
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
		// ドッキング中はウィンドウ幅いっぱいまで伸ばす
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
		//「アウトライン解析」のヘルプ
		// Apr. 5, 2001 JEPRO 修正漏れを追加 (Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした)
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_OUTLINE));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDOK:
		return OnJump();
	case IDCANCEL:
		if (bModal) {		// モーダル ダイアログか
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
		// Windowsクリップボードにコピー 
		// 2004.02.17 Moca 関数化
		SetClipboardText(GetHwnd(), memClipText.GetStringPtr(), memClipText.GetStringLength());
		return TRUE;
	case IDC_BUTTON_WINSIZE:
		{// ウィンドウの位置とサイズを記憶 // 20060201 aroka
			pShareData->common.outline.bRememberOutlineWindowPos = IsButtonChecked(IDC_BUTTON_WINSIZE);
		}
		// ボタンが押されているかはっきりさせる 2008/6/5 Uchi
		SetItemText(IDC_BUTTON_WINSIZE,
			pShareData->common.outline.bRememberOutlineWindowPos ? _T("■") : _T("□"));
		return TRUE;
	// 2002.02.08 オプション切替後List/Treeにフォーカス移動
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
	// 基底クラスメンバ
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
				// この時点ではまだ選択変更されていないが OnJump() の予備動作として先に選択変更しておく
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
			// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 3/4
			OnJump();
			bWaitTreeProcess=true;
			::SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, TRUE);	// ツリーの展開／縮小をしない
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
			// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 4/4
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
			//	Apr. 23, 2005 genta 関数として独立させた
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
					{	// 選択アイテムを反転表示にする
						const TypeConfig* typeDataPtr = &(pEditView->pEditDoc->docType.GetDocumentAttribute());
						COLORREF clrText = typeDataPtr->colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
						COLORREF clrTextBk = typeDataPtr->colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;
						if (hwndList == pnmh->hwndFrom) {
							//if (lpnmcd->uItemState & CDIS_SELECTED) {	// 非選択のアイテムもすべて CDIS_SELECTED で来る？
							if (ListView_GetItemState(hwndList, lpnmcd->dwItemSpec, LVIS_SELECTED)) {
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrText = clrText ^ RGB(255, 255, 255);
								((LPNMLVCUSTOMDRAW)lpnmcd)->clrTextBk = clrTextBk ^ RGB(255, 255, 255);
								lpnmcd->uItemState = 0;	// リストビューには選択としての描画をさせないようにする？
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
	指定されたカラムでリストビューをソートする．
	同時にヘッダも書き換える．

	ソート後はフォーカスが画面内に現れるように表示位置を調整する．

	@par 表示位置調整の小技
	EnsureVisibleの結果は，上スクロールの場合は上端に，下スクロールの場合は
	下端に目的の項目が現れる．端から少し離したい場合はオフセットを与える必要が
	あるが，スクロール方向がわからないと±がわからない
	そのため最初に一番下に一回スクロールさせることでEnsureVisibleでは
	かならず上スクロールになるようにすることで，ソート後の表示位置を
	固定する

	@param[in] hwndList	リストビューのウィンドウハンドル
	@param[in] sortcol	ソートするカラム番号(0-2)

	@date 2005.04.23 genta 関数として独立させた
	@date 2005.04.29 genta ソート後の表示位置調整
	@date 2010.03.17 syat 桁追加
*/
void DlgFuncList::SortListView(
	HWND hwndList,
	int sortcol
	)
{
	LV_COLUMN col;
	int col_no;

	//	Apr. 23, 2005 genta 行番号を左端へ

//	if (sortcol == 1) {
	{
		col_no = FL_COL_NAME;
		col.mask = LVCF_TEXT;
	// From Here 2001.12.03 hor
	//	col.pszText = _T("関数名 *");
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

		// 2010.03.17 syat 桁追加
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
	//	2005.04.23 zenryaku 選択された項目が見えるようにする

	//	Apr. 29, 2005 genta 一旦一番下にスクロールさせる
	ListView_EnsureVisible(
		hwndList,
		ListView_GetItemCount(hwndList) - 1,
		FALSE
	);
	
	//	Jan.  9, 2006 genta 先頭から1つ目と2つ目の関数が
	//	選択された場合にスクロールされなかった
	int keypos = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED) - 2;
	ListView_EnsureVisible(
		hwndList,
		keypos >= 0 ? keypos : 0,
		FALSE
	);
}

/*!	ウィンドウサイズが変更された

	@date 2003.06.22 Moca コードの整理(コントロールの処理方法をテーブルに持たせる)
	@date 2003.08.16 genta 配列はstaticに(無駄な初期化を行わないため)
*/
BOOL DlgFuncList::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 今のところ EditWnd::OnSize() からの呼び出しでは lParam は EditWnd 側 の lParam のまま渡される	// 2010.06.05 ryoji
	RECT rcDlg;
	::GetClientRect(GetHwnd(), &rcDlg);
	lParam = MAKELONG(rcDlg.right - rcDlg.left, rcDlg.bottom -  rcDlg.top);	// 自前で補正

	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	RECT  rc;
	POINT ptNew;
	ptNew.x = rcDlg.right - rcDlg.left;
	ptNew.y = rcDlg.bottom - rcDlg.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		HWND hwndCtrl = GetItemHwnd(anchorList[i].id);
		ResizeItem(hwndCtrl, ptDefaultSizeClient, ptNew, rcItems[i], anchorList[i].anchor, (anchorList[i].anchor != AnchorStyle::All));
//	2013.2.6 aroka ちらつき防止用の試行錯誤
		if (anchorList[i].anchor == AnchorStyle::All) {
			::UpdateWindow(hwndCtrl);
		}
	}

//	if (IsDocking())
	{
		// ダイアログ部分を再描画（ツリー／リストの範囲はちらつかないように除外）
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

	// アウトライン ■位置とサイズを記憶する	// 20060201 aroka
	// 前提条件：lParam が Dialog::OnDestroy でクリアされないこと
	EditView* pEditView = (EditView*)lParam;
	HWND hwndEdit = pEditView->editWnd.GetHwnd();
	if (!IsDocking() && pShareData->common.outline.bRememberOutlineWindowPos) {
		// 親のウィンドウ位置・サイズを記憶
		WINDOWPLACEMENT windowPlacement;
		windowPlacement.length = sizeof(windowPlacement);
		if (::GetWindowPlacement(hwndEdit, &windowPlacement)) {
			// ウィンドウ位置・サイズを記憶
			pShareData->common.outline.xOutlineWindowPos = xPos - windowPlacement.rcNormalPosition.left;
			pShareData->common.outline.yOutlineWindowPos = yPos - windowPlacement.rcNormalPosition.top;
			pShareData->common.outline.widthOutlineWindow = nWidth;
			pShareData->common.outline.heightOutlineWindow = nHeight;
		}
	}

	// ドッキング画面を閉じるときは画面を再レイアウトする
	// ドッキングでアプリ終了時には hwndEdit は NULL になっている（親に先に WM_DESTROY が送られるため）
	if (IsDocking() && hwndEdit) {
		pEditView->editWnd.EndLayoutBars();
	}

	// 明示的にアウトライン画面を閉じたときだけアウトライン表示フラグを OFF にする
	// フローティングでアプリ終了時やタブモードで裏にいる場合は ::IsWindowVisible(hwndEdit) が FALSE を返す
	if (hwndEdit && ::IsWindowVisible(hwndEdit) && !bInChangeLayout) {	// ChangeLayout() 処理中は設定変更しない
		bool bType = (ProfDockSet() != 0);
		if (bType) {
			DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ProfDockDisp() = FALSE;
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);
		}
		// 他ウィンドウに変更を通知する
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

	// ファイルを開いていない場合は自分で開く
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
	)	// 2002.02.08 hor 引数追加
{
	size_t nLineTo;
	size_t nColTo;
	// ダイアログデータの取得
	if (0 < GetData() && (funcInfo || 0 < sJumpFile.size() )) {
		if (bModal) {		// モーダル ダイアログか
			// モーダル表示する場合は、funcInfoを取得するアクセサを実装して結果取得すること。
			::EndDialog(GetHwnd(), 1);
		}else {
			bool bFileJumpSelf = true;
			if (0 < sJumpFile.size()) {
				if (bFileJump) {
					// ファイルツリーの場合
					if (bModal) {		// モーダル ダイアログか
						// モーダル表示する場合は、funcInfoを取得するアクセサを実装して結果取得すること。
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
					// 別のファイルへジャンプ
					Point poCaret; // TagJumpSubも1開始
					poCaret.x = (int)nColTo;
					poCaret.y = (int)nLineTo;
					bFileJumpSelf = TagJumpTimer(funcInfo->memFileName.GetStringPtr(), poCaret, bCheckAutoClose);
				}
			}else {
				nLineTo = funcInfo->nFuncLineCRLF;
				nColTo = funcInfo->nFuncColCRLF;
				// カーソルを移動させる
				Point	poCaret;
				poCaret.x = (int)nColTo - 1;
				poCaret.y = (int)nLineTo - 1;

				pShareData->workBuffer.logicPoint = poCaret;

				//	2006.07.09 genta 移動時に選択状態を保持するように
				::SendMessage(((EditView*)lParam)->editWnd.GetHwnd(),
					MYWM_SETCARETPOS, 0, PM_SETCARETPOS_KEEPSELECT );
			}
			if (bCheckAutoClose && bFileJumpSelf) {
				// アウトライン ダイアログを自動的に閉じる
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


// キー操作をコマンドに変換するヘルパー関数
void DlgFuncList::Key2Command(WORD KeyCode)
{
	EditView*	pEditView;
// novice 2004/10/10
	// Shift,Ctrl,Altキーが押されていたか
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
	case F_OUTLINE_TOGGLE: // 20060201 aroka フォーカスがあるときはリロード
	case F_BOOKMARK_VIEW:
	case F_FILETREE:
		pEditView=(EditView*)lParam;
		pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0); // 引数の変更 20060201 aroka

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
	pFuncInfoArr = pFuncInfoArr;	// 関数情報配列
	nCurLine = nCurLine;				// 現在行
	nCurCol = nCurCol;				// 現在桁

	bool bType = (ProfDockSet() != 0);
	if (bType) {
		type.nDockOutline = nOutlineType;
		SetTypeConfig( TypeConfigNum(nDocType), type );
	}else {
		CommonSet().nDockOutline = nOutlineType;
	}

	SetData();
}

// ダイアログタイトルの設定
void DlgFuncList::SetWindowText(const TCHAR* szTitle)
{
	::SetWindowText(GetHwnd(), szTitle);
}

/** 配色適用処理
	@date 2010.06.05 ryoji 新規作成
*/
void DlgFuncList::SyncColor(void)
{
	if (!IsDocking()) {
		return;
	}
#ifdef DEFINE_SYNCCOLOR
	// テキスト色・背景色をビューと同色にする
	EditView* pEditView = (EditView*)lParam;
	const TypeConfig* pTypeData = &(pEditView->pEditDoc->docType.GetDocumentAttribute());
	COLORREF clrText = pTypeData->colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
	COLORREF clrBack = pTypeData->colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	TreeView_SetTextColor(hwndTree, clrText);
	TreeView_SetBkColor(hwndTree, clrBack);
	{
		// WinNT4.0 あたりではウィンドウスタイルを強制的に再設定しないと
		// ツリーアイテムの左側が真っ黒になる
		LONG lStyle = (LONG)GetWindowLongPtr(hwndTree, GWL_STYLE);
		SetWindowLongPtr(hwndTree, GWL_STYLE, lStyle & ~(TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT));
		SetWindowLongPtr(hwndTree, GWL_STYLE, lStyle);
	}
	::SetWindowPos(hwndTree, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);	// なぜかこうしないと四辺１ドット幅分だけ色変更が即時適用されない（←スタイル再設定とは無関係）
	::InvalidateRect(hwndTree, NULL, TRUE);

	HWND hwndList = GetItemHwnd(IDC_LIST_FL);
	ListView_SetTextColor(hwndList, clrText);
	ListView_SetTextBkColor(hwndList, clrBack);
	ListView_SetBkColor(hwndList, clrBack);
	::InvalidateRect(hwndList, NULL, TRUE);
#endif
}

/** ドッキング対象矩形の取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
*/
void DlgFuncList::GetDockSpaceRect(LPRECT pRect)
{
	EditView* pEditView = (EditView*)lParam;
	// DlgFuncList と CSplitterWnd の外接矩形
	// 2014.12.02 ミニマップ対応
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

/**キャプション矩形取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
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

/** キャプション上のボタン矩形取得（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
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

/** 分割バーへのヒットテスト（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
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

/** キャプション上のボタンへのヒットテスト（スクリーン座標）
	@date 2010.06.05 ryoji 新規作成
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
			nBtn = i;	// 右端から i 番目のボタン上
			break;
		}
		::OffsetRect(&rcBtn, -(rcBtn.right - rcBtn.left), 0);
	}

	return nBtn;
}

/** WM_NCCALCSIZE 処理
	@date 2010.06.05 ryoji 新規作成
*/
INT_PTR DlgFuncList::OnNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!IsDocking()) {
		return 0L;
	}

	// 自ウィンドウのクライアント領域を定義する
	// これでキャプションや分割バーを非クライアント領域にすることができる
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

/** WM_NCHITTEST 処理
	@date 2010.06.05 ryoji 新規作成
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

/** WM_TIMER 処理
	@date 2010.06.05 ryoji 新規作成
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
		// カーソルがウィンドウ外にある場合にも WM_NCMOUSEMOVE を送る
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

/** WM_NCMOUSEMOVE 処理
	@date 2010.06.05 ryoji 新規作成
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

	// カーソルがウィンドウ内に入ったらタイマー起動
	// ウィンドウ外に出たらタイマー削除
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

	// マウスカーソルがボタン上にあればハイライト
	int nHilightedBtn = HitTestCaptionButton(pt.x, pt.y);
	if (this->nHilightedBtn != nHilightedBtn) {
		// ハイライト状態の変更を反映するために再描画する
		this->nHilightedBtn = nHilightedBtn;
		::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);

		// ツールチップ更新
		TOOLINFO ti = {0};
		ti.cbSize	= CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.hwnd		= GetHwnd();
		ti.hinst	= hInstance;
		ti.uId		= (UINT_PTR)GetHwnd();
		switch (nHilightedBtn) {
		case 0: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_CLOSE)); break;
		case 1: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_WIN)); break;
		case 2: ti.lpszText = const_cast<TCHAR*>(LS(STR_DLGFNCLST_TIP_UPDATE)); break;
		default: ti.lpszText = NULL;	// 消す
		}
		Tooltip_UpdateTipText(hwndToolTip, &ti);
	}

	return 0L;
}

/** WM_MOUSEMOVE 処理
	@date 2010.06.05 ryoji 新規作成
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

	if (bStretching) {	// マウスのドラッグ位置にあわせてサイズを変更する
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		::ClientToScreen(GetHwnd(), &pt);

		RECT rc;
		GetDockSpaceRect(&rc);

		// 画面サイズが小さすぎるときは何もしない
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

		// マウスが上下左右に行き過ぎなら範囲内に調整する
		if (pt.x > rc.right - DOCK_MIN_SIZE)	pt.x = rc.right - DOCK_MIN_SIZE;
		if (pt.x < rc.left + DOCK_MIN_SIZE)	pt.x = rc.left + DOCK_MIN_SIZE;
		if (pt.y > rc.bottom - DOCK_MIN_SIZE)	pt.y = rc.bottom - DOCK_MIN_SIZE;
		if (pt.y < rc.top + DOCK_MIN_SIZE)		pt.y = rc.top + DOCK_MIN_SIZE;

		// クライアント座標系に変換して新しい位置とサイズを計算する
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

		// 以前と同じ配置なら無駄に移動しない
		RECT rcOld;
		GetWindowRect(&rcOld);
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top);
		if (::EqualRect(&rcOld, &rc)) {
			return 0L;
		}

		// 移動する
		::SetWindowPos(GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		((EditView*)lParam)->editWnd.EndLayoutBars(bEditWndReady);

		// 移動後の配置情報を記憶する
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

/** WM_NCLBUTTONDOWN 処理
	@date 2010.06.05 ryoji 新規作成
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
				// 上の SetActiveWindow() で WM_ACTIVATEAPP へ行くケースでは、WM_ACTIVATEAPP に入れた特殊処理（エディタ本体を一時的にアクティブ化して戻す）
				// に余計に時間がかかるため、上の SetActiveWindow() 後にはボタンが離されていることがある。その場合は Track() を開始せずに抜ける。
				if ((::GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON)? VK_RBUTTON: VK_LBUTTON) & 0x8000) == 0) {
					return 1L;	// ボタンは既に離されている
				}
				Track(pt);	// タイトルバーのドラッグ＆ドロップによるドッキング配置変更
				return 1L;
			}
		}
		return 0L;
	}

	int nBtn;
	if (HitTestSplitter(pt.x, pt.y)) {	// 分割バー
		bStretching = true;
		::SetCapture(GetHwnd());	// OnMouseMoveでのサイズ制限のために自前のキャプチャが必要
	}else {
		if ((nBtn = HitTestCaptionButton(pt.x, pt.y)) >= 0) {	// キャプション上のボタン
			if (nBtn == 1) {	// メニュー
				RECT rcBtn;
				GetCaptionButtonRect(nBtn, &rcBtn);
				pt.x = rcBtn.left;
				pt.y = rcBtn.bottom;
				DoMenu(pt, GetHwnd());
				// メニュー選択せずにリストやツリーをクリックしたらボタンがハイライトのままになるので更新
				::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOINTERNALPAINT);
			}else {
				nCapturingBtn = nBtn;
				::SetCapture(GetHwnd());
			}
		}else {	// 残りはタイトルバーのみ
			Track(pt);	// タイトルバーのドラッグ＆ドロップによるドッキング配置変更
		}
	}

	return 1L;
}

/** WM_LBUTTONUP 処理
	@date 2010.06.05 ryoji 新規作成
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
			// 他ウィンドウに変更を通知する
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
			if (nBtn == 0) {	// 閉じる
				::DestroyWindow(GetHwnd());
			}else if (nCapturingBtn == 2) {	// 更新
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

/** WM_NCPAINT 処理
	@date 2010.06.05 ryoji 新規作成
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

	// 描画対象
	hdc = ::GetWindowDC(hwnd);
	Graphics gr(hdc);
	::GetWindowRect(hwnd, &rcScr);
	rc = rcScr;
	::OffsetRect(&rc, -rcScr.left, -rcScr.top);

	// 背景を描画する
	//::FillRect(gr, &rc, (HBRUSH)(COLOR_3DFACE + 1));

	// 分割線を描画する
	rcWk = rc;
	switch (dockSideType) {
	case DockSideType::Left:	rcWk.left = rcWk.right - DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Top:		rcWk.top = rcWk.bottom - DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Right:	rcWk.right = rcWk.left + DOCK_SPLITTER_WIDTH; break;
	case DockSideType::Bottom:	rcWk.bottom = rcWk.top + DOCK_SPLITTER_WIDTH; break;
	}
	::FillRect(gr, &rcWk, (HBRUSH)(COLOR_3DFACE + 1));
	::DrawEdge(gr, &rcWk, EDGE_ETCHED, BF_TOPLEFT);

	// タイトルを描画する
	BOOL bThemeActive = UxTheme::getInstance().IsThemeActive();
	BOOL bGradient = FALSE;
	::SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &bGradient, 0);
	if (!bThemeActive) {
		bGradient = FALSE;	// 適当に調整
	}
	HWND hwndFocus = ::GetFocus();
	BOOL bActive = (GetHwnd() == hwndFocus || ::IsChild(GetHwnd(), hwndFocus));
	RECT rcCaption;
	GetCaptionRect(&rcCaption);
	::OffsetRect(&rcCaption, -rcScr.left, -rcScr.top);
	rcWk = rcCaption;
	rcWk.top += 1;
	rcWk.right -= DOCK_BUTTON_NUM * (::GetSystemMetrics(SM_CXSMSIZE));
	// ↓DrawCaption() に DC_SMALLCAP を指定してはいけないっぽい
	// ↓DC_SMALLCAP 指定のものを Win7(64bit版) で動かしてみたら描画位置が下にずれて上半分しか見えなかった（x86ビルド/x64ビルドのどちらも NG）
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

	// タイトル上のボタンを描画する
	NONCLIENTMETRICS ncm;
	ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0);
	LOGFONT lf = {0};
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = ncm.lfCaptionFont.lfHeight;
	::lstrcpy(lf.lfFaceName, _T("Marlett"));
	HFONT hFont = ::CreateFontIndirect(&lf);
	::lstrcpy(lf.lfFaceName, _T("Wingdings"));
	HFONT hFont2 = ::CreateFontIndirect(&lf);
	gr.SetTextBackTransparent(true);

	static const TCHAR szBtn[DOCK_BUTTON_NUM] = { (TCHAR)0x72/* 閉じる */, (TCHAR)0x36/* メニュー */, (TCHAR)0xFF/* 更新 */ };
	HFONT hFontBtn[DOCK_BUTTON_NUM] = { hFont/* 閉じる */, hFont/* メニュー */, hFont2/* 更新 */ };
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
		// マウスカーソルがボタン上にあればハイライト
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

/** メニュー処理
	@date 2010.06.05 ryoji 新規作成
*/
void DlgFuncList::DoMenu(POINT pt, HWND hwndFrom)
{
	// メニューを作成する
	EditView* pEditView = &EditDoc::GetInstance(0)->pEditWnd->GetActiveView();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
	DockSideType dockSideType = ProfDockSide();	// 設定上の配置
	UINT uFlags = MF_BYPOSITION | MF_STRING;
	HMENU hMenu = ::CreatePopupMenu();
	HMENU hMenuSub = ::CreatePopupMenu();
	int iPos = 0;
	int iPosSub = 0;
	HMENU& hMenuRef = (hwndFrom == GetHwnd())? hMenu: hMenuSub;
	int& iPosRef = (hwndFrom == GetHwnd())? iPos: iPosSub;

	if (hwndFrom != GetHwnd()) {
		// 将来、ここに hwndFrom に応じた状況依存メニューを追加するといいかも
		// （ツリーなら「すべて展開」／「すべて縮小」とか、そういうの）
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

	// メニューを表示する
	RECT rcWork;
	GetMonitorWorkRect(pt, &rcWork);	// モニタのワークエリア
	int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
								(pt.x > rcWork.left)? pt.x: rcWork.left,
								(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
								0, GetHwnd(), NULL);
	::DestroyMenu(hMenu);	// サブメニューは再帰的に破棄される

	// メニュー選択された状態に切り替える
	HWND hwndEdit = pEditView->editWnd.GetHwnd();
	if (nId == 450) {	// 更新
		EFunctionCode nFuncCode = GetFuncCodeRedraw(nOutlineType);
		EditView* pEditView = (EditView*)(this->lParam);
		pEditView->GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Reload, 0, 0, 0);
	}else if (nId == 451) {	// コピー
		// Windowsクリップボードにコピー 
		SetClipboardText(GetHwnd(), memClipText.GetStringPtr(), memClipText.GetStringLength());
	}else if (nId == 452) {	// 閉じる
		::DestroyWindow(GetHwnd());
	}else if (nId == 300 || nId == 301) {	// ドッキング配置の継承方法
		ProfDockSet() = nId - 300;
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// 自分自身への強制変更
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// 他ウィンドウにドッキング配置変更を通知する
		}
	}else if (nId == 305) {	// 設定コピー
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
				switch (GetDockSide()) {	// 現在のドッキングモード
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
			ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// 自分自身への強制変更
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// 他ウィンドウにドッキング配置変更を通知する
		}
	}else if (nId == 200) {	// ドッキング配置の同期をとる
		ProfDockSync() = !ProfDockSync();
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// 自分自身への強制変更
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// 他ウィンドウにドッキング配置変更を通知する
		}
	}else if (nId >= 100 - 1) {	// ドッキングモード （※ DockSideType::Undockable は -1 です） */
		int* pnWidth = NULL;
		int* pnHeight = NULL;
		RECT rc;
		GetDockSpaceRect(&rc);
		dockSideType = DockSideType(nId - 100);	// 新しいドッキングモード
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
				if (*pnWidth == 0) {	// 初回
					*pnWidth = (rc.right - rc.left) / 3;
				}
				if (*pnWidth > rc.right - rc.left - DOCK_MIN_SIZE) {
					*pnWidth = rc.right - rc.left - DOCK_MIN_SIZE;
				}
				if (*pnWidth < DOCK_MIN_SIZE) {
					*pnWidth = DOCK_MIN_SIZE;
				}
			}else {
				if (*pnHeight == 0) {	// 初回
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

		// ドッキング配置変更
		ProfDockDisp() = GetHwnd()? TRUE: FALSE;
		ProfDockSide() = dockSideType;	// 新しいドッキングモードを適用
		if (bType) {
			SetTypeConfig(TypeConfigNum(nDocType), type);
		}
		ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// 自分自身への強制変更
		if (ProfDockSync()) {
			PostOutlineNotifyToAllEditors((WPARAM)0, (LPARAM)hwndEdit);	// 他ウィンドウにドッキング配置変更を通知する
		}
	}
}

/** 現在の設定に応じて表示を刷新する
	@date 2010.06.05 ryoji 新規作成
*/
void DlgFuncList::Refresh(void)
{
	EditWnd* pEditWnd = EditDoc::GetInstance(0)->pEditWnd;
	BOOL bReloaded = ChangeLayout(OUTLINE_LAYOUT_FILECHANGED);	// 現在設定に従ってアウトライン画面を再配置する
	if (!bReloaded && pEditWnd->dlgFuncList.GetHwnd()) {
		OutlineType nOutlineType = GetOutlineTypeRedraw(this->nOutlineType);
		pEditWnd->GetActiveView().GetCommander().Command_FuncList(ShowDialogType::Reload, nOutlineType);	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷Preview状態で実行されないので Command_FUNCLIST()
	}
	if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == pEditWnd->GetHwnd()) {
		::SetFocus(pEditWnd->GetActiveView().GetHwnd());	// フォーカスを戻す
	}
}

/** 現在の設定に応じて配置を変更する（できる限り再解析しない）

	@param nId [in] 動作指定．OUTLINE_LAYOUT_FOREGROUND: 前面用の動作 / OUTLINE_LAYOUT_BACKGROUND: 背後用の動作 / OUTLINE_LAYOUT_FILECHANGED: ファイル切替用の動作（前面だが特殊）
	@retval 解析を実行したかどうか．true: 実行した / false: 実行しなかった

	@date 2010.06.10 ryoji 新規作成
*/
bool DlgFuncList::ChangeLayout(int nId)
{
	struct AutoSwitch {
		AutoSwitch(bool* pbSwitch): pbSwitch(pbSwitch) { *pbSwitch = true; }
		~AutoSwitch() { *pbSwitch = false; }
		bool* pbSwitch;
	} autoSwitch(&bInChangeLayout);	// 処理中は InChangeLayout フラグを ON にしておく

	EditDoc* pDoc = EditDoc::GetInstance(0);	// 今は非表示かもしれないので (EditView*)lParam は使えない
	nDocType = pDoc->docType.GetDocumentType().GetIndex();
	DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);

	BOOL bDockDisp = ProfDockDisp();
	DockSideType eDockSideNew = ProfDockSide();

	if (!GetHwnd()) {	// 現在は非表示
		if (bDockDisp) {	// 新設定は表示
			if (eDockSideNew <= DockSideType::Float) {
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// 裏ではフローティングは開かない（従来互換）※無理に開くとタブモード時は画面が切り替わってしまう
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED) {
					return false;	// ファイル切替ではフローティングは開かない（従来互換）
				}
			}
			// ※ 裏では一時的に Disable 化しておいて開く（タブモードでの不正な画面切り替え抑止）
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
			OutlineType nOutlineType = GetOutlineTypeRedraw(this->nOutlineType);	// ブックマークかアウトライン解析かは最後に開いていた時の状態を引き継ぐ（初期状態はアウトライン解析）
			pEditView->GetCommander().Command_FuncList(ShowDialogType::Normal, nOutlineType);	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷Preview状態で実行されないので Command_FUNCLIST()
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), TRUE);
			}
			return true;	// 解析した
		}
	}else {	// 現在は表示
		DockSideType eDockSideOld = GetDockSide();

		EditView* pEditView = (EditView*)(this->lParam);
		if (!bDockDisp) {	// 新設定は非表示
			if (eDockSideOld <= DockSideType::Float) {	// 現在はフローティング
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// 裏ではフローティングは閉じない（従来互換）
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED && eDockSideNew <= DockSideType::Float) {
					return false;	// ファイル切替では新設定もフローティングなら再利用（従来互換）
				}
			}
			::DestroyWindow(GetHwnd());	// 閉じる
			return false;
		}

		// ドッキング⇔フローティング切替では閉じて開く
		if ((eDockSideOld <= DockSideType::Float) != (eDockSideNew <= DockSideType::Float)) {
			::DestroyWindow(GetHwnd());	// 閉じる
			if (eDockSideNew <= DockSideType::Float) {	// 新設定はフローティング
				xPos = yPos = -1;	// 画面位置を初期化する
				if (nId == OUTLINE_LAYOUT_BACKGROUND) {
					return false;	// 裏ではフローティングは開かない（従来互換）※無理に開くとタブモード時は画面が切り替わってしまう
				}
				if (nId == OUTLINE_LAYOUT_FILECHANGED) {
					return false;	// ファイル切替ではフローティングは開かない（従来互換）
				}
			}
			// ※ 裏では一時的に Disable 化しておいて開く（タブモードでの不正な画面切り替え抑止）
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
			pEditView->GetCommander().Command_FuncList(ShowDialogType::Normal, nOutlineType);	// 開く	※ HandleCommand(F_OUTLINE,...) だと印刷Preview状態で実行されないので Command_FUNCLIST()
			if (nId == OUTLINE_LAYOUT_BACKGROUND) {
				::EnableWindow(pEditView->editWnd.GetHwnd(), TRUE);
			}
			return true;	// 解析した
		}

		// フローティング→フローティングでは配置同期せずに現状維持
		if (eDockSideOld <= DockSideType::Float) {
			eDockSide = eDockSideNew;
			return false;
		}

		// ドッキング→ドッキングでは配置同期
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

		// 以前と同じ配置なら無駄に移動しない
		RECT rcOld;
		GetWindowRect(&rcOld);
		ptLT.x = rcOld.left;
		ptLT.y = rcOld.top;
		::ScreenToClient(hwndParent, &ptLT);
		::OffsetRect(&rcOld, ptLT.x - rcOld.left, ptLT.y - rcOld.top);
		if (eDockSideOld == eDockSideNew && ::EqualRect(&rcOld, &rc)) {
			::InvalidateRect(GetHwnd(), NULL, TRUE);	// いちおう再描画だけ
			return false;	// 配置変更不要（例：別のファイルタイプからの通知）
		}

		// 移動する
		eDockSide = eDockSideNew;	// 自身のドッキング配置の記憶を更新
		::SetWindowPos(
			GetHwnd(), NULL,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | ((eDockSideOld == eDockSideNew)? 0: SWP_FRAMECHANGED)
		);	// SWP_FRAMECHANGED 指定で WM_NCCALCSIZE（非クライアント領域の再計算）に誘導する
		pEditView->editWnd.EndLayoutBars(bEditWndReady);
	}
	return false;
}

/** アウトライン通知(MYWM_OUTLINE_NOTIFY)処理

	wParam: 通知種別
	lParam: 種別毎のパラメータ

	@date 2010.06.07 ryoji 新規作成
*/
void DlgFuncList::OnOutlineNotify(WPARAM wParam, LPARAM lParam)
{
	EditDoc* pDoc = EditDoc::GetInstance(0);	// 今は非表示かもしれないので (EditView*)lParam は使えない
	switch (wParam) {
	case 0:	// 設定変更通知（ドッキングモード or サイズ）, lParam: 通知元の HWND
		if ((HWND)lParam == pDoc->pEditWnd->GetHwnd()) {
			return;	// 自分からの通知は無視
		}
		ChangeLayout(OUTLINE_LAYOUT_BACKGROUND);	// アウトライン画面を再配置
		break;
	}
	return;
}

/** 他ウィンドウにアウトライン通知をポストする
	@date 2010.06.10 ryoji 新規作成
*/
BOOL DlgFuncList::PostOutlineNotifyToAllEditors(WPARAM wParam, LPARAM lParam)
{
	return AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_OUTLINE_NOTIFY, (WPARAM)wParam, (LPARAM)lParam, GetHwnd());
}

void DlgFuncList::SetTypeConfig(TypeConfigNum docType, const TypeConfig& type)
{
	DocTypeManager().SetTypeConfig(docType, type);
}

/** コンテキストメニュー処理
	@date 2010.06.07 ryoji 新規作成
*/
BOOL DlgFuncList::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	// キャプションかリスト／ツリー上ならメニューを表示する
	HWND hwndFrom = (HWND)wParam;
	if (::SendMessage(GetHwnd(), WM_NCHITTEST, 0, lParam) == HTCAPTION
			|| hwndFrom == GetItemHwnd(IDC_LIST_FL)
			|| hwndFrom == GetItemHwnd(IDC_TREE_FL)
	) {
		POINT pt;
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;
		// キーボード（メニューキー や Shift F10）からの呼び出し
		if (pt.x == -1 && pt.y == -1) {
			RECT rc;
			::GetWindowRect(hwndFrom, &rc);
			pt.x = rc.left;
			pt.y = rc.top;
		}
		DoMenu(pt, hwndFrom);
		return TRUE;
	}

	return Dialog::OnContextMenu(wParam, lParam);	// その他のコントロール上ではポップアップヘルプを表示する
}

/** タイトルバーのドラッグ＆ドロップでドッキング配置する際の移動先矩形を求める
	@date 2010.06.17 ryoji 新規作成
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

	// 移動しない矩形を取得する
	RECT rcWnd;
	GetWindowRect(&rcWnd);
	if (IsDocking() && !bForceFloat) {
		if (::PtInRect(&rcWnd, ptDrop)) {
			*pRect = rcWnd;
			return GetDockSide();	// 移動しない位置だった
		}
	}

	// ドッキング用の矩形を取得する
	DockSideType dockSideType = DockSideType::Float;	// フローティングに仮決め
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
			return dockSideType;	// ドッキング位置だった
		}
	}

	// フローティング用の矩形を取得する
	if (!IsDocking()) {	// フローティング → フローティング
		::OffsetRect(&rcWnd, ptDrop.x - ptDrag.x, ptDrop.y - ptDrag.y);
		*pRect = rcWnd;
	}else {	// ドッキング → フローティング
		int cx, cy;
		RECT rcFloat;
		rcFloat.left = 0;
		rcFloat.top = 0;
		if (pShareData->common.outline.bRememberOutlineWindowPos
				&& pShareData->common.outline.widthOutlineWindow	// 初期値だと 0 になっている
				&& pShareData->common.outline.heightOutlineWindow	// 初期値だと 0 になっている
		) {
			// 記憶しているサイズ
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
				// 言語切り替えでリソースがアンロードされていないか確認するためインスタンスを記憶する
				lastRcInstance = hInstance2;
			}
			// デフォルトのサイズ（ダイアログテンプレートで決まるサイズ）
			rcFloat.right = pDlgTemplate->cx;
			rcFloat.bottom = pDlgTemplate->cy;
			::MapDialogRect(GetHwnd(), &rcFloat);
			rcFloat.right += ::GetSystemMetrics(SM_CXDLGFRAME) * 2;	// ※ Create 時のスタイル変更でサイズ変更不可からサイズ変更可能にしている
			rcFloat.bottom += ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYDLGFRAME) * 2;
		}
		cy = ::GetSystemMetrics(SM_CYCAPTION);
		::OffsetRect(&rcFloat, ptDrop.x - cy * 2, ptDrop.y - cy / 2);
		*pRect = rcFloat;
	}

	return DockSideType::Float;	// フローティング位置だった
}

/** タイトルバーのドラッグ＆ドロップでドッキング配置を変更する
	@date 2010.06.17 ryoji 新規作成
*/
BOOL DlgFuncList::Track(POINT ptDrag)
{
	if (::GetCapture()) {
		return FALSE;
	}

	// 画面にゴミが残らないように
	struct LockWindowUpdate {
		LockWindowUpdate() { ::LockWindowUpdate(::GetDesktopWindow()); }
		~LockWindowUpdate() { ::LockWindowUpdate(NULL); }
	} lockWindowUpdate;

	const SIZE sizeFull = {8, 8};	// フローティング配置用の枠線の太さ
	const SIZE sizeHalf = {4, 4};	// ドッキング配置用の枠線の太さ
	const SIZE sizeClear = {0, 0};	// 枠線描画しない

	POINT pt;
	RECT rc;
	RECT rcDragLast;
	SIZE sizeLast = sizeClear;
	bool bDragging = false;	// まだ本格開始しない
	int cxDragSm = ::GetSystemMetrics(SM_CXDRAG);
	int cyDragSm = ::GetSystemMetrics(SM_CYDRAG);

	::SetCapture(GetHwnd());	// キャプチャ開始

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
				// 押した位置からいくらか動いてからドラッグ開始にする
				if (abs(pt.x - ptDrag.x) >= cxDragSm || abs(pt.y - ptDrag.y) >= cyDragSm) {
					bDragging = bStart = true;	// ここから開始
				}
			}
			if (bDragging) {	// ドラッグ中
				// ドロップ先矩形を描画する
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
				// ドッキング配置を変更する
				DockSideType dockSideType = GetDropRect(ptDrag, pt, &rc, GetKeyState_Control());
				Graphics::DrawDropRect(NULL, sizeClear, &rcDragLast, sizeLast);

				bool bType = (ProfDockSet() != 0);
				if (bType) {
					DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type);
				}
				ProfDockDisp() = GetHwnd()? TRUE: FALSE;
				ProfDockSide() = dockSideType;	// 新しいドッキングモードを適用
				switch (dockSideType) {
				case DockSideType::Left:	ProfDockLeft() = rc.right - rc.left;	break;
				case DockSideType::Top:		ProfDockTop() = rc.bottom - rc.top;		break;
				case DockSideType::Right:	ProfDockRight() = rc.right - rc.left;	break;
				case DockSideType::Bottom:	ProfDockBottom() = rc.bottom - rc.top;	break;
				}
				if (bType) {
					SetTypeConfig(TypeConfigNum(nDocType), type);
				}
				ChangeLayout(OUTLINE_LAYOUT_FOREGROUND);	// 自分自身への強制変更
				if (!IsDocking()) {
					::MoveWindow(GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				}
				if (ProfDockSync()) {
					PostOutlineNotifyToAllEditors(
						(WPARAM)0,
						(LPARAM)((EditView*)(this->lParam))->editWnd.GetHwnd()
					);	// 他ウィンドウにドッキング配置変更を通知する
				}
				return TRUE;
			}
			return FALSE;

		case WM_KEYUP:
			if (bDragging) {
				if (msg.wParam == VK_CONTROL) {
					// フローティングを強制するモードを抜ける
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
					// フローティングを強制するモードに入る
					::GetCursorPos(&pt);
					GetDropRect(ptDrag, pt, &rc, true);
					Graphics::DrawDropRect(&rc, sizeFull, &rcDragLast, sizeLast);
					sizeLast = sizeFull;
					rcDragLast = rc;
				}
			}
			if (msg.wParam == VK_ESCAPE) {
				// キャンセル
				::ReleaseCapture();
				if (bDragging) {
					Graphics::DrawDropRect(NULL, sizeClear, &rcDragLast, sizeLast);
				}
				return FALSE;
			}
			break;

		case WM_RBUTTONDOWN:
			// キャンセル
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
		// 各フォルダのプロジェクトファイル読み込み
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
		// デフォルトプロジェクトファイル読み込み
		bool bReadIni = false;
		if (pFileTree->szProjectIni[0] != _T('\0')) {
			DataProfile profile;
			profile.SetReadingMode();
			const TCHAR* pszIniFileName;
			TCHAR szDir[_MAX_PATH * 2];
			if (_IS_REL_PATH( pFileTree->szProjectIni )) {
				// sakura.iniからの相対パス
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
			// 共通設定orタイプ別設定から読み込み
			//fileTreeSetting = *pFileTree;
			data.items.resize( pFileTree->nItemCount );
			for (int i=0; i<pFileTree->nItemCount; ++i) {
				data.items[i] = pFileTree->items[i];
			}
		}
	}
}

