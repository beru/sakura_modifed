#pragma once

class EditView;
enum EFunctionCode;
class EditDoc;
struct DllSharedData;
class OpeBlk;
class Caret;
class EditWnd;
class ColorStrategy;
class ColorStrategyPool;
class SMacroMgr;
#include "Eol.h"

class ViewCommander {
public:
	ViewCommander(EditView& editView);

public:
	// 外部依存
	EditDoc& GetDocument();
	EditWnd& GetEditWindow();
	HWND GetMainWindow();
	OpeBlk* GetOpeBlk();
	void SetOpeBlk(OpeBlk* p);
	Range& GetSelect();
	Caret& GetCaret();

private:
	EditView&	view;
	SMacroMgr*	pSMacroMgr;

public:
	// キーリピート状態
	int bPrevCommand;

private:
	enum class IndentType {
		None,
		Tab,
		Space
	};

	// -- -- -- -- 以下、コマンド処理関数群 -- -- -- -- //
public:
	bool HandleCommand(
		EFunctionCode	nCommand,
		bool			bRedraw,
		LPARAM			lparam1,
		LPARAM			lparam2,
		LPARAM			lparam3,
		LPARAM			lparam4
	);

	// ファイル操作系
	void Command_FileNew(void);				// 新規作成
	void Command_FileNew_NewWindow(void);	// 新規作成（タブで開く版）
	// ファイルを開く
	void Command_FileOpen(
		const wchar_t*	filename	= NULL,
		EncodingType	nCharCode	= CODE_AUTODETECT,
		bool			bViewMode	= false,
		const wchar_t*	defaultName	= NULL
	);

	// 上書き保存
	bool Command_FileSave(bool warnbeep = true, bool askname = true);
	bool Command_FileSaveAs_Dialog(const wchar_t*, EncodingType, EolType);		// 名前を付けて保存
	bool Command_FileSaveAs(const wchar_t* filename, EolType eEolType);		// 名前を付けて保存
	bool Command_FileSaveAll(void);				// 全て上書き保存
	void Command_FileClose(void);				// 開じて(無題)
	// 閉じて開く
	void Command_FileClose_Open(LPCWSTR filename = NULL,
		EncodingType nCharCode = CODE_AUTODETECT, bool bViewMode = false);

	void Command_File_Reopen(EncodingType nCharCode, bool bNoConfirm);		// 再オープン

	void Command_Print(void);					// 印刷
	void Command_Print_Preview(void);			// 印刷Preview
	void Command_Print_PageSetUp(void);			// 印刷ページ設定
	bool Command_Open_HfromtoC(bool);			// 同名のC/C++ヘッダ(ソース)を開く
	bool Command_Open_HHPP(bool bCheckOnly, bool bBeepWhenMiss);				// 同名のC/C++ヘッダファイルを開く
	bool Command_Open_CCPP(bool bCheckOnly, bool bBeepWhenMiss);				// 同名のC/C++ソースファイルを開く
	void Command_Activate_SQLPlus(void);			// Oracle SQL*Plusをアクティブ表示
	void Command_PLSQL_Compile_On_SQLPlus(void);	// Oracle SQL*Plusで実行
	void Command_Browse(void);					// ブラウズ
	void Command_ViewMode(void);				// ビューモード
	void Command_Property_File(void);			// ファイルのプロパティ
	void Command_ProfileMgr(void);				// プロファイルマネージャ
	void Command_ExitAllEditors(void);			// 編集の全終了
	void Command_ExitAll(void);					// サクラエディタの全終了
	bool Command_PutFile(LPCWSTR, EncodingType, int);	// 作業中ファイルの一時出力
	bool Command_InsFile(LPCWSTR, EncodingType, int);	// キャレット位置にファイル挿入

