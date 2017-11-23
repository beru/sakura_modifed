/*!	@file
	@brief タグジャンプリストダイアログボックス
*/

#include "StdAfx.h"
#include "dlg/DlgTagJumpList.h"
#include "SortedTagJumpList.h"
#include "func/Funccode.h"
#include "env/DllSharedData.h"
#include "util/container.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


const DWORD p_helpids[] = {
	IDC_LIST_TAGJUMP,		HIDC_LIST_TAGJUMPLIST,			// ファイル
	IDOK,					HIDC_TAGJUMPLIST_IDOK,			// OK
	IDCANCEL,				HIDC_TAGJUMPLIST_IDCANCEL,		// キャンセル
	IDC_BUTTON_HELP,		HIDC_BUTTON_TAGJUMPLIST_HELP,	// ヘルプ
	IDC_KEYWORD,			HDIC_TAGJUMPLIST_KEYWORD,		// キーワード
	IDC_CHECK_ICASE,		HIDC_CHECK_ICASE,
	IDC_CHECK_ANYWHERE,		HIDC_CHECK_ANYWHERE,
	IDC_BUTTON_NEXTTAG,		HIDC_BUTTON_NEXTTAG,
	IDC_BUTTON_PREVTAG,		HIDC_BUTTON_PREVTAG,
//	IDC_STATIC,				-1,
	0, 0
};

static const AnchorListItem anchorList[] = {
	{IDC_STATIC_BASEDIR,	AnchorStyle::Bottom},
	{IDC_STATIC_KEYWORD,	AnchorStyle::Bottom},
	{IDC_KEYWORD,			AnchorStyle::Bottom},
	{IDC_LIST_TAGJUMP,		AnchorStyle::All},
	{IDC_BUTTON_PREVTAG,	AnchorStyle::Bottom},
	{IDC_BUTTON_NEXTTAG,	AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,		AnchorStyle::Bottom},
	{IDOK,					AnchorStyle::Bottom},
	{IDCANCEL,				AnchorStyle::Bottom},
	{IDC_CHECK_ICASE,		AnchorStyle::Bottom},
	{IDC_CHECK_ANYWHERE,	AnchorStyle::Bottom},
};


// タグファイルのフォーマット
#define TAG_FORMAT_2_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%s\t%s"
#define TAG_FORMAT_1_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d"
#define TAG_FILE_INFO_A      "%[^\t\r\n]\t%[^\t\r\n]\t%[^\t\r\n]"
// #define TAG_FORMAT_E_FILE_A  "%[^\t\r\n,],%d"
// #define TAG_FORMAT_E_NAME_A  "%[^\x7f\r\n]\x7f%[^\x7ff\r\n\x01]\x01%d,%d"

// キーワードを入力して該当する情報を表示するまでの時間(ミリ秒)
#define TAGJUMP_TIMER_DELAY 700
#define TAGJUMP_TIMER_DELAY_SHORT 50


