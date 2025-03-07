/*!	@file
	@brief 機能分類定義
*/
// キーワード：コマンド一覧順序
// ここに登録されているコマンドが共通設定の機能種別に表示され、キー割り当てにも設定できるようになる
// このファイルは「コマンド一覧」のメニューの順番や表示にも使われている
// sakura_rc.rcファイルの下のほうにあるString Tableも参照のこと

#include "StdAfx.h"
#include "func/Funccode.h"
#include "config/maxdata.h" // MAX_MRU
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "doc/EditDoc.h"
#include "_main/AppMode.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "macro/SMacroMgr.h"
#include "window/EditWnd.h"
#include "docplus/DiffManager.h"
#include "MarkMgr.h"	// CAutoMarkMgr
#include "sakura.hh"

//using namespace nsFuncCode;

const uint16_t nsFuncCode::ppszFuncKind[] = {
	STR_ERR_DLGFUNCLKUP04,	// _T("ファイル操作系"),
	STR_ERR_DLGFUNCLKUP05,	// _T("編集系"),
	STR_ERR_DLGFUNCLKUP06,	// _T("カーソル移動系"),
	STR_ERR_DLGFUNCLKUP07,	// _T("選択系"),
	STR_ERR_DLGFUNCLKUP08,	// _T("矩形選択系"),
	STR_ERR_DLGFUNCLKUP09,	// _T("クリップボード系"),
	STR_ERR_DLGFUNCLKUP10,	// _T("挿入系"),
	STR_ERR_DLGFUNCLKUP11,	// _T("変換系"),
	STR_ERR_DLGFUNCLKUP12,	// _T("検索系"),
	STR_ERR_DLGFUNCLKUP13,	// _T("モード切り替え系"),
	STR_ERR_DLGFUNCLKUP14,	// _T("設定系"),
	STR_ERR_DLGFUNCLKUP15,	// ("マクロ系"),
	STR_ERR_DLGFUNCLKUP16,	// _T("ウィンドウ系"),
	STR_ERR_DLGFUNCLKUP17,	// _T("支援"),
	STR_ERR_DLGFUNCLKUP18	// _T("その他")
};
const size_t nsFuncCode::nFuncKindNum = _countof(nsFuncCode::ppszFuncKind);


// ファイル操作系
const EFunctionCode pnFuncList_File[] = {
	F_FILENEW			,	// 新規作成
	F_FILENEW_NEWWINDOW	,	// 新規ウィンドウを開く
	F_FILEOPEN			,	// 開く
	F_FILEOPEN_DROPDOWN	,	// 開く(ドロップダウン)
	F_FILESAVE			,	// 上書き保存
	F_FILESAVEAS_DIALOG	,	// 名前を付けて保存
	F_FILESAVEALL		,	// 全て上書き保存
	F_FILECLOSE			,	// 閉じて(無題)
	F_FILECLOSE_OPEN	,	// 閉じて開く
	F_WINCLOSE			,	// ウィンドウを閉じる
	F_FILESAVECLOSE		,	// 保存して閉じる
	F_FILE_REOPEN		,	// 開き直す
	F_FILE_REOPEN_SJIS	,	// SJISで開き直す
	F_FILE_REOPEN_JIS	,	// JISで開き直す
	F_FILE_REOPEN_EUC	,	// EUCで開き直す
	F_FILE_REOPEN_LATIN1,	// Latin1で開き直す
	F_FILE_REOPEN_UNICODE,	// Unicodeで開き直す
	F_FILE_REOPEN_UNICODEBE,// UnicodeBEで開き直す
	F_FILE_REOPEN_UTF8	,	// UTF-8で開き直す
	F_FILE_REOPEN_CESU8	,	// CESU-8で開き直す
	F_FILE_REOPEN_UTF7	,	// UTF-7で開き直す
	F_PRINT				,	// 印刷
	F_PRINT_PREVIEW		,	// 印刷Preview
	F_PRINT_PAGESETUP	,	// 印刷ページ設定
	F_OPEN_HfromtoC		,	// 同名のC/C++ヘッダ(ソース)を開く
//	F_OPEN_HHPP			,	// 同名のC/C++ヘッダファイルを開く
//	F_OPEN_CCPP			,	// 同名のC/C++ソースファイルを開く
	F_ACTIVATE_SQLPLUS	,	// Oracle SQL*Plusをアクティブ表示
	F_PLSQL_COMPILE_ON_SQLPLUS,	// Oracle SQL*Plusで実行
	F_BROWSE			,	// ブラウズ
	F_VIEWMODE			,	// ビューモード
	F_PROPERTY_FILE		,	// ファイルのプロパティ
	F_EXITALLEDITORS	,	// 編集の全終了
	F_EXITALL				// サクラエディタの全終了
};
const int nFincList_File_Num = _countof(pnFuncList_File);

// 編集系
const EFunctionCode pnFuncList_Edit[] = {
	F_UNDO				,	// 元に戻す(Undo)
	F_REDO				,	// やり直し(Redo)
	F_DELETE			,	// 削除
	F_DELETE_BACK		,	// カーソル前を削除
	F_WordDeleteToStart	,	// 単語の左端まで削除
	F_WordDeleteToEnd	,	// 単語の右端まで削除
	F_WordCut			,	// 単語切り取り
	F_WordDelete		,	// 単語削除
	F_LineCutToStart	,	// 行頭まで切り取り(改行単位)
	F_LineCutToEnd		,	// 行末まで切り取り(改行単位)
	F_LineDeleteToStart	,	// 行頭まで削除(改行単位)
	F_LineDeleteToEnd	,	// 行末まで削除(改行単位)
	F_CUT_LINE			,	// 行切り取り(折り返し単位)
	F_DELETE_LINE		,	// 行削除(折り返し単位)
	F_DUPLICATELINE		,	// 行の二重化(折り返し単位)
	F_INDENT_TAB		,	// TABインデント
	F_UNINDENT_TAB		,	// 逆TABインデント
	F_INDENT_SPACE		,	// SPACEインデント
	F_UNINDENT_SPACE	,	// 逆SPACEインデント
	F_LTRIM				,	// 左(先頭)の空白を削除
	F_RTRIM				,	// 右(末尾)の空白を削除
	F_SORT_ASC			,	// 選択行の昇順ソート
	F_SORT_DESC			,	// 選択行の降順ソート
	F_MERGE				,	// 選択行のマージ	
	F_RECONVERT				// 再変換
//	F_WORDSREFERENCE		// 単語リファレンス
};
const int nFincList_Edit_Num = _countof(pnFuncList_Edit);


// カーソル移動系
const EFunctionCode pnFuncList_Move[] = {
	F_UP				,	// カーソル上移動
	F_DOWN				,	// カーソル下移動
	F_LEFT				,	// カーソル左移動
	F_RIGHT				,	// カーソル右移動
	F_UP2				,	// カーソル上移動(２行ごと)
	F_DOWN2				,	// カーソル下移動(２行ごと)
	F_WORDLEFT			,	// 単語の左端に移動
	F_WORDRIGHT			,	// 単語の右端に移動
	F_GOLINETOP			,	// 行頭に移動(折り返し単位)
	F_GOLINEEND			,	// 行末に移動(折り返し単位)
//	F_ROLLDOWN			,	// スクロールダウン
//	F_ROLLUP			,	// スクロールアップ
	F_HalfPageUp		,	// 半ページアップ
	F_HalfPageDown		,	// 半ページダウン
	F_1PageUp			,	// １ページアップ
	F_1PageDown			,	// １ページダウン
	F_GOFILETOP			,	// ファイルの先頭に移動
	F_GOFILEEND			,	// ファイルの最後に移動
	F_CURLINECENTER		,	// カーソル行をウィンドウ中央へ
	F_JUMP_DIALOG		,	// 指定行ヘジャンプ
	F_JUMP_SRCHSTARTPOS	,	// 検索開始位置へ戻る
	F_JUMPHIST_PREV		,	// 移動履歴: 前へ
	F_JUMPHIST_NEXT		,	// 移動履歴: 次へ
	F_JUMPHIST_SET		,	// 現在位置を移動履歴に登録
	F_WndScrollDown		,	// テキストを１行下へスクロール
	F_WndScrollUp		,	// テキストを１行上へスクロール
	F_GONEXTPARAGRAPH	,	// 次の段落へ移動
	F_GOPREVPARAGRAPH	,	// 前の段落へ移動
	F_AUTOSCROLL		,	// オートスクロール
	F_WHEELUP			,	// ホイールアップ
	F_WHEELDOWN			,	// ホイールダウン
	F_WHEELLEFT			,	// ホイール左
	F_WHEELRIGHT		,	// ホイール右
	F_WHEELPAGEUP		,	// ホイールページアップ
	F_WHEELPAGEDOWN		,	// ホイールページダウン
	F_WHEELPAGELEFT		,	// ホイールページ左
	F_WHEELPAGERIGHT	,	// ホイールページ右
};
const int nFincList_Move_Num = _countof(pnFuncList_Move);