	// 編集系
	void Command_WCHAR(wchar_t, bool bConvertEOL = true);			// 文字入力
	void Command_IME_CHAR(WORD);			// 全角文字入力
	void Command_Undo(void);				// 元に戻す(Undo)
	void Command_Redo(void);				// やり直し(Redo)
	void Command_Delete(void);				// カーソル位置または選択エリアを削除
	void Command_Delete_Back(void);			// カーソル前を削除
	void Command_WordDeleteToStart(void);	// 単語の左端まで削除
	void Command_WordDeleteToEnd(void);		// 単語の右端まで削除
	void Command_WordCut(void);				// 単語切り取り
	void Command_WordDelete(void);			// 単語削除
	void Command_LineCutToStart(void);		// 行頭まで切り取り(改行単位)
	void Command_LineCutToEnd(void);		// 行末まで切り取り(改行単位)
	void Command_LineDeleteToStart(void);	// 行頭まで削除(改行単位)
	void Command_LineDeleteToEnd(void);  	// 行末まで削除(改行単位)
	void Command_Cut_Line(void);			// 行切り取り(折り返し単位)
	void Command_Delete_Line(void);			// 行削除(折り返し単位)
	void Command_DuplicateLine(void);		// 行の二重化(折り返し単位)
	void Command_Indent(wchar_t cChar, IndentType = IndentType::None); // インデント ver 1
	void Command_Indent(const wchar_t*, size_t, IndentType = IndentType::None);// インデント ver0
	void Command_Unindent(wchar_t wcChar);// 逆インデント
	void Command_Trim(bool);
	void Command_Sort(bool);
	void Command_Merge(void);
	void Command_Reconvert(void);			// メニューからの再変換対応
	void Command_CtrlCode_Dialog(void);		// コントロールコードの入力(ダイアログ)

	// カーソル移動系
	void Command_MoveCursor(Point pos, int option);
	void Command_MoveCursorLayout(Point pos, int option);
	int Command_Up(bool bSelect, bool bRepeat, int line = 0);			// カーソル上移動
	int Command_Down(bool bSelect, bool bRepeat);	// カーソル下移動
	int Command_Left(bool, bool);					// カーソル左移動
	void Command_Right(bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat);	// カーソル右移動
	void Command_Up2(bool bSelect);					// カーソル上移動（２行づつ）
	void Command_Down2(bool bSelect);				// カーソル下移動（２行づつ）
	void Command_WordLeft(bool bSelect);			// 単語の左端に移動
	void Command_WordRight(bool bSelect);			// 単語の右端に移動
	void Command_GoLineTop(bool bSelect, int lparam);	// 行頭に移動（折り返し単位）
	void Command_GoLineEnd(bool bSelect, int, int);	// 行末に移動（折り返し単位）
	void Command_HalfPageUp(bool bSelect, int);		// 半ページアップ
	void Command_HalfPageDown(bool bSelect, int);	// 半ページダウン
	void Command_1PageUp(bool bSelect, int);		// １ページアップ	
	void Command_1PageDown(bool bSelect, int);		// １ページダウン
	void Command_GoFileTop(bool bSelect);			// ファイルの先頭に移動
	void Command_GoFileEnd(bool bSelect);			// ファイルの最後に移動
	void Command_CurLineCenter(void);				// カーソル行をウィンドウ中央へ
	void Command_JumpHist_Prev(void);				// 移動履歴: 前へ
	void Command_JumpHist_Next(void);				// 移動履歴: 次へ
	void Command_JumpHist_Set(void);				// 現在位置を移動履歴に登録
	void Command_WndScrollDown(void);				// テキストを１行下へScroll
	void Command_WndScrollUp(void);					// テキストを１行上へScroll
	void Command_GoNextParagraph(bool bSelect);		// 次の段落へ進む
	void Command_GoPrevParagraph(bool bSelect);		// 前の段落へ戻る
	void Command_AutoScroll();						// Auto Scroll
	void Command_WheelUp(int);
	void Command_WheelDown(int);
	void Command_WheelLeft(int);
	void Command_WheelRight(int);
	void Command_WheelPageUp(int);
	void Command_WheelPageDown(int);
	void Command_WheelPageLeft(int);
	void Command_WheelPageRight(int);
	void Command_ModifyLine_Next(bool bSelect);	// 次の変更行へ
	void Command_ModifyLine_Prev(bool bSelect);	// 前の変更行へ

	// 選択系
	bool Command_SelectWord(const Point* pptCaretPos = nullptr);		// 現在位置の単語選択
	void Command_SelectAll(void);			// すべて選択
	void Command_SelectLine(int lparam);	// 1行選択
	void Command_Begin_Select(void);		// 範囲選択開始

