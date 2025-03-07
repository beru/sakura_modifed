/*!	@file
	@brief 共通設定ダイアログボックス、「キーバインド」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

#define STR_SHIFT_PLUS        _T("Shift+")
#define STR_CTRL_PLUS         _T("Ctrl+")
#define STR_ALT_PLUS          _T("Alt+")

static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYBIND,		// インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYBIND,		// エクスポート
	IDC_BUTTON_ASSIGN,				HIDC_BUTTON_ASSIGN,				// キー割り当て
	IDC_BUTTON_RELEASE,				HIDC_BUTTON_RELEASE,			// キー解除
	IDC_CHECK_SHIFT,				HIDC_CHECK_SHIFT,				// Shiftキー
	IDC_CHECK_CTRL,					HIDC_CHECK_CTRL,				// Ctrlキー
	IDC_CHECK_ALT,					HIDC_CHECK_ALT,					// Altキー
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_KEYBIND,	// 機能の種別
	IDC_EDIT_KEYSFUNC,				HIDC_EDIT_KEYSFUNC,				// キーに割り当てられている機能
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_KEYBIND,			// 機能一覧
	IDC_LIST_KEY,					HIDC_LIST_KEY,					// キー一覧
	IDC_LIST_ASSIGNEDKEYS,			HIDC_LIST_ASSIGNEDKEYS,			// 機能に割り当てられているキー
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_KEYKIND,				(DWORD)-1,
	IDC_LABEL_FUNCtoKEY,			(DWORD)-1,
	IDC_LABEL_KEYtoFUNC,			(DWORD)-1,
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropKeybind::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropKeybind::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// ウィンドウプロシージャの中で・・・
LRESULT CALLBACK CPropComKeybindWndProc(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg) {
	// WM_CTLCOLORSTATIC メッセージに対して
	case WM_CTLCOLORSTATIC:
	// 白色のブラシハンドルを返す
		return (LRESULT)GetStockObject(WHITE_BRUSH);
//	default:
//		break;
	}
	return 0;
}

// Keybind メッセージ処理
INT_PTR PropKeybind::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	static HWND	hwndCombo;
	static HWND	hwndFuncList;
	static HWND	hwndKeyList;
	static HWND	hwndCheckShift;
	static HWND	hwndCheckCtrl;
	static HWND	hwndCheckAlt;
	static HWND	hwndAssignedkeyList;
//	static HWND hwndLIST_KEYSFUNC;
	static HWND hwndEDIT_KEYSFUNC;
//	int			nLength;
	int			nAssignedKeyNum;

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	int			j;
	EFunctionCode	nFuncCode;
	static wchar_t szLabel[256];
	auto& csKeybind = common.keyBind;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Keybind
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// コントロールのハンドルを取得
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
		hwndFuncList = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
		hwndAssignedkeyList = ::GetDlgItem(hwndDlg, IDC_LIST_ASSIGNEDKEYS);
		hwndCheckShift = ::GetDlgItem(hwndDlg, IDC_CHECK_SHIFT);
		hwndCheckCtrl = ::GetDlgItem(hwndDlg, IDC_CHECK_CTRL);
		hwndCheckAlt = ::GetDlgItem(hwndDlg, IDC_CHECK_ALT);
		hwndKeyList = ::GetDlgItem(hwndDlg, IDC_LIST_KEY);
//		hwndLIST_KEYSFUNC = ::GetDlgItem(hwndDlg, IDC_LIST_KEYSFUNC);
		hwndEDIT_KEYSFUNC = ::GetDlgItem(hwndDlg, IDC_EDIT_KEYSFUNC);

		// キー選択時の処理
	// キーリストの先頭の項目を選択（リストボックス）
		List_SetCurSel(hwndKeyList, 0);
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);

		::SetTimer(hwndDlg, 1, 300, NULL);

		return TRUE;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_KEYBIND);
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE(_T("Keybind PSN_KILLACTIVE\n"));
			// ダイアログデータの取得 Keybind
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_KEYBOARD;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）
			nIndex = List_GetCurSel(hwndKeyList);
			nIndex2 = Combo_GetCurSel(hwndCombo);
			nIndex3 = List_GetCurSel(hwndFuncList);
			if (nIndex != LB_ERR) {
				::SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);
			}
			if (nIndex2 != CB_ERR) {
				::SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);
				if (nIndex3 != LB_ERR) {
					List_SetCurSel(hwndFuncList, nIndex3);
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);		// 項目ID､ コントロールID､ またはアクセラレータID
		hwndCtl = (HWND) lParam;	// コントロールのハンドル

		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_IMPORT:	// インポート
				// Keybind:キー割り当て設定をインポートする
				Import(hwndDlg);
				return TRUE;
			case IDC_BUTTON_EXPORT:	// エクスポート
				// Keybind:キー割り当て設定をエクスポートする
				Export(hwndDlg);
				return TRUE;
			case IDC_BUTTON_ASSIGN:	// 割付
				nIndex = List_GetCurSel(hwndKeyList);
				nIndex2 = Combo_GetCurSel(hwndCombo);
				nIndex3 = List_GetCurSel(hwndFuncList);
				if (nIndex == LB_ERR || nIndex2 == CB_ERR || nIndex3 == LB_ERR) {
					return TRUE;
				}
				nFuncCode = lookup.Pos2FuncCode(nIndex2, nIndex3);
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHIFT)) {
					i |= _SHIFT;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_CTRL)) {
					i |= _CTRL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALT)) {
					i |= _ALT;
				}
				csKeybind.pKeyNameArr[nIndex].nFuncCodeArr[i] = nFuncCode;
				::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);
				::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_FUNC, LBN_SELCHANGE), (LPARAM)hwndFuncList);
				return TRUE;
			case IDC_BUTTON_RELEASE:	// 解除
				nIndex = List_GetCurSel(hwndKeyList);
				if (nIndex == LB_ERR) {
					return TRUE;
				}
				nFuncCode = F_DEFAULT;
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHIFT)) {
					i |= _SHIFT;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_CTRL)) {
					i |= _CTRL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALT)) {
					i |= _ALT;
				}
				csKeybind.pKeyNameArr[nIndex].nFuncCodeArr[i] = nFuncCode;
				::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);
				::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_FUNC, LBN_SELCHANGE), (LPARAM)hwndFuncList);
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		if (hwndCheckShift == hwndCtl
		 || hwndCheckCtrl == hwndCtl
		 || hwndCheckAlt == hwndCtl
		) {
			switch (wNotifyCode) {
			case BN_CLICKED:
				ChangeKeyList(hwndDlg);
				return TRUE;
			}
		}else
		if (hwndKeyList == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel(hwndKeyList);
				i = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHIFT)) {
					i |= _SHIFT;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_CTRL)) {
					i |= _CTRL;
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALT)) {
					i |= _ALT;
				}
				nFuncCode = csKeybind.pKeyNameArr[nIndex].nFuncCodeArr[i];
				// F_DISABLEなら未割付
				if (nFuncCode == F_DISABLE) {
					auto_strcpy(szLabel, LSW(STR_PROPCOMKEYBIND_UNASSIGN));
				}else {
					lookup.Funccode2Name(nFuncCode, szLabel, 255);
				}
				Wnd_SetText(hwndEDIT_KEYSFUNC, szLabel);
				return TRUE;
			}
		}else
		if (hwndFuncList == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel(hwndKeyList);
				nIndex2 = Combo_GetCurSel(hwndCombo);
				nIndex3 = List_GetCurSel(hwndFuncList);
				nFuncCode = lookup.Pos2FuncCode(nIndex2, nIndex3);
				// 機能に対応するキー名の取得(複数)
				NativeT**	ppcAssignedKeyList;
				nAssignedKeyNum = KeyBind::GetKeyStrList(	// 機能に対応するキー名の取得(複数)
					G_AppInstance(), csKeybind.nKeyNameArrNum, (KeyData*)csKeybind.pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// デフォルト機能は取得しない
				);	
				// 割り当てキーリストをクリアして値の設定
				List_ResetContent(hwndAssignedkeyList);
				if (0 < nAssignedKeyNum) {
					for (j=0; j<nAssignedKeyNum; ++j) {
						// デバッグモニタに出力
						const TCHAR* cpszString = ppcAssignedKeyList[j]->GetStringPtr();
						::List_AddString(hwndAssignedkeyList, cpszString);
						delete ppcAssignedKeyList[j];
					}
					delete[] ppcAssignedKeyList;
				}
				return TRUE;
			}
		}else
		if (hwndCombo == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel(hwndCombo);
				// 機能一覧に文字列をセット（リストボックス）
				if (nIndex2 != CB_ERR) {
					lookup.SetListItem(hwndFuncList, nIndex2);
				}
				return TRUE;
			}

		}else
		if (hwndAssignedkeyList == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
			//case LBN_DBLCLK:
				{
					TCHAR	buff[1024], *p;
					int	ret;

					nIndex = List_GetCurSel(hwndAssignedkeyList);
					auto_memset(buff, 0, _countof(buff));
					ret = List_GetText(hwndAssignedkeyList, nIndex, buff);
					if (ret != LB_ERR) {
						i = 0;
						p = buff;
						// SHIFT
						if (auto_memcmp(p, STR_SHIFT_PLUS, _tcslen(STR_SHIFT_PLUS)) == 0) {
							p += _tcslen(STR_SHIFT_PLUS);
							i |= _SHIFT;
						}
						// CTRL
						if (auto_memcmp(p, STR_CTRL_PLUS, _tcslen(STR_CTRL_PLUS)) == 0) {
							p += _tcslen(STR_CTRL_PLUS);
							i |= _CTRL;
						}
						// ALT
						if (auto_memcmp(p, STR_ALT_PLUS, _tcslen(STR_ALT_PLUS)) == 0) {
							p += _tcslen(STR_ALT_PLUS);
							i |= _ALT;
						}
						for (j=0; j<csKeybind.nKeyNameArrNum; ++j) {
							if (_tcscmp(csKeybind.pKeyNameArr[j].szKeyName, p) == 0) {
								List_SetCurSel(hwndKeyList, j);
								if (i & _SHIFT) ::CheckDlgButton(hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED);	// チェック
								else            ::CheckDlgButton(hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED);	// チェックをはずす
								if (i & _CTRL)  ::CheckDlgButton(hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED);	// チェック
								else            ::CheckDlgButton(hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED);	// チェックをはずす
								if (i & _ALT)   ::CheckDlgButton(hwndDlg, IDC_CHECK_ALT,   BST_CHECKED);	// チェック
								else            ::CheckDlgButton(hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED);	// チェックをはずす
								::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);

								// キー一覧の文字列も変更
								ChangeKeyList(hwndDlg);
								break;
							}
						}
					}
					return TRUE;
				}
			}

		}
		break;

	case WM_TIMER:
		// ボタンの有効／無効を切り替える
		nIndex = List_GetCurSel(hwndKeyList);
		nIndex2 = Combo_GetCurSel(hwndCombo);
		nIndex3 = List_GetCurSel(hwndFuncList);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ASSIGN), !(nIndex == LB_ERR || nIndex2 == CB_ERR || nIndex3 == LB_ERR));
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RELEASE), !(nIndex == LB_ERR));
		break;

	case WM_DESTROY:
		::KillTimer(hwndDlg, 1);
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		//break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


// ダイアログデータの設定 Keybind
void PropKeybind::SetData(HWND hwndDlg)
{
	// 機能種別一覧に文字列をセット（コンボボックス）
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
	lookup.SetCategory2Combo(hwndCombo);

	// 種別の先頭の項目を選択（コンボボックス）
	Combo_SetCurSel(hwndCombo, 0);

	// キー一覧に文字列をセット（リストボックス）
	HWND hwndKeyList = ::GetDlgItem(hwndDlg, IDC_LIST_KEY);
	auto& csKeybind = common.keyBind;
	for (int i=0; i<csKeybind.nKeyNameArrNum; ++i) {
		::List_AddString(hwndKeyList, csKeybind.pKeyNameArr[i].szKeyName);
	}

	return;
}


// ダイアログデータの取得 Keybind
int PropKeybind::GetData(HWND hwndDlg)
{
	return TRUE;
}
	
// Keybind: キーリストをチェックボックスの状態に合わせて更新する
void PropKeybind::ChangeKeyList(HWND hwndDlg) {
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem(hwndDlg, IDC_LIST_KEY);
	nIndex = List_GetCurSel(hwndKeyList);
	nIndexTop = List_GetTopIndex(hwndKeyList);
	szKeyState[0] = 0;
	i = 0;
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHIFT)) {
		i |= _SHIFT;
		wcscat(szKeyState, L"Shift+");
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_CTRL)) {
		i |= _CTRL;
		wcscat(szKeyState, L"Ctrl+");
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALT)) {
		i |= _ALT;
		wcscat(szKeyState, L"Alt+");
	}
	// キー一覧に文字列をセット（リストボックス）
	List_ResetContent(hwndKeyList);
	auto& csKeybind = common.keyBind;
	for (i=0; i<csKeybind.nKeyNameArrNum; ++i) {
		TCHAR	szLabel[256];
		auto_sprintf(szLabel, _T("%ls%ts"), szKeyState, csKeybind.pKeyNameArr[i].szKeyName);
		::List_AddString(hwndKeyList, szLabel);
	}
	List_SetCurSel(hwndKeyList, nIndex);
	List_SetTopIndex(hwndKeyList, nIndexTop);
	::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_KEY, LBN_SELCHANGE), (LPARAM)hwndKeyList);
}

// Keybind:キー割り当て設定をインポートする
void PropKeybind::Import(HWND hwndDlg)
{
	ImpExpKeybind	impExpKeybind(common);

	// インポート
	if (!impExpKeybind.ImportUI(G_AppInstance(), hwndDlg)) {
		// インポートをしていない
		return;
	}

	// ダイアログデータの設定 Keybind
	ChangeKeyList(hwndDlg);
	// 機能に割り当てられているキーを更新する
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
	::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_FUNC, LBN_SELCHANGE), (LPARAM)hwndCtrl);
}


// Keybind:キー割り当て設定をエクスポートする
void PropKeybind::Export(HWND hwndDlg)
{
	ImpExpKeybind	impExpKeybind(common);

	// エクスポート
	if (!impExpKeybind.ExportUI(G_AppInstance(), hwndDlg)) {
		// エクスポートをしていない
		return;
	}
}