// 選択系
const EFunctionCode pnFuncList_Select[] = {
	F_SELECTWORD			,	// 現在位置の単語選択
	F_SELECTALL				,	// すべて選択
	F_SELECTLINE			,	// 1行選択
	F_BEGIN_SEL				,	// 範囲選択開始
	F_UP_SEL				,	// (範囲選択)カーソル上移動
	F_DOWN_SEL				,	// (範囲選択)カーソル下移動
	F_LEFT_SEL				,	// (範囲選択)カーソル左移動
	F_RIGHT_SEL				,	// (範囲選択)カーソル右移動
	F_UP2_SEL				,	// (範囲選択)カーソル上移動(２行ごと)
	F_DOWN2_SEL				,	// (範囲選択)カーソル下移動(２行ごと)
	F_WORDLEFT_SEL			,	// (範囲選択)単語の左端に移動
	F_WORDRIGHT_SEL			,	// (範囲選択)単語の右端に移動
	F_GOLINETOP_SEL			,	// (範囲選択)行頭に移動(折り返し単位)
	F_GOLINEEND_SEL			,	// (範囲選択)行末に移動(折り返し単位)
//	F_ROLLDOWN_SEL			,	// (範囲選択)スクロールダウン
//	F_ROLLUP_SEL			,	// (範囲選択)スクロールアップ
	F_HalfPageUp_Sel		,	// (範囲選択)半ページアップ
	F_HalfPageDown_Sel		,	// (範囲選択)半ページダウン
	F_1PageUp_Sel			,	// (範囲選択)１ページアップ
	F_1PageDown_Sel			,	// (範囲選択)１ページダウン
	F_GOFILETOP_SEL			,	// (範囲選択)ファイルの先頭に移動
	F_GOFILEEND_SEL			,	// (範囲選択)ファイルの最後に移動
	F_GONEXTPARAGRAPH_SEL	,	// (範囲選択)次の段落へ移動
	F_GOPREVPARAGRAPH_SEL		// (範囲選択)前の段落へ移動
};
const int nFincList_Select_Num = _countof(pnFuncList_Select);


// 矩形選択系
const EFunctionCode pnFuncList_Box[] = {
//	F_BOXSELALL			,	// 矩形ですべて選択
	F_BEGIN_BOX			,	// 矩形範囲選択開始
	F_UP_BOX			,	// (矩形選択)カーソル上移動
	F_DOWN_BOX			,	// (矩形選択)カーソル下移動
	F_LEFT_BOX			,	// (矩形選択)カーソル左移動
	F_RIGHT_BOX			,	// (矩形選択)カーソル右移動
	F_UP2_BOX			,	// (矩形選択)カーソル上移動(２行ごと)
	F_DOWN2_BOX			,	// (矩形選択)カーソル下移動(２行ごと)
	F_WORDLEFT_BOX		,	// (矩形選択)単語の左端に移動
	F_WORDRIGHT_BOX		,	// (矩形選択)単語の右端に移動
	F_GOLOGICALLINETOP_BOX	,	// (矩形選択)行頭に移動(改行単位)
	F_GOLINETOP_BOX		,	// (矩形選択)行頭に移動(折り返し単位)
	F_GOLINEEND_BOX		,	// (矩形選択)行末に移動(折り返し単位)
	F_HalfPageUp_BOX	,	// (矩形選択)半ページアップ
	F_HalfPageDown_BOX	,	// (矩形選択)半ページダウン
	F_1PageUp_BOX		,	// (矩形選択)１ページアップ
	F_1PageDown_BOX		,	// (矩形選択)１ページダウン
	F_GOFILETOP_BOX		,	// (矩形選択)ファイルの先頭に移動
	F_GOFILEEND_BOX			// (矩形選択)ファイルの最後に移動
};
const int nFincList_Box_Num = _countof(pnFuncList_Box);


// クリップボード系
const EFunctionCode pnFuncList_Clip[] = {
	F_CUT						,	// 切り取り(選択範囲をクリップボードにコピーして削除)
	F_COPY						,	// コピー(選択範囲をクリップボードにコピー)
	F_COPY_ADDCRLF				,	// 折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	F_COPY_CRLF					,	// CRLF改行でコピー
	F_PASTE						,	// 貼り付け(クリップボードから貼り付け)
	F_PASTEBOX					,	// 矩形貼り付け(クリップボードから矩形貼り付け)
//	F_INSTEXT_W					,	// テキストを貼り付け
//	F_ADDTAIL_W					,	// 最後にテキストを追加
	F_COPYLINES					,	// 選択範囲内全行コピー
	F_COPYLINESASPASSAGE		,	// 選択範囲内全行引用符付きコピー
	F_COPYLINESWITHLINENUMBER	,	// 選択範囲内全行行番号付きコピー
	F_COPY_COLOR_HTML			,	// 選択範囲内色付きHTMLコピー
	F_COPY_COLOR_HTML_LINENUMBER,	// 選択範囲内行番号色付きHTMLコピー
	F_COPYFNAME					,	// このファイル名をクリップボードにコピー
	F_COPYPATH					,	// このファイルのパス名をクリップボードにコピー
	F_COPYTAG					,	// このファイルのパス名とカーソル位置をコピー
	F_CREATEKEYBINDLIST				// キー割り当て一覧をコピー
};
const int nFincList_Clip_Num = _countof(pnFuncList_Clip);


// 挿入系
const EFunctionCode pnFuncList_Insert[] = {
	F_INS_DATE				,	// 日付挿入
	F_INS_TIME				,	// 時刻挿入
	F_CTRL_CODE_DIALOG			// コントロールコードの入力
};
const int nFincList_Insert_Num = _countof(pnFuncList_Insert);


// 変換系
const EFunctionCode pnFuncList_Convert[] = {
	F_TOLOWER				,	// 小文字
	F_TOUPPER				,	// 大文字
	F_TOHANKAKU				,	// 全角→半角
	F_TOZENKAKUKATA			,	// 半角＋全ひら→全角・カタカナ
	F_TOZENKAKUHIRA			,	// 半角＋全カタ→全角・ひらがな
	F_TOZENEI				,	// 半角英数→全角英数
	F_TOHANEI				,	// 全角英数→半角英数
	F_TOHANKATA				,	// 全角カタカナ→半角カタカナ
	F_HANKATATOZENKATA		,	// 半角カタカナ→全角カタカナ
	F_HANKATATOZENHIRA		,	// 半角カタカナ→全角ひらがな
	F_TABTOSPACE			,	// TAB→空白
	F_SPACETOTAB			,	// 空白→TAB
	F_CODECNV_AUTO2SJIS		,	// 自動判別→SJISコード変換
	F_CODECNV_EMAIL			,	// E-Mail(JIS→SJIS)コード変換
	F_CODECNV_EUC2SJIS		,	// EUC→SJISコード変換
	F_CODECNV_UNICODE2SJIS	,	// Unicode→SJISコード変換
	F_CODECNV_UNICODEBE2SJIS,	// Unicode→SJISコード変換
	F_CODECNV_UTF82SJIS		,	// UTF-8→SJISコード変換
	F_CODECNV_UTF72SJIS		,	// UTF-7→SJISコード変換
	F_CODECNV_SJIS2JIS		,	// SJIS→JISコード変換
	F_CODECNV_SJIS2EUC		,	// SJIS→EUCコード変換
	F_CODECNV_SJIS2UTF8		,	// SJIS→UTF-8コード変換
	F_CODECNV_SJIS2UTF7		,	// SJIS→UTF-7コード変換
	F_BASE64DECODE			,	// Base64デコードして保存
	F_UUDECODE					// uudecodeして保存
};
const int nFincList_Convert_Num = _countof(pnFuncList_Convert);


