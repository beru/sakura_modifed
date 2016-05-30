/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <CdErr.h>
#include <Dlgs.h>
#include "dlg/DlgOpenFile.h"
#include "func/Funccode.h"	// Stonee, 2001/05/18
#include "FileExt.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "EditApp.h"
#include "charset/CodePage.h"
#include "doc/DocListener.h"
#include "recent/Recent.h"
#include "_os/OsVersionInfo.h"
#include "dlg/Dialog.h"
#include "util/window.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/os.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// オープンファイル CDlgOpenFile.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,					// Winのヘルプで勝手に出てくる
//	IDCANCEL,				HIDCANCEL_OPENDLG,				// Winのヘルプで勝手に出てくる
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		// ヘルプボタン
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		// 文字コードセット
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			// 最近のファイル
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	// 最近のフォルダ
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			// 改行コード
	IDC_CHECK_BOM,			HIDC_OPENDLG_CHECK_BOM,			// BOM	// 2006.08.06 ryoji
	IDC_CHECK_CP,			HIDC_OPENDLG_CHECK_CP,			//CP
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

// 2005.10.29 ryoji
// Windows 2000 version of OPENFILENAME.
// The new version has three extra members.
// See CommDlg.h
#if (_WIN32_WINNT >= 0x0500)
struct OPENFILENAMEZ : public OPENFILENAME {
};
#else
struct OPENFILENAMEZ : public OPENFILENAME {
	void*	pvReserved;
	DWORD	dwReserved;
	DWORD	FlagsEx;
};
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif // (_WIN32_WINNT >= 0x0500)

#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

static int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit);

// 2014.05.22 Moca FileDialogの再入サポート
class DlgOpenFileMem {
public:
	HINSTANCE		hInstance;	// アプリケーションインスタンスのハンドル
	HWND			hwndParent;	// オーナーウィンドウのハンドル

	DllSharedData*	pShareData;

	SFilePath		szDefaultWildCard;	// 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される）
	SFilePath		szInitialDir;			// 「開く」での初期ディレクトリ

	std::vector<LPCTSTR>	vMRU;
	std::vector<LPCTSTR>	vOPENFOLDER;
};

class DlgOpenFileData {
public:
	DlgOpenFile*	pDlgOpenFile;

	WNDPROC			wpOpenDialogProc;
	int				nHelpTopicID;
	bool			bViewMode;		// ビューモードか
	bool			bIsSaveDialog;	// 保存のダイアログか
	EncodingType	nCharCode;		// 文字コード

	Eol				eol;
	bool			bUseCharCode;
	bool			bUseEol;
	bool			bBom;		// BOMを付けるかどうか	//	Jul. 26, 2003 ryoji BOM
	bool			bUseBom;	// BOMの有無を選択する機能を利用するかどうか
	SFilePath		szPath;	// 拡張子の補完を自前で行ったときのファイルパス	// 2006.11.10 ryoji

	bool			bInitCodePage;

	ComboBoxItemDeleter		combDelFile;
	RecentFile				recentFile;
	ComboBoxItemDeleter		combDelFolder;
	RecentFolder			recentFolder;

	OPENFILENAME*	pOf;
	OPENFILENAMEZ	ofn;		// 2005.10.29 ryoji OPENFILENAMEZ「ファイルを開く」ダイアログ用構造体
	HWND			hwndOpenDlg;
	HWND			hwndComboMRU;
	HWND			hwndComboOPENFOLDER;
	HWND			hwndComboCODES;
	HWND			hwndComboEOL;	//	Feb. 9, 2001 genta
	HWND			hwndCheckBOM;	//	Jul. 26, 2003 ryoji BOMチェックボックス

	DlgOpenFileData()
		:
		pDlgOpenFile(nullptr),
		nHelpTopicID(0)
	{}
};

static const TCHAR* s_pszOpenFileDataName = _T("FileOpenData");


