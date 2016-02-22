/*!	@file
	@brief DLL�v���O�C���N���X

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
#include "plugin/DllPlugin.h"
#include "view/EditView.h"

// �f�X�g���N�^
DllPlugin::~DllPlugin(void)
{
	for (auto it=m_plugs.begin(); it!=m_plugs.end(); ++it) {
		delete (DllPlug*)(*it);
	}
}

// �v���O�̐���
// Plug�̑����DllPlug���쐬����
Plug* DllPlugin::CreatePlug(
	Plugin& plugin,
	PlugId id,
	const wstring& sJack,
	const wstring& sHandler,
	const wstring& sLabel
	)
{
	DllPlug* newPlug = new DllPlug(plugin, id, sJack, sHandler, sLabel);
	return newPlug;
}

// �v���O�C����`�t�@�C���̓ǂݍ���
bool DllPlugin::ReadPluginDef(
	DataProfile* profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// DLL���̓ǂݍ���
	profile->IOProfileData(PII_DLL, PII_DLL_NAME, m_sDllName);

	// �v���O�̓ǂݍ���
	ReadPluginDefPlug(profile, profileMlang);

	// �R�}���h�̓ǂݍ���
	ReadPluginDefCommand(profile, profileMlang);

	// �I�v�V������`�̓ǂݍ���	// 2010/3/24 Uchi
	ReadPluginDefOption(profile, profileMlang);

	// �������`�̓ǂݍ���
	ReadPluginDefString(profile, profileMlang);

	return true;
}

// �v���O���s
bool DllPlugin::InvokePlug(
	EditView* view,
	Plug& plug_raw,
	WSHIfObj::List& params
	)
{
	tstring dllPath = GetFilePath(to_tchar(m_sDllName.c_str()));
	InitDllResultType resInit = InitDll(to_tchar(dllPath.c_str()));
	if (resInit != InitDllResultType::Success) {
		::MYMESSAGEBOX(view->m_hwndParent, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR1), dllPath.c_str(), sName.c_str());
		return false;
	}

	DllPlug& plug = *(static_cast<DllPlug*>(&plug_raw));
	if (!plug.m_handler) {
		// DLL�֐��̎擾
		ImportTable imp[2] = {
			{ &plug.m_handler, to_achar(plug.m_sHandler.c_str()) },
			{ NULL, 0 }
		};
		if (!RegisterEntries(imp)) {
//			DWORD err = GetLastError();
			::MYMESSAGEBOX(NULL, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR2));
			return false;
		}
	}
	MacroBeforeAfter ba;
	int flags = FA_NONRECORD | FA_FROMMACRO;
	ba.ExecKeyMacroBefore(view, flags);
	// DLL�֐��̌Ăяo��
	plug.m_handler();
	ba.ExecKeyMacroAfter(view, flags, true);
	
	return true;
}
