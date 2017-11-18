/*!	@file
	@brief SmartIndent�I�u�W�F�N�g
*/
#pragma once

#include "macro/WSHIfObj.h"

// �X�}�[�g�C���f���g�pWSH�I�u�W�F�N�g
class SmartIndentIfObj : public WSHIfObj {
	// �^��`
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					// ���R�}���h�͈ȉ��ɒǉ�����
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ���֐��͈ȉ��ɒǉ�����
		F_SI_GETCHAR							// ���������L�[���擾����
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	SmartIndentIfObj(wchar_t ch)
		: WSHIfObj(L"Indent", false)
		, wcChar(ch)
	{
	}

	// �f�X�g���N�^
public:
	~SmartIndentIfObj() {}

	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const {
		static MacroFuncInfo macroFuncInfoArr[] = {
			//	�I�[
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoArr;
	}
	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			//index									�֐���							����										�߂�l�̌^	pszData
			{EFunctionCode(F_SI_GETCHAR),			LTEXT("GetChar"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, // ���������L�[���擾����
			//	�I�[
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoNotCommandArr;
	}
	// �֐�����������
	bool HandleFunction(
		EditView& view,
		EFunctionCode index,
		const VARIANT* arguments,
		const int argSize,
		VARIANT& result
		)
	{
		switch (LOWORD(index)) {
		case F_SI_GETCHAR:						// ���������L�[���擾����
			{
				wstring value;
				value += wcChar;
				SysString s(value.c_str(), value.size());
				Wrap(&result)->Receive(s);
			}
			return true;
		}
		return false;
	}
	// �R�}���h����������
	bool HandleCommand(
		EditView& view,
		EFunctionCode index,
		const wchar_t* arguments[],
		const int argLengths[],
		const int argSize
		)
	{
		return false;
	}

	// �����o�ϐ�
public:
	wchar_t wcChar;
};

