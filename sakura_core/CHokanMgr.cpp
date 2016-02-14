/*!	@file
	@brief キーワード補完

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, genta, Moca, novice
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CHokanMgr.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "plugin/CJackManager.h"
#include "plugin/CComplementIfObj.h"
#include "util/input.h"
#include "util/os.h"
#include "util/other_util.h"
#include "sakura_rc.h"

WNDPROC gm_wpHokanListProc;


LRESULT APIENTRY HokanList_SubclassProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	// Modified by KEITA for WIN64 2003.9.6
	Dialog* pDialog = (Dialog*)::GetWindowLongPtr(::GetParent(hwnd), DWLP_USER);
	HokanMgr* pHokanMgr = (HokanMgr*)::GetWindowLongPtr(::GetParent(hwnd), DWLP_USER);
	MSG* pMsg;
	int nVKey;

	switch (uMsg) {
	case WM_KEYDOWN:
		nVKey = (int) wParam;	// virtual-key code
		// キー操作を偽造しよう
		if (nVKey == VK_SPACE) {	//	Space
// novice 2004/10/10
			// Shift,Ctrl,Altキーが押されていたか
			int nIdx = getCtrlKeyState();
			if (nIdx == _SHIFT) {
				//	Shift + Spaceで↑を偽造
				wParam = VK_UP;
			}else if (nIdx == 0) {
				//	Spaceのみで↓を偽造
				wParam = VK_DOWN;
			}
		}
		// 補完実行キーなら補完する
		if (pHokanMgr->KeyProc(wParam, lParam) != -1) {
			// キーストロークを親に転送
			return ::PostMessage(::GetParent(::GetParent(pDialog->m_hwndParent)), uMsg, wParam, lParam);
		}
		break;
	case WM_GETDLGCODE:
		pMsg = (MSG*) lParam; // pointer to an MSG structure
		if (!pMsg) {
//			MYTRACE(_T("WM_GETDLGCODE  pMsg==NULL\n"));
			return 0;
		}
//		MYTRACE(_T("WM_GETDLGCODE  pMsg->message = %xh\n"), pMsg->message);
		return DLGC_WANTALLKEYS; // すべてのキーストロークを私に下さい	//	Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	}
	return CallWindowProc(gm_wpHokanListProc, hwnd, uMsg, wParam, lParam);
}


HokanMgr::HokanMgr()
{
	m_memCurWord.SetString(L"");

	m_nCurKouhoIdx = -1;
	m_bTimerFlag = TRUE;
}

HokanMgr::~HokanMgr()
{
}

// モードレスダイアログの表示
HWND HokanMgr::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	HWND hwndWork = Dialog::DoModeless(hInstance, hwndParent, IDD_HOKAN, lParam, SW_HIDE);
	OnSize(0, 0);
	// リストをフック
	// Modified by KEITA for WIN64 2003.9.6
	::gm_wpHokanListProc = (WNDPROC) ::SetWindowLongPtr(GetItemHwnd(IDC_LIST_WORDS), GWLP_WNDPROC, (LONG_PTR)HokanList_SubclassProc);

	::ShowWindow(GetHwnd(), SW_HIDE);
	return hwndWork;
}

// モードレス時：対象となるビューの変更
void HokanMgr::ChangeView(LPARAM pcEditView)
{
	m_lParam = pcEditView;
	return;
}

void HokanMgr::Hide(void)
{

	::ShowWindow(GetHwnd(), SW_HIDE);
	m_nCurKouhoIdx = -1;
	// 入力フォーカスを受け取ったときの処理
	EditView* pEditView = reinterpret_cast<EditView*>(m_lParam);
	pEditView->OnSetFocus();
	return;

}

/*!	初期化
	pcmemHokanWord == NULLのとき、補完候補がひとつだったら、補完ウィンドウを表示しないで終了します。
	Search()呼び出し元で確定処理を進めてください。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
int HokanMgr::Search(
	POINT*			ppoWin,
	int				nWinHeight,
	int				nColumnWidth,
	const wchar_t*	pszCurWord,
	const TCHAR*	pszHokanFile,
	bool			bHokanLoHiCase,	// 入力補完機能：英大文字小文字を同一視する 2001/06/19 asa-o
	bool			bHokanByFile,	// 編集中データから候補を探す 2003.06.23 Moca
	int				nHokanType,
	bool			bHokanByKeyword,
	NativeW*		pcmemHokanWord	// 2001/06/19 asa-o
)
{
	EditView* pEditView = reinterpret_cast<EditView*>(m_lParam);

	// 共有データ構造体のアドレスを返す
	m_pShareData = &GetDllShareData();

	/*
	||  補完キーワードの検索
	||
	||  ・見つかった候補をすべて返す(改行で区切って返す)
	||  ・指定された候補の最大数を超えると処理を中断する
	||  ・見つかった数を返す
	||
	*/
	m_vKouho.clear();
	DicMgr::HokanSearch(
		pszCurWord,
		bHokanLoHiCase,								// 引数からに変更	2001/06/19 asa-o
		m_vKouho,
		0, // Max候補数
		pszHokanFile
	);

	// 2003.05.16 Moca 追加 編集中データ内から候補を探す
	if (bHokanByFile) {
		pEditView->HokanSearchByFile(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho,
			1024 // 編集中データからなので数を制限しておく
		);
	}
	// 2012.10.13 Moca 強調キーワードから候補を探す
	if (bHokanByKeyword) {
		HokanSearchByKeyword(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho
		);
	}

	{
		int nOption = (
			  (bHokanLoHiCase ? 0x01 : 0)
			  | (bHokanByFile ? 0x02 : 0)
			);
		
		Plug::Array plugs;
		Plug::Array plugType;
		JackManager::getInstance()->GetUsablePlug(PP_COMPLEMENTGLOBAL, 0, &plugs);
		if (nHokanType != 0) {
			JackManager::getInstance()->GetUsablePlug(PP_COMPLEMENT, nHokanType, &plugType);
			if (0 < plugType.size()) {
				plugs.push_back(plugType[0]);
			}
		}

		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			// インタフェースオブジェクト準備
			WSHIfObj::List params;
			std::wstring curWord = pszCurWord;
			ComplementIfObj* objComp = new ComplementIfObj(curWord , this, nOption);
			objComp->AddRef();
			params.push_back(objComp);
			// プラグイン呼び出し
			(*it)->Invoke(pEditView, params);

			objComp->Release();
		}
	}

	if (m_vKouho.size() == 0) {
		m_nCurKouhoIdx = -1;
		return 0;
	}

