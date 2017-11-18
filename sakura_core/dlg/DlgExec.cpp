/*!	@file
	@brief 外部コマンド実行ダイアログ
*/

#include "StdAfx.h"
#include "dlg/DlgExec.h"
#include "dlg/DlgOpenFile.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/window.h"
#include "_main/AppMode.h"
#include "doc/EditDoc.h"
#include "sakura_rc.h"
#include "sakura.hh"

// 外部コマンド CDlgExec.cpp
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		// 参照
	IDOK,							HIDOK_EXEC,						// 実行
	IDCANCEL,						HIDCANCEL_EXEC,					// キャンセル
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			// ヘルプ
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		// 標準出力を得る
	IDC_COMBO_CODE_GET,				HIDC_COMBO_CODE_GET,			// 標準出力文字コード
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	// コマンド
	IDC_RADIO_OUTPUT,				HIDC_RADIO_OUTPUT,				// 標準出力リダイレクト先：アウトプットウィンドウ
	IDC_RADIO_EDITWINDOW,			HIDC_RADIO_EDITWINDOW,			// 標準出力リダイレクト先：編集中のウィンドウ
	IDC_CHECK_SENDSTDIN,			HIDC_CHECK_SENDSTDIN,			// 標準入力に送る
	IDC_COMBO_CODE_SEND,			HIDC_COMBO_CODE_SEND,			// 標準出力文字コード
	IDC_CHECK_CUR_DIR,				HIDC_CHECK_CUR_DIR,				// カレントディレクトリ
	IDC_COMBO_CUR_DIR,				HIDC_COMBO_CUR_DIR,				// カレントディレクトリ指定
	IDC_BUTTON_REFERENCE2,			HIDC_COMBO_CUR_DIR,				// カレントディレクトリ指定(参照)
//	IDC_STATIC,						-1,
	0, 0
};

DlgExec::DlgExec()
{
	szCommand[0] = _T('\0');	// コマンドライン
	return;
}

static const int codeTable1[] = { 0x00, 0x08, 0x80 };
static const int codeTable2[] = { 0x00, 0x10, 0x100 };


// モーダルダイアログの表示
INT_PTR DlgExec::DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam)
{
	szCommand[0] = _T('\0');	// コマンドライン
	bEditable = EditDoc::GetInstance(0)->IsEditable();
	return Dialog::DoModal(hInstance, hwndParent, IDD_EXEC, lParam);
}


BOOL DlgExec::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwnd);
	
	EncodingType codes[] = { CODE_SJIS, CODE_UNICODE, CODE_UTF8 };
	HWND hwndCombo;
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_GET);
	for (size_t i=0; i<_countof(codes); ++i) {
		Combo_AddString(hwndCombo, CodeTypeName(codes[i]).Normal());
	}
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_SEND);
	for (size_t i=0; i<_countof(codes); ++i) {
		Combo_AddString(hwndCombo, CodeTypeName(codes[i]).Normal());
	}

	BOOL bRet = Dialog::OnInitDialog(hwnd, wParam, lParam);

	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentCmd;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_m_szCommand), &comboDel);
	comboDelCur = ComboBoxItemDeleter();
	comboDelCur.pRecent = &recentCur;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_CUR_DIR), &comboDelCur);
	return bRet;
}

// ダイアログデータの設定
void DlgExec::SetData(void)
{
//	MYTRACE(_T("DlgExec::SetData()"));
	/*****************************
	*           初期             *
	*****************************/
	// ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する
	Combo_LimitText(GetItemHwnd(IDC_COMBO_m_szCommand), _countof(szCommand) - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_CUR_DIR), _countof2(szCurDir) - 1);
	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_m_szCommand), TRUE);

	{	// From Here 2007.01.02 maru 引数を拡張のため
		// マクロからの呼び出しではShareDataに保存させないように，ShareDataとの受け渡しはExecCmdの外で
		int nExecFlgOpt;
		nExecFlgOpt = pShareData->nExecFlgOpt;
		
		// ビューモードや上書き禁止のときは編集中ウィンドウへは出力しない	
		if (!bEditable) {
			nExecFlgOpt &= ~0x02;
		}

		CheckButton(IDC_CHECK_GETSTDOUT,	(nExecFlgOpt & 0x01) ? true : false);
		CheckButton(IDC_RADIO_OUTPUT,		(nExecFlgOpt & 0x02) ? false : true);
		CheckButton(IDC_RADIO_EDITWINDOW,	(nExecFlgOpt & 0x02) ? true : false);
		CheckButton(IDC_CHECK_SENDSTDIN,	(nExecFlgOpt & 0x04) ? true : false);
		CheckButton(IDC_CHECK_CUR_DIR,		(nExecFlgOpt & 0x200) ? true : false);

		EnableItem(IDC_RADIO_OUTPUT,		(nExecFlgOpt & 0x01) ? true : false);
		EnableItem(IDC_RADIO_EDITWINDOW,	((nExecFlgOpt & 0x01) && bEditable)? true : false);
		EnableItem(IDC_COMBO_CODE_GET,		(nExecFlgOpt & 0x01) ? true : false);		// 標準出力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
		EnableItem(IDC_COMBO_CODE_SEND,		(nExecFlgOpt & 0x04) ? true : false);		// 標準入力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
		EnableItem(IDC_COMBO_CUR_DIR,		(nExecFlgOpt & 0x200) ? true : false);
		EnableItem(IDC_BUTTON_REFERENCE2,	(nExecFlgOpt & 0x200) ? true : false);
	}

	/*****************************
	*         データ設定         *
	*****************************/
	_tcscpy(szCommand, pShareData->history.aCommands[0]);
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_m_szCommand);
	Combo_ResetContent(hwndCombo);
	SetItemText(IDC_COMBO_TEXT, szCommand);
	size_t nSize = pShareData->history.aCommands.size();
	for (size_t i=0; i<nSize; ++i) {
		Combo_AddString(hwndCombo, pShareData->history.aCommands[i]);
	}
	Combo_SetCurSel(hwndCombo, 0);

	_tcscpy(szCurDir, pShareData->history.aCurDirs[0]);
	hwndCombo = GetItemHwnd(IDC_COMBO_CUR_DIR);
	Combo_ResetContent(hwndCombo);
	SetItemText(IDC_COMBO_TEXT, szCurDir);
	for (size_t i=0; i<pShareData->history.aCurDirs.size(); ++i) {
		Combo_AddString(hwndCombo, pShareData->history.aCurDirs[i]);
	}
	Combo_SetCurSel(hwndCombo, 0);
	
	int nOpt;
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_GET);
	nOpt = pShareData->nExecFlgOpt & 0x88;
	for (size_t i=0; _countof(codeTable1); ++i) {
		if (codeTable1[i] == nOpt) {
			Combo_SetCurSel(hwndCombo, i);
			break;
		}
	}
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_SEND);
	nOpt = pShareData->nExecFlgOpt & 0x110;
	for (size_t i=0; _countof(codeTable2); ++i) {
		if (codeTable2[i] == nOpt) {
			Combo_SetCurSel(hwndCombo, i);
			break;
		}
	}
	return;
}


