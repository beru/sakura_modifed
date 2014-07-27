/*!	@file
	@brief エディタプラグインクラス
	
	@date 2014.02.08	CtrlCode, TagJumpEx追加
*/
/*
	Copyright (C) 2013-2014, Plugins developers

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
#ifndef CEXTERNALEDITORIFOBJ_H_D0815A78_4BA4_4E2B_B9AF_79D32CB1B367
#define CEXTERNALEDITORIFOBJ_H_D0815A78_4BA4_4E2B_B9AF_79D32CB1B367

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	Plugin Editor interface object class
*/
class CExternalEditorIfObj : public CExternalIfObj
{
private:
	int m_nFlags;

public:
	CExternalEditorIfObj(){
		m_nFlags = 0;
	}
	virtual ~CExternalEditorIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Editor";
	}

	void SetFlags(int flags){
		m_nFlags = flags;
	}

	virtual BOOL HandleFunction(DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result){
		return CExternalIfObj::HandleFunction(ID | m_nFlags, Arguments, ArgSize, Result);
	}

	virtual void HandleCommand(DWORD ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize){
		CExternalIfObj::HandleCommand(ID | m_nFlags, Arguments, ArgLengths, ArgSize);
	}

public:	//EditorIfObj
	//Function
	WideString S_GetFilename();
	WideString S_GetSaveFilename();
	WideString S_GetSelectedString(const int arg1);
	WideString S_ExpandParameter(LPCWSTR arg1);
	WideString S_GetLineStr(const int arg1);
	int        S_GetLineCount(const int arg1);
	int        S_ChangeTabWidth(const int arg1);
	int        S_IsTextSelected();
	int        S_GetSelectLineFrom();
	int        S_GetSelectColumnFrom();
	int        S_GetSelectLineTo();
	int        S_GetSelectColumnTo();
	int        S_IsInsMode();
	int        S_GetCharCode();
	int        S_GetLineCode();
	int        S_IsPossibleUndo();
	int        S_IsPossibleRedo();
	int        S_ChangeWrapColumn(const int arg1);
	int        S_IsCurTypeExt(LPCWSTR arg1);
	int        S_IsSameTypeExt(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_InputBox(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	int        S_MessageBox(LPCWSTR arg1, const int arg2);
	int        S_ErrorMsg(LPCWSTR arg1);
	int        S_WarnMsg(LPCWSTR arg1);
	int        S_InfoMsg(LPCWSTR arg1);
	int        S_OkCancelBox(LPCWSTR arg1);
	int        S_YesNoBox(LPCWSTR arg1);
	int        S_CompareVersion(LPCWSTR arg1, LPCWSTR arg2);
	int        S_Sleep(const int arg1);
	WideString S_FileOpenDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_FileSaveDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_FolderDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_GetClipboard(const int arg1);
	int        S_SetClipboard(const int arg1, LPCWSTR arg2);
	int        S_LayoutToLogicLineNum(const int arg1);
	int        S_LogicToLayoutLineNum(const int arg1, const int arg2);
	int        S_LineColumnToIndex(const int arg1, const int arg2);
	int        S_LineIndexToColumn(const int arg1, const int arg2);
	WideString S_GetCookie(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_GetCookieDefault(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	int        S_SetCookie(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	int        S_DeleteCookie(LPCWSTR arg1, LPCWSTR arg2);
	WideString S_GetCookieNames(LPCWSTR arg1);
	int        S_SetDrawSwitch(const int arg1);
	int        S_GetDrawSwitch();
	int        S_IsShownStatus();
	int        S_GetStrWidth(LPCWSTR arg1, const int arg2);
	int        S_GetStrLayoutLength(LPCWSTR arg1, const int arg2);
	int        S_GetDefaultCharLength();
	int        S_IsIncludeClipboardFormat(LPCWSTR arg1);
	WideString S_GetClipboardByFormat(LPCWSTR arg1, const int arg2, const int arg3);
	int        S_SetClipboardByFormat(LPCWSTR arg1, LPCWSTR arg2, const int arg3, const int arg4);

	//Command
	//ファイル操作系
	void S_FileNew();
	void S_FileOpen(LPCWSTR arg1, const int arg2, const int arg3, LPCWSTR arg4);
	void S_FileSave();
	void S_FileSaveAll();
	void S_FileSaveAsDialog(LPCWSTR arg1, const int arg2, const int arg3);
	void S_FileSaveAs(LPCWSTR arg1, const int arg2, const int arg3);
	void S_FileClose();
	void S_FileCloseOpen();
	void S_FileReopen(const int arg1);
	void S_FileReopenSJIS(const int arg1);
	void S_FileReopenJIS(const int arg1);
	void S_FileReopenEUC(const int arg1);
	void S_FileReopenLatin1(const int arg1);
	void S_FileReopenUNICODE(const int arg1);
	void S_FileReopenUNICODEBE(const int arg1);
	void S_FileReopenUTF8(const int arg1);
	void S_FileReopenCESU8(const int arg1);
	void S_FileReopenUTF7(const int arg1);
	void S_Print();
	void S_PrintPreview();
	void S_PrintPageSetup();
	void S_OpenHfromtoC();
	void S_ActivateSQLPLUS();
	void S_ExecSQLPLUS();
	void S_Browse();
	void S_ViewMode();
	void S_ReadOnly();
	void S_PropertyFile();
	void S_ExitAllEditors();
	void S_ExitAll();
	void S_PutFile(LPCWSTR arg1, const int arg2, const int arg3);
	void S_InsFile(LPCWSTR arg1, const int arg2, const int arg3);
	//編集系
	void S_Char(const int arg1);
	void S_CharIme(const int arg1);
	void S_Undo();
	void S_Redo();
	void S_Delete();
	void S_DeleteBack();
	void S_WordDeleteToStart();
	void S_WordDeleteToEnd();
	void S_WordCut();
	void S_WordDelete();
	void S_LineCutToStart();
	void S_LineCutToEnd();
	void S_LineDeleteToStart();
	void S_LineDeleteToEnd();
	void S_CutLine();
	void S_DeleteLine();
	void S_DuplicateLine();
	void S_IndentTab();
	void S_UnindentTab();
	void S_IndentSpace();
	void S_UnindentSpace();
	void S_LTrim();
	void S_RTrim();
	void S_SortAsc();
	void S_SortDesc();
	void S_Merge();
	//カーソル移動系
	void S_Up();
	void S_Down();
	void S_Left();
	void S_Right();
	void S_Up2();
	void S_Down2();
	void S_WordLeft();
	void S_WordRight();
	void S_GoLineTop(const int arg1);
	void S_GoLineEnd(const int arg1);
	void S_HalfPageUp();
	void S_HalfPageDown();
	void S_PageUp();
	void S_1PageUp();
	void S_PageDown();
	void S_1PageDown();
	void S_GoFileTop();
	void S_GoFileEnd();
	void S_CurLineCenter();
	void S_MoveHistPrev();
	void S_MoveHistNext();
	void S_MoveHistSet();
	void S_WndScrollDown();
	void S_WndScrollUp();
	void S_GoNextParagraph();
	void S_GoPrevParagraph();
	void S_MoveCursor(const int arg1, const int arg2, const int arg3);
	void S_MoveCursorLayout(const int arg1, const int arg2, const int arg3);
	void S_WheelUp(const int arg1);
	void S_WheelDown(const int arg1);
	void S_WheelLeft(const int arg1);
	void S_WheelRight(const int arg1);
	void S_WheelPageUp(const int arg1);
	void S_WheelPageDown(const int arg1);
	void S_WheelPageLeft(const int arg1);
	void S_WheelPageRight(const int arg1);
	//選択系
	void S_SelectWord();
	void S_SelectAll();
	void S_SelectLine(const int arg1);
	void S_BeginSelect();
	void S_Up_Sel();
	void S_Down_Sel();
	void S_Left_Sel();
	void S_Right_Sel();
	void S_Up2_Sel();
	void S_Down2_Sel();
	void S_WordLeft_Sel();
	void S_WordRight_Sel();
	void S_GoLineTop_Sel(const int arg1);
	void S_GoLineEnd_Sel(const int arg1);
	void S_HalfPageUp_Sel();
	void S_HalfPageDown_Sel();
	void S_PageUp_Sel();
	void S_1PageUp_Sel();
	void S_PageDown_Sel();
	void S_1PageDown_Sel();
	void S_GoFileTop_Sel();
	void S_GoFileEnd_Sel();
	void S_GoNextParagraph_Sel();
	void S_GoPrevParagraph_Sel();
	void S_BeginBoxSelect();
	void S_Up_BoxSel();
	void S_Down_BoxSel();
	void S_Left_BoxSel();
	void S_Right_BoxSel();
	void S_Up2_BoxSel();
	void S_Down2_BoxSel();
	//void S_Left2_BoxSel();
	//void S_Right2_BoxSel();
	void S_WordLeft_BoxSel();
	void S_WordRight_BoxSel();
	void S_GoLogicalLineTop_BoxSel(const int arg1);
	void S_GoLineTop_BoxSel(const int arg1);
	void S_GoLineEnd_BoxSel(const int arg1);
	void S_HalfPageUp_BoxSel();
	void S_HalfPageDown_BoxSel();
	void S_PageUp_BoxSel();
	void S_1PageUp_BoxSel();
	void S_PageDown_BoxSel();
	void S_1PageDown_BoxSel();
	void S_GoFileTop_BoxSel();
	void S_GoFileEnd_BoxSel();
	//クリップボード系
	void S_Cut();
	void S_Copy();
	void S_Paste(const int arg1);
	void S_CopyAddCRLF();
	void S_CopyCRLF();
	void S_PasteBox(const int arg1);
	void S_InsBoxText(LPCWSTR arg1);
	void S_InsText(LPCWSTR arg1);
	void S_AddTail(LPCWSTR arg1);
	void S_CopyLines();
	void S_CopyLinesAsPassage();
	void S_CopyLinesWithLineNumber();
	void S_CopyColorHtml();
	void S_CopyColorHtmlWithLineNumber();
	void S_CopyPath();
	void S_CopyFilename();
	void S_CopyTag();
	void S_CopyKeyBindList();
	//挿入系
	void S_InsertDate();
	void S_InsertTime();
	void S_CtrlCodeDialog();
	void S_CtrlCode(const int arg1);
	//変換系
	void S_ToLower();
	void S_ToUpper();
	void S_ToHankaku();
	void S_ToHankata();
	void S_ToZenEi();
	void S_ToHanEi();
	void S_ToZenKata();
	void S_ToZenHira();
	void S_HanKataToZenKata();
	void S_HanKataToZenHira();
	void S_TABToSPACE();
	void S_SPACEToTAB();
	void S_AutoToSJIS();
	void S_JIStoSJIS();
	void S_EUCtoSJIS();
	void S_CodeCnvUNICODEtoSJIS();
	void S_CodeCnvUNICODEBEtoSJIS();
	void S_UTF8toSJIS();
	void S_UTF7toSJIS();
	void S_SJIStoJIS();
	void S_SJIStoEUC();
	void S_SJIStoUTF8();
	void S_SJIStoUTF7();
	void S_Base64Decode();
	void S_Uudecode();
	//検索系
	void S_SearchDialog();
	void S_SearchNext(LPCWSTR arg1, const int arg2);
	void S_SearchPrev(LPCWSTR arg1, const int arg2);
	void S_ReplaceDialog();
	void S_Replace(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	void S_ReplaceAll(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	void S_SearchClearMark();
	void S_SearchStartPos();
	void S_Grep(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3, const int arg4, const int arg5);
	void S_Jump(const int arg1, const int arg2);
	void S_Outline(const int arg1);
	void S_TagJump();
	void S_TagJumpBack();
	void S_TagMake();
	void S_DirectTagJump();
	void S_KeywordTagJump(LPCWSTR arg1);
	void S_Compare();
	void S_DiffDialog();
	void S_Diff(LPCWSTR arg1, const int arg2);
	void S_DiffNext();
	void S_DiffPrev();
	void S_DiffReset();
	void S_BracketPair();
	void S_BookmarkSet();
	void S_BookmarkNext();
	void S_BookmarkPrev();
	void S_BookmarkReset();
	void S_BookmarkView();
	void S_BookmarkPattern(LPCWSTR arg1, const int arg2);
	void S_TagJumpEx(LPCWSTR arg1, const int arg2, const int arg3, const int arg4);
	//モード切り替え系
	void S_ChgmodINS();
	void S_ChgCharSet(const int arg1, const int arg2);
	void S_ChgmodEOL(const int arg1);
	void S_CancelMode();
	//マクロ系
	void S_ExecExternalMacro(LPCWSTR arg1, LPCWSTR arg2);
	//設定系
	void S_ShowToolbar();
	void S_ShowFunckey();
	void S_ShowTab();
	void S_ShowStatusbar();
	void S_TypeList();
	void S_ChangeType(const int arg1);
	void S_OptionType();
	void S_OptionCommon();
	void S_SelectFont();
	void S_SetFontSize(const int arg1, const int arg2, const int arg3);
	void S_WrapWindowWidth();
	void S_OptionFavorite();
	void S_SetMsgQuoteStr(LPCWSTR arg1);
	void S_TextWrapMethod(const int arg1);
	void S_SelectCountMode(const int arg1);
	void S_ExecCommand(LPCWSTR arg1, const int arg2, LPCWSTR arg3);
	void S_ExecCommandDialog();
	//カスタムメニュー
	void S_RMenu();
	void S_CustMenu1();
	void S_CustMenu2();
	void S_CustMenu3();
	void S_CustMenu4();
	void S_CustMenu5();
	void S_CustMenu6();
	void S_CustMenu7();
	void S_CustMenu8();
	void S_CustMenu9();
	void S_CustMenu10();
	void S_CustMenu11();
	void S_CustMenu12();
	void S_CustMenu13();
	void S_CustMenu14();
	void S_CustMenu15();
	void S_CustMenu16();
	void S_CustMenu17();
	void S_CustMenu18();
	void S_CustMenu19();
	void S_CustMenu20();
	void S_CustMenu21();
	void S_CustMenu22();
	void S_CustMenu23();
	void S_CustMenu24();
	//ウィンドウ系
	void S_SplitWinV();
	void S_SplitWinH();
	void S_SplitWinVH();
	void S_WinClose();
	void S_WinCloseAll();
	void S_CascadeWin();
	void S_TileWinV();
	void S_TileWinH();
	void S_NextWindow();
	void S_PrevWindow();
	void S_WindowList();
	void S_MaximizeV();
	void S_MaximizeH();
	void S_MinimizeAll();
	void S_ReDraw();
	void S_ActivateWinOutput();
	void S_TraceOut(LPCWSTR arg1, const int arg2);
	void S_WindowTopMost(const int arg1);
	void S_GroupClose();
	void S_NextGroup();
	void S_PrevGroup();
	void S_TabMoveRight();
	void S_TabMoveLeft();
	void S_TabSeparate();
	void S_TabJointNext();
	void S_TabJointPrev();
	void S_TabCloseOther();
	void S_TabCloseLeft();
	void S_TabCloseRight();
	//支援
	void S_Complete();
	void S_ToggleKeyHelpSearch(const int arg1);
	void S_HelpContents();
	void S_HelpSearch();
	void S_CommandList();
	void S_ExtHelp1();
	void S_ExtHtmlHelp(LPCWSTR arg1, LPCWSTR arg2);
	void S_About();
	//マクロ用
	void S_StatusMsg(LPCWSTR arg1, const int arg2);
	void S_MsgBeep(const int arg1);
	void S_CommitUndoBuffer();
	void S_AddRefUndoBuffer();
	void S_SetUndoBuffer();
	void S_AppendUndoBufferCursor();
	void S_ClipboardEmpty();

	//Aliases
	//Function
	WideString GetFilename();
	WideString GetSaveFilename();
	WideString GetSelectedString(const int arg1);
	WideString ExpandParameter(LPCWSTR arg1);
	WideString GetLineStr(const int arg1);
	int        GetLineCount(const int arg1);
	int        ChangeTabWidth(const int arg1);
	int        IsTextSelected();
	int        GetSelectLineFrom();
	int        GetSelectColumnFrom();
	int        GetSelectLineTo();
	int        GetSelectColumnTo();
	int        IsInsMode();
	int        GetCharCode();
	int        GetLineCode();
	int        IsPossibleUndo();
	int        IsPossibleRedo();
	int        ChangeWrapColumn(const int arg1);
	int        IsCurTypeExt(LPCWSTR arg1);
	int        IsSameTypeExt(LPCWSTR arg1, LPCWSTR arg2);
	WideString InputBox(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	int        MessageBox(LPCWSTR arg1, const int arg2);
	int        ErrorMsg(LPCWSTR arg1);
	int        WarnMsg(LPCWSTR arg1);
	int        InfoMsg(LPCWSTR arg1);
	int        OkCancelBox(LPCWSTR arg1);
	int        YesNoBox(LPCWSTR arg1);
	int        CompareVersion(LPCWSTR arg1, LPCWSTR arg2);
	int        Sleep(const int arg1);
	WideString FileOpenDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString FileSaveDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString FolderDialog(LPCWSTR arg1, LPCWSTR arg2);
	WideString GetClipboard(const int arg1);
	int        SetClipboard(const int arg1, LPCWSTR arg2);
	int        LayoutToLogicLineNum(const int arg1);
	int        LogicToLayoutLineNum(const int arg1, const int arg2);
	int        LineColumnToIndex(const int arg1, const int arg2);
	int        LineIndexToColumn(const int arg1, const int arg2);
	WideString GetCookie(LPCWSTR arg1, LPCWSTR arg2);
	WideString GetCookieDefault(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	int        SetCookie(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	int        DeleteCookie(LPCWSTR arg1, LPCWSTR arg2);
	WideString GetCookieNames(LPCWSTR arg1);
	int        SetDrawSwitch(const int arg1);
	int        GetDrawSwitch();
	int        IsShownStatus();
	int        GetStrWidth(LPCWSTR arg1, const int arg2);
	int        GetStrLayoutLength(LPCWSTR arg1, const int arg2);
	int        GetDefaultCharLength();
	int        IsIncludeClipboardFormat(LPCWSTR arg1);
	WideString GetClipboardByFormat(LPCWSTR arg1, const int arg2, const int arg3);
	int        SetClipboardByFormat(LPCWSTR arg1, LPCWSTR arg2, const int arg3, const int arg4);

	//Command
	//ファイル操作系
	void FileNew();
	void FileOpen(LPCWSTR arg1, const int arg2, const int arg3, LPCWSTR arg4);
	void FileSave();
	void FileSaveAll();
	void FileSaveAsDialog(LPCWSTR arg1, const int arg2, const int arg3);
	void FileSaveAs(LPCWSTR arg1, const int arg2, const int arg3);
	void FileClose();
	void FileCloseOpen();
	void FileReopen(const int arg1);
	void FileReopenSJIS(const int arg1);
	void FileReopenJIS(const int arg1);
	void FileReopenEUC(const int arg1);
	void FileReopenLatin1(const int arg1);
	void FileReopenUNICODE(const int arg1);
	void FileReopenUNICODEBE(const int arg1);
	void FileReopenUTF8(const int arg1);
	void FileReopenCESU8(const int arg1);
	void FileReopenUTF7(const int arg1);
	void Print();
	void PrintPreview();
	void PrintPageSetup();
	void OpenHfromtoC();
	void ActivateSQLPLUS();
	void ExecSQLPLUS();
	void Browse();
	void ViewMode();
	void ReadOnly();
	void PropertyFile();
	void ExitAllEditors();
	void ExitAll();
	void PutFile(LPCWSTR arg1, const int arg2, const int arg3);
	void InsFile(LPCWSTR arg1, const int arg2, const int arg3);
	//編集系
	void Char(const int arg1);
	void CharIme(const int arg1);
	void Undo();
	void Redo();
	void Delete();
	void DeleteBack();
	void WordDeleteToStart();
	void WordDeleteToEnd();
	void WordCut();
	void WordDelete();
	void LineCutToStart();
	void LineCutToEnd();
	void LineDeleteToStart();
	void LineDeleteToEnd();
	void CutLine();
	void DeleteLine();
	void DuplicateLine();
	void IndentTab();
	void UnindentTab();
	void IndentSpace();
	void UnindentSpace();
	void LTrim();
	void RTrim();
	void SortAsc();
	void SortDesc();
	void Merge();
	//カーソル移動系
	void Up();
	void Down();
	void Left();
	void Right();
	void Up2();
	void Down2();
	void WordLeft();
	void WordRight();
	void GoLineTop(const int arg1);
	void GoLineEnd(const int arg1);
	void HalfPageUp();
	void HalfPageDown();
	void PageUp();
	void OnePageUp();
	void PageDown();
	void OnePageDown();
	void GoFileTop();
	void GoFileEnd();
	void CurLineCenter();
	void MoveHistPrev();
	void MoveHistNext();
	void MoveHistSet();
	void WndScrollDown();
	void WndScrollUp();
	void GoNextParagraph();
	void GoPrevParagraph();
	void MoveCursor(const int arg1, const int arg2, const int arg3);
	void MoveCursorLayout(const int arg1, const int arg2, const int arg3);
	void WheelUp(const int arg1);
	void WheelDown(const int arg1);
	void WheelLeft(const int arg1);
	void WheelRight(const int arg1);
	void WheelPageUp(const int arg1);
	void WheelPageDown(const int arg1);
	void WheelPageLeft(const int arg1);
	void WheelPageRight(const int arg1);
	//選択系
	void SelectWord();
	void SelectAll();
	void SelectLine(const int arg1);
	void BeginSelect();
	void Up_Sel();
	void Down_Sel();
	void Left_Sel();
	void Right_Sel();
	void Up2_Sel();
	void Down2_Sel();
	void WordLeft_Sel();
	void WordRight_Sel();
	void GoLineTop_Sel(const int arg1);
	void GoLineEnd_Sel(const int arg1);
	void HalfPageUp_Sel();
	void HalfPageDown_Sel();
	void PageUp_Sel();
	void OnePageUp_Sel();
	void PageDown_Sel();
	void OnePageDown_Sel();
	void GoFileTop_Sel();
	void GoFileEnd_Sel();
	void GoNextParagraph_Sel();
	void GoPrevParagraph_Sel();
	//矩形選択系
	void BeginBoxSelect();
	void Up_BoxSel();
	void Down_BoxSel();
	void Left_BoxSel();
	void Right_BoxSel();
	void Up2_BoxSel();
	void Down2_BoxSel();
	//void Left2_BoxSel();
	//void Right2_BoxSel();
	void WordLeft_BoxSel();
	void WordRight_BoxSel();
	void GoLogicalLineTop_BoxSel(const int arg1);
	void GoLineTop_BoxSel(const int arg1);
	void GoLineEnd_BoxSel(const int arg1);
	void HalfPageUp_BoxSel();
	void HalfPageDown_BoxSel();
	void PageUp_BoxSel();
	void OnePageUp_BoxSel();	//1PageUp_BoxSel
	void PageDown_BoxSel();
	void OnePageDown_BoxSel();	//1PageDown_BoxSel
	void GoFileTop_BoxSel();
	void GoFileEnd_BoxSel();
	//クリップボード系
	void Cut();
	void Copy();
	void Paste(const int arg1);
	void CopyAddCRLF();
	void CopyCRLF();
	void PasteBox(const int arg1);
	void InsBoxText(LPCWSTR arg1);
	void InsText(LPCWSTR arg1);
	void AddTail(LPCWSTR arg1);
	void CopyLines();
	void CopyLinesAsPassage();
	void CopyLinesWithLineNumber();
	void CopyColorHtml();
	void CopyColorHtmlWithLineNumber();
	void CopyPath();
	void CopyFilename();
	void CopyTag();
	void CopyKeyBindList();
	//挿入系
	void InsertDate();
	void InsertTime();
	void CtrlCodeDialog();
	void CtrlCode(const int arg1);
	//変換系
	void ToLower();
	void ToUpper();
	void ToHankaku();
	void ToHankata();
	void ToZenEi();
	void ToHanEi();
	void ToZenKata();
	void ToZenHira();
	void HanKataToZenKata();
	void HanKataToZenHira();
	void TABToSPACE();
	void SPACEToTAB();
	void AutoToSJIS();
	void JIStoSJIS();
	void EUCtoSJIS();
	void CodeCnvUNICODEtoSJIS();
	void CodeCnvUNICODEBEtoSJIS();
	void UTF8toSJIS();
	void UTF7toSJIS();
	void SJIStoJIS();
	void SJIStoEUC();
	void SJIStoUTF8();
	void SJIStoUTF7();
	void Base64Decode();
	void Uudecode();
	//検索系
	void SearchDialog();
	void SearchNext(LPCWSTR arg1, const int arg2);
	void SearchPrev(LPCWSTR arg1, const int arg2);
	void ReplaceDialog();
	void Replace(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	void ReplaceAll(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	void SearchClearMark();
	void SearchStartPos();
	void Grep(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3, const int arg4, const int arg5);
	void Jump(const int arg1, const int arg2);
	void Outline(const int arg1);
	void TagJump();
	void TagJumpBack();
	void TagMake();
	void DirectTagJump();
	void KeywordTagJump(LPCWSTR arg1);
	void Compare();
	void DiffDialog();
	void Diff(LPCWSTR arg1, const int arg2);
	void DiffNext();
	void DiffPrev();
	void DiffReset();
	void BracketPair();
	void BookmarkSet();
	void BookmarkNext();
	void BookmarkPrev();
	void BookmarkReset();
	void BookmarkView();
	void BookmarkPattern(LPCWSTR arg1, const int arg2);
	void TagJumpEx(LPCWSTR arg1, const int arg2, const int arg3, const int arg4);
	//モード切り替え系
	void ChgmodINS();
	void ChgCharSet(const int arg1, const int arg2);
	void ChgmodEOL(const int arg1);
	void CancelMode();
	//マクロ系
	void ExecExternalMacro(LPCWSTR arg1, LPCWSTR arg2);
	//設定系
	void ShowToolbar();
	void ShowFunckey();
	void ShowTab();
	void ShowStatusbar();
	void TypeList();
	void ChangeType(const int arg1);
	void OptionType();
	void OptionCommon();
	void SelectFont();
	void SetFontSize(const int arg1, const int arg2, const int arg3);
	void WrapWindowWidth();
	void OptionFavorite();
	void SetMsgQuoteStr(LPCWSTR arg1);
	void TextWrapMethod(const int arg1);
	void SelectCountMode(const int arg1);
	void ExecCommand(LPCWSTR arg1, const int arg2, LPCWSTR arg3);
	void ExecCommandDialog();
	//カスタムメニュー
	void RMenu();
	void CustMenu1();
	void CustMenu2();
	void CustMenu3();
	void CustMenu4();
	void CustMenu5();
	void CustMenu6();
	void CustMenu7();
	void CustMenu8();
	void CustMenu9();
	void CustMenu10();
	void CustMenu11();
	void CustMenu12();
	void CustMenu13();
	void CustMenu14();
	void CustMenu15();
	void CustMenu16();
	void CustMenu17();
	void CustMenu18();
	void CustMenu19();
	void CustMenu20();
	void CustMenu21();
	void CustMenu22();
	void CustMenu23();
	void CustMenu24();
	//ウィンドウ系
	void SplitWinV();
	void SplitWinH();
	void SplitWinVH();
	void WinClose();
	void WinCloseAll();
	void CascadeWin();
	void TileWinV();
	void TileWinH();
	void NextWindow();
	void PrevWindow();
	void WindowList();
	void MaximizeV();
	void MaximizeH();
	void MinimizeAll();
	void ReDraw();
	void ActivateWinOutput();
	void TraceOut(LPCWSTR arg1, const int arg2);
	void WindowTopMost(const int arg1);
	void GroupClose();
	void NextGroup();
	void PrevGroup();
	void TabMoveRight();
	void TabMoveLeft();
	void TabSeparate();
	void TabJointNext();
	void TabJointPrev();
	void TabCloseOther();
	void TabCloseLeft();
	void TabCloseRight();
	//支援
	void Complete();
	void ToggleKeyHelpSearch(const int arg1);
	void HelpContents();
	void HelpSearch();
	void CommandList();
	void ExtHelp1();
	void ExtHtmlHelp(LPCWSTR arg1, LPCWSTR arg2);
	void About();
	//マクロ用
	void StatusMsg(LPCWSTR arg1, const int arg2);
	void MsgBeep(const int arg1);
	void CommitUndoBuffer();
	void AddRefUndoBuffer();
	void SetUndoBuffer();
	void AppendUndoBufferCursor();
	void ClipboardEmpty();

	//Aliases
	//Function
	WideString S_ExpandParameter(const WideString& arg1);
	int        S_IsCurTypeExt(const WideString& arg1);
	int        S_IsSameTypeExt(const WideString& arg1, const WideString& arg2);
	WideString S_InputBox(const WideString& arg1, const WideString& arg2, const int arg3);
	int        S_MessageBox(const WideString& arg1, const int arg2);
	int        S_ErrorMsg(const WideString& arg1);
	int        S_WarnMsg(const WideString& arg1);
	int        S_InfoMsg(const WideString& arg1);
	int        S_OkCancelBox(const WideString& arg1);
	int        S_YesNoBox(const WideString& arg1);
	int        S_CompareVersion(const WideString& arg1, const WideString& arg2);
	WideString S_FileOpenDialog(const WideString& arg1, const WideString& arg2);
	WideString S_FileSaveDialog(const WideString& arg1, const WideString& arg2);
	WideString S_FolderDialog(const WideString& arg1, const WideString& arg2);
	int        S_SetClipboard(const int arg1, const WideString& arg2);
	WideString S_GetCookie(const WideString& arg1, const WideString& arg2);
	WideString S_GetCookieDefault(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	int        S_SetCookie(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	int        S_DeleteCookie(const WideString& arg1, const WideString& arg2);
	WideString S_GetCookieNames(const WideString& arg1);
	int        S_GetStrWidth(const WideString& arg1, const int arg2);
	int        S_GetStrLayoutLength(const WideString& arg1, const int arg2);
	int        S_IsIncludeClipboardFormat(const WideString& arg1);
	WideString S_GetClipboardByFormat(const WideString& arg1, const int arg2, const int arg3);
	int        S_SetClipboardByFormat(const WideString& arg1, const WideString& arg2, const int arg3, const int arg4);

	//Command
	void S_FileOpen(const WideString& arg1, const int arg2, const int arg3, const WideString& arg4);
	void S_FileSaveAs(const WideString& arg1, const int arg2, const int arg3);
	void S_PutFile(const WideString& arg1, const int arg2, const int arg3);
	void S_InsFile(const WideString& arg1, const int arg2, const int arg3);
	void S_InsBoxText(const WideString& arg1);
	void S_InsText(const WideString& arg1);
	void S_AddTail(const WideString& arg1);
	void S_SearchNext(const WideString& arg1, const int arg2);
	void S_SearchPrev(const WideString& arg1, const int arg2);
	void S_Replace(const WideString& arg1, const WideString& arg2, const int arg3);
	void S_ReplaceAll(const WideString& arg1, const WideString& arg2, const int arg3);
	void S_Grep(const WideString& arg1, const WideString& arg2, const WideString& arg3, const int arg4, const int arg5);
	void S_KeywordTagJump(const WideString& arg1);
	void S_Diff(const WideString& arg1, const int arg2);
	void S_BookmarkPattern(const WideString& arg1, const int arg2);
	void S_TagJumpEx(const WideString& arg1, const int arg2, const int arg3, const int arg4);
	void S_ExecExternalMacro(const WideString& arg1, const WideString& arg2);
	void S_SetMsgQuoteStr(const WideString& arg1);
	void S_ExecCommand(const WideString& arg1, const int arg2, const WideString& arg3);
	void S_TraceOut(const WideString& arg1, const int arg2);
	void S_ExtHtmlHelp(const WideString& arg1, const WideString& arg2);

	//Aliases
	//Function
	WideString ExpandParameter(const WideString& arg1);
	int        IsCurTypeExt(const WideString& arg1);
	int        IsSameTypeExt(const WideString& arg1, const WideString& arg2);
	WideString InputBox(const WideString& arg1, const WideString& arg2, const int arg3);
	int        MessageBox(const WideString& arg1, const int arg2);
	int        ErrorMsg(const WideString& arg1);
	int        WarnMsg(const WideString& arg1);
	int        InfoMsg(const WideString& arg1);
	int        OkCancelBox(const WideString& arg1);
	int        YesNoBox(const WideString& arg1);
	int        CompareVersion(const WideString& arg1, const WideString& arg2);
	WideString FileOpenDialog(const WideString& arg1, const WideString& arg2);
	WideString FileSaveDialog(const WideString& arg1, const WideString& arg2);
	WideString FolderDialog(const WideString& arg1, const WideString& arg2);
	int        SetClipboard(const int arg1, const WideString& arg2);
	WideString GetCookie(const WideString& arg1, const WideString& arg2);
	WideString GetCookieDefault(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	int        SetCookie(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	int        DeleteCookie(const WideString& arg1, const WideString& arg2);
	WideString GetCookieNames(const WideString& arg1);
	int        GetStrWidth(const WideString& arg1, const int arg2);
	int        GetStrLayoutLength(const WideString& arg1, const int arg2);
	int        IsIncludeClipboardFormat(const WideString& arg1);
	WideString GetClipboardByFormat(const WideString& arg1, const int arg2, const int arg3);
	int        SetClipboardByFormat(const WideString& arg1, const WideString& arg2, const int arg3, const int arg4);

	//Command
	void FileOpen(const WideString& arg1, const int arg2, const int arg3, const WideString& arg4);
	void FileSaveAs(const WideString& arg1, const int arg2, const int arg3);
	void PutFile(const WideString& arg1, const int arg2, const int arg3);
	void InsFile(const WideString& arg1, const int arg2, const int arg3);
	void InsBoxText(const WideString& arg1);
	void InsText(const WideString& arg1);
	void AddTail(const WideString& arg1);
	void SearchNext(const WideString& arg1, const int arg2);
	void SearchPrev(const WideString& arg1, const int arg2);
	void Replace(const WideString& arg1, const WideString& arg2, const int arg3);
	void ReplaceAll(const WideString& arg1, const WideString& arg2, const int arg3);
	void Grep(const WideString& arg1, const WideString& arg2, const WideString& arg3, const int arg4, const int arg5);
	void KeywordTagJump(const WideString& arg1);
	void Diff(const WideString& arg1, const int arg2);
	void BookmarkPattern(const WideString& arg1, const int arg2);
	void TagJumpEx(const WideString& arg1, const int arg2, const int arg3, const int arg4);
	void ExecExternalMacro(const WideString& arg1, const WideString& arg2);
	void SetMsgQuoteStr(const WideString& arg1);
	void ExecCommand(const WideString& arg1, const int arg2, const WideString& arg3);
	void TraceOut(const WideString& arg1, const int arg2);
	void ExtHtmlHelp(const WideString& arg1, const WideString& arg2);

	//Aliases
	int ExpandParameter_x(){
		return _wtoi(ExpandParameter(L"$x").c_str());
	}
	int ExpandParameter_y(){
		return _wtoi(ExpandParameter(L"$y").c_str());
	}
};

#endif	//CEXTERNALEDITORIFOBJ_H_D0815A78_4BA4_4E2B_B9AF_79D32CB1B367