/*
	ctags.exe が出力する、拡張子と対応する種類
*/
static const TCHAR* p_extentions[] = {
	/*asm*/			_T("asm,s"),								_T("d=define,l=label,m=macro,t=type"),
	/*asp*/			_T("asp,asa"),								_T("f=function,s=sub"),
	/*awk*/			_T("awk,gawk,mawk"),						_T("f=function"),
	/*beta*/		_T("bet"),									_T("f=fragment,p=pattern,s=slot,v=virtual"),
	/*c*/			_T("c,h"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*c++*/			_T("c++,cc,cp,cpp,cxx,h++,hh,hp,hpp,hxx"),	_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*java*/		_T("java"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*vera*/		_T("vr,vri,vrh"),							_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*cobol*/		_T("cbl,cob"),								_T("d=data,f=file,g=group,p=paragraph,P=program,s=section"),
	/*eiffel*/		_T("e"),									_T("c=class,f=feature,l=local"),
	/*fortran*/		_T("f,for,ftn,f77,f90,f95"),				_T("b=block data,c=common,e=entry,f=function,i=interface,k=component,l=label,L=local,m=module,n=namelist,p=program,s=subroutine,t=type,v=variable"),
	/*lisp*/		_T("cl,clisp,el,l,lisp,lsp,ml"),			_T("f=function"),
	/*lua*/			_T("lua"),									_T("f=function"),
	/*makefile*/	_T("mak"),									_T("m=macro"),
	/*pascal*/		_T("p,pas"),								_T("f=function,p=procedure"),
	/*perl*/		_T("pl,pm,perl"),							_T("s=subroutine,p=package"),
	/*php*/			_T("php,php3,phtml"),						_T("c=class,f=function"),
	/*python*/		_T("py,python"),							_T("c=class,f=function,m=member"),
	/*rexx*/		_T("cmd,rexx,rx"),							_T("s=subroutine"),
	/*ruby*/		_T("rb"),									_T("c=class,f=method,F=singleton method,m=mixin"),
	/*scheme*/		_T("sch,scheme,scm,sm"),					_T("f=function,s=set"),
	/*sh*/			_T("sh,bsh,bash,ksh,zsh"),					_T("f=function"),
	/*slang*/		_T("sl"),									_T("f=function,n=namespace"),
	/*sql*/			_T("sql"),									_T("c=cursor,d=prototype,f=function,F=field,l=local,P=package,p=procedure,r=record,s=subtype,t=table,T=trigger,v=variable"),
	/*tcl*/			_T("tcl,tk,wish,itcl"),						_T("p=procedure,c=class,f=method"),
	/*verilog*/		_T("v"),									_T("f=function,m=module,P=parameter,p=port,r=reg,t=task,v=variable,w=wire"),
	/*vim*/			_T("vim"),									_T("f=function,v=variable"),
	/*yacc*/		_T("y"),									_T("l=label"),
//	/*vb*/			_T("bas,cls,ctl,dob,dsr,frm,pag"),			_T("a=attribute,c=class,f=function,l=label,s=procedure,v=variable"),
					NULL,									NULL
};

inline bool DlgTagJumpList::IsDirectTagJump() {
	return bDirectTagJump;
}

inline void DlgTagJumpList::ClearPrevFindInfo() {
	psFindPrev->nMatchAll = -1;
	psFind0Match->nDepth  = -1;
	psFind0Match->nMatchAll = 0;
}


DlgTagJumpList::DlgTagJumpList(bool bDirectTagJump)
	:
	Dialog(true),
	bDirectTagJump(bDirectTagJump),
	nIndex(-1),
	pszFileName(NULL),
	pszKeyword(NULL),
	nLoop(-1),
	pList(NULL),
	nTimerId(0),
	bTagJumpICase(FALSE),
	bTagJumpAnyWhere(FALSE),
	nTop(0),
	bNextItem(false),
	psFindPrev(NULL),
	psFind0Match(NULL),
	strOldKeyword(L"")
{
	// サイズ変更時に位置を制御するコントロール数
	assert(_countof(anchorList) == _countof(rcItems));

	pList = new SortedTagJumpList(50);
	psFindPrev = new TagFindState();
	psFind0Match = new TagFindState();
	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
	ClearPrevFindInfo();
}

DlgTagJumpList::~DlgTagJumpList()
{
	Empty();

	if (pszFileName) {
		free(pszFileName);
	}
	pszFileName = NULL;
	if (pszKeyword) {
		free(pszKeyword);
	}
	pszKeyword = NULL;

	StopTimer();
	SAFE_DELETE(pList);
	SAFE_DELETE(psFindPrev);
	SAFE_DELETE(psFind0Match);
}

/*!
	タイマー停止
*/
void DlgTagJumpList::StopTimer(void)
{
	if (nTimerId != 0) {
		::KillTimer(GetHwnd(), nTimerId);
		nTimerId = 0;
	}
}

/*!
	タイマー開始
	
	キーワード指定時，一定期間文字入力がなければリストを更新するため
	「一定期間」を計るタイマーが必要
*/
void DlgTagJumpList::StartTimer(int nDelay = TAGJUMP_TIMER_DELAY)
{
	StopTimer();
	nTimerId = ::SetTimer(GetHwnd(), 12345, nDelay, NULL);
}

/*!
	リストのクリア
*/
void DlgTagJumpList::Empty(void)
{
	nIndex = -1;
	pList->Empty();
}

/*
	モーダルダイアログの表示

	@param[in] lParam 0=ダイレクトタグジャンプ, 1=キーワードを指定してタグジャンプ
*/
INT_PTR DlgTagJumpList::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
	)
{
	INT_PTR ret = Dialog::DoModal(hInstance, hwndParent, IDD_TAGJUMPLIST, lParam);
	StopTimer();
	return ret;
}

// ダイアログデータの設定
void DlgTagJumpList::SetData(void)
{
	if (IsDirectTagJump()) {
		bTagJumpICase = false;
		CheckButton(IDC_CHECK_ICASE, false);
		bTagJumpAnyWhere = false;
		CheckButton(IDC_CHECK_ANYWHERE, false);
		bTagJumpExactMatch = true;

		if (pszKeyword) {
			SetItemText(IDC_KEYWORD, pszKeyword);
		}
	}else {
		// From Here 2005.04.03 MIK 設定値の読み込み
		HWND hwndKey;
		hwndKey = GetItemHwnd(IDC_KEYWORD);

		bTagJumpICase = pShareData->tagJump.bTagJumpICase;
		CheckButton(IDC_CHECK_ICASE, bTagJumpICase);
		bTagJumpAnyWhere = pShareData->tagJump.bTagJumpAnyWhere;
		CheckButton(IDC_CHECK_ANYWHERE, bTagJumpAnyWhere);
		bTagJumpExactMatch = FALSE;
		Combo_LimitText(hwndKey, _MAX_PATH-1);
		RecentTagJumpKeyword cRecentTagJump;
		for (size_t i=0; i<cRecentTagJump.GetItemCount(); ++i) {
			Combo_AddString(hwndKey, cRecentTagJump.GetItemText(i));
		}
		if (pszKeyword) {
			SetItemText(IDC_KEYWORD, pszKeyword);
		}else if (cRecentTagJump.GetItemCount() > 0) {
			Combo_SetCurSel(hwndKey, 0);
		}
		cRecentTagJump.Terminate();
	}
	
	SetTextDir();

	UpdateData(true);

	// 念のため上からUpdateDataの後に移動
	if (!IsDirectTagJump()) {
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT); // 最初は規定時間待たない
	}
}

/*! @brief Jump候補の更新 */
void DlgTagJumpList::UpdateData(bool bInit)
{
	HWND	hwndList;
	LV_ITEM	lvi;
	int		nIndex;
	int		count;

	hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);
	ListView_DeleteAllItems(hwndList);

	count = pList->GetCount();

	TCHAR	tmp[32];
	for (nIndex=0; nIndex<count; ++nIndex) {
		SortedTagJumpList::TagJumpInfo* item;
		item = pList->GetPtr(nIndex);
		if (!item) break;

		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = item->keyword;
		ListView_InsertItem(hwndList, &lvi);

		if (item->baseDirId) {
			auto_sprintf( tmp, _T("(%d)"), item->depth );
		}else {
			auto_sprintf( tmp, _T("%d"), item->depth );
		}
		ListView_SetItemText(hwndList, nIndex, 1, tmp);

		auto_sprintf( tmp, _T("%d"), item->no );
		ListView_SetItemText(hwndList, nIndex, 2, tmp);

		TCHAR* p;
		p = GetNameByType(item->type, item->filename);
		ListView_SetItemText(hwndList, nIndex, 3, p);
		free(p);

		ListView_SetItemText(hwndList, nIndex, 4, item->filename);
		ListView_SetItemText(hwndList, nIndex, 5, item->note);
		ListView_SetItemState(hwndList, nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	const TCHAR* pszMsgText = NULL;

	if (!bInit && pList->GetCount() == 0) {
		pszMsgText = LS(STR_DLGTAGJMP2);
	}
	if (pszMsgText) {
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP1));
		lvi.lParam   = -1;
		ListView_InsertItem(hwndList, &lvi);
//		ListView_SetItemText(hwndList, nIndex, 1, _T(""));
//		ListView_SetItemText(hwndList, nIndex, 2, _T(""));
//		ListView_SetItemText(hwndList, nIndex, 3, _T(""));
		ListView_SetItemText(hwndList, nIndex, 4, const_cast<TCHAR*>(pszMsgText));
//		ListView_SetItemText(hwndList, nIndex, 5, _T(""));
	}

	if (IsDirectTagJump() && nTop == 0 && ! bNextItem) {
		// ダイレクトタグジャンプで、ページングの必要がないときは非表示
		::ShowWindow(GetItemHwnd(IDC_BUTTON_NEXTTAG), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_PREVTAG), SW_HIDE);
	}else {
		EnableItem(IDC_BUTTON_NEXTTAG, bNextItem);
		EnableItem(IDC_BUTTON_PREVTAG, (0 < nTop));
	}

	nIndex = SearchBestTag();
	if (nIndex != -1) {
		ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hwndList, nIndex, FALSE);
	}

	return;
}