	// 矩形選択系
	void Command_Begin_BoxSelect(bool bSelectingLock = false);	// 矩形範囲選択開始

	// クリップボード系
	void Command_Cut(void);						// 切り取り（選択範囲をクリップボードにコピーして削除
	void Command_Copy(bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);// コピー(選択範囲をクリップボードにコピー)
	void Command_Paste(int option);				// 貼り付け（クリップボードから貼り付け
	void Command_PasteBox(int option);			// 矩形貼り付け（クリップボードから矩形貼り付け
	// 矩形貼り付け（引数渡しでの張り付け）
	void Command_PasteBox(const wchar_t* szPaste, size_t nPasteSize);
	void Command_InsBoxText(const wchar_t*, int); // 矩形貼り付け
	void Command_InsText(bool bRedraw, const wchar_t*, size_t, bool bNoWaitCursor,
		                   bool bLinePaste = false,
                       bool bFastMode = false,
                       const Range* psDelRangeLogicFast = nullptr);
	void Command_AddTail(const wchar_t* pszData, size_t nDataLen);	// 最後にテキストを追加
	void Command_CopyFileName(void);				// このファイル名をクリップボードにコピー
	void Command_CopyPath(void);					// このファイルのパス名をクリップボードにコピー
	void Command_CopyTag(void);						// このファイルのパス名とカーソル位置をコピー
	void Command_CopyLines(void);					// 選択範囲内全行コピー
	void Command_CopyLinesAsPassage(void);			// 選択範囲内全行引用符付きコピー
	void Command_CopyLinesWithLineNumber(void);		// 選択範囲内全行行番号付きコピー
	void Command_Copy_Color_HTML(bool bLineNumber = false);	// 選択範囲内全行行番号付きコピー
	void Command_Copy_Color_HTML_LineNumber(void);		// 選択範囲内色付きHTMLコピー
	ColorStrategy* GetColorStrategyHTML(const StringRef&, int, const ColorStrategyPool*, ColorStrategy**, ColorStrategy**, bool& bChange);
	void Command_CreateKeyBindList(void);				// キー割り当て一覧をコピー

	// 挿入系
	void Command_Ins_Date(void);	// 日付挿入
	void Command_Ins_Time(void);	// 時刻挿入

	// 変換系
	void Command_ToLower(void);					// 小文字
	void Command_ToUpper(void);					// 大文字
	void Command_ToZenkakuKata(void);			// 半角＋全ひら→全角・カタカナ
	void Command_ToZenkakuHira(void);			// 半角＋全カタ→全角・ひらがな
	void Command_ToHankaku(void);				// 全角→半角
	void Command_ToHankata(void);				// 全角カタカナ→半角カタカナ
	void Command_ToZenEi(void);					// 半角英数→全角英数
	void Command_ToHanEi(void);					// 全角英数→半角英数
	void Command_HanKataToZenkakuKata(void);	// 半角カタカナ→全角カタカナ
	void Command_HanKataToZenKakuHira(void);	// 半角カタカナ→全角ひらがな
	void Command_TabToSpace(void);				// TAB→空白
	void Command_SpaceToTab(void);				// 空白→TAB
	void Command_CodeCnv_Auto2SJIS(void);		// 自動判別→SJISコード変換
	void Command_CodeCnv_EMail(void);			// E-Mail(JIS→SJIS)コード変換
	void Command_CodeCnv_EUC2SJIS(void);		// EUC→SJISコード変換
	void Command_CodeCnv_Unicode2SJIS(void);	// Unicode→SJISコード変換
	void Command_CodeCnv_UnicodeBE2SJIS(void);	// UnicodeBE→SJISコード変換
	void Command_CodeCnv_UTF82SJIS(void);		// UTF-8→SJISコード変換
	void Command_CodeCnv_UTF72SJIS(void);		// UTF-7→SJISコード変換
	void Command_CodeCnv_SJIS2JIS(void);		// SJIS→JISコード変換
	void Command_CodeCnv_SJIS2EUC(void);		// SJIS→EUCコード変換
	void Command_CodeCnv_SJIS2UTF8(void);		// SJIS→UTF-8コード変換
	void Command_CodeCnv_SJIS2UTF7(void);		// SJIS→UTF-7コード変換
	void Command_Base64Decode(void);			// Base64デコードして保存
	void Command_UUDecode(void);				// uudecodeして保存