// 検索系
const EFunctionCode pnFuncList_Search[] = {
	F_SEARCH_DIALOG		,	// 検索(単語検索ダイアログ)
	F_SEARCH_BOX		,	// 検索(ボックス)
	F_SEARCH_NEXT		,	// 次を検索
	F_SEARCH_PREV		,	// 前を検索
	F_REPLACE_DIALOG	,	// 置換
	F_SEARCH_CLEARMARK	,	// 検索マークのクリア
	F_JUMP_SRCHSTARTPOS	,	// 検索開始位置へ戻る
	F_GREP_DIALOG		,	// Grep
	F_GREP_REPLACE_DLG	,	//Grep置換
	F_JUMP_DIALOG		,	// 指定行ヘジャンプ
	F_OUTLINE			,	// アウトライン解析
	F_OUTLINE_TOGGLE	,	// アウトライン解析(toggle)
	F_FILETREE			,	//ファイルツリー
	F_TAGJUMP			,	// タグジャンプ機能
	F_TAGJUMP_CLOSE		,	// 閉じてタグジャンプ(元ウィンドウを閉じる)
	F_TAGJUMPBACK		,	// タグジャンプバック機能
	F_TAGS_MAKE			,	// タグファイルの作成
	F_DIRECT_TAGJUMP	,	// ダイレクトタグジャンプ
	F_TAGJUMP_KEYWORD	,	// キーワードを指定してダイレクトタグジャンプ
	F_COMPARE			,	// ファイル内容比較
	F_DIFF_DIALOG		,	// DIFF差分表示(ダイアログ)
	F_DIFF_NEXT			,	// 次の差分へ
	F_DIFF_PREV			,	// 前の差分へ
	F_DIFF_RESET		,	// 差分の全解除
	F_BRACKETPAIR		,	// 対括弧の検索
	F_BOOKMARK_SET		,	// ブックマーク設定・解除
	F_BOOKMARK_NEXT		,	// 次のブックマークへ
	F_BOOKMARK_PREV		,	// 前のブックマークへ
	F_BOOKMARK_RESET	,	// ブックマークの全解除
	F_BOOKMARK_VIEW		,	// ブックマークの一覧
	F_ISEARCH_NEXT	    ,   // 前方インクリメンタルサーチ
	F_ISEARCH_PREV		,	// 後方インクリメンタルサーチ
	F_ISEARCH_REGEXP_NEXT,	// 前方正規表現インクリメンタルサーチ
	F_ISEARCH_REGEXP_PREV,	// 後方正規表現インクリメンタルサーチ
	F_ISEARCH_MIGEMO_NEXT,	// 前方MIGEMOインクリメンタルサーチ
	F_ISEARCH_MIGEMO_PREV,	// 後方MIGEMOインクリメンタルサーチ
	F_FUNCLIST_NEXT		,	// 次の関数リストマーク
	F_FUNCLIST_PREV		,	// 前の関数リストマーク
};
const int nFincList_Search_Num = _countof(pnFuncList_Search);


// モード切り替え系
const EFunctionCode pnFuncList_Mode[] = {
	F_CHGMOD_INS		,	// 挿入／上書きモード切り替え
	F_CHG_CHARSET		,	// 文字コードセット指定
	F_CHGMOD_EOL_CRLF	,	// 入力改行コード指定(CRLF)
	F_CHGMOD_EOL_LF		,	// 入力改行コード指定(LF)
	F_CHGMOD_EOL_CR		,	// 入力改行コード指定(CR)
	F_CANCEL_MODE			// 各種モードの取り消し
};
const int nFincList_Mode_Num = _countof(pnFuncList_Mode);


// 設定系
const EFunctionCode pnFuncList_Set[] = {
	F_SHOWTOOLBAR		,	// ツールバーの表示
	F_SHOWFUNCKEY		,	// ファンクションキーの表示
	F_SHOWTAB			,	// タブの表示
	F_SHOWSTATUSBAR		,	// ステータスバーの表示
	F_SHOWMINIMAP		,	// ミニマップの表示
	F_TYPE_LIST			,	// タイプ別設定一覧
	F_OPTION_TYPE		,	// タイプ別設定
	F_OPTION			,	// 共通設定
	F_FONT				,	// フォント設定
	F_SETFONTSIZEUP		,	// フォントサイズ拡大
	F_SETFONTSIZEDOWN	,	// フォントサイズ縮小
	F_WRAPWINDOWWIDTH	,	// 現在のウィンドウ幅で折り返し
	F_PRINT_PAGESETUP	,	// 印刷ページ設定
	F_FAVORITE			,	// 履歴の管理
	F_TMPWRAPNOWRAP		,	// 折り返さない（一時設定）
	F_TMPWRAPSETTING	,	// 指定桁で折り返す（一時設定）
	F_TMPWRAPWINDOW		,	// 右端で折り返す（一時設定）
	F_SELECT_COUNT_MODE		// 文字カウント設定
};
int		nFincList_Set_Num = _countof(pnFuncList_Set);


// マクロ系
const EFunctionCode pnFuncList_Macro[] = {
	F_RECKEYMACRO	,	// キーマクロの記録開始／終了
	F_SAVEKEYMACRO	,	// キーマクロの保存
	F_LOADKEYMACRO	,	// キーマクロの読み込み
	F_EXECKEYMACRO	,	// キーマクロの実行
	F_EXECEXTMACRO	,	// 名前を指定してマクロ実行
//	F_EXECCMMAND		// 外部コマンド実行
	F_EXECMD_DIALOG		// 外部コマンド実行

};
const int nFincList_Macro_Num = _countof(pnFuncList_Macro);

// ウィンドウ系
const EFunctionCode pnFuncList_Win[] = {
	F_SPLIT_V			,	// 上下に分割
	F_SPLIT_H			,	// 左右に分割
	F_SPLIT_VH			,	// 縦横に分割
	F_WINCLOSE			,	// ウィンドウを閉じる
	F_WIN_CLOSEALL		,	// すべてのウィンドウを閉じる
	F_TAB_CLOSEOTHER	,	// このタブ以外を閉じる
	F_NEXTWINDOW		,	// 次のウィンドウ
	F_PREVWINDOW		,	// 前のウィンドウ
 	F_WINLIST			,	// 開いているウィンドウ一覧ポップアップ表示
	F_CASCADE			,	// 重ねて表示
	F_TILE_V			,	// 上下に並べて表示
	F_TILE_H			,	// 左右に並べて表示
	F_TOPMOST			,	// 常に手前に表示
	F_BIND_WINDOW		,	// 結合して表示
	F_GROUPCLOSE		,	// グループを閉じる
	F_NEXTGROUP			,	// 次のグループ
	F_PREVGROUP			,	// 前のグループ
	F_TAB_MOVERIGHT		,	// タブを右に移動
	F_TAB_MOVELEFT		,	// タブを左に移動
	F_TAB_SEPARATE		,	// 新規グループ
	F_TAB_JOINTNEXT		,	// 次のグループに移動
	F_TAB_JOINTPREV		,	// 前のグループに移動
	F_TAB_CLOSELEFT 	,	// 左をすべて閉じる
	F_TAB_CLOSERIGHT	,	// 右をすべて閉じる
	F_MAXIMIZE_V		,	// 縦方向に最大化
	F_MAXIMIZE_H		,	// 横方向に最大化
	F_MINIMIZE_ALL		,	// すべて最小化
	F_REDRAW			,	// 再描画
	F_WIN_OUTPUT		,	// アウトプットウィンドウ表示
	F_TAB_1				,	// Tab1
	F_TAB_2				,	// Tab2
	F_TAB_3				,	// Tab3
	F_TAB_4				,	// Tab4
	F_TAB_5				,	// Tab5
	F_TAB_6				,	// Tab6
	F_TAB_7				,	// Tab7
	F_TAB_8				,	// Tab8
	F_TAB_9				,	// Tab9
};
const int nFincList_Win_Num = _countof(pnFuncList_Win);


// 支援
const EFunctionCode pnFuncList_Support[] = {
	F_HOKAN						,	// 入力補完
	F_TOGGLE_KEY_SEARCH			,	// キャレット位置の単語を辞書検索する機能ON/OFF
	F_HELP_CONTENTS				,	// ヘルプ目次
	F_HELP_SEARCH				,	// ヘルプキーワード検索
	F_MENU_ALLFUNC				,	// コマンド一覧
	F_EXTHELP1					,	// 外部ヘルプ１
	F_EXTHTMLHELP				,	// 外部HTMLヘルプ
	F_ABOUT							// バージョン情報
};
const int nFincList_Support_Num = _countof(pnFuncList_Support);

// その他
const EFunctionCode pnFuncList_Others[] = {
	F_DISABLE
};
const int nFincList_Others_Num = _countof(pnFuncList_Others);

// 特殊機能
const EFunctionCode nsFuncCode::pnFuncList_Special[] = {
	F_WINDOW_LIST,
	F_FILE_USED_RECENTLY,
	F_FOLDER_USED_RECENTLY,
	F_CUSTMENU_LIST,
	F_USERMACRO_LIST,
	F_PLUGIN_LIST,
};
const size_t nsFuncCode::nFuncList_Special_Num = _countof(nsFuncCode::pnFuncList_Special);


