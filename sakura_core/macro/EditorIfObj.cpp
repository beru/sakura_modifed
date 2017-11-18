/*!	@file
	@brief Editor�I�u�W�F�N�g
*/
#include "StdAfx.h"
#include "EditorIfObj.h"
#include "macro/Macro.h"
#include "macro/SMacroMgr.h"

// �R�}���h�����擾����
MacroFuncInfoArray EditorIfObj::GetMacroCommandInfo() const
{
	return SMacroMgr::macroFuncInfoCommandArr;
}

// �֐������擾����
MacroFuncInfoArray EditorIfObj::GetMacroFuncInfo() const
{
	return SMacroMgr::macroFuncInfoArr;
}

// �֐�����������
bool EditorIfObj::HandleFunction(
	EditView& view,
	EFunctionCode index,
	const VARIANT* arguments,
	const int argSize,
	VARIANT& result
	)
{
	return Macro::HandleFunction(view, index, arguments, argSize, result);
}

// �R�}���h����������
bool EditorIfObj::HandleCommand(
	EditView& view,
	EFunctionCode index,
	const wchar_t* arguments[],
	const int argLengths[],
	const int argSize
	)
{
	return Macro::HandleCommand(view, index, arguments, argLengths, argSize);
}

