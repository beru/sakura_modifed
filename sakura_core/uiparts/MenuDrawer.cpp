#include "StdAfx.h"
#include "MenuDrawer.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "window/SplitBoxWnd.h"
#include "ImageListMgr.h"
#include "func/KeyBind.h"
#include "_os/OSVersionInfo.h"
#include "util/window.h"

// メニューアイコンの背景をボタンの色にする
#define DRAW_MENU_ICON_BACKGROUND_3DFACE

// メニューの選択色を淡くする
#define DRAW_MENU_SELECTION_LIGHT

MenuDrawer::MenuDrawer()
{
	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	hInstance = NULL;
	hWndOwner = NULL;
	nMenuHeight = 0;
	nMenuFontHeight = 0;
	hFontMenu = NULL;
	pIcons = nullptr;
	hCompBitmap = NULL;
	hCompDC = NULL;

	/* ツールバーのボタン TBBUTTON構造体 */
	/*
	typedef struct _TBBUTTON {
		int iBitmap;	// ボタン イメージの 0 から始まるインデックス
		int idCommand;	// ボタンが押されたときに送られるコマンド
		BYTE fsState;	// ボタンの状態--以下を参照
		BYTE fsStyle;	// ボタン スタイル--以下を参照
		DWORD dwData;	// アプリケーション-定義された値
		int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
	} TBBUTTON;
	*/
//	キーワード：アイコン順序(アイコンインデックス)
//	アイコン登録メニュー
//	以下の登録はツールバーだけでなくアイコンをもつすべてのメニューで利用されている
//	数字はビットマップリソースのIDB_MYTOOLに登録されているアイコンの先頭からの順番のようである
//	アイコンをもっと登録できるように横幅を16dotsx218=2048dotsに拡大
//	縦も15dotsから16dotsにして「プリンタ」アイコンや「ヘルプ1」の、下が欠けている部分を補ったが15dotsまでしか表示されないらしく効果なし
//	→
//	・配置の基本は「コマンド一覧」に入っている機能(コマンド)順	なお「コマンド一覧」自体は「メニューバー」の順におおよそ準拠している
//	・アイコンビットマップファイルには横32個X15段ある
//	・互換性と新コマンド追加の両立の都合で飛び地あり
//	・メニューに属する系および各系の段との関係は次の通り
//		ファイル----- ファイル操作系	(1段目32個: 1-32)
//		編集--------- 編集系			(2段目32個: 33-64)
//		移動--------- カーソル移動系	(3段目32個: 65-96)
//		選択--------- 選択系			(4段目32個: 97-128)
//					+ 矩形選択系		(5段目32個: 129-160) //(注. 矩形選択系のほとんどは未実装)
//					+ クリップボード系	(6段目24個: 161-184)
//			★挿入系					(6段目残りの8個: 185-192)
//		変換--------- 変換系			(7段目32個: 193-224)
//		検索--------- 検索系			(8段目32個: 225-256)
//		ツール------- モード切り替え系	(9段目4個: 257-260)
//					+ 設定系			(9段目次の16個: 261-276)
//					+ マクロ系			(9段目最後の11個: 277-287)
//					+ 外部マクロ		(12段目32個: 353-384/13段目19個: 385-403)
//					+ カスタムメニュー	(10段目25個: 289-313)
//		ウィンドウ--- ウィンドウ系		(11段目22個: 321-342)
//					+ タブ系			(10段目残りの7個: 314-320/9段目最期の1個: 288)
//		ヘルプ------- 支援				(11段目残りの10個: 343-352)
//	注1.「挿入系」はメニューでは「編集」に入っている
//	注2.「コマンド一覧」に入ってないコマンドもわかっている範囲で位置予約にしておいた
//  注3. F_DISABLE は未定義用(ダミーとしても使う)
//	注4. ユーザー用に確保された場所は特にないので各段の空いている後ろの方を使ってください。
//	注5. アイコンビットマップの有効段数は、ImageListMgr の MAX_Y です。

	static const int tbd[] = {
/* ファイル操作系(1段目32個: 1-32) */
/*  1 */		F_FILENEW					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 新規作成
/*  2 */		F_FILEOPEN					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 開く
/*  3 */		F_FILESAVE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 上書き保存
/*  4 */		F_FILESAVEAS_DIALOG			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 名前を付けて保存
/*  5 */		F_FILECLOSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 閉じて(無題)
/*  6 */		F_FILECLOSE_OPEN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 閉じて開く
/*  7 */		F_FILE_REOPEN_SJIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SJISで開き直す
/*  8 */		F_FILE_REOPEN_JIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// JISで開き直す
/*  9 */		F_FILE_REOPEN_EUC			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// EUCで開き直す
/* 10 */		F_FILE_REOPEN_UNICODE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Unicodeで開き直す
/* 11 */		F_FILE_REOPEN_UTF8			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UTF-8で開き直す
/* 12 */		F_FILE_REOPEN_UTF7			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UTF-7で開き直す
/* 13 */		F_PRINT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 印刷
/* 14 */		F_PRINT_PREVIEW				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 印刷Preview
/* 15 */		F_PRINT_PAGESETUP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 印刷ページ設定
/* 16 */		F_OPEN_HfromtoC				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 同名のC/C++ヘッダ(ソース)を開く
/* 17 */		F_DISABLE	/*F_OPEN_HHPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 同名のC/C++ヘッダファイルを開く
/* 18 */		F_DISABLE	/*F_OPEN_CCPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 同名のC/C++ソースファイルを開く
/* 19 */		F_ACTIVATE_SQLPLUS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Oracle SQL*Plusをアクティブ表示
/* 20 */		F_PLSQL_COMPILE_ON_SQLPLUS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Oracle SQL*Plusで実行
/* 21 */		F_BROWSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ブラウズ
/* 22 */		F_PROPERTY_FILE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ファイルのプロパティ
/* 23 */		F_VIEWMODE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ビューモード
/* 24 */		F_FILE_REOPEN_UNICODEBE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UnicodeBEで開き直す
/* 25 */		F_FILEOPEN_DROPDOWN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 開く(ドロップダウン)
/* 26 */		F_FILE_REOPEN				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 開きなおす
/* 27 */		F_EXITALL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// サクラエディタの全終了
/* 28 */		F_FILESAVECLOSE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 保存して閉じる
/* 29 */		F_FILENEW_NEWWINDOW			/* , TBSTATE_ENABLED, TBSTATE_BUTTON, 0, 0 */,	// 新規ウィンドウを開く
/* 30 */		F_FILESAVEALL				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 全て上書き保存
/* 31 */		F_EXITALLEDITORS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 編集の全終了
/* 32 */		F_FILE_REOPEN_CESU8			/* . TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,  // CESU-8で開きなおす

/* 編集系(2段目32個: 32-64) */
/* 33 */		F_UNDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 元に戻す(Undo)
/* 34 */		F_REDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// やり直し(Redo)
/* 35 */		F_DELETE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 削除
/* 36 */		F_DELETE_BACK					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル前を削除
/* 37 */		F_WordDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語の左端まで削除
/* 38 */		F_WordDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語の右端まで削除
/* 39 */		F_WordDelete					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語削除
/* 40 */		F_WordCut						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語切り取り
/* 41 */		F_LineDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行頭まで削除(改行単位)
/* 42 */		F_LineDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行末まで削除(改行単位)
/* 43 */		F_LineCutToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行頭まで切り取り(改行単位)
/* 44 */		F_LineCutToEnd					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行末まで切り取り(改行単位)
/* 45 */		F_DELETE_LINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行削除(折り返し単位)
/* 46 */		F_CUT_LINE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行切り取り(改行単位)
/* 47 */		F_DUPLICATELINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行の二重化(折り返し単位)
/* 48 */		F_INDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// TABインデント
/* 49 */		F_UNINDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 逆TABインデント
/* 50 */		F_INDENT_SPACE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SPACEインデント
/* 51 */		F_UNINDENT_SPACE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 逆SPACEインデント
/* 52 */		F_DISABLE/*F_WORDSREFERENCE*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語リファレンス	// アイコン未作
/* 53 */		F_LTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// LTRIM
/* 54 */		F_RTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// RTRIM
/* 55 */		F_SORT_ASC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SORT_ASC
/* 56 */		F_SORT_DESC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SORT_DES
/* 57 */		F_MERGE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// MERGE
/* 58 */		F_RECONVERT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 再変換
/* 59 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 60 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 61 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 62 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 63 */		F_PROFILEMGR					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//プロファイルマネージャ
/* 64 */		F_FILE_REOPEN_LATIN1			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Latin1で開きなおす

/* カーソル移動系(3段目32個: 65-96) */
/* 65 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 66 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 67 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 68 */		F_UP							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル上移動
/* 69 */		F_DOWN							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル下移動
/* 70 */		F_LEFT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル左移動
/* 71 */		F_RIGHT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル右移動
/* 72 */		F_UP2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル上移動(２行ごと)
/* 73 */		F_DOWN2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル下移動(２行ごと)
/* 74 */		F_WORDLEFT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語の左端に移動
/* 75 */		F_WORDRIGHT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 単語の右端に移動
/* 76 */		F_GOLINETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行頭に移動(折り返し単位)
/* 77 */		F_GOLINEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 行末に移動(折り返し単位)
/* 78 */		F_HalfPageUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半ページアップ
/* 79 */		F_HalfPageDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半ページダウン
/* 80 */		F_1PageUp						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// １ページアップ
/* 81 */		F_1PageDown						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// １ページダウン
/* 82 */		F_DISABLE/*F_DISPLAYTOP*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 画面の先頭に移動(未実装)
/* 83 */		F_DISABLE/*F_DISPLAYEND*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 画面の最後に移動(未実装)
/* 84 */		F_GOFILETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ファイルの先頭に移動
/* 85 */		F_GOFILEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ファイルの最後に移動
/* 86 */		F_CURLINECENTER					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カーソル行をウィンドウ中央へ
/* 87 */		F_JUMPHIST_PREV					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 移動履歴: 前へ
/* 88 */		F_JUMPHIST_NEXT					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 移動履歴: 次へ
/* 89 */		F_WndScrollDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// テキストを１行下へスクロール
/* 90 */		F_WndScrollUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// テキストを１行上へスクロール
/* 91 */		F_GONEXTPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次の段落へ
/* 92 */		F_GOPREVPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前の段落へ
/* 93 */		F_JUMPHIST_SET					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 現在位置を移動履歴に登録
/* 94 */		F_MODIFYLINE_PREV				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の変更行へ
/* 95 */		F_MODIFYLINE_NEXT				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の変更行へ
/* 96 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 選択系(4段目32個: 97-128) */
/* 97 */		F_SELECTWORD					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 現在位置の単語選択
/* 98 */		F_SELECTALL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// すべて選択
/* 99 */		F_BEGIN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 範囲選択開始
/* 100 */		F_UP_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル上移動
/* 101 */		F_DOWN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル下移動
/* 102 */		F_LEFT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル左移動
/* 103 */		F_RIGHT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル右移動
/* 104 */		F_UP2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル上移動(２行ごと)
/* 105 */		F_DOWN2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)カーソル下移動(２行ごと)
/* 106 */		F_WORDLEFT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)単語の左端に移動
/* 107 */		F_WORDRIGHT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)単語の右端に移動
/* 108 */		F_GOLINETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)行頭に移動(折り返し単位)
/* 109 */		F_GOLINEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)行末に移動(折り返し単位)
/* 110 */		F_HalfPageUp_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)半ページアップ
/* 111 */		F_HalfPageDown_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)半ページダウン
/* 112 */		F_1PageUp_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)１ページアップ
/* 113 */		F_1PageDown_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)１ページダウン
/* 114 */		F_DISABLE/*F_DISPLAYTOP_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)画面の先頭に移動(未実装)
/* 115 */		F_DISABLE/*F_DISPLAYEND_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)画面の最後に移動(未実装)
/* 116 */		F_GOFILETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)ファイルの先頭に移動
/* 117 */		F_GOFILEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)ファイルの最後に移動
/* 118 */		F_GONEXTPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)次の段落へ
/* 119 */		F_GOPREVPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (範囲選択)前の段落へ
/* 120 */		F_SELECTLINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 1行選択
/* 121 */		F_FUNCLIST_PREV					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の関数リストマーク
/* 122 */		F_FUNCLIST_NEXT					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の関数リストマーク
/* 123 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 124 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 125 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 126 */		F_MODIFYLINE_PREV_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)前の変更行へ
/* 127 */		F_MODIFYLINE_NEXT_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)次の変更行へ
/* 128 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 矩形選択系(5段目32個: 129-160) */ //(注. 矩形選択系のほとんどは未実装)
/* 129 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 130 */		F_DISABLE/*F_BOXSELALL*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 矩形ですべて選択
/* 131 */		F_BEGIN_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 矩形範囲選択開始
/* 132 */		F_UP_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル上移動
/* 133 */		F_DOWN_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル下移動
/* 134 */		F_LEFT_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル左移動
/* 135 */		F_RIGHT_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル右移動
/* 136 */		F_UP2_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル上移動(２行ごと)
/* 137 */		F_DOWN2_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)カーソル下移動(２行ごと)
/* 138 */		F_WORDLEFT_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)単語の左端に移動
/* 139 */		F_WORDRIGHT_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)単語の右端に移動
/* 140 */		F_GOLINETOP_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)行頭に移動(折り返し単位)
/* 141 */		F_GOLINEEND_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)行末に移動(折り返し単位)
/* 142 */		F_HalfPageUp_BOX				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)半ページアップ
/* 143 */		F_HalfPageDown_BOX				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)半ページダウン
/* 144 */		F_1PageUp_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)１ページアップ
/* 145 */		F_1PageDown_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)１ページダウン
/* 146 */		F_DISABLE/*F_DISPLAYTOP_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)画面の先頭に移動(未実装)
/* 147 */		F_DISABLE/*F_DISPLAYEND_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)画面の最後に移動(未実装)
/* 148 */		F_GOFILETOP_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)ファイルの先頭に移動
/* 149 */		F_GOFILEEND_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)ファイルの最後に移動
/* 150 */		F_GOLOGICALLINETOP_BOX			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// (矩形選択)行頭に移動(改行単位)
/* 151 */		F_DISABLE/*F_GOLOGICALLINEEND_BOX*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 152 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 153 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 154 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 155 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 156 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 157 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 158 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 159 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 160 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* クリップボード系(6段目24個: 161-184) */
/* 161 */		F_CUT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 切り取り(選択範囲をクリップボードにコピーして削除)
/* 162 */		F_COPY							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// コピー(選択範囲をクリップボードにコピー)
/* 163 */		F_COPY_CRLF						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// CRLF改行でコピー
/* 164 */		F_PASTE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 貼り付け(クリップボードから貼り付け)
/* 165 */		F_PASTEBOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 矩形貼り付け(クリップボードから貼り付け)
/* 166 */		F_DISABLE/*F_INSTEXT_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// テキストを貼り付け	(未公開コマンド？未完成？)
/* 167 */		F_DISABLE/*F_ADDTAIL_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 最後にテキストを追加	(未公開コマンド？未完成？)
/* 168 */		F_COPYLINES						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 選択範囲内全行コピー
/* 169 */		F_COPYLINESASPASSAGE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 選択範囲内全行引用符付きコピー
/* 170 */		F_COPYLINESWITHLINENUMBER		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 選択範囲内全行行番号付きコピー
/* 171 */		F_COPYPATH						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// このファイルのパス名をコピー
/* 172 */		F_COPYTAG						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// このファイルのパス名とカーソル位置をコピー
/* 173 */		F_CREATEKEYBINDLIST				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キー割り当て一覧をコピー
/* 174 */		F_COPYFNAME						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// このファイル名をクリップボードにコピー
/* 175 */		F_COPY_ADDCRLF					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 折り返し位置に改行をつけてコピー
/* 176 */		F_COPY_COLOR_HTML				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 選択範囲内色付きHTMLコピー
/* 177 */		F_COPY_COLOR_HTML_LINENUMBER	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 選択範囲内行番号色付きHTMLコピー
/* 178 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 179 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 180 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 181 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 182 */		F_CHGMOD_EOL_CRLF,
/* 183 */		F_CHGMOD_EOL_LF,
/* 184 */		F_CHGMOD_EOL_CR,