//	2001/06/19 asa-o 候補が１つの場合補完ウィンドウは表示しない(逐次補完の場合は除く)
	if (m_vKouho.size() == 1) {
		if (pcmemHokanWord) {
			m_nCurKouhoIdx = -1;
			pcmemHokanWord->SetString(m_vKouho[0].c_str());
			return 1;
		}
	}


//	m_hFont = hFont;
	m_poWin.x = ppoWin->x;
	m_poWin.y = ppoWin->y;
	m_nWinHeight = nWinHeight;
	m_nColumnWidth = nColumnWidth;
//	m_memCurWord.SetData(pszCurWord, lstrlen(pszCurWord));
	m_memCurWord.SetString(pszCurWord);


	m_nCurKouhoIdx = 0;
//	SetCurKouhoStr();

//	::ShowWindow(GetHwnd(), SW_SHOWNA);

	HWND hwndList;
	hwndList = GetItemHwnd(IDC_LIST_WORDS);
	List_ResetContent(hwndList);
	{
		size_t kouhoNum = m_vKouho.size();
		for (size_t i=0; i<kouhoNum; ++i) {
			::List_AddString(hwndList, m_vKouho[i].c_str());
		}
	}
	List_SetCurSel(hwndList, 0);


//@@	::EnableWindow(::GetParent(::GetParent(m_hwndParent)), FALSE);


	RECT rcDesktop;
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect(GetHwnd(), &rcDesktop );

	int nX = m_poWin.x - m_nColumnWidth;
	int nY = m_poWin.y + m_nWinHeight + 4;
	int nCX = m_nWidth;
	int nCY = m_nHeight;

	// 下に入るなら
	if (nY + nCY < rcDesktop.bottom) {
		// 何もしない
	}else
	// 上に入るなら
	if (rcDesktop.top < m_poWin.y - m_nHeight - 4) {
		// 上に出す
		nY = m_poWin.y - m_nHeight - 4;
	}else
	// 上に出すか下に出すか(広いほうに出す)
	if (rcDesktop.bottom - nY > m_poWin.y) {
		// 下に出す
//		m_nHeight = rcDesktop.bottom - nY;
		nCY = rcDesktop.bottom - nY;
	}else {
		// 上に出す
		nY = rcDesktop.top;
		nCY = m_poWin.y - 4 - rcDesktop.top;
	}

