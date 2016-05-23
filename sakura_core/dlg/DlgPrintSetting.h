/*!	@file
	@brief 印刷設定ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

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
#pragma once

#include "dlg/Dialog.h"
#include "config/maxdata.h"	// MAX_PrintSettingARR
#include "print/Print.h"	// PrintSetting

/*!	印刷設定ダイアログ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class DlgPrintSetting : public Dialog {
public:
	/*
	||  Constructors
	*/

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int*, PrintSetting*, int);	// モーダルダイアログの表示

private:
	int				nCurrentPrintSetting;
	PrintSetting	printSettingArr[MAX_PrintSettingARR];
	int				nLineNumberColumns;					// 行番号表示する場合の桁数
	bool			bPrintableLinesAndColumnInvalid;
	HFONT			hFontDlg;								// ダイアログのフォントハンドル
	int				nFontHeight;							// ダイアログのフォントのサイズ

protected:
	/*
	||  実装ヘルパ関数
	*/
	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy(void);
	BOOL OnNotify(WPARAM,  LPARAM);
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	BOOL OnStnClicked(int);
	BOOL OnEnChange(HWND hwndCtl, int wID);
	BOOL OnEnKillFocus(HWND hwndCtl, int wID);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType(BOOL);	// 設定のタイプが変わった
	void OnSpin(int , BOOL);			// スピンコントロールの処理
	int DataCheckAndCorrect(int , int);	// 入力値(数値)のエラーチェックをして正しい値を返す
	bool CalcPrintableLineAndColumn();		// 行数と桁数を計算
	void UpdatePrintableLineAndColumn();	// 行数と桁数の計算要求
	void SetFontName(int idTxt, int idUse, LOGFONT& lf, int nPointSize);	// フォント名/使用ボタンの設定
};