/*
|| 	開くダイアログのサブクラスプロシージャ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
LRESULT APIENTRY OFNHookProcMain(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	OFNOTIFY*				pofn;
	DlgOpenFileData* pData = (DlgOpenFileData*)::GetProp(hwnd, s_pszOpenFileDataName);
	WORD wNotifyCode;
	WORD wID;
	static DllSharedData* pShareData;
	switch (uMsg) {
	case WM_MOVE:
		//「開く」ダイアログのサイズと位置
		pShareData = &GetDllShareData();
		::GetWindowRect(hwnd, &pShareData->common.others.rcOpenDialog);
//		MYTRACE(_T("WM_MOVE 1\n"));
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		switch (wNotifyCode) {
//			break;
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case pshHelp:
				// ヘルプ
				MyWinHelp(hwnd, HELP_CONTEXT, pData->nHelpTopicID);	// 2006.10.10 ryoji MyWinHelpに変更に変更
				break;
			case chx1:	// The read-only check box
				pData->bViewMode = DlgButton_IsChecked(hwnd , chx1);
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
//		pofn = (OFNOTIFY*) lParam;
//		MYTRACE(_T("=========WM_NOTIFY=========\n"));
//		MYTRACE(_T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom);
//		MYTRACE(_T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom);
//		MYTRACE(_T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code);
		break;
	}
//	return ::CallWindowProc((int (__stdcall *)(void))(WNDPROC)wpOpenDialogProc, hwnd, uMsg, wParam, lParam);

	return ::CallWindowProc(pData->wpOpenDialogProc, hwnd, uMsg, wParam, lParam);
}


/*!
	開くダイアログのフックプロシージャ
*/
// Modified by KEITA for WIN64 2003.9.6
// APIENTRY -> CALLBACK Moca 2003.09.09
//UINT APIENTRY OFNHookProc(
UINT_PTR CALLBACK OFNHookProc(
	HWND hdlg,		// handle to child dialog window
	UINT uiMsg,		// message identifier
	WPARAM wParam,	// message parameter
	LPARAM lParam 	// message parameter
	)
{
	POINT		po;
	RECT		rc;
	size_t		i;
	OFNOTIFY*	pofn;
	LRESULT		lRes;
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	LONG_PTR	nIdx;
	LONG_PTR	nIdxSel;
	int			nWidth;
	WPARAM		fCheck;	// Jul. 26, 2003 ryoji BOM状態用

	// From Here	Feb. 9, 2001 genta
	static const EolType nEolValueArr[] = {
		EolType::None,
		EolType::CRLF,
		EolType::LF,
		EolType::CR,
	};
	// 文字列はResource内に入れる
	static const TCHAR*	const	pEolNameArr[] = {
		_T("変換なし"), // ダミー
		_T("CR+LF"),
		_T("LF (UNIX)"),
		_T("CR (Mac)"),
	};

// To Here	Feb. 9, 2001 genta
	int	nRightMargin = 24;

	switch (uiMsg) {
	case WM_MOVE:
//		MYTRACE(_T("WM_MOVE 2\n"));
		break;
	case WM_SIZE:
		{
			nWidth = LOWORD(lParam);	// width of client area
			//「開く」ダイアログのサイズと位置
			DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			HWND hwndFrame = ::GetParent(hdlg);
			::GetWindowRect(hwndFrame, &pData->pDlgOpenFile->mem->pShareData->common.others.rcOpenDialog);
			// 2005.10.29 ryoji 最近のファイル／フォルダ コンボの右端を子ダイアログの右端に合わせる
			::GetWindowRect(pData->hwndComboMRU, &rc);
			po.x = rc.left;
			po.y = rc.top;
			::ScreenToClient(hdlg, &po);
			::SetWindowPos(pData->hwndComboMRU, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			::SetWindowPos(pData->hwndComboOPENFOLDER, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			return 0;
		}
	case WM_INITDIALOG:
		{
			// Save off the long pointer to the OPENFILENAME structure.
			// Modified by KEITA for WIN64 2003.9.6
			OPENFILENAME* pOfn = (OPENFILENAME*)lParam;
			DlgOpenFileData* pData = reinterpret_cast<DlgOpenFileData*>(pOfn->lCustData);
			::SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pData);
			pData->pOf = pOfn;

			// Explorerスタイルの「開く」ダイアログのハンドル
			pData->hwndOpenDlg = ::GetParent( hdlg );
			// コントロールのハンドル
			pData->hwndComboCODES = ::GetDlgItem(hdlg, IDC_COMBO_CODE);
			pData->hwndComboMRU = ::GetDlgItem(hdlg, IDC_COMBO_MRU);
			pData->hwndComboOPENFOLDER = ::GetDlgItem(hdlg, IDC_COMBO_OPENFOLDER);
			pData->hwndComboEOL = ::GetDlgItem(hdlg, IDC_COMBO_EOL);
			pData->hwndCheckBOM = ::GetDlgItem(hdlg, IDC_CHECK_BOM);//	Jul. 26, 2003 ryoji BOMチェックボックス
			pData->bInitCodePage = false;

			// 2005.11.02 ryoji 初期レイアウト設定
			DlgOpenFile::InitLayout( pData->hwndOpenDlg, hdlg, pData->hwndComboCODES );

			// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
			Combo_SetExtendedUI(pData->hwndComboCODES, TRUE);
			Combo_SetExtendedUI(pData->hwndComboMRU, TRUE);
			Combo_SetExtendedUI(pData->hwndComboOPENFOLDER, TRUE);
			Combo_SetExtendedUI(pData->hwndComboEOL, TRUE);

			// From Here Feb. 9, 2001 genta
			// 改行コードの選択コンボボックス初期化
			// 必要なときのみ利用する
			if (pData->bUseEol) {
				// 値の設定
				// 2013.05.27 初期値をSaveInfoから設定する
				nIdxSel = 0;
				for (i=0; i<_countof(pEolNameArr); ++i) {
					if (i == 0) {
						nIdx = Combo_AddString(pData->hwndComboEOL, LS(STR_DLGOPNFL1));
					}else {
						nIdx = Combo_AddString(pData->hwndComboEOL, pEolNameArr[i]);
					}
					Combo_SetItemData(pData->hwndComboEOL, nIdx, (int)nEolValueArr[i]);
					if (nEolValueArr[i] == pData->eol) {
						nIdxSel = nIdx;
					}
				}
				Combo_SetCurSel(pData->hwndComboEOL, nIdxSel);
			}else {
				// 使わないときは隠す
				::ShowWindow(::GetDlgItem(hdlg, IDC_STATIC_EOL), SW_HIDE);
				::ShowWindow(pData->hwndComboEOL, SW_HIDE);
			}
			// To Here Feb. 9, 2001 genta

			// From Here Jul. 26, 2003 ryoji BOMチェックボックスの初期化
			if (pData->bUseBom) {
				// 使うときは有効／無効を切り替え、チェック状態を初期値に設定する
				if (CodeTypeName(pData->nCharCode).UseBom()) {
					::EnableWindow(pData->hwndCheckBOM, TRUE);
					fCheck = pData->bBom? BST_CHECKED: BST_UNCHECKED;
				}else {
					::EnableWindow(pData->hwndCheckBOM, FALSE);
					fCheck = BST_UNCHECKED;
				}
				BtnCtl_SetCheck(pData->hwndCheckBOM, fCheck);
			}else {
				// 使わないときは隠す
				::ShowWindow(pData->hwndCheckBOM, SW_HIDE);
			}
			// To Here Jul. 26, 2003 ryoji BOMチェックボックスの初期化

			// Explorerスタイルの「開く」ダイアログをフック
			::SetProp(pData->hwndOpenDlg, s_pszOpenFileDataName, (HANDLE)pData);
			// Modified by KEITA for WIN64 2003.9.6
			pData->wpOpenDialogProc = (WNDPROC) ::SetWindowLongPtr(pData->hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR) OFNHookProcMain);

			// 文字コード選択コンボボックス初期化
			nIdxSel = -1;
			if (pData->bIsSaveDialog) {	// 保存のダイアログか
				i = 1; // 「自動選択」飛ばし
			}else {
				i = 0;
			}
			CodeTypesForCombobox codeTypes;
			for (/*i = 0*/; i<codeTypes.GetCount(); ++i) {
				nIdx = Combo_AddString(pData->hwndComboCODES, codeTypes.GetName(i));
				Combo_SetItemData(pData->hwndComboCODES, nIdx, codeTypes.GetCode(i));
				if (codeTypes.GetCode(i) == pData->nCharCode) {
					nIdxSel = nIdx;
				}
			}
			if (nIdxSel != -1) {
				Combo_SetCurSel(pData->hwndComboCODES, nIdxSel);
			}else {
				CheckDlgButtonBool( hdlg, IDC_CHECK_CP, true );
				if (AddComboCodePages(hdlg, pData->hwndComboCODES, pData->nCharCode, pData->bInitCodePage) == -1) {
					Combo_SetCurSel(pData->hwndComboCODES, 0);
				}
			}
			if (!pData->bUseCharCode) {
				::ShowWindow(GetDlgItem(hdlg, IDC_STATIC_CHARCODE), SW_HIDE);
				::ShowWindow(pData->hwndComboCODES, SW_HIDE);
			}

			// ビューモードの初期値セット
			::CheckDlgButton(pData->hwndOpenDlg, chx1, pData->bViewMode);
			pData->combDelFile = ComboBoxItemDeleter();
			pData->combDelFile.pRecent = &pData->recentFile;
			Dialog::SetComboBoxDeleter(pData->hwndComboMRU, &pData->combDelFile);
			pData->combDelFolder = ComboBoxItemDeleter();
			pData->combDelFolder.pRecent = &pData->recentFolder;
			Dialog::SetComboBoxDeleter(pData->hwndComboOPENFOLDER, &pData->combDelFolder);
		}
		break;

	case WM_DESTROY:
		// フック解除
		{
			DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr(pData->hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR)pData->wpOpenDialogProc);
			::RemoveProp(pData->hwndOpenDlg, s_pszOpenFileDataName);
		}
		return FALSE;

	case WM_NOTIFY:
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE(_T("=========WM_NOTIFY=========\n"));
//		MYTRACE(_T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom);
//		MYTRACE(_T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom);
//		MYTRACE(_T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code);

		switch (pofn->hdr.code) {
		case CDN_FILEOK:
			// 拡張子の補完を自前で行う	// 2006.11.10 ryoji
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				if (pData->bIsSaveDialog) {
					TCHAR szDefExt[_MAX_EXT];	// 補完する拡張子
					TCHAR szBuf[_MAX_PATH + _MAX_EXT];	// ワーク
					LPTSTR pszCur, pszNext;
					int i;
					CommDlg_OpenSave_GetSpec(pData->hwndOpenDlg, szBuf, _MAX_PATH);	// ファイル名入力ボックス内の文字列
					pszCur = szBuf;
					while (*pszCur == _T(' ')) {	// 空白を読み飛ばす
						pszCur = ::CharNext(pszCur);
					}
					if (*pszCur == _T('\"')) {	// 二重引用部で始まっている
						::lstrcpyn(pData->szPath, pData->pOf->lpstrFile, _MAX_PATH);
					}else {
						_tsplitpath( pData->pOf->lpstrFile, NULL, NULL, NULL, szDefExt );
						if (szDefExt[0] == _T('.') /* && szDefExt[1] != _T('\0') */) {	// 既に拡張子がついている	2文字目のチェックの削除	2008/6/14 Uchi
						// .のみの場合にも拡張子付きとみなす。
							lstrcpyn(pData->szPath, pData->pOf->lpstrFile, _MAX_PATH);
						}else {
							switch (pData->pOf->nFilterIndex) {	// 選択されているファイルの種類
							case 1:		// ユーザー定義
								pszCur = pData->pDlgOpenFile->mem->szDefaultWildCard;
								while (*pszCur != _T('.') && *pszCur != _T('\0')) {	// '.'まで読み飛ばす
									pszCur = ::CharNext(pszCur);
								}
								i = 0;
								while (*pszCur != _T(';') && *pszCur != _T('\0')) {	// ';'までコピーする
									pszNext = ::CharNext(pszCur);
									while (pszCur < pszNext) {
										szDefExt[i++] = *pszCur++;
									}
								}
								szDefExt[i] = _T('\0');
								if (::_tcslen(szDefExt) < 2 || szDefExt[1] == _T('*')) {	// 無効な拡張子?
									szDefExt[0] = _T('\0');
								}
								break;
							case 2:		// *.txt
								::_tcscpy(szDefExt, _T(".txt"));
								break;
							case 3:		// *.*
							default:	// 不明
								szDefExt[0] = _T('\0');
								break;
							}
							lstrcpyn(szBuf, pData->pOf->lpstrFile, _MAX_PATH + 1);
							::_tcscat(szBuf, szDefExt);
							lstrcpyn(pData->szPath, szBuf, _MAX_PATH);
						}
					}

					// ファイルの上書き確認を自前で行う	// 2006.11.10 ryoji
					if (IsFileExists(pData->szPath, true)) {
						TCHAR szText[_MAX_PATH + 100];
						lstrcpyn(szText, pData->szPath, _MAX_PATH);
						::_tcscat(szText, LS(STR_DLGOPNFL2));
						if (::MessageBox(pData->hwndOpenDlg, szText, LS(STR_DLGOPNFL3), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) {
							::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
					}
				}

				// 文字コード選択コンボボックス 値を取得
				nIdx = Combo_GetCurSel(pData->hwndComboCODES);
				lRes = Combo_GetItemData(pData->hwndComboCODES, nIdx);
				pData->nCharCode = (EncodingType)lRes;	// 文字コード
				// Feb. 9, 2001 genta
				if (pData->bUseEol) {
					nIdx = Combo_GetCurSel(pData->hwndComboEOL);
					lRes = Combo_GetItemData(pData->hwndComboEOL, nIdx);
					pData->eol = (EolType)lRes;	// 文字コード
				}
				// From Here Jul. 26, 2003 ryoji
				// BOMチェックボックスの状態を取得
				if (pData->bUseBom) {
					lRes = BtnCtl_GetCheck(pData->hwndCheckBOM);
					pData->bBom = (lRes == BST_CHECKED);	// BOM
				}
				// To Here Jul. 26, 2003 ryoji

//			MYTRACE(_T("文字コード  lRes=%d\n"), lRes);
//			MYTRACE(_T("pofn->hdr.code=CDN_FILEOK        \n"));break;
			}
			break;	// CDN_FILEOK

		case CDN_FOLDERCHANGE:
//			MYTRACE(_T("pofn->hdr.code=CDN_FOLDERCHANGE  \n"));
			{
				wchar_t szFolder[_MAX_PATH];
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				lRes = CommDlg_OpenSave_GetFolderPath(pData->hwndOpenDlg, szFolder, _countof(szFolder));
			}
//			MYTRACE(_T("\tlRes=%d\tszFolder=[%ls]\n"), lRes, szFolder);

			break;
		case CDN_SELCHANGE:
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				// OFNの再設定はNT系ではUnicode版APIのみ有効
				if (1
					&& (pData->ofn.Flags & OFN_ALLOWMULTISELECT)
					&&
#ifdef _UNICODE
						IsWin32NT()
#else
						!IsWin32NT()
#endif
				) {
					DWORD nLength = CommDlg_OpenSave_GetSpec(pData->hwndOpenDlg, NULL, 0);
					nLength += _MAX_PATH + 2;
					if (pData->ofn.nMaxFile < nLength) {
						delete[] pData->ofn.lpstrFile;
						pData->ofn.lpstrFile = new TCHAR[nLength];
						pData->ofn.nMaxFile = nLength;
					}
				}
			}
			// MYTRACE(_T("pofn->hdr.code=CDN_SELCHANGE     \n"));
			break;
//		case CDN_HELP			:	MYTRACE(_T("pofn->hdr.code=CDN_HELP          \n"));break;
//		case CDN_INITDONE		:	MYTRACE(_T("pofn->hdr.code=CDN_INITDONE      \n"));break;
//		case CDN_SHAREVIOLATION	:	MYTRACE(_T("pofn->hdr.code=CDN_SHAREVIOLATION\n"));break;
//		case CDN_TYPECHANGE		:	MYTRACE(_T("pofn->hdr.code=CDN_TYPECHANGE    \n"));break;
//		default:					MYTRACE(_T("pofn->hdr.code=???\n"));break;
		}

//		MYTRACE(_T("=======================\n"));
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND)lParam;		// handle of control
		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			switch ((int) LOWORD(wParam)) {
			// From Here Jul. 26, 2003 ryoji
			// 文字コードの変更をBOMチェックボックスに反映
			case IDC_COMBO_CODE:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					nIdx = Combo_GetCurSel((HWND) lParam);
					lRes = Combo_GetItemData((HWND) lParam, nIdx);
					CodeTypeName codeTypeName(lRes);
					if (codeTypeName.UseBom()) {
						::EnableWindow(pData->hwndCheckBOM, TRUE);
						if (lRes == pData->nCharCode){
							fCheck = pData->bBom ? BST_CHECKED: BST_UNCHECKED;
						}else {
							fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
						}
					}else {
						::EnableWindow(pData->hwndCheckBOM, FALSE);
						fCheck = BST_UNCHECKED;
					}
					BtnCtl_SetCheck(pData->hwndCheckBOM, fCheck);
				}
				break;
			// To Here Jul. 26, 2003 ryoji
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					TCHAR szWork[_MAX_PATH + 1];
					nIdx = Combo_GetCurSel((HWND) lParam);
					if (Combo_GetLBText((HWND) lParam, nIdx, szWork) != CB_ERR) {
						// 2005.11.02 ryoji ファイル名指定のコントロールを確認する
						HWND hwndFilebox = ::GetDlgItem( pData->hwndOpenDlg, cmb13 );		// ファイル名コンボ（Windows 2000タイプ）
						if (!::IsWindow(hwndFilebox))
							hwndFilebox = ::GetDlgItem( pData->hwndOpenDlg, edt1 );	// ファイル名エディット（レガシータイプ）
						if (::IsWindow(hwndFilebox)) {
							::SetWindowText(hwndFilebox, szWork);
							if (wID == IDC_COMBO_OPENFOLDER)
								::PostMessage(hwndFilebox, WM_KEYDOWN, VK_RETURN, (LPARAM)0);
						}
					}
				}
				break;
			}
			break;	// CBN_SELCHANGE
		case CBN_DROPDOWN:
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);

				switch (wID) {
				case IDC_COMBO_MRU:
					if (Combo_GetCount( pData->hwndComboMRU ) == 0) {
						// 最近開いたファイル コンボボックス初期値設定
						//	2003.06.22 Moca vMRU がNULLの場合を考慮する
						size_t nSize = pData->pDlgOpenFile->mem->vMRU.size();
						for (i=0; i<nSize; ++i) {
							Combo_AddString(pData->hwndComboMRU, pData->pDlgOpenFile->mem->vMRU[i]);
						}
					}
					Dialog::OnCbnDropDown( hwndCtl, true );
					break;

				case IDC_COMBO_OPENFOLDER:
					if (Combo_GetCount( pData->hwndComboOPENFOLDER ) == 0) {
						// 最近開いたフォルダ コンボボックス初期値設定
						//	2003.06.22 Moca vOPENFOLDER がNULLの場合を考慮する
						size_t nSize = pData->pDlgOpenFile->mem->vOPENFOLDER.size();
						for (i=0; i<nSize; ++i) {
							Combo_AddString(pData->hwndComboOPENFOLDER, pData->pDlgOpenFile->mem->vOPENFOLDER[i]);
						}
					}
					Dialog::OnCbnDropDown(hwndCtl, true);
					break;
				}
				break;	// CBN_DROPDOWN
			}
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_CP:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					if (IsDlgButtonCheckedBool(hdlg, IDC_CHECK_CP)) {
						AddComboCodePages(hdlg, pData->hwndComboCODES, -1, pData->bInitCodePage);
					}
				}
				break;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND

	//@@@ 2002.01.08 add start
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hdlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	//@@@ 2002.01.08 add end

	default:
		return FALSE;
	}
	return TRUE;
}

