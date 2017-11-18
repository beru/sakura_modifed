/*!	@file
	@brief Editor�I�u�W�F�N�g
*/
#pragma once

#include "_os/OleTypes.h"
#include "macro/WSHIfObj.h"


class EditorIfObj : public WSHIfObj {
	// �R���X�g���N�^
public:
	EditorIfObj() : WSHIfObj(L"Editor", true) {}

	// ����
	MacroFuncInfoArray GetMacroCommandInfo() const;	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroFuncInfo() const;	// �֐������擾����
	bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* arguments, const int argSize, VARIANT& result);	// �֐�����������
	bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize);	// �R�}���h����������
};

