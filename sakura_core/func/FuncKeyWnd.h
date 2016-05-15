/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

#include "window/Wnd.h"
#include "env/DllSharedData.h"

struct DllSharedData;
class EditDoc; // 2002/2/10 aroka

// ファンクションキーウィンドウ
// @date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class FuncKeyWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	FuncKeyWnd();
	virtual ~FuncKeyWnd();
	/*
	|| メンバ関数
	*/
	HWND Open(HINSTANCE, HWND, EditDoc*, bool);	// ウィンドウ オープン
	void Close(void);	// ウィンドウ クローズ
	void SizeBox_ONOFF(bool);	// サイズボックスの表示／非表示切り替え
	void Timer_ONOFF(bool); // 更新の開始／停止 20060126 aroka
	/*
	|| メンバ変数
	*/
private:
	// 20060126 aroka すべてPrivateにして、初期化順序に合わせて並べ替え
	EditDoc*		pEditDoc;
	DllSharedData&	shareData;
	int				nCurrentKeyState;
	WCHAR			szFuncNameArr[12][256];
	HWND			hwndButtonArr[12];
	HFONT			hFont;	// 表示用フォント
	bool			bSizeBox;
	HWND			hwndSizeBox;
	int				nTimerCount;
	int				nButtonGroupNum;	// Openで初期化
	EFunctionCode	nFuncCodeArr[12];	// Open->CreateButtonsで初期化
protected:
	/*
	|| 実装ヘルパ系
	*/
	void CreateButtons(void);	// ボタンの生成
	int CalcButtonSize(void);	// ボタンのサイズを計算
	
	// 仮想関数
	virtual void AfterCreateWindow(void) {}	// ウィンドウ作成後の処理	// 2007.03.13 ryoji 可視化しない
	
	// 仮想関数 メッセージ処理 詳しくは実装を参照
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);	// WM_TIMERタイマーの処理
	virtual LRESULT OnCommand(HWND, UINT, WPARAM, LPARAM);	// WM_COMMAND処理
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);		// WM_SIZE処理
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);	// WM_DESTROY処理
};

