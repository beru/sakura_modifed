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

#include "StdAfx.h"
#include "CPluginService.h"
#include "CRubyEditorHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyEditorHandler::CRubyEditorHandler()
{
}
///////////////////////////////////////////////////////////////////////////////
CRubyEditorHandler::~CRubyEditorHandler()
{
}
///////////////////////////////////////////////////////////////////////////////
VALUE CRubyEditorHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}
///////////////////////////////////////////////////////////////////////////////
VALUE CRubyEditorHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyEditorHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE Editor = Ruby.rb_define_class("Editor", Ruby.rb_rcObject());

	//Commands
	Ruby.rb_define_singleton_method(Editor, "FileNew", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileNew), 0);
	Ruby.rb_define_singleton_method(Editor, "FileOpen", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileOpen), 4);
	Ruby.rb_define_singleton_method(Editor, "FileSave", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileSave), 0);
	Ruby.rb_define_singleton_method(Editor, "FileSaveAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileSaveAll), 0);
	Ruby.rb_define_singleton_method(Editor, "FileSaveAsDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileSaveAsDialog), 3);
	Ruby.rb_define_singleton_method(Editor, "FileSaveAs", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileSaveAs), 3);
	Ruby.rb_define_singleton_method(Editor, "FileClose", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileClose), 0);
	Ruby.rb_define_singleton_method(Editor, "FileCloseOpen", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileCloseOpen), 0);
	Ruby.rb_define_singleton_method(Editor, "FileReopen", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopen), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenSJIS), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenJIS), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenEUC", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenEUC), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenLatin1", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenLatin1), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenUNICODE", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenUNICODE), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenUNICODEBE", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenUNICODEBE), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenUTF8", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenUTF8), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenCESU8", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenCESU8), 1);
	Ruby.rb_define_singleton_method(Editor, "FileReopenUTF7", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileReopenUTF7), 1);
	Ruby.rb_define_singleton_method(Editor, "Print", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Print), 0);
	Ruby.rb_define_singleton_method(Editor, "PrintPreview", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PrintPreview), 0);
	Ruby.rb_define_singleton_method(Editor, "PrintPageSetup", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PrintPageSetup), 0);
	Ruby.rb_define_singleton_method(Editor, "OpenHfromtoC", reinterpret_cast<VALUE(__cdecl *)(...)>(S_OpenHfromtoC), 0);
	Ruby.rb_define_singleton_method(Editor, "ActivateSQLPLUS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ActivateSQLPLUS), 0);
	Ruby.rb_define_singleton_method(Editor, "ExecSQLPLUS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExecSQLPLUS), 0);
	Ruby.rb_define_singleton_method(Editor, "Browse", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Browse), 0);
	Ruby.rb_define_singleton_method(Editor, "ViewMode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ViewMode), 0);
	Ruby.rb_define_singleton_method(Editor, "ReadOnly", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ReadOnly), 0);
	Ruby.rb_define_singleton_method(Editor, "PropertyFile", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PropertyFile), 0);
	Ruby.rb_define_singleton_method(Editor, "ExitAllEditors", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExitAllEditors), 0);
	Ruby.rb_define_singleton_method(Editor, "ExitAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExitAll), 0);
	Ruby.rb_define_singleton_method(Editor, "PutFile", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PutFile), 3);
	Ruby.rb_define_singleton_method(Editor, "InsFile", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InsFile), 3);
	Ruby.rb_define_singleton_method(Editor, "Char", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Char), 1);
	Ruby.rb_define_singleton_method(Editor, "CharIme", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CharIme), 1);
	Ruby.rb_define_singleton_method(Editor, "Undo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Undo), 0);
	Ruby.rb_define_singleton_method(Editor, "Redo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Redo), 0);
	Ruby.rb_define_singleton_method(Editor, "Delete", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Delete), 0);
	Ruby.rb_define_singleton_method(Editor, "DeleteBack", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DeleteBack), 0);
	Ruby.rb_define_singleton_method(Editor, "WordDeleteToStart", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordDeleteToStart), 0);
	Ruby.rb_define_singleton_method(Editor, "WordDeleteToEnd", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordDeleteToEnd), 0);
	Ruby.rb_define_singleton_method(Editor, "WordCut", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordCut), 0);
	Ruby.rb_define_singleton_method(Editor, "WordDelete", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordDelete), 0);
	Ruby.rb_define_singleton_method(Editor, "LineCutToStart", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineCutToStart), 0);
	Ruby.rb_define_singleton_method(Editor, "LineCutToEnd", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineCutToEnd), 0);
	Ruby.rb_define_singleton_method(Editor, "LineDeleteToStart", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineDeleteToStart), 0);
	Ruby.rb_define_singleton_method(Editor, "LineDeleteToEnd", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineDeleteToEnd), 0);
	Ruby.rb_define_singleton_method(Editor, "CutLine", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CutLine), 0);
	Ruby.rb_define_singleton_method(Editor, "DeleteLine", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DeleteLine), 0);
	Ruby.rb_define_singleton_method(Editor, "DuplicateLine", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DuplicateLine), 0);
	Ruby.rb_define_singleton_method(Editor, "IndentTab", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IndentTab), 0);
	Ruby.rb_define_singleton_method(Editor, "UnindentTab", reinterpret_cast<VALUE(__cdecl *)(...)>(S_UnindentTab), 0);
	Ruby.rb_define_singleton_method(Editor, "IndentSpace", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IndentSpace), 0);
	Ruby.rb_define_singleton_method(Editor, "UnindentSpace", reinterpret_cast<VALUE(__cdecl *)(...)>(S_UnindentSpace), 0);
	Ruby.rb_define_singleton_method(Editor, "LTrim", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LTrim), 0);
	Ruby.rb_define_singleton_method(Editor, "RTrim", reinterpret_cast<VALUE(__cdecl *)(...)>(S_RTrim), 0);
	Ruby.rb_define_singleton_method(Editor, "SortAsc", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SortAsc), 0);
	Ruby.rb_define_singleton_method(Editor, "SortDesc", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SortDesc), 0);
	Ruby.rb_define_singleton_method(Editor, "Merge", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Merge), 0);
	Ruby.rb_define_singleton_method(Editor, "Up", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up), 0);
	Ruby.rb_define_singleton_method(Editor, "Down", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down), 0);
	Ruby.rb_define_singleton_method(Editor, "Left", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Left), 0);
	Ruby.rb_define_singleton_method(Editor, "Right", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Right), 0);
	Ruby.rb_define_singleton_method(Editor, "Up2", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up2), 0);
	Ruby.rb_define_singleton_method(Editor, "Down2", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down2), 0);
	Ruby.rb_define_singleton_method(Editor, "WordLeft", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordLeft), 0);
	Ruby.rb_define_singleton_method(Editor, "WordRight", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordRight), 0);
	Ruby.rb_define_singleton_method(Editor, "GoLineTop", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineTop), 1);
	Ruby.rb_define_singleton_method(Editor, "GoLineEnd", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineEnd), 1);
	Ruby.rb_define_singleton_method(Editor, "HalfPageUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageUp), 0);
	Ruby.rb_define_singleton_method(Editor, "HalfPageDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageDown), 0);
	Ruby.rb_define_singleton_method(Editor, "PageUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageUp), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp), 0);
	Ruby.rb_define_singleton_method(Editor, "PageDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageDown), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileTop", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileTop), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileEnd", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileEnd), 0);
	Ruby.rb_define_singleton_method(Editor, "CurLineCenter", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CurLineCenter), 0);
	Ruby.rb_define_singleton_method(Editor, "MoveHistPrev", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MoveHistPrev), 0);
	Ruby.rb_define_singleton_method(Editor, "MoveHistNext", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MoveHistNext), 0);
	Ruby.rb_define_singleton_method(Editor, "MoveHistSet", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MoveHistSet), 0);
	Ruby.rb_define_singleton_method(Editor, "F_WndScrollDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_F_WndScrollDown), 0);
	Ruby.rb_define_singleton_method(Editor, "F_WndScrollUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_F_WndScrollUp), 0);
	Ruby.rb_define_singleton_method(Editor, "GoNextParagraph", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoNextParagraph), 0);
	Ruby.rb_define_singleton_method(Editor, "GoPrevParagraph", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoPrevParagraph), 0);
	Ruby.rb_define_singleton_method(Editor, "MoveCursor", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MoveCursor), 3);
	Ruby.rb_define_singleton_method(Editor, "MoveCursorLayout", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MoveCursorLayout), 3);
	Ruby.rb_define_singleton_method(Editor, "WheelUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelUp), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelDown), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelLeft", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelLeft), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelRight", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelRight), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelPageUp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelPageUp), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelPageDown", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelPageDown), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelPageLeft", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelPageLeft), 1);
	Ruby.rb_define_singleton_method(Editor, "WheelPageRight", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WheelPageRight), 1);
	Ruby.rb_define_singleton_method(Editor, "SelectWord", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SelectWord), 0);
	Ruby.rb_define_singleton_method(Editor, "SelectAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SelectAll), 0);
	Ruby.rb_define_singleton_method(Editor, "SelectLine", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SelectLine), 1);
	Ruby.rb_define_singleton_method(Editor, "BeginSelect", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BeginSelect), 0);
	Ruby.rb_define_singleton_method(Editor, "Up_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "Down_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "Left_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Left_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "Right_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Right_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "Up2_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up2_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "Down2_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down2_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "WordLeft_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordLeft_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "WordRight_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordRight_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoLineTop_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineTop_Sel), 1);
	Ruby.rb_define_singleton_method(Editor, "GoLineEnd_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineEnd_Sel), 1);
	Ruby.rb_define_singleton_method(Editor, "HalfPageUp_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageUp_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "HalfPageDown_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageDown_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "PageUp_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageUp_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageUp_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp_Sel), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageUp_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "PageDown_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageDown_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageDown_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown_Sel), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageDown_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileTop_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileTop_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileEnd_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileEnd_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoNextParagraph_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoNextParagraph_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoPrevParagraph_Sel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoPrevParagraph_Sel), 0);
	Ruby.rb_define_singleton_method(Editor, "BeginBoxSelect", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BeginBoxSelect), 0);
	Ruby.rb_define_singleton_method(Editor, "Up_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Down_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Left_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Left_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Right_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Right_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Up2_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Up2_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Down2_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Down2_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "WordLeft_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordLeft_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "WordRight_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WordRight_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoLogicalLineTop_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLogicalLineTop_BoxSel), 1);
	Ruby.rb_define_singleton_method(Editor, "GoLineTop_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineTop_BoxSel), 1);
	Ruby.rb_define_singleton_method(Editor, "GoLineEnd_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoLineEnd_BoxSel), 1);
	Ruby.rb_define_singleton_method(Editor, "HalfPageUp_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageUp_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "HalfPageDown_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HalfPageDown_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "PageUp_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageUp_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageUp_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp_BoxSel), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageUp_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageUp_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "PageDown_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PageDown_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "OnePageDown_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown_BoxSel), 0);
		Ruby.rb_define_singleton_method(Editor, "S_1PageDown_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_1PageDown_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileTop_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileTop_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "GoFileEnd_BoxSel", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GoFileEnd_BoxSel), 0);
	Ruby.rb_define_singleton_method(Editor, "Cut", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Cut), 0);
	Ruby.rb_define_singleton_method(Editor, "Copy", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Copy), 0);
	Ruby.rb_define_singleton_method(Editor, "Paste", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Paste), 1);
	Ruby.rb_define_singleton_method(Editor, "CopyAddCRLF", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyAddCRLF), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyCRLF", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyCRLF), 0);
	Ruby.rb_define_singleton_method(Editor, "PasteBox", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PasteBox), 1);
	Ruby.rb_define_singleton_method(Editor, "InsBoxText", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InsBoxText), 1);
	Ruby.rb_define_singleton_method(Editor, "InsText", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InsText), 1);
	Ruby.rb_define_singleton_method(Editor, "AddTail", reinterpret_cast<VALUE(__cdecl *)(...)>(S_AddTail), 1);
	Ruby.rb_define_singleton_method(Editor, "CopyLines", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyLines), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyLinesAsPassage", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyLinesAsPassage), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyLinesWithLineNumber", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyLinesWithLineNumber), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyColorHtml", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyColorHtml), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyColorHtmlWithLineNumber", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyColorHtmlWithLineNumber), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyPath", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyPath), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyFilename", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyFilename), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyTag", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyTag), 0);
	Ruby.rb_define_singleton_method(Editor, "CopyKeyBindList", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CopyKeyBindList), 0);
	Ruby.rb_define_singleton_method(Editor, "InsertDate", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InsertDate), 0);
	Ruby.rb_define_singleton_method(Editor, "InsertTime", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InsertTime), 0);
	Ruby.rb_define_singleton_method(Editor, "CtrlCodeDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CtrlCodeDialog), 0);
	Ruby.rb_define_singleton_method(Editor, "CtrlCode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CtrlCode), 1);
	Ruby.rb_define_singleton_method(Editor, "ToLower", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToLower), 0);
	Ruby.rb_define_singleton_method(Editor, "ToUpper", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToUpper), 0);
	Ruby.rb_define_singleton_method(Editor, "ToHankaku", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToHankaku), 0);
	Ruby.rb_define_singleton_method(Editor, "ToHankata", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToHankata), 0);
	Ruby.rb_define_singleton_method(Editor, "ToZenEi", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToZenEi), 0);
	Ruby.rb_define_singleton_method(Editor, "ToHanEi", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToHanEi), 0);
	Ruby.rb_define_singleton_method(Editor, "ToZenKata", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToZenKata), 0);
	Ruby.rb_define_singleton_method(Editor, "ToZenHira", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToZenHira), 0);
	Ruby.rb_define_singleton_method(Editor, "HanKataToZenKata", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HanKataToZenKata), 0);
	Ruby.rb_define_singleton_method(Editor, "HanKataToZenHira", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HanKataToZenHira), 0);
	Ruby.rb_define_singleton_method(Editor, "TABToSPACE", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TABToSPACE), 0);
	Ruby.rb_define_singleton_method(Editor, "SPACEToTAB", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SPACEToTAB), 0);
	Ruby.rb_define_singleton_method(Editor, "AutoToSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_AutoToSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "JIStoSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_JIStoSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "EUCtoSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_EUCtoSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "CodeCnvUNICODEtoSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CodeCnvUNICODEtoSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "CodeCnvUNICODEBEtoSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CodeCnvUNICODEBEtoSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "UTF8toSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_UTF8toSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "UTF7toSJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_UTF7toSJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "SJIStoJIS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SJIStoJIS), 0);
	Ruby.rb_define_singleton_method(Editor, "SJIStoEUC", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SJIStoEUC), 0);
	Ruby.rb_define_singleton_method(Editor, "SJIStoUTF8", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SJIStoUTF8), 0);
	Ruby.rb_define_singleton_method(Editor, "SJIStoUTF7", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SJIStoUTF7), 0);
	Ruby.rb_define_singleton_method(Editor, "Base64Decode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Base64Decode), 0);
	Ruby.rb_define_singleton_method(Editor, "Uudecode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Uudecode), 0);
	Ruby.rb_define_singleton_method(Editor, "SearchDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SearchDialog), 0);
	Ruby.rb_define_singleton_method(Editor, "SearchNext", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SearchNext), 2);
	Ruby.rb_define_singleton_method(Editor, "SearchPrev", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SearchPrev), 2);
	Ruby.rb_define_singleton_method(Editor, "ReplaceDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ReplaceDialog), 0);
	Ruby.rb_define_singleton_method(Editor, "Replace", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Replace), 3);
	Ruby.rb_define_singleton_method(Editor, "ReplaceAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ReplaceAll), 3);
	Ruby.rb_define_singleton_method(Editor, "SearchClearMark", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SearchClearMark), 0);
	Ruby.rb_define_singleton_method(Editor, "SearchStartPos", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SearchStartPos), 0);
	Ruby.rb_define_singleton_method(Editor, "Grep", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Grep), 5);
	Ruby.rb_define_singleton_method(Editor, "Jump", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Jump), 2);
	Ruby.rb_define_singleton_method(Editor, "Outline", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Outline), 1);
	Ruby.rb_define_singleton_method(Editor, "TagJump", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TagJump), 0);
	Ruby.rb_define_singleton_method(Editor, "TagJumpBack", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TagJumpBack), 0);
	Ruby.rb_define_singleton_method(Editor, "TagMake", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TagMake), 0);
	Ruby.rb_define_singleton_method(Editor, "DirectTagJump", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DirectTagJump), 0);
	Ruby.rb_define_singleton_method(Editor, "KeywordTagJump", reinterpret_cast<VALUE(__cdecl *)(...)>(S_KeywordTagJump), 1);
	Ruby.rb_define_singleton_method(Editor, "Compare", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Compare), 0);
	Ruby.rb_define_singleton_method(Editor, "DiffDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DiffDialog), 0);
	Ruby.rb_define_singleton_method(Editor, "Diff", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Diff), 2);
	Ruby.rb_define_singleton_method(Editor, "DiffNext", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DiffNext), 0);
	Ruby.rb_define_singleton_method(Editor, "DiffPrev", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DiffPrev), 0);
	Ruby.rb_define_singleton_method(Editor, "DiffReset", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DiffReset), 0);
	Ruby.rb_define_singleton_method(Editor, "BracketPair", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BracketPair), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkSet", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkSet), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkNext", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkNext), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkPrev", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkPrev), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkReset", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkReset), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkView", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkView), 0);
	Ruby.rb_define_singleton_method(Editor, "BookmarkPattern", reinterpret_cast<VALUE(__cdecl *)(...)>(S_BookmarkPattern), 2);
	Ruby.rb_define_singleton_method(Editor, "TagJumpEx", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TagJumpEx), 4);
	Ruby.rb_define_singleton_method(Editor, "ChgmodINS", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChgmodINS), 0);
	Ruby.rb_define_singleton_method(Editor, "ChgCharSet", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChgCharSet), 2);
	Ruby.rb_define_singleton_method(Editor, "ChgmodEOL", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChgmodEOL), 1);
	Ruby.rb_define_singleton_method(Editor, "CancelMode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CancelMode), 0);
	Ruby.rb_define_singleton_method(Editor, "ExecExternalMacro", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExecExternalMacro), 2);
	Ruby.rb_define_singleton_method(Editor, "ShowToolbar", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ShowToolbar), 0);
	Ruby.rb_define_singleton_method(Editor, "ShowFunckey", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ShowFunckey), 0);
	Ruby.rb_define_singleton_method(Editor, "ShowTab", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ShowTab), 0);
	Ruby.rb_define_singleton_method(Editor, "ShowStatusbar", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ShowStatusbar), 0);
	Ruby.rb_define_singleton_method(Editor, "TypeList", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TypeList), 0);
	Ruby.rb_define_singleton_method(Editor, "ChangeType", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChangeType), 1);
	Ruby.rb_define_singleton_method(Editor, "OptionType", reinterpret_cast<VALUE(__cdecl *)(...)>(S_OptionType), 0);
	Ruby.rb_define_singleton_method(Editor, "OptionCommon", reinterpret_cast<VALUE(__cdecl *)(...)>(S_OptionCommon), 0);
	Ruby.rb_define_singleton_method(Editor, "SelectFont", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SelectFont), 0);
	Ruby.rb_define_singleton_method(Editor, "SetFontSize", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetFontSize), 3);
	Ruby.rb_define_singleton_method(Editor, "WrapWindowWidth", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WrapWindowWidth), 0);
	Ruby.rb_define_singleton_method(Editor, "OptionFavorite", reinterpret_cast<VALUE(__cdecl *)(...)>(S_OptionFavorite), 0);
	Ruby.rb_define_singleton_method(Editor, "SetMsgQuoteStr", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetMsgQuoteStr), 1);
	Ruby.rb_define_singleton_method(Editor, "TextWrapMethod", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TextWrapMethod), 1);
	Ruby.rb_define_singleton_method(Editor, "SelectCountMode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SelectCountMode), 1);
	Ruby.rb_define_singleton_method(Editor, "ExecCommand", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExecCommand), 3);
	Ruby.rb_define_singleton_method(Editor, "ExecCommandDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExecCommandDialog), 0);
	Ruby.rb_define_singleton_method(Editor, "RMenu", reinterpret_cast<VALUE(__cdecl *)(...)>(S_RMenu), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu1", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu1), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu2", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu2), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu3", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu3), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu4", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu4), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu5", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu5), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu6", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu6), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu7", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu7), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu8", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu8), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu9", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu9), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu10", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu10), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu11", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu11), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu12", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu12), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu13", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu13), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu14", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu14), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu15", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu15), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu16", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu16), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu17", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu17), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu18", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu18), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu19", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu19), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu20", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu20), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu21", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu21), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu22", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu22), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu23", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu23), 0);
	Ruby.rb_define_singleton_method(Editor, "CustMenu24", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CustMenu24), 0);
	Ruby.rb_define_singleton_method(Editor, "SplitWinV", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SplitWinV), 0);
	Ruby.rb_define_singleton_method(Editor, "SplitWinH", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SplitWinH), 0);
	Ruby.rb_define_singleton_method(Editor, "SplitWinVH", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SplitWinVH), 0);
	Ruby.rb_define_singleton_method(Editor, "WinClose", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WinClose), 0);
	Ruby.rb_define_singleton_method(Editor, "WinCloseAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WinCloseAll), 0);
	Ruby.rb_define_singleton_method(Editor, "CascadeWin", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CascadeWin), 0);
	Ruby.rb_define_singleton_method(Editor, "TileWinV", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TileWinV), 0);
	Ruby.rb_define_singleton_method(Editor, "TileWinH", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TileWinH), 0);
	Ruby.rb_define_singleton_method(Editor, "NextWindow", reinterpret_cast<VALUE(__cdecl *)(...)>(S_NextWindow), 0);
	Ruby.rb_define_singleton_method(Editor, "PrevWindow", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PrevWindow), 0);
	Ruby.rb_define_singleton_method(Editor, "WindowList", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WindowList), 0);
	Ruby.rb_define_singleton_method(Editor, "MaximizeV", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MaximizeV), 0);
	Ruby.rb_define_singleton_method(Editor, "MaximizeH", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MaximizeH), 0);
	Ruby.rb_define_singleton_method(Editor, "MinimizeAll", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MinimizeAll), 0);
	Ruby.rb_define_singleton_method(Editor, "ReDraw", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ReDraw), 0);
	Ruby.rb_define_singleton_method(Editor, "ActivateWinOutput", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ActivateWinOutput), 0);
	Ruby.rb_define_singleton_method(Editor, "TraceOut", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TraceOut), 2);
	Ruby.rb_define_singleton_method(Editor, "WindowTopMost", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WindowTopMost), 1);
	Ruby.rb_define_singleton_method(Editor, "GroupClose", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GroupClose), 0);
	Ruby.rb_define_singleton_method(Editor, "NextGroup", reinterpret_cast<VALUE(__cdecl *)(...)>(S_NextGroup), 0);
	Ruby.rb_define_singleton_method(Editor, "PrevGroup", reinterpret_cast<VALUE(__cdecl *)(...)>(S_PrevGroup), 0);
	Ruby.rb_define_singleton_method(Editor, "TabMoveRight", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabMoveRight), 0);
	Ruby.rb_define_singleton_method(Editor, "TabMoveLeft", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabMoveLeft), 0);
	Ruby.rb_define_singleton_method(Editor, "TabSeparate", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabSeparate), 0);
	Ruby.rb_define_singleton_method(Editor, "TabJointNext", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabJointNext), 0);
	Ruby.rb_define_singleton_method(Editor, "TabJointPrev", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabJointPrev), 0);
	Ruby.rb_define_singleton_method(Editor, "TabCloseOther", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabCloseOther), 0);
	Ruby.rb_define_singleton_method(Editor, "TabCloseLeft", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabCloseLeft), 0);
	Ruby.rb_define_singleton_method(Editor, "TabCloseRight", reinterpret_cast<VALUE(__cdecl *)(...)>(S_TabCloseRight), 0);
	Ruby.rb_define_singleton_method(Editor, "Complete", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Complete), 0);
	Ruby.rb_define_singleton_method(Editor, "ToggleKeyHelpSearch", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ToggleKeyHelpSearch), 1);
	Ruby.rb_define_singleton_method(Editor, "HelpContents", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HelpContents), 0);
	Ruby.rb_define_singleton_method(Editor, "HelpSearch", reinterpret_cast<VALUE(__cdecl *)(...)>(S_HelpSearch), 0);
	Ruby.rb_define_singleton_method(Editor, "CommandList", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CommandList), 0);
	Ruby.rb_define_singleton_method(Editor, "ExtHelp1", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExtHelp1), 0);
	Ruby.rb_define_singleton_method(Editor, "ExtHtmlHelp", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExtHtmlHelp), 2);
	Ruby.rb_define_singleton_method(Editor, "About", reinterpret_cast<VALUE(__cdecl *)(...)>(S_About), 0);
	Ruby.rb_define_singleton_method(Editor, "StatusMsg", reinterpret_cast<VALUE(__cdecl *)(...)>(S_StatusMsg), 2);
	Ruby.rb_define_singleton_method(Editor, "MsgBeep", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MsgBeep), 1);
	Ruby.rb_define_singleton_method(Editor, "CommitUndoBuffer", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CommitUndoBuffer), 0);
	Ruby.rb_define_singleton_method(Editor, "AddRefUndoBuffer", reinterpret_cast<VALUE(__cdecl *)(...)>(S_AddRefUndoBuffer), 0);
	Ruby.rb_define_singleton_method(Editor, "SetUndoBuffer", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetUndoBuffer), 0);
	Ruby.rb_define_singleton_method(Editor, "AppendUndoBufferCursor", reinterpret_cast<VALUE(__cdecl *)(...)>(S_AppendUndoBufferCursor), 0);
	Ruby.rb_define_singleton_method(Editor, "ClipboardEmpty", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ClipboardEmpty), 0);

	//Functions
	Ruby.rb_define_singleton_method(Editor, "GetFilename", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetFilename), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSaveFilename", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSaveFilename), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectedString", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectedString), 1);
	Ruby.rb_define_singleton_method(Editor, "ExpandParameter", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ExpandParameter), 1);
	Ruby.rb_define_singleton_method(Editor, "GetLineStr", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetLineStr), 1);
	Ruby.rb_define_singleton_method(Editor, "GetLineCount", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetLineCount), 1);
	Ruby.rb_define_singleton_method(Editor, "ChangeTabWidth", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChangeTabWidth), 1);
	Ruby.rb_define_singleton_method(Editor, "IsTextSelected", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsTextSelected), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectLineFrom", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectLineFrom), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectColmFrom", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectColmFrom), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectColumnFrom", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectColumnFrom), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectLineTo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectLineTo), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectColmTo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectColmTo), 0);
	Ruby.rb_define_singleton_method(Editor, "GetSelectColumnTo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetSelectColumnTo), 0);
	Ruby.rb_define_singleton_method(Editor, "IsInsMode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsInsMode), 0);
	Ruby.rb_define_singleton_method(Editor, "GetCharCode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetCharCode), 0);
	Ruby.rb_define_singleton_method(Editor, "GetLineCode", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetLineCode), 0);
	Ruby.rb_define_singleton_method(Editor, "IsPossibleUndo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsPossibleUndo), 0);
	Ruby.rb_define_singleton_method(Editor, "IsPossibleRedo", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsPossibleRedo), 0);
	Ruby.rb_define_singleton_method(Editor, "ChangeWrapColm", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChangeWrapColm), 1);
	Ruby.rb_define_singleton_method(Editor, "ChangeWrapColumn", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ChangeWrapColumn), 1);
	Ruby.rb_define_singleton_method(Editor, "IsCurTypeExt", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsCurTypeExt), 1);
	Ruby.rb_define_singleton_method(Editor, "IsSameTypeExt", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsSameTypeExt), 2);
	Ruby.rb_define_singleton_method(Editor, "InputBox", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InputBox), 3);
	Ruby.rb_define_singleton_method(Editor, "MessageBox", reinterpret_cast<VALUE(__cdecl *)(...)>(S_MessageBox), 2);
	Ruby.rb_define_singleton_method(Editor, "ErrorMsg", reinterpret_cast<VALUE(__cdecl *)(...)>(S_ErrorMsg), 1);
	Ruby.rb_define_singleton_method(Editor, "WarnMsg", reinterpret_cast<VALUE(__cdecl *)(...)>(S_WarnMsg), 1);
	Ruby.rb_define_singleton_method(Editor, "InfoMsg", reinterpret_cast<VALUE(__cdecl *)(...)>(S_InfoMsg), 1);
	Ruby.rb_define_singleton_method(Editor, "OkCancelBox", reinterpret_cast<VALUE(__cdecl *)(...)>(S_OkCancelBox), 1);
	Ruby.rb_define_singleton_method(Editor, "YesNoBox", reinterpret_cast<VALUE(__cdecl *)(...)>(S_YesNoBox), 1);
	Ruby.rb_define_singleton_method(Editor, "CompareVersion", reinterpret_cast<VALUE(__cdecl *)(...)>(S_CompareVersion), 2);
	Ruby.rb_define_singleton_method(Editor, "Sleep", reinterpret_cast<VALUE(__cdecl *)(...)>(S_Sleep), 1);
	Ruby.rb_define_singleton_method(Editor, "FileOpenDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileOpenDialog), 2);
	Ruby.rb_define_singleton_method(Editor, "FileSaveDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FileSaveDialog), 2);
	Ruby.rb_define_singleton_method(Editor, "FolderDialog", reinterpret_cast<VALUE(__cdecl *)(...)>(S_FolderDialog), 2);
	Ruby.rb_define_singleton_method(Editor, "GetClipboard", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetClipboard), 1);
	Ruby.rb_define_singleton_method(Editor, "SetClipboard", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetClipboard), 2);
	Ruby.rb_define_singleton_method(Editor, "LayoutToLogicLineNum", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LayoutToLogicLineNum), 1);
	Ruby.rb_define_singleton_method(Editor, "LogicToLayoutLineNum", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LogicToLayoutLineNum), 2);
	Ruby.rb_define_singleton_method(Editor, "LineColumnToIndex", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineColumnToIndex), 2);
	Ruby.rb_define_singleton_method(Editor, "LineIndexToColumn", reinterpret_cast<VALUE(__cdecl *)(...)>(S_LineIndexToColumn), 2);
	Ruby.rb_define_singleton_method(Editor, "GetCookie", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetCookie), 2);
	Ruby.rb_define_singleton_method(Editor, "GetCookieDefault", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetCookieDefault), 3);
	Ruby.rb_define_singleton_method(Editor, "SetCookie", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetCookie), 3);
	Ruby.rb_define_singleton_method(Editor, "DeleteCookie", reinterpret_cast<VALUE(__cdecl *)(...)>(S_DeleteCookie), 2);
	Ruby.rb_define_singleton_method(Editor, "GetCookieNames", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetCookieNames), 1);
	Ruby.rb_define_singleton_method(Editor, "SetDrawSwitch", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetDrawSwitch), 1);
	Ruby.rb_define_singleton_method(Editor, "GetDrawSwitch", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetDrawSwitch), 0);
	Ruby.rb_define_singleton_method(Editor, "IsShownStatus", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsShownStatus), 0);
	Ruby.rb_define_singleton_method(Editor, "GetStrWidth", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetStrWidth), 2);
	Ruby.rb_define_singleton_method(Editor, "GetStrLayoutLength", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetStrLayoutLength), 2);
	Ruby.rb_define_singleton_method(Editor, "GetDefaultCharLength", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetDefaultCharLength), 0);
	Ruby.rb_define_singleton_method(Editor, "IsIncludeClipboardFormat", reinterpret_cast<VALUE(__cdecl *)(...)>(S_IsIncludeClipboardFormat), 1);
	Ruby.rb_define_singleton_method(Editor, "GetClipboardByFormat", reinterpret_cast<VALUE(__cdecl *)(...)>(S_GetClipboardByFormat), 3);
	Ruby.rb_define_singleton_method(Editor, "SetClipboardByFormat", reinterpret_cast<VALUE(__cdecl *)(...)>(S_SetClipboardByFormat), 4);

	return Editor;
}