//	2001/06/19 Start by asa-o: 表示位置補正

	// 右に入る
	if (nX + nCX < rcDesktop.right) {
		// そのまま
	}else
	// 左に入る
	if (rcDesktop.left < nX - nCX + 8) {
		// 左に表示
		nX -= nCX - 8;
	}else {
		// サイズを調整して右に表示
		nCX = t_max((int)(rcDesktop.right - nX) , 100);	// 最低サイズを100くらいに
	}

//	2001/06/19 End

//	2001/06/18 Start by asa-o: 補正後の位置・サイズを保存
	m_poWin.x = nX;
	m_poWin.y = nY;
	m_nHeight = nCY;
	m_nWidth = nCX;
//	2001/06/18 End

	// はみ出すなら小さくする
//	if (rcDesktop.bottom < nY + nCY) {
//		// 下にはみ出す
//		if (m_poWin.y - 4 - nCY < 0) {
//			// 上にはみ出す
//			// →高さだけ調節
//			nCY = rcDesktop.bottom - nY - 4;
//		}else {
//
//		}
//
//	}
	::MoveWindow(GetHwnd(), nX, nY, nCX, nCY, TRUE);
	::ShowWindow(GetHwnd(), SW_SHOW);
//	::ShowWindow(GetHwnd(), SW_SHOWNA);
	::SetFocus(GetHwnd());
//	::SetFocus(GetItemHwnd(IDC_LIST_WORDS));
//	::SetFocus(::GetParent(::GetParent(m_hwndParent)));


//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示

//	2003.06.25 Moca 他のメソッドで使っていないので、とりあえず削除しておく
	int kouhoNum = m_vKouho.size();
	m_vKouho.clear();
	return kouhoNum;
}

void HokanMgr::HokanSearchByKeyword(
	const wchar_t*	pszCurWord,
	bool 			bHokanLoHiCase,
	vector_ex<std::wstring>& 	vKouho
) {
	const EditView* pEditView = reinterpret_cast<const EditView*>(m_lParam);
	const TypeConfig& type = pEditView->GetDocument()->m_docType.GetDocumentAttribute();
	KeyWordSetMgr& keywordMgr = m_pShareData->m_common.m_specialKeyword.m_keyWordSetMgr;
	const int nKeyLen = wcslen(pszCurWord);
	for (int n=0; n<MAX_KEYWORDSET_PER_TYPE; ++n) {
		int kwdset = type.m_nKeyWordSetIdx[n];
		if (kwdset == -1) {
			continue;
		}
		const int keyCount = keywordMgr.GetKeyWordNum(kwdset);
		for (int i=0; i<keyCount; ++i) {
			const wchar_t* word = keywordMgr.GetKeyWord(kwdset, i);
			int nRet;
			if (bHokanLoHiCase) {
				nRet = auto_memicmp(pszCurWord, word, nKeyLen );
			}else {
				nRet = auto_memcmp(pszCurWord, word, nKeyLen );
			}
			if (nRet != 0) {
				continue;
			}
			std::wstring strWord = std::wstring(word);
			AddKouhoUnique(vKouho, strWord);
		}
	}
}


BOOL HokanMgr::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);
	// 基底クラスメンバ
//-	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);

}

BOOL HokanMgr::OnDestroy(void)
{
	// 基底クラスメンバ
	CreateSizeBox();
	return Dialog::OnDestroy();
}


BOOL HokanMgr::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	int	Controls[] = {
		IDC_LIST_WORDS
	};
	RECT	rcDlg;
	::GetClientRect(GetHwnd(), &rcDlg);
	int nWidth = rcDlg.right - rcDlg.left;  // width of client area
	int nHeight = rcDlg.bottom - rcDlg.top; // height of client area

