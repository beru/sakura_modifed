/*!	@file
@brief ViewCommanderクラスのコマンド(挿入系)関数群

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCtrlCode.h"	// コントロールコードの入力(ダイアログ)
#include "env/FormatManager.h"

// 日付挿入
void ViewCommander::Command_INS_DATE(void)
{
	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);

	// テキストを貼り付け ver1
	Command_INSTEXT(true, to_wchar(szText), LogicInt(-1), true);
}


// 時刻挿入
void ViewCommander::Command_INS_TIME(void)
{
	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);

	// テキストを貼り付け ver1
	Command_INSTEXT(true, to_wchar(szText), LogicInt(-1), true);
}


// from ViewCommander_New.cpp
/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void ViewCommander::Command_CtrlCode_Dialog(void)
{
	DlgCtrlCode dlgCtrlCode;

	// コントロールコード入力ダイアログを表示する
	if (dlgCtrlCode.DoModal(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetDocument())) {
		// コントロールコードを入力する
		// 2013.06.11 Command_WCHAR -> HandleCommand マクロ記録対応
		// 2013.12.12 F_WCHAR -> F_CTRL_CODE
		HandleCommand(F_CTRL_CODE, true, dlgCtrlCode.GetCharCode(), 0, 0, 0);
	}
}

