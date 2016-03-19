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
		m_nMode = nMode;
		m_Ext = Ext;
		m_nFlags = flags | FA_FROMMACRO;
		m_nIsMatch = 0;
		m_source = Source;
		m_nIndex = INVALID_MACRO_IDX;
		if (nMode == MACRO_MODE_EXEC) {
			// �Ăяo���̒��O�Őݒ肳��Ă���ԍ���ۑ�����
			m_nIndex = EditApp::getInstance().m_pSMacroMgr->GetCurrentIdx();
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
			// ID									�֐���						����										�߂�l�̌^	m_pszData
			{ EFunctionCode(F_MA_SET_MATCH),		LTEXT("SetMatch"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// flags���擾����
			// �I�[
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, NULL }
		};
		return macroFuncInfoArr;
	}

	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			// ID									�֐���						����										�߂�l�̌^	m_pszData
			{ EFunctionCode(F_MA_GET_MODE),			LTEXT("GetMode"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// ���[�h���擾����
			{ EFunctionCode(F_MA_GET_FLAGS),		LTEXT("GetFlags"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// flags���擾����
			{ EFunctionCode(F_MA_GET_EXT),			LTEXT("GetExt"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	NULL },	// Ext���擾����
			{ EFunctionCode(F_MA_GET_SOURCE),		LTEXT("GetSource"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	NULL },	// Source���擾����
			{ EFunctionCode(F_MA_GET_INDEX),		LTEXT("GetIndex"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// �}�N��Index���擾����
			// �I�[
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, NULL }
		};
		return macroFuncInfoNotCommandArr;
	}

	// �֐�����������
	virtual
	bool HandleFunction(
		EditView*		View,
		EFunctionCode	ID,
		const VARIANT*	Arguments,
		const int		ArgSize,
		VARIANT&		Result
		)
	{
		switch (LOWORD(ID)) {
		case F_MA_GET_MODE:
			{
				Wrap(&Result)->Receive(m_nMode);
			}
			return true;
		case F_MA_GET_FLAGS:	// flags���擾����
			{
				Wrap(&Result)->Receive(m_nFlags);
			}
			return true;
		case F_MA_GET_EXT:
			{
				SysString S(m_Ext.c_str(), m_Ext.length());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_MA_GET_SOURCE:
			{
				SysString S(m_source.c_str(), m_source.length());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_MA_GET_INDEX:
			{
				Wrap(&Result)->Receive(m_nIndex);
				//Wrap(&Result)->Receive(CEditApp::getInstance()->m_pSMacroMgr->GetCurrentIdx());
			}
			return true;
		}
		return false;
	}

	// �R�}���h����������
	virtual
	bool HandleCommand(
		EditView*		View,
		EFunctionCode	ID,
		const WCHAR*	Arguments[],
		const int		ArgLengths[],
		const int		ArgSize
		)
	{
		switch (LOWORD(ID)) {
		case F_MA_SET_MATCH:
			if (Arguments[0]) {
				m_nIsMatch = _wtol(Arguments[0]);
			}
			return true;
		}
		return false;
	}

	bool IsMatch() {
		return (m_nIsMatch != 0);
	}

	void SetMatch(const int nMatch) {
		m_nIsMatch = nMatch;
	}

	// �����o�ϐ�
public:
	tagModeID m_nMode;
	int m_nIsMatch;
	int m_nFlags;
	std::wstring m_Ext;
	std::wstring m_source;
	int m_nIndex;
};