/* 挿入系(6段目残り8個: 185-192) */
/* 185 */		F_INS_DATE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 日付挿入
/* 186 */		F_INS_TIME	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 時刻挿入
/* 187 */		F_CTRL_CODE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// コントロールコードの入力(ダイアログ)
/* 188 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 189 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 190 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 191 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 192 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 変換系(7段目32個: 193-224) */
/* 193 */		F_TOLOWER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 小文字
/* 194 */		F_TOUPPER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 大文字
/* 195 */		F_TOHANKAKU				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 全角→半角
/* 196 */		F_TOZENKAKUKATA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半角＋全ひら→全角・カタカナ
/* 197 */		F_TOZENKAKUHIRA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半角＋全カタ→全角・ひらがな
/* 198 */		F_HANKATATOZENKATA		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半角カタカナ→全角カタカナ
/* 199 */		F_HANKATATOZENHIRA		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半角カタカナ→全角ひらがな
/* 200 */		F_TABTOSPACE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// TAB→空白
/* 201 */		F_CODECNV_AUTO2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 自動判別→SJISコード変換
/* 202 */		F_CODECNV_EMAIL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// E-Mail(JIS→SIJIS)コード変換
/* 203 */		F_CODECNV_EUC2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// EUC→SJISコード変換
/* 204 */		F_CODECNV_UNICODE2SJIS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Unicode→SJISコード変換
/* 205 */		F_CODECNV_UTF82SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UTF-8→SJISコード変換
/* 206 */		F_CODECNV_UTF72SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UTF-7→SJISコード変換
/* 207 */		F_CODECNV_SJIS2JIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SJIS→JISコード変換
/* 208 */		F_CODECNV_SJIS2EUC		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SJIS→EUCコード変換
/* 209 */		F_CODECNV_SJIS2UTF8		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SJIS→UTF-8コード変換
/* 210 */		F_CODECNV_SJIS2UTF7		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// SJIS→UTF-7コード変換
/* 211 */		F_BASE64DECODE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Base64デコードして保存
/* 212 */		F_UUDECODE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// uudecodeしてファイルに保存
/* 213 */		F_SPACETOTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 空白→TAB
/* 214 */		F_TOZENEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 半角英数→全角英数
/* 215 */		F_TOHANEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 全角英数→半角英数
/* 216 */		F_CODECNV_UNICODEBE2SJIS/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// UnicodeBE→SJISコード変換
/* 217 */		F_TOHANKATA				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 全角カタカナ→半角カタカナ
/* 218 */		F_FILETREE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルツリー表示
/* 219 */		F_SHOWMINIMAP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ミニマップを表示
/* 220 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 221 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 222 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 223 */		F_TAGJUMP_CLOSE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 閉じてタグジャンプ(元ウィンドウclose)
/* 224 */		F_OUTLINE_TOGGLE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// アウトライン解析(toggle)

