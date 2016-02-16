/*!	@file
	@brief Editor�I�u�W�F�N�g

*/
/*
	Copyright (C) 2009, syat

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
#include "EditorIfObj.h"
#include "macro/Macro.h"
#include "macro/SMacroMgr.h"

// �R�}���h�����擾����
MacroFuncInfoArray EditorIfObj::GetMacroCommandInfo() const
{
	return SMacroMgr::m_macroFuncInfoCommandArr;
}

// �֐������擾����
MacroFuncInfoArray EditorIfObj::GetMacroFuncInfo() const
{
	return SMacroMgr::m_macroFuncInfoArr;
}

// �֐�����������
bool EditorIfObj::HandleFunction(
	EditView* pView,
	EFunctionCode id,
	const VARIANT* arguments,
	const int argSize,
	VARIANT& result
	)
{
	return Macro::HandleFunction(pView, id, arguments, argSize, result);
}

// �R�}���h����������
bool EditorIfObj::HandleCommand(
	EditView* pView,
	EFunctionCode id,
	const WCHAR* arguments[],
	const int argLengths[],
	const int argSize
	)
{
	return Macro::HandleCommand(pView, id, arguments, argLengths, argSize);
}

