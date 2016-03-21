/*!	@file
	@brief Complement�I�u�W�F�N�g

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2011, Moca
	

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
		m_currentWord(curWord),
		m_hokanMgr(mgr),
		m_nOption(option)
	{
	}

	// �f�X�g���N�^
public:
	~ComplementIfObj() {}

	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const { return m_macroFuncInfoCommandArr; }
	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const { return m_macroFuncInfoArr; };
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
				SysString s(m_currentWord.c_str(), m_currentWord.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_CM_GETOPTION:	// �I�v�V�������擾
			{
				Wrap(&result)->Receive(m_nOption);
			}
			return true;
		case F_CM_ADDLIST:		// ���ɒǉ�����
			{
				std::wstring keyword;
				if (!variant_to_wstr(arguments[0], keyword)) {
					return false;
				}
				const wchar_t* word = keyword.c_str();
				int nWordLen = keyword.length();
				if (nWordLen <= 0) {
					return false;
				}
				std::wstring strWord = std::wstring(word, nWordLen);
				if (HokanMgr::AddKouhoUnique(m_hokanMgr.m_vKouho, strWord)) {
					Wrap(&result)->Receive(m_hokanMgr.m_vKouho.size());
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
		const WCHAR* arguments[],
		const int argLengths[],
		const int argSize
		)
	{
		return false;
	}

	// �����o�ϐ�
private:
	std::wstring m_currentWord;
	HokanMgr& m_hokanMgr;
	int m_nOption; // 0x01 == IgnoreCase

private:
	static MacroFuncInfo m_macroFuncInfoCommandArr[];	// �R�}���h���(�߂�l�Ȃ�)
	static MacroFuncInfo m_macroFuncInfoArr[];			// �֐����(�߂�l����)
};

// �R�}���h���
MacroFuncInfo ComplementIfObj::m_macroFuncInfoCommandArr[] = {
	//ID									�֐���							����										�߂�l�̌^	m_pszData
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// �֐����
MacroFuncInfo ComplementIfObj::m_macroFuncInfoArr[] = {
	//ID								�֐���				����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_CM_GETCURRENTWORD),L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �⊮�Ώۂ̕�������擾
	{EFunctionCode(F_CM_GETOPTION),		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �⊮�Ώۂ̕�������擾
	{EFunctionCode(F_CM_ADDLIST),		L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // ���ɒǉ�����
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