/*!	ダイアログデータの取得

	@return TRUE: 正常, FALSE: 入力エラー
*/
int DlgTagJumpList::GetData(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);
	nIndex = ListView_GetNextItem(hwndList, -1, LVIS_SELECTED);

	if (!IsDirectTagJump()) {
		pShareData->tagJump.bTagJumpICase = bTagJumpICase;
		pShareData->tagJump.bTagJumpAnyWhere = bTagJumpAnyWhere;
		// 候補が空でもジャンプで閉じたときは、オプションを保存する
		if (nIndex == -1 || nIndex >= pList->GetCapacity()) {
			return FALSE;
		}
		wchar_t	tmp[MAX_TAG_STRING_LENGTH];
		tmp[0] = 0;
		GetItemText(IDC_KEYWORD, tmp, _countof(tmp));
		SetKeyword(tmp);

		// 設定を保存
		RecentTagJumpKeyword cRecentTagJumpKeyword;
		cRecentTagJumpKeyword.AppendItem(pszKeyword);
		cRecentTagJumpKeyword.Terminate();
	}
	// To Here 2005.04.03 MIK
	if (nIndex == -1 || nIndex >= pList->GetCapacity()) return FALSE;

	return TRUE;
}

/*!
	階層カラムの追加．キーワード指定欄の追加
*/
BOOL DlgTagJumpList::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;
	BOOL		bRet;

	_SetHwnd(hwndDlg);
	::SetWindowLongPtr(GetHwnd(), DWLP_USER, lParam);

	CreateSizeBox();
	Dialog::OnSize();
	
	::GetWindowRect(hwndDlg, &rc);
	ptDefaultSize.x = rc.right - rc.left;
	ptDefaultSize.y = rc.bottom - rc.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().common.others.rcTagJumpDialog;
	if (0
		|| rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		xPos = rcDialog.left;
		yPos = rcDialog.top;
		nWidth = rcDialog.right - rcDialog.left;
		nHeight = rcDialog.bottom - rcDialog.top;
	}

	// ウィンドウのリサイズ
	SetDialogPosSize();

	// リストビューの表示位置を取得する。
	hwndList = ::GetDlgItem(hwndDlg, IDC_LIST_TAGJUMP);
	//ListView_DeleteAllItems(hwndList);
	rc.left = rc.top = rc.right = rc.bottom = 0;
	::GetWindowRect(hwndList, &rc);
	
	int nWidth = (rc.right - rc.left) - ::GetSystemMetrics(SM_CXHSCROLL) - TextWidthCalc::WIDTH_MARGIN_SCROLLBER;

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 20 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST1));
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_CENTER;
	col.cx       = nWidth * 7 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST2));
	col.iSubItem = 1;
	ListView_InsertColumn(hwndList, 1, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_RIGHT;
	col.cx       = nWidth * 8 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST3));
	col.iSubItem = 2;
	ListView_InsertColumn(hwndList, 2, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 9 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST4));
	col.iSubItem = 3;
	ListView_InsertColumn(hwndList, 3, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 35 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST5));
	col.iSubItem = 4;
	ListView_InsertColumn(hwndList, 4, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 21 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST6));
	col.iSubItem = 5;
	ListView_InsertColumn(hwndList, 5, &col);

	// 行選択
	lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	// ダイレクトタブジャンプの時は、キーワードを非表示
	HWND hwndKey = GetItemHwnd(IDC_KEYWORD);
	int nShowFlag = (IsDirectTagJump() ? SW_HIDE : SW_SHOW);
	::ShowWindow(GetItemHwnd(IDC_STATIC_KEYWORD), nShowFlag);
	::ShowWindow(hwndKey, nShowFlag);
	::ShowWindow(GetItemHwnd(IDC_CHECK_ICASE), nShowFlag);
	::ShowWindow(GetItemHwnd(IDC_CHECK_ANYWHERE), nShowFlag);
	if (IsDirectTagJump()) {
		// ダイレクトタグジャンプ
		bRet = TRUE;
	}else {
		// キーワード指定
		::SetFocus(hwndKey);
		bRet = FALSE;	// for set focus
	}

	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentKeyword;
	SetComboBoxDeleter(hwndKey, &comboDel);

	// 基底クラスメンバ
	Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
	
	return bRet;
}

BOOL DlgTagJumpList::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGJUMP_LIST));
		return TRUE;

	case IDOK:			// 左右に表示
		StopTimer();
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	case IDCANCEL:
		StopTimer();
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;

	// 検索条件設定
	case IDC_CHECK_ICASE:
		bTagJumpICase = IsButtonChecked(IDC_CHECK_ICASE);
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT);
		return TRUE;

	case IDC_CHECK_ANYWHERE:
		bTagJumpAnyWhere = IsButtonChecked(IDC_CHECK_ANYWHERE);
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT);
		return TRUE;

	case IDC_BUTTON_NEXTTAG:
		nTop += pList->GetCapacity();
		StopTimer();
		FindNext(false);
		return TRUE;
	case IDC_BUTTON_PREVTAG:
		nTop = t_max(0, nTop - pList->GetCapacity());
		StopTimer();
		FindNext(false);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