const int nsFuncCode::pnFuncListNumArr[] = {
	nFincList_File_Num,		// ファイル操作系
	nFincList_Edit_Num,		// 編集系
	nFincList_Move_Num,		// カーソル移動系
	nFincList_Select_Num,	// 選択系
	nFincList_Box_Num,		// 矩形選択系
	nFincList_Clip_Num,		// クリップボード系
	nFincList_Insert_Num,	// 挿入系 
	nFincList_Convert_Num,	// 変換系
	nFincList_Search_Num,	// 検索系
	nFincList_Mode_Num,		// モード切り替え系
	nFincList_Set_Num,		// 設定系
	nFincList_Macro_Num,	// マクロ系
	nFincList_Win_Num,		// ウィンドウ系
	nFincList_Support_Num,	// 支援
	nFincList_Others_Num	// その他
};
const EFunctionCode* nsFuncCode::ppnFuncListArr[] = {
	pnFuncList_File,	// ファイル操作系
	pnFuncList_Edit,	// 編集系
	pnFuncList_Move,	// カーソル移動系
	pnFuncList_Select,	// 選択系
	pnFuncList_Box,		// 矩形選択系
	pnFuncList_Clip,	// クリップボード系
	pnFuncList_Insert,	// 挿入系
	pnFuncList_Convert,	// 変換系
	pnFuncList_Search,	// 検索系
	pnFuncList_Mode,	// モード切り替え系
	pnFuncList_Set,		// 設定系
	pnFuncList_Macro,	// マクロ系
// カスタムメニューの文字列を動的に変更可能にするためこれは削除
	pnFuncList_Win,		// ウィンドウ系
	pnFuncList_Support,	// 支援
	pnFuncList_Others	// その他
};
const size_t nsFuncCode::nFincListNumArrNum = _countof(nsFuncCode::pnFuncListNumArr);


