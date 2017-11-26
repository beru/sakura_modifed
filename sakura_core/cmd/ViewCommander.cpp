#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "macro/SMacroMgr.h"
#include "EditApp.h"
#include "plugin/JackManager.h"

// EditViewクラスのコマンド処理系関数群

ViewCommander::ViewCommander(EditView& editView)
	:
	view(editView)
{
	bPrevCommand = 0;
	pSMacroMgr = EditApp::getInstance().pSMacroMgr;
}


/*!
	コマンドコードによる処理振り分け

	@param nCommand コマンドコード
	@param lparam1 parameter1(内容はコマンドコードによって変わります)
	@param lparam2 parameter2(内容はコマンドコードによって変わります)
	@param lparam3 parameter3(内容はコマンドコードによって変わります)
	@param lparam4 parameter4(内容はコマンドコードによって変わります)
*/
bool ViewCommander::HandleCommand(
	EFunctionCode	nCommand,
	bool			bRedraw,
	LPARAM			lparam1,
	LPARAM			lparam2,
	LPARAM			lparam3,
	LPARAM			lparam4
	)
{
	bool bRet = true;
	bool bRepeat = false;
	int nFuncID;

	// 上位16bitに送信元の識別子が入るように変更したので下位16ビットのみを取り出す
	// 定数と比較するためにシフトしないで使う
	int nCommandFrom = nCommand & ~0xffff;
	nCommand = (EFunctionCode)LOWORD(nCommand);


	if (view.nAutoScrollMode && nCommand != F_AUTOSCROLL) {
		view.AutoScrollExit();
	}
	view.GetCaret().bClearStatus = true;
	view.TranslateCommand_grep(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4);
	view.TranslateCommand_isearch(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4);

	// 機能が利用可能か調べる
	if (!IsFuncEnable(GetDocument(), GetDllShareData(), nCommand)) {
		return true;
	}

	++GetDocument().nCommandExecNum;		// コマンド実行回数
//	if (nCommand != F_COPY) {
		// 辞書Tipを消す
		view.tipWnd.Hide();
		view.dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
//	}
	// 印刷Previewモードか
	if (GetEditWindow().pPrintPreview && nCommand != F_PRINT_PREVIEW) {
		ErrorBeep();
		return true;
	}
	// キーリピート状態
	if (bPrevCommand == nCommand) {
		bRepeat = true;
	}
	bPrevCommand = nCommand;
	if (GetDllShareData().flags.bRecordingKeyMacro &&									// キーボードマクロの記録中
		GetDllShareData().flags.hwndRecordingKeyMacro == GetMainWindow() &&	// キーボードマクロを記録中のウィンドウ
		(nCommandFrom & FA_NONRECORD) != FA_NONRECORD	// 記録抑制フラグ off
	) {
		// キーリピート状態をなくする
		bRepeat = false;
		// キーマクロに記録可能な機能かどうかを調べる
		// F_EXECEXTMACROコマンドはファイルを選択した後にマクロ文が確定するため個別に記録する。
		if (SMacroMgr::CanFuncIsKeyMacro(nCommand) &&
			nCommand != F_EXECEXTMACRO	// F_EXECEXTMACROは個別で記録します
		) {
			// キーマクロのバッファにデータ追加
			LPARAM lparams[] = {lparam1, lparam2, lparam3, lparam4};
			pSMacroMgr->Append(STAND_KEYMACRO, nCommand, lparams, view);
		}
	}

	// マクロ実行中フラグの設定
	// マクロからのコマンドかどうかはnCommandFromでわかるが
	// nCommandFromを引数で浸透させるのが大変なので，従来のフラグにも値をコピーする
	view.bExecutingKeyMacro = (nCommandFrom & FA_FROMMACRO) ? true : false;

	// キーボードマクロの実行中
	if (view.bExecutingKeyMacro) {
		// キーリピート状態をなくする
		bRepeat = false;
	}

	// マクロの実行機能追加
	if (F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO) {
		if (!pSMacroMgr->Exec(nCommand - F_USERMACRO_0, G_AppInstance(), view,
			nCommandFrom & FA_NONRECORD)
		) {
			InfoMessage(
				view.hwndParent,
				LS(STR_ERR_MACRO1),
				nCommand - F_USERMACRO_0,
				pSMacroMgr->GetFile(nCommand - F_USERMACRO_0)
			);
		}
		return true;
	}

	view.PreprocessCommand_hokan(nCommand);
	if (view.ProcessCommand_isearch(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4))
		return true;

	// -------------------------------------
	// ここより前ではUndoバッファの準備ができていないので
	// 文書の操作を行ってはいけない
	if (!GetOpeBlk()) {	// 操作ブロック
		SetOpeBlk(new OpeBlk);
	}
	GetOpeBlk()->AddRef();	// 参照カウンタ増加

	// ここより後ではswitchの後ろでUndoを正しく登録するため，
	// 途中で処理の打ち切りを行ってはいけない
	// -------------------------------------

	switch (nCommand) {
	case F_WCHAR:	// 文字入力
		{
			Command_WCHAR((wchar_t)lparam1);
		}
		break;

	// ファイル操作系
	case F_FILENEW:				Command_FileNew(); break;			// 新規作成
	case F_FILENEW_NEWWINDOW:	Command_FileNew_NewWindow(); break;
	case F_FILEOPEN:			Command_FileOpen((const wchar_t*)lparam1); break;			// ファイルを開く
	case F_FILEOPEN2:			Command_FileOpen((const wchar_t*)lparam1, (EncodingType)lparam2, lparam3 != 0, (const wchar_t*)lparam4); break;	// ファイルを開く2
	case F_FILEOPEN_DROPDOWN:	Command_FileOpen((const wchar_t*)lparam1); break;			// ファイルを開く(ドロップダウン)
	case F_FILESAVE:			bRet = Command_FileSave(); break;	// 上書き保存
	case F_FILESAVEAS_DIALOG:	bRet = Command_FileSaveAs_Dialog((const wchar_t*)lparam1, (EncodingType)lparam2, (EolType)lparam3); break;	// 名前を付けて保存
	case F_FILESAVEAS:			bRet = Command_FileSaveAs((const wchar_t*)lparam1, (EolType)lparam3); break;	// 名前を付けて保存
	case F_FILESAVEALL:			bRet = Command_FileSaveAll(); break;	// 全ての編集ウィンドウで上書き保存
	case F_FILESAVE_QUIET:		bRet = Command_FileSave(false, false); break;	// 静かに上書き保存
	case F_FILESAVECLOSE:
		// 保存＆閉じる
		// 保存が不要なら単に閉じる
		{	// Command_FileSave()とは別に保存不要をチェック
			if (!GetDllShareData().common.file.bEnableUnmodifiedOverwrite && !GetDocument().docEditor.IsModified()) {
				Command_WinClose();
				break;
			}
		}
		if (Command_FileSave(false, true)) {
			Command_WinClose();
		}
		break;
	case F_FILECLOSE:										// 閉じて(無題)
		Command_FileClose();
		break;
	case F_FILECLOSE_OPEN:	// 閉じて開く
		Command_FileClose_Open();
		break;
	case F_FILE_REOPEN:				Command_File_Reopen(GetDocument().GetDocumentEncoding(), lparam1 != 0); break;
	case F_FILE_REOPEN_SJIS:		Command_File_Reopen(CODE_SJIS, lparam1 != 0); break;		// SJISで開き直す
	case F_FILE_REOPEN_JIS:			Command_File_Reopen(CODE_JIS, lparam1 != 0); break;			// JISで開き直す
	case F_FILE_REOPEN_EUC:			Command_File_Reopen(CODE_EUC, lparam1 != 0); break;			// EUCで開き直す
	case F_FILE_REOPEN_LATIN1:		Command_File_Reopen(CODE_LATIN1, lparam1 != 0); break;		// Latin1で開きなおす
	case F_FILE_REOPEN_UNICODE:		Command_File_Reopen(CODE_UNICODE, lparam1 != 0); break;		// Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE: 	Command_File_Reopen(CODE_UNICODEBE, lparam1 != 0); break;	// UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8:		Command_File_Reopen(CODE_UTF8, lparam1 != 0); break;		// UTF-8で開き直す
	case F_FILE_REOPEN_CESU8:		Command_File_Reopen(CODE_CESU8, lparam1 != 0); break;		// CESU-8で開きなおす
	case F_FILE_REOPEN_UTF7:		Command_File_Reopen(CODE_UTF7, lparam1 != 0); break;		// UTF-7で開き直す
	case F_PRINT:					Command_Print(); break;					// 印刷
	case F_PRINT_PREVIEW:			Command_Print_Preview(); break;			// 印刷Preview
	case F_PRINT_PAGESETUP:			Command_Print_PageSetUp(); break;		// 印刷ページ設定
	case F_OPEN_HfromtoC:			bRet = Command_Open_HfromtoC(lparam1 != 0); break;			// 同名のC/C++ヘッダ(ソース)を開く
//	case F_OPEN_HHPP:				bRet = Command_Open_HHPP((bool)lparam1, true); break;		// 同名のC/C++ヘッダファイルを開く
//	case F_OPEN_CCPP:				bRet = Command_Open_CCPP((bool)lparam1, true); break;		// 同名のC/C++ソースファイルを開く
	case F_ACTIVATE_SQLPLUS:		Command_Activate_SQLPlus(); break;		// Oracle SQL*Plusをアクティブ表示
	case F_PLSQL_COMPILE_ON_SQLPLUS:									// Oracle SQL*Plusで実行
		Command_PLSQL_Compile_On_SQLPlus();
		break;
	case F_BROWSE:				Command_Browse(); break;			// ブラウズ
	case F_VIEWMODE:			Command_ViewMode(); break;			// ビューモード
	case F_PROPERTY_FILE:		Command_Property_File(); break;		// ファイルのプロパティ
	case F_PROFILEMGR:			Command_ProfileMgr(); break;		// プロファイルマネージャ
	case F_EXITALLEDITORS:		Command_ExitAllEditors(); break;	// 編集の全終了
	case F_EXITALL:				Command_ExitAll(); break;			// サクラエディタの全終了
	case F_PUTFILE:				Command_PutFile((LPCWSTR)lparam1, (EncodingType)lparam2, (int)lparam3); break;	// 作業中ファイルの一時出力
	case F_INSFILE:				Command_InsFile((LPCWSTR)lparam1, (EncodingType)lparam2, (int)lparam3); break;	// キャレット位置にファイル挿入

	// 編集系
	case F_UNDO:				Command_Undo(); break;				// 元に戻す(Undo)
	case F_REDO:				Command_Redo(); break;				// やり直し(Redo)
	case F_DELETE:				Command_Delete(); break;			// 削除
	case F_DELETE_BACK:			Command_Delete_Back(); break;		// カーソル前を削除
	case F_WordDeleteToStart:	Command_WordDeleteToStart(); break;	// 単語の左端まで削除
	case F_WordDeleteToEnd:		Command_WordDeleteToEnd(); break;	// 単語の右端まで削除
	case F_WordDelete:			Command_WordDelete(); break;		// 単語削除
	case F_WordCut:				Command_WordCut(); break;			// 単語切り取り
	case F_LineCutToStart:		Command_LineCutToStart(); break;	// 行頭まで切り取り(改行単位)
	case F_LineCutToEnd:		Command_LineCutToEnd(); break;		// 行末まで切り取り(改行単位)
	case F_LineDeleteToStart:	Command_LineDeleteToStart(); break;	// 行頭まで削除(改行単位)
	case F_LineDeleteToEnd:		Command_LineDeleteToEnd(); break;	// 行末まで削除(改行単位)
	case F_CUT_LINE:			Command_Cut_Line(); break;			// 行切り取り(折り返し単位)
	case F_DELETE_LINE:			Command_Delete_Line(); break;		// 行削除(折り返し単位)
	case F_DUPLICATELINE:		Command_DuplicateLine(); break;		// 行の二重化(折り返し単位)
	case F_INDENT_TAB:			Command_Indent(WCODE::TAB, IndentType::Tab); break;		// TABインデント
	case F_UNINDENT_TAB:		Command_Unindent(WCODE::TAB); break;				// 逆TABインデント
	case F_INDENT_SPACE:		Command_Indent(WCODE::SPACE, IndentType::Space); break;	// SPACEインデント
	case F_UNINDENT_SPACE:		Command_Unindent(WCODE::SPACE); break;			// 逆SPACEインデント
//	case F_WORDSREFERENCE:		Command_WORDSREFERENCE(); break;		// 単語リファレンス
	case F_LTRIM:				Command_Trim(true); break;
	case F_RTRIM:				Command_Trim(false); break;
	case F_SORT_ASC:			Command_Sort(true); break;
	case F_SORT_DESC:			Command_Sort(false); break;
	case F_MERGE:				Command_Merge(); break;
	case F_RECONVERT:			Command_Reconvert(); break;			// メニューからの再変換対応

	// カーソル移動系
	case F_IME_CHAR:			Command_IME_CHAR((WORD)lparam1); break;					// 全角文字入力
	case F_MOVECURSOR:			Command_MoveCursor(Point((int)lparam2, (int)lparam1), (int)lparam3); break;
	case F_MOVECURSORLAYOUT:	Command_MoveCursorLayout(Point((int)lparam2, (int)lparam1), (int)lparam3); break;
	case F_UP:					Command_Up(view.GetSelectionInfo().bSelectingLock, bRepeat); break;				// カーソル上移動
	case F_DOWN:				Command_Down(view.GetSelectionInfo().bSelectingLock, bRepeat); break;			// カーソル下移動
	case F_LEFT:				Command_Left(view.GetSelectionInfo().bSelectingLock, bRepeat); break;			// カーソル左移動
	case F_RIGHT:				Command_Right(view.GetSelectionInfo().bSelectingLock, false, bRepeat); break;	// カーソル右移動
	case F_UP2:					Command_Up2(view.GetSelectionInfo().bSelectingLock); break;						// カーソル上移動(２行づつ)
	case F_DOWN2:				Command_Down2(view.GetSelectionInfo().bSelectingLock); break;					// カーソル下移動(２行づつ)
	case F_WORDLEFT:			Command_WordLeft(view.GetSelectionInfo().bSelectingLock); break;					// 単語の左端に移動
	case F_WORDRIGHT:			Command_WordRight(view.GetSelectionInfo().bSelectingLock); break;				// 単語の右端に移動
	// マクロ向け機能拡張
	case F_GOLINETOP:			Command_GoLineTop(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;		// 行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND:			Command_GoLineEnd(view.GetSelectionInfo().bSelectingLock, 0, (int)lparam1); break;	// 行末に移動(折り返し単位)
	case F_HalfPageUp:			Command_HalfPageUp(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;				// 半ページアップ	
	case F_HalfPageDown:		Command_HalfPageDown(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;			// 半ページダウン	
	case F_1PageUp:				Command_1PageUp(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;					// １ページアップ	
	case F_1PageDown:			Command_1PageDown(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;				// １ページダウン	
	case F_GOFILETOP:			Command_GoFileTop(view.GetSelectionInfo().bSelectingLock); break;				// ファイルの先頭に移動
	case F_GOFILEEND:			Command_GoFileEnd(view.GetSelectionInfo().bSelectingLock); break;				// ファイルの最後に移動
	case F_CURLINECENTER:		Command_CurLineCenter(); break;								// カーソル行をウィンドウ中央へ
	case F_JUMPHIST_PREV:		Command_JumpHist_Prev(); break;								// 移動履歴: 前へ
	case F_JUMPHIST_NEXT:		Command_JumpHist_Next(); break;								// 移動履歴: 次へ
	case F_JUMPHIST_SET:		Command_JumpHist_Set(); break;								// 現在位置を移動履歴に登録
	case F_WndScrollDown:		Command_WndScrollDown(); break;								// テキストを１行下へScroll
	case F_WndScrollUp:			Command_WndScrollUp(); break;								// テキストを１行上へScroll
	case F_GONEXTPARAGRAPH:		Command_GoNextParagraph(view.GetSelectionInfo().bSelectingLock); break;			// 次の段落へ進む
	case F_GOPREVPARAGRAPH:		Command_GoPrevParagraph(view.GetSelectionInfo().bSelectingLock); break;			// 前の段落へ戻る
	case F_AUTOSCROLL:			Command_AutoScroll(); break;									// Auto Scroll
	case F_WHEELUP:				Command_WheelUp((int)lparam1); break;
	case F_WHEELDOWN:			Command_WheelDown((int)lparam1); break;
	case F_WHEELLEFT:			Command_WheelLeft((int)lparam1); break;
	case F_WHEELRIGHT:			Command_WheelRight((int)lparam1); break;
	case F_WHEELPAGEUP:			Command_WheelPageUp((int)lparam1); break;
	case F_WHEELPAGEDOWN:		Command_WheelPageDown((int)lparam1); break;
	case F_WHEELPAGELEFT:		Command_WheelPageLeft((int)lparam1); break;
	case F_WHEELPAGERIGHT:		Command_WheelPageRight((int)lparam1); break;
	case F_MODIFYLINE_NEXT:		Command_ModifyLine_Next( view.GetSelectionInfo().bSelectingLock ); break;	// 次の変更行へ
	case F_MODIFYLINE_PREV:		Command_ModifyLine_Prev( view.GetSelectionInfo().bSelectingLock ); break;	// 前の変更行へ

	// 選択系
	case F_SELECTWORD:		Command_SelectWord(); break;					// 現在位置の単語選択
	case F_SELECTALL:		Command_SelectAll(); break;						// すべて選択
	case F_SELECTLINE:		Command_SelectLine((int)lparam1); break;				// 1行選択
	case F_BEGIN_SEL:		Command_Begin_Select(); break;					// 範囲選択開始
	case F_UP_SEL:			Command_Up(true, bRepeat, (int)lparam1); break;		// (範囲選択)カーソル上移動
	case F_DOWN_SEL:		Command_Down(true, bRepeat); break;				// (範囲選択)カーソル下移動
	case F_LEFT_SEL:		Command_Left(true, bRepeat); break;				// (範囲選択)カーソル左移動
	case F_RIGHT_SEL:		Command_Right(true, false, bRepeat); break;		// (範囲選択)カーソル右移動
	case F_UP2_SEL:			Command_Up2(true); break;						// (範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL:		Command_Down2(true); break;						// (範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL:	Command_WordLeft(true); break;					// (範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL:	Command_WordRight(true); break;					// (範囲選択)単語の右端に移動
	case F_GOLINETOP_SEL:	Command_GoLineTop(true, (int)lparam1); break;		// (範囲選択)行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND_SEL:	Command_GoLineEnd(true, 0, (int)lparam1); break;		// (範囲選択)行末に移動(折り返し単位)
	case F_HalfPageUp_Sel:	Command_HalfPageUp(true, (int)lparam1); break;				//(範囲選択)半ページアップ
	case F_HalfPageDown_Sel:Command_HalfPageDown(true, (int)lparam1); break;			//(範囲選択)半ページダウン
	case F_1PageUp_Sel:		Command_1PageUp(true, (int)lparam1); break;					//(範囲選択)１ページアップ
	case F_1PageDown_Sel:	Command_1PageDown(true, (int)lparam1); break;				//(範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	Command_GoFileTop(true); break;					// (範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	Command_GoFileEnd(true); break;					// (範囲選択)ファイルの最後に移動
	case F_GONEXTPARAGRAPH_SEL:	Command_GoNextParagraph(true); break;		// 次の段落へ進む
	case F_GOPREVPARAGRAPH_SEL:	Command_GoPrevParagraph(true); break;		// 前の段落へ戻る
	case F_MODIFYLINE_NEXT_SEL:	Command_ModifyLine_Next( true ); break;			//(範囲選択)次の変更行へ
	case F_MODIFYLINE_PREV_SEL:	Command_ModifyLine_Prev( true ); break;			//(範囲選択)前の変更行へ

	// 矩形選択系
	case F_BEGIN_BOX:		Command_Begin_BoxSelect(true); break;	// 矩形範囲選択開始
	case F_UP_BOX:			Sub_BoxSelectLock((int)lparam1); this->Command_Up(true, bRepeat); break;		// (矩形選択)カーソル上移動
	case F_DOWN_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Down(true, bRepeat); break;	// (矩形選択)カーソル下移動
	case F_LEFT_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Left(true, bRepeat); break;	// (矩形選択)カーソル左移動
	case F_RIGHT_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Right(true, false, bRepeat); break;	// (矩形選択)カーソル右移動
	case F_UP2_BOX:			Sub_BoxSelectLock((int)lparam1); this->Command_Up2(true); break;				// (矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Down2(true);break;			// (矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_WordLeft(true);break;			// (矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_WordRight(true);break;		// (矩形選択)単語の右端に移動
	case F_GOLOGICALLINETOP_BOX:Sub_BoxSelectLock((int)lparam2); this->Command_GoLineTop(true, 8 | (int)lparam1);break;	// (矩形選択)行頭に移動(改行単位)
	case F_GOLINETOP_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_GoLineTop(true, (int)lparam1);break;		// (矩形選択)行頭に移動(折り返し単位/改行単位)
	case F_GOLINEEND_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_GoLineEnd(true, 0, (int)lparam1);break;	// (矩形選択)行末に移動(折り返し単位/改行単位)
	case F_HalfPageUp_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_HalfPageUp(true, (int)lparam1); break;		// (矩形選択)半ページアップ
	case F_HalfPageDown_BOX:Sub_BoxSelectLock((int)lparam2); this->Command_HalfPageDown(true, (int)lparam1); break;		// (矩形選択)半ページダウン
	case F_1PageUp_BOX:		Sub_BoxSelectLock((int)lparam2); this->Command_1PageUp(true, (int)lparam1); break;			// (矩形選択)１ページアップ
	case F_1PageDown_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_1PageDown(true, (int)lparam1); break;			// (矩形選択)１ページダウン
	case F_GOFILETOP_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_GoFileTop(true);break;			// (矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_GoFileEnd(true);break;			// (矩形選択)ファイルの最後に移動

	// クリップボード系
	case F_CUT:						Command_Cut(); break;					// 切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:					Command_Copy(false, GetDllShareData().common.edit.bAddCRLFWhenCopy); break;			// コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:			Command_Copy(false, true); break;		// 折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:				Command_Copy(false, GetDllShareData().common.edit.bAddCRLFWhenCopy, EolType::CRLF); break;	// CRLF改行でコピー(選択範囲をクリップボードにコピー)
	case F_PASTE:					Command_Paste((int)lparam1); break;					// 貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:				Command_PasteBox((int)lparam1); break;				// 矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSBOXTEXT:				Command_InsBoxText((const wchar_t*)lparam1, (int)lparam2); break;				// 矩形テキスト挿入
	case F_INSTEXT_W:				Command_InsText(bRedraw, (const wchar_t*)lparam1, (size_t)lparam2, lparam3 != FALSE); break; // テキストを貼り付け
	case F_ADDTAIL_W:				Command_AddTail((const wchar_t*)lparam1, (int)lparam2); break;	// 最後にテキストを追加
	case F_COPYFNAME:				Command_CopyFileName(); break;						// このファイル名をクリップボードにコピー
	case F_COPYPATH:				Command_CopyPath(); break;							// このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:					Command_CopyTag(); break;							// このファイルのパス名とカーソル位置をコピー
	case F_COPYLINES:				Command_CopyLines(); break;							// 選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		Command_CopyLinesAsPassage(); break;				// 選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	Command_CopyLinesWithLineNumber(); break;			// 選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:				Command_Copy_Color_HTML(); break;				// 選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	Command_Copy_Color_HTML_LineNumber(); break;	// 選択範囲内行番号色付きHTMLコピー

	case F_CREATEKEYBINDLIST:		Command_CreateKeyBindList(); break;		// キー割り当て一覧をコピー

	// 挿入系
	case F_INS_DATE:				Command_Ins_Date(); break;				// 日付挿入
	case F_INS_TIME:				Command_Ins_Time(); break;				// 時刻挿入
    case F_CTRL_CODE_DIALOG:		Command_CtrlCode_Dialog(); break;		// コントロールコードの入力(ダイアログ)
    case F_CTRL_CODE:				Command_WCHAR((wchar_t)lparam1, false); break;

	// 変換
	case F_TOLOWER:					Command_ToLower(); break;				// 小文字
	case F_TOUPPER:					Command_ToUpper(); break;				// 大文字
	case F_TOHANKAKU:				Command_ToHankaku(); break;				// 全角→半角
	case F_TOHANKATA:				Command_ToHankata(); break;				// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					Command_ToZenEi(); break;				// 全角→半角
	case F_TOHANEI:					Command_ToHanEi(); break;				// 半角→全角
	case F_TOZENKAKUKATA:			Command_ToZenkakuKata(); break;			// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			Command_ToZenkakuHira(); break;			// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		Command_HanKataToZenkakuKata(); break;	// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		Command_HanKataToZenKakuHira(); break;	// 半角カタカナ→全角ひらがな
	case F_TABTOSPACE:				Command_TabToSpace(); break;			// TAB→空白
	case F_SPACETOTAB:				Command_SpaceToTab(); break;			// 空白→TAB
	case F_CODECNV_AUTO2SJIS:		Command_CodeCnv_Auto2SJIS(); break;		// 自動判別→SJISコード変換
	case F_CODECNV_EMAIL:			Command_CodeCnv_EMail(); break;			// E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS:		Command_CodeCnv_EUC2SJIS(); break;		// EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS:	Command_CodeCnv_Unicode2SJIS(); break;	// Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS:	Command_CodeCnv_UnicodeBE2SJIS(); break;	// UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS:		Command_CodeCnv_UTF82SJIS(); break;		// UTF-8→SJISコード変換
	case F_CODECNV_UTF72SJIS:		Command_CodeCnv_UTF72SJIS(); break;		// UTF-7→SJISコード変換
	case F_CODECNV_SJIS2JIS:		Command_CodeCnv_SJIS2JIS(); break;		// SJIS→JISコード変換
	case F_CODECNV_SJIS2EUC:		Command_CodeCnv_SJIS2EUC(); break;		// SJIS→EUCコード変換
	case F_CODECNV_SJIS2UTF8:		Command_CodeCnv_SJIS2UTF8(); break;		// SJIS→UTF-8コード変換
	case F_CODECNV_SJIS2UTF7:		Command_CodeCnv_SJIS2UTF7(); break;		// SJIS→UTF-7コード変換
	case F_BASE64DECODE:			Command_Base64Decode(); break;			// Base64デコードして保存
	case F_UUDECODE:				Command_UUDecode(); break;				// uudecodeして保存

	// 検索系
	case F_SEARCH_DIALOG:		Command_Search_Dialog(); break;				// 検索(単語検索ダイアログ)
	case F_SEARCH_BOX:			Command_Search_Box(); break;		// 検索(ボックス)
	case F_SEARCH_NEXT:			Command_Search_Next(true, bRedraw, false, (HWND)lparam1, (const wchar_t*)lparam2); break;	// 次を検索
	case F_SEARCH_PREV:			Command_Search_Prev(bRedraw, (HWND)lparam1); break;						// 前を検索
	case F_REPLACE_DIALOG:	// 置換(置換ダイアログ)
		Command_Replace_Dialog();	// ダイアログ呼び出しと、実行を分離
		break;
	case F_REPLACE:				Command_Replace((HWND)lparam1); break;			// 置換実行
	case F_REPLACE_ALL:			Command_Replace_All(); break;		// すべて置換実行(通常)
	case F_SEARCH_CLEARMARK:	Command_Search_ClearMark(); break;	// 検索マークのクリア
	case F_GREP_DIALOG:	// Grepダイアログの表示
		// 再帰処理対策
		view.SetUndoBuffer(true);
		Command_Grep_Dialog();
		return bRet;
	case F_GREP:			Command_Grep(); break;							// Grep
	case F_GREP_REPLACE_DLG:	// Grep置換ダイアログの表示
		// 再帰処理対策
		view.SetUndoBuffer( true );
		Command_Grep_Replace_Dlg();
		return bRet;
	case F_GREP_REPLACE:	Command_Grep_Replace();break;							//Grep置換
	case F_JUMP_DIALOG:		Command_Jump_Dialog(); break;					// 指定行ヘジャンプダイアログの表示
	case F_JUMP:			Command_Jump(); break;							// 指定行ヘジャンプ
	case F_OUTLINE:			bRet = Command_FuncList((ShowDialogType)lparam1, OutlineType::Default); break;	// アウトライン解析
	case F_OUTLINE_TOGGLE:	bRet = Command_FuncList(ShowDialogType::Toggle, OutlineType::Default); break;	// アウトライン解析(toggle)
	case F_FILETREE:		bRet = Command_FuncList((ShowDialogType)lparam1, OutlineType::FileTree); break;	//ファイルツリー
	case F_TAGJUMP:			Command_TagJump(lparam1 != 0); break;			// タグジャンプ機能
	case F_TAGJUMP_CLOSE:	Command_TagJump(true); break;					// タグジャンプ(元ウィンドウClose)
	case F_TAGJUMPBACK:		Command_TagJumpBack(); break;					// タグジャンプバック機能
	case F_TAGS_MAKE:		Command_TagsMake(); break;						// タグファイルの作成
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFileMsg(true); break;		// ダイレクトタグジャンプ機能
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword((const wchar_t*)lparam1); break;	// キーワードを指定してダイレクトタグジャンプ機能
	case F_COMPARE:			Command_Compare(); break;						// ファイル内容比較
	case F_DIFF_DIALOG:		Command_Diff_Dialog(); break;					// DIFF差分表示(ダイアログ)
	case F_DIFF:			Command_Diff((const wchar_t*)lparam1, (int)lparam2); break;		// DIFF差分表示
	case F_DIFF_NEXT:		Command_Diff_Next(); break;						// DIFF差分表示(次へ)
	case F_DIFF_PREV:		Command_Diff_Prev(); break;						// DIFF差分表示(前へ)
	case F_DIFF_RESET:		Command_Diff_Reset(); break;					// DIFF差分表示(全解除)
	case F_BRACKETPAIR:		Command_BracketPair(); break;					// 対括弧の検索
	case F_BOOKMARK_SET:	Command_Bookmark_Set(); break;					// ブックマーク設定・解除
	case F_BOOKMARK_NEXT:	Command_Bookmark_Next(); break;					// 次のブックマークへ
	case F_BOOKMARK_PREV:	Command_Bookmark_Prev(); break;					// 前のブックマークへ
	case F_BOOKMARK_RESET:	Command_Bookmark_Reset(); break;				// ブックマークの全解除
	case F_BOOKMARK_VIEW:	bRet = Command_FuncList((ShowDialogType)lparam1 , OutlineType::BookMark); break;	// アウトライン解析
	case F_BOOKMARK_PATTERN:Command_Bookmark_Pattern(); break;				// 指定パターンに一致する行をマーク
	case F_JUMP_SRCHSTARTPOS:	Command_Jump_SrchStartPos(); break;			// 検索開始位置へ戻る
	case F_FUNCLIST_NEXT:	Command_FuncList_Next();break;					// 次の関数リストマーク
	case F_FUNCLIST_PREV:	Command_FuncList_Prev();break;					// 前の関数リストマーク

	// モード切り替え系
	case F_CHGMOD_INS:		Command_ChgMod_Ins(); break;		// 挿入／上書きモード切り替え
	case F_CHG_CHARSET:		Command_Chg_Charset((EncodingType)lparam1, lparam2 != 0); break;	// 文字コードセット指定
	// F_CHGMOD_EOL_xxx はマクロに記録されないが、F_CHGMOD_EOLはマクロに記録されるので、マクロ関数を統合できるという手はず
	case F_CHGMOD_EOL_CRLF:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::CRLF, 0, 0, 0); break;	// 入力する改行コードをCRLFに設定
	case F_CHGMOD_EOL_LF:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::LF, 0, 0, 0); break;	// 入力する改行コードをLFに設定
	case F_CHGMOD_EOL_CR:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::CR, 0, 0, 0); break;	// 入力する改行コードをCRに設定
	case F_CHGMOD_EOL:		Command_ChgMod_EOL((EolType)lparam1); break;	// 入力する改行コードを設定
	case F_CANCEL_MODE:		Command_Cancel_Mode(); break;	// 各種モードの取り消し

	// 設定系
	case F_SHOWTOOLBAR:		Command_ShowToolBar(); break;	// ツールバーの表示/非表示
	case F_SHOWFUNCKEY:		Command_ShowFuncKey(); break;	// ファンクションキーの表示/非表示
	case F_SHOWTAB:			Command_ShowTab(); break;		// タブの表示/非表示
	case F_SHOWSTATUSBAR:	Command_ShowStatusBar(); break;	// ステータスバーの表示/非表示
	case F_SHOWMINIMAP:		Command_ShowMiniMap(); break;	// ミニマップの表示/非表示
	case F_TYPE_LIST:		Command_Type_List(); break;		// タイプ別設定一覧
	case F_CHANGETYPE:		Command_ChangeType((int)lparam1); break;		// タイプ別設定一時適用
	case F_OPTION_TYPE:		Command_Option_Type(); break;	// タイプ別設定
	case F_OPTION:			Command_Option(); break;		// 共通設定
	case F_FONT:			Command_Font(); break;			// フォント設定
	case F_SETFONTSIZE:		Command_SetFontSize((int)lparam1, (int)lparam2, (int)lparam3); break;	// フォントサイズ設定
	case F_SETFONTSIZEUP:	HandleCommand(F_SETFONTSIZE, bRedraw, 0, 1, 2, 0); break;	// フォントサイズ拡大
	case F_SETFONTSIZEDOWN:	HandleCommand(F_SETFONTSIZE, bRedraw, 0, -1, 2, 0); break;	// フォントサイズ縮小
	case F_WRAPWINDOWWIDTH:	Command_WrapWindowWidth(); break;// 現在のウィンドウ幅で折り返し
	case F_FAVORITE:		Command_Favorite(); break;		// 履歴の管理
	// 引用符の設定
	case F_SET_QUOTESTRING:	Command_Set_QuoteString((const wchar_t*)lparam1);	break;
	case F_TMPWRAPNOWRAP:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::NoWrapping, 0, 0, 0); break;	// 折り返さない（一時設定）
	case F_TMPWRAPSETTING:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::SettingWidth, 0, 0, 0); break;	// 指定桁で折り返す（一時設定）
	case F_TMPWRAPWINDOW:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::WindowWidth, 0, 0, 0); break;	// 右端で折り返す（一時設定）
	case F_TEXTWRAPMETHOD:	Command_TextWrapMethod((TextWrappingMethod)lparam1); break;			// テキストの折り返し方法
	case F_SELECT_COUNT_MODE:	Command_Select_Count_Mode((int)lparam1); break;		// 文字カウントの方法

	// マクロ系
	case F_RECKEYMACRO:		Command_RecKeyMacro(); break;	// キーマクロの記録開始／終了
	case F_SAVEKEYMACRO:	Command_SaveKeyMacro(); break;	// キーマクロの保存
	case F_LOADKEYMACRO:	Command_LoadKeyMacro(); break;	// キーマクロの読み込み
	case F_EXECKEYMACRO:									// キーマクロの実行
		// 再帰処理対策
		view.SetUndoBuffer(true);
		Command_ExecKeyMacro(); return bRet;
	case F_EXECEXTMACRO:
		// 再帰処理対策
		view.SetUndoBuffer(true);
		// 名前を指定してマクロ実行
		Command_ExecExtMacro((const wchar_t*)lparam1, (const wchar_t*)lparam2);
		return bRet;
	//	case F_EXECCMMAND:		Command_ExecCmmand(); break;	// 外部コマンド実行
	case F_EXECMD_DIALOG:
		//Command_ExecCommand_Dialog((const char*)lparam1);	// 外部コマンド実行
		Command_ExecCommand_Dialog();	// 外部コマンド実行	// 引数つかってないみたいなので
		break;
	case F_EXECMD:
		//Command_ExecCommand((const char*)lparam1);
		Command_ExecCommand((LPCWSTR)lparam1, (int)lparam2, (LPCWSTR)lparam3);
		break;

	// カスタムメニュー
	case F_MENU_RBUTTON:	// 右クリックメニュー
		// 再帰処理対策
		view.SetUndoBuffer(true);
		Command_Menu_RButton();
		return bRet;
	case F_CUSTMENU_1:  // カスタムメニュー1
	case F_CUSTMENU_2:  // カスタムメニュー2
	case F_CUSTMENU_3:  // カスタムメニュー3
	case F_CUSTMENU_4:  // カスタムメニュー4
	case F_CUSTMENU_5:  // カスタムメニュー5
	case F_CUSTMENU_6:  // カスタムメニュー6
	case F_CUSTMENU_7:  // カスタムメニュー7
	case F_CUSTMENU_8:  // カスタムメニュー8
	case F_CUSTMENU_9:  // カスタムメニュー9
	case F_CUSTMENU_10: // カスタムメニュー10
	case F_CUSTMENU_11: // カスタムメニュー11
	case F_CUSTMENU_12: // カスタムメニュー12
	case F_CUSTMENU_13: // カスタムメニュー13
	case F_CUSTMENU_14: // カスタムメニュー14
	case F_CUSTMENU_15: // カスタムメニュー15
	case F_CUSTMENU_16: // カスタムメニュー16
	case F_CUSTMENU_17: // カスタムメニュー17
	case F_CUSTMENU_18: // カスタムメニュー18
	case F_CUSTMENU_19: // カスタムメニュー19
	case F_CUSTMENU_20: // カスタムメニュー20
	case F_CUSTMENU_21: // カスタムメニュー21
	case F_CUSTMENU_22: // カスタムメニュー22
	case F_CUSTMENU_23: // カスタムメニュー23
	case F_CUSTMENU_24: // カスタムメニュー24
		// 再帰処理対策
		view.SetUndoBuffer(true);
		nFuncID = Command_CustMenu(nCommand - F_CUSTMENU_1 + 1);
		if (nFuncID != 0) {
			// コマンドコードによる処理振り分け
//			HandleCommand(nFuncID, true, 0, 0, 0, 0);
			::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nFuncID, 0), (LPARAM)NULL);
		}
		return bRet;

	// ウィンドウ系
	case F_SPLIT_V:			Command_Split_V(); break;	// 上下に分割
	case F_SPLIT_H:			Command_Split_H(); break;	// 左右に分割
	case F_SPLIT_VH:		Command_Split_VH(); break;	// 縦横に分割
	case F_WINCLOSE:		Command_WinClose(); break;	// ウィンドウを閉じる
	case F_WIN_CLOSEALL:	// すべてのウィンドウを閉じる
		Command_FileCloseAll();
		break;
	case F_BIND_WINDOW:		Command_Bind_Window(); break;	// 結合して表示
	case F_CASCADE:			Command_Cascade(); break;		// 重ねて表示
	case F_TILE_V:			Command_Tile_V(); break;		// 上下に並べて表示
	case F_TILE_H:			Command_Tile_H(); break;		// 左右に並べて表示
	case F_MAXIMIZE_V:		Command_Maximize_V(); break;	// 縦方向に最大化
	case F_MAXIMIZE_H:		Command_Maximize_H(); break;	// 横方向に最大化
	case F_MINIMIZE_ALL:	Command_Minimize_All(); break;	// すべて最小化
	case F_REDRAW:			Command_Redraw(); break;		// 再描画
	case F_WIN_OUTPUT:		Command_Win_Output(); break;	// アウトプットウィンドウ表示
	case F_TRACEOUT:		Command_TraceOut((const wchar_t*)lparam1, (int)lparam2, (int)lparam3); break;		// マクロ用アウトプットウィンドウに表示
	case F_TOPMOST:			Command_WinTopMost(lparam1); break;		// 常に手前に表示
	case F_WINLIST:			Command_WinList(nCommandFrom); break;	// ウィンドウ一覧ポップアップ表示処理
	case F_GROUPCLOSE:		Command_GroupClose(); break;	// グループを閉じる
	case F_NEXTGROUP:		Command_NextGroup(); break;		// 次のグループ
	case F_PREVGROUP:		Command_PrevGroup(); break;		// 前のグループ
	case F_TAB_MOVERIGHT:	Command_Tab_MoveRight(); break;	// タブを右に移動
	case F_TAB_MOVELEFT:	Command_Tab_MoveLeft(); break;	// タブを左に移動
	case F_TAB_SEPARATE:	Command_Tab_Separate(); break;	// 新規グループ
	case F_TAB_JOINTNEXT:	Command_Tab_JointNext(); break;	// 次のグループに移動
	case F_TAB_JOINTPREV:	Command_Tab_JointPrev(); break;	// 前のグループに移動
	case F_TAB_CLOSEOTHER:	Command_Tab_CloseOther(); break;	// このタブ以外を閉じる
	case F_TAB_CLOSELEFT:	Command_Tab_CloseLeft(); break;		// 左をすべて閉じる
	case F_TAB_CLOSERIGHT:	Command_Tab_CloseRight(); break;	// 右をすべて閉じる

	// 同一グループ内の左からn番目のタブにフォーカス切替
	case F_TAB_1:
	case F_TAB_2:
	case F_TAB_3:
	case F_TAB_4:
	case F_TAB_5:
	case F_TAB_6:
	case F_TAB_7:
	case F_TAB_8:
	case F_TAB_9:
		// TODO: refactoring...
		{
			auto sd = &GetDllShareData();
			HWND hActiveWnd = GetActiveWindow();
			int activeWndGroup = -1;
			for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
				auto& editNode = sd->nodes.pEditArr[i];
				if (editNode.GetHwnd() == hActiveWnd) {
					activeWndGroup = editNode.nGroup;
					break;
				}
			}
			if (activeWndGroup != -1) {
				std::vector<int> indices;
				for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
					auto& editNode = sd->nodes.pEditArr[i];
					if (editNode.nGroup == activeWndGroup) {
						indices.push_back(editNode.nIndex);
					}
				}
				std::sort(indices.begin(), indices.end());
				int idx = nCommand - F_TAB_1;
				if (idx < (int)indices.size()) {
					int nodeIdx = indices[idx];
					for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
						auto& editNode = sd->nodes.pEditArr[i];
						if (editNode.nIndex == nodeIdx) {
							ActivateFrameWindow(editNode.hWnd);
							break;
						}
					}
				}
			}
		}
		break;

	// 支援
	case F_HOKAN:			Command_Hokan(); break;			// 入力補完
	case F_HELP_CONTENTS:	Command_Help_Contents(); break;	// ヘルプ目次
	case F_HELP_SEARCH:		Command_Help_Search(); break;	// ヘルプトキーワード検索
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch((int)lparam1); break;	// キャレット位置の単語を辞書検索する機能ON-OFF
	case F_MENU_ALLFUNC:									// コマンド一覧
		// 再帰処理対策
		view.SetUndoBuffer(true);
		Command_Menu_AllFunc();
		return bRet;
	case F_EXTHELP1:	Command_ExtHelp1(); break;		// 外部ヘルプ１
	case F_EXTHTMLHELP:	// 外部HTMLヘルプ
		Command_ExtHTMLHelp((const wchar_t*)lparam1, (const wchar_t*)lparam2);
		break;
	case F_ABOUT:	Command_About(); break;				// バージョン情報

	// その他

	case F_0: break; // F_0でプラグインが実行されるバグ対策	// ← rev1886 の問題は呼び元で対策したが安全弁として残す

	default:
		// プラグインコマンドを実行する
		{
			view.SetUndoBuffer(true); // 再帰対応

			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_COMMAND, nCommand, &plugs);
			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// インタフェースオブジェクト準備
				WSHIfObj::List params;
				// プラグイン呼び出し
				(*plugs.begin())->Invoke(view, params);

				return bRet;
			}
		}
	}

	// アンドゥバッファの処理
	view.SetUndoBuffer(true);

	return bRet;
}