INT_PTR DlgTagJumpList::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);

	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}


BOOL DlgTagJumpList::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().common.others.rcTagJumpDialog);

	RECT  rc;
	POINT ptNew;
	GetWindowRect(&rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (size_t i=0 ; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), ptDefaultSize, ptNew, rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}


BOOL DlgTagJumpList::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().common.others.rcTagJumpDialog);

	return Dialog::OnMove(wParam, lParam);
}


BOOL DlgTagJumpList::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = ptDefaultSize.y*3;
	return 0;
}


BOOL DlgTagJumpList::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);

	// 候補一覧リストボックス
	if (hwndList == pNMHDR->hwndFrom) {
		switch (pNMHDR->code) {
		case NM_DBLCLK:
			StopTimer();
			::EndDialog(GetHwnd(), GetData());
			return TRUE;
		}
	}

	// 基底クラスメンバ
	return Dialog::OnNotify(wParam, lParam);
}

/*!
	タイマー経過

	タイマーを停止し，候補リストを更新する
*/
BOOL DlgTagJumpList::OnTimer(WPARAM wParam)
{
	StopTimer();

	FindNext(true);

	return TRUE;
}

/*!
	タイマー経過

	タイマーを開始し，候補リストを更新する準備をする
*/
BOOL DlgTagJumpList::OnCbnEditChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// 基底クラスメンバ
	return Dialog::OnCbnEditChange(hwndCtl, wID);
}

BOOL DlgTagJumpList::OnCbnSelChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// 基底クラスメンバ
	return Dialog::OnCbnSelChange(hwndCtl, wID);
}

#if 0
BOOL DlgTagJumpList::OnEnChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// 基底クラスメンバ
	return Dialog::OnEnChange(hwndCtl, wID);
}
#endif

LPVOID DlgTagJumpList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

#if 0
bool DlgTagJumpList::AddParamA(
	const char* s0,
	const char* s1,
	int n2,
	const char* s3,
	const char* s4,
	int depth,
	int fileBase
	)
{
	if (nIndex == -1) nIndex = 0;	// 規定値

	ClearPrevFindInfo();
	bNextItem = false;
	pList->AddParamA(s0, s1, n2, s3[0], s4, depth, fileBase);

	return true;
}
#endif

bool DlgTagJumpList::GetSelectedFullPathAndLine(
	TCHAR *fullPath,
	int count,
	int* lineNum,
	int* depth
	)
{
	if (pList->GetCount() != 1) {
		if (nIndex == -1 || nIndex >= pList->GetCount()) {
			return false;
		}
	}else {
		nIndex = 0;
	}

	return GetFullPathAndLine( nIndex, fullPath, count, lineNum, depth );
}

/*
	@param lineNum [out] オプション
	@param depth [out] オプション
*/
bool DlgTagJumpList::GetFullPathAndLine(
	int index,
	TCHAR* fullPath,
	int count,
	int* lineNum,
	int* depth
	)
{
	TCHAR path[1024];
	TCHAR fileName[1024];
	TCHAR dirFileName[1024];
	int tempDepth = 0;
	SplitPath_FolderAndFile(GetFilePath(), path, NULL);
	AddLastYenFromDirectoryPath(path);
	
	pList->GetParam(index, NULL, fileName, lineNum, NULL, NULL, &tempDepth, dirFileName);
	if (depth) {
		*depth = tempDepth;
	}
	const TCHAR* fileNamePath;
	// ファイル名、ディレクトリ指定、基準ファイルパス、の順に適用。途中でフルパスならそのまま。
	if (dirFileName[0]) {
		AddLastYenFromDirectoryPath(dirFileName);
		const TCHAR	*p = fileName;
		if (p[0] == _T('\\')) {
			if (p[1] == _T('\\')) {
				auto_strcpy(dirFileName, p);
			}else {
				auto_strcpy(dirFileName, p);
			}
		}else if (_istalpha(p[0]) && p[1] == _T(':')) {
			auto_strcpy(dirFileName, p);
		}else {
			// 相対パス：連結する
			auto_strcat(dirFileName, p);
		}
		fileNamePath = dirFileName;
	}else {
		fileNamePath = fileName;
	}
	bool ret = GetFullPathFromDepth(fullPath, count, path, fileNamePath, tempDepth) != NULL;
	if (ret) {
		DEBUG_TRACE(_T("jump to: %ts\n"), static_cast<const TCHAR*>(fullPath));
	}else {
		DEBUG_TRACE(_T("jump to: error\n"));
	}
	return ret;
}

/*!
	@return 「.ext」形式のタイプ情報。 freeすること
*/
TCHAR* DlgTagJumpList::GetNameByType(const TCHAR type, const TCHAR* name)
{
	const TCHAR* p;
	TCHAR*	token;
	TCHAR	tmp[MAX_TAG_STRING_LENGTH];

	p = _tcsrchr(name, _T('.'));
	if (!p) p = _T(".c");	// 見つからないときは ".c" と想定する。
	++p;

	for (int i=0; p_extentions[i]; i+=2) {
		_tcscpy(tmp, p_extentions[i]);
		token = _tcstok(tmp, _T(","));
		while (token) {
			if (_tcsicmp(p, token) == 0) {
				_tcscpy(tmp, p_extentions[i + 1]);
				token = _tcstok(tmp, _T(","));
				while (token) {
					if (token[0] == type) {
						return _tcsdup(&token[2]);
					}
					token = _tcstok(NULL, _T(","));
				}
				return _tcsdup(_T(""));
			}
			token = _tcstok(NULL, _T(","));
		}
	}
	return _tcsdup(_T(""));
}

/*!
	基準ファイル名を設定
*/
void DlgTagJumpList::SetFileName(const TCHAR* pszFileName)
{
	assert_warning(pszFileName);
	if (!pszFileName) {
		return;
	}

	if (this->pszFileName) {
		free(this->pszFileName);
	}

	this->pszFileName = _tcsdup(pszFileName);
	
	nLoop = CalcMaxUpDirectory(pszFileName);
}