//	2001/06/18 Start by asa-o: サイズ変更後の位置を保存
	m_poWin.x = rcDlg.left - 4;
	m_poWin.y = rcDlg.top - 3;
	::ClientToScreen(GetHwnd(), &m_poWin);
//	2001/06/18 End

	int nControls = _countof(Controls);
	for (int i=0; i<nControls; ++i) {
		HWND hwndCtrl = GetItemHwnd(Controls[i]);
		RECT rc;
		::GetWindowRect(hwndCtrl, &rc);
		POINT po;
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient(GetHwnd(), &po);
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient(GetHwnd(), &po);
		rc.right = po.x;
		rc.bottom  = po.y;
		if (Controls[i] == IDC_LIST_WORDS) {
			::SetWindowPos(
				hwndCtrl,
				NULL,
				rc.left,
				rc.top,
				nWidth - rc.left * 2,
				nHeight - rc.top * 2/* - 20*/,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
	}

//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示

	return TRUE;

}

BOOL HokanMgr::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:
//		CloseDialog(0);
		Hide();
		return TRUE;
	case IDOK:
//		CloseDialog(0);
		// 補完実行
		DoHokan(VK_RETURN);
		return TRUE;
	}
	return FALSE;
}


BOOL HokanMgr::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	int nVKey = (int) wParam;	// virtual-key code
//	lKeyData = lParam;			// key data
	switch (nVKey) {
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		return 1;
	}
 	return 0;
}


BOOL HokanMgr::OnLbnSelChange(HWND hwndCtl, int wID)
{
//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示
	return TRUE;
}

BOOL HokanMgr::OnLbnDblclk(int wID)
{
	// 補完実行
	DoHokan(0);
	return TRUE;

}


BOOL HokanMgr::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
//	Hide();
	return TRUE;
}


// 補完実行
BOOL HokanMgr::DoHokan(int nVKey)
{
	DEBUG_TRACE(_T("HokanMgr::DoHokan(nVKey==%xh)\n"), nVKey);

	// 補完候補決定キー
	auto& csHelper = m_pShareData->m_common.m_helper;
	if (nVKey == VK_RETURN	&& !csHelper.m_bHokanKey_RETURN)	return FALSE; // VK_RETURN 補完決定キーが有効/無効
	if (nVKey == VK_TAB		&& !csHelper.m_bHokanKey_TAB)		return FALSE; // VK_TAB    補完決定キーが有効/無効
	if (nVKey == VK_RIGHT	&& !csHelper.m_bHokanKey_RIGHT)		return FALSE; // VK_RIGHT  補完決定キーが有効/無効

	HWND hwndList = GetItemHwnd(IDC_LIST_WORDS);
	int nItem = List_GetCurSel(hwndList);
	if (nItem == LB_ERR) {
		return FALSE;
	}
	int nLabelLen = List_GetTextLen( hwndList, nItem );
	auto_array_ptr<WCHAR> wszLabel( new WCHAR [nLabelLen + 1] );
	List_GetText( hwndList, nItem, &wszLabel[0] );

 	// テキストを貼り付け
	EditView* pEditView = reinterpret_cast<EditView*>(m_lParam);
	//	Apr. 28, 2000 genta
	pEditView->GetCommander().HandleCommand(F_WordDeleteToStart, false, 0, 0, 0, 0);
	pEditView->GetCommander().HandleCommand( F_INSTEXT_W, true, (LPARAM)&wszLabel[0], wcslen(&wszLabel[0]), TRUE, 0 );

	// Until here
//	pEditView->GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)(wszLabel + m_memCurWord.GetLength()), TRUE, 0, 0);
	Hide();

	m_pShareData->m_common.m_helper.m_bUseHokan = FALSE;	//	補完したら
	return TRUE;
}

/*
戻り値が -2 の場合は、アプリケーションは項目の選択を完了し、
リスト ボックスでそれ以上の動作が必要でないことを示します。

戻り値が -1 の場合は、リスト ボックスがキーストロークに応じて
デフォルトの動作を実行することを示します。

 戻り値が 0 以上の場合は、その値はリスト ボックスの項目の 0 を
基準としたインデックスを意味し、リスト ボックスがその項目での
キーストロークに応じてデフォルトの動作を実行することを示します。

*/
//	int HokanMgr::OnVKeyToItem(WPARAM wParam, LPARAM lParam)
//	{
//		return KeyProc(wParam, lParam);
//	}

