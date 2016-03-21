/*!	@file
	@brief 共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

//////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif

#include <Windows.h>
#include <tchar.h>



// ウィンドウのID
#define IDW_STATUSBAR 123

#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000

#include "charset/charset.h"

// ダイアログ表示方法	 アウトラインウィンドウ用に作成 20060201 aroka
enum class ShowDialogType {
	Normal,
	Reload,
	Toggle,
};

// 選択領域描画用パラメータ
extern const COLORREF	SELECTEDAREA_RGB;
extern const int		SELECTEDAREA_ROP2;


//@@@ From Here 2003.05.31 MIK
// タブウィンドウ用メッセージサブコマンド
enum class TabWndNotifyType {
	Refresh,		// 再表示
	Add,			// ウィンドウ登録
	Delete,			// ウィンドウ削除
	Reorder,		// ウィンドウ順序変更
	Rename,			// ファイル名変更
	Enable,			// タブモード有効化	// 2004.07.14 Kazika 追加
	Disable,		// タブモード無効化	// 2004.08.27 Kazika 追加
	Adjust,			// ウィンドウ位置合わせ	// 2007.04.03 ryoji 追加
};

// バーの表示・非表示
enum class BarChangeNotifyType {
	Toolbar,		// ツールバー
	FuncKey,		// ファンクションキー
	Tab,			// タブ
	StatusBar,		// ステータスバー
	MiniMap,		//ミニマップ
};
//@@@ To Here 2003.05.31 MIK

// タブで使うカスタムメニューのインデックス	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
// 右クリックメニューで使うカスタムメニューのインデックス	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


// 色タイプ
//@@@ From Here 2006.12.18 ryoji
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
#define COLOR_ATTRIB_NO_TEXT		0x00000010
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	予約値
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct ColorAttributeData {
	const TCHAR*	szName;
	unsigned int	fAttribute;
};
extern const ColorAttributeData g_ColorAttributeArr[];

//@@@ To Here 2006.12.18 ryoji

// 設定値の上限・下限
// ルーラの高さ
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

/**	マウスクリックとキー定義の対応

	@date 2007.11.04 genta 新規作成．即値回避と範囲サイズ定義のため
*/
enum class MouseFunctionType {
	DoubleClick,	// ダブルクリック
	RightClick,		// 右クリック
	CenterClick,	// 中クリック
	LeftSideClick,	// 左サイドクリック
	RightSideClick,	// 右サイドクリック
	TripleClick,	// トリプルクリック
	QuadrapleClick,	// クアドラプルクリック
	WheelUp,		// ホイールアップ
	WheelDown,		// ホイールダウン
	WheelLeft,		// ホイール左
	WheelRight,		// ホイール右
	KeyBegin,		// マウスへの割り当て個数＝本当のキー割り当て先頭INDEX
};

// 2008.05.30 nasukoji	テキストの折り返し方法
enum class TextWrappingMethod {
	NoWrapping,		// 折り返さない（ScrollBarをテキスト幅に合わせる）
	SettingWidth,	// 指定桁で折り返す
	WindowWidth,	// 右端で折り返す
};

// 2009.07.06 syat	文字カウント方法
enum class SelectCountMode {
	Toggle,		// 文字カウント方法をトグル
	ByChar,		// 文字数でカウント
	ByByte,		// バイト数でカウント
};

// 2007.09.06 kobake 追加
// 検索方向
enum class SearchDirection {
	Backward,	// 前方検索 (前を検索)
	Forward,	// 後方検索 (次を検索) (普通)
};

// 2007.09.06 kobake 追加
struct SearchOption {
//	SearchDirection	eDirection;
//	bool	bPrevOrNext;	// false==前方検索 true==後方検索
	bool	bRegularExp;	// true==正規表現
	bool	bLoHiCase;		// true==英大文字小文字の区別
	bool	bWordOnly;		// true==単語のみ検索

	SearchOption()
		:
		bRegularExp(false),
		bLoHiCase(false),
		bWordOnly(false)
	{
	}
	SearchOption(
		bool bRegularExp,
		bool bLoHiCase,
		bool bWordOnly
	)
		:
		bRegularExp(bRegularExp),
		bLoHiCase(bLoHiCase),
		bWordOnly(bWordOnly)
	{
	}
	void Reset() {
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	// 演算子
	bool operator == (const SearchOption& rhs) const {
		// とりあえずmemcmpでいいや
		return memcmp(this, &rhs, sizeof(*this)) == 0;
	}
	bool operator != (const SearchOption& rhs) const {
		return !operator == (rhs);
	}

};

// 2007.10.02 kobake EditWndのインスタンスへのポインタをここに保存しておく
class EditWnd;
extern EditWnd* g_pcEditWnd;

HINSTANCE G_AppInstance();