/*!
	検索キーワードの設定

*/
void DlgTagJumpList::SetKeyword(const wchar_t* pszKeyword)
{
	if (!pszKeyword) {
		return;
	}

	if (this->pszKeyword) {
		free(this->pszKeyword);
	}

	this->pszKeyword = wcsdup(pszKeyword);
}

struct TagPathInfo {
	TCHAR	szFileNameDst[_MAX_PATH*4];
	TCHAR	szDriveSrc[_MAX_DRIVE*2];
	TCHAR	szDriveDst[_MAX_DRIVE*2];
	TCHAR	szPathSrc[_MAX_PATH*4];
	TCHAR	szPathDst[_MAX_PATH*4];
	TCHAR	szFileSrc[_MAX_PATH*4];
	TCHAR	szFileDst[_MAX_PATH*4];
	TCHAR	szExtSrc[_MAX_EXT*2];
	TCHAR	szExtDst[_MAX_EXT*2];
	size_t	nDriveSrc;
	size_t	nDriveDst;
	size_t	nPathSrc;
	size_t	nPathDst;
	size_t	nFileSrc;
	size_t	nFileDst;
	size_t	nExtSrc;
	size_t	nExtDst;
};

/*!
	得られた候補から最も期待に近いと思われるものを
	選び出す．(初期選択位置決定のため)

	@return 選択されたアイテムのindex

*/
int DlgTagJumpList::SearchBestTag(void)
{
	if (pList->GetCount() <= 0) {
		return -1;	// 選べません。
	}
	if (!pszFileName) {
		return 0;
	}

	auto lpPathInfo = std::make_unique<TagPathInfo>();
	int nMatch1 = -1;
	int nMatch2 = -1;
	int nMatch3 = -1;
	int nMatch4 = -1;
	int nMatch5 = -1;
	int nMatch6 = -1;
	int nMatch7 = -1;
	int		i;
	int		count;

	lpPathInfo->szDriveSrc[0] = _T('\0');
	lpPathInfo->szPathSrc[0] = _T('\0');
	lpPathInfo->szFileSrc[0] = _T('\0');
	lpPathInfo->szExtSrc[0] = _T('\0');
	_tsplitpath( pszFileName, lpPathInfo->szDriveSrc, lpPathInfo->szPathSrc, lpPathInfo->szFileSrc, lpPathInfo->szExtSrc );
	lpPathInfo->nDriveSrc = _tcslen(lpPathInfo->szDriveSrc);
	lpPathInfo->nPathSrc = _tcslen(lpPathInfo->szPathSrc);
	lpPathInfo->nFileSrc = _tcslen(lpPathInfo->szFileSrc);
	lpPathInfo->nExtSrc = _tcslen(lpPathInfo->szExtSrc);

	count = pList->GetCount();

	for (i=0; i<count; ++i) {
		// タグのファイル名部分をフルパスにする
		lpPathInfo->szFileNameDst[0] = _T('\0');
		{
			TCHAR szPath[_MAX_PATH];
			GetFullPathAndLine( i, szPath, _countof(szPath), NULL, NULL );
			if (!GetLongFileName(szPath, lpPathInfo->szFileNameDst)) {
				_tcscpy( lpPathInfo->szFileNameDst, szPath );
			}
		}

		lpPathInfo->szDriveDst[0] = _T('\0');
		lpPathInfo->szPathDst[0] = _T('\0');
		lpPathInfo->szFileDst[0] = _T('\0');
		lpPathInfo->szExtDst[0] = _T('\0');
		_tsplitpath( lpPathInfo->szFileNameDst, lpPathInfo->szDriveDst, lpPathInfo->szPathDst, lpPathInfo->szFileDst, lpPathInfo->szExtDst );
		lpPathInfo->nDriveDst = _tcslen(lpPathInfo->szDriveDst);
		lpPathInfo->nPathDst = _tcslen(lpPathInfo->szPathDst);
		lpPathInfo->nFileDst = _tcslen(lpPathInfo->szFileDst);
		lpPathInfo->nExtDst = _tcslen(lpPathInfo->szExtDst);
		
		if (_tcsicmp(pszFileName, lpPathInfo->szFileNameDst) == 0) {
			return i;	//同一ファイルを見つけた
		}

		if ((nMatch1 == -1)
		&& (_tcsicmp(lpPathInfo->szDriveSrc, lpPathInfo->szDriveDst) == 0)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			//ファイル名まで一致
			nMatch1 = i;
		}

		if ((nMatch2 == -1)
		&& (_tcsicmp(lpPathInfo->szDriveSrc, lpPathInfo->szDriveDst) == 0)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		) {
			//パス名まで一致
			nMatch2 = i;
		}

		if ((nMatch5 == -1)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			nMatch5 = i;
		}

		if ((nMatch6 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			if ((lpPathInfo->nPathSrc >= lpPathInfo->nPathDst)
			&& (_tcsicmp(&lpPathInfo->szPathSrc[lpPathInfo->nPathSrc - lpPathInfo->nPathDst], lpPathInfo->szPathDst) == 0)
			) {
				nMatch6 = i;
			}
		}

		if ((nMatch7 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			if ((lpPathInfo->nPathSrc >= lpPathInfo->nPathDst)
			&& (_tcsicmp(&lpPathInfo->szPathSrc[lpPathInfo->nPathSrc - lpPathInfo->nPathDst], lpPathInfo->szPathDst) == 0)
			) {
				nMatch7 = i;
			}
		}

		if ((nMatch3 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			//ファイル名・拡張子が一致
			nMatch3 = i;
		}

		if ((nMatch4 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			//ファイル名が一致
			nMatch4 = i;
		}
	}

	if (nMatch1 != -1) return nMatch1;
	if (nMatch5 != -1) return nMatch5;
	if (nMatch6 != -1) return nMatch6;
	if (nMatch7 != -1) return nMatch7;
	if (nMatch3 != -1) return nMatch3;
	if (nMatch4 != -1) return nMatch4;
	if (nMatch2 != -1) return nMatch2;

	return 0;
}

/*!
	@param bNewFind 新しい検索条件(次・前のときfalse)
*/
void DlgTagJumpList::FindNext(bool bNewFind)
{
	wchar_t	szKey[MAX_TAG_STRING_LENGTH];
	szKey[0] = 0;
	GetItemText(IDC_KEYWORD, szKey, _countof(szKey));
	if (bNewFind) {
		// 前回のキーワードからの絞込検索のときで、tagsをスキップできるときはスキップ
		if (-1 < psFind0Match->nDepth
			&& (bTagJumpAnyWhere == bOldTagJumpAnyWhere || !bTagJumpAnyWhere)
			&& (bTagJumpICase    == bOldTagJumpICase    || !bTagJumpICase)
			&& wcsncmp(strOldKeyword.GetStringPtr(), szKey, strOldKeyword.GetStringLength() == 0)
		) {
			// 元のキーワードで１件もヒットしないtagsがあるので飛ばす
			// 条件は同じか、厳しくなるなら問題ない
		}else {
			ClearPrevFindInfo();
		}
		nTop = 0;
	}
	find_key(szKey);
	UpdateData(false);
}

/*!
	ダイレクトタグジャンプ検索(DoModal前に実行)
*/
int DlgTagJumpList::FindDirectTagJump()
{
	return find_key_core(
		0,	// 0開始
		pszKeyword,
		false, // 部分一致
		true,  // 完全一致
		false, // 大小を区別
		true,  // 自動モード
		pShareData->common.search.nTagJumpMode
	);
}

void DlgTagJumpList::find_key(const wchar_t* keyword)
{
	SetItemText(IDC_STATIC_KEYWORD, LS(STR_DLGTAGJMP3));
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));

	find_key_core(
		nTop,
		keyword,
		bTagJumpAnyWhere,
		bTagJumpExactMatch,
		bTagJumpICase,
		IsDirectTagJump(),
		IsDirectTagJump() ? (pShareData->common.search.nTagJumpMode) : pShareData->common.search.nTagJumpModeKeyword
	);
	SetItemText(IDC_STATIC_KEYWORD, LS(STR_DLGTAGJMP_LIST1));
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));
}

