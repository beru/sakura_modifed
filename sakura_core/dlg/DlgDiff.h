/*!	@file
	@brief DIFF差分表示ダイアログボックス
*/

class DlgDiff;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief DIFF差分表示ダイアログボックス
*/
class DlgDiff : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgDiff();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal( HINSTANCE, HWND, LPARAM, const TCHAR* );	// モーダルダイアログの表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked(int);
	BOOL	OnLbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnLbnDblclk(int wID);
	BOOL	OnEnChange(HWND hwndCtl, int wID);
	LPVOID	GetHelpIdTable(void);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 標準以外のメッセージを捕捉する
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得

private:
	int			nIndexSave;		// 最後に選択されていた番号
	POINT		ptDefaultSize;
	RECT		rcItems[22];

public:
	SFilePath	szFile1;			// 自ファイル
	SFilePath	szFile2;			// 相手ファイル
	bool		bIsModifiedDst;		// 相手ファイル更新中
	EncodingType	nCodeTypeDst;	// 相手ファイルの文字コード
	bool		bBomDst;			// 相手ファイルのBOM
	int			nDiffFlgOpt;		// DIFFオプション
	HWND		hWnd_Dst;			// 相手ウィンドウハンドル

};