	// 検索系
	void Command_Search_Box(void);						// 検索(ボックス)
	void Command_Search_Dialog(void);					// 検索(単語検索ダイアログ)
	void Command_Search_Next(bool, bool, bool, HWND, const wchar_t*, Range* = nullptr);// 次を検索
	void Command_Search_Prev(bool bReDraw, HWND);		// 前を検索
	void Command_Replace_Dialog(void);					// 置換(置換ダイアログ)
	void Command_Replace(HWND hwndParent);				// 置換(実行)
	void Command_Replace_All();							// すべて置換(実行)
	void Command_Search_ClearMark(void);				// 検索マークのクリア
	void Command_Jump_SrchStartPos(void);				// 検索開始位置へ戻る


	void Command_Grep_Dialog(void);						// Grepダイアログの表示
	void Command_Grep(void);							// Grep
	void Command_Grep_Replace_Dlg( void );				// Grep置換ダイアログの表示
	void Command_Grep_Replace( void );					// Grep置換
	void Command_Jump_Dialog(void);						// 指定行ヘジャンプダイアログの表示
	void Command_Jump(void);							// 指定行ヘジャンプ
	bool Command_FuncList(ShowDialogType nAction, OutlineType outlineType);	// アウトライン解析
	bool Command_TagJump(bool bClose = false);			// タグジャンプ機能
	void Command_TagJumpBack(void);						// タグジャンプバック機能
	bool Command_TagJumpByTagsFileMsg(bool);			// ダイレクトタグジャンプ(通知つき)
	bool Command_TagJumpByTagsFile(bool);				// ダイレクトタグジャンプ

	bool Command_TagsMake(void);						// タグファイルの作成
	bool Command_TagJumpByTagsFileKeyword(const wchar_t* keyword);
	void Command_Compare(void);							// ファイル内容比較
	void Command_Diff_Dialog(void);						// DIFF差分表示ダイアログ
	void Command_Diff(const wchar_t* szTmpFile2, int nFlgOpt);	// DIFF差分表示
	void Command_Diff_Next(void);						// 次の差分へ
	void Command_Diff_Prev(void);						// 前の差分へ
	void Command_Diff_Reset(void);						// 差分の全解除
	void Command_BracketPair(void);						// 対括弧の検索
	void Command_Bookmark_Set(void);					// ブックマーク設定・解除
	void Command_Bookmark_Next(void);					// 次のブックマークへ
	void Command_Bookmark_Prev(void);					// 前のブックマークへ
	void Command_Bookmark_Reset(void);					// ブックマークの全解除
	void Command_Bookmark_Pattern(void);				// 指定パターンに一致する行をマーク
	void Command_FuncList_Next( void );					// 次の関数リストマーク
	void Command_FuncList_Prev( void );					// 前の関数リストマーク

	// モード切り替え系
	void Command_ChgMod_Ins(void);						// 挿入／上書きモード切り替え
	void Command_Chg_Charset(EncodingType, bool);		// 文字コードセット指定
	void Command_ChgMod_EOL(EolType);					// 入力する改行コードを設定
	void Command_Cancel_Mode(int whereCursorIs = 0);	// 各種モードの取り消し

	// 設定系
	void Command_ShowToolBar(void);					// ツールバーの表示/非表示
	void Command_ShowFuncKey(void);					// ファンクションキーの表示/非表示
	void Command_ShowTab(void);						// タブの表示/非表示
	void Command_ShowStatusBar(void);				// ステータスバーの表示/非表示
	void Command_ShowMiniMap(void);					// ミニマップの表示/非表示
	void Command_Type_List(void);					// タイプ別設定一覧
	void Command_ChangeType(int nTypePlusOne);		// タイプ別設定一時適用
	void Command_Option_Type(void);					// タイプ別設定
	void Command_Option(void);						// 共通設定
	void Command_Font(void);						// フォント設定
	void Command_SetFontSize(int, int, int);		// フォントサイズ設定
	void Command_WrapWindowWidth(void);				// 現在のウィンドウ幅で折り返し
	void Command_Favorite(void);					// 履歴の管理
	void Command_Set_QuoteString(const wchar_t*);	// 引用符の設定
	void Command_TextWrapMethod(TextWrappingMethod);// テキストの折り返し方法を変更する
	void Command_Select_Count_Mode(int nMode);		// 文字カウント方法

