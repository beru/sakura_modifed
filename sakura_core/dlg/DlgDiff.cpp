/*!	@file
	@brief DIFF差分表示ダイアログボックス

	@author MIK
	@date 2002.5.27
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, MIK, genta, じゅうじ
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/DlgDiff.h"
#include "dlg/DlgOpenFile.h"
#include "window/EditWnd.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/fileUtil.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13200
	IDC_BUTTON_DIFF_DST,		HIDC_BUTTON_DIFF_DST,
	IDC_CHECK_DIFF_OPT_BLINE,	HIDC_CHECK_DIFF_OPT_BLINE,
	IDC_CHECK_DIFF_OPT_CASE,	HIDC_CHECK_DIFF_OPT_CASE,
	IDC_CHECK_DIFF_OPT_SPACE,	HIDC_CHECK_DIFF_OPT_SPACE,
	IDC_CHECK_DIFF_OPT_SPCCHG,	HIDC_CHECK_DIFF_OPT_SPCCHG,
	IDC_CHECK_DIFF_OPT_TABSPC,	HIDC_CHECK_DIFF_OPT_TABSPC,
	IDC_EDIT_DIFF_DST,			HIDC_EDIT_DIFF_DST,
	IDC_FRAME_DIFF_FILE12,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE1,		HIDC_RADIO_DIFF_FILE1,
	IDC_RADIO_DIFF_FILE2,		HIDC_RADIO_DIFF_FILE2,
	IDC_FRAME_DIFF_DST,			HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST1,		HIDC_RADIO_DIFF_DST1,
	IDC_RADIO_DIFF_DST2,		HIDC_RADIO_DIFF_DST2,
	IDC_LIST_DIFF_FILES,		HIDC_LIST_DIFF_FILES,
	IDC_STATIC_DIFF_SRC,		HIDC_STATIC_DIFF_SRC,
	IDOK,						HIDC_DIFF_IDOK,
	IDCANCEL,					HIDC_DIFF_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_DIFF_HELP,
	IDC_CHECK_DIFF_EXEC_STATE,	HIDC_CHECK_DIFF_EXEC_STATE,		// DIFF差分が見つからないときにメッセージを表示  2003.05.12 MIK
	IDC_CHECK_NOTIFYNOTFOUND,	HIDC_CHECK_DIFF_NOTIFYNOTFOUND,	// 見つからないときにメッセージを表示	// 2006.10.10 ryoji
	IDC_CHECK_SEARCHALL,		HIDC_CHECK_DIFF_SEARCHALL,		// 先頭（末尾）から再検索する	// 2006.10.10 ryoji
//	IDC_FRAME_SEARCH_MSG,		HIDC_FRAME_DIFF_SEARCH_MSG,
//	IDC_STATIC,					-1,
	0, 0
};

static const AnchorListItem anchorList[] = {
	{IDC_BUTTON_DIFF_DST,       AnchorStyle::Right},
	{IDC_CHECK_DIFF_OPT_BLINE,  AnchorStyle::Bottom},
	{IDC_CHECK_DIFF_OPT_CASE,   AnchorStyle::Bottom},
	{IDC_CHECK_DIFF_OPT_SPACE,  AnchorStyle::Bottom},
	{IDC_CHECK_DIFF_OPT_SPCCHG, AnchorStyle::Bottom},
	{IDC_CHECK_DIFF_OPT_TABSPC, AnchorStyle::Bottom},
	{IDC_EDIT_DIFF_DST,         AnchorStyle::LeftRight},
	{IDC_FRAME_DIFF_FILE12,     AnchorStyle::Bottom},
	{IDC_RADIO_DIFF_FILE1,      AnchorStyle::Bottom},
	{IDC_RADIO_DIFF_FILE2,      AnchorStyle::Bottom},
	{IDC_FRAME_DIFF_DST,        AnchorStyle::All},
	{IDC_RADIO_DIFF_DST1,		AnchorStyle::TopLeft},
	{IDC_RADIO_DIFF_DST2,		AnchorStyle::TopLeft},
	{IDC_LIST_DIFF_FILES,       AnchorStyle::All},
	{IDC_STATIC_DIFF_SRC,       AnchorStyle::LeftRight},
	{IDOK,                      AnchorStyle::Bottom},
	{IDCANCEL,                  AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,           AnchorStyle::Bottom},
	{IDC_CHECK_DIFF_EXEC_STATE, AnchorStyle::Bottom},
	{IDC_CHECK_NOTIFYNOTFOUND,  AnchorStyle::Bottom},
	{IDC_CHECK_SEARCHALL,       AnchorStyle::Bottom},
	{IDC_FRAME_SEARCH_MSG,      AnchorStyle::Bottom},
};


DlgDiff::DlgDiff()
	: Dialog(true)
	, m_nIndexSave( 0 )
{
	// サイズ変更時に位置を制御するコントロール数
	assert(_countof(anchorList) == _countof(m_rcItems));

	m_nDiffFlgOpt    = 0;
	m_bIsModifiedDst = false;
	m_nCodeTypeDst = CODE_ERROR;
	m_bBomDst = false;
	m_hWnd_Dst       = NULL;
	m_ptDefaultSize.x = -1;
	m_ptDefaultSize.y = -1;
	return;
}

// モーダルダイアログの表示
int DlgDiff::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	LPARAM				lParam,
	const TCHAR*		pszPath		// 自ファイル
	)
{
	_tcscpy(m_szFile1, pszPath);
	return (int)Dialog::DoModal(hInstance, hwndParent, IDD_DIFF, lParam);
}

BOOL DlgDiff::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DIFF_DIALOG));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDC_BUTTON_DIFF_DST:	// 参照
		{
			DlgOpenFile dlgOpenFile;
			TCHAR szPath[_MAX_PATH];
			_tcscpy(szPath, m_szFile2);
			// ファイルオープンダイアログの初期化
			dlgOpenFile.Create(
				m_hInstance,
				GetHwnd(),
				_T("*.*"),
				m_szFile1 //m_szFile2
			);
			if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
				_tcscpy(m_szFile2, szPath);
				SetItemText(IDC_EDIT_DIFF_DST, m_szFile2);
				// 外部ファイルを選択状態に
				CheckButton(IDC_RADIO_DIFF_DST1, true);
				CheckButton(IDC_RADIO_DIFF_DST2, false);
				List_SetCurSel(GetItemHwnd(IDC_LIST_DIFF_FILES), -1);
			}
		}
		return TRUE;

	case IDOK:			// 左右に表示
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), GetData());
		return TRUE;

	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;

	case IDC_RADIO_DIFF_DST1:
		CheckButton(IDC_RADIO_DIFF_DST2, false);
		//EnableItem(IDC_EDIT_DIFF_DST), true);
		//EnableItem(IDC_BUTTON_DIFF_DST), true);
		//EnableItem(IDC_LIST_DIFF_FILES), false);
		// Feb. 28, 2004 genta 選択解除前に前回の位置を記憶
		{
			int n = List_GetCurSel(GetItemHwnd(IDC_LIST_DIFF_FILES));
			if (n != LB_ERR) {
				m_nIndexSave = n;
			}
		}
		List_SetCurSel(GetItemHwnd(IDC_LIST_DIFF_FILES), -1);
		return TRUE;

	case IDC_RADIO_DIFF_DST2:
		CheckButton(IDC_RADIO_DIFF_DST1, false);
		//EnableItem(IDC_EDIT_DIFF_DST), false);
		//EnableItem(IDC_BUTTON_DIFF_DST), false);
		//EnableItem(IDC_LIST_DIFF_FILES), true);
		{
			// Aug. 9, 2003 genta
			// ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetItemHwnd(IDC_LIST_DIFF_FILES);
			if (List_GetCurSel(hwndList) == LB_ERR) {
				List_SetCurSel(hwndList, m_nIndexSave);
			}
		}
		return TRUE;

	case IDC_RADIO_DIFF_FILE1:
		CheckButton(IDC_RADIO_DIFF_FILE2, false);
		return TRUE;

	case IDC_RADIO_DIFF_FILE2:
		CheckButton(IDC_RADIO_DIFF_FILE1, false);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

// ダイアログデータの設定
void DlgDiff::SetData(void)
{
	// オプション
	m_nDiffFlgOpt = m_pShareData->m_nDiffFlgOpt;
	if (m_nDiffFlgOpt & 0x0001) CheckButton(IDC_CHECK_DIFF_OPT_CASE,   true);
	if (m_nDiffFlgOpt & 0x0002) CheckButton(IDC_CHECK_DIFF_OPT_SPACE,  true);
	if (m_nDiffFlgOpt & 0x0004) CheckButton(IDC_CHECK_DIFF_OPT_SPCCHG, true);
	if (m_nDiffFlgOpt & 0x0008) CheckButton(IDC_CHECK_DIFF_OPT_BLINE,  true);
	if (m_nDiffFlgOpt & 0x0010) CheckButton(IDC_CHECK_DIFF_OPT_TABSPC, true);

	// 新旧ファイル
	if (m_nDiffFlgOpt & 0x0020) {
		CheckButton(IDC_RADIO_DIFF_FILE1, false);
		CheckButton(IDC_RADIO_DIFF_FILE2, true);
	}else {
		CheckButton(IDC_RADIO_DIFF_FILE1, true);
		CheckButton(IDC_RADIO_DIFF_FILE2, false);
	}
	//EnableItem(IDC_FRAME_DIFF_FILE12), false);
	//EnableItem(IDC_RADIO_DIFF_FILE1), false);
	//EnableItem(IDC_RADIO_DIFF_FILE2), false);

	// DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if (m_nDiffFlgOpt & 0x0040) CheckButton(IDC_CHECK_DIFF_EXEC_STATE, true);

	// 見つからないときメッセージを表示
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, m_pShareData->m_common.search.bNotifyNotFound);
	
	// 先頭（末尾）から再検索
	CheckButton(IDC_CHECK_SEARCHALL, m_pShareData->m_common.search.bSearchAll);

	// 編集中のファイル一覧を作成する
	{
		EditNode*	pEditNode;
		int			nItem;
		WIN_CHAR	szName[_MAX_PATH];
		int			count = 0;
		int			selIndex = 0;
		EncodingType	code;
		int			selCode = CODE_NONE;

		// 自分の文字コードを取得
		::SendMessage(EditWnd::getInstance()->GetHwnd(), MYWM_GETFILEINFO, 0, 0);
		EditInfo* pFileInfo = &m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
		code = pFileInfo->m_nCharCode;

		// リストのハンドル取得
		HWND hwndList = GetItemHwnd(IDC_LIST_DIFF_FILES);

		// 現在開いている編集窓のリストをメニューにする
		int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNode, TRUE);
		if (nRowNum > 0) {
			// 水平スクロール幅は実際に表示する文字列の幅を計測して決める	// 2009.09.26 ryoji
			TextWidthCalc calc(hwndList);
			int score = 0;
			TCHAR szFile1[_MAX_PATH];
			SplitPath_FolderAndFile(m_szFile1, NULL, szFile1);
			for (int i=0; i<nRowNum; ++i) {
				// トレイからエディタへの編集ファイル名要求通知
				::SendMessage(pEditNode[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
				pFileInfo = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;

				// 自分ならスキップ
				if (pEditNode[i].GetHwnd() == EditWnd::getInstance()->GetHwnd()) {
					// 同じ形式にしておく。ただしアクセスキー番号はなし
					FileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szName, _countof(szName), pFileInfo, pEditNode[i].m_nId, -1, calc.GetDC() );
					SetItemText(IDC_STATIC_DIFF_SRC, szName);
					continue;
				}

				// 番号はウィンドウ一覧と同じ番号を使う
				FileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szName, _countof(szName), pFileInfo, pEditNode[i].m_nId, i, calc.GetDC() );


				// リストに登録する
				nItem = ::List_AddString(hwndList, szName);
				List_SetItemData(hwndList, nItem, pEditNode[i].GetHwnd());
				++count;

				// 横幅を計算する
				calc.SetTextWidthIfMax(szName);

				// ファイル名一致のスコアを計算する
				TCHAR szFile2[_MAX_PATH];
				SplitPath_FolderAndFile(pFileInfo->m_szPath, NULL, szFile2);
				int scoreTemp = FileMatchScoreSepExt(szFile1, szFile2);
				if (score < scoreTemp
					|| (selCode != code && code == pFileInfo->m_nCharCode && score == scoreTemp) 
				) {
					// スコアのいいものを選択. 同じなら文字コードが同じものを選択
					score = scoreTemp;
					selIndex = nItem;
					selCode = pFileInfo->m_nCharCode;
				}
			}

			delete [] pEditNode;
			// 2002/11/01 Moca 追加 リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
			List_SetHorizontalExtent( hwndList, calc.GetCx() + 8 );

			// 最初を選択
			//List_SetCurSel(hwndList, 0);
		}

		// From Here 2004.02.22 じゅうじ
		// 開いているファイルがある場合には初期状態でそちらを優先
		if (count == 0) {
			// 相手ファイルの選択
			CheckButton(IDC_RADIO_DIFF_DST1, true);
			CheckButton(IDC_RADIO_DIFF_DST2, false);
			// その他の編集中リストはなし
			EnableItem(IDC_RADIO_DIFF_DST2, false);
			EnableItem(IDC_LIST_DIFF_FILES, false);
		}else {
			// 相手ファイルの選択
			CheckButton(IDC_RADIO_DIFF_DST1, false);
			CheckButton(IDC_RADIO_DIFF_DST2, true);
			// ListBoxが選択されていなかったら，先頭のファイルを選択する．
			HWND hwndList = GetItemHwnd(IDC_LIST_DIFF_FILES);
			if (List_GetCurSel(hwndList) == LB_ERR) {
			    List_SetCurSel(hwndList, selIndex);
			}
		}
		// To Here 2004.02.22 じゅうじ
		// Feb. 28, 2004 genta 一番上を選択位置とする．
		m_nIndexSave = selIndex;
	}

	return;
}

// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgDiff::GetData(void)
{
	BOOL ret = TRUE;

	// DIFFオプション
	m_nDiffFlgOpt = 0;
	if (IsButtonChecked(IDC_CHECK_DIFF_OPT_CASE)) m_nDiffFlgOpt |= 0x0001;
	if (IsButtonChecked(IDC_CHECK_DIFF_OPT_SPACE)) m_nDiffFlgOpt |= 0x0002;
	if (IsButtonChecked(IDC_CHECK_DIFF_OPT_SPCCHG)) m_nDiffFlgOpt |= 0x0004;
	if (IsButtonChecked(IDC_CHECK_DIFF_OPT_BLINE)) m_nDiffFlgOpt |= 0x0008;
	if (IsButtonChecked(IDC_CHECK_DIFF_OPT_TABSPC)) m_nDiffFlgOpt |= 0x0010;
	// ファイル新旧
	if (IsButtonChecked(IDC_RADIO_DIFF_FILE2)) m_nDiffFlgOpt |= 0x0020;
	// DIFF差分が見つからないときにメッセージを表示 2003.05.12 MIK
	if (IsButtonChecked(IDC_CHECK_DIFF_EXEC_STATE)) m_nDiffFlgOpt |= 0x0040;
	m_pShareData->m_nDiffFlgOpt = m_nDiffFlgOpt;

	// 相手ファイル名
	m_szFile2[0] = 0;
	m_hWnd_Dst = NULL;
	m_bIsModifiedDst = false;
	if (IsButtonChecked(IDC_RADIO_DIFF_DST1)) {
		GetItemText(IDC_EDIT_DIFF_DST, m_szFile2, _countof2(m_szFile2));
		// 2004.05.19 MIK 外部ファイルが指定されていない場合はキャンセル
		// 相手ファイルが指定されてなければキャンセル
		if (m_szFile2[0] == '\0') ret = FALSE;

	}else if (IsButtonChecked(IDC_RADIO_DIFF_DST2)) {
		// リストから相手のウィンドウハンドルを取得
		HWND hwndList = GetItemHwnd(IDC_LIST_DIFF_FILES);
		int nItem = List_GetCurSel(hwndList);
		if (nItem != LB_ERR) {
			m_hWnd_Dst = (HWND)List_GetItemData(hwndList, nItem);

			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage(m_hWnd_Dst, MYWM_GETFILEINFO, 0, 0);
			EditInfo* pFileInfo = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
			_tcscpy(m_szFile2, pFileInfo->m_szPath);
			m_bIsModifiedDst = pFileInfo->m_bIsModified;
			m_nCodeTypeDst = pFileInfo->m_nCharCode;
			m_bBomDst = pFileInfo->m_bBom;
		}else {
			ret = FALSE;
		}
	}else {
		ret = FALSE;
	}

	// 見つからないときメッセージを表示
	m_pShareData->m_common.search.bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	// 先頭（末尾）から再検索
	m_pShareData->m_common.search.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	// 相手ファイルが指定されてなければキャンセル
	// 2004.02.21 MIK 相手が無題だと比較できないので判定削除
	//if (m_szFile2[0] == '\0') ret = FALSE;

	return ret;
}

BOOL DlgDiff::OnLbnSelChange(HWND hwndCtl, int wID)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_DIFF_FILES);
	if (hwndList == hwndCtl) {
		CheckButton(IDC_RADIO_DIFF_DST1, false);
		CheckButton(IDC_RADIO_DIFF_DST2, true);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnLbnSelChange(hwndCtl, wID);
}

BOOL DlgDiff::OnEnChange(HWND hwndCtl, int wID)
{
	HWND hwndEdit = GetItemHwnd(IDC_EDIT_DIFF_DST);
	if (hwndEdit == hwndCtl) {
		CheckButton(IDC_RADIO_DIFF_DST1, true);
		CheckButton(IDC_RADIO_DIFF_DST2, false);
		// Feb. 28, 2004 genta 選択解除前に前回の位置を記憶して選択解除
		int n = List_GetCurSel(GetItemHwnd(IDC_LIST_DIFF_FILES));
		if (n != LB_ERR) {
			m_nIndexSave = n;
		}
		List_SetCurSel(GetItemHwnd(IDC_LIST_DIFF_FILES), -1);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnEnChange(hwndCtl, wID);
}

LPVOID DlgDiff::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


INT_PTR DlgDiff::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}

BOOL DlgDiff::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	CreateSizeBox();
	Dialog::OnSize();
	
	LONG_PTR lStyle;
	lStyle = ::GetWindowLongPtr(GetItemHwnd(IDC_FRAME_DIFF_DST), GWL_EXSTYLE);
	::SetWindowLongPtr(GetItemHwnd(IDC_FRAME_DIFF_DST), GWL_EXSTYLE, lStyle | WS_EX_TRANSPARENT);
	lStyle = ::GetWindowLongPtr(GetItemHwnd(IDC_FRAME_DIFF_FILE12), GWL_EXSTYLE);
	::SetWindowLongPtr(GetItemHwnd(IDC_FRAME_DIFF_FILE12), GWL_EXSTYLE, lStyle | WS_EX_TRANSPARENT);
	lStyle = ::GetWindowLongPtr(GetItemHwnd(IDC_FRAME_SEARCH_MSG), GWL_EXSTYLE);
	::SetWindowLongPtr(GetItemHwnd(IDC_FRAME_SEARCH_MSG), GWL_EXSTYLE, lStyle | WS_EX_TRANSPARENT);

	RECT rc;
	::GetWindowRect(hwndDlg, &rc);
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for (int i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, m_rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().m_common.others.rcDiffDialog;
	if (rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgDiff::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().m_common.others.rcDiffDialog);

	RECT  rc;
	POINT ptNew;
	GetWindowRect(&rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (int i=0; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

BOOL DlgDiff::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().m_common.others.rcDiffDialog);
	
	return Dialog::OnMove(wParam, lParam);
}

BOOL DlgDiff::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (m_ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*2;
	return 0;
}

BOOL DlgDiff::OnLbnDblclk( int wID )
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_DIFF_FILES );
	if (List_GetCurSel(hwndList) == LB_ERR) {
		return FALSE;
	}
	return OnBnClicked(IDOK);
}

