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

#include "stdafx.h"
#include "CExternalEditorIfObj.h"

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////
WideString CExternalEditorIfObj::S_GetFilename()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETFILENAME, Arguments, 0, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetSaveFilename()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETSAVEFILENAME, Arguments, 0, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetSelectedString(const int arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_GETSELECTED, Arguments, 1, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_ExpandParameter(LPCWSTR arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_EXPANDPARAMETER, Arguments, 1, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetLineStr(const int arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_GETLINESTR, Arguments, 1, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetLineCount(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_GETLINECOUNT, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_ChangeTabWidth(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_CHGTABWIDTH, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsTextSelected()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_ISTEXTSELECTED, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetSelectLineFrom()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETSELLINEFROM, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetSelectColumnFrom()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETSELCOLUMNFROM, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetSelectLineTo()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETSELLINETO, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetSelectColumnTo()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETSELCOLUMNTO, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsInsMode()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_ISINSMODE, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetCharCode()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETCHARCODE, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetLineCode()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETLINECODE, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsPossibleUndo()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_ISPOSSIBLEUNDO, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsPossibleRedo()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_ISPOSSIBLEREDO, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_ChangeWrapColumn(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_CHGWRAPCOLUMN, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsCurTypeExt(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_ISCURTYPEEXT, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsSameTypeExt(LPCWSTR arg1, LPCWSTR arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_ISSAMETYPEEXT, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_InputBox(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	SET_PARAM(Arguments[2], arg3);
	if(HandleFunction(F_INPUTBOX, Arguments, 3, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_MessageBox(LPCWSTR arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_MESSAGEBOX, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_ErrorMsg(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_ERRORMSG, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_WarnMsg(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_WARNMSG, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_InfoMsg(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_INFOMSG, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_OkCancelBox(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_OKCANCELBOX, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_YesNoBox(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_YESNOBOX, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_CompareVersion(LPCWSTR arg1, LPCWSTR arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_COMPAREVERSION, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_Sleep(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_MACROSLEEP, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_FileOpenDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_FILEOPENDIALOG, Arguments, 2, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_FileSaveDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_FILESAVEDIALOG, Arguments, 2, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_FolderDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_FOLDERDIALOG, Arguments, 2, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetClipboard(const int arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_GETCLIPBOARD, Arguments, 1, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_SetClipboard(const int arg1, LPCWSTR arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_SETCLIPBOARD, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_LayoutToLogicLineNum(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_LAYOUTTOLOGICLINENUM, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_LogicToLayoutLineNum(const int arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_LOGICTOLAYOUTLINENUM, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_LineColumnToIndex(const int arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_LINECOLUMNTOINDEX, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_LineIndexToColumn(const int arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_LINEINDEXTOCOLUMN, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetCookie(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_GETCOOKIE, Arguments, 2, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetCookieDefault(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	SET_PARAM(Arguments[2], arg3);
	if(HandleFunction(F_GETCOOKIEDEFAULT, Arguments, 3, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_SetCookie(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	SET_PARAM(Arguments[2], arg3);
	if(HandleFunction(F_SETCOOKIE, Arguments, 3, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_DeleteCookie(LPCWSTR arg1, LPCWSTR arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_DELETECOOKIE, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetCookieNames(LPCWSTR arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_GETCOOKIENAMES, Arguments, 1, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_SetDrawSwitch(const int arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_SETDRAWSWITCH, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetDrawSwitch()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETDRAWSWITCH, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsShownStatus()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_ISSHOWNSTATUS, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetStrWidth(LPCWSTR arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_GETSTRWIDTH, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetStrLayoutLength(LPCWSTR arg1, const int arg2)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_GETSTRLAYOUTLENGTH, Arguments, 2, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_GetDefaultCharLength()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_GETDEFAULTCHARLENGTH, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_IsIncludeClipboardFormat(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_ISINCLUDECLIPBOARDFORMAT, Arguments, 1, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

WideString CExternalEditorIfObj::S_GetClipboardByFormat(LPCWSTR arg1, const int arg2, const int arg3)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	SET_PARAM(Arguments[2], arg3);
	if(HandleFunction(F_GETCLIPBOARDBYFORMAT, Arguments, 3, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

int CExternalEditorIfObj::S_SetClipboardByFormat(LPCWSTR arg1, LPCWSTR arg2, const int arg3, const int arg4)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	SET_PARAM(Arguments[2], arg3);
	SET_PARAM(Arguments[3], arg4);
	if(HandleFunction(F_SETCLIPBOARDBYFORMAT, Arguments, 4, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

void CExternalEditorIfObj::S_FileNew()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FILENEW, Arguments, ArgLengths, 0);
}
/*
void CExternalEditorIfObj::S_FileOpen(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_FILEOPEN, Arguments, ArgLengths, 1);
}
*/
void CExternalEditorIfObj::S_FileOpen(LPCWSTR arg1, const int arg2, const int arg3, LPCWSTR arg4)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	SET_WCHAR_ARG(3, arg4);
	HandleCommand(F_FILEOPEN2, Arguments, ArgLengths, 4);
}

void CExternalEditorIfObj::S_FileSave()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FILESAVE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_FileSaveAll()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FILESAVEALL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_FileSaveAsDialog(LPCWSTR arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_FILESAVEAS, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_FileSaveAs(LPCWSTR arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_FILESAVEAS, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_FileClose()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FILECLOSE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_FileCloseOpen()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FILECLOSE_OPEN, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_FileReopen(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenSJIS(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_SJIS, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenJIS(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_JIS, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenEUC(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_EUC, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenLatin1(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_LATIN1, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenUNICODE(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_UNICODE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenUNICODEBE(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_UNICODEBE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenUTF8(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_UTF8, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenCESU8(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_CESU8, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_FileReopenUTF7(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_FILE_REOPEN_UTF7, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_Print()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PRINT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PrintPreview()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PRINT_PREVIEW, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PrintPageSetup()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PRINT_PAGESETUP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_OpenHfromtoC()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_OPEN_HfromtoC, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ActivateSQLPLUS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_ACTIVATE_SQLPLUS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExecSQLPLUS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PLSQL_COMPILE_ON_SQLPLUS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Browse()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BROWSE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ViewMode()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_VIEWMODE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ReadOnly()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_VIEWMODE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PropertyFile()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PROPERTY_FILE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExitAllEditors()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_EXITALLEDITORS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExitAll()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_EXITALL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PutFile(LPCWSTR arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_PUTFILE, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_InsFile(LPCWSTR arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_INSFILE, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_Char(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WCHAR, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_CharIme(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_IME_CHAR, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_Undo()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UNDO, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Redo()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_REDO, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Delete()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DELETE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DeleteBack()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DELETE_BACK, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordDeleteToStart()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WordDeleteToStart, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordDeleteToEnd()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WordDeleteToEnd, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordCut()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WordCut, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordDelete()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WordDelete, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_LineCutToStart()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LineCutToStart, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_LineCutToEnd()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LineCutToEnd, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_LineDeleteToStart()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LineDeleteToStart, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_LineDeleteToEnd()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LineDeleteToEnd, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CutLine()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUT_LINE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DeleteLine()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DELETE_LINE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DuplicateLine()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DUPLICATELINE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_IndentTab()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_INDENT_TAB, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_UnindentTab()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UNINDENT_TAB, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_IndentSpace()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_INDENT_SPACE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_UnindentSpace()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UNINDENT_SPACE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_LTrim()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LTRIM, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_RTrim()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_RTRIM, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SortAsc()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SORT_ASC, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SortDesc()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SORT_DESC, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Merge()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MERGE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Left()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LEFT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Right()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_RIGHT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up2()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP2, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down2()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN2, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordLeft()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDLEFT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordRight()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDRIGHT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoLineTop(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINETOP, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_GoLineEnd(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINEEND, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_HalfPageUp()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageUp, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HalfPageDown()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageDown, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageUp()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageUp()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageDown()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageDown()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileTop()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILETOP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileEnd()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILEEND, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CurLineCenter()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CURLINECENTER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MoveHistPrev()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_JUMPHIST_PREV, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MoveHistNext()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_JUMPHIST_NEXT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MoveHistSet()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_JUMPHIST_SET, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WndScrollDown()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WndScrollDown, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WndScrollUp()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WndScrollUp, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoNextParagraph()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GONEXTPARAGRAPH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoPrevParagraph()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOPREVPARAGRAPH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MoveCursor(const int arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_MOVECURSOR, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_MoveCursorLayout(const int arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_MOVECURSORLAYOUT, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_WheelUp(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELUP, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelDown(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELDOWN, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelLeft(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELLEFT, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelRight(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELRIGHT, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelPageUp(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELPAGEUP, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelPageDown(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELPAGEDOWN, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelPageLeft(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELPAGELEFT, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_WheelPageRight(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_WHEELPAGERIGHT, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_SelectWord()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SELECTWORD, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SelectAll()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SELECTALL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SelectLine(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_SELECTLINE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_BeginSelect()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BEGIN_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Left_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LEFT_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Right_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_RIGHT_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up2_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP2_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down2_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN2_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordLeft_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDLEFT_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordRight_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDRIGHT_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoLineTop_Sel(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINETOP_SEL, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_GoLineEnd_Sel(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINEEND_SEL, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_HalfPageUp_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageUp_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HalfPageDown_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageDown_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageUp_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageUp_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageDown_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageDown_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown_Sel, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileTop_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILETOP_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileEnd_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILEEND_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoNextParagraph_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GONEXTPARAGRAPH_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoPrevParagraph_Sel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOPREVPARAGRAPH_SEL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BeginBoxSelect()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BEGIN_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Left_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LEFT_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Right_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_RIGHT_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Up2_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UP2_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Down2_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DOWN2_BOX, Arguments, ArgLengths, 0);
}
/*
void CExternalEditorIfObj::S_Left2_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_LEFT2_BOX, Arguments, ArgLengths, 0);
}
*/
/*
void CExternalEditorIfObj::S_Right2_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_RIGHT2_BOX, Arguments, ArgLengths, 0);
}
*/
void CExternalEditorIfObj::S_WordLeft_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDLEFT_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WordRight_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WORDRIGHT_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoLogicalLineTop_BoxSel(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLOGICALLINETOP_BOX, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_GoLineTop_BoxSel(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINETOP_BOX, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_GoLineEnd_BoxSel(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_GOLINEEND_BOX, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_HalfPageUp_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageUp_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HalfPageDown_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HalfPageDown_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageUp_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageUp_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageUp_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PageDown_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_1PageDown_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_1PageDown_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileTop_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILETOP_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_GoFileEnd_BoxSel()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GOFILEEND_BOX, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Cut()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Copy()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPY, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Paste(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_PASTE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_CopyAddCRLF()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPY_ADDCRLF, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyCRLF()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPY_CRLF, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PasteBox(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_PASTEBOX, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_InsBoxText(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_INSBOXTEXT, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_InsText(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_INSTEXT_W, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_AddTail(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_ADDTAIL_W, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_CopyLines()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYLINES, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyLinesAsPassage()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYLINESASPASSAGE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyLinesWithLineNumber()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYLINESWITHLINENUMBER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyColorHtml()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPY_COLOR_HTML, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyColorHtmlWithLineNumber()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPY_COLOR_HTML_LINENUMBER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyPath()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYPATH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyFilename()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYFNAME, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyTag()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COPYTAG, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CopyKeyBindList()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CREATEKEYBINDLIST, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_InsertDate()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_INS_DATE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_InsertTime()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_INS_TIME, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CtrlCodeDialog()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CTRL_CODE_DIALOG, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CtrlCode(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_CTRL_CODE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_ToLower()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOLOWER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToUpper()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOUPPER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToHankaku()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOHANKAKU, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToHankata()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOHANKATA, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToZenEi()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOZENEI, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToHanEi()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOHANEI, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToZenKata()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOZENKAKUKATA, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToZenHira()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TOZENKAKUHIRA, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HanKataToZenKata()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HANKATATOZENKATA, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HanKataToZenHira()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HANKATATOZENHIRA, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TABToSPACE()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TABTOSPACE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SPACEToTAB()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SPACETOTAB, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_AutoToSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_AUTO2SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_JIStoSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_EMAIL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_EUCtoSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_EUC2SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CodeCnvUNICODEtoSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_UNICODE2SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CodeCnvUNICODEBEtoSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_UNICODEBE2SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_UTF8toSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_UTF82SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_UTF7toSJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_UTF72SJIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SJIStoJIS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_SJIS2JIS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SJIStoEUC()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_SJIS2EUC, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SJIStoUTF8()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_SJIS2UTF8, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SJIStoUTF7()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CODECNV_SJIS2UTF7, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Base64Decode()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BASE64DECODE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Uudecode()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_UUDECODE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SearchDialog()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SEARCH_DIALOG, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SearchNext(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_SEARCH_NEXT, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_SearchPrev(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_SEARCH_PREV, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_ReplaceDialog()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_REPLACE_DIALOG, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Replace(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_REPLACE, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_ReplaceAll(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_REPLACE_ALL, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_SearchClearMark()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SEARCH_CLEARMARK, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SearchStartPos()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_JUMP_SRCHSTARTPOS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Grep(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3, const int arg4, const int arg5)
{
	LPCWSTR Arguments[5] = { L"", L"", L"", L"", L"" };
	int ArgLengths[5] = { 0, 0, 0, 0, 0 };
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	SET_INT_ARG(3, arg4);
	SET_INT_ARG(4, arg5);
	HandleCommand(F_GREP, Arguments, ArgLengths, 5);
}

void CExternalEditorIfObj::S_Jump(const int arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_JUMP, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_Outline(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
HandleCommand(F_OUTLINE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_TagJump()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAGJUMP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TagJumpBack()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAGJUMPBACK, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TagMake()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAGS_MAKE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DirectTagJump()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DIRECT_TAGJUMP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_KeywordTagJump(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_TAGJUMP_KEYWORD, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_Compare()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COMPARE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DiffDialog()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DIFF_DIALOG, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Diff(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_DIFF, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_DiffNext()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DIFF_NEXT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DiffPrev()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DIFF_PREV, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_DiffReset()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_DIFF_RESET, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BracketPair()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BRACKETPAIR, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkSet()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BOOKMARK_SET, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkNext()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BOOKMARK_NEXT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkPrev()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BOOKMARK_PREV, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkReset()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BOOKMARK_RESET, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkView()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_BOOKMARK_VIEW, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_BookmarkPattern(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_BOOKMARK_PATTERN, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_TagJumpEx(LPCWSTR arg1, const int arg2, const int arg3, const int arg4)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	SET_INT_ARG(3, arg4);
	HandleCommand(F_TAGJUMP_EX, Arguments, ArgLengths, 4);
}

void CExternalEditorIfObj::S_ChgmodINS()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CHGMOD_INS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ChgCharSet(const int arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_CHG_CHARSET, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_ChgmodEOL(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_CHGMOD_EOL, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_CancelMode()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CANCEL_MODE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExecExternalMacro(LPCWSTR arg1, LPCWSTR arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	HandleCommand(F_EXECEXTMACRO, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_ShowToolbar()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SHOWTOOLBAR, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ShowFunckey()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SHOWFUNCKEY, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ShowTab()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SHOWTAB, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ShowStatusbar()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SHOWSTATUSBAR, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TypeList()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TYPE_LIST, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ChangeType(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_TYPE_LIST, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_OptionType()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_OPTION_TYPE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_OptionCommon()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_OPTION, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SelectFont()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FONT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SetFontSize(const int arg1, const int arg2, const int arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_INT_ARG(2, arg3);
	HandleCommand(F_SETFONTSIZE, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_WrapWindowWidth()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WRAPWINDOWWIDTH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_OptionFavorite()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_FAVORITE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SetMsgQuoteStr(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_SET_QUOTESTRING, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_TextWrapMethod(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_TEXTWRAPMETHOD, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_SelectCountMode(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_SELECT_COUNT_MODE, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_ExecCommand(LPCWSTR arg1, const int arg2, LPCWSTR arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	HandleCommand(F_EXECMD, Arguments, ArgLengths, 3);
}

void CExternalEditorIfObj::S_ExecCommandDialog()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_EXECMD, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_RMenu()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MENU_RBUTTON, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu1()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_1, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu2()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_2, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu3()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_3, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu4()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_4, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu5()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_5, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu6()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_6, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu7()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_7, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu8()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_8, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu9()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_9, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu10()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_10, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu11()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_11, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu12()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_12, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu13()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_13, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu14()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_14, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu15()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_15, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu16()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_16, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu17()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_17, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu18()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_18, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu19()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_19, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu20()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_20, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu21()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_21, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu22()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_22, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu23()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_23, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CustMenu24()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CUSTMENU_24, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SplitWinV()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SPLIT_V, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SplitWinH()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SPLIT_H, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SplitWinVH()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SPLIT_VH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WinClose()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WINCLOSE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WinCloseAll()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WIN_CLOSEALL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CascadeWin()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CASCADE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TileWinV()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TILE_V, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TileWinH()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TILE_H, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_NextWindow()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_NEXTWINDOW, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PrevWindow()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PREVWINDOW, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_WindowList()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WINLIST, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MaximizeV()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MAXIMIZE_V, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MaximizeH()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MAXIMIZE_H, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_MinimizeAll()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MINIMIZE_ALL, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ReDraw()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_REDRAW, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ActivateWinOutput()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_WIN_OUTPUT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TraceOut(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_TRACEOUT, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_WindowTopMost(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_TOPMOST, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_GroupClose()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_GROUPCLOSE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_NextGroup()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_NEXTGROUP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_PrevGroup()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_PREVGROUP, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabMoveRight()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_MOVERIGHT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabMoveLeft()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_MOVELEFT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabSeparate()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_SEPARATE, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabJointNext()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_JOINTNEXT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabJointPrev()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_JOINTPREV, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabCloseOther()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_CLOSEOTHER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabCloseLeft()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_CLOSELEFT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_TabCloseRight()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_TAB_CLOSERIGHT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_Complete()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HOKAN, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ToggleKeyHelpSearch(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_HOKAN, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_HelpContents()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HELP_CONTENTS, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_HelpSearch()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_HELP_SEARCH, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_CommandList()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_MENU_ALLFUNC, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExtHelp1()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_EXTHELP1, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ExtHtmlHelp(LPCWSTR arg1, LPCWSTR arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	HandleCommand(F_EXTHTMLHELP, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_About()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_ABOUT, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_StatusMsg(LPCWSTR arg1, const int arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	HandleCommand(F_STATUSMSG, Arguments, ArgLengths, 2);
}

void CExternalEditorIfObj::S_MsgBeep(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_MSGBEEP, Arguments, ArgLengths, 1);
}

void CExternalEditorIfObj::S_CommitUndoBuffer()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_COMMITUNDOBUFFER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_AddRefUndoBuffer()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_ADDREFUNDOBUFFER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_SetUndoBuffer()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_SETUNDOBUFFER, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_AppendUndoBufferCursor()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_APPENDUNDOBUFFERCURSOR, Arguments, ArgLengths, 0);
}

void CExternalEditorIfObj::S_ClipboardEmpty()
{
	DEFINE_WCHAR_PARAMS(Arguments);
	HandleCommand(F_CLIPBOARDEMPTY, Arguments, ArgLengths, 0);
}

///////////////////////////////////////////////////////////////////////////////
// Aliases
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////
WideString CExternalEditorIfObj::GetFilename()
{
	return S_GetFilename();
}

WideString CExternalEditorIfObj::GetSaveFilename()
{
	return S_GetSaveFilename();
}

WideString CExternalEditorIfObj::GetSelectedString(const int arg1)
{
	return S_GetSelectedString(arg1);
}

WideString CExternalEditorIfObj::ExpandParameter(LPCWSTR arg1)
{
	return S_ExpandParameter(arg1);
}

WideString CExternalEditorIfObj::GetLineStr(const int arg1)
{
	return S_GetLineStr(arg1);
}

int CExternalEditorIfObj::GetLineCount(const int arg1)
{
	return S_GetLineCount(arg1);
}

int CExternalEditorIfObj::ChangeTabWidth(const int arg1)
{
	return S_ChangeTabWidth(arg1);
}

int CExternalEditorIfObj::IsTextSelected()
{
	return S_IsTextSelected();
}

int CExternalEditorIfObj::GetSelectLineFrom()
{
	return S_GetSelectLineFrom();
}

int CExternalEditorIfObj::GetSelectColumnFrom()
{
	return S_GetSelectColumnFrom();
}

int CExternalEditorIfObj::GetSelectLineTo()
{
	return S_GetSelectLineTo();
}

int CExternalEditorIfObj::GetSelectColumnTo()
{
	return S_GetSelectColumnTo();
}

int CExternalEditorIfObj::IsInsMode()
{
	return S_IsInsMode();
}

int CExternalEditorIfObj::GetCharCode()
{
	return S_GetCharCode();
}

int CExternalEditorIfObj::GetLineCode()
{
	return S_GetLineCode();
}

int CExternalEditorIfObj::IsPossibleUndo()
{
	return S_IsPossibleUndo();
}

int CExternalEditorIfObj::IsPossibleRedo()
{
	return S_IsPossibleRedo();
}

int CExternalEditorIfObj::ChangeWrapColumn(const int arg1)
{
	return S_ChangeWrapColumn(arg1);
}

int CExternalEditorIfObj::IsCurTypeExt(LPCWSTR arg1)
{
	return S_IsCurTypeExt(arg1);
}

int CExternalEditorIfObj::IsSameTypeExt(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_IsSameTypeExt(arg1, arg2);
}

WideString CExternalEditorIfObj::InputBox(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	return S_InputBox(arg1, arg2, arg3);
}

int CExternalEditorIfObj::MessageBox(LPCWSTR arg1, const int arg2)
{
	return S_MessageBox(arg1, arg2);
}

int CExternalEditorIfObj::ErrorMsg(LPCWSTR arg1)
{
	return S_ErrorMsg(arg1);
}

int CExternalEditorIfObj::WarnMsg(LPCWSTR arg1)
{
	return S_WarnMsg(arg1);
}

int CExternalEditorIfObj::InfoMsg(LPCWSTR arg1)
{
	return S_InfoMsg(arg1);
}

int CExternalEditorIfObj::OkCancelBox(LPCWSTR arg1)
{
	return S_OkCancelBox(arg1);
}

int CExternalEditorIfObj::YesNoBox(LPCWSTR arg1)
{
	return S_YesNoBox(arg1);
}

int CExternalEditorIfObj::CompareVersion(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_CompareVersion(arg1, arg2);
}

int CExternalEditorIfObj::Sleep(const int arg1)
{
	return S_Sleep(arg1);
}

WideString CExternalEditorIfObj::FileOpenDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_FileOpenDialog(arg1, arg2);
}

WideString CExternalEditorIfObj::FileSaveDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_FileSaveDialog(arg1, arg2);
}

WideString CExternalEditorIfObj::FolderDialog(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_FolderDialog(arg1, arg2);
}

WideString CExternalEditorIfObj::GetClipboard(const int arg1)
{
	return S_GetClipboard(arg1);
}

int CExternalEditorIfObj::SetClipboard(const int arg1, LPCWSTR arg2)
{
	return S_SetClipboard(arg1, arg2);
}

int CExternalEditorIfObj::LayoutToLogicLineNum(const int arg1)
{
	return S_LayoutToLogicLineNum(arg1);
}

int CExternalEditorIfObj::LogicToLayoutLineNum(const int arg1, const int arg2)
{
	return S_LogicToLayoutLineNum(arg1, arg2);
}

int CExternalEditorIfObj::LineColumnToIndex(const int arg1, const int arg2)
{
	return S_LineColumnToIndex(arg1, arg2);
}

int CExternalEditorIfObj::LineIndexToColumn(const int arg1, const int arg2)
{
	return S_LineIndexToColumn(arg1, arg2);
}

WideString CExternalEditorIfObj::GetCookie(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_GetCookie(arg1, arg2);
}

WideString CExternalEditorIfObj::GetCookieDefault(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	return S_GetCookieDefault(arg1, arg2, arg3);
}

int CExternalEditorIfObj::SetCookie(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	return S_SetCookie(arg1, arg2, arg3);
}

int CExternalEditorIfObj::DeleteCookie(LPCWSTR arg1, LPCWSTR arg2)
{
	return S_DeleteCookie(arg1, arg2);
}

WideString CExternalEditorIfObj::GetCookieNames(LPCWSTR arg1)
{
	return S_GetCookieNames(arg1);
}

int CExternalEditorIfObj::SetDrawSwitch(const int arg1)
{
	return S_SetDrawSwitch(arg1);
}

int CExternalEditorIfObj::GetDrawSwitch()
{
	return S_GetDrawSwitch();
}

int CExternalEditorIfObj::IsShownStatus()
{
	return S_IsShownStatus();
}

int CExternalEditorIfObj::GetStrWidth(LPCWSTR arg1, const int arg2)
{
	return S_GetStrWidth(arg1, arg2);
}

int CExternalEditorIfObj::GetStrLayoutLength(LPCWSTR arg1, const int arg2)
{
	return S_GetStrLayoutLength(arg1, arg2);
}

int CExternalEditorIfObj::GetDefaultCharLength()
{
	return S_GetDefaultCharLength();
}

int CExternalEditorIfObj::IsIncludeClipboardFormat(LPCWSTR arg1)
{
	return S_IsIncludeClipboardFormat(arg1);
}

WideString CExternalEditorIfObj::GetClipboardByFormat(LPCWSTR arg1, const int arg2, const int arg3)
{
	return S_GetClipboardByFormat(arg1, arg2, arg3);
}

int CExternalEditorIfObj::SetClipboardByFormat(LPCWSTR arg1, LPCWSTR arg2, const int arg3, const int arg4)
{
	return S_SetClipboardByFormat(arg1, arg2, arg3, arg4);
}

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////
void CExternalEditorIfObj::FileNew()
{
	S_FileNew();
}
/*
void CExternalEditorIfObj::FileOpen(LPCWSTR arg1)
{
	S_FileOpen(arg1);
}
*/
void CExternalEditorIfObj::FileOpen(LPCWSTR arg1, const int arg2, const int arg3, LPCWSTR arg4)
{
	S_FileOpen(arg1, arg2, arg3, arg4);
}

void CExternalEditorIfObj::FileSave()
{
	S_FileSave();
}

void CExternalEditorIfObj::FileSaveAll()
{
	S_FileSaveAll();
}

void CExternalEditorIfObj::FileSaveAsDialog(LPCWSTR arg1, const int arg2, const int arg3)
{
	S_FileSaveAsDialog(arg1, arg2, arg3);
}

void CExternalEditorIfObj::FileSaveAs(LPCWSTR arg1, const int arg2, const int arg3)
{
	S_FileSaveAs(arg1, arg2, arg3);
}

void CExternalEditorIfObj::FileClose()
{
	S_FileClose();
}

void CExternalEditorIfObj::FileCloseOpen()
{
	S_FileCloseOpen();
}

void CExternalEditorIfObj::FileReopen(const int arg1)
{
	S_FileReopen(arg1);
}

void CExternalEditorIfObj::FileReopenSJIS(const int arg1)
{
	S_FileReopenSJIS(arg1);
}

void CExternalEditorIfObj::FileReopenJIS(const int arg1)
{
	S_FileReopenJIS(arg1);
}

void CExternalEditorIfObj::FileReopenEUC(const int arg1)
{
	S_FileReopenEUC(arg1);
}

void CExternalEditorIfObj::FileReopenLatin1(const int arg1)
{
	S_FileReopenLatin1(arg1);
}

void CExternalEditorIfObj::FileReopenUNICODE(const int arg1)
{
	S_FileReopenUNICODE(arg1);
}

void CExternalEditorIfObj::FileReopenUNICODEBE(const int arg1)
{
	S_FileReopenUNICODEBE(arg1);
}

void CExternalEditorIfObj::FileReopenUTF8(const int arg1)
{
	S_FileReopenUTF8(arg1);
}

void CExternalEditorIfObj::FileReopenCESU8(const int arg1)
{
	S_FileReopenCESU8(arg1);
}

void CExternalEditorIfObj::FileReopenUTF7(const int arg1)
{
	S_FileReopenUTF7(arg1);
}

void CExternalEditorIfObj::Print()
{
	S_Print();
}

void CExternalEditorIfObj::PrintPreview()
{
	S_PrintPreview();
}

void CExternalEditorIfObj::PrintPageSetup()
{
	S_PrintPageSetup();
}

void CExternalEditorIfObj::OpenHfromtoC()
{
	S_OpenHfromtoC();
}

void CExternalEditorIfObj::ActivateSQLPLUS()
{
	S_ActivateSQLPLUS();
}

void CExternalEditorIfObj::ExecSQLPLUS()
{
	S_ExecSQLPLUS();
}

void CExternalEditorIfObj::Browse()
{
	S_Browse();
}

void CExternalEditorIfObj::ViewMode()
{
	S_ViewMode();
}

void CExternalEditorIfObj::ReadOnly()
{
	S_ReadOnly();
}

void CExternalEditorIfObj::PropertyFile()
{
	S_PropertyFile();
}

void CExternalEditorIfObj::ExitAllEditors()
{
	S_ExitAllEditors();
}

void CExternalEditorIfObj::ExitAll()
{
	S_ExitAll();
}

void CExternalEditorIfObj::PutFile(LPCWSTR arg1, const int arg2, const int arg3)
{
	S_PutFile(arg1, arg2, arg3);
}

void CExternalEditorIfObj::InsFile(LPCWSTR arg1, const int arg2, const int arg3)
{
	S_InsFile(arg1, arg2, arg3);
}

void CExternalEditorIfObj::Char(const int arg1)
{
	S_Char(arg1);
}

void CExternalEditorIfObj::CharIme(const int arg1)
{
	S_CharIme(arg1);
}

void CExternalEditorIfObj::Undo()
{
	S_Undo();
}

void CExternalEditorIfObj::Redo()
{
	S_Redo();
}

void CExternalEditorIfObj::Delete()
{
	S_Delete();
}

void CExternalEditorIfObj::DeleteBack()
{
	S_DeleteBack();
}

void CExternalEditorIfObj::WordDeleteToStart()
{
	S_WordDeleteToStart();
}

void CExternalEditorIfObj::WordDeleteToEnd()
{
	S_WordDeleteToEnd();
}

void CExternalEditorIfObj::WordCut()
{
	S_WordCut();
}

void CExternalEditorIfObj::WordDelete()
{
	S_WordDelete();
}

void CExternalEditorIfObj::LineCutToStart()
{
	S_LineCutToStart();
}

void CExternalEditorIfObj::LineCutToEnd()
{
	S_LineCutToEnd();
}

void CExternalEditorIfObj::LineDeleteToStart()
{
	S_LineDeleteToStart();
}

void CExternalEditorIfObj::LineDeleteToEnd()
{
	S_LineDeleteToEnd();
}

void CExternalEditorIfObj::CutLine()
{
	S_CutLine();
}

void CExternalEditorIfObj::DeleteLine()
{
	S_DeleteLine();
}

void CExternalEditorIfObj::DuplicateLine()
{
	S_DuplicateLine();
}

void CExternalEditorIfObj::IndentTab()
{
	S_IndentTab();
}

void CExternalEditorIfObj::UnindentTab()
{
	S_UnindentTab();
}

void CExternalEditorIfObj::IndentSpace()
{
	S_IndentSpace();
}

void CExternalEditorIfObj::UnindentSpace()
{
	S_UnindentSpace();
}

void CExternalEditorIfObj::LTrim()
{
	S_LTrim();
}

void CExternalEditorIfObj::RTrim()
{
	S_RTrim();
}

void CExternalEditorIfObj::SortAsc()
{
	S_SortAsc();
}

void CExternalEditorIfObj::SortDesc()
{
	S_SortDesc();
}

void CExternalEditorIfObj::Merge()
{
	S_Merge();
}

void CExternalEditorIfObj::Up()
{
	S_Up();
}

void CExternalEditorIfObj::Down()
{
	S_Down();
}

void CExternalEditorIfObj::Left()
{
	S_Left();
}

void CExternalEditorIfObj::Right()
{
	S_Right();
}

void CExternalEditorIfObj::Up2()
{
	S_Up2();
}

void CExternalEditorIfObj::Down2()
{
	S_Down2();
}

void CExternalEditorIfObj::WordLeft()
{
	S_WordLeft();
}

void CExternalEditorIfObj::WordRight()
{
	S_WordRight();
}

void CExternalEditorIfObj::GoLineTop(const int arg1)
{
	S_GoLineTop(arg1);
}

void CExternalEditorIfObj::GoLineEnd(const int arg1)
{
	S_GoLineEnd(arg1);
}

void CExternalEditorIfObj::HalfPageUp()
{
	S_HalfPageUp();
}

void CExternalEditorIfObj::HalfPageDown()
{
	S_HalfPageDown();
}

void CExternalEditorIfObj::PageUp()
{
	S_PageUp();
}

void CExternalEditorIfObj::OnePageUp()
{
	S_1PageUp();
}

void CExternalEditorIfObj::PageDown()
{
	S_PageDown();
}

void CExternalEditorIfObj::OnePageDown()
{
	S_1PageDown();
}

void CExternalEditorIfObj::GoFileTop()
{
	S_GoFileTop();
}

void CExternalEditorIfObj::GoFileEnd()
{
	S_GoFileEnd();
}

void CExternalEditorIfObj::CurLineCenter()
{
	S_CurLineCenter();
}

void CExternalEditorIfObj::MoveHistPrev()
{
	S_MoveHistPrev();
}

void CExternalEditorIfObj::MoveHistNext()
{
	S_MoveHistNext();
}

void CExternalEditorIfObj::MoveHistSet()
{
	S_MoveHistSet();
}

void CExternalEditorIfObj::WndScrollDown()
{
	S_WndScrollDown();
}

void CExternalEditorIfObj::WndScrollUp()
{
	S_WndScrollUp();
}

void CExternalEditorIfObj::GoNextParagraph()
{
	S_GoNextParagraph();
}

void CExternalEditorIfObj::GoPrevParagraph()
{
	S_GoPrevParagraph();
}

void CExternalEditorIfObj::MoveCursor(const int arg1, const int arg2, const int arg3)
{
	S_MoveCursor(arg1, arg2, arg3);
}

void CExternalEditorIfObj::MoveCursorLayout(const int arg1, const int arg2, const int arg3)
{
	S_MoveCursorLayout(arg1, arg2, arg3);
}

void CExternalEditorIfObj::WheelUp(const int arg1)
{
	S_WheelUp(arg1);
}

void CExternalEditorIfObj::WheelDown(const int arg1)
{
	S_WheelDown(arg1);
}

void CExternalEditorIfObj::WheelLeft(const int arg1)
{
	S_WheelLeft(arg1);
}

void CExternalEditorIfObj::WheelRight(const int arg1)
{
	S_WheelRight(arg1);
}

void CExternalEditorIfObj::WheelPageUp(const int arg1)
{
	S_WheelPageUp(arg1);
}

void CExternalEditorIfObj::WheelPageDown(const int arg1)
{
	S_WheelPageDown(arg1);
}

void CExternalEditorIfObj::WheelPageLeft(const int arg1)
{
	S_WheelPageLeft(arg1);
}

void CExternalEditorIfObj::WheelPageRight(const int arg1)
{
	S_WheelPageRight(arg1);
}

void CExternalEditorIfObj::SelectWord()
{
	S_SelectWord();
}

void CExternalEditorIfObj::SelectAll()
{
	S_SelectAll();
}

void CExternalEditorIfObj::SelectLine(const int arg1)
{
	S_SelectLine(arg1);
}

void CExternalEditorIfObj::BeginSelect()
{
	S_BeginSelect();
}

void CExternalEditorIfObj::Up_Sel()
{
	S_Up_Sel();
}

void CExternalEditorIfObj::Down_Sel()
{
	S_Down_Sel();
}

void CExternalEditorIfObj::Left_Sel()
{
	S_Left_Sel();
}

void CExternalEditorIfObj::Right_Sel()
{
	S_Right_Sel();
}

void CExternalEditorIfObj::Up2_Sel()
{
	S_Up2_Sel();
}

void CExternalEditorIfObj::Down2_Sel()
{
	S_Down2_Sel();
}

void CExternalEditorIfObj::WordLeft_Sel()
{
	S_WordLeft_Sel();
}

void CExternalEditorIfObj::WordRight_Sel()
{
	S_WordRight_Sel();
}

void CExternalEditorIfObj::GoLineTop_Sel(const int arg1)
{
	S_GoLineTop_Sel(arg1);
}

void CExternalEditorIfObj::GoLineEnd_Sel(const int arg1)
{
	S_GoLineEnd_Sel(arg1);
}

void CExternalEditorIfObj::HalfPageUp_Sel()
{
	S_HalfPageUp_Sel();
}

void CExternalEditorIfObj::HalfPageDown_Sel()
{
	S_HalfPageDown_Sel();
}

void CExternalEditorIfObj::PageUp_Sel()
{
	S_PageUp_Sel();
}

void CExternalEditorIfObj::OnePageUp_Sel()
{
	S_1PageUp_Sel();
}

void CExternalEditorIfObj::PageDown_Sel()
{
	S_PageDown_Sel();
}

void CExternalEditorIfObj::OnePageDown_Sel()
{
	S_1PageDown_Sel();
}

void CExternalEditorIfObj::GoFileTop_Sel()
{
	S_GoFileTop_Sel();
}

void CExternalEditorIfObj::GoFileEnd_Sel()
{
	S_GoFileEnd_Sel();
}

void CExternalEditorIfObj::GoNextParagraph_Sel()
{
	S_GoNextParagraph_Sel();
}

void CExternalEditorIfObj::GoPrevParagraph_Sel()
{
	S_GoPrevParagraph_Sel();
}

void CExternalEditorIfObj::BeginBoxSelect()
{
	S_BeginBoxSelect();
}

void CExternalEditorIfObj::Up_BoxSel()
{
	S_Up_BoxSel();
}

void CExternalEditorIfObj::Down_BoxSel()
{
	S_Down_BoxSel();
}

void CExternalEditorIfObj::Left_BoxSel()
{
	S_Left_BoxSel();
}

void CExternalEditorIfObj::Right_BoxSel()
{
	S_Right_BoxSel();
}

void CExternalEditorIfObj::Up2_BoxSel()
{
	S_Up2_BoxSel();
}

void CExternalEditorIfObj::Down2_BoxSel()
{
	S_Down2_BoxSel();
}
/*
void CExternalEditorIfObj::Left2_BoxSel()
{
	S_Left2_BoxSel();
}
*/
/*
void CExternalEditorIfObj::Right2_BoxSel()
{
	S_Right2_BoxSel();
}
*/
void CExternalEditorIfObj::WordLeft_BoxSel()
{
	S_WordLeft_BoxSel();
}

void CExternalEditorIfObj::WordRight_BoxSel()
{
	S_WordRight_BoxSel();
}

void CExternalEditorIfObj::GoLogicalLineTop_BoxSel(const int arg1)
{
	S_GoLogicalLineTop_BoxSel(arg1);
}

void CExternalEditorIfObj::GoLineTop_BoxSel(const int arg1)
{
	S_GoLineTop_BoxSel(arg1);
}

void CExternalEditorIfObj::GoLineEnd_BoxSel(const int arg1)
{
	S_GoLineEnd_BoxSel(arg1);
}

void CExternalEditorIfObj::HalfPageUp_BoxSel()
{
	S_HalfPageUp_BoxSel();
}

void CExternalEditorIfObj::HalfPageDown_BoxSel()
{
	S_HalfPageDown_BoxSel();
}

void CExternalEditorIfObj::PageUp_BoxSel()
{
	S_PageUp_BoxSel();
}

void CExternalEditorIfObj::OnePageUp_BoxSel()
{
	S_1PageUp_BoxSel();
}

void CExternalEditorIfObj::PageDown_BoxSel()
{
	S_PageDown_BoxSel();
}

void CExternalEditorIfObj::OnePageDown_BoxSel()
{
	S_1PageDown_BoxSel();
}

void CExternalEditorIfObj::GoFileTop_BoxSel()
{
	S_GoFileTop_BoxSel();
}

void CExternalEditorIfObj::GoFileEnd_BoxSel()
{
	S_GoFileEnd_BoxSel();
}

void CExternalEditorIfObj::Cut()
{
	S_Cut();
}

void CExternalEditorIfObj::Copy()
{
	S_Copy();
}

void CExternalEditorIfObj::Paste(const int arg1)
{
	S_Paste(arg1);
}

void CExternalEditorIfObj::CopyAddCRLF()
{
	S_CopyAddCRLF();
}

void CExternalEditorIfObj::CopyCRLF()
{
	S_CopyCRLF();
}

void CExternalEditorIfObj::PasteBox(const int arg1)
{
	S_PasteBox(arg1);
}

void CExternalEditorIfObj::InsBoxText(LPCWSTR arg1)
{
	S_InsBoxText(arg1);
}

void CExternalEditorIfObj::InsText(LPCWSTR arg1)
{
	S_InsText(arg1);
}

void CExternalEditorIfObj::AddTail(LPCWSTR arg1)
{
	S_AddTail(arg1);
}

void CExternalEditorIfObj::CopyLines()
{
	S_CopyLines();
}

void CExternalEditorIfObj::CopyLinesAsPassage()
{
	S_CopyLinesAsPassage();
}

void CExternalEditorIfObj::CopyLinesWithLineNumber()
{
	S_CopyLinesWithLineNumber();
}

void CExternalEditorIfObj::CopyColorHtml()
{
	S_CopyColorHtml();
}

void CExternalEditorIfObj::CopyColorHtmlWithLineNumber()
{
	S_CopyColorHtmlWithLineNumber();
}

void CExternalEditorIfObj::CopyPath()
{
	S_CopyPath();
}

void CExternalEditorIfObj::CopyFilename()
{
	S_CopyFilename();
}

void CExternalEditorIfObj::CopyTag()
{
	S_CopyTag();
}

void CExternalEditorIfObj::CopyKeyBindList()
{
	S_CopyKeyBindList();
}

void CExternalEditorIfObj::InsertDate()
{
	S_InsertDate();
}

void CExternalEditorIfObj::InsertTime()
{
	S_InsertTime();
}

void CExternalEditorIfObj::CtrlCodeDialog()
{
	S_CtrlCodeDialog();
}

void CExternalEditorIfObj::CtrlCode(const int arg1)
{
	S_CtrlCode(arg1);
}

void CExternalEditorIfObj::ToLower()
{
	S_ToLower();
}

void CExternalEditorIfObj::ToUpper()
{
	S_ToUpper();
}

void CExternalEditorIfObj::ToHankaku()
{
	S_ToHankaku();
}

void CExternalEditorIfObj::ToHankata()
{
	S_ToHankata();
}

void CExternalEditorIfObj::ToZenEi()
{
	S_ToZenEi();
}

void CExternalEditorIfObj::ToHanEi()
{
	S_ToHanEi();
}

void CExternalEditorIfObj::ToZenKata()
{
	S_ToZenKata();
}

void CExternalEditorIfObj::ToZenHira()
{
	S_ToZenHira();
}

void CExternalEditorIfObj::HanKataToZenKata()
{
	S_HanKataToZenKata();
}

void CExternalEditorIfObj::HanKataToZenHira()
{
	S_HanKataToZenHira();
}

void CExternalEditorIfObj::TABToSPACE()
{
	S_TABToSPACE();
}

void CExternalEditorIfObj::SPACEToTAB()
{
	S_SPACEToTAB();
}

void CExternalEditorIfObj::AutoToSJIS()
{
	S_AutoToSJIS();
}

void CExternalEditorIfObj::JIStoSJIS()
{
	S_JIStoSJIS();
}

void CExternalEditorIfObj::EUCtoSJIS()
{
	S_EUCtoSJIS();
}

void CExternalEditorIfObj::CodeCnvUNICODEtoSJIS()
{
	S_CodeCnvUNICODEtoSJIS();
}

void CExternalEditorIfObj::CodeCnvUNICODEBEtoSJIS()
{
	S_CodeCnvUNICODEBEtoSJIS();
}

void CExternalEditorIfObj::UTF8toSJIS()
{
	S_UTF8toSJIS();
}

void CExternalEditorIfObj::UTF7toSJIS()
{
	S_UTF7toSJIS();
}

void CExternalEditorIfObj::SJIStoJIS()
{
	S_SJIStoJIS();
}

void CExternalEditorIfObj::SJIStoEUC()
{
	S_SJIStoEUC();
}

void CExternalEditorIfObj::SJIStoUTF8()
{
	S_SJIStoUTF8();
}

void CExternalEditorIfObj::SJIStoUTF7()
{
	S_SJIStoUTF7();
}

void CExternalEditorIfObj::Base64Decode()
{
	S_Base64Decode();
}

void CExternalEditorIfObj::Uudecode()
{
	S_Uudecode();
}

void CExternalEditorIfObj::SearchDialog()
{
	S_SearchDialog();
}

void CExternalEditorIfObj::SearchNext(LPCWSTR arg1, const int arg2)
{
	S_SearchNext(arg1, arg2);
}

void CExternalEditorIfObj::SearchPrev(LPCWSTR arg1, const int arg2)
{
	S_SearchPrev(arg1, arg2);
}

void CExternalEditorIfObj::ReplaceDialog()
{
	S_ReplaceDialog();
}

void CExternalEditorIfObj::Replace(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	S_Replace(arg1, arg2, arg3);
}

void CExternalEditorIfObj::ReplaceAll(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	S_ReplaceAll(arg1, arg2, arg3);
}

void CExternalEditorIfObj::SearchClearMark()
{
	S_SearchClearMark();
}

void CExternalEditorIfObj::SearchStartPos()
{
	S_SearchStartPos();
}

void CExternalEditorIfObj::Grep(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3, const int arg4, const int arg5)
{
	S_Grep(arg1, arg2, arg3, arg4, arg5);
}

void CExternalEditorIfObj::Jump(const int arg1, const int arg2)
{
	S_Jump(arg1, arg2);
}

void CExternalEditorIfObj::Outline(const int arg1)
{
	S_Outline(arg1);
}

void CExternalEditorIfObj::TagJump()
{
	S_TagJump();
}

void CExternalEditorIfObj::TagJumpBack()
{
	S_TagJumpBack();
}

void CExternalEditorIfObj::TagMake()
{
	S_TagMake();
}

void CExternalEditorIfObj::DirectTagJump()
{
	S_DirectTagJump();
}

void CExternalEditorIfObj::KeywordTagJump(LPCWSTR arg1)
{
	S_KeywordTagJump(arg1);
}

void CExternalEditorIfObj::Compare()
{
	S_Compare();
}

void CExternalEditorIfObj::DiffDialog()
{
	S_DiffDialog();
}

void CExternalEditorIfObj::Diff(LPCWSTR arg1, const int arg2)
{
	S_Diff(arg1, arg2);
}

void CExternalEditorIfObj::DiffNext()
{
	S_DiffNext();
}

void CExternalEditorIfObj::DiffPrev()
{
	S_DiffPrev();
}

void CExternalEditorIfObj::DiffReset()
{
	S_DiffReset();
}

void CExternalEditorIfObj::BracketPair()
{
	S_BracketPair();
}

void CExternalEditorIfObj::BookmarkSet()
{
	S_BookmarkSet();
}

void CExternalEditorIfObj::BookmarkNext()
{
	S_BookmarkNext();
}

void CExternalEditorIfObj::BookmarkPrev()
{
	S_BookmarkPrev();
}

void CExternalEditorIfObj::BookmarkReset()
{
	S_BookmarkReset();
}

void CExternalEditorIfObj::BookmarkView()
{
	S_BookmarkView();
}

void CExternalEditorIfObj::BookmarkPattern(LPCWSTR arg1, const int arg2)
{
	S_BookmarkPattern(arg1, arg2);
}

void CExternalEditorIfObj::TagJumpEx(LPCWSTR arg1, const int arg2, const int arg3, const int arg4)
{
	S_TagJumpEx(arg1, arg2, arg3, arg4);
}

void CExternalEditorIfObj::ChgmodINS()
{
	S_ChgmodINS();
}

void CExternalEditorIfObj::ChgCharSet(const int arg1, const int arg2)
{
	S_ChgCharSet(arg1, arg2);
}

void CExternalEditorIfObj::ChgmodEOL(const int arg1)
{
	S_ChgmodEOL(arg1);
}

void CExternalEditorIfObj::CancelMode()
{
	S_CancelMode();
}

void CExternalEditorIfObj::ExecExternalMacro(LPCWSTR arg1, LPCWSTR arg2)
{
	S_ExecExternalMacro(arg1, arg2);
}

void CExternalEditorIfObj::ShowToolbar()
{
	S_ShowToolbar();
}

void CExternalEditorIfObj::ShowFunckey()
{
	S_ShowFunckey();
}

void CExternalEditorIfObj::ShowTab()
{
	S_ShowTab();
}

void CExternalEditorIfObj::ShowStatusbar()
{
	S_ShowStatusbar();
}

void CExternalEditorIfObj::TypeList()
{
	S_TypeList();
}

void CExternalEditorIfObj::ChangeType(const int arg1)
{
	S_ChangeType(arg1);
}

void CExternalEditorIfObj::OptionType()
{
	S_OptionType();
}

void CExternalEditorIfObj::OptionCommon()
{
	S_OptionCommon();
}

void CExternalEditorIfObj::SelectFont()
{
	S_SelectFont();
}

void CExternalEditorIfObj::SetFontSize(const int arg1, const int arg2, const int arg3)
{
	S_SetFontSize(arg1, arg2, arg3);
}

void CExternalEditorIfObj::WrapWindowWidth()
{
	S_WrapWindowWidth();
}

void CExternalEditorIfObj::OptionFavorite()
{
	S_OptionFavorite();
}

void CExternalEditorIfObj::SetMsgQuoteStr(LPCWSTR arg1)
{
	S_SetMsgQuoteStr(arg1);
}

void CExternalEditorIfObj::TextWrapMethod(const int arg1)
{
	S_TextWrapMethod(arg1);
}

void CExternalEditorIfObj::SelectCountMode(const int arg1)
{
	S_SelectCountMode(arg1);
}

void CExternalEditorIfObj::ExecCommand(LPCWSTR arg1, const int arg2, LPCWSTR arg3)
{
	S_ExecCommand(arg1, arg2, arg3);
}

void CExternalEditorIfObj::ExecCommandDialog()
{
	S_ExecCommandDialog();
}

void CExternalEditorIfObj::RMenu()
{
	S_RMenu();
}

void CExternalEditorIfObj::CustMenu1()
{
	S_CustMenu1();
}

void CExternalEditorIfObj::CustMenu2()
{
	S_CustMenu2();
}

void CExternalEditorIfObj::CustMenu3()
{
	S_CustMenu3();
}

void CExternalEditorIfObj::CustMenu4()
{
	S_CustMenu4();
}

void CExternalEditorIfObj::CustMenu5()
{
	S_CustMenu5();
}

void CExternalEditorIfObj::CustMenu6()
{
	S_CustMenu6();
}

void CExternalEditorIfObj::CustMenu7()
{
	S_CustMenu7();
}

void CExternalEditorIfObj::CustMenu8()
{
	S_CustMenu8();
}

void CExternalEditorIfObj::CustMenu9()
{
	S_CustMenu9();
}

void CExternalEditorIfObj::CustMenu10()
{
	S_CustMenu10();
}

void CExternalEditorIfObj::CustMenu11()
{
	S_CustMenu11();
}

void CExternalEditorIfObj::CustMenu12()
{
	S_CustMenu12();
}

void CExternalEditorIfObj::CustMenu13()
{
	S_CustMenu13();
}

void CExternalEditorIfObj::CustMenu14()
{
	S_CustMenu14();
}

void CExternalEditorIfObj::CustMenu15()
{
	S_CustMenu15();
}

void CExternalEditorIfObj::CustMenu16()
{
	S_CustMenu16();
}

void CExternalEditorIfObj::CustMenu17()
{
	S_CustMenu17();
}

void CExternalEditorIfObj::CustMenu18()
{
	S_CustMenu18();
}

void CExternalEditorIfObj::CustMenu19()
{
	S_CustMenu19();
}

void CExternalEditorIfObj::CustMenu20()
{
	S_CustMenu20();
}

void CExternalEditorIfObj::CustMenu21()
{
	S_CustMenu21();
}

void CExternalEditorIfObj::CustMenu22()
{
	S_CustMenu22();
}

void CExternalEditorIfObj::CustMenu23()
{
	S_CustMenu23();
}

void CExternalEditorIfObj::CustMenu24()
{
	S_CustMenu24();
}

void CExternalEditorIfObj::SplitWinV()
{
	S_SplitWinV();
}

void CExternalEditorIfObj::SplitWinH()
{
	S_SplitWinH();
}

void CExternalEditorIfObj::SplitWinVH()
{
	S_SplitWinVH();
}

void CExternalEditorIfObj::WinClose()
{
	S_WinClose();
}

void CExternalEditorIfObj::WinCloseAll()
{
	S_WinCloseAll();
}

void CExternalEditorIfObj::CascadeWin()
{
	S_CascadeWin();
}

void CExternalEditorIfObj::TileWinV()
{
	S_TileWinV();
}

void CExternalEditorIfObj::TileWinH()
{
	S_TileWinH();
}

void CExternalEditorIfObj::NextWindow()
{
	S_NextWindow();
}

void CExternalEditorIfObj::PrevWindow()
{
	S_PrevWindow();
}

void CExternalEditorIfObj::WindowList()
{
	S_WindowList();
}

void CExternalEditorIfObj::MaximizeV()
{
	S_MaximizeV();
}

void CExternalEditorIfObj::MaximizeH()
{
	S_MaximizeH();
}

void CExternalEditorIfObj::MinimizeAll()
{
	S_MinimizeAll();
}

void CExternalEditorIfObj::ReDraw()
{
	S_ReDraw();
}

void CExternalEditorIfObj::ActivateWinOutput()
{
	S_ActivateWinOutput();
}

void CExternalEditorIfObj::TraceOut(LPCWSTR arg1, const int arg2)
{
	S_TraceOut(arg1, arg2);
}

void CExternalEditorIfObj::WindowTopMost(const int arg1)
{
	S_WindowTopMost(arg1);
}

void CExternalEditorIfObj::GroupClose()
{
	S_GroupClose();
}

void CExternalEditorIfObj::NextGroup()
{
	S_NextGroup();
}

void CExternalEditorIfObj::PrevGroup()
{
	S_PrevGroup();
}

void CExternalEditorIfObj::TabMoveRight()
{
	S_TabMoveRight();
}

void CExternalEditorIfObj::TabMoveLeft()
{
	S_TabMoveLeft();
}

void CExternalEditorIfObj::TabSeparate()
{
	S_TabSeparate();
}

void CExternalEditorIfObj::TabJointNext()
{
	S_TabJointNext();
}

void CExternalEditorIfObj::TabJointPrev()
{
	S_TabJointPrev();
}

void CExternalEditorIfObj::TabCloseOther()
{
	S_TabCloseOther();
}

void CExternalEditorIfObj::TabCloseLeft()
{
	S_TabCloseLeft();
}

void CExternalEditorIfObj::TabCloseRight()
{
	S_TabCloseRight();
}

void CExternalEditorIfObj::Complete()
{
	S_Complete();
}

void CExternalEditorIfObj::ToggleKeyHelpSearch(const int arg1)
{
	S_ToggleKeyHelpSearch(arg1);
}

void CExternalEditorIfObj::HelpContents()
{
	S_HelpContents();
}

void CExternalEditorIfObj::HelpSearch()
{
	S_HelpSearch();
}

void CExternalEditorIfObj::CommandList()
{
	S_CommandList();
}

void CExternalEditorIfObj::ExtHelp1()
{
	S_ExtHelp1();
}

void CExternalEditorIfObj::ExtHtmlHelp(LPCWSTR arg1, LPCWSTR arg2)
{
	S_ExtHtmlHelp(arg1, arg2);
}

void CExternalEditorIfObj::About()
{
	S_About();
}

void CExternalEditorIfObj::StatusMsg(LPCWSTR arg1, const int arg2)
{
	S_StatusMsg(arg1, arg2);
}

void CExternalEditorIfObj::MsgBeep(const int arg1)
{
	S_MsgBeep(arg1);
}

void CExternalEditorIfObj::CommitUndoBuffer()
{
	S_CommitUndoBuffer();
}

void CExternalEditorIfObj::AddRefUndoBuffer()
{
	S_AddRefUndoBuffer();
}

void CExternalEditorIfObj::SetUndoBuffer()
{
	S_SetUndoBuffer();
}

void CExternalEditorIfObj::AppendUndoBufferCursor()
{
	S_AppendUndoBufferCursor();
}

void CExternalEditorIfObj::ClipboardEmpty()
{
	S_ClipboardEmpty();
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////
WideString CExternalEditorIfObj::S_ExpandParameter(const WideString& arg1)
{
	return S_ExpandParameter(arg1.c_str());
}

int CExternalEditorIfObj::S_IsCurTypeExt(const WideString& arg1)
{
	return S_IsCurTypeExt(arg1.c_str());
}

int CExternalEditorIfObj::S_IsSameTypeExt(const WideString& arg1, const WideString& arg2)
{
	return S_IsSameTypeExt(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_InputBox(const WideString& arg1, const WideString& arg2, const int arg3)
{
	return S_InputBox(arg1.c_str(), arg2.c_str(), arg3);
}

int CExternalEditorIfObj::S_MessageBox(const WideString& arg1, const int arg2)
{
	return S_MessageBox(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::S_ErrorMsg(const WideString& arg1)
{
	return S_ErrorMsg(arg1.c_str());
}

int CExternalEditorIfObj::S_WarnMsg(const WideString& arg1)
{
	return S_WarnMsg(arg1.c_str());
}

int CExternalEditorIfObj::S_InfoMsg(const WideString& arg1)
{
	return S_InfoMsg(arg1.c_str());
}

int CExternalEditorIfObj::S_OkCancelBox(const WideString& arg1)
{
	return S_OkCancelBox(arg1.c_str());
}

int CExternalEditorIfObj::S_YesNoBox(const WideString& arg1)
{
	return S_YesNoBox(arg1.c_str());
}

int CExternalEditorIfObj::S_CompareVersion(const WideString& arg1, const WideString& arg2)
{
	return S_CompareVersion(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_FileOpenDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FileOpenDialog(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_FileSaveDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FileSaveDialog(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_FolderDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FolderDialog(arg1.c_str(), arg2.c_str());
}

int CExternalEditorIfObj::S_SetClipboard(const int arg1, const WideString& arg2)
{
	return S_SetClipboard(arg1, arg2.c_str());
}

WideString CExternalEditorIfObj::S_GetCookie(const WideString& arg1, const WideString& arg2)
{
	return S_GetCookie(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_GetCookieDefault(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	return S_GetCookieDefault(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

int CExternalEditorIfObj::S_SetCookie(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	return S_SetCookie(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

int CExternalEditorIfObj::S_DeleteCookie(const WideString& arg1, const WideString& arg2)
{
	return S_DeleteCookie(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::S_GetCookieNames(const WideString& arg1)
{
	return S_GetCookieNames(arg1.c_str());
}

int CExternalEditorIfObj::S_GetStrWidth(const WideString& arg1, const int arg2)
{
	return S_GetStrWidth(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::S_GetStrLayoutLength(const WideString& arg1, const int arg2)
{
	return S_GetStrLayoutLength(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::S_IsIncludeClipboardFormat(const WideString& arg1)
{
	return S_IsIncludeClipboardFormat(arg1.c_str());
}

WideString CExternalEditorIfObj::S_GetClipboardByFormat(const WideString& arg1, const int arg2, const int arg3)
{
	return S_GetClipboardByFormat(arg1.c_str(), arg2, arg3);
}

int CExternalEditorIfObj::S_SetClipboardByFormat(const WideString& arg1, const WideString& arg2, const int arg3, const int arg4)
{
	return S_SetClipboardByFormat(arg1.c_str(), arg2.c_str(), arg3, arg4);
}

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////
void CExternalEditorIfObj::S_FileOpen(const WideString& arg1, const int arg2, const int arg3, const WideString& arg4)
{
	S_FileOpen(arg1.c_str(), arg2, arg3, arg4.c_str());
}

void CExternalEditorIfObj::S_FileSaveAs(const WideString& arg1, const int arg2, const int arg3)
{
	S_FileSaveAs(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::S_PutFile(const WideString& arg1, const int arg2, const int arg3)
{
	S_PutFile(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::S_InsFile(const WideString& arg1, const int arg2, const int arg3)
{
	S_InsFile(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::S_InsBoxText(const WideString& arg1)
{
	S_InsBoxText(arg1.c_str());
}

void CExternalEditorIfObj::S_InsText(const WideString& arg1)
{
	S_InsText(arg1.c_str());
}

void CExternalEditorIfObj::S_AddTail(const WideString& arg1)
{
	S_AddTail(arg1.c_str());
}

void CExternalEditorIfObj::S_SearchNext(const WideString& arg1, const int arg2)
{
	S_SearchNext(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::S_SearchPrev(const WideString& arg1, const int arg2)
{
	S_SearchPrev(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::S_Replace(const WideString& arg1, const WideString& arg2, const int arg3)
{
	S_Replace(arg1.c_str(), arg2.c_str(), arg3);
}

void CExternalEditorIfObj::S_ReplaceAll(const WideString& arg1, const WideString& arg2, const int arg3)
{
	S_ReplaceAll(arg1.c_str(), arg2.c_str(), arg3);
}

void CExternalEditorIfObj::S_Grep(const WideString& arg1, const WideString& arg2, const WideString& arg3, const int arg4, const int arg5)
{
	S_Grep(arg1.c_str(), arg2.c_str(), arg3.c_str(), arg4, arg5);
}

void CExternalEditorIfObj::S_KeywordTagJump(const WideString& arg1)
{
	S_KeywordTagJump(arg1.c_str());
}

void CExternalEditorIfObj::S_Diff(const WideString& arg1, const int arg2)
{
	S_Diff(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::S_BookmarkPattern(const WideString& arg1, const int arg2)
{
	S_BookmarkPattern(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::S_TagJumpEx(const WideString& arg1, const int arg2, const int arg3, const int arg4)
{
	S_TagJumpEx(arg1.c_str(), arg2, arg3, arg4);
}

void CExternalEditorIfObj::S_ExecExternalMacro(const WideString& arg1, const WideString& arg2)
{
	S_ExecExternalMacro(arg1.c_str(), arg2.c_str());
}

void CExternalEditorIfObj::S_SetMsgQuoteStr(const WideString& arg1)
{
	S_SetMsgQuoteStr(arg1.c_str());
}

void CExternalEditorIfObj::S_ExecCommand(const WideString& arg1, const int arg2, const WideString& arg3)
{
	S_ExecCommand(arg1.c_str(), arg2, arg3.c_str());
}

void CExternalEditorIfObj::S_TraceOut(const WideString& arg1, const int arg2)
{
	S_TraceOut(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::S_ExtHtmlHelp(const WideString& arg1, const WideString& arg2)
{
	S_ExtHtmlHelp(arg1.c_str(), arg2.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// Aliases
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////
WideString CExternalEditorIfObj::ExpandParameter(const WideString& arg1)
{
	return S_ExpandParameter(arg1.c_str());
}

int CExternalEditorIfObj::IsCurTypeExt(const WideString& arg1)
{
	return S_IsCurTypeExt(arg1.c_str());
}

int CExternalEditorIfObj::IsSameTypeExt(const WideString& arg1, const WideString& arg2)
{
	return S_IsSameTypeExt(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::InputBox(const WideString& arg1, const WideString& arg2, const int arg3)
{
	return S_InputBox(arg1.c_str(), arg2.c_str(), arg3);
}

int CExternalEditorIfObj::MessageBox(const WideString& arg1, const int arg2)
{
	return S_MessageBox(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::ErrorMsg(const WideString& arg1)
{
	return S_ErrorMsg(arg1.c_str());
}

int CExternalEditorIfObj::WarnMsg(const WideString& arg1)
{
	return S_WarnMsg(arg1.c_str());
}

int CExternalEditorIfObj::InfoMsg(const WideString& arg1)
{
	return S_InfoMsg(arg1.c_str());
}

int CExternalEditorIfObj::OkCancelBox(const WideString& arg1)
{
	return S_OkCancelBox(arg1.c_str());
}

int CExternalEditorIfObj::YesNoBox(const WideString& arg1)
{
	return S_YesNoBox(arg1.c_str());
}

int CExternalEditorIfObj::CompareVersion(const WideString& arg1, const WideString& arg2)
{
	return S_CompareVersion(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::FileOpenDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FileOpenDialog(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::FileSaveDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FileSaveDialog(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::FolderDialog(const WideString& arg1, const WideString& arg2)
{
	return S_FolderDialog(arg1.c_str(), arg2.c_str());
}

int CExternalEditorIfObj::SetClipboard(const int arg1, const WideString& arg2)
{
	return S_SetClipboard(arg1, arg2.c_str());
}

WideString CExternalEditorIfObj::GetCookie(const WideString& arg1, const WideString& arg2)
{
	return S_GetCookie(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::GetCookieDefault(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	return S_GetCookieDefault(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

int CExternalEditorIfObj::SetCookie(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	return S_SetCookie(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

int CExternalEditorIfObj::DeleteCookie(const WideString& arg1, const WideString& arg2)
{
	return S_DeleteCookie(arg1.c_str(), arg2.c_str());
}

WideString CExternalEditorIfObj::GetCookieNames(const WideString& arg1)
{
	return S_GetCookieNames(arg1.c_str());
}

int CExternalEditorIfObj::GetStrWidth(const WideString& arg1, const int arg2)
{
	return S_GetStrWidth(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::GetStrLayoutLength(const WideString& arg1, const int arg2)
{
	return S_GetStrLayoutLength(arg1.c_str(), arg2);
}

int CExternalEditorIfObj::IsIncludeClipboardFormat(const WideString& arg1)
{
	return S_IsIncludeClipboardFormat(arg1.c_str());
}

WideString CExternalEditorIfObj::GetClipboardByFormat(const WideString& arg1, const int arg2, const int arg3)
{
	return S_GetClipboardByFormat(arg1.c_str(), arg2, arg3);
}

int CExternalEditorIfObj::SetClipboardByFormat(const WideString& arg1, const WideString& arg2, const int arg3, const int arg4)
{
	return S_SetClipboardByFormat(arg1.c_str(), arg2.c_str(), arg3, arg4);
}

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////
void CExternalEditorIfObj::FileOpen(const WideString& arg1, const int arg2, const int arg3, const WideString& arg4)
{
	S_FileOpen(arg1.c_str(), arg2, arg3, arg4.c_str());
}

void CExternalEditorIfObj::FileSaveAs(const WideString& arg1, const int arg2, const int arg3)
{
	S_FileSaveAs(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::PutFile(const WideString& arg1, const int arg2, const int arg3)
{
	S_PutFile(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::InsFile(const WideString& arg1, const int arg2, const int arg3)
{
	S_InsFile(arg1.c_str(), arg2, arg3);
}

void CExternalEditorIfObj::InsBoxText(const WideString& arg1)
{
	S_InsBoxText(arg1.c_str());
}

void CExternalEditorIfObj::InsText(const WideString& arg1)
{
	S_InsText(arg1.c_str());
}

void CExternalEditorIfObj::AddTail(const WideString& arg1)
{
	S_AddTail(arg1.c_str());
}

void CExternalEditorIfObj::SearchNext(const WideString& arg1, const int arg2)
{
	S_SearchNext(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::SearchPrev(const WideString& arg1, const int arg2)
{
	S_SearchPrev(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::Replace(const WideString& arg1, const WideString& arg2, const int arg3)
{
	S_Replace(arg1.c_str(), arg2.c_str(), arg3);
}

void CExternalEditorIfObj::ReplaceAll(const WideString& arg1, const WideString& arg2, const int arg3)
{
	S_ReplaceAll(arg1.c_str(), arg2.c_str(), arg3);
}

void CExternalEditorIfObj::Grep(const WideString& arg1, const WideString& arg2, const WideString& arg3, const int arg4, const int arg5)
{
	S_Grep(arg1.c_str(), arg2.c_str(), arg3.c_str(), arg4, arg5);
}

void CExternalEditorIfObj::KeywordTagJump(const WideString& arg1)
{
	S_KeywordTagJump(arg1.c_str());
}

void CExternalEditorIfObj::Diff(const WideString& arg1, const int arg2)
{
	S_Diff(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::BookmarkPattern(const WideString& arg1, const int arg2)
{
	S_BookmarkPattern(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::TagJumpEx(const WideString& arg1, const int arg2, const int arg3, const int arg4)
{
	S_TagJumpEx(arg1.c_str(), arg2, arg3, arg4);
}

void CExternalEditorIfObj::ExecExternalMacro(const WideString& arg1, const WideString& arg2)
{
	S_ExecExternalMacro(arg1.c_str(), arg2.c_str());
}

void CExternalEditorIfObj::SetMsgQuoteStr(const WideString& arg1)
{
	S_SetMsgQuoteStr(arg1.c_str());
}

void CExternalEditorIfObj::ExecCommand(const WideString& arg1, const int arg2, const WideString& arg3)
{
	S_ExecCommand(arg1.c_str(), arg2, arg3.c_str());
}

void CExternalEditorIfObj::TraceOut(const WideString& arg1, const int arg2)
{
	S_TraceOut(arg1.c_str(), arg2);
}

void CExternalEditorIfObj::ExtHtmlHelp(const WideString& arg1, const WideString& arg2)
{
	S_ExtHtmlHelp(arg1.c_str(), arg2.c_str());
}