///////////////////////////////////////////////////////////////////////////////
//Editor
VALUE __cdecl CRubyEditorHandler::S_FileNew(VALUE self){
	return CommandHandler(F_FILENEW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_FileOpen(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4){
	VALUE args[4] = { arg1, arg2, arg3, arg4 };
	return CommandHandler(F_FILEOPEN2, self, args, 4);
}

VALUE __cdecl CRubyEditorHandler::S_FileSave(VALUE self){
	return CommandHandler(F_FILESAVE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_FileSaveAll(VALUE self){
	return CommandHandler(F_FILESAVEALL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_FileSaveAsDialog(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_FILESAVEAS_DIALOG, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_FileSaveAs(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_FILESAVEAS, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_FileClose(VALUE self){
	return CommandHandler(F_FILECLOSE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_FileCloseOpen(VALUE self){
	return CommandHandler(F_FILECLOSE_OPEN, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopen(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenSJIS(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_SJIS, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenJIS(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_JIS, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenEUC(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_EUC, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenLatin1(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_LATIN1, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenUNICODE(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_UNICODE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenUNICODEBE(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_UNICODEBE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenUTF8(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_UTF8, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenCESU8(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_CESU8, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileReopenUTF7(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_FILE_REOPEN_UTF7, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_Print(VALUE self){
	return CommandHandler(F_PRINT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PrintPreview(VALUE self){
	return CommandHandler(F_PRINT_PREVIEW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PrintPageSetup(VALUE self){
	return CommandHandler(F_PRINT_PAGESETUP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_OpenHfromtoC(VALUE self){
	return CommandHandler(F_OPEN_HfromtoC, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ActivateSQLPLUS(VALUE self){
	return CommandHandler(F_ACTIVATE_SQLPLUS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExecSQLPLUS(VALUE self){
	return CommandHandler(F_PLSQL_COMPILE_ON_SQLPLUS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Browse(VALUE self){
	return CommandHandler(F_BROWSE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ViewMode(VALUE self){
	return CommandHandler(F_VIEWMODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ReadOnly(VALUE self){
	return CommandHandler(F_VIEWMODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PropertyFile(VALUE self){
	return CommandHandler(F_PROPERTY_FILE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExitAllEditors(VALUE self){
	return CommandHandler(F_EXITALLEDITORS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExitAll(VALUE self){
	return CommandHandler(F_EXITALL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PutFile(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_PUTFILE, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_InsFile(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_INSFILE, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_Char(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WCHAR, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CharIme(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_IME_CHAR, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_Undo(VALUE self){
	return CommandHandler(F_UNDO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Redo(VALUE self){
	return CommandHandler(F_REDO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Delete(VALUE self){
	return CommandHandler(F_DELETE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DeleteBack(VALUE self){
	return CommandHandler(F_DELETE_BACK, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordDeleteToStart(VALUE self){
	return CommandHandler(F_WordDeleteToStart, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordDeleteToEnd(VALUE self){
	return CommandHandler(F_WordDeleteToEnd, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordCut(VALUE self){
	return CommandHandler(F_WordCut, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordDelete(VALUE self){
	return CommandHandler(F_WordDelete, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_LineCutToStart(VALUE self){
	return CommandHandler(F_LineCutToStart, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_LineCutToEnd(VALUE self){
	return CommandHandler(F_LineCutToEnd, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_LineDeleteToStart(VALUE self){
	return CommandHandler(F_LineDeleteToStart, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_LineDeleteToEnd(VALUE self){
	return CommandHandler(F_LineDeleteToEnd, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CutLine(VALUE self){
	return CommandHandler(F_CUT_LINE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DeleteLine(VALUE self){
	return CommandHandler(F_DELETE_LINE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DuplicateLine(VALUE self){
	return CommandHandler(F_DUPLICATELINE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IndentTab(VALUE self){
	return CommandHandler(F_INDENT_TAB, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_UnindentTab(VALUE self){
	return CommandHandler(F_UNINDENT_TAB, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IndentSpace(VALUE self){
	return CommandHandler(F_INDENT_SPACE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_UnindentSpace(VALUE self){
	return CommandHandler(F_UNINDENT_SPACE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_LTrim(VALUE self){
	return CommandHandler(F_LTRIM, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_RTrim(VALUE self){
	return CommandHandler(F_RTRIM, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SortAsc(VALUE self){
	return CommandHandler(F_SORT_ASC, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SortDesc(VALUE self){
	return CommandHandler(F_SORT_DESC, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Merge(VALUE self){
	return CommandHandler(F_MERGE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up(VALUE self){
	return CommandHandler(F_UP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down(VALUE self){
	return CommandHandler(F_DOWN, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Left(VALUE self){
	return CommandHandler(F_LEFT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Right(VALUE self){
	return CommandHandler(F_RIGHT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up2(VALUE self){
	return CommandHandler(F_UP2, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down2(VALUE self){
	return CommandHandler(F_DOWN2, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordLeft(VALUE self){
	return CommandHandler(F_WORDLEFT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordRight(VALUE self){
	return CommandHandler(F_WORDRIGHT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineTop(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINETOP, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineEnd(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINEEND, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageUp(VALUE self){
	return CommandHandler(F_HalfPageUp, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageDown(VALUE self){
	return CommandHandler(F_HalfPageDown, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageUp(VALUE self){
	return CommandHandler(F_1PageUp, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageUp(VALUE self){
	return CommandHandler(F_1PageUp, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageDown(VALUE self){
	return CommandHandler(F_1PageDown, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageDown(VALUE self){
	return CommandHandler(F_1PageDown, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileTop(VALUE self){
	return CommandHandler(F_GOFILETOP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileEnd(VALUE self){
	return CommandHandler(F_GOFILEEND, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CurLineCenter(VALUE self){
	return CommandHandler(F_CURLINECENTER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MoveHistPrev(VALUE self){
	return CommandHandler(F_JUMPHIST_PREV, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MoveHistNext(VALUE self){
	return CommandHandler(F_JUMPHIST_NEXT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MoveHistSet(VALUE self){
	return CommandHandler(F_JUMPHIST_SET, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_F_WndScrollDown(VALUE self){
	return CommandHandler(F_WndScrollDown, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_F_WndScrollUp(VALUE self){
	return CommandHandler(F_WndScrollUp, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoNextParagraph(VALUE self){
	return CommandHandler(F_GONEXTPARAGRAPH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoPrevParagraph(VALUE self){
	return CommandHandler(F_GOPREVPARAGRAPH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MoveCursor(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_MOVECURSOR, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_MoveCursorLayout(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_MOVECURSORLAYOUT, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_WheelUp(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEEL_FIRST, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelDown(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELDOWN, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelLeft(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELLEFT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelRight(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELRIGHT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelPageUp(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELPAGEUP, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelPageDown(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELPAGEDOWN, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelPageLeft(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELPAGELEFT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WheelPageRight(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_WHEELPAGERIGHT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_SelectWord(VALUE self){
	return CommandHandler(F_SELECTWORD, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SelectAll(VALUE self){
	return CommandHandler(F_SELECTALL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SelectLine(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_SELECTLINE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_BeginSelect(VALUE self){
	return CommandHandler(F_BEGIN_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up_Sel(VALUE self){
	return CommandHandler(F_UP_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down_Sel(VALUE self){
	return CommandHandler(F_DOWN_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Left_Sel(VALUE self){
	return CommandHandler(F_LEFT_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Right_Sel(VALUE self){
	return CommandHandler(F_RIGHT_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up2_Sel(VALUE self){
	return CommandHandler(F_UP2_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down2_Sel(VALUE self){
	return CommandHandler(F_DOWN2_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordLeft_Sel(VALUE self){
	return CommandHandler(F_WORDLEFT_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordRight_Sel(VALUE self){
	return CommandHandler(F_WORDRIGHT_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineTop_Sel(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINETOP_SEL, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineEnd_Sel(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINEEND_SEL, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageUp_Sel(VALUE self){
	return CommandHandler(F_HalfPageUp_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageDown_Sel(VALUE self){
	return CommandHandler(F_HalfPageDown_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageUp_Sel(VALUE self){
	return CommandHandler(F_1PageUp_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageUp_Sel(VALUE self){
	return CommandHandler(F_1PageUp_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageDown_Sel(VALUE self){
	return CommandHandler(F_1PageDown_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageDown_Sel(VALUE self){
	return CommandHandler(F_1PageDown_Sel, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileTop_Sel(VALUE self){
	return CommandHandler(F_GOFILETOP_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileEnd_Sel(VALUE self){
	return CommandHandler(F_GOFILEEND_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoNextParagraph_Sel(VALUE self){
	return CommandHandler(F_GONEXTPARAGRAPH_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoPrevParagraph_Sel(VALUE self){
	return CommandHandler(F_GOPREVPARAGRAPH_SEL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BeginBoxSelect(VALUE self){
	return CommandHandler(F_BEGIN_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up_BoxSel(VALUE self){
	return CommandHandler(F_UP_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down_BoxSel(VALUE self){
	return CommandHandler(F_DOWN_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Left_BoxSel(VALUE self){
	return CommandHandler(F_LEFT_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Right_BoxSel(VALUE self){
	return CommandHandler(F_RIGHT_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Up2_BoxSel(VALUE self){
	return CommandHandler(F_UP2_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Down2_BoxSel(VALUE self){
	return CommandHandler(F_DOWN2_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordLeft_BoxSel(VALUE self){
	return CommandHandler(F_WORDLEFT_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WordRight_BoxSel(VALUE self){
	return CommandHandler(F_WORDRIGHT_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoLogicalLineTop_BoxSel(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLOGICALLINETOP_BOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineTop_BoxSel(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINETOP_BOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GoLineEnd_BoxSel(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_GOLINEEND_BOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageUp_BoxSel(VALUE self){
	return CommandHandler(F_HalfPageUp_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HalfPageDown_BoxSel(VALUE self){
	return CommandHandler(F_HalfPageDown_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageUp_BoxSel(VALUE self){
	return CommandHandler(F_1PageUp_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageUp_BoxSel(VALUE self){
	return CommandHandler(F_1PageUp_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PageDown_BoxSel(VALUE self){
	return CommandHandler(F_1PageDown_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_1PageDown_BoxSel(VALUE self){
	return CommandHandler(F_1PageDown_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileTop_BoxSel(VALUE self){
	return CommandHandler(F_GOFILETOP_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GoFileEnd_BoxSel(VALUE self){
	return CommandHandler(F_GOFILEEND_BOX, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Cut(VALUE self){
	return CommandHandler(F_CUT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Copy(VALUE self){
	return CommandHandler(F_COPY, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Paste(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_PASTE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CopyAddCRLF(VALUE self){
	return CommandHandler(F_COPY_ADDCRLF, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyCRLF(VALUE self){
	return CommandHandler(F_COPY_CRLF, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PasteBox(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_PASTEBOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_InsBoxText(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_INSBOXTEXT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_InsText(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_INSTEXT_W, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_AddTail(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_ADDTAIL_W, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CopyLines(VALUE self){
	return CommandHandler(F_COPYLINES, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyLinesAsPassage(VALUE self){
	return CommandHandler(F_COPYLINESASPASSAGE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyLinesWithLineNumber(VALUE self){
	return CommandHandler(F_COPYLINESWITHLINENUMBER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyColorHtml(VALUE self){
	return CommandHandler(F_COPY_COLOR_HTML, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyColorHtmlWithLineNumber(VALUE self){
	return CommandHandler(F_COPY_COLOR_HTML_LINENUMBER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyPath(VALUE self){
	return CommandHandler(F_COPYPATH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyFilename(VALUE self){
	return CommandHandler(F_COPYFNAME, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyTag(VALUE self){
	return CommandHandler(F_COPYTAG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CopyKeyBindList(VALUE self){
	return CommandHandler(F_CREATEKEYBINDLIST, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_InsertDate(VALUE self){
	return CommandHandler(F_INS_DATE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_InsertTime(VALUE self){
	return CommandHandler(F_INS_TIME, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CtrlCodeDialog(VALUE self){
	return CommandHandler(F_CTRL_CODE_DIALOG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CtrlCode(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_CTRL_CODE_DIALOG, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_ToLower(VALUE self){
	return CommandHandler(F_TOLOWER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToUpper(VALUE self){
	return CommandHandler(F_TOUPPER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToHankaku(VALUE self){
	return CommandHandler(F_TOHANKAKU, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToHankata(VALUE self){
	return CommandHandler(F_TOHANKATA, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToZenEi(VALUE self){
	return CommandHandler(F_TOZENEI, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToHanEi(VALUE self){
	return CommandHandler(F_TOHANEI, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToZenKata(VALUE self){
	return CommandHandler(F_TOZENKAKUKATA, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToZenHira(VALUE self){
	return CommandHandler(F_TOZENKAKUHIRA, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HanKataToZenKata(VALUE self){
	return CommandHandler(F_HANKATATOZENKATA, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HanKataToZenHira(VALUE self){
	return CommandHandler(F_HANKATATOZENHIRA, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TABToSPACE(VALUE self){
	return CommandHandler(F_TABTOSPACE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SPACEToTAB(VALUE self){
	return CommandHandler(F_SPACETOTAB, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_AutoToSJIS(VALUE self){
	return CommandHandler(F_CODECNV_AUTO2SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_JIStoSJIS(VALUE self){
	return CommandHandler(F_CODECNV_EMAIL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_EUCtoSJIS(VALUE self){
	return CommandHandler(F_CODECNV_EUC2SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CodeCnvUNICODEtoSJIS(VALUE self){
	return CommandHandler(F_CODECNV_UNICODE2SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CodeCnvUNICODEBEtoSJIS(VALUE self){
	return CommandHandler(F_CODECNV_UNICODEBE2SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_UTF8toSJIS(VALUE self){
	return CommandHandler(F_CODECNV_UTF82SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_UTF7toSJIS(VALUE self){
	return CommandHandler(F_CODECNV_UTF72SJIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SJIStoJIS(VALUE self){
	return CommandHandler(F_CODECNV_SJIS2JIS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SJIStoEUC(VALUE self){
	return CommandHandler(F_CODECNV_SJIS2EUC, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SJIStoUTF8(VALUE self){
	return CommandHandler(F_CODECNV_SJIS2UTF8, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SJIStoUTF7(VALUE self){
	return CommandHandler(F_CODECNV_SJIS2UTF7, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Base64Decode(VALUE self){
	return CommandHandler(F_BASE64DECODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Uudecode(VALUE self){
	return CommandHandler(F_UUDECODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SearchDialog(VALUE self){
	return CommandHandler(F_SEARCH_DIALOG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SearchNext(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_SEARCH_NEXT, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_SearchPrev(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_SEARCH_PREV, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_ReplaceDialog(VALUE self){
	return CommandHandler(F_REPLACE_DIALOG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Replace(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_REPLACE, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_ReplaceAll(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_REPLACE_ALL, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_SearchClearMark(VALUE self){
	return CommandHandler(F_SEARCH_CLEARMARK, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SearchStartPos(VALUE self){
	return CommandHandler(F_JUMP_SRCHSTARTPOS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Grep(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4, VALUE arg5){
	VALUE args[5] = { arg1, arg2, arg3, arg4, arg5 };
	return CommandHandler(F_GREP, self, args, 5);
}

VALUE __cdecl CRubyEditorHandler::S_Jump(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_JUMP, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_Outline(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_OUTLINE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_TagJump(VALUE self){
	return CommandHandler(F_TAGJUMP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TagJumpBack(VALUE self){
	return CommandHandler(F_TAGJUMPBACK, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TagMake(VALUE self){
	return CommandHandler(F_TAGS_MAKE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DirectTagJump(VALUE self){
	return CommandHandler(F_DIRECT_TAGJUMP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_KeywordTagJump(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_TAGJUMP_KEYWORD, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_Compare(VALUE self){
	return CommandHandler(F_COMPARE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DiffDialog(VALUE self){
	return CommandHandler(F_DIFF_DIALOG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Diff(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_DIFF, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_DiffNext(VALUE self){
	return CommandHandler(F_DIFF_NEXT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DiffPrev(VALUE self){
	return CommandHandler(F_DIFF_PREV, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_DiffReset(VALUE self){
	return CommandHandler(F_DIFF_RESET, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BracketPair(VALUE self){
	return CommandHandler(F_BRACKETPAIR, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkSet(VALUE self){
	return CommandHandler(F_BOOKMARK_SET, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkNext(VALUE self){
	return CommandHandler(F_BOOKMARK_NEXT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkPrev(VALUE self){
	return CommandHandler(F_BOOKMARK_PREV, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkReset(VALUE self){
	return CommandHandler(F_BOOKMARK_RESET, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkView(VALUE self){
	return CommandHandler(F_BOOKMARK_VIEW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_BookmarkPattern(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_BOOKMARK_PATTERN, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_TagJumpEx(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4){
	VALUE args[4] = { arg1, arg2, arg3, arg4 };
	return CommandHandler(F_TAGJUMP_EX, self, args, 4);
}

VALUE __cdecl CRubyEditorHandler::S_ChgmodINS(VALUE self){
	return CommandHandler(F_CHGMOD_INS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ChgCharSet(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_CHG_CHARSET, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_ChgmodEOL(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_CHGMOD_EOL, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CancelMode(VALUE self){
	return CommandHandler(F_CANCEL_MODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExecExternalMacro(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_EXECEXTMACRO, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_ShowToolbar(VALUE self){
	return CommandHandler(F_SHOWTOOLBAR, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ShowFunckey(VALUE self){
	return CommandHandler(F_SHOWFUNCKEY, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ShowTab(VALUE self){
	return CommandHandler(F_SHOWTAB, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ShowStatusbar(VALUE self){
	return CommandHandler(F_SHOWSTATUSBAR, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TypeList(VALUE self){
	return CommandHandler(F_TYPE_LIST, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ChangeType(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_CHANGETYPE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_OptionType(VALUE self){
	return CommandHandler(F_OPTION_TYPE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_OptionCommon(VALUE self){
	return CommandHandler(F_OPTION, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SelectFont(VALUE self){
	return CommandHandler(F_FONT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SetFontSize(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_SETFONTSIZE, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_WrapWindowWidth(VALUE self){
	return CommandHandler(F_WRAPWINDOWWIDTH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_OptionFavorite(VALUE self){
	return CommandHandler(F_FAVORITE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SetMsgQuoteStr(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_SET_QUOTESTRING, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_TextWrapMethod(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_TEXTWRAPMETHOD, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_SelectCountMode(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_SELECT_COUNT_MODE, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_ExecCommand(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(F_EXECMD, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_ExecCommandDialog(VALUE self){
	return CommandHandler(F_EXECMD_DIALOG, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_RMenu(VALUE self){
	return CommandHandler(F_MENU_RBUTTON, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu1(VALUE self){
	return CommandHandler(F_CUSTMENU_1, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu2(VALUE self){
	return CommandHandler(F_CUSTMENU_2, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu3(VALUE self){
	return CommandHandler(F_CUSTMENU_3, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu4(VALUE self){
	return CommandHandler(F_CUSTMENU_4, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu5(VALUE self){
	return CommandHandler(F_CUSTMENU_5, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu6(VALUE self){
	return CommandHandler(F_CUSTMENU_6, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu7(VALUE self){
	return CommandHandler(F_CUSTMENU_7, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu8(VALUE self){
	return CommandHandler(F_CUSTMENU_8, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu9(VALUE self){
	return CommandHandler(F_CUSTMENU_9, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu10(VALUE self){
	return CommandHandler(F_CUSTMENU_10, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu11(VALUE self){
	return CommandHandler(F_CUSTMENU_11, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu12(VALUE self){
	return CommandHandler(F_CUSTMENU_12, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu13(VALUE self){
	return CommandHandler(F_CUSTMENU_13, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu14(VALUE self){
	return CommandHandler(F_CUSTMENU_14, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu15(VALUE self){
	return CommandHandler(F_CUSTMENU_15, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu16(VALUE self){
	return CommandHandler(F_CUSTMENU_16, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu17(VALUE self){
	return CommandHandler(F_CUSTMENU_17, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu18(VALUE self){
	return CommandHandler(F_CUSTMENU_18, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu19(VALUE self){
	return CommandHandler(F_CUSTMENU_19, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu20(VALUE self){
	return CommandHandler(F_CUSTMENU_20, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu21(VALUE self){
	return CommandHandler(F_CUSTMENU_21, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu22(VALUE self){
	return CommandHandler(F_CUSTMENU_22, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu23(VALUE self){
	return CommandHandler(F_CUSTMENU_23, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CustMenu24(VALUE self){
	return CommandHandler(F_CUSTMENU_24, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SplitWinV(VALUE self){
	return CommandHandler(F_SPLIT_V, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SplitWinH(VALUE self){
	return CommandHandler(F_SPLIT_H, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SplitWinVH(VALUE self){
	return CommandHandler(F_SPLIT_VH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WinClose(VALUE self){
	return CommandHandler(F_WINCLOSE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WinCloseAll(VALUE self){
	return CommandHandler(F_WIN_CLOSEALL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CascadeWin(VALUE self){
	return CommandHandler(F_CASCADE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TileWinV(VALUE self){
	return CommandHandler(F_TILE_V, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TileWinH(VALUE self){
	return CommandHandler(F_TILE_H, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_NextWindow(VALUE self){
	return CommandHandler(F_NEXTWINDOW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PrevWindow(VALUE self){
	return CommandHandler(F_PREVWINDOW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_WindowList(VALUE self){
	return CommandHandler(F_WINLIST, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MaximizeV(VALUE self){
	return CommandHandler(F_MAXIMIZE_V, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MaximizeH(VALUE self){
	return CommandHandler(F_MAXIMIZE_H, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_MinimizeAll(VALUE self){
	return CommandHandler(F_MINIMIZE_ALL, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ReDraw(VALUE self){
	return CommandHandler(F_REDRAW, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ActivateWinOutput(VALUE self){
	return CommandHandler(F_WIN_OUTPUT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TraceOut(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_TRACEOUT, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_WindowTopMost(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_TOPMOST, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GroupClose(VALUE self){
	return CommandHandler(F_GROUPCLOSE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_NextGroup(VALUE self){
	return CommandHandler(F_NEXTGROUP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_PrevGroup(VALUE self){
	return CommandHandler(F_PREVGROUP, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabMoveRight(VALUE self){
	return CommandHandler(F_TAB_MOVERIGHT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabMoveLeft(VALUE self){
	return CommandHandler(F_TAB_MOVELEFT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabSeparate(VALUE self){
	return CommandHandler(F_TAB_SEPARATE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabJointNext(VALUE self){
	return CommandHandler(F_TAB_JOINTNEXT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabJointPrev(VALUE self){
	return CommandHandler(F_TAB_JOINTPREV, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabCloseOther(VALUE self){
	return CommandHandler(F_TAB_CLOSEOTHER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabCloseLeft(VALUE self){
	return CommandHandler(F_TAB_CLOSELEFT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_TabCloseRight(VALUE self){
	return CommandHandler(F_TAB_CLOSERIGHT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_Complete(VALUE self){
	return CommandHandler(F_HOKAN, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ToggleKeyHelpSearch(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_TOGGLE_KEY_SEARCH, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_HelpContents(VALUE self){
	return CommandHandler(F_HELP_CONTENTS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_HelpSearch(VALUE self){
	return CommandHandler(F_HELP_SEARCH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_CommandList(VALUE self){
	return CommandHandler(F_MENU_ALLFUNC, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExtHelp1(VALUE self){
	return CommandHandler(F_EXTHELP1, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ExtHtmlHelp(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_EXTHTMLHELP, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_About(VALUE self){
	return CommandHandler(F_ABOUT, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_StatusMsg(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(F_STATUSMSG, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_MsgBeep(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(F_MSGBEEP, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CommitUndoBuffer(VALUE self){
	return CommandHandler(F_COMMITUNDOBUFFER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_AddRefUndoBuffer(VALUE self){
	return CommandHandler(F_ADDREFUNDOBUFFER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_SetUndoBuffer(VALUE self){
	return CommandHandler(F_SETUNDOBUFFER, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_AppendUndoBufferCursor(VALUE self){
	return CommandHandler(F_APPENDUNDOBUFFERCURSOR, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ClipboardEmpty(VALUE self){
	return CommandHandler(F_CLIPBOARDEMPTY, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetFilename(VALUE self){
	return FunctionHandler(F_GETFILENAME, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSaveFilename(VALUE self){
	return FunctionHandler(F_GETSAVEFILENAME, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectedString(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_GETSELECTED, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_ExpandParameter(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_EXPANDPARAMETER, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GetLineStr(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_GETLINESTR, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GetLineCount(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_GETLINECOUNT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_ChangeTabWidth(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_CHGTABWIDTH, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_IsTextSelected(VALUE self){
	return FunctionHandler(F_ISTEXTSELECTED, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectLineFrom(VALUE self){
	return FunctionHandler(F_GETSELLINEFROM, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectColmFrom(VALUE self){
	return FunctionHandler(F_GETSELCOLUMNFROM, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectColumnFrom(VALUE self){
	return FunctionHandler(F_GETSELCOLUMNFROM, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectLineTo(VALUE self){
	return FunctionHandler(F_GETSELLINETO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectColmTo(VALUE self){
	return FunctionHandler(F_GETSELCOLUMNTO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetSelectColumnTo(VALUE self){
	return FunctionHandler(F_GETSELCOLUMNTO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IsInsMode(VALUE self){
	return FunctionHandler(F_ISINSMODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetCharCode(VALUE self){
	return FunctionHandler(F_GETCHARCODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetLineCode(VALUE self){
	return FunctionHandler(F_GETLINECODE, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IsPossibleUndo(VALUE self){
	return FunctionHandler(F_ISPOSSIBLEUNDO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IsPossibleRedo(VALUE self){
	return FunctionHandler(F_ISPOSSIBLEREDO, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_ChangeWrapColm(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_CHGWRAPCOLUMN, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_ChangeWrapColumn(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_CHGWRAPCOLUMN, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_IsCurTypeExt(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_ISCURTYPEEXT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_IsSameTypeExt(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_ISSAMETYPEEXT, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_InputBox(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return FunctionHandler(F_INPUTBOX, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_MessageBox(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_MESSAGEBOX, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_ErrorMsg(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_ERRORMSG, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_WarnMsg(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_WARNMSG, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_InfoMsg(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_INFOMSG, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_OkCancelBox(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_OKCANCELBOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_YesNoBox(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_YESNOBOX, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_CompareVersion(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_COMPAREVERSION, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_Sleep(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_MACROSLEEP, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_FileOpenDialog(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_FILEOPENDIALOG, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_FileSaveDialog(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_FILESAVEDIALOG, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_FolderDialog(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_FOLDERDIALOG, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetClipboard(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_GETCLIPBOARD, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_SetClipboard(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_SETCLIPBOARD, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_LayoutToLogicLineNum(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_LAYOUTTOLOGICLINENUM, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_LogicToLayoutLineNum(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_LOGICTOLAYOUTLINENUM, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_LineColumnToIndex(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_LINECOLUMNTOINDEX, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_LineIndexToColumn(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_LINEINDEXTOCOLUMN, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetCookie(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_GETCOOKIE, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetCookieDefault(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return FunctionHandler(F_GETCOOKIEDEFAULT, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_SetCookie(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return FunctionHandler(F_SETCOOKIE, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_DeleteCookie(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_DELETECOOKIE, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetCookieNames(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_GETCOOKIENAMES, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_SetDrawSwitch(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_SETDRAWSWITCH, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GetDrawSwitch(VALUE self){
	return FunctionHandler(F_GETDRAWSWITCH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IsShownStatus(VALUE self){
	return FunctionHandler(F_ISSHOWNSTATUS, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_GetStrWidth(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_GETSTRWIDTH, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetStrLayoutLength(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(F_GETSTRLAYOUTLENGTH, self, args, 2);
}

VALUE __cdecl CRubyEditorHandler::S_GetDefaultCharLength(VALUE self){
	return FunctionHandler(F_GETDEFAULTCHARLENGTH, self, NULL, 0);
}

VALUE __cdecl CRubyEditorHandler::S_IsIncludeClipboardFormat(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(F_ISINCLUDECLIPBOARDFORMAT, self, args, 1);
}

VALUE __cdecl CRubyEditorHandler::S_GetClipboardByFormat(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return FunctionHandler(F_GETCLIPBOARDBYFORMAT, self, args, 3);
}

VALUE __cdecl CRubyEditorHandler::S_SetClipboardByFormat(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4){
	VALUE args[4] = { arg1, arg2, arg3, arg4 };
	return FunctionHandler(F_SETCLIPBOARDBYFORMAT, self, args, 4);
}