/*!
	タグファイルからキーワードにマッチするデータを抽出し，m_cListに設定する
*/
int DlgTagJumpList::find_key_core(
	int nTop,
	const wchar_t* keyword,
	bool bTagJumpAnyWhere,		// 部分一致
	bool bTagJumpExactMatch,	// 完全一致
	bool bTagJumpICase,
	bool bTagJumpICaseByTags,	// Tagファイル側のソートに従う
	int  nDefaultNextMode
	)
{
	assert_warning(!(bTagJumpAnyWhere && bTagJumpExactMatch));

	// to_acharは一時バッファで破壊される可能性があるのでコピー
	NativeA memKeyA = NativeA(to_achar(keyword));
	const char* paszKeyword = memKeyA.GetStringPtr();
	size_t	length = memKeyA.GetStringLength();

	Empty();

	strOldKeyword.SetString(keyword);
	bOldTagJumpAnyWhere = bTagJumpAnyWhere;
	bOldTagJumpICase    = bTagJumpICase;
	bNextItem = false;

	if (length == 0) {
		ClearPrevFindInfo();
		return -1;
	}
	// 往復変換してみて一致しなかったら、検索キーには一致しないということにする
	if (wcscmp(to_wchar(paszKeyword), keyword) != 0) {
		ClearPrevFindInfo();
		return -1;
	}
	SortedTagJumpList& list = *pList;
	const int nCap = list.GetCapacity();
	TagFindState state;
	state.nDepth    = 0;
	state.nMatchAll = 0;
	state.nNextMode = nDefaultNextMode;
	state.nLoop     = -1;
	state.bJumpPath = false;	// 親以外のパスの移動先指定
	state.szCurPath[0] = 0;
	
	// 前回の結果から検索対象tagsを絞る
	if (psFindPrev->nMatchAll <= nTop && -1 < psFindPrev->nMatchAll) {
		// 指定ページの検索をスキップ
		state = *psFindPrev;
		DEBUG_TRACE(_T("skip count  d:%d m:%d n:%d\n"), state.nDepth, state.nMatchAll, state.nNextMode);
	}else if (0 <= psFind0Match->nDepth) {
		// depthが浅い順にヒットしなかった分をスキップ
		state = *psFind0Match;
		DEBUG_TRACE(_T("skip 0match d:%d m:%d n:%d\n"), state.nDepth, state.nMatchAll, state.nNextMode);
	}else {
		// 初回or使えないときはクリア
		ClearPrevFindInfo();
		// ファイル名をコピーしたあと、ディレクトリ(最後\)のみにする
		_tcscpy( state.szCurPath, GetFilePath() );
		state.szCurPath[GetFileName() - GetFilePath()] = _T('\0');
		state.nLoop = nLoop;
	}
	
	TCHAR	szTagFile[1024];		// タグファイル
	TCHAR	szNextPath[1024];		// 次検索フォルダ
	char	szLineData[1024];		// 行バッファ
	char	s[4][1024];
	int		n2;
	szNextPath[0] = _T('\0');
	vector_ex<std::tstring> seachDirs;

	// パスのJumpで循環している場合に最大値を規制する
	for (; state.nDepth<=state.nLoop && state.nDepth<(_MAX_PATH/2); state.nDepth++) {
		// 0 次のファイルは検索しない
		// 1 1つでもヒットしたら次は検索しない
		// 2 完全一致のときは1に同じ。 それ以外は3に同じ
		// 3 必ず次も検索
		if (state.nNextMode == 0) break;
		if (state.nNextMode == 1 && 0 < state.nMatchAll) break;
		if (state.nNextMode == 2 && bTagJumpExactMatch && 0 < state.nMatchAll) break; 
		{
			std::tstring curPath = state.szCurPath;
			if (seachDirs.exist(curPath)) {
				// 検索済み =>終了
				break;
			}
			seachDirs.push_back(curPath);
		}

		// タグファイル名を作成する。
		auto_sprintf_s(szTagFile, _T("%ts%ts"), state.szCurPath, TAG_FILENAME_T);
		DEBUG_TRACE(_T("tag: %ts\n"), szTagFile);
		
		// タグファイルを開く。
		FILE* fp = _tfopen(szTagFile, _T("rb"));
		if (fp) {
			DEBUG_TRACE(_T("open tags\n"));
			bool bSorted = true;
			bool bFoldcase = false;
			int  nTagFormat = 2; // 2は1も読めるのでデフォルトは2
			int  nLines = 0;
			int  baseDirId = 0;
			if (state.bJumpPath) {
				baseDirId = list.AddBaseDir(state.szCurPath);
			}
			state.nNextMode = nDefaultNextMode;

			// バッファの後ろから2文字目が\0かどうかで、行末まで読み込んだか確認する
			const int nLINEDATA_LAST_CHAR = _countof(szLineData) - 2;
			szLineData[nLINEDATA_LAST_CHAR] = '\0';
			while (fgets(szLineData, _countof(szLineData), fp)) {
				nLines++;
				int  nRet;
				// fgetsが行すべてを読み込めていない場合の考慮
				if (1
					&& szLineData[nLINEDATA_LAST_CHAR] != '\0'
				    && szLineData[nLINEDATA_LAST_CHAR] != '\n'
				) {
					// 改行コードまでを捨てる
					int ch = fgetc(fp);
					while (ch != '\n' && ch != EOF) {
						ch = fgetc(fp);
					}
				}
				if (nLines == 1 && szLineData[0] == '\x0c') {
					// etagsなので次のファイル
					break;
				}
				if (szLineData[0] == '!') {
					if (strncmp_literal(szLineData + 1, "_TAG_") == 0) {
						s[0][0] = s[1][0] = s[2][0] = 0;
						nRet = sscanf(
							szLineData, 
							TAG_FILE_INFO_A,	// tagsファイル情報
							s[0], s[1], s[2]
						);
						if (nRet < 2) goto next_line;
						const char* pTag = s[0] + 6;
						if (strncmp_literal(pTag , "FILE_FORMAT") == 0) {
							n2 = atoi(s[1]);
							if (1 <= n2 && n2 <= 2) {
								nTagFormat = n2;
							}
						}else if (strncmp_literal(pTag, "FILE_SORTED") == 0) {
							n2 = atoi(s[1]);
							bSorted = (n2 == 1);
							bFoldcase = (n2 == 2);
							if (bTagJumpICaseByTags) {
								bTagJumpICase = bFoldcase;
							}
						}else if (strncmp_literal(pTag, "S_SEARCH_NEXT") == 0) {
							// 独自拡張:次に検索するtagファイルの指定
							if ('0' <= s[1][0] && s[1][0] <= '3') {
								n2 = atoi(s[1]);
								if (0 <= n2 && n2 <= 3) {
									state.nNextMode = n2;
								}
								if (1 <= n2 && s[2][0]) {
									// s[2] == 絶対パス(ディレクトリ)
									TCHAR baseWork[1024];
									CopyDirDir(baseWork, to_tchar(s[2]), state.szCurPath);
									szNextPath[0] = 0;
									if (!GetLongFileName(baseWork, szNextPath)) {
										// エラーなら変換前を適用
										auto_strcpy(szNextPath, baseWork);
									}
								}
							}
						}else if (strncmp_literal(pTag, "S_FILE_BASEDIR") == 0) {
							TCHAR baseWork[1024];
							// 独自拡張:ファイル名の基準ディレクトリ
							if (state.bJumpPath) {
								// パス親読み替え中は、相対パスだった場合に連結が必要
								CopyDirDir(baseWork, to_tchar(s[1]), state.szCurPath);
								baseDirId = list.AddBaseDir(baseWork);
							}else {
								auto_strcpy(baseWork, to_tchar(s[1]));
								AddLastYenFromDirectoryPath(baseWork);
								baseDirId = list.AddBaseDir(baseWork);
							}
						}
					}
					goto next_line;	// コメントならスキップ
				}
				if (szLineData[0] < '!') {
					goto next_line;
				}
				//chop(szLineData);

				s[0][0] = s[1][0] = s[2][0] = s[3][0] = '\0';
				n2 = 0;
				// TAG_FORMAT定数化
				if (nTagFormat == 2) {
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_2_A,	// 拡張tagsフォーマット
						s[0], s[1], &n2, s[2], s[3]
						);
					if (nRet < 3) goto next_line;
					if (n2 <= 0) goto next_line;	// 行番号不正(-excmd=nが指定されてないかも)
				}else {
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_1_A,	// tagsフォーマット
						s[0], s[1], &n2
						);
					if (nRet < 2) goto next_line;
					if (n2 <= 0) goto next_line;
				}

				int cmp;
				if (bTagJumpAnyWhere) {
					if (bTagJumpICase) {
						cmp = stristr_j(s[0], paszKeyword) != NULL ? 0 : -1;
					}else {
						cmp = strstr_j(s[0], paszKeyword) != NULL ? 0 : -1;
					}
				}else {
					if (bTagJumpExactMatch) {
						// 完全一致
						if (bTagJumpICase) {
							cmp = auto_stricmp(s[0], paszKeyword);
						}else {
							cmp = auto_strcmp(s[0], paszKeyword);
						}
					}else {
						// 前方一致
						if (bTagJumpICase) {
							cmp = my_strnicmp(s[0], paszKeyword, length);
						}else {
							cmp = strncmp(s[0], paszKeyword, length);
						}
					}
				}
				if (cmp == 0) {
					state.nMatchAll++;
					if (nTop < state.nMatchAll) {
						if (list.GetCount() < nCap) {
							list.AddParamA(s[0], s[1], n2, s[2][0], s[3], state.nDepth, baseDirId);
						}else {
							// 探索打ち切り(次ページでやり直し)
							bNextItem = true;
							break;
						}
					}
				}else if (0 < cmp) {
					// tagsはソートされているので，先頭からのcase sensitiveな
					// 比較結果によって検索の時は処理の打ち切りが可能
					if ((!bTagJumpICase) && bSorted && (!bTagJumpAnyWhere)) break;
					// Foldcase時も打ち切る。ただしtagsとサクラ側のソート順が同じでなければならない
					if (bTagJumpICase  && bFoldcase && (!bTagJumpAnyWhere)) break;
				}
next_line:
				;
				szLineData[nLINEDATA_LAST_CHAR] = '\0';
			}

			// ファイルを閉じる。
			fclose(fp);
			DEBUG_TRACE(_T("close m:%d\n "), state.nMatchAll);
		}
		
		if (szNextPath[0]) {
			state.bJumpPath = true;
			auto_strcpy(state.szCurPath, szNextPath);
			std::tstring path = state.szCurPath;
			path += _T("\\dummy");
			state.nLoop = CalcMaxUpDirectory( path.c_str() );
			state.nDepth = 0;
			szNextPath[0] = 0;
		}else {
//			_tcscat(state.szCurPath, _T("..\\"));
			// カレントパスを1階層上へ。
			DirUp(state.szCurPath);
		}
		
		if (state.nMatchAll != 0 && !bNextItem) {
			// 0 ページめくり用: 打ち切られていないので次のページでは、このtagsの次から検索できる
			// (最後に通過したものを保持)
			*psFindPrev = state;
			++(psFindPrev->nDepth);
			DEBUG_TRACE(_T("FindPrev udpate: d:%d m:%d n:%d L:%d j:%d\n") , psFindPrev->nDepth, psFindPrev->nMatchAll, psFindPrev->nNextMode, psFindPrev->nLoop, (int)psFindPrev->bJumpPath);
		}
		if (state.nMatchAll == 0) {
			// キーワード絞込み用: 次の絞り込み検索では、このtagsの次から検索できる
			// (最後に通過したものを保持)
			*psFind0Match = state;
			++(psFind0Match->nDepth);
			DEBUG_TRACE(_T("Find0Match udpate: d:%d m:%d n:%d L:%d j:%d\n") , psFind0Match->nDepth, psFind0Match->nMatchAll, psFind0Match->nNextMode, psFind0Match->nLoop, (int)psFind0Match->bJumpPath);
		}
		if (bNextItem) {
			break;
		}
	}
	return state.nMatchAll;
}

