/*!	@file
	@brief Complement�I�u�W�F�N�g
*/
#pragma once

#include "macro/WSHIfObj.h"
#include "util/ole_convert.h"

class ComplementIfObj : public WSHIfObj {
	// �^��`
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					// ���R�}���h�͈ȉ��ɒǉ�����
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ���֐��͈ȉ��ɒǉ�����
		F_CM_GETCURRENTWORD,					// �⊮�Ώۂ̕�������擾
		F_CM_GETOPTION,							// �I�v�V�������擾
		F_CM_ADDLIST,							// ���ɒǉ�
	};

	// �R���X�g���N�^
public:
	ComplementIfObj(std::wstring& curWord, HokanMgr& mgr, int option)
		:
		WSHIfObj(L"Complement", false),
		currentWord(curWord),
		hokanMgr(mgr),
		nOption(option)
	{
	}

	// �f�X�g���N�^
public:
	~ComplementIfObj() {}

	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const { return macroFuncInfoCommandArr; }
	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const { return macroFuncInfoArr; };
	// �֐�����������
	bool HandleFunction(
		EditView&		view,
		EFunctionCode	index,
		const VARIANT*	arguments,
		const int		argSize,
		VARIANT&		result
	) {
		Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

		switch (LOWORD(index)) {
		case F_CM_GETCURRENTWORD:	// �⊮�Ώۂ̕�������擾
			{
				SysString s(currentWord.c_str(), currentWord.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_CM_GETOPTION:	// �I�v�V�������擾
			{
				Wrap(&result)->Receive(nOption);
			}
			return true;
		case F_CM_ADDLIST:		// ���ɒǉ�����
			{
				std::wstring keyword;
				if (!variant_to_wstr(arguments[0], keyword)) {
					return false;
				}
				const wchar_t* word = keyword.c_str();
				size_t nWordLen = keyword.length();
				if (nWordLen <= 0) {
					return false;
				}
				std::wstring strWord = std::wstring(word, nWordLen);
				if (HokanMgr::AddKouhoUnique(hokanMgr.vKouho, strWord)) {
					Wrap(&result)->Receive((int)hokanMgr.vKouho.size());
				}else {
					Wrap(&result)->Receive(-1);
				}
				return true;
			}
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
private:
	std::wstring currentWord;
	HokanMgr& hokanMgr;
	int nOption; // 0x01 == IgnoreCase

private:
	static MacroFuncInfo macroFuncInfoCommandArr[];	// �R�}���h���(�߂�l�Ȃ�)
	static MacroFuncInfo macroFuncInfoArr[];			// �֐����(�߂�l����)
};

// �R�}���h���
MacroFuncInfo ComplementIfObj::macroFuncInfoCommandArr[] = {
	//ID									�֐���							����										�߂�l�̌^	pszData
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// �֐����
MacroFuncInfo ComplementIfObj::macroFuncInfoArr[] = {
	//ID								�֐���				����										�߂�l�̌^	pszData
	{EFunctionCode(F_CM_GETCURRENTWORD),L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �⊮�Ώۂ̕�������擾
	{EFunctionCode(F_CM_GETOPTION),		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �⊮�Ώۂ̕�������擾
	{EFunctionCode(F_CM_ADDLIST),		L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // ���ɒǉ�����
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

