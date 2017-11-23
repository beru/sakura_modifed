#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCtrlCode.h"	// コントロールコードの入力(ダイアログ)
#include "env/FormatManager.h"

// ViewCommanderクラスのコマンド(挿入系)関数群

// 日付挿入
void ViewCommander::Command_Ins_Date(void)
{
	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);

	// テキストを貼り付け ver1
	Command_InsText(true, to_wchar(szText), wcslen(szText), true);
}


// 時刻挿入
void ViewCommander::Command_Ins_Time(void)
{
	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);

	// テキストを貼り付け ver1
	Command_InsText(true, to_wchar(szText), wcslen(szText), true);
}


//	コントロールコードの入力(ダイアログ)
void ViewCommander::Command_CtrlCode_Dialog(void)
{
	DlgCtrlCode dlgCtrlCode;

	// コントロールコード入力ダイアログを表示する
	if (dlgCtrlCode.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument())) {
		// コントロールコードを入力する
		HandleCommand(F_CTRL_CODE, true, dlgCtrlCode.GetCharCode(), 0, 0, 0);
	}
}