/* 検索系(8段目32個: 225-256) */
/* 225 */		F_SEARCH_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 検索(単語検索ダイアログ)
/* 226 */		F_SEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次を検索
/* 227 */		F_SEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前を検索
/* 228 */		F_REPLACE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 置換
/* 229 */		F_SEARCH_CLEARMARK	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 検索マークのクリア
/* 230 */		F_GREP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// Grep
/* 231 */		F_JUMP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 指定行へジャンプ
/* 232 */		F_OUTLINE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// アウトライン解析
/* 233 */		F_TAGJUMP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タグジャンプ機能
/* 234 */		F_TAGJUMPBACK		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タグジャンプバック機能
/* 235 */		F_COMPARE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ファイル内容比較
/* 236 */		F_BRACKETPAIR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 対括弧の検索
/* 237 */		F_BOOKMARK_SET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ブックマーク設定・解除
/* 238 */		F_BOOKMARK_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次のブックマークへ
/* 239 */		F_BOOKMARK_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前のブックマークへ
/* 240 */		F_BOOKMARK_RESET	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ブックマークの全解除
/* 241 */		F_BOOKMARK_VIEW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ブックマークの一覧
/* 242 */		F_DIFF_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// DIFF差分表示
/* 243 */		F_DIFF_NEXT			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次の差分へ
/* 244 */		F_DIFF_PREV			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前の差分へ
/* 245 */		F_DIFF_RESET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 差分の全解除
/* 246 */		F_SEARCH_BOX		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 検索(ボックス)
/* 247 */		F_JUMP_SRCHSTARTPOS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 検索開始位置へ戻る
/* 248 */		F_TAGS_MAKE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タグファイルの作成
/* 249 */		F_DIRECT_TAGJUMP	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダイレクトタグジャンプ
/* 250 */		F_ISEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前方インクリメンタルサーチ
/* 251 */		F_ISEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 後方インクリメンタルサーチ
/* 252 */		F_ISEARCH_REGEXP_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 正規表現前方インクリメンタルサーチ
/* 253 */		F_ISEARCH_REGEXP_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 正規表現前方インクリメンタルサーチ
/* 254 */		F_ISEARCH_MIGEMO_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// MIGEMO前方インクリメンタルサーチ
/* 255 */		F_ISEARCH_MIGEMO_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// MIGEMO前方インクリメンタルサーチ
/* 256 */		F_TAGJUMP_KEYWORD	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーワードを指定してダイレクトタグジャンプ

/* モード切り替え系(9段目4個: 257-260) */
/* 257 */		F_CHGMOD_INS	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 挿入／上書きモード切り替え
/* 258 */		F_CANCEL_MODE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 各種モードの取り消し
/* 259 */		F_CHG_CHARSET	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 文字コードセット指定
/* 260 */		F_GREP_REPLACE_DLG	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Grep置換

/* 設定系(9段目次の16個: 261-276) */
/* 261 */		F_SHOWTOOLBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ツールバーの表示
/* 262 */		F_SHOWFUNCKEY		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ファンクションキーの表示
/* 263 */		F_SHOWSTATUSBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ステータスバーの表示
/* 264 */		F_TYPE_LIST			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タイプ別設定一覧
/* 265 */		F_OPTION_TYPE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タイプ別設定
/* 266 */		F_OPTION			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 共通設定
/* 267 */		F_FONT				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// フォント設定
/* 268 */		F_WRAPWINDOWWIDTH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 現在のウィンドウ幅で折り返し
/* 269 */		F_FAVORITE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 履歴の管理
/* 270 */		F_SHOWTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タブの表示
/* 271 */		F_DISABLE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 272 */		F_TOGGLE_KEY_SEARCH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーワードヘルプ自動表示
/* 273 */		F_TMPWRAPNOWRAP		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 折り返さない（一時設定）
/* 274 */		F_TMPWRAPSETTING	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 指定桁で折り返す（一時設定）
/* 275 */		F_TMPWRAPWINDOW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 右端で折り返す（一時設定）
/* 276 */		F_SELECT_COUNT_MODE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 文字カウント方法

/* マクロ系(9段目最後の12個: 277-288) */
/* 277 */		F_RECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーマクロの記録開始／終了
/* 278 */		F_SAVEKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーマクロの保存
/* 279 */		F_LOADKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーマクロの読み込み
/* 280 */		F_EXECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// キーマクロの実行
/* 281 */		F_EXECMD_DIALOG			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部コマンド実行
/* 282 */		F_EXECEXTMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 名前を指定してマクロ実行
/* 283 */		F_PLUGCOMMAND			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// プラグインコマンド用に予約
/* 284 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 285 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 286 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 287 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 288 */		F_TAB_CLOSEOTHER		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// このタブ以外を閉じる

/* カスタムメニュー(10段目25個: 289-313) */
/* 289 */		F_MENU_RBUTTON				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 右クリックメニュー
/* 290 */		F_CUSTMENU_1				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー1
/* 291 */		F_CUSTMENU_2				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー2
/* 292 */		F_CUSTMENU_3				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー3
/* 293 */		F_CUSTMENU_4				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー4
/* 294 */		F_CUSTMENU_5				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー5
/* 295 */		F_CUSTMENU_6				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー6
/* 296 */		F_CUSTMENU_7				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー7
/* 297 */		F_CUSTMENU_8				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー8
/* 298 */		F_CUSTMENU_9				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー9
/* 299 */		F_CUSTMENU_10				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー10
/* 300 */		F_CUSTMENU_11				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー11
/* 301 */		F_CUSTMENU_12				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー12
/* 302 */		F_CUSTMENU_13				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー13
/* 303 */		F_CUSTMENU_14				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー14
/* 304 */		F_CUSTMENU_15				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー15
/* 305 */		F_CUSTMENU_16				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー16
/* 306 */		F_CUSTMENU_17				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー17
/* 307 */		F_CUSTMENU_18				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー18
/* 308 */		F_CUSTMENU_19				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー19
/* 309 */		F_CUSTMENU_20				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー20
/* 310 */		F_CUSTMENU_21				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー21
/* 311 */		F_CUSTMENU_22				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー22
/* 312 */		F_CUSTMENU_23				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー23
/* 313 */		F_CUSTMENU_24				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// カスタムメニュー24

/* ウィンドウ系(10段目7個: 314-320) */
/* 314 */		F_TAB_MOVERIGHT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タブを右に移動
/* 315 */		F_TAB_MOVELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// タブを左に移動
/* 316 */		F_TAB_SEPARATE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 新規グループ
/* 317 */		F_TAB_JOINTNEXT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次のグループに移動
/* 318 */		F_TAB_JOINTPREV				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前のグループに移動
/* 319 */		F_TAB_CLOSERIGHT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 右をすべて閉じる
/* 320 */		F_TAB_CLOSELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 左をすべて閉じる

