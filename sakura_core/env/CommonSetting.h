// 2007.09.28 kobake Common整理
/*
	Copyright (C) 2008, kobake

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

#include "KeywordSetMgr.h"
#include "func/KeyBind.h"
#include "func/FuncLookup.h" // MacroRec
#include "io/File.h" // ShareMode

// Apr. 05, 2003 genta WindowCaption用領域（変換前）の長さ
static const uint32_t MAX_CAPTION_CONF_LEN = 256;

static const uint32_t MAX_DATETIMEFOREMAT_LEN	= 100;
static const uint32_t MAX_CUSTOM_MENU			=  25;
static const uint32_t MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const uint32_t MAX_CUSTOM_MENU_ITEMS		=  48;
static const uint32_t MAX_TOOLBAR_BUTTON_ITEMS	= 512;	// ツールバーに登録可能なボタン最大数	
static const uint32_t MAX_TOOLBAR_ICON_X		=  32;	// アイコンBMPの桁数
static const uint32_t MAX_TOOLBAR_ICON_Y		=  15;	// アイコンBMPの段数
static const uint32_t MAX_TOOLBAR_ICON_COUNT	= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =480
// Oct. 22, 2000 JEPRO アイコンの最大登録数を128個増やした(256→384)	
// 2010/3/14 Uchi アイコンの最大登録数を32個増やした(384→416)
// 2010/6/26 syat アイコンの最大登録数を15段に増やした(416→480)

// 旧版と違い、bool型使えるようにしてあります by kobake

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           全般                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_General {
	// Jul. 3, 2000 genta
	// アクセス関数(簡易)
	// intをビット単位に分割して使う
	// 下4bitをCaretTypeに当てておく(将来の予約で多めに取っておく)
	int		GetCaretType(void) const { return nCaretType & 0xf; }
	void	SetCaretType(const int f) { nCaretType &= ~0xf; nCaretType |= f & 0xf; }

	// カーソル
	int		nCaretType;							// カーソルのタイプ 0=win 1=dos 
	bool	bIsINSMode;							// 挿入／上書きモード
	bool	bIsFreeCursorMode;					// フリーカーソルモードか
	bool	bStopsBothEndsWhenSearchWord;		// 単語単位で移動するときに、単語の両端で止まるか
	bool	bStopsBothEndsWhenSearchParagraph;	// 段落単位で移動するときに、段落の両端で止まるか
	bool	bNoCaretMoveByActivation;			// マウスクリックにてアクティベートされた時はカーソル位置を移動しない  2007.10.02 nasukoji (add by genta)

	// スクロール
	int		nRepeatedScrollLineNum;			// キーリピート時のスクロール行数
	bool	nRepeatedScroll_Smooth;			// キーリピート時のスクロールを滑らかにするか
	int		nPageScrollByWheel;				// キー/マウスボタン + ホイールスクロールでページUP/DOWNする	// 2009.01.17 nasukoji
	int		nHorizontalScrollByWheel;		// キー/マウスボタン + ホイールスクロールで横スクロールする		// 2009.01.17 nasukoji

	// タスクトレイ
	bool	bUseTaskTray;					// タスクトレイのアイコンを使う
	bool	bStayTaskTray;					// タスクトレイのアイコンを常駐
	WORD	wTrayMenuHotKeyCode;			// タスクトレイ左クリックメニュー キー
	WORD	wTrayMenuHotKeyMods;			// タスクトレイ左クリックメニュー キー

	// 履歴
	size_t	nMRUArrNum_MAX;					// ファイルの履歴MAX
	size_t	nOPENFOLDERArrNum_MAX;			// フォルダの履歴MAX

	// ノーカテゴリ
	bool	bCloseAllConfirm;				// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	bool	bExitConfirm;					// 終了時の確認をする

	// INI内設定のみ
	bool	bDispExitingDialog;				// 終了ダイアログを表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ウィンドウ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 2004.05.13 Moca
// ウィンドウサイズ・位置の制御方法
enum class WinSizeMode {
	Default		= 0,	// 指定なし
	Save		= 1,	// 継承(保存)
	Set			= 2		// 直接指定(固定)
};

struct CommonSetting_Window {
	// 基本設定
	bool			bDispToolBar;				// 次回ウィンドウを開いたときツールバーを表示する
	bool			bDispStatusBar;				// 次回ウィンドウを開いたときステータスバーを表示する
	bool			bDispFuncKeyWnd;			// 次回ウィンドウを開いたときファンクションキーを表示する
	bool			bDispMiniMap;				// ミニマップを表示する
	bool			bMenuIcon;					// メニューにアイコンを表示する (アイコン付きメニュー)
	bool			bScrollBarHorz;				// 水平スクロールバーを使う
	bool			bUseCompatibleBMP;			// 再作画用互換ビットマップを使う 2007.09.09 Moca

	// 位置と大きさの設定
	WinSizeMode		eSaveWindowSize;			// ウィンドウサイズ継承・固定 WinSizeModeに順ずる 2004.05.13 Moca
	int				nWinSizeType;				// 大きさの指定
	int				nWinSizeCX;					// 直接指定 幅
	int				nWinSizeCY;					// 直接指定 高さ
	WinSizeMode		eSaveWindowPos;				// ウィンドウ位置継承・固定 WinSizeModeに順ずる 2004.05.13 Moca
	int				nWinPosX;					// 直接指定 X座標
	int				nWinPosY;					// 直接指定 Y座標

	// ファンクションキー
	int				nFuncKeyWnd_Place;			// ファンクションキー表示位置／0:上 1:下
	int				nFuncKeyWnd_GroupNum;		// 2002/11/04 Moca ファンクションキーのグループボタン数

	// ルーラー・行番号
	int				nRulerHeight;				// ルーラー高さ
	int				nRulerBottomSpace;			// ルーラーとテキストの隙間
	int				nRulerType;					// ルーラーのタイプ $$$未使用っぽい
	int				nLineNumRightSpace;			// 行番号の右のスペース Sep. 18, 2002 genta

	// 分割ウィンドウ
	bool			bSplitterWndHScroll;		// 分割ウィンドウの水平スクロールの同期をとる 2001/06/20 asa-o
	bool			bSplitterWndVScroll;		// 分割ウィンドウの垂直スクロールの同期をとる 2001/06/20 asa-o

	// タイトルバー
	TCHAR			szWindowCaptionActive[MAX_CAPTION_CONF_LEN];	// タイトルバー(アクティブ時)
	TCHAR			szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];	// タイトルバー(非アクティブ時)

	// INI内設定のみ
	int				nVertLineOffset;			// 縦線の描画座標オフセット 2005.11.10 Moca

	// 言語選択
	TCHAR			szLanguageDll[MAX_PATH];	// 言語DLLファイル名

	// ミニマップ
	int				nMiniMapFontSize;
	int				nMiniMapQuality;
	int				nMiniMapWidth;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         タブバー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 閉じるボタン
enum class DispTabCloseType {
	No			= 0, // なし
	Always		= 1, // 常に表示
	Auto		= 2  // 自動表示
};

enum class TabPosition {
	Top,
	Bottom,
	Left,
	Right,
	None = -1,
};

struct CommonSetting_TabBar {
	bool		bDispTabWnd;					// タブウィンドウ表示する	//@@@ 2003.05.31 MIK
	bool		bDispTabWndMultiWin;			// タブをまとめない	//@@@ 2003.05.31 MIK
	bool		bTab_RetainEmptyWin;			// 最後の文書が閉じられたとき(無題)を残す
	bool		bTab_CloseOneWin;				// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる
	bool		bNewWindow;						// 外部から起動するときは新しいウィンドウで開く
	bool		bTabMultiLine;					// タブ多段
	TabPosition	eTabPosition;					// タブ位置

	wchar_t		szTabWndCaption[MAX_CAPTION_CONF_LEN];	// タブウィンドウキャプション	//@@@ 2003.06.13 MIK
	bool		bSameTabWidth;					// タブを等幅にする			//@@@ 2006.01.28 ryoji
	bool		bDispTabIcon;					// タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
	DispTabCloseType	dispTabClose;			// タブに閉じるボタンを表示する	//@@@ 2012.04.14 syat
	bool		bSortTabList;					// タブ一覧をソートする	//@@@ 2006.03.23 fon
	bool		bTab_ListFull;					// タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji

	bool		bChgWndByWheel;					// マウスホイールでウィンドウ切り替え	//@@@ 2006.03.26 ryoji

	LOGFONT		lf;								// タブフォント // 2011.12.01 Moca
	INT			nPointSize;						// フォントサイズ（1/10ポイント単位）
	int			nTabMaxWidth;					// タブ幅の最大値
	int			nTabMinWidth;					// タブ幅の最小値
	int			nTabMinWidthOnMulti;			// タブ幅の最小値(タブ多段時)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           編集                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ファイルダイアログの初期位置
enum EOpenDialogDir {
	OPENDIALOGDIR_CUR, // カレントフォルダ
	OPENDIALOGDIR_MRU, // 最近使ったフォルダ
	OPENDIALOGDIR_SEL, // 指定フォルダ
};

struct CommonSetting_Edit {
	// コピー
	bool	bAddCRLFWhenCopy;			// 折り返し行に改行を付けてコピー
	bool	bEnableNoSelectCopy;		// 選択なしでコピーを可能にする 2007.11.18 ryoji
	bool	bCopyAndDisablSelection;	// コピーしたら選択解除
	bool	bEnableLineModePaste;		// ラインモード貼り付けを可能にする  2007.10.08 ryoji
	bool	bConvertEOLPaste;			// 改行コードを変換して貼り付ける  2009.2.28 salarm

	// ドラッグ＆ドロップ
	bool	bUseOLE_DragDrop;			// OLEによるドラッグ & ドロップを使う
	bool	bUseOLE_DropSource;			// OLEによるドラッグ元にするか

	// 上書きモード
	bool	bNotOverWriteCRLF;			// 改行は上書きしない
	bool	bOverWriteFixMode;			// 文字幅に合わせてスペースを詰める
	bool	bOverWriteBoxDelete;		// 上書きモードでの矩形入力で選択範囲を削除する

	// クリッカブルURL
	bool	bJumpSingleClickURL;		// URLのシングルクリックでJump $$$未使用
	bool	bSelectClickedURL;			// URLがクリックされたら選択するか

	EOpenDialogDir	eOpenDialogDir;		// ファイルダイアログの初期位置
	SFilePath	openDialogSelDir;		// 指定フォルダ

	bool	bEnableExtEol;				// NEL,PS,LSを改行コードとして利用する
	bool	bBoxSelectLock;				// (矩形選択)移動でロックする

	// (ダイアログ項目無し)
	bool	bAutoColumnPaste;			// 矩形コピーのテキストは常に矩形貼り付け
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ファイル                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File {
public:
	// カーソル位置を復元するかどうか  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ bRestoreCurPosition = i; }

	// ブックマークを復元するかどうか  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ bRestoreBookmarks = i; }

	// ファイル読み込み時にMIMEのdecodeを行うか  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return bAutoMimeDecode; }
	void	SetAutoMIMEdecode(bool i)			{ bAutoMimeDecode = i; }

	// 前回と文字コードが異なるときに問い合わせを行う  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ bQueryIfCodeChange = i; }
	
	// 開こうとしたファイルが存在しないとき警告する  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ bAlertIfFileNotExist = i; }

public:
	// ファイルの排他制御モード
	FileShareMode	nFileShareMode;				// ファイルの排他制御モード
	bool		bCheckFileTimeStamp;		// 更新の監視
	int 		nAutoloadDelay;				// 自動読込時遅延
	bool		bUneditableIfUnwritable;	// 上書き禁止検出時は編集禁止にする

	// ファイルの保存
	bool	bEnableUnmodifiedOverwrite;		// 無変更でも上書きするか

	//「名前を付けて保存」でファイルの種類が[ユーザー指定]のときのファイル一覧表示
	// ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	bool	bNoFilterSaveNew;				// 新規から保存時は全ファイル表示
	bool	bNoFilterSaveFile;				// 新規以外から保存時は全ファイル表示

	// ファイルオープン
	bool	bDropFileAndClose;				// ファイルをドロップしたときは閉じて開く
	int		nDropFileNumMax;				// 一度にドロップ可能なファイル数
	bool	bRestoreCurPosition;			// ファイルを開いたときカーソル位置を復元するか
	bool	bRestoreBookmarks;				// ブックマークを復元するかどうか 2002.01.16 hor
	bool	bAutoMimeDecode;				// ファイル読み込み時にMIMEのdecodeを行うか
	bool	bQueryIfCodeChange;				// 前回と文字コードが異なるときに問い合わせを行う Oct. 03, 2004 genta
	bool	bAlertIfFileNotExist;			// 開こうとしたファイルが存在しないとき警告する Oct. 09, 2004 genta
	bool	bAlertIfLargeFile;				// 開こうとしたファイルサイズが大きい場合に警告する
	int		nAlertFileSize;					// 警告を始めるファイルサイズ(MB)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       バックアップ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Aug. 15, 2000 genta
// Backup Flags
enum EBackupOptionFlag {
	BKUP_YEAR		= 32,
	BKUP_MONTH		= 16,
	BKUP_DAY		= 8,
	BKUP_HOUR		= 4,
	BKUP_MIN		= 2,
	BKUP_SEC		= 1,

	// Aug. 21, 2000 genta
	BKUP_AUTO		= 64,
};

struct CommonSetting_Backup {
public:
	// Aug. 15, 2000 genta
	// Backup設定のアクセス関数
	int		GetBackupType(void) const			{ return nBackUpType; }
	void	SetBackupType(int n)				{ nBackUpType = n; }

	bool	GetBackupOpt(EBackupOptionFlag flag) const			{ return (flag & nBackUpType_Opt1) == flag; }
	void	SetBackupOpt(EBackupOptionFlag flag, bool value)	{ nBackUpType_Opt1 = value ? (flag | nBackUpType_Opt1) :  ((~flag) & nBackUpType_Opt1); }

	// バックアップ数
	int		GetBackupCount(void) const			{ return nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value)			{ nBackUpType_Opt2 = (nBackUpType_Opt2 & 0xffff0000) | (value & 0xffff); }

	// バックアップの拡張子先頭文字(1文字)
	TCHAR	GetBackupExtChar(void) const		{ return (TCHAR)((nBackUpType_Opt2 >> 16) & 0xff) ; }
	void	SetBackupExtChar(int value)			{ nBackUpType_Opt2 = (nBackUpType_Opt2 & 0xff00ffff) | ((value & 0xff) << 16); }

	// Aug. 21, 2000 genta
	// 自動Backup
	bool	IsAutoBackupEnabled(void) const		{ return GetBackupOpt(BKUP_AUTO); }
	void	EnableAutoBackup(bool flag)			{ SetBackupOpt(BKUP_AUTO, flag); }
	
	int		GetAutoBackupInterval(void) const	{ return nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i)		{ nBackUpType_Opt3 = i; }

	// Backup詳細設定のアクセス関数
	int		GetBackupTypeAdv(void) const { return nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n) { nBackUpType_Opt4 = n; }

public:
	bool		bBackUp;					// 保存時にバックアップを作成する
	bool		bBackUpDialog;				// バックアップの作成前に確認
	bool		bBackUpFolder;				// 指定フォルダにバックアップを作成する
	bool		bBackUpFolderRM;			// 指定フォルダにバックアップを作成する(リムーバブルメディアのみ)
	SFilePath	szBackUpFolder;				// バックアップを作成するフォルダ
	int 		nBackUpType;				// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	int 		nBackUpType_Opt1;			// バックアップファイル名：オプション1
	int 		nBackUpType_Opt2;			// バックアップファイル名：オプション2
	int 		nBackUpType_Opt3;			// バックアップファイル名：オプション3
	int 		nBackUpType_Opt4;			// バックアップファイル名：オプション4
	int 		nBackUpType_Opt5;			// バックアップファイル名：オプション5
	int 		nBackUpType_Opt6;			// バックアップファイル名：オプション6
	bool		bBackUpDustBox;				// バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add MIK
	bool		bBackUpPathAdvanced;		// バックアップ先フォルダを詳細設定する 20051107 aroka
	SFilePath	szBackUpPathAdvanced;		// バックアップを作成するフォルダの詳細設定 20051107 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           書式                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format {
	// 日付書式
	int			nDateFormatType;							// 日付書式のタイプ
	TCHAR		szDateFormat[MAX_DATETIMEFOREMAT_LEN];		// 日付書式

	// 時刻書式
	int			nTimeFormatType;							// 時刻書式のタイプ
	TCHAR		szTimeFormat[MAX_DATETIMEFOREMAT_LEN];		// 時刻書式

	// 見出し記号
	wchar_t		szMidashiKigou[256];						// 見出し記号

	// 引用符
	wchar_t		szInyouKigou[32];							// 引用符
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search {
	
	int				nSearchKeySequence;			// 検索シーケンス(未保存)
	SearchOption	searchOption;				// 検索／置換  条件

	int				nReplaceKeySequence;		// 置換後シーケンス(未保存)
	bool			bConsecutiveAll;			//「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	bool			bNotifyNotFound;			// 検索／置換  見つからないときメッセージを表示
	bool			bSelectedArea;				// 置換  選択範囲内置換

	bool			bGrepSubFolder;				// Grep: サブフォルダも検索
	int				nGrepOutputLineType;		// Grep: 行を出力/該当部分/否マッチ行 を出力
	int				nGrepOutputStyle;			// Grep: 出力形式
	bool			bGrepDefaultFolder;			// Grep: フォルダの初期値をカレントフォルダにする
	EncodingType	nGrepCharSet;				// Grep: 文字コードセット // 2002/09/20 Moca Add
	bool			bGrepOutputFileOnly;		// Grep: ファイル毎最初のみ検索
	bool			bGrepOutputBaseFolder;		// Grep: ベースフォルダ表示
	bool			bGrepSeparateFolder;		// Grep: フォルダ毎に表示
	bool			bGrepBackup;				// Grep: バックアップ作成
	
	bool			bCaretTextForSearch;		// カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji
	bool			bInheritKeyOtherView;		// 次・前検索で他のビューの検索条件を引き継ぐ
	TCHAR			szRegexpLib[_MAX_PATH];		// 使用する正規表現DLL  2007.08.22 genta

	// Grep
	bool			bGrepExitConfirm;			// Grepモードで保存確認するか
	bool			bGrepRealTimeView;			// Grep結果のリアルタイム表示 2003.06.16 Moca

	bool			bGTJW_Return;				// エンターキーでタグジャンプ
	bool			bGTJW_DoubleClick;			// ダブルクリックでタグジャンプ

	// 検索・置換ダイアログ
	bool			bAutoCloseDlgFind;			// 検索ダイアログを自動的に閉じる
	bool			bAutoCloseDlgReplace;		// 置換 ダイアログを自動的に閉じる
	bool			bSearchAll;					// 先頭（末尾）から再検索 2002.01.26 hor

	int				nTagJumpMode;				// タグジャンプモード(0-3)
	int				nTagJumpModeKeyword;		// タグジャンプモード(0-3)

	// INI内設定のみ
	bool			bUseCaretKeyword;			// キャレット位置の単語を辞書検索		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       キー割り当て                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_KeyBind {
	// キー割り当て
	int					nKeyNameArrNum;					// キー割り当て表の有効データ数
	KeyData				pKeyNameArr[100 + 1];			// キー割り当て表 未割り当てキーコード用にダミーを追加
	BYTE				keyToKeyNameArr[256 + 10];		// キーコード→割り当て表インデックス // 2012.11.25 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu {
	WCHAR			szCustMenuNameArr   [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int				nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	EFunctionCode	nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	KEYCODE			nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	bool			bCustMenuPopupArr   [MAX_CUSTOM_MENU];
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ツールバー                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar {
	int		nToolBarButtonNum;								// ツールバーボタンの数
	int		nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	// ツールバーボタン構造体
	bool	bToolBarIsFlat;									// フラットツールバーにする／しない
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      強調キーワード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword {
	// 強調キーワード設定
	KeywordSetMgr		keywordSetMgr;					// 強調キーワード
	char				szKeywordSetDir[MAX_PATH];		// 強調キーワードファイルのディレクトリ
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           支援                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper {
	// 入力補完機能
	bool		bHokanKey_RETURN;				// VK_RETURN	補完決定キーが有効/無効
	bool		bHokanKey_TAB;					// VK_TAB		補完決定キーが有効/無効
	bool		bHokanKey_RIGHT;				// VK_RIGHT		補完決定キーが有効/無効
	bool		bHokanKey_SPACE;				// VK_SPACE		補完決定キーが有効/無効 $$$ほぼ未使用

	// 外部ヘルプの設定
	TCHAR		szExtHelp[_MAX_PATH];			// 外部ヘルプ１

	// 外部HTMLヘルプの設定
	TCHAR		szExtHtmlHelp[_MAX_PATH];		// 外部HTMLヘルプ
	bool		bHtmlHelpIsSingle;				// HtmlHelpビューアはひとつ (ビューアを複数起動しない)

	// migemo設定
	TCHAR		szMigemoDll[_MAX_PATH];			// migemo dll
	TCHAR		szMigemoDict[_MAX_PATH];		// migemo dict

	// キーワードヘルプ
	LOGFONT		lf;								// キーワードヘルプのフォント情報		// ai 02/05/21 Add
	INT			nPointSize;						// キーワードヘルプのフォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji

	// INI内設定のみ
	bool		bUseHokan;						// 入力補完機能を使用する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          マクロ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro {
	TCHAR		szKeyMacroFileName[MAX_PATH];	// キーボードマクロのファイル名
	MacroRec	macroTable[MAX_CUSTMACRO];		// キー割り当て用マクロテーブル	 Sep. 14, 2001 genta
	SFilePath	szMACROFOLDER;					// マクロ用フォルダ
	int			nMacroOnOpened;					// オープン後自動実行マクロ番号		@@@ 2006.09.01 ryoji
	int			nMacroOnTypeChanged;			// タイプ変更後自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int			nMacroOnSave;					// 保存前自動実行マクロ番号			@@@ 2006.09.01 ryoji
	int			nMacroCancelTimer;				// マクロ停止ダイアログ表示待ち時間	@@@ 2011.08.04 syat
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ファイル名表示                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName {
	bool	bTransformShortPath;											// ファイル名の省略表記
	int		nTransformShortMaxWidth;										// ファイル名の省略表記の最大長
	int		nTransformFileNameArrNum;										// ファイル名の簡易表示登録数
	TCHAR	szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];		// ファイル名の簡易表示変換前文字列
	TCHAR	szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];		// ファイル名の簡易表示変換後文字列	//@@@ 2003.04.08 MIK
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アウトライン                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ドッキング配置
enum class DockSideType {
	Float,				// フローティング
	Left,				// 左ドッキング
	Top,				// 上ドッキング
	Right,				// 右ドッキング
	Bottom,				// 下ドッキング
	Undockable = -1,	// ドッキング禁止
};

enum class FileTreeItemType {
	Grep,
	File,
	Folder
};

struct FileTreeItem {
public:
	FileTreeItemType eFileTreeItemType;
	SFilePath	szTargetPath;					// フォルダorファイルパス
	StaticString<TCHAR,_MAX_PATH> szLabelName;	// ラベル名(""のときはファイル名を使う)
	int  nDepth;	// 階層

	// GrepタイプTreeItem
	StaticString<TCHAR,_MAX_PATH>	szTargetFile;	// ファイル一覧
	bool		bIgnoreHidden;		// 隠しファイルを除く
	bool		bIgnoreReadOnly;	// 読み取り専用ファイルを除く
	bool		bIgnoreSystem;		// システムファイルを除く

	FileTreeItem()
		: eFileTreeItemType(FileTreeItemType::Grep)
		, nDepth(0)
		, bIgnoreHidden(true)
		, bIgnoreReadOnly(false)
		, bIgnoreSystem(false)
		{}
};

struct FileTree {
	bool		bProject;			// プロジェクトファイルモード
	SFilePath	szProjectIni;		// デフォルトiniパス
	int			nItemCount;			// ファイルパス数
	FileTreeItem	items[20];		// ツリーアイテム
};


struct CommonSetting_OutLine {
	// 20060201 aroka アウトライン/トピックリスト の位置とサイズを記憶
	bool		bRememberOutlineWindowPos;	// アウトライン/トピックリスト の位置とサイズを記憶する
	int			widthOutlineWindow;			// アウトライン/トピックリスト のサイズ(幅)
	int			heightOutlineWindow;		// アウトライン/トピックリスト のサイズ(高さ)
	int			xOutlineWindowPos;			// アウトライン/トピックリスト の位置(X座標)
	int			yOutlineWindowPos;			// アウトライン/トピックリスト の位置(Y座標)

	int			nOutlineDockSet;			// アウトライン解析のドッキング位置継承方法(0:共通設定, 1:タイプ別設定)
	bool		bOutlineDockSync;			// アウトライン解析のドッキング位置を同期する
	bool		bOutlineDockDisp;			// アウトライン解析表示の有無
	DockSideType	eOutlineDockSide;		// アウトライン解析ドッキング配置
	int			cxOutlineDockLeft;			// アウトラインの左ドッキング幅
	int			cyOutlineDockTop;			// アウトラインの上ドッキング高
	int			cxOutlineDockRight;			// アウトラインの右ドッキング幅
	int			cyOutlineDockBottom;		// アウトラインの下ドッキング高
	enum class OutlineType nDockOutline;	// アウトラインタイプ

	// IDD_FUNCLIST (ツール - アウトライン解析)
	bool		bAutoCloseDlgFuncList;		// アウトラインダイアログを自動的に閉じる
	bool		bFunclistSetFocusOnJump;	// フォーカスを移す 2002.02.08 hor
	bool		bMarkUpBlankLineEnable;	// 空行を無視する 2002.02.08 aroka,hor

	FileTree	fileTree;					// ファイルツリー設定
	SFilePath	fileTreeDefIniName;			// ファイルツリー設定のデフォルトファイル名(GUIなし)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイル内容比較                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare {
	// ファイル内容比較ダイアログ
	bool		bCompareAndTileHorz;		// 文書比較後、左右に並べて表示
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ビュー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View {
	// INI内設定のみ
	LOGFONT		lf;						// 現在のフォント情報
	bool		bFontIs_FixedPitch;		// 現在のフォントは固定幅フォントである
	INT			nPointSize;				// フォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others {
	// INI内設定のみ
	RECT		rcOpenDialog;				//「開く」ダイアログのサイズと位置
	RECT		rcCompareDialog;			//「ファイル比較」ダイアログボックスのサイズと位置
	RECT		rcDiffDialog;				//「DIFF差分表示」ダイアログボックスのサイズと位置
	RECT		rcFavoriteDialog;			//「履歴とお気に入りの管理」ダイアログボックスのサイズと位置
	RECT		rcTagJumpDialog;			//「ダイレクトタグジャンプ候補一覧」ダイアログボックスのサイズと位置
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ステータスバー                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 2008/6/21	Uchi
struct CommonSetting_StatusBar {
	// 表示文字コードの指定
	bool		bDispUniInSjis;				// SJISで文字コード値をUnicodeで表示する
	bool		bDispUniInJis;				// JISで文字コード値をUnicodeで表示する
	bool		bDispUniInEuc;				// EUCで文字コード値をUnicodeで表示する
	bool		bDispUtf8Codepoint;			// UTF-8をコードポイントで表示する
	bool		bDispSPCodepoint;			// サロゲートペアをコードポイントで表示する
	bool		bDispSelCountByByte;		// 選択文字数を文字単位ではなくバイト単位で表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        プラグイン                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// プラグイン状態
enum EPluginState {
	PLS_NONE,			// プラグインテーブルに登録がない
	PLS_INSTALLED,		// 追加された
	PLS_UPDATED,		// 更新された
	PLS_STOPPED,		// 停止している
	PLS_LOADED,			// 読み込まれた
	PLS_DELETED			// 削除された
};

struct PluginRec {
	WCHAR			szId[MAX_PLUGIN_ID];		// プラグインID
	WCHAR			szName[MAX_PLUGIN_NAME];	// プラグインフォルダ/設定ファイル名
	EPluginState	state;						// プラグイン状態。設定ファイルに保存せずメモリ上のみ。
	int 			nCmdNum;					// プラグイン コマンドの数	// 2010/7/3 Uchi
};

struct CommonSetting_Plugin {
	bool			bEnablePlugin;				// プラグインを使用するかどうか
	PluginRec		pluginTable[MAX_PLUGIN];	// プラグインテーブル
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        メインメニュー                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// メインメニュー種類
enum class MainMenuType {
	Node,			// Node
	Leaf,			// 機能コマンド
	Separator,		// 区切線
	Special,		// 特殊機能コマンド
}; 

class MainMenu {
public:
	MainMenuType	type;		// 種類
	EFunctionCode	nFunc;		// Function
	WCHAR			sKey[2];	// アクセスキー
	WCHAR			sName[MAX_MAIN_MENU_NAME_LEN + 1];	// 名前
	int 			nLevel;		// レベル
};

struct CommonSetting_MainMenu {
	int				nVersion;							// メインメニューバージョン
	int				nMenuTopIdx[MAX_MAINMENU_TOP];		// メインメニュートップレベル
	int 			nMainMenuNum;						// メインメニューデータの数
	MainMenu		mainMenuTbl[MAX_MAINMENU];			// メインメニューデータ
	bool 			bMainMenuKeyParentheses;			// アクセスキーを()付で表示
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                                                             //
//                          まとめ                             //
//                                                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 共通設定
struct CommonSetting {
	CommonSetting_General			general;			// 全般
	CommonSetting_Window			window;				// ウィンドウ
	CommonSetting_TabBar			tabBar;				// タブバー
	CommonSetting_Edit				edit;				// 編集
	CommonSetting_File				file;				// ファイル
	CommonSetting_Backup			backup;				// バックアップ
	CommonSetting_Format			format;				// 書式
	CommonSetting_Search			search;				// 検索
	CommonSetting_KeyBind			keyBind;			// キー割り当て
	//
	CommonSetting_CustomMenu		customMenu;			// カスタムメニュー
	CommonSetting_ToolBar			toolBar;			// ツールバー
	CommonSetting_SpecialKeyword	specialKeyword;		// 強調キーワード
	CommonSetting_Helper			helper;				// 支援
	CommonSetting_Macro				macro;				// マクロ
	CommonSetting_FileName			fileName;			// ファイル名表示
	//
	CommonSetting_OutLine			outline;			// アウトライン
	CommonSetting_Compare			compare;			// ファイル内容比較
	CommonSetting_View				view;				// ビュー
	CommonSetting_Others			others;				// その他

	//
	CommonSetting_StatusBar			statusBar;			// ステータスバー		// 2008/6/21 Uchi
	CommonSetting_Plugin			plugin;				// プラグイン 2009/11/30 syat
	CommonSetting_MainMenu			mainMenu;			// メインメニュー		// 2010/5/15 Uchi
};