void ViewCommander::Sub_BoxSelectLock( int flags )
{
	bool bSelLock;
	if (flags == 0x00) {
		bSelLock = GetDllShareData().common.edit.bBoxSelectLock;
	}else if (flags == 0x01) {
		bSelLock = true;
	}else if (flags == 0x02) {
		bSelLock = false;
	}
	if (!this->view.GetSelectionInfo().IsBoxSelecting()) {
		this->Command_Begin_BoxSelect( bSelLock );
	}
}


size_t ViewCommander::ConvertEol(
	const wchar_t* pszText,
	size_t nTextLen,
	wchar_t* pszConvertedText
	)
{
	size_t nConvertedTextLen;
	Eol eol = GetDocument().docEditor.GetNewLineCode();

	nConvertedTextLen = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	if (!pszConvertedText) {
		for (size_t i=0; i<nTextLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
				if (pszText[i] == WCODE::CR) {
					if (i + 1 < nTextLen && pszText[i + 1] == WCODE::LF) {
						++i;
					}
				}
				nConvertedTextLen += eol.GetLen();
			}else {
				++nConvertedTextLen;
			}
		}
	}else {
		for (size_t i=0; i<nTextLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
				if (pszText[i] == WCODE::CR) {
					if (i + 1 < nTextLen && pszText[i + 1] == WCODE::LF) {
						++i;
					}
				}
				wmemcpy(&pszConvertedText[nConvertedTextLen], eol.GetValue2(), eol.GetLen());
				nConvertedTextLen += eol.GetLen();
			}else {
				pszConvertedText[nConvertedTextLen++] = pszText[i];
			}
		}
	}
	return nConvertedTextLen;
}


/*!
	@brief 検索で見つからないときの警告（メッセージボックス／サウンド）
*/
void ViewCommander::AlertNotFound(
	HWND hwnd,
	bool bReplaceAll,
	LPCTSTR format,
	...
	)
{
	if (GetDllShareData().common.search.bNotifyNotFound
		&& !bReplaceAll
	) {
		if (!hwnd) {
			hwnd = view.GetHwnd();
		}
		//InfoMessage(hwnd, format, __VA_ARGS__);
		va_list p;
		va_start(p, format);
		VMessageBoxF(hwnd, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, format, p);
		va_end(p);
	}else {
		DefaultBeep();
	}
}

