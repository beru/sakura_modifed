/*!	@file
	共通設定ダイアログボックス、「メインメニュー」ページ

	@author Uchi
*/
/*
	Copyright (C) 2010, Uchi

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

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/ShareData.h"
#include "env/ShareData_IO.h"
#include "typeprop/ImpExpManager.h"
#include "dlg/DlgInput1.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

using std::wstring;

// TreeView 表示固定初期値

static const DWORD p_helpids[] = {
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,				// 機能の種別
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,						// 機能一覧
	IDC_TREE_RES,					HIDC_TREE_RES,						// メニュー一覧
	IDC_BUTTON_DELETE,				HIDC_BUTTON_TREE_DELETE,			// メニューから機能削除
	IDC_BUTTON_INSERT_NODE,			HIDC_BUTTON_TREE_INSERT_NODE,		// メニューへノード追加
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_TREE_INSERT_SEPARATOR,	// メニューへ区切線挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_TREE_INSERT,			// メニューへ機能挿入(上)
	IDC_BUTTON_INSERT_A,			HIDC_BUTTON_TREE_INSERT_A,			// メニューへ機能挿入(下)
	IDC_BUTTON_ADD,					HIDC_BUTTON_TREE_ADD,				// メニューへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_TREE_UP,				// メニューの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_TREE_DOWN,				// メニューの機能を下へ移動
	IDC_BUTTON_RIGHT,				HIDC_BUTTON_TREE_RIGHT,				// メニューの機能を右へ移動
	IDC_BUTTON_LEFT,				HIDC_BUTTON_TREE_LEFT,				// メニューの機能を左へ移動
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,					// メニューのインポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,					// メニューのエクスポート
	IDC_BUTTON_CHECK,				HIDC_BUTTON_NENU_CHECK,				// メニューの検査
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_TREE_CLEAR,				// メニューをクリア
	IDC_BUTTON_INITIALIZE,			HIDC_BUTTON_TREE_INITIALIZE,		// メニューを初期状態に戻す
	IDC_CHECK_KEY_PARENTHESES,		HIDC_CHECK_KEY_PARENTHESES,			// アクセスキーを必ず()付で表示(&P)
	0, 0
};

// 内部使用変数
// 機能格納(Work)
struct MainMenuWork {
	wstring			sName;		// 名前
	EFunctionCode	nFunc;		// Function
	WCHAR			sKey[2];		// アクセスキー
	bool			bDupErr;		// アクセスキー重複エラー
	bool			bIsNode;		// ノードか否か（ノードでもnFuncがF_NODE(0)でないものがあるため）
};

static	std::map<int, MainMenuWork>	msMenu;	// 一時データ
static	int		nMenuCnt = 0;					// 一時データ番号


// ローカル関数定義
static HTREEITEM TreeCopy(HWND, HTREEITEM, HTREEITEM, bool, bool);
static void TreeView_ExpandAll(HWND, bool);
static const TCHAR* MakeDispLabel(MainMenuWork*);

// 特別機能
struct SpecialFunc	{
	EFunctionCode	nFunc;		// Function
	int			 	nNameId;		// 名前
};

extern const	SpecialFunc	gSpecialFuncs[] = {
	{F_WINDOW_LIST,				STR_SPECIAL_FUNC_WINDOW },
	{F_FILE_USED_RECENTLY,		STR_SPECIAL_FUNC_RECENT_FILE },
	{F_FOLDER_USED_RECENTLY,	STR_SPECIAL_FUNC_RECENT_FOLDER },
	{F_CUSTMENU_LIST,			STR_SPECIAL_FUNC_CUST_MENU },
	{F_USERMACRO_LIST,			STR_SPECIAL_FUNC_MACRO },
	{F_PLUGIN_LIST,				STR_SPECIAL_FUNC_PLUGIN_CMD },
};
extern const int gSpecialFuncsCount = (int)_countof(gSpecialFuncs);

static	int 	nSpecialFuncsNum;		// 特別機能のコンボボックス内での番号

//  TreeViewキー入力時のメッセージ処理用
static WNDPROC	wpTreeView = NULL;
static HWND		hwndDlg;

// TreeViewラベル編集時のメッセージ処理用
static WNDPROC	wpEdit = NULL;


/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropMainMenu::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropMainMenu::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}


// TreeViewキー入力時のメッセージ処理
static LRESULT CALLBACK TreeViewProc(
	HWND	hwndTree,		// handle to dialog box
	UINT	uMsg,			// message
	WPARAM	wParam,			// first message parameter
	LPARAM	lParam 			// second message parameter
	)
{
	HTREEITEM		htiItem;
	TV_ITEM			tvi;		// 取得用
	WCHAR			cKey;
	MainMenuWork*	pFuncWk;	// 機能

	switch (uMsg) {
	case WM_GETDLGCODE:
		MSG*	pMsg;
		if (lParam == 0) {
			break;
		}
		pMsg = (MSG*)lParam;
		if (pMsg->wParam == wParam && (wParam == VK_RETURN|| wParam == VK_ESCAPE || wParam == VK_TAB)) {
			break;
		}
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		htiItem = TreeView_GetSelection(hwndTree);
		cKey = (WCHAR)MapVirtualKey(wParam, 2);
		if (cKey > ' ') {
			// アクセスキー設定
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			if (!TreeView_GetItem(hwndTree, &tvi)) {
				break;
			}
			pFuncWk = &msMenu[tvi.lParam];
			if (pFuncWk->nFunc == F_SEPARATOR) {
				return 0;
			}
			pFuncWk->sKey[0] = cKey;
			pFuncWk->sKey[1] = L'\0';
			pFuncWk->bDupErr = false;
			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.pszText = const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
			TreeView_SetItem(hwndTree , &tvi);		//	キー設定結果を反映
			return 0;
		}

		switch (wParam) {
		case VK_BACK:
		case VK_DELETE:	//	DELキーが押されたらダイアログボックスにメッセージを送信
			::SendMessage(hwndDlg, WM_COMMAND, IDC_BUTTON_DELETE, (LPARAM)::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE));
			return 0;
		case VK_F2:						// F2で編集
			if (htiItem) {
				TreeView_EditLabel(hwndTree, htiItem);
			}
			return 0;
		}
		break;
	case WM_CHAR:
		return 0;
	}
	return  CallWindowProc(wpTreeView, hwndTree, uMsg, wParam, lParam);
}

// TreeViewラベル編集時のメッセージ処理
static LRESULT CALLBACK WindowProcEdit(
	HWND	hwndEdit,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	switch (uMsg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}
	return CallWindowProc(wpEdit, hwndEdit, uMsg, wParam, lParam);
}

// Menu メッセージ処理
INT_PTR PropMainMenu::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	static HWND	hwndComboFunkKind;
	static HWND	hwndListFunk;
	static HWND	hwndTreeRes;

	HTREEITEM	nIdxMenu;
	int			nIdxFIdx;
	int			nIdxFunc;
	WCHAR		szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	EFunctionCode	eFuncCode;
	MainMenuWork*	pFuncWk;	// 機能
	TCHAR			szKey[2];

	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	HTREEITEM		htiItem;
	HTREEITEM		htiParent;
	HTREEITEM		htiTemp;
	HTREEITEM		htiTemp2;
	TV_DISPINFO*	ptdi;

	DlgInput1		dlgInput1;

	static	bool	bInMove;
	bool			bIsNode;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Menu
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// コントロールのハンドルを取得
		hwndComboFunkKind = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
		hwndListFunk = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
		hwndTreeRes = ::GetDlgItem(hwndDlg, IDC_TREE_RES);

		// キー選択時の処理
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndComboFunkKind);

		// TreeViewのメッセージ処理（アクセスキー入力用）
		::hwndDlg = hwndDlg;
		wpTreeView = (WNDPROC)SetWindowLongPtr(hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)TreeViewProc);
		::SetTimer(hwndDlg, 1, 300, NULL);
		bInMove = false;

		return TRUE;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		ptdi = (TV_DISPINFO*)lParam;

		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_MAINMENU);
			return TRUE;
		case PSN_KILLACTIVE:
			// ダイアログデータの取得 Menu
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_MAINMENU;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）
			nIdxFIdx = Combo_GetCurSel(hwndComboFunkKind);
			nIdxFunc = List_GetCurSel(hwndListFunk);
			if (nIdxFIdx != CB_ERR) {
				::SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndComboFunkKind);
				if (nIdxFunc != LB_ERR) {
					List_SetCurSel(hwndListFunk, nIdxFunc);
				}
			}
			return TRUE;
		case TVN_BEGINLABELEDIT:	//	アイテムの編集開始
			if (pNMHDR->hwndFrom == hwndTreeRes) { 
				HWND hEdit = TreeView_GetEditControl(hwndTreeRes);
				if (msMenu[ptdi->item.lParam].bIsNode) {
					// ノードのみ有効
					SetWindowText(hEdit, to_tchar(msMenu[ptdi->item.lParam].sName.c_str())) ;
					EditCtl_LimitText(hEdit, MAX_MAIN_MENU_NAME_LEN);
					// 編集時のメッセージ処理
					wpEdit = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)WindowProcEdit);
				}else {
					// ノード以外編集不可
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
				}
			}
			return TRUE;
		case TVN_ENDLABELEDIT:		//	アイテムの編集が終了
 			if (pNMHDR->hwndFrom == hwndTreeRes 
			  && msMenu[ptdi->item.lParam].bIsNode) {
				// ノード有効
				pFuncWk = &msMenu[ptdi->item.lParam];
				std::wstring strNameOld = pFuncWk->sName;
				if (!ptdi->item.pszText) {
					// Esc
					//	何も設定しない（元のまま）
				}else if (auto_strcmp(ptdi->item.pszText, _T("")) == 0) {
					// 空
					pFuncWk->sName = LSW(STR_PROPCOMMAINMENU_EDIT);
				}else {
					pFuncWk->sName = to_wchar(ptdi->item.pszText);
				}
				if (strNameOld != pFuncWk->sName) {
					// ラベルを編集したらリソースからの文字列取得をやめる 2012.10.14 syat 各国語対応
					pFuncWk->nFunc = F_NODE;
				}
				ptdi->item.pszText = const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
				TreeView_SetItem(hwndTreeRes , &ptdi->item);	//	編集結果を反映

				// 編集時のメッセージ処理を戻す
				SetWindowLongPtr(TreeView_GetEditControl(hwndTreeRes), GWLP_WNDPROC, (LONG_PTR)wpEdit);
				wpEdit = nullptr;
			}
			return TRUE;
		case TVN_DELETEITEM:
			if (!bInMove && !msMenu.empty()
			  && pNMHDR->hwndFrom == hwndTreeRes
			  && (htiItem = TreeView_GetSelection(hwndTreeRes))
			) {
				// 付属情報を削除
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (TreeView_GetItem(hwndTreeRes, &tvi)) {
					msMenu.erase(tvi.lParam);
				}
				return 0;
			}
			break;
		case NM_DBLCLK:
			// ダブルクリック時の処理
			if (pNMHDR->hwndFrom == hwndTreeRes) {
				htiItem = TreeView_GetSelection(hwndTreeRes);
				if (!htiItem) {
					break;
				}
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (!TreeView_GetItem(hwndTreeRes, &tvi)) {
					break;
				}
				pFuncWk = &msMenu[tvi.lParam];
				if (pFuncWk->nFunc != F_SEPARATOR) {
					auto_sprintf_s(szKey, _T("%ls"), pFuncWk->sKey);

					if (!dlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							LS(STR_PROPCOMMAINMENU_ACCKEY1),
							LS(STR_PROPCOMMAINMENU_ACCKEY2),
							1,
							szKey)
					) {
						return TRUE;
					}
					auto_sprintf_s(pFuncWk->sKey, L"%ts", szKey);
					pFuncWk->bDupErr = false;

					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
					TreeView_SetItem(hwndTreeRes, &tvi);
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID
		hwndCtl = (HWND) lParam;		// コントロールのハンドル

		if (hwndComboFunkKind == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIdxFIdx = Combo_GetCurSel(hwndComboFunkKind);

				if (nIdxFIdx == nSpecialFuncsNum) {
					// 機能一覧に特殊機能をセット
					List_ResetContent(hwndListFunk);
					for (int i=0; i<_countof(gSpecialFuncs); ++i) {
						List_AddString(hwndListFunk, LSW(gSpecialFuncs[i].nNameId));
					}
				}else {
					// 機能一覧に文字列をセット（リストボックス）
					lookup.SetListItem(hwndListFunk, nIdxFIdx);
				}
				return TRUE;
			}
		}else {
			switch (wNotifyCode) {
			// ボタン／チェックボックスがクリックされた
			case BN_CLICKED:
				switch (wID) {
				case IDC_BUTTON_IMPORT:	// インポート
					// カスタムメニュー設定をインポートする
					Import(hwndDlg);
					return TRUE;
				case IDC_BUTTON_EXPORT:	// エクスポート
					// カスタムメニュー設定をエクスポートする
					Export(hwndDlg);
					return TRUE;

				case IDC_BUTTON_CLEAR:
					if (::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						LS(STR_PROPCOMMAINMENU_CLEAR)) == IDCANCEL
					) {
						return TRUE;
					}
					// 内部データ初期化
					msMenu.clear();
					nMenuCnt = 0;
					// TreeView初期化
					TreeView_DeleteAllItems(hwndTreeRes);
					return TRUE;

				case IDC_BUTTON_INITIALIZE:
					if (::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						LS(STR_PROPCOMMAINMENU_INIT)) == IDCANCEL
					) {
						return TRUE;
					}
					// 初期状態に戻す
					{
						DataProfile	profile;
						profile.SetReadingMode();
						profile.ReadProfileRes(MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI));

						ShareData_IO::IO_MainMenu(profile, common.mainMenu, false);
						
						SetData(hwndDlg); 
					}
					return TRUE;

				case IDC_BUTTON_DELETE:
					htiItem = TreeView_GetSelection(hwndTreeRes);
					if (htiItem) {
						if (TreeView_GetChild(hwndTreeRes, htiItem)
						  && ::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
							LS(STR_PROPCOMMAINMENU_DEL)) == IDCANCEL
						) {
							return TRUE;
						}
						htiTemp = TreeView_GetNextSibling(hwndTreeRes, htiItem);
						if (!htiTemp) {
							// 末尾ならば、前を取る
							htiTemp = TreeView_GetPrevSibling(hwndTreeRes, htiItem);
						}
						TreeView_DeleteItem(hwndTreeRes, htiItem);
						if (htiTemp) {
							TreeView_SelectItem(hwndTreeRes, htiTemp);
						}
					}
					break;


				case IDC_BUTTON_INSERT_NODE:			// ノード挿入
				case IDC_BUTTON_INSERTSEPARATOR:		// 区切線挿入
				case IDC_BUTTON_INSERT:					// 挿入(上)
				case IDC_BUTTON_INSERT_A:				// 挿入(下)
				case IDC_BUTTON_ADD:					// 追加
					eFuncCode = F_INVALID;
					bIsNode = false;
					switch (wID) {
					case IDC_BUTTON_INSERT_NODE:		// ノード挿入
						eFuncCode = F_NODE;
						bIsNode = true;
						auto_strcpy(szLabel , LSW(STR_PROPCOMMAINMENU_EDIT));
						break;
					case IDC_BUTTON_INSERTSEPARATOR:	// 区切線挿入
						eFuncCode = F_SEPARATOR;
						auto_strcpy(szLabel , LSW(STR_PROPCOMMAINMENU_SEP));
						break;
					case IDC_BUTTON_INSERT:				// 挿入
					case IDC_BUTTON_INSERT_A:			// 挿入
					case IDC_BUTTON_ADD:				// 追加
						// Function 取得
						if ((nIdxFIdx = Combo_GetCurSel(hwndComboFunkKind)) == CB_ERR) {
							return FALSE;
						}
						if ((nIdxFunc = List_GetCurSel(hwndListFunk)) == LB_ERR) {
							return FALSE;
						}
						if (nIdxFIdx == nSpecialFuncsNum) {
							// 特殊機能
							auto_strcpy(szLabel, LSW(gSpecialFuncs[nIdxFunc].nNameId));
							eFuncCode = gSpecialFuncs[nIdxFunc].nFunc;
						}else if (lookup.Pos2FuncCode(nIdxFIdx, nIdxFunc) != 0) {
							List_GetText(hwndListFunk, nIdxFunc, szLabel);
							eFuncCode = lookup.Pos2FuncCode(nIdxFIdx, nIdxFunc);
						}else {
							auto_strcpy(szLabel, L"?");
							eFuncCode = F_SEPARATOR;
						}
						break;
					}

					// 挿入位置検索
					htiTemp = TreeView_GetSelection(hwndTreeRes);
					if (!htiTemp) {
						// 取れなかったらRootの末尾
						htiParent = TVI_ROOT;
						htiTemp = TVI_LAST;
					}else {
						if (wID == IDC_BUTTON_ADD) {
							// 追加
							tvi.mask = TVIF_HANDLE | TVIF_PARAM;
							tvi.hItem = htiTemp;
							if (!TreeView_GetItem(hwndTreeRes, &tvi)) {
								// 取れなかったらRootの末尾
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}else {
								if (msMenu[tvi.lParam].bIsNode) {
									// ノード
									htiParent = htiTemp;
									htiTemp = TVI_LAST;
								}else {
									// 子を付けられないので親に付ける（選択アイテムの下に付く）
									htiParent = TreeView_GetParent(hwndTreeRes, htiTemp);
									htiTemp = TVI_LAST;
									if (!htiParent) {
										// 取れなかったらRootの末尾
										htiParent = TVI_ROOT;
									}
								}
							}
						}else if (wID == IDC_BUTTON_INSERT_NODE || wID == IDC_BUTTON_INSERT_A) {
							// ノード挿入、挿入(下)
							// 追加先を探る
							htiTemp = TreeView_GetSelection(hwndTreeRes);
							if (!htiTemp) {
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}else {
								tvi.mask = TVIF_HANDLE | TVIF_PARAM;
								tvi.hItem = htiTemp;
								if (TreeView_GetItem(hwndTreeRes, &tvi)) {
									if (msMenu[tvi.lParam].bIsNode) {
										// ノード
										htiParent = htiTemp;
										htiTemp = TVI_FIRST;
									}else {
										// 子を付けられないので親に付ける（選択アイテムの下に付く）
										htiParent = TreeView_GetParent(hwndTreeRes, htiTemp);
										if (!htiParent) {
											// 取れなかったらRoot
											htiParent = TVI_ROOT;
										}
									}
								}else {
									// 取れなかったらRoot
									htiParent = TVI_ROOT;
									htiTemp = TVI_LAST;
								}
							}
						}else {
							// 挿入(上)、区切線
							// 挿入先を探る
							htiParent = TreeView_GetParent(hwndTreeRes, htiTemp);
							if (!htiParent) {
								// 取れなかったらRootのトップ
								htiParent = TVI_ROOT;
								htiTemp = TVI_FIRST;
							}else {
								// 一つ手前
								htiTemp = TreeView_GetPrevSibling(hwndTreeRes, htiTemp);
								if (!htiTemp) {
									// 取れなかったら親の最初
									htiTemp = TVI_FIRST;
								}
							}
						}
					}

					// TreeViewに挿入
					pFuncWk = &msMenu[nMenuCnt];
					pFuncWk->nFunc = (EFunctionCode)eFuncCode;
					pFuncWk->sName = szLabel;
					pFuncWk->bDupErr = false;
					pFuncWk->bIsNode = bIsNode;
					auto_strcpy(pFuncWk->sKey, L"");
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
					tvis.hParent = htiParent;
					tvis.hInsertAfter = htiTemp;
					tvis.item.pszText = const_cast<TCHAR*>(to_tchar(szLabel));
					tvis.item.lParam = nMenuCnt++;
					tvis.item.cChildren = (wID == IDC_BUTTON_INSERT_NODE);
					htiItem = TreeView_InsertItem(hwndTreeRes, &tvis);
					// 展開
					if (htiParent != TVI_ROOT) {
						TreeView_Expand(hwndTreeRes, htiParent, TVE_EXPAND);
					}
					TreeView_SelectItem(hwndTreeRes, htiItem);

					// リストを1つ進める
					switch (wID) {
					case IDC_BUTTON_INSERT:				// 挿入
					case IDC_BUTTON_INSERT_A:			// 挿入
					case IDC_BUTTON_ADD:				// 追加
						List_SetCurSel(hwndListFunk, nIdxFunc + 1);
						break;
					}
					break;

				case IDC_BUTTON_UP:
					htiItem = TreeView_GetSelection(hwndTreeRes);
					if (!htiItem) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling(hwndTreeRes, htiItem);
					if (!htiTemp) {
						// そのエリアで最初
						break;
					}

					// コピー
					bInMove = true;
					TreeCopy(hwndTreeRes, htiItem, htiTemp, false, true);

					// 削除
					TreeView_DeleteItem(hwndTreeRes, htiTemp);
					bInMove = false;
					break;

				case IDC_BUTTON_DOWN:
					htiItem = TreeView_GetSelection(hwndTreeRes);
					if (!htiItem) {
						break;
					}
					htiTemp = TreeView_GetNextSibling(hwndTreeRes, htiItem);
					if (!htiTemp) {
						// そのエリアで最後
						break;
					}

					// コピー
					bInMove = true;
					TreeCopy(hwndTreeRes, htiTemp, htiItem, false, true);

					// 削除
					TreeView_DeleteItem(hwndTreeRes, htiItem);
					bInMove = false;

					// 選択
					htiItem = TreeView_GetNextSibling(hwndTreeRes, htiTemp);
					if (htiItem) {
						TreeView_SelectItem(hwndTreeRes, htiItem);
					}
					break;

				case IDC_BUTTON_RIGHT:
					htiItem = TreeView_GetSelection(hwndTreeRes);
					if (!htiItem) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling(hwndTreeRes, htiItem);
					if (!htiTemp) {
						// そのエリアで最初
						break;
					}
					// ノード確認
					tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
					tvi.hItem = htiTemp;
					//i = TreeView_GetItem(hwndTreeRes, &tvi);
					if (!TreeView_GetItem(hwndTreeRes, &tvi)) {
						// エラー
						break;
					}
					if (tvi.cChildren) {
						// 直前がノード
						HTREEITEM		htiTemp2;
						// コピー
						bInMove = true;
						htiTemp2 = TreeCopy(hwndTreeRes, htiTemp, htiItem, true, true);

						// 削除
						TreeView_DeleteItem(hwndTreeRes, htiItem);
						bInMove = false;

						// 選択
						TreeView_SelectItem(hwndTreeRes, htiTemp2);
					}else {
						// ノードが無い
						break;
					}
					break;

				case IDC_BUTTON_LEFT:
					htiItem = TreeView_GetSelection(hwndTreeRes);
					if (!htiItem) {
						break;
					}
					htiParent = TreeView_GetParent(hwndTreeRes, htiItem);
					if (!htiParent) {
						// Root
						break;
					}
					// コピー
					bInMove = true;
					htiTemp2 = TreeCopy(hwndTreeRes, htiParent, htiItem, false, true);

					// 削除
					TreeView_DeleteItem(hwndTreeRes, htiItem);
					bInMove = false;

					// 選択
					TreeView_SelectItem(hwndTreeRes, htiTemp2);
					break;

				case IDC_BUTTON_CHECK:		// メニューの検査
					{
						wstring sErrMsg;
						if (Check_MainMenu(hwndTreeRes, sErrMsg)) {
							InfoMessage(hwndDlg, LS(STR_PROPCOMMAINMENU_OK));
						}else {
							WarningMessage(hwndDlg, to_tchar(sErrMsg.c_str()));
						}
					}
					break;

				case IDC_BUTTON_EXPAND:		// ツリー全開
					TreeView_ExpandAll(hwndTreeRes, true);
					break;

				case IDC_BUTTON_COLLAPSE:	// ツリー全閉
					TreeView_ExpandAll(hwndTreeRes, false);
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdxMenu = TreeView_GetSelection(hwndTreeRes);
		nIdxFIdx = Combo_GetCurSel(hwndComboFunkKind);
		nIdxFunc = List_GetCurSel(hwndListFunk);
		//i = List_GetCount(hwndTreeRes);
		if (!nIdxMenu) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP),     FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN),   FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RIGHT),  FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_LEFT),   FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP),     TreeView_GetPrevSibling(hwndTreeRes, nIdxMenu) != NULL);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN),   TreeView_GetNextSibling(hwndTreeRes, nIdxMenu) != NULL);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RIGHT),  TreeView_GetPrevSibling(hwndTreeRes, nIdxMenu) != NULL);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_LEFT),   TreeView_GetParent(hwndTreeRes, nIdxMenu) != NULL);
		}
		if (0
			|| nIdxFunc == LB_ERR
			|| (1
				&& CB_ERR != nIdxFIdx
				&& LB_ERR != nIdxFunc
				&& lookup.Pos2FuncCode(nIdxFIdx, nIdxFunc) == 0
				&& nIdxFIdx != nSpecialFuncsNum
			)
		) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT_A), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), nIdxMenu != NULL);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT_A), nIdxMenu != NULL);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), TRUE);
		}
		break;
	case WM_DESTROY:
		::KillTimer(hwndDlg, 1);

		// 編集時のメッセージ処理を戻す
		SetWindowLongPtr(hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)wpTreeView);
		wpTreeView = NULL;

		// ワークのクリア
		msMenu.clear();
		nMenuCnt = 0;
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);

		return TRUE;
	}
	return FALSE;
}


// & の補完
static
wstring SupplementAmpersand(wstring sLavel)
{
	size_t nPos =0;
	while ((nPos = sLavel.find(L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos + 1] != L'&') {
			// &&でない
			sLavel.replace(nPos, 1, L"&&");
		}
		nPos +=2;
	}
	return sLavel;
}

// & の削除
static
wstring RemoveAmpersand(wstring sLavel)
{
	size_t nPos = 0;
	while ((nPos = sLavel.find(L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos + 1] == L'&') {
			// &&
			sLavel.replace(nPos, 1, L"");
		}
		nPos ++;
	}
	return sLavel;
}

// ダイアログデータの設定 MainMenu
void PropMainMenu::SetData(HWND hwndDlg)
{
	MainMenu*	pMenuTBL = common.mainMenu.mainMenuTbl;
	MainMenu*	pFunc;
	HWND		hwndCombo;
	HWND		hwndCheck;
	HWND		hwndTreeRes;
	const int	MAX_LABEL_CCH = 256 + 10;
	WCHAR		szLabel[MAX_LABEL_CCH];
	int			nCurLevel;
	HTREEITEM	htiItem;
	HTREEITEM	htiParent;
	TV_INSERTSTRUCT	tvis;			// 挿入用
	MainMenuWork*	pFuncWk;		// 機能(work)

	// 機能種別一覧に文字列をセット（コンボボックス）
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
	lookup.SetCategory2Combo(hwndCombo);

	// 特別機能追加
	nSpecialFuncsNum = Combo_AddString(hwndCombo, LS(STR_SPECIAL_FUNC));

	// 種別の先頭の項目を選択（コンボボックス）
	Combo_SetCurSel(hwndCombo, 0);

	// ワーク、TreeViewの初期化
	msMenu.clear();
	nMenuCnt = 0;

	hwndTreeRes = ::GetDlgItem(hwndDlg, IDC_TREE_RES);
	TreeView_DeleteAllItems(hwndTreeRes);

	// アクセスキーを()付で表示
	hwndCheck = ::GetDlgItem(hwndDlg, IDC_CHECK_KEY_PARENTHESES);
	BtnCtl_SetCheck(hwndCheck, common.mainMenu.bMainMenuKeyParentheses);

	// メニュー項目一覧と内部データをセット（TreeView）
	nCurLevel = 0;
	htiParent = TVI_ROOT;
	htiItem = TreeView_GetRoot(hwndTreeRes);
	for (int i=0; i<common.mainMenu.nMainMenuNum; ++i) {
		pFunc = &pMenuTBL[i];
		if (pFunc->nLevel < nCurLevel) {
			// Level Up
			for (; pFunc->nLevel<nCurLevel; --nCurLevel) {
				htiParent = (htiParent == TVI_ROOT) ? TVI_ROOT : TreeView_GetParent(hwndTreeRes, htiParent);
				if (!htiParent)		htiParent = TVI_ROOT;
			}
		}else if (pFunc->nLevel > nCurLevel) {
			// Level Down
			for (htiParent=htiItem, ++nCurLevel; pFunc->nLevel<nCurLevel; ++nCurLevel) {
				// 実行されることは無いはず（データが正常ならば）
				htiParent = TreeView_GetChild(hwndTreeRes, htiItem);
				if (!htiParent) {
					htiParent = htiItem;
				}
			}
		}

		// 内部データを作成
		pFuncWk = &msMenu[nMenuCnt];
		pFuncWk->nFunc = pFunc->nFunc;
		pFuncWk->bIsNode = false;
		switch (pFunc->type) {
		case MainMenuType::Leaf:
			lookup.Funccode2Name(pFunc->nFunc, szLabel, MAX_MAIN_MENU_NAME_LEN);
			pFuncWk->sName = szLabel;
			break;
		case MainMenuType::Separator:
			pFuncWk->sName = LSW(STR_PROPCOMMAINMENU_SEP);
			break;
		case MainMenuType::Special:
			pFuncWk->sName = pFunc->sName;
			if (pFuncWk->sName.empty()) {
				for (int j=0; j<_countof(gSpecialFuncs); ++j) {
					if (pFunc->nFunc == gSpecialFuncs[j].nFunc) {
						pFuncWk->sName = RemoveAmpersand(LSW(gSpecialFuncs[j].nNameId));
						break;
					}
				}
			}
			break;
		case MainMenuType::Node:
			pFuncWk->bIsNode = true;
			// ラベル編集後のノードはiniから、それ以外はリソースからラベルを取得 2012.10.14 syat 各国語対応
			if (pFuncWk->nFunc == F_NODE) {
				pFuncWk->sName = RemoveAmpersand(pFunc->sName);
			}else {
				pFuncWk->sName = LSW(pFuncWk->nFunc);
			}
			break;
		}
		auto_strcpy(pFuncWk->sKey, pFunc->sKey);
		pFuncWk->bDupErr = false;
		// TreeViewに挿入
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.hParent = htiParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.pszText = const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
		tvis.item.lParam = nMenuCnt++;								// 内部データインデックスのインクリメント
		tvis.item.cChildren = (pFunc->type == MainMenuType::Node);
		htiItem = TreeView_InsertItem(hwndTreeRes, &tvis);
	}
}

// ダイアログデータの取得 MainMenu
int PropMainMenu::GetData(HWND hwndDlg)
{
	HWND		hwndTreeRes;
	HWND		hwndCheck;
	HTREEITEM	htiItem;

	// アクセスキーを()付で表示
	hwndCheck = ::GetDlgItem(hwndDlg, IDC_CHECK_KEY_PARENTHESES);
	common.mainMenu.bMainMenuKeyParentheses = (BtnCtl_GetCheck(hwndCheck) != 0);

	// メニュートップ項目をセット
	common.mainMenu.nMainMenuNum = 0;
	memset(common.mainMenu.nMenuTopIdx, -1, sizeof(common.mainMenu.nMenuTopIdx));

	hwndTreeRes = ::GetDlgItem(hwndDlg, IDC_TREE_RES);

	htiItem = TreeView_GetRoot(hwndTreeRes);
	GetDataTree(hwndTreeRes, htiItem, 0);

	return TRUE;
}

// ダイアログデータの取得 TreeViewの 1 level
bool PropMainMenu::GetDataTree(
	HWND hwndTree,
	HTREEITEM htiTrg,
	int nLevel
	)
{
	static	bool	bOptionOk;
	MainMenu*		pMenuTbl = common.mainMenu.mainMenuTbl;
	MainMenu*		pFunc;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;			// 取得用
	MainMenuWork*	pFuncWk;		// 機能(work)
	int 			nTopCount = 0;

	if (nLevel == 0) {
		// 共通設定フラグ
		bOptionOk = false;
	}

	for (s=htiTrg; s; s=TreeView_GetNextSibling(hwndTree, s)) {
		if (common.mainMenu.nMainMenuNum >= MAX_MAINMENU) {
			// 登録数 over
			return false;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (!TreeView_GetItem(hwndTree, &tvi)) {
			// Error
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];

		if (nLevel == 0) {
			if (nTopCount >= MAX_MAINMENU_TOP) {
				continue;
			}
			// Top Levelの記録
			common.mainMenu.nMenuTopIdx[nTopCount++] = common.mainMenu.nMainMenuNum;
		}
		pFunc = &pMenuTbl[common.mainMenu.nMainMenuNum++];

		switch (pFuncWk->nFunc) {
		case F_NODE:
			pFunc->type = MainMenuType::Node;
			auto_strcpy_s(pFunc->sName, MAX_MAIN_MENU_NAME_LEN + 1, SupplementAmpersand(pFuncWk->sName).c_str());
			break;
		case F_SEPARATOR:
			pFunc->type = MainMenuType::Separator;
			pFunc->sName[0] = L'\0';
			break;
		default:
			if (pFuncWk->bIsNode) {
				// コマンド定義外のIDの場合、ノードとして扱う 2012.10.14 syat 各国語対応
				pFunc->type = MainMenuType::Node;
				pFunc->sName[0] = L'\0';	// 名前は、リソースから取得するため空白に設定
				break;
			}
			if (pFuncWk->nFunc >= F_SPECIAL_FIRST && pFuncWk->nFunc <= F_SPECIAL_LAST) {
				pFunc->type = MainMenuType::Special;
				// 2014.05.04 nLevel == 0 のときも"名前なし"にする
					pFunc->sName[0] = L'\0';
			}else {
				if (pFuncWk->nFunc == F_OPTION) {
					bOptionOk = true;
				}
				pFunc->type = MainMenuType::Leaf;
				pFunc->sName[0] = L'\0';
			}
			break;
		}
		pFunc->nFunc = pFuncWk->nFunc;
		auto_strcpy(pFunc->sKey, pFuncWk->sKey);
		pFunc->nLevel = nLevel;

		if (tvi.cChildren) {
			ts = TreeView_GetChild(hwndTree, s);	//	子の取得
			if (ts) {
				if (!GetDataTree(hwndTree, ts, nLevel + 1)) {
					return false;
				}
			}
		}
	}
	if (nLevel == 0 && !bOptionOk) {
		// 共通設定が無い
		if (nTopCount < MAX_MAINMENU_TOP && common.mainMenu.nMainMenuNum + 1 < MAX_MAINMENU) {
			// Top Levelの記録
			common.mainMenu.nMenuTopIdx[nTopCount++] = common.mainMenu.nMainMenuNum;
			// Top Levelの追加（ダミー）
			pFunc = &pMenuTbl[common.mainMenu.nMainMenuNum++];
			pFunc->type = MainMenuType::Node;
			pFunc->nFunc = F_NODE;
			auto_strcpy(pFunc->sName, L"auto_add");
			pFunc->sKey[0] = L'\0';
			pFunc->nLevel = nLevel++;
		}else {
			// 末尾に追加を指定
			nLevel = 1;
		}
		if (common.mainMenu.nMainMenuNum < MAX_MAINMENU) {
			// 共通設定
			pFunc = &pMenuTbl[common.mainMenu.nMainMenuNum++];
			pFunc->type = MainMenuType::Leaf;
			pFunc->nFunc = F_OPTION;
			pFunc->sName[0] = L'\0';
			pFunc->sKey[0] = L'\0';
			pFunc->nLevel = nLevel;
		}else {
			// 登録数 over
			return false;
		}
	}
	return true;
}

// メインメニュー設定をインポートする
void PropMainMenu::Import(HWND hwndDlg)
{
	ImpExpMainMenu	impExp(common);

	// インポート
	if (!impExp.ImportUI(G_AppInstance(), hwndDlg)) {
		// インポートをしていない
		return;
	}
	SetData(hwndDlg);
}

// メインメニュー設定をエクスポートする
void PropMainMenu::Export(HWND hwndDlg)
{
	ImpExpMainMenu impExp(common);
	GetData(hwndDlg);

	// エクスポート
	if (!impExp.ExportUI(G_AppInstance(), hwndDlg)) {
		// エクスポートをしていない
		return;
	}
}


// ツリーのコピー
//		fChildがtrueの時はdstの子としてコピー, そうでなければdstの兄弟としてdstの後ろにコピー
//		fOnryOneがtrueの時は1つだけコピー（子があったらコピー）
static
HTREEITEM TreeCopy(
	HWND hwndTree,
	HTREEITEM dst,
	HTREEITEM src,
	bool fChild,
	bool fOnryOne
	)
{
	HTREEITEM		s;
	HTREEITEM		ts;
	HTREEITEM		td = NULL;
	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	int				n = 0;
#ifdef _UNICODE
	const int		MAX_LABEL_CCH = 256+10;
#else
	const int		MAX_LABEL_CCH = (256+10)*2;
#endif
	TCHAR			szLabel[MAX_LABEL_CCH];

	for (s=src; s; s = fOnryOne ? NULL:TreeView_GetNextSibling(hwndTree, s)) {
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		tvi.pszText = szLabel;
		tvi.cchTextMax = MAX_LABEL_CCH;
		if (!TreeView_GetItem(hwndTree, &tvi)) {
			// Error
			break;
		}
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		if (fChild || n != 0) {
			// dstの子供として作成
			tvis.hParent = dst;
			tvis.hInsertAfter = TVI_LAST;
		}else {
			//	dstの兄弟として作成
			tvis.hParent = TreeView_GetParent(hwndTree, dst);
			tvis.hInsertAfter = dst;
		}
		tvis.item.pszText = szLabel;
		tvis.item.lParam = tvi.lParam;
		tvis.item.cChildren = tvi.cChildren;
		td = TreeView_InsertItem(hwndTree, &tvis);	//	Itemの作成

		if (tvi.cChildren) {
			ts = TreeView_GetChild(hwndTree, s);	//	子の取得
			if (ts) {
				TreeCopy(hwndTree, td, ts, true, false);
			}
			// 展開
			if (tvi.state & TVIS_EXPANDEDONCE) {
				TreeView_Expand(hwndTree, td, TVE_EXPAND);
			}
		}
		++n;
	}
	return td;
}

// TreeView 全開･全閉
static void TreeView_ExpandAll(HWND hwndTree, bool bExpand)
{
	std::map<int, HTREEITEM>	htiStack;
	HTREEITEM	htiCur;
	HTREEITEM	htiItem;
	HTREEITEM	htiNext;
	int			nLevel;

	nLevel = 0;
	htiCur = htiItem = TreeView_GetSelection(hwndTree);
	if (!bExpand && htiCur) {
		// 閉じる時はトップに変更
		for (htiNext=htiCur; htiNext!=NULL; ) {
			htiItem = htiNext;
			htiNext = TreeView_GetParent(hwndTree, htiItem);
		}
		if (htiCur != htiItem) {
			htiCur = htiItem;
			TreeView_SelectItem(hwndTree, htiCur);
		}
	}


	for (htiItem=TreeView_GetRoot(hwndTree); htiItem; htiItem=htiNext) {
		htiNext = TreeView_GetChild(hwndTree, htiItem);
		if (htiNext) {
			TreeView_Expand(hwndTree, htiItem, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
			// 子の開閉
			htiStack[nLevel++] = htiItem;
		}else {
			htiNext = TreeView_GetNextSibling(hwndTree, htiItem);
			while (!htiNext && nLevel > 0) {
				htiItem = htiStack[--nLevel];
				htiNext = TreeView_GetNextSibling(hwndTree, htiItem);
			}
		}
	}
	// 選択位置を戻す
	if (!htiCur) {
		if (bExpand) {
			htiItem = TreeView_GetRoot(hwndTree);
			TreeView_SelectSetFirstVisible(hwndTree, htiItem);
		}
		TreeView_SelectItem(hwndTree, NULL);
	}else {
		TreeView_SelectSetFirstVisible(hwndTree, htiCur);
	}
}


// 表示用データの作成（アクセスキー付加）
static const TCHAR* MakeDispLabel(MainMenuWork* pFunc)
{
	static WCHAR szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	if (pFunc->sKey[0]) {
		auto_sprintf_s(szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls(%ls)",
			pFunc->bDupErr ? L">" : L"",
			pFunc->sName.substr(0, MAX_MAIN_MENU_NAME_LEN).c_str(), pFunc->sKey);
	}else {
		auto_sprintf_s(szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls",
			pFunc->bDupErr ? L">" : L"",
			pFunc->sName.substr(0, MAX_MAIN_MENU_NAME_LEN).c_str() );
	}
	return to_tchar(szLabel);
}


// メニューの検査
bool PropMainMenu::Check_MainMenu(
	HWND		hwndTree,		// handle to TreeView
	wstring&	sErrMsg			// エラーメッセージ
	)
{
	HTREEITEM		htiItem;
	
	sErrMsg = L"";
	
	htiItem = TreeView_GetRoot(hwndTree);

	bool bRet = Check_MainMenu_Sub( hwndTree, htiItem, 0, sErrMsg );
	return bRet;
}

// メニューの検査 TreeViewの 1 level
bool PropMainMenu::Check_MainMenu_Sub(
	HWND		hwndTree,		// handle to dialog box
	HTREEITEM 	htiTrg,			// ターゲット
	int 		nLevel,
	wstring&	sErrMsg
	)
{
	// 検査用
	static	bool		bOptionOk;		// 「共通設定」
	static	int 		nMenuNum;		// メニュー項目数		最大 MAX_MAINMENU
	static	int 		nTopNum;		// トップレベル項目数	最大 MAX_MAINMENU_TOP
	static	int 		nDupErrNum;		// 重複エラー個数
	static	int 		nNoSetErrNum;	// 未設定エラー個数
	static	HTREEITEM	htiErr;
	//
	bool			bRet = true;
	MainMenuType	nType;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;							// 取得用
	MainMenuWork*	pFuncWk;						// 機能(work)
	std::map< WCHAR, HTREEITEM >	mKey;			// 重複エラー検出用

	if (nLevel == 0) {
		bOptionOk = false;
		nMenuNum = nTopNum = nDupErrNum = nNoSetErrNum = 0;
		htiErr = NULL;
	}
	mKey.clear();

	for (s=htiTrg; s; s=TreeView_GetNextSibling(hwndTree, s)) {
		// メニュー数のカウント
		++nMenuNum;
		if (nLevel == 0) {
			++nTopNum;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (!TreeView_GetItem(hwndTree, &tvi)) {
			// Error
			sErrMsg = LSW(STR_PROPCOMMAINMENU_ERR1);
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];
		switch (pFuncWk->nFunc) {
		case F_NODE:
			nType = MainMenuType::Node;
			break;
		case F_SEPARATOR:
			nType = MainMenuType::Separator;
			break;
		default:
			if (pFuncWk->nFunc >= F_SPECIAL_FIRST && pFuncWk->nFunc <= F_SPECIAL_LAST) {
				nType = MainMenuType::Special;
			}else if (pFuncWk->bIsNode) {
				nType = MainMenuType::Node;
			}else {
				if (pFuncWk->nFunc == F_OPTION) {
					bOptionOk = true;
				}
				nType = MainMenuType::Leaf;
			}
			break;
		}
		if (pFuncWk->sKey[0] == '\0') {
			if (nType == MainMenuType::Node || nType == MainMenuType::Leaf) {
				// 未設定
				if (nNoSetErrNum == 0) {
					if (!htiErr) {
						htiErr = s;
					}
				}
				TreeView_SelectItem(hwndTree, s);
				++nNoSetErrNum;
			}
		}else {
			auto itKey = mKey.find(pFuncWk->sKey[0]);
			if (itKey == mKey.end()) {
				mKey[pFuncWk->sKey[0]] = s;

				if (pFuncWk->bDupErr) {
					// 目印クリア
					pFuncWk->bDupErr = false;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText =  const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
					TreeView_SetItem(hwndTree , &tvi);		//	キー設定結果を反映
				}
			}else {
				// 重複エラー
				if (nDupErrNum == 0) {
					if (!htiErr) {
						htiErr = mKey[pFuncWk->sKey[0]];
					}
				}
				TreeView_SelectItem(hwndTree, mKey[pFuncWk->sKey[0]]);

				++nDupErrNum;

				// 目印設定
				pFuncWk->bDupErr = true;
				tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
				tvi.pszText = const_cast<TCHAR*>(MakeDispLabel(pFuncWk));
				TreeView_SetItem(hwndTree , &tvi);		//	キー設定結果を反映

				// 目印設定（元分）
				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
				tvi.hItem = mKey[pFuncWk->sKey[0]];
				if (!TreeView_GetItem(hwndTree, &tvi)) {
					// Error
					sErrMsg = LSW(STR_PROPCOMMAINMENU_ERR1);
					return false;
				}
				if (!msMenu[tvi.lParam].bDupErr) {
					msMenu[tvi.lParam].bDupErr = true;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>(MakeDispLabel(&msMenu[tvi.lParam]));
					TreeView_SetItem(hwndTree , &tvi);		//	キー設定結果を反映
				}
			}
		}
		if (tvi.cChildren) {
			ts = TreeView_GetChild(hwndTree, s);	//	子の取得
			if (ts) {
				if (!Check_MainMenu_Sub(hwndTree, ts, nLevel + 1, sErrMsg)) {
					// 内部エラー
					return false;
				}
			}
		}
	}

	if (nLevel == 0) {
		sErrMsg = L"";
		if (!bOptionOk) {
			sErrMsg += LSW(STR_PROPCOMMAINMENU_ERR2);
			bRet = false;
		}
		if (nTopNum > MAX_MAINMENU_TOP) {
			sErrMsg += LSW(STR_PROPCOMMAINMENU_ERR3);
			bRet = false;
		}
		if (nMenuNum > MAX_MAINMENU) {
			sErrMsg += LSW(STR_PROPCOMMAINMENU_ERR4);
			bRet = false;
		}
		if (nDupErrNum > 0) {
			sErrMsg += LSW(STR_PROPCOMMAINMENU_ERR5);
			bRet = false;
		}
		if (nNoSetErrNum > 0) {
			sErrMsg += LSW(STR_PROPCOMMAINMENU_ERR6);
			bRet = false;
		}
		if (htiErr) {
			TreeView_SelectItem(hwndTree, htiErr);
			TreeView_SelectSetFirstVisible(hwndTree, htiErr);
		}
	}
	return bRet;
}

