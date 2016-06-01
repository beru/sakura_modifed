/*!	@file
	@brief Plugin�I�u�W�F�N�g

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
#pragma once

#include "macro/WSHIfObj.h"
#include "_os/OleTypes.h"
#include "util/ole_convert.h"

// cpp�ֈړ��\��
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "Plugin.h"

class PluginIfObj : public WSHIfObj {
	// �^��`
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					// ���R�}���h�͈ȉ��ɒǉ�����
		F_PL_SETOPTION,							// �I�v�V�����t�@�C���ɒl������
		F_PL_ADDCOMMAND,						// �R�}���h��ǉ�����
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ���֐��͈ȉ��ɒǉ�����
		F_PL_GETPLUGINDIR,						// �v���O�C���t�H���_�p�X���擾����
		F_PL_GETDEF,							// �ݒ�t�@�C������l��ǂ�
		F_PL_GETOPTION,							// �I�v�V�����t�@�C������l��ǂ�
		F_PL_GETCOMMANDNO,						// ���s���v���O�̔ԍ����擾����
		F_PL_GETSTRING,							// �ݒ�t�@�C�����當�����ǂ݂���(������Ή�)
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	PluginIfObj(Plugin& plugin)
		:
		WSHIfObj(L"Plugin", false),
		plugin(plugin)
	{
		nPlugIndex = -1;
	}

	// �f�X�g���N�^
public:
	~PluginIfObj() {}

	// ����
public:
	void SetPlugIndex(int nIndex) { nPlugIndex = nIndex; }
	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const {
		return macroFuncInfoCommandArr;
	}
	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const {
		return macroFuncInfoArr;
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
		Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

		switch (LOWORD(index)) {
		case F_PL_GETPLUGINDIR:			// �v���O�C���t�H���_�p�X���擾����
			{
				SysString s(plugin.sBaseDir.c_str(), plugin.sBaseDir.size());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_PL_GETDEF:				// �ݒ�t�@�C������l��ǂ�
		case F_PL_GETOPTION:			// �I�v�V�����t�@�C������l��ǂ�
			{
				DataProfile profile;
				wstring sSection;
				wstring sKey;
				wstring sValue;
				if (!variant_to_wstr(arguments[0], sSection)) {
					return false;
				}
				if (!variant_to_wstr(arguments[1], sKey)) {
					return false;
				}

				profile.SetReadingMode();
				if (LOWORD(index) == F_PL_GETDEF) {
					profile.ReadProfile(plugin.GetPluginDefPath().c_str());
				}else {
					profile.ReadProfile(plugin.GetOptionPath().c_str());
				}
				if (!profile.IOProfileData(sSection.c_str(), sKey.c_str(), sValue)
					&& LOWORD(index) == F_PL_GETOPTION
				) {
					// �ݒ肳��Ă��Ȃ���΃f�t�H���g���擾 
					for (auto it=plugin.options.begin(); it!=plugin.options.end(); ++it) {
						wstring sSectionTmp;
						wstring sKeyTmp;
						(*it)->GetKey(&sSectionTmp, &sKeyTmp);
						if (sSection == sSectionTmp && sKey == sKeyTmp) {
							sValue = (*it)->GetDefaultVal();
							break;
						}
					}
				}

				SysString s(sValue.c_str(), sValue.size());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_PL_GETCOMMANDNO:			// ���s���v���O�̔ԍ����擾����
			{
				Wrap(&result)->Receive(nPlugIndex);
			}
			return true;
		case F_PL_GETSTRING:
			{
				int num;
				if (!variant_to_int(arguments[0], num)) {
					return false;
				}
				if (0 < num && num < MAX_PLUG_STRING) {
					std::wstring& str = plugin.aStrings[num];
					SysString s(str.c_str(), str.size());
					Wrap(&result)->Receive(s);
					return true;
				}else if (num == 0) {
					std::wstring str = to_wchar(plugin.sLangName.c_str());
					SysString s(str.c_str(), str.size());
					Wrap(&result)->Receive(s);
					return true;
				}
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
		switch (LOWORD(index)) {
		case F_PL_SETOPTION:			// �I�v�V�����t�@�C���ɒl������
			{
				if (!arguments[0]) return false;
				if (!arguments[1]) return false;
				if (!arguments[2]) return false;
				DataProfile profile;

				profile.ReadProfile(plugin.GetOptionPath().c_str());
				profile.SetWritingMode();
				wstring tmp(arguments[2]);
				profile.IOProfileData(arguments[0], arguments[1], tmp);
				profile.WriteProfile(plugin.GetOptionPath().c_str(), (plugin.sName + L" �v���O�C���ݒ�t�@�C��").c_str());
			}
			break;
		case F_PL_ADDCOMMAND:			// �R�}���h��ǉ�����
			{
				int id = plugin.AddCommand(arguments[0], arguments[1], arguments[2], true);
				view.editWnd.RegisterPluginCommand(id);
			}
			break;
		}
		return true;
	}

	// �����o�ϐ�
public:
private:
	Plugin& plugin;
	static MacroFuncInfo macroFuncInfoCommandArr[];	// �R�}���h���(�߂�l�Ȃ�)
	static MacroFuncInfo macroFuncInfoArr[];	// �֐����(�߂�l����)
	int nPlugIndex;	// ���s���v���O�̔ԍ�
};