int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit)
{
	int nSel = -1;
	if (!bInit) {
		::EnableWindow(GetDlgItem(hdlg, IDC_CHECK_CP), FALSE);
		// コードページ追加
		bInit = true;
		nSel = CodePage::AddComboCodePages(hdlg, combo, nSelCode);
	}
	return nSel;
}


/*! コンストラクタ
	@date 2008.05.05 novice GetModuleHandle(NULL) → NULLに変更
*/
DlgOpenFile::DlgOpenFile()
{
	// メンバの初期化
	mem = new DlgOpenFileMem();

	mem->hInstance = NULL;		// アプリケーションインスタンスのハンドル
	mem->hwndParent = NULL;		// オーナーウィンドウのハンドル

	// 共有データ構造体のアドレスを返す
	mem->pShareData = &GetDllShareData();

	TCHAR szFile[_MAX_PATH + 1];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	::GetModuleFileName(
		NULL,
		szFile, _countof(szFile)
	);
	_tsplitpath(szFile, szDrive, szDir, NULL, NULL);
	_tcscpy(mem->szInitialDir, szDrive);
	_tcscat(mem->szInitialDir, szDir);

	_tcscpy(mem->szDefaultWildCard, _T("*.*"));	//「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される）

	return;
}


DlgOpenFile::~DlgOpenFile()
{
	delete mem;
	mem = NULL;
	return;
}