/* ウィンドウ系(11段目22個: 321-342) */
/* 321 */		F_SPLIT_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 上下に分割
/* 322 */		F_SPLIT_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 左右に分割
/* 323 */		F_SPLIT_VH		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 縦横に分割
/* 324 */		F_WINCLOSE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ウィンドウを閉じる
/* 325 */		F_WIN_CLOSEALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// すべてのウィンドウを閉じる
/* 329 */		F_NEXTWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次のウィンドウ
/* 330 */		F_PREVWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前のウィンドウ
/* 326 */		F_CASCADE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 重ねて表示
/* 237 */		F_TILE_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 上下に並べて表示
/* 328 */		F_TILE_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 左右に並べて表示
/* 331 */		F_MAXIMIZE_V	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 縦方向に最大化
/* 332 */		F_MAXIMIZE_H	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 横方向に最大化
/* 333 */		F_MINIMIZE_ALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// すべて最小化
/* 334 */		F_REDRAW		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 再描画
/* 335 */		F_WIN_OUTPUT	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// アウトプットウィンドウ表示
/* 336 */		F_BIND_WINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 結合して表示
/* 337 */		F_TOPMOST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 常に手前に表示
/* 338 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 339 */		F_WINLIST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ウィンドウ一覧ポップアップ表示
/* 340 */		F_GROUPCLOSE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// グループを閉じる
/* 341 */		F_NEXTGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 次のグループ
/* 342 */		F_PREVGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 前のグループ

/* 支援(11段目残りの10個: 343-352) */
/* 343 */		F_HOKAN			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 入力補完
/* 344 */		F_HELP_CONTENTS /*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ヘルプ目次
/* 345 */		F_HELP_SEARCH	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ヘルプキーワード検索
/* 346 */		F_MENU_ALLFUNC	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// コマンド一覧
/* 347 */		F_EXTHELP1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部ヘルプ１
/* 348 */		F_EXTHTMLHELP	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部HTMLヘルプ
/* 349 */		F_ABOUT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// バージョン情報
/* 350 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 351 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 352 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 外部マクロ(12段目31個: 353-383) */
/* 353 */		F_USERMACRO_0+0		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�@
/* 354 */		F_USERMACRO_0+1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�A
/* 355 */		F_USERMACRO_0+2		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�B
/* 356 */		F_USERMACRO_0+3		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�C
/* 357 */		F_USERMACRO_0+4		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�D
/* 358 */		F_USERMACRO_0+5		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�E
/* 359 */		F_USERMACRO_0+6		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�F
/* 360 */		F_USERMACRO_0+7		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�G
/* 361 */		F_USERMACRO_0+8		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�H
/* 362 */		F_USERMACRO_0+9		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�I
/* 363 */		F_USERMACRO_0+10	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�J
/* 364 */		F_USERMACRO_0+11	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�K
/* 365 */		F_USERMACRO_0+12	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�L
/* 366 */		F_USERMACRO_0+13	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�M
/* 367 */		F_USERMACRO_0+14	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�N
/* 368 */		F_USERMACRO_0+15	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�O
/* 369 */		F_USERMACRO_0+16	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�P
/* 370 */		F_USERMACRO_0+17	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�Q
/* 371 */		F_USERMACRO_0+18	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�R
/* 372 */		F_USERMACRO_0+19	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ�S
/* 373 */		F_USERMACRO_0+20	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ21
/* 374 */		F_USERMACRO_0+21	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ22
/* 375 */		F_USERMACRO_0+22	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ23
/* 376 */		F_USERMACRO_0+23	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ24
/* 377 */		F_USERMACRO_0+24	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ25
/* 378 */		F_USERMACRO_0+25	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ26
/* 379 */		F_USERMACRO_0+26	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ27
/* 380 */		F_USERMACRO_0+27	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ28
/* 381 */		F_USERMACRO_0+28	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ29
/* 382 */		F_USERMACRO_0+29	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ30
/* 383 */		F_USERMACRO_0+30	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ31
//	384は折り返しマークとして使用しているのでアイコンとしては使用できない
//	アイコン位置のみ追加マクロ用として利用する
/* 384 */		F_TOOLBARWRAP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 追加マクロ用icon位置兼、折返ツールバーボタンID

/* 外部マクロ(13段目19個: 385-403) */
/* 385 */		F_USERMACRO_0+31	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ32
/* 386 */		F_USERMACRO_0+32	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ33
/* 387 */		F_USERMACRO_0+33	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ34
/* 388 */		F_USERMACRO_0+34	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ35
/* 389 */		F_USERMACRO_0+35	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ36
/* 390 */		F_USERMACRO_0+36	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ37
/* 391 */		F_USERMACRO_0+37	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ38
/* 392 */		F_USERMACRO_0+38	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ39
/* 393 */		F_USERMACRO_0+39	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ40
/* 394 */		F_USERMACRO_0+40	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ41
/* 395 */		F_USERMACRO_0+41	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ42
/* 396 */		F_USERMACRO_0+42	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ43
/* 397 */		F_USERMACRO_0+43	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ44
/* 398 */		F_USERMACRO_0+44	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ45
/* 399 */		F_USERMACRO_0+45	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ46
/* 400 */		F_USERMACRO_0+46	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ47
/* 401 */		F_USERMACRO_0+47	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ48
/* 402 */		F_USERMACRO_0+48	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ49
/* 403 */		F_USERMACRO_0+49	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// 外部マクロ50
/* 404 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 405 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 406 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 407 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 408 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 409 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 410 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 411 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 412 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 413 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 414 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 415 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 416 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 417 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 418 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 419 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 420 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 421 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 422 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 423 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 424 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 425 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 426 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 427 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 428 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 429 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 430 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 431 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 432 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 433 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 434 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 435 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 436 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 437 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 438 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 439 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 440 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 441 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 442 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 443 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 444 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 445 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 446 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 447 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 448 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 449 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 450 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 451 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 452 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 453 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 454 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 455 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 456 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 457 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 458 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 459 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 460 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 461 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 462 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 463 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 464 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 465 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 466 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 467 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 468 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 469 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 470 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 471 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 472 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 473 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 474 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 475 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 476 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 477 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 478 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 479 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー
/* 480 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	// ダミー

/* 481 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */	// 最終行用ダミー(最終行末にはカンマを付けないこと)

	};
	int tbd_num = _countof(tbd);

	// tbMyButton[0]にはセパレータが入っているため、アイコン番号とボタン番号は１つずれる
	const int INDEX_GAP = 1;
	const int myButtonEnd = tbd_num + INDEX_GAP;
	// 定数の整合性確認
	// アイコン番号
	assert_warning(tbd[TOOLBAR_ICON_MACRO_INTERNAL      - INDEX_GAP] == F_MACRO_EXTRA);
	assert_warning(tbd[TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - INDEX_GAP] == F_PLUGCOMMAND);
	// コマンド番号
	assert_warning(tbd[TOOLBAR_BUTTON_F_TOOLBARWRAP     - INDEX_GAP] == F_TOOLBARWRAP);
	tbMyButton.resize(tbd_num + INDEX_GAP);
	SetTBBUTTONVal(&tbMyButton[0], -1, F_SEPARATOR, 0, TBSTYLE_SEP, 0, 0);	// セパレータ	アイコンの未定義化(-1)

	// ループインデックスの基準をtbMyButtonに変更
	for (int i=INDEX_GAP; i<myButtonEnd; ++i) {
		const int funcCode = tbd[i-INDEX_GAP];
		const int imageIndex = i - INDEX_GAP;

		if (funcCode == F_TOOLBARWRAP) {
			// ツールバー改行用の仮想ボタン（実際は表示されない）
			//	折り返しボタンが最後のデータと重なっているが，
			//	インデックスを変更するとsakura.iniが引き継げなくなるので
			//	重複を承知でそのままにする
			//	アイコン位置は外部マクロのデフォルトアイコンとして利用中
			//	tbMyButton[384]自体は、ツールバーの折り返し用
			SetTBBUTTONVal(
				&tbMyButton[i],
				-1,						// アイコンの未定義化(-1)
				F_MENU_NOT_USED_FIRST,
				TBSTATE_ENABLED|TBSTATE_WRAP,
				TBSTYLE_SEP, 0, 0
			);
			continue;
		}

		BYTE	style;
		switch (funcCode) {
		case F_FILEOPEN_DROPDOWN:
			style = TBSTYLE_DROPDOWN;	// ドロップダウン
			break;

		case F_SEARCH_BOX:
			style = TBSTYLE_COMBOBOX;	// コンボボックス
			break;

		default:
			style = TBSTYLE_BUTTON;	// ボタン
			break;
		}

		SetTBBUTTONVal(
			&tbMyButton[i],
			(F_DUMMY_MAX_CODE < funcCode)? imageIndex : -1,	// アイコンの未定義化(-1)
			funcCode,
			(tbd[i] == F_DISABLE)? 0 : TBSTATE_ENABLED,	// F_DISABLE なら DISABLEに
			style, 0, 0
		);
	}
	
	nMyButtonFixSize = tbMyButton.size();
	
	// 専用アイコンのない外部マクロがあれば、同じアイコンを共有して登録
	if (MAX_CUSTMACRO_ICO < MAX_CUSTMACRO) {
		const int nAddFuncs = MAX_CUSTMACRO - MAX_CUSTMACRO_ICO;
		const size_t nBaseIndex = tbMyButton.size();
		tbMyButton.resize(tbMyButton.size() + nAddFuncs);
		for (int k=0; k<nAddFuncs; ++k) {
			const int macroFuncCode = F_USERMACRO_0 + MAX_CUSTMACRO_ICO + k;
			SetTBBUTTONVal(
				&tbMyButton[k + nBaseIndex],
				TOOLBAR_ICON_MACRO_INTERNAL - INDEX_GAP,
				macroFuncCode, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0
			);
		}
	}
	
	nMyButtonNum = tbMyButton.size();
	return;
}