/*!
	パスからファイル名部分のみを取り出す．(2バイト対応)
*/
const TCHAR* DlgTagJumpList::GetFileName(void)
{
	return GetFileTitlePointer(GetFilePath());
}


void DlgTagJumpList::SetTextDir()
{
	if (GetHwnd()) {
		SetItemText(IDC_STATIC_BASEDIR, _T(""));
		if (GetFileName()) {
			std::tstring strPath = GetFilePath();
			strPath[GetFileName() - GetFilePath()] = _T('\0');
			SetItemText(IDC_STATIC_BASEDIR, strPath.c_str());
		}
	}
}

int DlgTagJumpList::CalcMaxUpDirectory(const TCHAR* p)
{
	int loop = CalcDirectoryDepth(p);
	if (loop <  0) loop =  0;
	if (loop > (_MAX_PATH/2)) loop = (_MAX_PATH/2);	// \A\B\C...のようなとき1フォルダで2文字消費するので...
	return loop;
}

/*!
	@param basePath [in,out] \付ディレクトリパス絶対パス推奨。書き換わるのに注意
	@param fileName [in] 相対・絶対ファイル名パス
	@param depth    [in] fineNameが絶対パスの時無視。1==1つ上のディレクトリ
	@retval pszOutput 成功 「C:\dir1\filename.txt」の形式(..\付加は廃止)
	@retval NULL   失敗
*/
TCHAR* DlgTagJumpList::GetFullPathFromDepth(TCHAR* pszOutput, int count,
	TCHAR* basePath, const TCHAR* fileName, int depth)
{
	DEBUG_TRACE(_T("base  %ts\n"), basePath);
	DEBUG_TRACE(_T("file  %ts\n"), fileName);
	DEBUG_TRACE(_T("depth %d\n"),  depth);
	// 完全パス名を作成する。
	const TCHAR	*p = fileName;
	if (p[0] == _T('\\')) {		// ドライブなし絶対パスか？
		if (p[1] == _T('\\')) {	// ネットワークパスか？
			_tcscpy(pszOutput, p);	// 何も加工しない。
		}else {
			// ドライブ加工したほうがよい？
			_tcscpy(pszOutput, p);	// 何も加工しない。
		}
	}else if (_istalpha(p[0]) && p[1] == _T(':')) {	// 絶対パスか？
		_tcscpy(pszOutput, p);	// 何も加工しない。
	}else {
		for (int i=0; i<depth; ++i) {
			//_tcscat(basePath, _T("..\\"));
			DirUp(basePath);
		}
		if (auto_snprintf_s(pszOutput, count, _T("%ts%ts"), basePath, p) == -1) {
			return NULL;
		}
	}
	return pszOutput;
}

/*!
	ディレクトリとディレクトリを連結する
*/
TCHAR* DlgTagJumpList::CopyDirDir(TCHAR* dest, const TCHAR* target, const TCHAR* base)
{
	if (_IS_REL_PATH(target)) {
		auto_strcpy(dest, base);
		AddLastYenFromDirectoryPath(dest);
		auto_strcat(dest, target);
	}else {
		auto_strcpy(dest, target);
	}
	AddLastYenFromDirectoryPath(dest);
	return dest;
}

/*
	@param dir [in,out] フォルダのパス 
	in == C:\dir\subdir\
	out == C:\dir\
*/
TCHAR* DlgTagJumpList::DirUp(TCHAR* dir)
{
	CutLastYenFromDirectoryPath(dir);
	const TCHAR* p = GetFileTitlePointer(dir); // 最後の\の次の文字を取得 last_index_of('\\') + 1;
	if (0 < p - dir) {
		dir[p - dir] = '\0';
	}
	return dir;
}