// 機能番号に応じてヘルプトピック番号を返す
/*!
	@param nFuncID 機能番号
	@return ヘルプトピック番号。該当IDが無い場合には0を返す。

	内容はcase文の羅列。
*/
int FuncID_To_HelpContextID(EFunctionCode nFuncID)
{
	switch (nFuncID) {

	// ファイル操作系
	case F_FILENEW:				return HLP000025;			// 新規作成
	case F_FILENEW_NEWWINDOW:	return HLP000339;			// 新規ウィンドウで開く
	case F_FILEOPEN:			return HLP000015;			// 開く
	case F_FILEOPEN_DROPDOWN:	return HLP000015;			// 開く(ドロップダウン)
	case F_FILESAVE:			return HLP000020;			// 上書き保存
	case F_FILESAVEAS_DIALOG:	return HLP000021;			// 名前を付けて保存
	case F_FILESAVEALL:			return HLP000313;			// すべて上書き保存
	case F_FILESAVECLOSE:		return HLP000287;			// 保存して閉じる
	case F_FILECLOSE:			return HLP000017;			// 閉じて(無題)
	case F_FILECLOSE_OPEN:		return HLP000119;			// 閉じて開く
	case F_FILE_REOPEN:			return HLP000283;			// 開き直す
	case F_FILE_REOPEN_SJIS:	return HLP000156;			// SJISで開き直す
	case F_FILE_REOPEN_JIS:		return HLP000157;			// JISで開き直す
	case F_FILE_REOPEN_EUC:		return HLP000158;			// EUCで開き直す
	case F_FILE_REOPEN_LATIN1:	return HLP000341;			// Latin1で開き直す
	case F_FILE_REOPEN_UNICODE:	return HLP000159;			// Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE:	return HLP000256;		// UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8:	return HLP000160;			// UTF-8で開き直す
	case F_FILE_REOPEN_CESU8:	return HLP000337;			// CESU-8で開き直す
	case F_FILE_REOPEN_UTF7:	return HLP000161;			// UTF-7で開き直す
	case F_PRINT:				return HLP000162;			// 印刷
	case F_PRINT_PREVIEW:		return HLP000120;			// 印刷Preview
	case F_PRINT_PAGESETUP:		return HLP000122;			// 印刷ページ設定
	case F_OPEN_HfromtoC:		return HLP000192;			// 同名のC/C++ヘッダ(ソース)を開く
//	case F_OPEN_HHPP:			return HLP000024;			// 同名のC/C++ヘッダファイルを開く
//	case F_OPEN_CCPP:			return HLP000026;			// 同名のC/C++ソースファイルを開く
	case F_ACTIVATE_SQLPLUS:	return HLP000132;			// Oracle SQL*Plusをアクティブ表示
	case F_PLSQL_COMPILE_ON_SQLPLUS:	return HLP000027;	// Oracle SQL*Plusで実行
	case F_BROWSE:				return HLP000121;			// ブラウズ
	case F_VIEWMODE:			return HLP000249;			// ビューモード
	case F_PROPERTY_FILE:		return HLP000022;			// ファイルのプロパティ
	case F_PROFILEMGR:			return HLP000363;			//プロファイルマネージャ

	case F_EXITALLEDITORS:	return HLP000030;				// 編集の全終了
	case F_EXITALL:			return HLP000028;				// サクラエディタの全終了

	// 編集系
	case F_UNDO:						return HLP000032;	// 元に戻す(Undo)
	case F_REDO:						return HLP000033;	// やり直し(Redo)
	case F_DELETE:						return HLP000041;	// 削除
	case F_DELETE_BACK:					return HLP000042;	// カーソル前を削除
	case F_WordDeleteToStart:			return HLP000166;	// 単語の左端まで削除
	case F_WordDeleteToEnd:				return HLP000167;	// 単語の右端まで削除
	case F_WordCut:						return HLP000169;	// 単語切り取り
	case F_WordDelete:					return HLP000168;	// 単語削除
	case F_LineCutToStart:				return HLP000172;	// 行頭まで切り取り(改行単位)
	case F_LineCutToEnd:				return HLP000173;	// 行末まで切り取り(改行単位)
	case F_LineDeleteToStart:			return HLP000170;	// 行頭まで削除(改行単位)
	case F_LineDeleteToEnd:				return HLP000171;	// 行末まで削除(改行単位)
	case F_CUT_LINE:					return HLP000174;	// 行切り取り(折り返し単位)
	case F_DELETE_LINE:					return HLP000137;	// 行削除(折り返し単位)
	case F_DUPLICATELINE:				return HLP000043;	// 行の二重化(折り返し単位)
	case F_INDENT_TAB:					return HLP000113;	// TABインデント
	case F_UNINDENT_TAB:				return HLP000113;	// 逆TABインデント
	case F_INDENT_SPACE:				return HLP000114;	// SPACEインデント
	case F_UNINDENT_SPACE:				return HLP000114;	// 逆SPACEインデント
	case F_RECONVERT:					return HLP000218;	// 再変換
//	case ORDSREFERENCE:					return ;			// 単語リファレンス


	// カーソル移動系
	case F_UP:				return HLP000289;	// カーソル上移動
	case F_DOWN:			return HLP000289;	// カーソル下移動
	case F_LEFT:			return HLP000289;	// カーソル左移動
	case F_RIGHT:			return HLP000289;	// カーソル右移動
	case F_UP2:				return HLP000220;	// カーソル上移動(２行ごと)
	case F_DOWN2:			return HLP000221;	// カーソル下移動(２行ごと)
	case F_WORDLEFT:		return HLP000222;	// 単語の左端に移動
	case F_WORDRIGHT:		return HLP000223;	// 単語の右端に移動
	case F_GOLINETOP:		return HLP000224;	// 行頭に移動(折り返し単位)
	case F_GOLINEEND:		return HLP000225;	// 行末に移動(折り返し単位)
//	case F_ROLLDOWN:		return ;	// スクロールダウン
//	case F_ROLLUP:			return ;	// スクロールアップ
	case F_HalfPageUp:		return HLP000245;	// 半ページアップ
	case F_HalfPageDown:	return HLP000246;	// 半ページダウン
	case F_1PageUp:			return HLP000226;	// １ページアップ
	case F_1PageDown:		return HLP000227;	// １ページダウン
	case F_GOFILETOP:		return HLP000228;	// ファイルの先頭に移動
	case F_GOFILEEND:		return HLP000229;	// ファイルの最後に移動
	case F_CURLINECENTER:	return HLP000230;	// カーソル行をウィンドウ中央へ
	case F_JUMP_SRCHSTARTPOS:	return HLP000264;	// 検索開始位置へ戻る
	case F_JUMPHIST_PREV:		return HLP000231;	// 移動履歴: 前へ
	case F_JUMPHIST_NEXT:		return HLP000232;	// 移動履歴: 次へ
	case F_JUMPHIST_SET:	return HLP000265;	// 現在位置を移動履歴に登録
	case F_WndScrollDown:	return HLP000198;	// テキストを１行下へスクロール
	case F_WndScrollUp:		return HLP000199;	// テキストを１行上へスクロール
	case F_GONEXTPARAGRAPH:	return HLP000262;	// 前の段落へ移動
	case F_GOPREVPARAGRAPH:	return HLP000263;	// 前の段落へ移動
	case F_AUTOSCROLL:		return HLP000296;	// オートスクロール
	case F_SETFONTSIZEUP:	return HLP000359;	// フォントサイズ拡大
	case F_SETFONTSIZEDOWN:	return HLP000360;	// フォントサイズ縮小
	case F_MODIFYLINE_NEXT:	return HLP000366;	// 次の変更行へ移動
	case F_MODIFYLINE_PREV:	return HLP000367;	// 前の変更行へ移動

	// 選択系
	case F_SELECTWORD:		return HLP000045;	// 現在位置の単語選択
	case F_SELECTALL:		return HLP000044;	// すべて選択
	case F_SELECTLINE:		return HLP000108;	// 1行選択
	case F_BEGIN_SEL:		return HLP000233;	// 範囲選択開始
	case F_UP_SEL:			return HLP000290;	// (範囲選択)カーソル上移動
	case F_DOWN_SEL:		return HLP000290;	// (範囲選択)カーソル下移動
	case F_LEFT_SEL:		return HLP000290;	// (範囲選択)カーソル左移動
	case F_RIGHT_SEL:		return HLP000290;	// (範囲選択)カーソル右移動
	case F_UP2_SEL:			return HLP000234;	// (範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL:		return HLP000235;	// (範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL:	return HLP000236;	// (範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL:	return HLP000237;	// (範囲選択)単語の右端に移動
	case F_GONEXTPARAGRAPH_SEL:	return HLP000273;	// (範囲選択)前の段落へ移動
	case F_GOPREVPARAGRAPH_SEL:	return HLP000274;	// (範囲選択)前の段落へ移動
	case F_GOLINETOP_SEL:	return HLP000238;	// (範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL:	return HLP000239;	// (範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	return ;	// (範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		return ;	// (範囲選択)スクロールアップ
	case F_HalfPageUp_Sel:	return HLP000247;	// (範囲選択)半ページアップ
	case F_HalfPageDown_Sel:return HLP000248;	// (範囲選択)半ページダウン
	case F_1PageUp_Sel:		return HLP000240;	// (範囲選択)１ページアップ
	case F_1PageDown_Sel:	return HLP000241;	// (範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	return HLP000242;	// (範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	return HLP000243;	// (範囲選択)ファイルの最後に移動
	case F_MODIFYLINE_NEXT_SEL:	return HLP000369;	// (範囲選択)次の変更行へ移動
	case F_MODIFYLINE_PREV_SEL:	return HLP000370;	// (範囲選択)前の変更行へ移動


	// 矩形選択系
//	case F_BOXSELALL:		return ;	// 矩形ですべて選択
	case F_BEGIN_BOX:		return HLP000244;	// 矩形範囲選択開始
	case F_UP_BOX:			return HLP000299;	// (矩形選択)カーソル上移動
	case F_DOWN_BOX:		return HLP000299;	// (矩形選択)カーソル下移動
	case F_LEFT_BOX:		return HLP000299;	// (矩形選択)カーソル左移動
	case F_RIGHT_BOX:		return HLP000299;	// (矩形選択)カーソル右移動
	case F_UP2_BOX:			return HLP000344;	// (矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX:		return HLP000345;	// (矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX:	return HLP000346;	// (矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX:	return HLP000347;	// (矩形選択)単語の右端に移動
	case F_GOLINETOP_BOX:	return HLP000350;	// (矩形選択)行頭に移動(折り返し単位)
	case F_GOLOGICALLINETOP_BOX:	return HLP000361;	// (矩形選択)行頭に移動(改行単位)
	case F_GOLINEEND_BOX:	return HLP000351;	// (矩形選択)行末に移動(折り返し単位)
	case F_HalfPageUp_BOX:	return HLP000356;	// (矩形選択)半ページアップ
	case F_HalfPageDown_BOX:return HLP000357;	// (矩形選択)半ページダウン
	case F_1PageUp_BOX:		return HLP000352;	// (矩形選択)１ページアップ
	case F_1PageDown_BOX:	return HLP000353;	// (矩形選択)１ページダウン
	case F_GOFILETOP_BOX:	return HLP000354;	// (矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX:	return HLP000355;	// (矩形選択)ファイルの最後に移動
//	case F_GONEXTPARAGRAPH_BOX:	return HLP000348;	// (範囲選択)次の段落へ
//	case F_GOPREVPARAGRAPH_BOX:	return HLP000349;	// (範囲選択)前の段落へ

	// 整形系
	case F_LTRIM:		return HLP000210;	// 左(先頭)の空白を削除
	case F_RTRIM:		return HLP000211;	// 右(末尾)の空白を削除
	case F_SORT_ASC:	return HLP000212;	// 選択行の昇順ソート
	case F_SORT_DESC:	return HLP000213;	// 選択行の降順ソート
	case F_MERGE:		return HLP000214;	// 選択行のマージ

	// クリップボード系
	case F_CUT:				return HLP000034;			// 切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:			return HLP000035;			// コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:	return HLP000219;			// 折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:		return HLP000163;			// CRLF改行でコピー(選択範囲をクリップボードにコピー)
	case F_PASTE:			return HLP000039;			// 貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:		return HLP000040;			// 矩形貼り付け(クリップボードから矩形貼り付け)
//	case F_INSTEXT_W:		return ;					// テキストを貼り付け
	case F_COPYLINES:				return HLP000036;	// 選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		return HLP000037;	// 選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	return HLP000038;	// 選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:			return HLP000342;	// 選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	return HLP000343;	// 選択範囲内行番号色付きHTMLコピー
	case F_COPYPATH:		return HLP000056;			// このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:			return HLP000175;			// このファイルのパス名とカーソル位置をコピー
	case F_COPYFNAME:		return HLP000303;			// このファイル名をクリップボードにコピー
//	case IDM_TEST_CREATEKEYBINDLIST:	return 57;		// キー割り当て一覧をクリップボードへコピー
	case F_CREATEKEYBINDLIST:		return HLP000057;	// キー割り当て一覧をクリップボードへコピー


	// 挿入系
	case F_INS_DATE:				return HLP000164;	// 日付挿入
	case F_INS_TIME:				return HLP000165;	// 時刻挿入
	case F_CTRL_CODE_DIALOG:		return HLP000255;	// コントロールコード入力


	// 変換系
	case F_TOLOWER:					return HLP000047;	// 小文字
	case F_TOUPPER:					return HLP000048;	// 大文字
	case F_TOHANKAKU:				return HLP000049;	// 全角→半角
	case F_TOHANKATA:				return HLP000258;	// 全角カタカナ→半角カタカナ
	case F_TOZENKAKUKATA:			return HLP000050;	// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			return HLP000051;	// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		return HLP000123;	// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		return HLP000124;	// 半角カタカナ→全角ひらがな
	case F_TOZENEI:					return HLP000200;	// 半角英数→全角英数
	case F_TOHANEI:					return HLP000215;	// 全角英数→半角英数
	case F_TABTOSPACE:				return HLP000182;	// TAB→空白
	case F_SPACETOTAB:				return HLP000196;	// 空白→TAB
	case F_CODECNV_AUTO2SJIS:		return HLP000178;	// 自動判別→SJISコード変換
	case F_CODECNV_EMAIL:			return HLP000052;	// E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS:		return HLP000053;	// EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS:	return HLP000179;	// Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS:	return HLP000257;	// UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS:		return HLP000142;	// UTF-8→SJISコード変換
	case F_CODECNV_UTF72SJIS:		return HLP000143;	// UTF-7→SJISコード変換
	case F_CODECNV_SJIS2JIS:		return HLP000117;	// SJIS→JISコード変換
	case F_CODECNV_SJIS2EUC:		return HLP000118;	// SJIS→EUCコード変換
	case F_CODECNV_SJIS2UTF8:		return HLP000180;	// SJIS→UTF-8コード変換
	case F_CODECNV_SJIS2UTF7:		return HLP000181;	// SJIS→UTF-7コード変換
	case F_BASE64DECODE:			return HLP000054;	// Base64デコードして保存
	case F_UUDECODE:				return HLP000055;	// uudecodeして保存


	// 検索系
	case F_SEARCH_DIALOG:		return HLP000059;	// 検索(単語検索ダイアログ)
	case F_SEARCH_BOX:			return HLP000059;	// 検索(ボックス)
	case F_SEARCH_NEXT:			return HLP000061;	// 次を検索
	case F_SEARCH_PREV:			return HLP000060;	// 前を検索
	case F_REPLACE_DIALOG:		return HLP000062;	// 置換(置換ダイアログ)
	case F_SEARCH_CLEARMARK:	return HLP000136;	// 検索マークのクリア
	case F_GREP_DIALOG:			return HLP000067;	// Grep
	case F_GREP_REPLACE_DLG:	return HLP000362;	// Grep置換
	case F_JUMP_DIALOG:			return HLP000063;	// 指定行へジャンプ
	case F_OUTLINE:				return HLP000064;	// アウトライン解析
	case F_OUTLINE_TOGGLE:		return HLP000317;	// アウトライン解析(トグル)
	case F_TAGJUMP:				return HLP000065;	// タグジャンプ機能
	case F_TAGJUMPBACK:			return HLP000066;	// タグジャンプバック機能
	case F_TAGS_MAKE:			return HLP000280;	// タグファイルの作成
	case F_TAGJUMP_LIST:		return HLP000281;	// タグジャンプ一覧
	case F_DIRECT_TAGJUMP:		return HLP000281;	// ダイレクトタグジャンプ
	case F_TAGJUMP_CLOSE:		return HLP000291;	// 閉じてタグジャンプ(元ウィンドウClose)
	case F_TAGJUMP_KEYWORD:		return HLP000310;	// キーワードを指定してタグジャンプ
	case F_COMPARE:				return HLP000116;	// ファイル内容比較
	case F_DIFF_DIALOG:			return HLP000251;	// DIFF差分表示(ダイアログ)
//	case F_DIFF:				return HLP000251;	// DIFF差分表示
	case F_DIFF_NEXT:			return HLP000252;	// 次の差分へ
	case F_DIFF_PREV:			return HLP000253;	// 前の差分へ
	case F_DIFF_RESET:			return HLP000254;	// 差分の全解除
	case F_BRACKETPAIR:			return HLP000183;	// 対括弧の検索
	case F_BOOKMARK_SET:		return HLP000205;	// ブックマーク設定・解除
	case F_BOOKMARK_NEXT:		return HLP000206;	// 次のブックマークへ
	case F_BOOKMARK_PREV:		return HLP000207;	// 前のブックマークへ
	case F_BOOKMARK_RESET:		return HLP000208;	// ブックマークの全解除
	case F_BOOKMARK_VIEW:		return HLP000209;	// ブックマークの一覧
	case F_ISEARCH_NEXT:		return HLP000304;	// 前方インクリメンタルサーチ
	case F_ISEARCH_PREV:		return HLP000305;	// 後方インクリメンタルサーチ
	case F_ISEARCH_REGEXP_NEXT:	return HLP000306;	// 正規表現前方インクリメンタルサーチ
	case F_ISEARCH_REGEXP_PREV:	return HLP000307;	// 正規表現後方インクリメンタルサーチ
	case F_ISEARCH_MIGEMO_NEXT:	return HLP000308;	// MIGEMO前方インクリメンタルサーチ
	case F_ISEARCH_MIGEMO_PREV:	return HLP000309;	// MIGEMO後方インクリメンタルサーチ
	case F_FUNCLIST_NEXT:		return HLP000364;	// 次の関数リストマーク
	case F_FUNCLIST_PREV:		return HLP000365;	// 前の関数リストマーク
	case F_FILETREE:			return HLP000368;	// ファイルツリー

	// モード切り替え系
	case F_CHGMOD_INS:		return HLP000046;	// 挿入／上書きモード切り替え
	case F_CHG_CHARSET:		return HLP000297;	// 文字コードセット指定
	case F_CHGMOD_EOL_CRLF:	return HLP000285;	// 入力改行コード指定
	case F_CHGMOD_EOL_CR:	return HLP000285;	// 入力改行コード指定
	case F_CHGMOD_EOL_LF:	return HLP000285;	// 入力改行コード指定
	case F_CANCEL_MODE:		return HLP000194;	// 各種モードの取り消し


	// 設定系
	case F_SHOWTOOLBAR:		return HLP000069;	// ツールバーの表示
	case F_SHOWFUNCKEY:		return HLP000070;	// ファンクションキーの表示
	case F_SHOWTAB:			return HLP000282;	// タブの表示
	case F_SHOWSTATUSBAR:	return HLP000134;	// ステータスバーの表示
	case F_SHOWMINIMAP:		return HLP000371;	// ミニマップの表示
	case F_TYPE_LIST:		return HLP000072;	// タイプ別設定一覧
	case F_OPTION_TYPE:		return HLP000073;	// タイプ別設定
	case F_OPTION:			return HLP000076;	// 共通設定
	case F_TYPE_SCREEN:		return HLP000074;	// タイプ別設定『スクリーン』
	case F_TYPE_COLOR:		return HLP000075;	// タイプ別設定『カラー』
	case F_TYPE_WINDOW:		return HLP000319;	// タイプ別設定『ウィンドウ』
	case F_TYPE_HELPER:		return HLP000197;	// タイプ別設定『支援』
	case F_TYPE_REGEX_KEYWORD:	return HLP000203;	// タイプ別設定『正規表現キーワード』
	case F_TYPE_KEYHELP:	return HLP000315;	// タイプ別設定『キーワードヘルプ』
	case F_OPTION_GENERAL:	return HLP000081;	// 共通設定『全般』
	case F_OPTION_WINDOW:	return HLP000146;	// 共通設定『ウィンドウ』
	case F_OPTION_TAB:		return HLP000150;	// 共通設定『タブバー』
	case F_OPTION_EDIT:		return HLP000144;	// 共通設定『編集』
	case F_OPTION_FILE:		return HLP000083;	// 共通設定『ファイル』
	case F_OPTION_BACKUP:	return HLP000145;	// 共通設定『バックアップ』
	case F_OPTION_FORMAT:	return HLP000082;	// 共通設定『書式』
//	case F_OPTION_URL:		return HLP000147;	// 共通設定『クリッカブルURL』
	case F_OPTION_GREP:		return HLP000148;	// 共通設定『Grep』
	case F_OPTION_KEYBIND:	return HLP000084;	// 共通設定『キー割り当て』
	case F_OPTION_CUSTMENU:	return HLP000087;	// 共通設定『カスタムメニュー』
	case F_OPTION_TOOLBAR:	return HLP000085;	// 共通設定『ツールバー』
	case F_OPTION_KEYWORD:	return HLP000086;	// 共通設定『強調キーワード』
	case F_OPTION_HELPER:	return HLP000088;	// 共通設定『支援』
	case F_OPTION_MACRO:	return HLP000201;	// 共通設定『マクロ』
	case F_OPTION_STATUSBAR: return HLP000147;	// 共通設定『ステータスバー』
	case F_OPTION_PLUGIN:	return HLP000151;	// 共通設定『プラグイン』
	case F_OPTION_FNAME:	return HLP000277;	// 共通設定『ファイル名表示』プロパティ
	case F_OPTION_MAINMENU:	return HLP000152;	// 共通設定『メインメニュー』
	case F_FONT:			return HLP000071;	// フォント設定
	case F_WRAPWINDOWWIDTH:	return HLP000184;	// 現在のウィンドウ幅で折り返し
	case F_FAVORITE:		return HLP000279;	// 履歴の管理
	case F_TMPWRAPNOWRAP:	return HLP000340;	// 折り返さない
	case F_TMPWRAPSETTING:	return HLP000340;	// 指定桁で折り返す
	case F_TMPWRAPWINDOW:	return HLP000340;	// 右端で折り返す
	case F_SELECT_COUNT_MODE: return HLP000336;	// 文字カウント方法

	// マクロ
	case F_RECKEYMACRO:		return HLP000125;	// キーマクロ記録開始／終了
	case F_SAVEKEYMACRO:	return HLP000127;	// キーマクロ保存
	case F_LOADKEYMACRO:	return HLP000128;	// キーマクロ読み込み
	case F_EXECKEYMACRO:	return HLP000126;	// キーマクロ実行
	case F_EXECEXTMACRO:	return HLP000332;	// 名前を指定してマクロ実行
//	case F_EXECCMMAND:		return 103; // 外部コマンド実行
	case F_EXECMD_DIALOG:	return HLP000103;	// 外部コマンド実行

	// カスタムメニュー
	case F_MENU_RBUTTON:	return HLP000195;	// 右クリックメニュー
	case F_CUSTMENU_1:	return HLP000186;	// カスタムメニュー1
	case F_CUSTMENU_2:	return HLP000186;	// カスタムメニュー2
	case F_CUSTMENU_3:	return HLP000186;	// カスタムメニュー3
	case F_CUSTMENU_4:	return HLP000186;	// カスタムメニュー4
	case F_CUSTMENU_5:	return HLP000186;	// カスタムメニュー5
	case F_CUSTMENU_6:	return HLP000186;	// カスタムメニュー6
	case F_CUSTMENU_7:	return HLP000186;	// カスタムメニュー7
	case F_CUSTMENU_8:	return HLP000186;	// カスタムメニュー8
	case F_CUSTMENU_9:	return HLP000186;	// カスタムメニュー9
	case F_CUSTMENU_10:	return HLP000186;	// カスタムメニュー10
	case F_CUSTMENU_11:	return HLP000186;	// カスタムメニュー11
	case F_CUSTMENU_12:	return HLP000186;	// カスタムメニュー12
	case F_CUSTMENU_13:	return HLP000186;	// カスタムメニュー13
	case F_CUSTMENU_14:	return HLP000186;	// カスタムメニュー14
	case F_CUSTMENU_15:	return HLP000186;	// カスタムメニュー15
	case F_CUSTMENU_16:	return HLP000186;	// カスタムメニュー16
	case F_CUSTMENU_17:	return HLP000186;	// カスタムメニュー17
	case F_CUSTMENU_18:	return HLP000186;	// カスタムメニュー18
	case F_CUSTMENU_19:	return HLP000186;	// カスタムメニュー19
	case F_CUSTMENU_20:	return HLP000186;	// カスタムメニュー20
	case F_CUSTMENU_21:	return HLP000186;	// カスタムメニュー21
	case F_CUSTMENU_22:	return HLP000186;	// カスタムメニュー22
	case F_CUSTMENU_23:	return HLP000186;	// カスタムメニュー23
	case F_CUSTMENU_24:	return HLP000186;	// カスタムメニュー24


	// ウィンドウ系
	case F_SPLIT_V:			return HLP000093;	// 上下に分割
	case F_SPLIT_H:			return HLP000094;	// 左右に分割
	case F_SPLIT_VH:		return HLP000095;	// 縦横に分割
	case F_WINCLOSE:		return HLP000018;	// ウィンドウを閉じる
	case F_WIN_CLOSEALL:	return HLP000019;	// すべてのウィンドウを閉じる
	case F_NEXTWINDOW:		return HLP000092;	// 次のウィンドウ
	case F_PREVWINDOW:		return HLP000091;	// 前のウィンドウ
	case F_WINLIST:			return HLP000314;	// ウィンドウ一覧
	case F_BIND_WINDOW:		return HLP000311;	// 結合して表示
	case F_CASCADE:			return HLP000138;	// 重ねて表示
	case F_TILE_V:			return HLP000140;	// 上下に並べて表示
	case F_TILE_H:			return HLP000139;	// 左右に並べて表示
	case F_TOPMOST:			return HLP000312;	// 常に手前に表示
	case F_MAXIMIZE_V:		return HLP000141;	// 縦方向に最大化
	case F_MAXIMIZE_H:		return HLP000098;	// 横方向に最大化
	case F_MINIMIZE_ALL:	return HLP000096;	// すべて最小化
	case F_REDRAW:			return HLP000187;	// 再描画
	case F_WIN_OUTPUT:		return HLP000188;	// アウトプットウィンドウ表示
	case F_GROUPCLOSE:		return HLP000320;	// グループを閉じる
	case F_NEXTGROUP:		return HLP000321;	// 次のグループ
	case F_PREVGROUP:		return HLP000322;	// 前のグループ
	case F_TAB_MOVERIGHT:	return HLP000323;	// タブを右に移動
	case F_TAB_MOVELEFT:	return HLP000324;	// タブを左に移動
	case F_TAB_SEPARATE:	return HLP000325;	// 新規グループ
	case F_TAB_JOINTNEXT:	return HLP000326;	// 次のグループに移動
	case F_TAB_JOINTPREV:	return HLP000327;	// 前のグループに移動
	case F_TAB_CLOSEOTHER:	return HLP000333;	// このタブ以外を閉じる
	case F_TAB_CLOSELEFT:	return HLP000334;	// 左をすべて閉じる
	case F_TAB_CLOSERIGHT:	return HLP000335;	// 右をすべて閉じる

	// 支援
	case F_HOKAN:			return HLP000111;	// 入力補完機能
	case F_TOGGLE_KEY_SEARCH:	return HLP000318;	// キャレット位置辞書検索機能ON/OFF
	case F_HELP_CONTENTS:	return HLP000100;	// ヘルプ目次
	case F_HELP_SEARCH:		return HLP000101;	// ヘルプキーワード検索
	case F_MENU_ALLFUNC:	return HLP000189;	// コマンド一覧
	case F_EXTHELP1:		return HLP000190;	// 外部ヘルプ１
	case F_EXTHTMLHELP:		return HLP000191;	// 外部HTMLヘルプ
	case F_ABOUT:			return HLP000102;	// バージョン情報

	// その他
	default:
		if (IDM_SELMRU <= nFuncID && nFuncID < IDM_SELMRU + MAX_MRU) {
			return HLP000029;	// 最近使ったファイル
		}else if (IDM_SELOPENFOLDER <= nFuncID && nFuncID < IDM_SELOPENFOLDER + MAX_OPENFOLDER) {
			return HLP000023;	// 最近使ったフォルダ
		}else if (IDM_SELWINDOW <= nFuncID && nFuncID < IDM_SELWINDOW + MAX_EDITWINDOWS) {
			return HLP000097;	// ウィンドウリスト
		}else if (F_USERMACRO_0 <= nFuncID && nFuncID < F_USERMACRO_0 + MAX_CUSTMACRO) {
			return HLP000202;	// 登録済みマクロ
		}
		return 0;
	}
}