MenuDrawer::~MenuDrawer()
{
	if (hFontMenu) {
		::DeleteObject(hFontMenu);
		hFontMenu = NULL;
	}
	DeleteCompDC();
	return;
}

void MenuDrawer::Create(
	HINSTANCE hInstance,
	HWND hWndOwner,
	ImageListMgr* pIcons
	)
{
	this->hInstance = hInstance;
	this->hWndOwner = hWndOwner;
	this->pIcons = pIcons;

	return;
}


void MenuDrawer::ResetContents(void)
{
	LOGFONT	lf;
	menuItems.clear();

	NONCLIENTMETRICS ncm = {0};

	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する
	ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0);

	if (hFontMenu) {
		::DeleteObject(hFontMenu);
		hFontMenu = NULL;
	}
	lf = ncm.lfMenuFont;
	hFontMenu = ::CreateFontIndirect(&lf);
	nMenuFontHeight = lf.lfHeight;
	if (nMenuFontHeight < 0) {
		nMenuFontHeight = -nMenuFontHeight;
	}else {
		// ポイント(1/72インチ)をピクセルへ
		nMenuFontHeight = DpiScaleY(nMenuFontHeight);
		if (nMenuFontHeight == -1) {
			nMenuFontHeight = lf.lfHeight;
		}
	}
	nMenuHeight = nMenuFontHeight + 4; // margin
	if (pShareData->common.window.bMenuIcon) {
		// 最低アイコン分の高さを確保
    nMenuHeight = std::max(nMenuHeight, pIcons->GetCy() + 4);
	}
	return;
}


/* メニュー項目を追加 */
void MenuDrawer::MyAppendMenu(
	HMENU			hMenu,
	int				nFlag,
	UINT_PTR		nFuncId,
	const TCHAR*	pszLabel,
	const TCHAR*	pszKey,
	bool			bAddKeyStr,
	int				nForceIconId	//お気に入り
	)
{
	TCHAR	szLabel[_MAX_PATH * 2+ 30];
	TCHAR	szKey[10];
	int		nFlagAdd = 0;

	if (nForceIconId == -1) {
		nForceIconId = nFuncId;	// お気に入り
	}

	szLabel[0] = _T('\0');
	if (pszLabel) {
		_tcsncpy(szLabel, pszLabel, _countof(szLabel) - 1);
		szLabel[_countof(szLabel) - 1] = _T('\0');
	}
	auto_strcpy(szKey, pszKey); 
	if (nFuncId != 0) {
		// メニューラベルの作成
		auto& csKeyBind = pShareData->common.keyBind;
		KeyBind::GetMenuLabel(
			hInstance,
			csKeyBind.nKeyNameArrNum,
			csKeyBind.pKeyNameArr,
			nFuncId,
			szLabel,
			szKey,
			bAddKeyStr,
			_countof(szLabel)
		);

		// アイコン用ビットマップを持つものは、オーナードロウにする
		{
			MyMenuItemInfo item;
			item.nBitmapIdx = -1;
			item.nFuncId = nFuncId;
			item.memLabel.SetString(szLabel);
			// メニュー項目をオーナー描画にして、アイコンを表示する
			// アクセスキーの分を詰めるためいつもオーナードローにする。ただしVista未満限定
			// Vista以上ではメニューもテーマが適用されるので、オーナードローにすると見た目がXP風になってしまう。
			if (pShareData->common.window.bMenuIcon || !IsWinVista_or_later()) {
				nFlagAdd = MF_OWNERDRAW;
			}
			// 機能のビットマップの情報を覚えておく
			item.nBitmapIdx = GetIconIdByFuncId(nForceIconId);
			menuItems.push_back(item);
		}
	}else {
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
		// セパレータかサブメニュー
		if (nFlag & (MF_SEPARATOR | MF_POPUP)) {
			if (pShareData->common.window.bMenuIcon || !IsWinVista_or_later()) {
					nFlagAdd = MF_OWNERDRAW;
			}
		}
#endif
	}

	MENUITEMINFO mii = {0};
	mii.cbSize = SIZEOF_MENUITEMINFO; // Win95対策済みのsizeof(MENUITEMINFO)値

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = 0;
	if (MF_OWNERDRAW	& (nFlag | nFlagAdd)) mii.fType |= MFT_OWNERDRAW;
	if (MF_SEPARATOR	& (nFlag | nFlagAdd)) mii.fType |= MFT_SEPARATOR;
	if (MF_STRING		& (nFlag | nFlagAdd)) mii.fType |= MFT_STRING;
	if (MF_MENUBREAK	& (nFlag | nFlagAdd)) mii.fType |= MFT_MENUBREAK;
	if (MF_MENUBARBREAK	& (nFlag | nFlagAdd)) mii.fType |= MFT_MENUBARBREAK;

	mii.fState = 0;
	if (MF_GRAYED		& (nFlag | nFlagAdd)) mii.fState |= MFS_GRAYED;
	if (MF_CHECKED		& (nFlag | nFlagAdd)) mii.fState |= MFS_CHECKED;

	mii.wID = nFuncId;
	mii.hSubMenu = (nFlag & MF_POPUP) ? ((HMENU)nFuncId) : NULL;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwItemData = (ULONG_PTR)this;
	mii.dwTypeData = szLabel;
	mii.cch = 0;

	// メニュー内の指定された位置に、新しいメニュー項目を挿入します。
	::InsertMenuItem(hMenu, 0xFFFFFFFF, TRUE, &mii);
	return;
}


/*
	ツールバー番号をボタン配列のindexに変換する
*/
inline int MenuDrawer::ToolbarNoToIndex(int nToolbarNo) const
{
	if (nToolbarNo < 0) {
		return -1;
	}
	// 固定アクセス分のみ直接番号でアクセスさせる。nMyButtonNum は使わない
	if (0 <= nToolbarNo && nToolbarNo < (int)nMyButtonFixSize) {
		return nToolbarNo;
	}
	int nFuncID = nToolbarNo;
	return FindIndexFromCommandId(nFuncID, false);
}
 
/*
	ツールバー番号からアイコン番号を取得
*/
inline int MenuDrawer::GetIconIdByFuncId(int nFuncID) const
{
	int index = FindIndexFromCommandId(nFuncID, false);
	if (index < 0) {
		return -1;
	}
	return tbMyButton[index].iBitmap;
}