// ダイアログデータの取得
int DlgExec::GetData(void)
{
	GetItemText(IDC_COMBO_m_szCommand, szCommand, _countof(szCommand));
	if (IsButtonChecked(IDC_CHECK_CUR_DIR)) {
		GetItemText(IDC_COMBO_CUR_DIR, &szCurDir[0], _countof2(szCurDir));
	}else {
		szCurDir[0] = _T('\0');
	}
	
	{
		// マクロからの呼び出しではShareDataに保存させないように，ShareDataとの受け渡しはExecCmdの外で
		int nFlgOpt = 0;
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_GETSTDOUT)) ? 0x01 : 0;	// 標準出力を得る
		nFlgOpt |= (IsButtonChecked(IDC_RADIO_EDITWINDOW)) ? 0x02 : 0;	// 標準出力を編集中のウィンドウへ
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_SENDSTDIN)) ? 0x04 : 0;	// 編集中ファイルを標準入力へ
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_CUR_DIR)) ? 0x200 : 0;	// カレントディレクトリ指定
		int sel;
		sel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_CODE_GET));
		nFlgOpt |= codeTable1[sel];
		sel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_CODE_SEND));
		nFlgOpt |= codeTable2[sel];
		pShareData->nExecFlgOpt = nFlgOpt;
	}
	return 1;
}


BOOL DlgExec::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_GETSTDOUT:
		{
			bool bEnabled = IsButtonChecked(IDC_CHECK_GETSTDOUT);
			EnableItem(IDC_RADIO_OUTPUT, bEnabled);
			EnableItem(IDC_RADIO_EDITWINDOW, bEnabled && bEditable);	// ビューモードや上書き禁止の条件追加	// 2009.02.21 ryoji
		}

		// 標準出力Off時、Unicodeを使用するをDesableする	
		EnableItem(IDC_COMBO_CODE_GET, IsButtonChecked(IDC_CHECK_GETSTDOUT));
		break;
	case IDC_CHECK_SENDSTDIN:	// 標準入力Off時、Unicodeを使用するをDesableする
		EnableItem(IDC_COMBO_CODE_SEND, IsButtonChecked(IDC_CHECK_SENDSTDIN));
		break;
	case IDC_CHECK_CUR_DIR:
		EnableItem(IDC_COMBO_CUR_DIR, IsButtonChecked(IDC_CHECK_CUR_DIR));
		EnableItem(IDC_BUTTON_REFERENCE2, IsButtonChecked(IDC_CHECK_CUR_DIR));
		break;

	case IDC_BUTTON_HELP:
		//「検索」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECMD_DIALOG));
		break;

	case IDC_BUTTON_REFERENCE:	// ファイル名の「参照...」ボタン
		{
			DlgOpenFile	dlgOpenFile;
			TCHAR		szPath[_MAX_PATH + 1];
			int			size = _countof(szPath) - 1;
			_tcsncpy(szPath, szCommand, size);
			szPath[size] = _T('\0');
			// ファイルオープンダイアログの初期化
			dlgOpenFile.Create(
				hInstance,
				GetHwnd(),
				_T("*.com;*.exe;*.bat;*.cmd"),
				szCommand
			);
			if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
				_tcscpy(szCommand, szPath);
				SetItemText(IDC_COMBO_m_szCommand, szCommand);
			}
		}
		return TRUE;

	case IDC_BUTTON_REFERENCE2:
		{
			if (SelectDir(GetHwnd(), LS(STR_DLGEXEC_SELECT_CURDIR), &szCurDir[0], &szCurDir[0])) {
				SetItemText(IDC_COMBO_CUR_DIR, &szCurDir[0]);
			}
		}
		return TRUE;

	case IDOK:			// 下検索
		// ダイアログデータの取得
		GetData();
		CloseDialog(1);
		return TRUE;
	case IDCANCEL:
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

LPVOID DlgExec::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