// 初期化
void DlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER
	)
{
	mem->hInstance = hInstance;
	mem->hwndParent = hwndParent;

	// ユーザー定義ワイルドカード（保存時の拡張子補完でも使用される）
	if (pszUserWildCard) {
		_tcscpy(mem->szDefaultWildCard, pszUserWildCard);
	}

	//「開く」での初期フォルダ
	if (pszDefaultPath && pszDefaultPath[0] != _T('\0')) {	// 現在編集中のファイルのパス	//@@@ 2002.04.18
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		// Jun. 23, 2002 genta
		my_splitpath_t(pszDefaultPath, szDrive, szDir, NULL, NULL);
		// 2010.08.28 相対パス解決
		TCHAR szRelPath[_MAX_PATH];
		auto_sprintf(szRelPath, _T("%ts%ts"), szDrive, szDir);
		const TCHAR* p = szRelPath;
		if (!::GetLongFileName(p, mem->szInitialDir)) {
			auto_strcpy(mem->szInitialDir, p);
		}
	}
	mem->vMRU = vMRU;
	mem->vOPENFOLDER = vOPENFOLDER;
	return;
}


/*! 「開く」ダイアログ モーダルダイアログの表示

	@param[in,out] pszPath 初期ファイル名．選択されたファイル名の格納場所
	@param[in] bSetCurDir カレントディレクトリを変更するか デフォルト: false
	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool DlgOpenFile::DoModal_GetOpenFileName(TCHAR* pszPath, bool bSetCurDir)
{
	// カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CurrentDirectoryBackupPoint curDirBackup;

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));

	// 構造体の初期化
	auto pData = std::make_unique<DlgOpenFileData>();
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());

	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	// From Here Jun. 23, 2002 genta
	//「開く」での初期フォルダチェック強化
// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		// Jun. 23, 2002 Thanks to sui
		my_splitpath_t(pszPath, szDrive, szDir, szName, szExt);
	
		// 指定されたファイルが存在しないとき szName == NULL
		// ファイルの場所にディレクトリを指定するとエラーになるので
		// ファイルが無い場合は全く指定しないことにする．
		if (szName[0] == _T('\0')) {
			pszPath[0] = _T('\0');
		}else {
			TCHAR szRelPath[_MAX_PATH];
			auto_sprintf(szRelPath, _T("%ts%ts%ts%ts"), szDrive, szDir, szName, szExt);
			const TCHAR* p = szRelPath;
			if (!::GetLongFileName(p, pszPath)) {
				auto_strcpy(pszPath, p);
			}
		}
	}
	pData->ofn.lpstrFile = pszPath;
	// To Here Jun. 23, 2002 genta
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	pData->ofn.lpstrDefExt = _T(""); // 2005/02/20 novice 拡張子を省略したら補完する

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();
	if (_GetOpenFileNameRecover( &pData->ofn )) {
		return true;
	}else {
		// May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}


/*! 保存ダイアログ モーダルダイアログの表示
	@param pszPath [i/o] 初期ファイル名．選択されたファイル名の格納場所
	@param bSetCurDir [in] カレントディレクトリを変更するか デフォルト: false
	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool DlgOpenFile::DoModal_GetSaveFileName(TCHAR* pszPath, bool bSetCurDir)
{
	// カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CurrentDirectoryBackupPoint curDirBackup;

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));
	
	// 2010.08.28 カレントディレクトリを移動するのでパス解決する
	if (pszPath[0]) {
		TCHAR szFullPath[_MAX_PATH];
		const TCHAR* pOrg = pszPath;
		if (::GetLongFileName(pOrg, szFullPath)) {
			// 成功。書き戻す
			auto_strcpy(pszPath, szFullPath);
		}
	}

	// 構造体の初期化
	auto pData = std::make_unique<DlgOpenFileData>();
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pszPath; // 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	
	pData->ofn.lpstrDefExt = _T("");	// 2005/02/20 novice 拡張子を省略したら補完する
	
	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	if (GetSaveFileNameRecover( &pData->ofn )) {
		return true;
	}else {
		// May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}


/*! 「開く」ダイアログ モーダルダイアログの表示
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool DlgOpenFile::DoModalOpenDlg(
	LoadInfo* pLoadInfo,
	std::vector<std::tstring>* pFileNames,
	bool bOptions
	)
{
	auto pData = std::make_unique<DlgOpenFileData>();
	pData->bIsSaveDialog = FALSE;	// 保存のダイアログか

	bool bMultiSelect = pFileNames != NULL;

	// ファイルの種類	2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	for (int i=0; i<GetDllShareData().nTypesCount; ++i) {
		const TypeConfigMini* type;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(i), &type);
		fileExt.AppendExt(type->szTypeName, type->szTypeExts);
	}

	// メンバの初期化
	pData->bViewMode = pLoadInfo->bViewMode;
	pData->nCharCode = pLoadInfo->eCharCode;	// 文字コード自動判別
	pData->nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	pData->bUseCharCode = true;
	pData->bUseEol = false;	//	Feb. 9, 2001 genta
	pData->bUseBom = false;	//	Jul. 26, 2003 ryoji

	// ファイルパス受け取りバッファ
	TCHAR* pszPathBuf = new TCHAR[2000];
	auto_strcpy(pszPathBuf, pLoadInfo->filePath); // 2013.05.27 デフォルトファイル名を設定する

	// OPENFILENAME構造体の初期化
	InitOfn( &pData->ofn );		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pszPathBuf;
	pData->ofn.nMaxFile = 2000;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_ENABLESIZING;
	if (pData->bViewMode) pData->ofn.Flags |= OFN_READONLY;
	if (bMultiSelect) {
		pData->ofn.Flags |= OFN_ALLOWMULTISELECT;
	}
	pData->ofn.lpstrDefExt = _T("");	// 2005/02/20 novice 拡張子を省略したら補完する
	if (!bOptions) {
		pData->ofn.Flags |= OFN_HIDEREADONLY;
		pData->bUseCharCode = false;
	}

	// カレントディレクトリを保存。関数を抜けるときに自動でカレントディレクトリは復元されます。
	CurrentDirectoryBackupPoint curDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	// ダイアログ表示
	bool bDlgResult = _GetOpenFileNameRecover( &pData->ofn );
	if (bDlgResult) {
		if (bMultiSelect) {
			pLoadInfo->filePath = _T("");
			if (pData->ofn.nFileOffset < _tcslen( pData->ofn.lpstrFile )) {
				pFileNames->emplace_back(pData->ofn.lpstrFile);
			}else {
				std::tstring path;
				TCHAR* pos = pData->ofn.lpstrFile;
				pos += _tcslen(pos) + 1;
				while (*pos != _T('\0')) {
					path = pData->ofn.lpstrFile;
					path.append(_T("\\"));
					path.append(pos);
					pFileNames->push_back(path);
					pos += _tcslen(pos) + 1;
				}
			}
		}else {
			pLoadInfo->filePath = pData->ofn.lpstrFile;
		}
		pLoadInfo->eCharCode = pData->nCharCode;
		pLoadInfo->bViewMode = pData->bViewMode;
	}else {
		DlgOpenFail();
	}
	delete[] pData->ofn.lpstrFile;
	return bDlgResult;
}

/*! 保存ダイアログ モーダルダイアログの表示

	@date 2001.02.09 genta	引数追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をFileExtクラスで行う。
	@date 2003.07.26 ryoji BOMパラメータ追加
	@date 2005.02.20 novice 拡張子を省略したら補完する
	@date 2006.11.10 ryoji フックを使う場合は拡張子の補完を自前で行う
		Windowsで関連付けが無いような拡張子を指定して保存すると、明示的に
		拡張子入力してあるのにデフォルト拡張子が補完されてしまうことがある。
			例）hoge.abc -> hoge.abc.txt
		自前で補完することでこれを回避する。（実際の処理はフックプロシージャの中）
*/
bool DlgOpenFile::DoModalSaveDlg(
	SaveInfo* pSaveInfo,
	bool bSimpleMode
	)
{
	auto pData = std::make_unique<DlgOpenFileData>();
	pData->bIsSaveDialog = true;	// 保存のダイアログか

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));

	// ファイル名の初期設定	// 2006.11.10 ryoji
	if (pSaveInfo->filePath[0] == _T('\0')) {
		lstrcpyn(pSaveInfo->filePath, LS(STR_NO_TITLE2), _MAX_PATH);	// 無題
	}
	// OPENFILENAME構造体の初期化
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pSaveInfo->filePath;	// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_ENABLESIZING;
	if (!bSimpleMode) {
		pData->ofn.Flags = pData->ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
		pData->ofn.Flags &= ~OFN_OVERWRITEPROMPT;	// 2006.11.10 ryoji 上書き確認もフックの中で自前で処理する
	}