/*! メニューアイテムの描画サイズを計算
	@param pnItemHeight [out] 高さ。いつも高さを返す
	@retval 0  機能がない場合
	@retval 1 <= val 機能のメニュー幅/セパレータの場合はダミーの値
*/
int MenuDrawer::MeasureItem(int nFuncID, int* pnItemHeight)
{
	const TCHAR* pszLabel;
	Rect rc, rcSp;
	HDC hdc;
	HFONT hFontOld;

	if (nFuncID == F_0) { // F_0, なぜか F_SEPARATOR ではない
		// セパレータ。フォントの方の通常項目の半分の高さ
		*pnItemHeight = nMenuFontHeight / 2;
		return 30; // ダミーの幅
	}else if (!(pszLabel = GetLabel(nFuncID))) {
		*pnItemHeight = nMenuHeight;
		return 0;
	}
	*pnItemHeight = nMenuHeight;

	hdc = ::GetDC(hWndOwner);
	hFontOld = (HFONT)::SelectObject(hdc, hFontMenu);
	// DT_EXPANDTABSをやめる
	::DrawText(hdc, pszLabel, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(hWndOwner, hdc);

	int nMenuWidth = rc.Width() + 3;
	if (pShareData->common.window.bMenuIcon) {
		nMenuWidth += pIcons->GetCx() + 12 + DpiScaleX(8); // アイコンと枠 + アクセスキー隙間
	}else {
		// WM_MEASUREITEMで報告するメニュー幅より実際の幅は1文字分相当位広いので、その分は加えない
		nMenuWidth += ::GetSystemMetrics(SM_CXMENUCHECK) + 2 + 2;
	}
	return nMenuWidth;
}


/*! メニューアイテム描画 */
void MenuDrawer::DrawItem(DRAWITEMSTRUCT* lpdis)
{
	size_t		j;
	size_t		nItemStrLen;
	int			nIndentLeft;
	int			nIndentRight;
	HBRUSH		hBrush;
	RECT		rcText;
	int			nBkModeOld;
	int			nTxSysColor;

	const bool bMenuIconDraw = !!pShareData->common.window.bMenuIcon;
	const int nCxCheck = ::GetSystemMetrics(SM_CXMENUCHECK);
	const int nCyCheck = ::GetSystemMetrics(SM_CYMENUCHECK);

	if (bMenuIconDraw) {
		nIndentLeft  = 13 + pIcons->GetCx(); // 2+[2+16+2]+2 +5
	}else {
		nIndentLeft = 2 + 2 + nCxCheck;
	}
	// サブメニューの|＞の分は必要 最低8ぐらい
	nIndentRight = t_max(nMenuFontHeight, 8);

	// アイコンを描くときにチラつくので、バックサーフェスを使う
	const bool bBackSurface = bMenuIconDraw;
	const int nTargetWidth  = lpdis->rcItem.right - lpdis->rcItem.left;
	const int nTargetHeight = lpdis->rcItem.bottom - lpdis->rcItem.top;
	HDC hdcOrg = NULL;
	HDC hdc = NULL;
	if (bBackSurface) {
		hdcOrg = lpdis->hDC;
		if (hCompDC && nTargetWidth <= nCompBitmapWidth && nTargetHeight <= nCompBitmapHeight) {
			hdc = hCompDC;
		}else {
			if (hCompDC) {
				DeleteCompDC();
			}
			hdc = hCompDC  = ::CreateCompatibleDC(hdcOrg);
			hCompBitmap    = ::CreateCompatibleBitmap(hdcOrg, nTargetWidth + 20, nTargetHeight + 4);
			hCompBitmapOld = (HBITMAP)::SelectObject(hdc, hCompBitmap);
			nCompBitmapWidth  = nTargetWidth + 20;
			nCompBitmapHeight = nTargetHeight + 4;
		}
		::SetWindowOrgEx(hdc, lpdis->rcItem.left, lpdis->rcItem.top, NULL);
	}else {
		hdc = lpdis->hDC;
	}

	// 作画範囲を背景色で矩形塗りつぶし
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
	// アイコン部分の背景を灰色にする
	if (bMenuIconDraw) {
		const int nXIconMenu = lpdis->rcItem.left + nIndentLeft - 3 - 3;
		hBrush = ::GetSysColorBrush(COLOR_MENU);
		RECT rcFillMenuBack = lpdis->rcItem;
		rcFillMenuBack.left = nXIconMenu;
		::FillRect(hdc, &rcFillMenuBack, hBrush);

		COLORREF colMenu   = ::GetSysColor(COLOR_MENU);
		COLORREF colFace = ::GetSysColor(COLOR_3DFACE);
		COLORREF colIconBack;
		// 明度らしきもの
		if (64 < t_abs(t_max(t_max(GetRValue(colFace), GetGValue(colFace)), GetBValue(colFace))
			         - t_max(t_max(GetRValue(colMenu), GetGValue(colMenu)), GetBValue(colMenu)))
		) {
			colIconBack = colFace;
		}else {
			// 明るさが近いなら混色にして(XPテーマ等で)違和感を減らす
			BYTE valR = ((GetRValue(colFace) * 7 + GetRValue(colMenu) * 3) / 10);
			BYTE valG = ((GetGValue(colFace) * 7 + GetGValue(colMenu) * 3) / 10);
			BYTE valB = ((GetBValue(colFace) * 7 + GetBValue(colMenu) * 3) / 10);
			colIconBack = RGB(valR, valG, valB);
		}
		HBRUSH hbr = ::CreateSolidBrush(colIconBack);
		
		RECT rcIconBk = lpdis->rcItem;
		rcIconBk.right = nXIconMenu;
		::FillRect(hdc, &rcIconBk, hbr);
		::DeleteObject(hbr);

		// アイコンとテキストの間に縦線を描画する
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
		HPEN hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(nSepColor));
		HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);
		::MoveToEx(hdc, lpdis->rcItem.left + nIndentLeft - 3 - 3, lpdis->rcItem.top, NULL);
		::LineTo(  hdc, lpdis->rcItem.left + nIndentLeft - 3 - 3, lpdis->rcItem.bottom);
		::SelectObject(hdc, hPenOld);
		::DeleteObject(hPen);

	}else {
		// アイテム矩形塗りつぶし
		hBrush = ::GetSysColorBrush(COLOR_MENU);
		::FillRect(hdc, &lpdis->rcItem, hBrush);
	}
	
	if (lpdis->itemID == F_0) {
		// セパレータの作画(セパレータのFuncCodeはF_SEPARETORではなくF_0)
		int y = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2;
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
		HPEN hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(nSepColor));
		HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);
		::MoveToEx(hdc, lpdis->rcItem.left + (bMenuIconDraw ? nIndentLeft : 3), y, NULL);
		::LineTo(  hdc, lpdis->rcItem.right - 2, y);
		::SelectObject(hdc, hPenOld);
		::DeleteObject(hPen);
		
		if (bBackSurface) {
			::BitBlt(hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
				hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY);
		}
		return; // セパレータ。作画終了
	}

#else // DRAW_MENU_ICON_BACKGROUND_3DFACE
	hBrush = ::GetSysColorBrush(COLOR_MENU);
	::FillRect(hdc, &lpdis->rcItem, hBrush);
#endif

	const int    nItemIndex = Find((int)lpdis->itemID);
	const TCHAR* pszItemStr = menuItems[nItemIndex].memLabel.GetStringPtr(&nItemStrLen);
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontMenu);

	nBkModeOld = ::SetBkMode(hdc, TRANSPARENT);
	if (lpdis->itemState & ODS_SELECTED) {
		// アイテムが選択されている
		RECT rc1 = lpdis->rcItem;
		if (bMenuIconDraw
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
#else
			&& menuItems[nItemIndex].nBitmapIdx != -1 || lpdis->itemState & ODS_CHECKED
#endif
		) {
			//rc1.left += (nIndentLeft - 3);
		}
#ifdef DRAW_MENU_SELECTION_LIGHT
		HPEN hPenBorder = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
		HPEN hOldPen = (HPEN)::SelectObject(hdc, hPenBorder);
		COLORREF colHilight = ::GetSysColor(COLOR_HIGHLIGHT);
		COLORREF colMenu = ::GetSysColor(COLOR_MENU);
		BYTE valR = ((GetRValue(colHilight) * 4 + GetRValue(colMenu) * 6) / 10) | 0x18;
		BYTE valG = ((GetGValue(colHilight) * 4 + GetGValue(colMenu) * 6) / 10) | 0x18;
		BYTE valB = ((GetBValue(colHilight) * 4 + GetBValue(colMenu) * 6) / 10) | 0x18;
		hBrush = ::CreateSolidBrush(RGB(valR, valG, valB));
		HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);
		::Rectangle(hdc, rc1.left, rc1.top, rc1.right, rc1.bottom);
		::SelectObject(hdc, hOldPen);
		::SelectObject(hdc, hOldBrush);
		::DeleteObject(hPenBorder);
		::DeleteObject(hBrush);
#else
		hBrush = ::GetSysColorBrush(COLOR_HIGHLIGHT);
		// 選択ハイライト矩形
		::FillRect(hdc, &rc1, hBrush);
#endif

		if (lpdis->itemState & ODS_DISABLED) {
			// アイテムが使用不可
			nTxSysColor = COLOR_MENU;
		}else {
#ifdef DRAW_MENU_SELECTION_LIGHT
			nTxSysColor = COLOR_MENUTEXT;
#else
			nTxSysColor = COLOR_HIGHLIGHTTEXT;
#endif
		}
	}else {
		if (lpdis->itemState & ODS_DISABLED) {
			// アイテムが使用不可
			nTxSysColor = COLOR_GRAYTEXT;
		}else {
			nTxSysColor = COLOR_MENUTEXT;
		}
	}
	::SetTextColor(hdc, ::GetSysColor(nTxSysColor));