// 機能が利用可能か調べる
bool IsFuncEnable(const EditDoc& editDoc, const DllSharedData& shareData, EFunctionCode nId)
{
	// 書き換え禁止のときを一括チェック
	if (editDoc.IsModificationForbidden(nId))
		return false;

	switch (nId) {
	case F_RECKEYMACRO:	// キーマクロの記録開始／終了
		if (shareData.flags.bRecordingKeyMacro) {	// キーボードマクロの記録中
			return (shareData.flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd());	// キーボードマクロを記録中のウィンドウ
		}else {
			return true;
		}
	case F_SAVEKEYMACRO:	// キーマクロの保存
		// キーマクロエンジン以外のマクロを読み込んでいるときは
		// 実行はできるが保存はできない．
		if (shareData.flags.bRecordingKeyMacro) {	// キーボードマクロの記録中
			return (shareData.flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd());	// キーボードマクロを記録中のウィンドウ
		}else {
			return EditApp::getInstance().pSMacroMgr->IsSaveOk();
		}
	case F_EXECKEYMACRO:	// キーマクロの実行
		if (shareData.flags.bRecordingKeyMacro) {	// キーボードマクロの記録中
			return (shareData.flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd());	// キーボードマクロを記録中のウィンドウ
		}else {
			// szKeyMacroFileNameにファイル名がコピーされているかどうか。
			return (shareData.common.macro.szKeyMacroFileName[0] != NULL);
		}
	case F_LOADKEYMACRO:	// キーマクロの読み込み
		if (shareData.flags.bRecordingKeyMacro) {	// キーボードマクロの記録中
			return (shareData.flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd());	// キーボードマクロを記録中のウィンドウ
		}else {
			return true;
		}
	case F_EXECEXTMACRO:		// 名前を指定してマクロ実行
		return true;

	case F_SEARCH_CLEARMARK:	// 検索マークのクリア
		return true;

	case F_JUMP_SRCHSTARTPOS:	// 検索開始位置へ戻る
		return (editDoc.pEditWnd->GetActiveView().ptSrchStartPos_PHY.BothNatural());

	case F_COMPARE:	// ファイル内容比較
		return (2 <= shareData.nodes.nEditArrNum);

	case F_DIFF_NEXT:	// 次の差分へ
	case F_DIFF_PREV:	// 前の差分へ
	case F_DIFF_RESET:	// 差分の全解除
		return DiffManager::getInstance().IsDiffUse();
	case F_DIFF_DIALOG:	// DIFF差分表示
		return true;

	case F_BEGIN_BOX:	// 矩形範囲選択開始
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		return (shareData.common.view.bFontIs_FixedPitch);	// 現在のフォントは固定幅フォントである

	case F_PASTEBOX:
		// クリップボードから貼り付け可能か？
		return (editDoc.docEditor.IsEnablePaste() && shareData.common.view.bFontIs_FixedPitch);
		
	case F_PASTE:
		// クリップボードから貼り付け可能か？
		return (editDoc.docEditor.IsEnablePaste());

	case F_FILENEW:		// 新規作成
	case F_GREP_DIALOG:	// Grep
		// 編集ウィンドウの上限チェック
		return !(shareData.nodes.nEditArrNum >= MAX_EDITWINDOWS);	// 最大値修正

	case F_FILESAVE:	// 上書き保存
		if (!AppMode::getInstance().IsViewMode()) {	// ビューモード
			if (editDoc.docEditor.IsModified()) {	// 変更フラグ
				return true;
			}else if (editDoc.docFile.IsChgCodeSet()) {	// 文字コードの変更
				return true;
			}else {
				// 無変更でも上書きするか
				return (shareData.common.file.bEnableUnmodifiedOverwrite);
			}
		}else {
			return false;
		}
	case F_COPYLINES:				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	//選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:				//選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	//選択範囲内行番号色付きHTMLコピー
		// テキストが選択されていればtrue
		return editDoc.pEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_TOLOWER:					// 小文字
	case F_TOUPPER:					// 大文字
	case F_TOHANKAKU:				// 全角→半角
	case F_TOHANKATA:				// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					// 半角英数→全角英数
	case F_TOHANEI:					// 全角英数→半角英数
	case F_TOZENKAKUKATA:			// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		// 半角カタカナ→全角ひらがな
	case F_TABTOSPACE:				// TAB→空白
	case F_SPACETOTAB:				// 空白→TAB
	case F_CODECNV_AUTO2SJIS:		// 自動判別→SJISコード変換
	case F_CODECNV_EMAIL:			// E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS:		// EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS:	// Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS:	// UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS:		// UTF-8→SJISコード変換
	case F_CODECNV_UTF72SJIS:		// UTF-7→SJISコード変換
	case F_CODECNV_SJIS2JIS:		// SJIS→JISコード変換
	case F_CODECNV_SJIS2EUC:		// SJIS→EUCコード変換
	case F_CODECNV_SJIS2UTF8:		// SJIS→UTF-8コード変換
	case F_CODECNV_SJIS2UTF7:		// SJIS→UTF-7コード変換
	case F_BASE64DECODE:			// Base64デコードして保存
	case F_UUDECODE:				// uudecodeして保存
		// テキストが選択されていればtrue
		return editDoc.pEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_CUT_LINE:	// 行切り取り(折り返し単位)
	case F_DELETE_LINE:	// 行削除(折り返し単位)
		// テキストが選択されていなければtrue
		return !editDoc.pEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_UNDO:		return editDoc.docEditor.IsEnableUndo();	// Undo(元に戻す)可能な状態か？
	case F_REDO:		return editDoc.docEditor.IsEnableRedo();	// Redo(やり直し)可能な状態か？

	case F_COPYPATH:
	case F_COPYTAG:
	case F_COPYFNAME:
	case F_OPEN_HfromtoC:				// 同名のC/C++ヘッダ(ソース)を開く
//	case F_OPEN_HHPP:					// 同名のC/C++ヘッダファイルを開く
//	case F_OPEN_CCPP:					// 同名のC/C++ソースファイルを開く
	case F_PLSQL_COMPILE_ON_SQLPLUS:	// Oracle SQL*Plusで実行
	case F_BROWSE:						// ブラウズ
	//case F_VIEWMODE:					// ビューモード
	//case F_PROPERTY_FILE:				// ファイルのプロパティ
		return editDoc.docFile.GetFilePathClass().IsValidPath();	// 現在編集中のファイルのパス名をクリップボードにコピーできるか

	case F_JUMPHIST_PREV:	// 移動履歴: 前へ
		return (editDoc.pEditWnd->GetActiveView().pHistory->CheckPrev());

	case F_JUMPHIST_NEXT:	// 移動履歴: 次へ
		return (editDoc.pEditWnd->GetActiveView().pHistory->CheckNext());

	case F_JUMPHIST_SET:	// 現在位置を移動履歴に登録
		return true;
	// (無題)もダイレクトタグジャンプできるように
	case F_DIRECT_TAGJUMP:	// ダイレクトタグジャンプ
	case F_TAGJUMP_KEYWORD:	// キーワードを指定してダイレクトタグジャンプ
		return (!EditApp::getInstance().pGrepAgent->bGrepMode
			&& editDoc.docFile.GetFilePathClass().IsValidPath()
		);
		
	case F_TILE_H:
	case F_TILE_V:
	case F_CASCADE:
		return true;
	case F_BIND_WINDOW:
	case F_TAB_MOVERIGHT:
	case F_TAB_MOVELEFT:
	case F_TAB_CLOSELEFT:
	case F_TAB_CLOSERIGHT:
	case F_TAB_1:
	case F_TAB_2:
	case F_TAB_3:
	case F_TAB_4:
	case F_TAB_5:
	case F_TAB_6:
	case F_TAB_7:
	case F_TAB_8:
	case F_TAB_9:
		// 非タブモード時はウィンドウを結合して表示できない
		return shareData.common.tabBar.bDispTabWnd;
	case F_GROUPCLOSE:
	case F_NEXTGROUP:
	case F_PREVGROUP:
		return (shareData.common.tabBar.bDispTabWnd && !shareData.common.tabBar.bDispTabWndMultiWin);
	case F_TAB_SEPARATE:
	case F_TAB_JOINTNEXT:
	case F_TAB_JOINTPREV:
	case F_FILENEW_NEWWINDOW:
		return (shareData.common.tabBar.bDispTabWnd && !shareData.common.tabBar.bDispTabWndMultiWin);
	}
	return true;
}