// 2005/02/20 novice 拡張子を省略したら補完する
//	pData->ofn.lpstrDefExt = _T("");
	pData->ofn.lpstrDefExt = (pData->ofn.Flags & OFN_ENABLEHOOK) ? NULL: _T("");	// 2006.11.10 ryoji フックを使うときは自前で拡張子を補完する

	// カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CurrentDirectoryBackupPoint curDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	pData->nCharCode = pSaveInfo->eCharCode;

	// From Here Feb. 9, 2001 genta
	if (!bSimpleMode) {
		pData->eol = pSaveInfo->eol;	//	初期値は「改行コードを保存」に固定 // 2013.05.27 初期値を指定
		pData->bUseEol = true;
	}else {
		pData->bUseEol = false;
	}

	// To Here Feb. 9, 2001 genta
	// Jul. 26, 2003 ryoji BOM設定
	if (!bSimpleMode) {
		pData->bBom = pSaveInfo->bBomExist;
		pData->bUseBom = true;
		pData->bUseCharCode = true;
	}else {
		pData->bUseBom = false;
		pData->bUseCharCode = false;
	}

	pData->nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	if (GetSaveFileNameRecover(&pData->ofn)) {
		pSaveInfo->filePath = pData->ofn.lpstrFile;
		if (pData->ofn.Flags & OFN_ENABLEHOOK) {
			lstrcpyn(pSaveInfo->filePath, pData->szPath, _MAX_PATH);	// 自前で拡張子の補完を行ったときのファイルパス	// 2006.11.10 ryoji
		}
		pSaveInfo->eCharCode = pData->nCharCode;

		// Feb. 9, 2001 genta
		if (pData->bUseEol) {
			pSaveInfo->eol = pData->eol;
		}
		// Jul. 26, 2003 ryoji BOM設定
		if (pData->bUseBom) {
			pSaveInfo->bBomExist = pData->bBom;
		}
		return true;
	}else {
		// May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}

/*! @brief コモンダイアログボックス失敗処理

	コモンダイアログボックスからFALSEが返された場合に
	エラー原因を調べてエラーならメッセージを出す．
	
	@author genta
	@date 2004.05.29 genta 元々あった部分をまとめた
*/
void DlgOpenFile::DlgOpenFail(void)
{
	const TCHAR* pszError;
	DWORD dwError = ::CommDlgExtendedError();
	if (dwError == 0) {
		// ユーザキャンセルによる
		return;
	}
	
	switch (dwError) {
	case CDERR_DIALOGFAILURE  : pszError = _T("CDERR_DIALOGFAILURE  "); break;
	case CDERR_FINDRESFAILURE : pszError = _T("CDERR_FINDRESFAILURE "); break;
	case CDERR_NOHINSTANCE    : pszError = _T("CDERR_NOHINSTANCE    "); break;
	case CDERR_INITIALIZATION : pszError = _T("CDERR_INITIALIZATION "); break;
	case CDERR_NOHOOK         : pszError = _T("CDERR_NOHOOK         "); break;
	case CDERR_LOCKRESFAILURE : pszError = _T("CDERR_LOCKRESFAILURE "); break;
	case CDERR_NOTEMPLATE     : pszError = _T("CDERR_NOTEMPLATE     "); break;
	case CDERR_LOADRESFAILURE : pszError = _T("CDERR_LOADRESFAILURE "); break;
	case CDERR_STRUCTSIZE     : pszError = _T("CDERR_STRUCTSIZE     "); break;
	case CDERR_LOADSTRFAILURE : pszError = _T("CDERR_LOADSTRFAILURE "); break;
	case FNERR_BUFFERTOOSMALL : pszError = _T("FNERR_BUFFERTOOSMALL "); break;
	case CDERR_MEMALLOCFAILURE: pszError = _T("CDERR_MEMALLOCFAILURE"); break;
	case FNERR_INVALIDFILENAME: pszError = _T("FNERR_INVALIDFILENAME"); break;
	case CDERR_MEMLOCKFAILURE : pszError = _T("CDERR_MEMLOCKFAILURE "); break;
	case FNERR_SUBCLASSFAILURE: pszError = _T("FNERR_SUBCLASSFAILURE"); break;
	default: pszError = _T("UNKNOWN_ERRORCODE"); break;
	}

	ErrorBeep();
	TopErrorMessage( mem->hwndParent,
		LS(STR_DLGOPNFL_ERR1),
		pszError
	);
}

/*! OPENFILENAME 初期化

	OPENFILENAME に DlgOpenFile クラス用の初期規定値を設定する

	@author ryoji
	@date 2005.10.29
*/
void DlgOpenFile::InitOfn(OPENFILENAMEZ* ofn)
{
	memset_raw(ofn, 0, sizeof(*ofn));

	ofn->lStructSize = IsWinV5forOfn() ? sizeof(OPENFILENAMEZ): OPENFILENAME_SIZE_VERSION_400;
	ofn->lpfnHook = OFNHookProc;
	ofn->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);	// <-_T("IDD_FILEOPEN"); 2008/7/26 Uchi
	ofn->nFilterIndex = 1;	//Jul. 09, 2001 JEPRO		// 「開く」での最初のワイルドカード
}