/*
戻り値が -2 の場合は、アプリケーションは項目の選択を完了し、
リスト ボックスでそれ以上の動作が必要でないことを示します。

戻り値が -1 の場合は、リスト ボックスがキーストロークに応じて
デフォルトの動作を実行することを示します。

 戻り値が 0 以上の場合は、その値はリスト ボックスの項目の 0 を
基準としたインデックスを意味し、リスト ボックスがその項目での
キーストロークに応じてデフォルトの動作を実行することを示します。

*/
//	int HokanMgr::OnCharToItem(WPARAM wParam, LPARAM lParam)
//	{
//		WORD vkey;
//		WORD nCaretPos;
//		LPARAM hwndLB;
//		vkey = LOWORD(wParam);		// virtual-key code
//		nCaretPos = HIWORD(wParam);	// caret position
//		hwndLB = lParam;			// handle to list box
//	//	switch (vkey) {
//	//	}
//
//		MYTRACE(_T("HokanMgr::OnCharToItem vkey=%xh\n"), vkey);
//		return -1;
//	}

int HokanMgr::KeyProc(WPARAM wParam, LPARAM lParam)
{
	WORD vkey = LOWORD(wParam);		// virtual-key code
//	MYTRACE(_T("HokanMgr::OnVKeyToItem vkey=%xh\n"), vkey);
	switch (vkey) {
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		// リストボックスのデフォルトの動作をさせる
		return -1;
	case VK_RETURN:
	case VK_TAB:
	case VK_RIGHT:
#if 0
	case VK_SPACE:
#endif
		// 補完実行
		if (DoHokan(vkey)) {
			return -1;
		}else {
			return -2;
		}
	case VK_ESCAPE:
	case VK_LEFT:
		m_pShareData->m_common.m_helper.m_bUseHokan = FALSE;
		return -2;
	}
	return -2;
}

//	2001/06/18 Start by asa-o: 補完ウィンドウで選択中の単語にキーワードヘルプを表示
void HokanMgr::ShowTip()
{
	HWND hwndCtrl = GetItemHwnd(IDC_LIST_WORDS);
	int nItem = List_GetCurSel(hwndCtrl);
	if (nItem == LB_ERR) {
		return ;
	}

	int nLabelLen = List_GetTextLen( hwndCtrl, nItem );
	auto_array_ptr<WCHAR> szLabel( new WCHAR [nLabelLen + 1] );
	List_GetText( hwndCtrl, nItem, &szLabel[0] );	// 選択中の単語を取得

	EditView* pEditView = reinterpret_cast<EditView*>(m_lParam);
	// すでに辞書Tipが表示されていたら
	if (pEditView->m_dwTipTimer == 0) {
		// 辞書Tipを消す
		pEditView -> m_tipWnd.Hide();
		pEditView -> m_dwTipTimer = ::GetTickCount();
	}

	// 表示する位置を決定
	int nTopItem = List_GetTopIndex(hwndCtrl);
	int nItemHeight = List_GetItemHeight(hwndCtrl, 0);
	POINT point;
	point.x = m_poWin.x + m_nWidth;
	point.y = m_poWin.y + 4 + (nItem - nTopItem) * nItemHeight;
	// 2001/06/19 asa-o 選択中の単語が補完ウィンドウに表示されているなら辞書Tipを表示
	if (point.y > m_poWin.y && point.y < m_poWin.y + m_nHeight) {
		RECT rcHokanWin;
		::SetRect(&rcHokanWin , m_poWin.x, m_poWin.y, m_poWin.x + m_nWidth, m_poWin.y + m_nHeight);
		if (!pEditView -> ShowKeywordHelp( point, &szLabel[0], &rcHokanWin )) {
			pEditView -> m_dwTipTimer = ::GetTickCount();	// 表示するべきキーワードヘルプが無い
		}
	}
}
//	2001/06/18 End

bool HokanMgr::AddKouhoUnique(
	vector_ex<std::wstring>& kouhoList,
	const std::wstring& strWord
	)
{
	return kouhoList.push_back_unique(strWord);
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID HokanMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