// 機能がチェック状態か調べる
bool IsFuncChecked(const EditDoc& editDoc, const DllSharedData& shareData, EFunctionCode nId)
{
	EditWnd* pEditWnd;
	pEditWnd = (EditWnd*)::GetWindowLongPtr(EditWnd::getInstance().GetHwnd(), GWLP_USERDATA);
	EncodingType eDocCode = editDoc.GetDocumentEncoding();
	switch (nId) {
	case F_FILE_REOPEN_SJIS:		return eDocCode == CODE_SJIS;
	case F_FILE_REOPEN_JIS:			return eDocCode == CODE_JIS;
	case F_FILE_REOPEN_EUC:			return eDocCode == CODE_EUC;
	case F_FILE_REOPEN_LATIN1:		return eDocCode == CODE_LATIN1;
	case F_FILE_REOPEN_UNICODE:		return eDocCode == CODE_UNICODE;
	case F_FILE_REOPEN_UNICODEBE:	return eDocCode == CODE_UNICODEBE;
	case F_FILE_REOPEN_UTF8:		return eDocCode == CODE_UTF8;
	case F_FILE_REOPEN_CESU8:		return eDocCode == CODE_CESU8;
	case F_FILE_REOPEN_UTF7:		return eDocCode == CODE_UTF7;
	case F_RECKEYMACRO:	// キーマクロの記録開始／終了
		if (shareData.flags.bRecordingKeyMacro) {	// キーボードマクロの記録中
			return (shareData.flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd());	// キーボードマクロを記録中のウィンドウ
		}else {
			return false;
		}
	case F_SHOWTOOLBAR:			return pEditWnd->toolbar.GetToolbarHwnd() != NULL;
	case F_SHOWFUNCKEY:			return pEditWnd->funcKeyWnd.GetHwnd() != NULL;
	case F_SHOWTAB:				return pEditWnd->tabWnd.GetHwnd() != NULL;
	case F_SHOWSTATUSBAR:		return pEditWnd->statusBar.GetStatusHwnd() != NULL;
	// テキストの折り返し方法
	case F_TMPWRAPNOWRAP:		return (editDoc.nTextWrapMethodCur == TextWrappingMethod::NoWrapping);		// 折り返さない
	case F_TMPWRAPSETTING:		return (editDoc.nTextWrapMethodCur == TextWrappingMethod::SettingWidth);		// 指定桁で折り返す
	case F_TMPWRAPWINDOW:		return (editDoc.nTextWrapMethodCur == TextWrappingMethod::WindowWidth);		// 右端で折り返す
	// 文字カウント方法
	case F_SELECT_COUNT_MODE:	return (pEditWnd->nSelectCountMode == SelectCountMode::Toggle ?
											shareData.common.statusBar.bDispSelCountByByte != FALSE :
											pEditWnd->nSelectCountMode == SelectCountMode::ByByte);
	case F_VIEWMODE:			return AppMode::getInstance().IsViewMode(); // ビューモード
	case F_CHGMOD_EOL_CRLF:		return editDoc.docEditor.GetNewLineCode() == EolType::CRLF;
	case F_CHGMOD_EOL_LF:		return editDoc.docEditor.GetNewLineCode() == EolType::LF;
	case F_CHGMOD_EOL_CR:		return editDoc.docEditor.GetNewLineCode() == EolType::CR;
	case F_CHGMOD_INS:			return editDoc.docEditor.IsInsMode();	// 挿入モードはドキュメント毎に補完するように変更した
	case F_TOGGLE_KEY_SEARCH:	return shareData.common.search.bUseCaretKeyword != FALSE;	// キーワードポップアップのON/OFF状態を反映する
	case F_BIND_WINDOW:			return ((shareData.common.tabBar.bDispTabWnd) && !(shareData.common.tabBar.bDispTabWndMultiWin));
	case F_TOPMOST:				return ((DWORD)::GetWindowLongPtr(pEditWnd->GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
	// インクリメンタルサーチ
	case F_ISEARCH_NEXT:
	case F_ISEARCH_PREV:
	case F_ISEARCH_REGEXP_NEXT:
	case F_ISEARCH_REGEXP_PREV:
	case F_ISEARCH_MIGEMO_NEXT:
	case F_ISEARCH_MIGEMO_PREV:
		return editDoc.pEditWnd->GetActiveView().IsISearchEnabled(nId);
	case F_OUTLINE_TOGGLE: // アウトラインウィンドウ
		// ToDo:ブックマークリストが出ているときもへこんでしまう。
		return editDoc.pEditWnd->dlgFuncList.GetHwnd() != NULL;
	}

	return false;
}