#ifdef _DEBUG
	// デバッグ用：メニュー項目に対して、ヘルプがない場合に背景色を青くする
	TCHAR	szText[1024];
	MENUITEMINFO mii = {0};
	// メニュー項目に関する情報を取得します。

	mii.cbSize = SIZEOF_MENUITEMINFO; // Win95対策済みのsizeof(MENUITEMINFO)値

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	_tcscpy(szText, _T("--unknown--"));
	mii.dwTypeData = szText;
	mii.cch = _countof(szText) - 1;
	if (::GetMenuItemInfo((HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, &mii) != 0
		&& !mii.hSubMenu
		&& /* EditWnd */::FuncID_To_HelpContextID((EFunctionCode)lpdis->itemID) == 0 	// 機能IDに対応するメニューコンテキスト番号を返す
	) {
		if (lpdis->itemState & ODS_SELECTED) {
			::SetTextColor(hdc, ::GetSysColor(COLOR_HIGHLIGHTTEXT));	//	ハイライトカラー
		}else {
			::SetTextColor(hdc, RGB(0, 0, 255));	//	青くしてる。
		}
//		::SetTextColor(hdc, RGB(0, 0, 255));
	}
#endif

	rcText = lpdis->rcItem;
	rcText.left += nIndentLeft + 1;
	rcText.right -= nIndentRight;

	// TAB文字の前と後ろに分割してテキストを描画する
	for (j=0; j<nItemStrLen; ++j) {
		if (pszItemStr[j] == _T('\t')) {
			break;
		}
	}
	// TAB文字の後ろ側のテキストを描画する
	if (j < nItemStrLen) {
		::DrawText(
			hdc,
			&pszItemStr[j + 1],
			-1,
			&rcText,
			DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT
		);
	}
	// TAB文字の前側のテキストを描画する
	::DrawText(
		hdc,
		pszItemStr,
		(int)j,
		&rcText,
		DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT
	);
	::SelectObject(hdc, hFontOld );
	::SetBkMode(hdc, nBkModeOld);

	// 16*16のアイコンを上下中央へ置いたときの上の座標
	int nIconTop = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2 - (pIcons->GetCy()/2);

	// 枠は アイコン横幅xメニュー縦幅で表示し真ん中にアイコンを置く

	if (bMenuIconDraw && (lpdis->itemState & ODS_CHECKED)) {
		// アイコンを囲む枠
		{
			// フラットな枠 + 半透明の背景色
			HBRUSH hBrush = ::GetSysColorBrush(COLOR_HIGHLIGHT);
			const int MENUICO_PADDING = 0;
			const int MENUICO_BORDER  = 1;
			const int MENUICO_PB  = MENUICO_PADDING + MENUICO_BORDER;
			const int MENUICO_SIZE = MENUICO_PB + pIcons->GetCx() + MENUICO_PB;
			const int left = lpdis->rcItem.left + 2 - MENUICO_PB;
			const int top = nIconTop - MENUICO_PB;
			RECT rc;
			::SetRect(&rc, left - 1, top - 1, left + MENUICO_SIZE + 2, top + MENUICO_SIZE + 1);
			::FillRect(hdc, &rc, hBrush);

			COLORREF colHilight = ::GetSysColor(COLOR_HIGHLIGHT);
			COLORREF colMenu = ::GetSysColor(COLOR_MENU);
			// 16bitカラーの黒色でも少し明るくするように or 0x18 する
			BYTE valR;
			BYTE valG;
			BYTE valB;
			if (lpdis->itemState & ODS_SELECTED) {	// 選択状態
				valR = ((GetRValue(colHilight) * 6 + GetRValue(colMenu) * 4) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 6 + GetGValue(colMenu) * 4) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 6 + GetBValue(colMenu) * 4) / 10) | 0x18;
			}else {								// 非選択状態
				valR = ((GetRValue(colHilight) * 2 + GetRValue(colMenu) * 8) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 2 + GetGValue(colMenu) * 8) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 2 + GetBValue(colMenu) * 8) / 10) | 0x18;
			}
			HBRUSH hbr = ::CreateSolidBrush(RGB(valR, valG, valB));
			::SetRect(&rc, left + MENUICO_BORDER - 1, top + MENUICO_BORDER-1,
				left + MENUICO_SIZE - MENUICO_BORDER + 2, top + MENUICO_SIZE - MENUICO_BORDER + 1);
			::FillRect(hdc, &rc, hbr);
			::DeleteObject(hbr);
		}
	}

	// 機能の画像が存在するならメニューアイコン?を描画する
	if (bMenuIconDraw && menuItems[nItemIndex].nBitmapIdx != -1) {
		// 3D枠を描画する
		// アイテムが選択されている
		if (lpdis->itemState & ODS_SELECTED) {
			// アイテムが使用不可
			if (lpdis->itemState & ODS_DISABLED /*&& !(lpdis->itemState & ODS_SELECTED)*/) {
			}else {
				if (lpdis->itemState & ODS_CHECKED) {
				}else {
				}
			}
		}

		// アイテムが使用不可
		if (lpdis->itemState & ODS_DISABLED) {
			// 淡色アイコン
			pIcons->Draw(menuItems[nItemIndex].nBitmapIdx,
				hdc,	//	Target DC
				lpdis->rcItem.left + 2,	//	X
				nIconTop,	//	Y
				ILD_MASK
			);
		}else {
			// 通常のアイコン
			pIcons->Draw(menuItems[nItemIndex].nBitmapIdx,
				hdc,	//	Target DC
				lpdis->rcItem.left + 2,	//	X
				nIconTop,	//	Y
				ILD_NORMAL
			);
		}
	}else {
		// チェックボックスを表示
		if (lpdis->itemState & ODS_CHECKED) {
			// チェックマークの表示
			if (bMenuIconDraw) {
				// だいたい中心座標
				int nX = lpdis->rcItem.left + pIcons->GetCx()/2;
				int nY = nIconTop + pIcons->GetCy()/2;
				HPEN hPen   = NULL;
				HPEN hPenOld = NULL;
				// チェックの色を黒(未指定)からテキスト色に変更
				hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_MENUTEXT));
				hPenOld = (HPEN)::SelectObject(hdc, hPen);
#if 0
// チェックマークも自分で書く場合
				if (!bMenuIconDraw) {
					nX -= 4; // iconがない場合、左マージン=2アイコン枠=2分がない
				}
#endif
				const int nBASE = 100 * 100; // 座標,nScale共に0.01単位
				// 16dot幅しかないので 1.0倍から2.1倍までスケールする(10-23)
				const int nScale = t_max(100, t_min(210, int((lpdis->rcItem.bottom - lpdis->rcItem.top - 2) * 100) / (16-2)));
				for (int nBold=1; nBold<=(281*nScale)/nBASE; ++nBold) {
					::MoveToEx(hdc, nX - (187*nScale)/nBASE, nY - (187*nScale)/nBASE, NULL);
					::LineTo(  hdc, nX -   (0*nScale)/nBASE, nY -   (0*nScale)/nBASE);
					::LineTo(  hdc, nX + (468*nScale)/nBASE, nY - (468*nScale)/nBASE);
					++nY;
				}
				if (hPen) {
					::SelectObject(hdc, hPenOld);
					::DeleteObject(hPen);
				}
			}else {
				// OSにアイコン作画をしてもらう(黒背景等対応)
				HDC hdcMem = ::CreateCompatibleDC(hdc);
				HBITMAP hBmpMono = ::CreateBitmap(nCxCheck, nCyCheck, 1, 1, NULL);
				HBITMAP hOld = (HBITMAP)::SelectObject(hdcMem, hBmpMono);
				RECT rcCheck = {0, 0, nCxCheck, nCyCheck};
				::DrawFrameControl(hdcMem, &rcCheck, DFC_MENU, DFCS_MENUCHECK);
				COLORREF colTextOld = ::SetTextColor(hdc, RGB(0, 0, 0));
				COLORREF colBackOld = ::SetBkColor(hdc,   RGB(255, 255, 255));
				::BitBlt(hdc, lpdis->rcItem.left + 2, lpdis->rcItem.top + 2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCAND);
				::SetTextColor(hdc, ::GetSysColor(nTxSysColor));
				::SetBkColor(hdc, RGB(0, 0, 0));
				::BitBlt(hdc, lpdis->rcItem.left + 2, lpdis->rcItem.top + 2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCPAINT);
				::SetTextColor(hdc, colTextOld);
				::SetBkColor(hdc, colBackOld);
				::SelectObject(hdcMem, hOld);
				::DeleteObject(hBmpMono);
				::DeleteDC(hdcMem);
			}
		}
	}
	if (bBackSurface) {
		::BitBlt(hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
			hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY);
	}
	return;
}

/*!
	作画終了
	メニューループ終了時に呼び出すとリソース節約になる
*/
void MenuDrawer::EndDrawMenu()
{
	DeleteCompDC();
}


void MenuDrawer::DeleteCompDC()
{
	if (hCompDC) {
		::SelectObject(hCompDC, hCompBitmapOld);
		::DeleteObject(hCompBitmap);
		::DeleteObject(hCompDC);
//		DEBUG_TRACE(_T("MenuDrawer::DeleteCompDC %x\n"), hCompDC);
		hCompDC = NULL;
		hCompBitmap = NULL;
		hCompBitmapOld = NULL;
	}
}

/*
	ツールバー登録のための番号を返す。
	プラグインのみボタンのindexのかわりにidCommandをそのまま返す
	@note この値がiniのツールバーアイテムの記録に使われる
*/
int MenuDrawer::FindToolbarNoFromCommandId(int idCommand, bool bOnlyFunc) const
{
	// 先に存在確認をする
	int index = FindIndexFromCommandId(idCommand, bOnlyFunc);
	if (-1 < index) {
		// 固定部分以外(プラグインなど)はindexではなくidCommandのままにする
		if ((int)nMyButtonFixSize <= index) {
			// もし コマンド番号が明らかに小さいと区別がつかない
			assert_warning(idCommand < (int)nMyButtonFixSize);
			return idCommand;
		}
	}
	return index;
}