	// マクロ系
	void Command_RecKeyMacro(void);		// キーマクロの記録開始／終了
	void Command_SaveKeyMacro(void);	// キーマクロの保存
	void Command_LoadKeyMacro(void);	// キーマクロの読み込み
	void Command_ExecKeyMacro(void);	// キーマクロの実行
	void Command_ExecExtMacro(const wchar_t* path, const wchar_t* type);	// 名前を指定してマクロ実行
	void Command_ExecCommand_Dialog(void);		// 外部コマンド実行ダイアログ表示
	void Command_ExecCommand(LPCWSTR cmd, const int nFlgOpt, LPCWSTR);	// 外部コマンド実行

	// カスタムメニュー
	void Command_Menu_RButton(void);	// 右クリックメニュー
	int Command_CustMenu(int);			// カスタムメニュー表示

	// ウィンドウ系
	void Command_Split_V(void);			// 上下に分割
	void Command_Split_H(void);			// 左右に分割
	void Command_Split_VH(void);		// 縦横に分割
	void Command_WinClose(void);		// ウィンドウを閉じる
	void Command_FileCloseAll(void);	// すべてのウィンドウを閉じる
	void Command_Bind_Window(void);		// 結合して表示
	void Command_Cascade(void);			// 重ねて表示
	void Command_Tile_V(void);			// 上下に並べて表示
	void Command_Tile_H(void);			// 左右に並べて表示
	void Command_Maximize_V(void);		// 縦方向に最大化
	void Command_Maximize_H(void);		// 横方向に最大化
	void Command_Minimize_All(void);	// すべて最小化
	void Command_Redraw(void);			// 再描画
	void Command_Win_Output(void);		// アウトプットウィンドウ表示
	void Command_TraceOut(const wchar_t* outputstr , int, int);	// マクロ用アウトプットウィンドウに表示
	void Command_WinTopMost(LPARAM);		// 常に手前に表示
	void Command_WinList(int nCommandFrom);	// ウィンドウ一覧ポップアップ表示処理
	void Command_GroupClose(void);		// グループを閉じる
	void Command_NextGroup(void);		// 次のグループ
	void Command_PrevGroup(void);		// 前のグループ
	void Command_Tab_MoveRight(void);	// タブを右に移動	
	void Command_Tab_MoveLeft(void);	// タブを左に移動	
	void Command_Tab_Separate(void);	// 新規グループ
	void Command_Tab_JointNext(void);	// 次のグループに移動
	void Command_Tab_JointPrev(void);	// 前のグループに移動
	void Command_Tab_CloseOther(void);	// このタブ以外を閉じる
	void Command_Tab_CloseLeft(void);	// 左をすべて閉じる
	void Command_Tab_CloseRight(void);	// 右をすべて閉じる


	void Command_ToggleKeySearch(int);	// キャレット位置の単語を辞書検索する機能ON-OFF

	void Command_Hokan(void);			// 入力補完
	void Command_Help_Contents(void);	// ヘルプ目次
	void Command_Help_Search(void);		// ヘルプキーワード検索
	void Command_Menu_AllFunc(void);	// コマンド一覧
	void Command_ExtHelp1(void);		// 外部ヘルプ１
	void Command_ExtHTMLHelp(const wchar_t* helpfile = NULL, const wchar_t* kwd = NULL);	// 外部HTMLヘルプ
	void Command_About(void);			// バージョン情報

	// その他

private:
	void AlertNotFound(HWND hwnd, bool, LPCTSTR format, ...);
	void DelCharForOverwrite(const wchar_t* pszInput, size_t nLen);	// 上書き用の一文字削除
	bool Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, size_t count ); // タグジャンプの前処理
public:
	size_t ConvertEol(const wchar_t* pszText, size_t nTextLen, wchar_t* pszConvertedText);
	void Sub_BoxSelectLock(int flags);

};

