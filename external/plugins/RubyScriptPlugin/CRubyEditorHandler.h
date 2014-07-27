/*!	@file
	@brief Editor Handler

	Editor Handlerを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.25
	@date 2014.02.08	TagJumpEx, CtrlCode追加
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

#ifndef _CRUBY_EDITOR_HANDLER_H_
#define _CRUBY_EDITOR_HANDLER_H_

#include <stdio.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	Editor Handlerをサポートするクラス
*/
class CRubyEditorHandler
{
public:
	CRubyEditorHandler();
	virtual ~CRubyEditorHandler();

	VALUE ReadyCommands(CRuby& Ruby);

protected:
	static DWORD GetIfObjType(){
		return CPluginService::IFOBJ_TYPE_EDITOR;
	}
	static VALUE CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

	//Ruby callback handler
private:
	//Commands
	static VALUE __cdecl S_FileNew(VALUE self);
	static VALUE __cdecl S_FileOpen(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4);
	static VALUE __cdecl S_FileSave(VALUE self);
	static VALUE __cdecl S_FileSaveAll(VALUE self);
	static VALUE __cdecl S_FileSaveAsDialog(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_FileSaveAs(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_FileClose(VALUE self);
	static VALUE __cdecl S_FileCloseOpen(VALUE self);
	static VALUE __cdecl S_FileReopen(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenSJIS(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenJIS(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenEUC(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenLatin1(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenUNICODE(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenUNICODEBE(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenUTF8(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenCESU8(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileReopenUTF7(VALUE self, VALUE arg1);
	static VALUE __cdecl S_Print(VALUE self);
	static VALUE __cdecl S_PrintPreview(VALUE self);
	static VALUE __cdecl S_PrintPageSetup(VALUE self);
	static VALUE __cdecl S_OpenHfromtoC(VALUE self);
	static VALUE __cdecl S_ActivateSQLPLUS(VALUE self);
	static VALUE __cdecl S_ExecSQLPLUS(VALUE self);
	static VALUE __cdecl S_Browse(VALUE self);
	static VALUE __cdecl S_ViewMode(VALUE self);
	static VALUE __cdecl S_ReadOnly(VALUE self);
	static VALUE __cdecl S_PropertyFile(VALUE self);
	static VALUE __cdecl S_ExitAllEditors(VALUE self);
	static VALUE __cdecl S_ExitAll(VALUE self);
	static VALUE __cdecl S_PutFile(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_InsFile(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_Char(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CharIme(VALUE self, VALUE arg1);
	static VALUE __cdecl S_Undo(VALUE self);
	static VALUE __cdecl S_Redo(VALUE self);
	static VALUE __cdecl S_Delete(VALUE self);
	static VALUE __cdecl S_DeleteBack(VALUE self);
	static VALUE __cdecl S_WordDeleteToStart(VALUE self);
	static VALUE __cdecl S_WordDeleteToEnd(VALUE self);
	static VALUE __cdecl S_WordCut(VALUE self);
	static VALUE __cdecl S_WordDelete(VALUE self);
	static VALUE __cdecl S_LineCutToStart(VALUE self);
	static VALUE __cdecl S_LineCutToEnd(VALUE self);
	static VALUE __cdecl S_LineDeleteToStart(VALUE self);
	static VALUE __cdecl S_LineDeleteToEnd(VALUE self);
	static VALUE __cdecl S_CutLine(VALUE self);
	static VALUE __cdecl S_DeleteLine(VALUE self);
	static VALUE __cdecl S_DuplicateLine(VALUE self);
	static VALUE __cdecl S_IndentTab(VALUE self);
	static VALUE __cdecl S_UnindentTab(VALUE self);
	static VALUE __cdecl S_IndentSpace(VALUE self);
	static VALUE __cdecl S_UnindentSpace(VALUE self);
	static VALUE __cdecl S_LTrim(VALUE self);
	static VALUE __cdecl S_RTrim(VALUE self);
	static VALUE __cdecl S_SortAsc(VALUE self);
	static VALUE __cdecl S_SortDesc(VALUE self);
	static VALUE __cdecl S_Merge(VALUE self);
	static VALUE __cdecl S_Up(VALUE self);
	static VALUE __cdecl S_Down(VALUE self);
	static VALUE __cdecl S_Left(VALUE self);
	static VALUE __cdecl S_Right(VALUE self);
	static VALUE __cdecl S_Up2(VALUE self);
	static VALUE __cdecl S_Down2(VALUE self);
	static VALUE __cdecl S_WordLeft(VALUE self);
	static VALUE __cdecl S_WordRight(VALUE self);
	static VALUE __cdecl S_GoLineTop(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GoLineEnd(VALUE self, VALUE arg1);
	static VALUE __cdecl S_HalfPageUp(VALUE self);
	static VALUE __cdecl S_HalfPageDown(VALUE self);
	static VALUE __cdecl S_PageUp(VALUE self);
	static VALUE __cdecl S_1PageUp(VALUE self);
	static VALUE __cdecl S_PageDown(VALUE self);
	static VALUE __cdecl S_1PageDown(VALUE self);
	static VALUE __cdecl S_GoFileTop(VALUE self);
	static VALUE __cdecl S_GoFileEnd(VALUE self);
	static VALUE __cdecl S_CurLineCenter(VALUE self);
	static VALUE __cdecl S_MoveHistPrev(VALUE self);
	static VALUE __cdecl S_MoveHistNext(VALUE self);
	static VALUE __cdecl S_MoveHistSet(VALUE self);
	static VALUE __cdecl S_F_WndScrollDown(VALUE self);
	static VALUE __cdecl S_F_WndScrollUp(VALUE self);
	static VALUE __cdecl S_GoNextParagraph(VALUE self);
	static VALUE __cdecl S_GoPrevParagraph(VALUE self);
	static VALUE __cdecl S_MoveCursor(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_MoveCursorLayout(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_WheelUp(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelDown(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelLeft(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelRight(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelPageUp(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelPageDown(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelPageLeft(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WheelPageRight(VALUE self, VALUE arg1);
	static VALUE __cdecl S_SelectWord(VALUE self);
	static VALUE __cdecl S_SelectAll(VALUE self);
	static VALUE __cdecl S_SelectLine(VALUE self, VALUE arg1);
	static VALUE __cdecl S_BeginSelect(VALUE self);
	static VALUE __cdecl S_Up_Sel(VALUE self);
	static VALUE __cdecl S_Down_Sel(VALUE self);
	static VALUE __cdecl S_Left_Sel(VALUE self);
	static VALUE __cdecl S_Right_Sel(VALUE self);
	static VALUE __cdecl S_Up2_Sel(VALUE self);
	static VALUE __cdecl S_Down2_Sel(VALUE self);
	static VALUE __cdecl S_WordLeft_Sel(VALUE self);
	static VALUE __cdecl S_WordRight_Sel(VALUE self);
	static VALUE __cdecl S_GoLineTop_Sel(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GoLineEnd_Sel(VALUE self, VALUE arg1);
	static VALUE __cdecl S_HalfPageUp_Sel(VALUE self);
	static VALUE __cdecl S_HalfPageDown_Sel(VALUE self);
	static VALUE __cdecl S_PageUp_Sel(VALUE self);
	static VALUE __cdecl S_1PageUp_Sel(VALUE self);
	static VALUE __cdecl S_PageDown_Sel(VALUE self);
	static VALUE __cdecl S_1PageDown_Sel(VALUE self);
	static VALUE __cdecl S_GoFileTop_Sel(VALUE self);
	static VALUE __cdecl S_GoFileEnd_Sel(VALUE self);
	static VALUE __cdecl S_GoNextParagraph_Sel(VALUE self);
	static VALUE __cdecl S_GoPrevParagraph_Sel(VALUE self);
	static VALUE __cdecl S_BeginBoxSelect(VALUE self);
	static VALUE __cdecl S_Up_BoxSel(VALUE self);
	static VALUE __cdecl S_Down_BoxSel(VALUE self);
	static VALUE __cdecl S_Left_BoxSel(VALUE self);
	static VALUE __cdecl S_Right_BoxSel(VALUE self);
	static VALUE __cdecl S_Up2_BoxSel(VALUE self);
	static VALUE __cdecl S_Down2_BoxSel(VALUE self);
	static VALUE __cdecl S_WordLeft_BoxSel(VALUE self);
	static VALUE __cdecl S_WordRight_BoxSel(VALUE self);
	static VALUE __cdecl S_GoLogicalLineTop_BoxSel(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GoLineTop_BoxSel(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GoLineEnd_BoxSel(VALUE self, VALUE arg1);
	static VALUE __cdecl S_HalfPageUp_BoxSel(VALUE self);
	static VALUE __cdecl S_HalfPageDown_BoxSel(VALUE self);
	static VALUE __cdecl S_PageUp_BoxSel(VALUE self);
	static VALUE __cdecl S_1PageUp_BoxSel(VALUE self);
	static VALUE __cdecl S_PageDown_BoxSel(VALUE self);
	static VALUE __cdecl S_1PageDown_BoxSel(VALUE self);
	static VALUE __cdecl S_GoFileTop_BoxSel(VALUE self);
	static VALUE __cdecl S_GoFileEnd_BoxSel(VALUE self);
	static VALUE __cdecl S_Cut(VALUE self);
	static VALUE __cdecl S_Copy(VALUE self);
	static VALUE __cdecl S_Paste(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CopyAddCRLF(VALUE self);
	static VALUE __cdecl S_CopyCRLF(VALUE self);
	static VALUE __cdecl S_PasteBox(VALUE self, VALUE arg1);
	static VALUE __cdecl S_InsBoxText(VALUE self, VALUE arg1);
	static VALUE __cdecl S_InsText(VALUE self, VALUE arg1);
	static VALUE __cdecl S_AddTail(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CopyLines(VALUE self);
	static VALUE __cdecl S_CopyLinesAsPassage(VALUE self);
	static VALUE __cdecl S_CopyLinesWithLineNumber(VALUE self);
	static VALUE __cdecl S_CopyColorHtml(VALUE self);
	static VALUE __cdecl S_CopyColorHtmlWithLineNumber(VALUE self);
	static VALUE __cdecl S_CopyPath(VALUE self);
	static VALUE __cdecl S_CopyFilename(VALUE self);
	static VALUE __cdecl S_CopyTag(VALUE self);
	static VALUE __cdecl S_CopyKeyBindList(VALUE self);
	static VALUE __cdecl S_InsertDate(VALUE self);
	static VALUE __cdecl S_InsertTime(VALUE self);
	static VALUE __cdecl S_CtrlCodeDialog(VALUE self);
	static VALUE __cdecl S_CtrlCode(VALUE self, VALUE arg1);
	static VALUE __cdecl S_ToLower(VALUE self);
	static VALUE __cdecl S_ToUpper(VALUE self);
	static VALUE __cdecl S_ToHankaku(VALUE self);
	static VALUE __cdecl S_ToHankata(VALUE self);
	static VALUE __cdecl S_ToZenEi(VALUE self);
	static VALUE __cdecl S_ToHanEi(VALUE self);
	static VALUE __cdecl S_ToZenKata(VALUE self);
	static VALUE __cdecl S_ToZenHira(VALUE self);
	static VALUE __cdecl S_HanKataToZenKata(VALUE self);
	static VALUE __cdecl S_HanKataToZenHira(VALUE self);
	static VALUE __cdecl S_TABToSPACE(VALUE self);
	static VALUE __cdecl S_SPACEToTAB(VALUE self);
	static VALUE __cdecl S_AutoToSJIS(VALUE self);
	static VALUE __cdecl S_JIStoSJIS(VALUE self);
	static VALUE __cdecl S_EUCtoSJIS(VALUE self);
	static VALUE __cdecl S_CodeCnvUNICODEtoSJIS(VALUE self);
	static VALUE __cdecl S_CodeCnvUNICODEBEtoSJIS(VALUE self);
	static VALUE __cdecl S_UTF8toSJIS(VALUE self);
	static VALUE __cdecl S_UTF7toSJIS(VALUE self);
	static VALUE __cdecl S_SJIStoJIS(VALUE self);
	static VALUE __cdecl S_SJIStoEUC(VALUE self);
	static VALUE __cdecl S_SJIStoUTF8(VALUE self);
	static VALUE __cdecl S_SJIStoUTF7(VALUE self);
	static VALUE __cdecl S_Base64Decode(VALUE self);
	static VALUE __cdecl S_Uudecode(VALUE self);
	static VALUE __cdecl S_SearchDialog(VALUE self);
	static VALUE __cdecl S_SearchNext(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_SearchPrev(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_ReplaceDialog(VALUE self);
	static VALUE __cdecl S_Replace(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_ReplaceAll(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_SearchClearMark(VALUE self);
	static VALUE __cdecl S_SearchStartPos(VALUE self);
	static VALUE __cdecl S_Grep(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4, VALUE arg5);
	static VALUE __cdecl S_Jump(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_Outline(VALUE self, VALUE arg1);
	static VALUE __cdecl S_TagJump(VALUE self);
	static VALUE __cdecl S_TagJumpBack(VALUE self);
	static VALUE __cdecl S_TagMake(VALUE self);
	static VALUE __cdecl S_DirectTagJump(VALUE self);
	static VALUE __cdecl S_KeywordTagJump(VALUE self, VALUE arg1);
	static VALUE __cdecl S_Compare(VALUE self);
	static VALUE __cdecl S_DiffDialog(VALUE self);
	static VALUE __cdecl S_Diff(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_DiffNext(VALUE self);
	static VALUE __cdecl S_DiffPrev(VALUE self);
	static VALUE __cdecl S_DiffReset(VALUE self);
	static VALUE __cdecl S_BracketPair(VALUE self);
	static VALUE __cdecl S_BookmarkSet(VALUE self);
	static VALUE __cdecl S_BookmarkNext(VALUE self);
	static VALUE __cdecl S_BookmarkPrev(VALUE self);
	static VALUE __cdecl S_BookmarkReset(VALUE self);
	static VALUE __cdecl S_BookmarkView(VALUE self);
	static VALUE __cdecl S_BookmarkPattern(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_TagJumpEx(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4);
	static VALUE __cdecl S_ChgmodINS(VALUE self);
	static VALUE __cdecl S_ChgCharSet(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_ChgmodEOL(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CancelMode(VALUE self);
	static VALUE __cdecl S_ExecExternalMacro(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_ShowToolbar(VALUE self);
	static VALUE __cdecl S_ShowFunckey(VALUE self);
	static VALUE __cdecl S_ShowTab(VALUE self);
	static VALUE __cdecl S_ShowStatusbar(VALUE self);
	static VALUE __cdecl S_TypeList(VALUE self);
	static VALUE __cdecl S_ChangeType(VALUE self, VALUE arg1);
	static VALUE __cdecl S_OptionType(VALUE self);
	static VALUE __cdecl S_OptionCommon(VALUE self);
	static VALUE __cdecl S_SelectFont(VALUE self);
	static VALUE __cdecl S_SetFontSize(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_WrapWindowWidth(VALUE self);
	static VALUE __cdecl S_OptionFavorite(VALUE self);
	static VALUE __cdecl S_SetMsgQuoteStr(VALUE self, VALUE arg1);
	static VALUE __cdecl S_TextWrapMethod(VALUE self, VALUE arg1);
	static VALUE __cdecl S_SelectCountMode(VALUE self, VALUE arg1);
	static VALUE __cdecl S_ExecCommand(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_ExecCommandDialog(VALUE self);
	static VALUE __cdecl S_RMenu(VALUE self);
	static VALUE __cdecl S_CustMenu1(VALUE self);
	static VALUE __cdecl S_CustMenu2(VALUE self);
	static VALUE __cdecl S_CustMenu3(VALUE self);
	static VALUE __cdecl S_CustMenu4(VALUE self);
	static VALUE __cdecl S_CustMenu5(VALUE self);
	static VALUE __cdecl S_CustMenu6(VALUE self);
	static VALUE __cdecl S_CustMenu7(VALUE self);
	static VALUE __cdecl S_CustMenu8(VALUE self);
	static VALUE __cdecl S_CustMenu9(VALUE self);
	static VALUE __cdecl S_CustMenu10(VALUE self);
	static VALUE __cdecl S_CustMenu11(VALUE self);
	static VALUE __cdecl S_CustMenu12(VALUE self);
	static VALUE __cdecl S_CustMenu13(VALUE self);
	static VALUE __cdecl S_CustMenu14(VALUE self);
	static VALUE __cdecl S_CustMenu15(VALUE self);
	static VALUE __cdecl S_CustMenu16(VALUE self);
	static VALUE __cdecl S_CustMenu17(VALUE self);
	static VALUE __cdecl S_CustMenu18(VALUE self);
	static VALUE __cdecl S_CustMenu19(VALUE self);
	static VALUE __cdecl S_CustMenu20(VALUE self);
	static VALUE __cdecl S_CustMenu21(VALUE self);
	static VALUE __cdecl S_CustMenu22(VALUE self);
	static VALUE __cdecl S_CustMenu23(VALUE self);
	static VALUE __cdecl S_CustMenu24(VALUE self);
	static VALUE __cdecl S_SplitWinV(VALUE self);
	static VALUE __cdecl S_SplitWinH(VALUE self);
	static VALUE __cdecl S_SplitWinVH(VALUE self);
	static VALUE __cdecl S_WinClose(VALUE self);
	static VALUE __cdecl S_WinCloseAll(VALUE self);
	static VALUE __cdecl S_CascadeWin(VALUE self);
	static VALUE __cdecl S_TileWinV(VALUE self);
	static VALUE __cdecl S_TileWinH(VALUE self);
	static VALUE __cdecl S_NextWindow(VALUE self);
	static VALUE __cdecl S_PrevWindow(VALUE self);
	static VALUE __cdecl S_WindowList(VALUE self);
	static VALUE __cdecl S_MaximizeV(VALUE self);
	static VALUE __cdecl S_MaximizeH(VALUE self);
	static VALUE __cdecl S_MinimizeAll(VALUE self);
	static VALUE __cdecl S_ReDraw(VALUE self);
	static VALUE __cdecl S_ActivateWinOutput(VALUE self);
	static VALUE __cdecl S_TraceOut(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_WindowTopMost(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GroupClose(VALUE self);
	static VALUE __cdecl S_NextGroup(VALUE self);
	static VALUE __cdecl S_PrevGroup(VALUE self);
	static VALUE __cdecl S_TabMoveRight(VALUE self);
	static VALUE __cdecl S_TabMoveLeft(VALUE self);
	static VALUE __cdecl S_TabSeparate(VALUE self);
	static VALUE __cdecl S_TabJointNext(VALUE self);
	static VALUE __cdecl S_TabJointPrev(VALUE self);
	static VALUE __cdecl S_TabCloseOther(VALUE self);
	static VALUE __cdecl S_TabCloseLeft(VALUE self);
	static VALUE __cdecl S_TabCloseRight(VALUE self);
	static VALUE __cdecl S_Complete(VALUE self);
	static VALUE __cdecl S_ToggleKeyHelpSearch(VALUE self, VALUE arg1);
	static VALUE __cdecl S_HelpContents(VALUE self);
	static VALUE __cdecl S_HelpSearch(VALUE self);
	static VALUE __cdecl S_CommandList(VALUE self);
	static VALUE __cdecl S_ExtHelp1(VALUE self);
	static VALUE __cdecl S_ExtHtmlHelp(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_About(VALUE self);
	static VALUE __cdecl S_StatusMsg(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_MsgBeep(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CommitUndoBuffer(VALUE self);
	static VALUE __cdecl S_AddRefUndoBuffer(VALUE self);
	static VALUE __cdecl S_SetUndoBuffer(VALUE self);
	static VALUE __cdecl S_AppendUndoBufferCursor(VALUE self);
	static VALUE __cdecl S_ClipboardEmpty(VALUE self);
	static VALUE __cdecl S_GetFilename(VALUE self);
	static VALUE __cdecl S_GetSaveFilename(VALUE self);
	static VALUE __cdecl S_GetSelectedString(VALUE self, VALUE arg1);
	static VALUE __cdecl S_ExpandParameter(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GetLineStr(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GetLineCount(VALUE self, VALUE arg1);
	static VALUE __cdecl S_ChangeTabWidth(VALUE self, VALUE arg1);
	static VALUE __cdecl S_IsTextSelected(VALUE self);
	static VALUE __cdecl S_GetSelectLineFrom(VALUE self);
	static VALUE __cdecl S_GetSelectColmFrom(VALUE self);
	static VALUE __cdecl S_GetSelectColumnFrom(VALUE self);
	static VALUE __cdecl S_GetSelectLineTo(VALUE self);
	static VALUE __cdecl S_GetSelectColmTo(VALUE self);
	static VALUE __cdecl S_GetSelectColumnTo(VALUE self);
	static VALUE __cdecl S_IsInsMode(VALUE self);
	static VALUE __cdecl S_GetCharCode(VALUE self);
	static VALUE __cdecl S_GetLineCode(VALUE self);
	static VALUE __cdecl S_IsPossibleUndo(VALUE self);
	static VALUE __cdecl S_IsPossibleRedo(VALUE self);
	static VALUE __cdecl S_ChangeWrapColm(VALUE self, VALUE arg1);
	static VALUE __cdecl S_ChangeWrapColumn(VALUE self, VALUE arg1);
	static VALUE __cdecl S_IsCurTypeExt(VALUE self, VALUE arg1);
	static VALUE __cdecl S_IsSameTypeExt(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_InputBox(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_MessageBox(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_ErrorMsg(VALUE self, VALUE arg1);
	static VALUE __cdecl S_WarnMsg(VALUE self, VALUE arg1);
	static VALUE __cdecl S_InfoMsg(VALUE self, VALUE arg1);
	static VALUE __cdecl S_OkCancelBox(VALUE self, VALUE arg1);
	static VALUE __cdecl S_YesNoBox(VALUE self, VALUE arg1);
	static VALUE __cdecl S_CompareVersion(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_Sleep(VALUE self, VALUE arg1);
	static VALUE __cdecl S_FileOpenDialog(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_FileSaveDialog(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_FolderDialog(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetClipboard(VALUE self, VALUE arg1);
	static VALUE __cdecl S_SetClipboard(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_LayoutToLogicLineNum(VALUE self, VALUE arg1);
	static VALUE __cdecl S_LogicToLayoutLineNum(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_LineColumnToIndex(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_LineIndexToColumn(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetCookie(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetCookieDefault(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_SetCookie(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_DeleteCookie(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetCookieNames(VALUE self, VALUE arg1);
	static VALUE __cdecl S_SetDrawSwitch(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GetDrawSwitch(VALUE self);
	static VALUE __cdecl S_IsShownStatus(VALUE self);
	static VALUE __cdecl S_GetStrWidth(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetStrLayoutLength(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl S_GetDefaultCharLength(VALUE self);
	static VALUE __cdecl S_IsIncludeClipboardFormat(VALUE self, VALUE arg1);
	static VALUE __cdecl S_GetClipboardByFormat(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl S_SetClipboardByFormat(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4);
};

#endif	//_CRUBY_EDITOR_HANDLER_H_
