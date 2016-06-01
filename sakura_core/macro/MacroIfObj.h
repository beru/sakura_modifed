/*!	@file
	@brief �}�N���I�u�W�F�N�g�N���X
*/
/*
	Copyright (C) 2013, Plugins developers

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
#include "EditApp.h"

// �}�N��WSH�I�u�W�F�N�g
class MacroIfObj : public WSHIfObj {
	// �^��`
	enum FuncId {
		F_MA_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_MA_SET_MATCH,							//
		F_MA_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_MA_GET_MODE,							// ���[�h���擾����
		F_MA_GET_FLAGS,							// flags���擾����
		F_MA_GET_EXT,							// Ext���擾����
		F_MA_GET_SOURCE,						// Source���擾����
		F_MA_GET_INDEX,							// �}�N���C���f�b�N�X�ԍ����擾����
	};
	typedef std::string string;
	typedef std::wstring wstring;

public:
	enum tagModeID {
		MACRO_MODE_CREATOR = 0,
		MACRO_MODE_EXEC
	};

	// �R���X�g���N�^
public:
	MacroIfObj(tagModeID nMode, LPCWSTR Ext, int flags, LPCWSTR Source)
		: WSHIfObj(L"Macro", false)
	{
		nMode = nMode;
		ext = Ext;
		nFlags = flags | FA_FROMMACRO;
		nIsMatch = 0;
		source = Source;
		nIndex = INVALID_MACRO_IDX;
		if (nMode == MACRO_MODE_EXEC) {
			// �Ăяo���̒��O�Őݒ肳��Ă���ԍ���ۑ�����
			nIndex = EditApp::getInstance().pSMacroMgr->GetCurrentIdx();
		}
	}

	// �f�X�g���N�^
public:
	virtual ~MacroIfObj() {}

	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const {
		static MacroFuncInfo macroFuncInfoArr[] = {
			// index									�֐���						����										�߂�l�̌^	pszData
			{ EFunctionCode(F_MA_SET_MATCH),		LTEXT("SetMatch"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// flags���擾����
			// �I�[
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, nullptr }
		};
		return macroFuncInfoArr;
	}

	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			// index									�֐���						����										�߂�l�̌^	pszData
			{ EFunctionCode(F_MA_GET_MODE),			LTEXT("GetMode"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// ���[�h���擾����
			{ EFunctionCode(F_MA_GET_FLAGS),		LTEXT("GetFlags"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// flags���擾����
			{ EFunctionCode(F_MA_GET_EXT),			LTEXT("GetExt"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	nullptr },	// Ext���擾����
			{ EFunctionCode(F_MA_GET_SOURCE),		LTEXT("GetSource"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	nullptr },	// Source���擾����
			{ EFunctionCode(F_MA_GET_INDEX),		LTEXT("GetIndex"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// �}�N��Index���擾����
			// �I�[
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, nullptr }
		};
		return macroFuncInfoNotCommandArr;
	}

	// �֐�����������
	virtual
	bool HandleFunction(
		EditView&		view,
		EFunctionCode	index,
		const VARIANT*	arguments,
		const int		argSize,
		VARIANT&		result
		)
	{
		switch (LOWORD(index)) {
		case F_MA_GET_MODE:
			{
				Wrap(&result)->Receive(nMode);
			}
			return true;
		case F_MA_GET_FLAGS:	// flags���擾����
			{
				Wrap(&result)->Receive(nFlags);
			}
			return true;
		case F_MA_GET_EXT:
			{
				SysString s(ext.c_str(), ext.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_MA_GET_SOURCE:
			{
				SysString s(source.c_str(), source.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_MA_GET_INDEX:
			{
				Wrap(&result)->Receive(nIndex);
				//Wrap(&result)->Receive(CEditApp::getInstance()->pSMacroMgr->GetCurrentIdx());
			}
			return true;
		}
		return false;
	}

	// �R�}���h����������
	virtual
	bool HandleCommand(
		EditView&		view,
		EFunctionCode	index,
		const wchar_t*	arguments[],
		const int		argLengths[],
		const int		argSize
		)
	{
		switch (LOWORD(index)) {
		case F_MA_SET_MATCH:
			if (arguments[0]) {
				nIsMatch = _wtol(arguments[0]);
			}
			return true;
		}
		return false;
	}

	bool IsMatch() {
		return (nIsMatch != 0);
	}

	void SetMatch(const int nMatch) {
		nIsMatch = nMatch;
	}

	// �����o�ϐ�
public:
	tagModeID nMode;
	int nIsMatch;
	int nFlags;
	std::wstring ext;
	std::wstring source;
	int nIndex;
};