/*! 初期レイアウト設定処理

	追加コントロールのレイアウトを変更する

	@param hwndOpenDlg [in]		ファイルダイアログのウィンドウハンドル
	@param hwndDlg [in]			子ダイアログのウィンドウハンドル
	@param hwndBaseCtrl [in]	移動基準コントロール（ファイル名ボックスと左端を合わせるコントロール）のウィンドウハンドル

	@author ryoji
	@date 2005.11.02
*/
void DlgOpenFile::InitLayout(
	HWND hwndOpenDlg,
	HWND hwndDlg,
	HWND hwndBaseCtrl
	)
{
	HWND hwndFilelabel;
	// ファイル名ラベルとファイル名ボックスを取得する
	if (!::IsWindow(hwndFilelabel = ::GetDlgItem(hwndOpenDlg, stc3)))		// ファイル名ラベル
		return;
	HWND hwndFilebox;
	if (!::IsWindow(hwndFilebox = ::GetDlgItem(hwndOpenDlg, cmb13))) {		// ファイル名コンボ（Windows 2000タイプ）
		if (!::IsWindow(hwndFilebox = ::GetDlgItem(hwndOpenDlg, edt1)))		// ファイル名エディット（レガシータイプ）
			return;
	}

	// コントロールの基準位置、移動量を決定する
	RECT rcBase;
	RECT rc;
	::GetWindowRect(hwndFilelabel, &rc);
	int nLeft = rc.left;						// 左端に揃えるコントロールの位置
	::GetWindowRect(hwndFilebox, &rc);
	::GetWindowRect(hwndBaseCtrl, &rcBase);
	int nShift = rc.left - rcBase.left;			// 左端以外のコントロールの右方向への相対移動量

	// 追加コントロールをすべて移動する
	// ・基準コントロールよりも左にあるものはファイル名ラベルに合わせて左端に移動
	// ・その他は移動基準コントロール（ファイル名ボックスと左端を合わせるコントロール）と同じだけ右方向へ相対移動
	HWND hwndCtrl = ::GetWindow(hwndDlg, GW_CHILD);
	while (hwndCtrl) {
		if (::GetDlgCtrlID(hwndCtrl) != stc32) {
			::GetWindowRect(hwndCtrl, &rc);
			POINT pt;
			pt.x = (rc.right < rcBase.left)? nLeft: rc.left + nShift;
			pt.y = rc.top;
			::ScreenToClient(hwndDlg, &pt);
			::SetWindowPos(hwndCtrl, 0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
		hwndCtrl = ::GetWindow(hwndCtrl, GW_HWNDNEXT);
	}

	// 標準コントロールのプレースフォルダ（stc32）と子ダイアログの幅をオープンダイアログの幅にあわせる
	//     WM_INITDIALOG を抜けるとさらにオープンダイアログ側で現在の位置関係からレイアウト調整が行われる
	//     ここで以下の処理をやっておかないとコントロールが意図しない場所に動いてしまうことがある
	//     （例えば、BOM のチェックボックスが画面外に飛んでしまうなど）

	// オープンダイアログのクライアント領域の幅を取得する
	::GetClientRect(hwndOpenDlg, &rc);
	int nWidth = rc.right - rc.left;

	// 標準コントロールプレースフォルダの幅を変更する
	hwndCtrl = ::GetDlgItem(hwndDlg, stc32);
	::GetWindowRect(hwndCtrl, &rc);
	::SetWindowPos(hwndCtrl, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	// 子ダイアログの幅を変更する
	// ※この SetWindowPos() の中で WM_SIZE が発生する
	::GetWindowRect(hwndDlg, &rc);
	::SetWindowPos(hwndDlg, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}


/*! リトライ機能付き GetOpenFileName
	@author Moca
	@date 2006.09.03 新規作成
*/
bool DlgOpenFile::_GetOpenFileNameRecover(OPENFILENAMEZ* ofn)
{
	BOOL bRet = ::GetOpenFileName(ofn);
	if (!bRet) {
		if (::CommDlgExtendedError() == FNERR_INVALIDFILENAME) {
			_tcscpy(ofn->lpstrFile, _T(""));
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetOpenFileName(ofn);
		}
	}
	return bRet != FALSE;
}

/*! リトライ機能付き GetSaveFileName
	@author Moca
	@date 2006.09.03 新規作成
*/
bool DlgOpenFile::GetSaveFileNameRecover(OPENFILENAMEZ* ofn)
{
	BOOL bRet = ::GetSaveFileName(ofn);
	if (!bRet) {
		if (::CommDlgExtendedError() == FNERR_INVALIDFILENAME) {
			_tcscpy(ofn->lpstrFile, _T(""));
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetSaveFileName(ofn);
		}
	}
	return bRet != FALSE;
}