/** コマンドコードからツールバーボタン情報のINDEXを得る

	@param idCommand [in] コマンドコード
	@param bOnlyFunc [in] 有効な機能の範囲で検索する

	@retval みつからなければ-1を返す。
 */
int MenuDrawer::FindIndexFromCommandId(int idCommand, bool bOnlyFunc) const
{
	if (bOnlyFunc) {
		// 機能の範囲外（セパレータや折り返しなど特別なもの）は除外する
		if (!(F_MENU_FIRST <= idCommand && idCommand < F_MENU_NOT_USED_FIRST)
			&& !(F_PLUGCOMMAND_FIRST <= idCommand && idCommand < F_PLUGCOMMAND_LAST)
		) {
			return -1;
		}
	}

	int nIndex = -1;
	for (size_t i=0; i<nMyButtonNum; ++i) {
		if (tbMyButton[i].idCommand == idCommand) {
			nIndex = (int)i;
			break;
		}
	}

	return nIndex;
}

/** インデックスからボタン情報を得る

	@param nToolbarNo [in] ボタン情報のツールバー番号
	@retval ボタン情報
 */
TBBUTTON MenuDrawer::getButton(int nToolbarNo) const
{
	int index = ToolbarNoToIndex(nToolbarNo);
	if (0 <= index && index < (int)nMyButtonNum) {
		return tbMyButton[index];
	}

	// 範囲外なら未定義のボタン情報を作成して返す
	// （sakura.iniに範囲外インデックスが指定があった場合など、堅牢性のため）
	static TBBUTTON tbb;
	SetTBBUTTONVal(&tbb, -1, F_DISABLE, 0, TBSTYLE_BUTTON, 0, 0);
	return tbb;
}


int MenuDrawer::Find(int nFuncID)
{
	int i;
	int nItemNum = (int)menuItems.size();
	for (i=0; i<nItemNum; ++i) {
		if (nFuncID == menuItems[i].nFuncId) {
			break;
		}
	}
	if (i >= nItemNum) {
		return -1;
	}else {
		return i;
	}
}


const TCHAR* MenuDrawer::GetLabel(int nFuncID)
{
	int i;
	if ((i = Find(nFuncID)) == -1) {
		return NULL;
	}
	return menuItems[i].memLabel.GetStringPtr();
}

TCHAR MenuDrawer::GetAccelCharFromLabel(const TCHAR* pszLabel)
{
	int nLen = (int)_tcslen(pszLabel);
	for (int i=0; i+1<nLen; ++i) {
		if (_T('&') == pszLabel[i]) {
			if (_T('&') == pszLabel[i + 1]) {
				++i;
			}else {
				return (TCHAR)_totupper(pszLabel[i + 1]);
			}
		}
	}
	return _T('\0');
}

struct WorkData {
	int				idx;
	MENUITEMINFO	mii;
};

// メニューアクセスキー押下時の処理(WM_MENUCHAR処理)
LRESULT MenuDrawer::OnMenuChar(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	TCHAR chUser;
	HMENU hmenu;
	chUser = (TCHAR) LOWORD(wParam);	// character code
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE(_T("::GetMenuItemCount(%xh)==%d\n"), hmenu, ::GetMenuItemCount(hmenu));

	if (0 <= chUser && chUser < ' ') {
		chUser += '@';
	}else {
		chUser = (TCHAR)_totupper(chUser);
	}

	std::vector<WorkData> vecAccel;
	size_t nAccelSel = 99999;
	for (int i=0; i<::GetMenuItemCount(hmenu); ++i) {
		TCHAR	szText[1024];
		// メニュー項目に関する情報を取得します。
		MENUITEMINFO mii = {0};

		mii.cbSize = SIZEOF_MENUITEMINFO; // Win95対策済みのsizeof(MENUITEMINFO)値

		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		_tcscpy(szText, _T("--unknown--"));
		mii.dwTypeData = szText;
		mii.cch = _countof(szText) - 1;
		if (::GetMenuItemInfo(hmenu, i, TRUE, &mii) == 0) {
			continue;
		}
		const TCHAR* pszLabel;
		if (!(pszLabel = GetLabel(mii.wID))) {
			continue;
		}
		if (chUser == GetAccelCharFromLabel(pszLabel)) {
			WorkData work;
			work.idx = i;
			work.mii = mii;
			if (/*nAccelSel == -1 ||*/ MFS_HILITE & mii.fState) {
				nAccelSel = vecAccel.size();
			}
			vecAccel.push_back(work);
		}
	}
//	MYTRACE(_T("%d\n"), (int)mapAccel.size());
	if (vecAccel.size() == 0) {
		return  MAKELONG(0, MNC_IGNORE);
	}
	if (vecAccel.size() == 1) {
		return  MAKELONG(vecAccel[0].idx, MNC_EXECUTE);
	}
//	MYTRACE(_T("nAccelSel=%d vecAccel.size()=%d\n"), nAccelSel, vecAccel.size());
	if (nAccelSel + 1 >= vecAccel.size()) {
//		MYTRACE(_T("vecAccel[0].idx=%d\n"), vecAccel[0].idx);
		return  MAKELONG(vecAccel[0].idx, MNC_SELECT);
	}else {
//		MYTRACE(_T("vecAccel[nAccelSel + 1].idx=%d\n"), vecAccel[nAccelSel + 1].idx);
		return  MAKELONG(vecAccel[nAccelSel + 1].idx, MNC_SELECT);
	}
}


// TBBUTTON構造体にデータをセット
void MenuDrawer::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD_PTR	dwData,
	INT_PTR		iString
	) const
{
	/*
typedef struct _TBBUTTON {
	int iBitmap;	// ボタン イメージの 0 から始まるインデックス
	int idCommand;	// ボタンが押されたときに送られるコマンド
	BYTE fsState;	// ボタンの状態--以下を参照
	BYTE fsStyle;	// ボタン スタイル--以下を参照
	DWORD dwData;	// アプリケーション-定義された値
	int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
} TBBUTTON;
*/

	ptb->iBitmap	= iBitmap;
	ptb->idCommand	= idCommand;
	ptb->fsState	= fsState;
	ptb->fsStyle	= fsStyle;
	ptb->dwData		= dwData;
	ptb->iString	= iString;
	return;
}

// ツールバーボタンを追加する
//		全ウィンドウで同じ機能番号の場合、同じICON番号を持つように調整
void MenuDrawer::AddToolButton(int iBitmap, int iCommand)
{
	TBBUTTON tbb;
	int iCmdNo;
	
	if (pShareData->maxToolBarButtonNum < nMyButtonNum) {
		pShareData->maxToolBarButtonNum = nMyButtonNum;
	}

	if (iCommand >= F_PLUGCOMMAND_FIRST && iCommand <= F_PLUGCOMMAND_LAST) {
		auto& cmdIcons = pShareData->plugCmdIcons;
		iCmdNo = iCommand - F_PLUGCOMMAND_FIRST;
		if (cmdIcons[iCmdNo] != 0) {
			if (tbMyButton.size() <= cmdIcons[iCmdNo]) {
				// このウィンドウで未登録
				// 空きを詰め込む
				SetTBBUTTONVal(&tbb, TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1, 0, 0, TBSTYLE_BUTTON, 0, 0);
				for (size_t i=tbMyButton.size(); i<cmdIcons[iCmdNo]; ++i) {
					tbMyButton.push_back(tbb);
					++nMyButtonNum;
				}

				// 未登録
				SetTBBUTTONVal(&tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0);
				tbMyButton.push_back(tbb);
				++nMyButtonNum;
			}else {
				// 再設定
				SetTBBUTTONVal(&tbMyButton[cmdIcons[iCmdNo]],
					iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0);
			}
		}else {
			// 全体で未登録
			if (tbMyButton.size() < (size_t)pShareData->maxToolBarButtonNum) {
				// 空きを詰め込む
				SetTBBUTTONVal(&tbb, TOOLBAR_ICON_PLUGCOMMAND_DEFAULT-1, 0, 0, TBSTYLE_BUTTON, 0, 0);
				for (size_t i=tbMyButton.size(); i<pShareData->maxToolBarButtonNum; ++i) {
					tbMyButton.push_back(tbb);
					++nMyButtonNum;
				}
			}
			// 新規登録
			SetTBBUTTONVal(&tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0);

			cmdIcons[iCmdNo] = (short)tbMyButton.size();
			tbMyButton.push_back(tbb);
			++nMyButtonNum;
		}
	}
	if (pShareData->maxToolBarButtonNum < nMyButtonNum) {
		pShareData->maxToolBarButtonNum = nMyButtonNum;
	}
}

