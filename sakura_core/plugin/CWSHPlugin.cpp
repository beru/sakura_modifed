/*!	@file
	@brief WSH�v���O�C���N���X

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
#include "plugin/CWSHPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CWSHManager.h"

// �f�X�g���N�^
WSHPlugin::~WSHPlugin(void)
{
	for (auto it=m_plugs.begin(); it!=m_plugs.end(); ++it) {
		delete *it;
	}
}

// �v���O�C����`�t�@�C����ǂݍ���
bool WSHPlugin::ReadPluginDef(
	DataProfile* profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// WSH�Z�N�V�����̓ǂݍ���
	profile->IOProfileData<bool>(PII_WSH, PII_WSH_USECACHE, m_bUseCache);

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

// �I�v�V�����t�@�C����ǂݍ���
bool WSHPlugin::ReadPluginOption(
	DataProfile* profile
	)
{
	return true;
}

// �v���O�����s����
bool WSHPlugin::InvokePlug(
	EditView* view,
	Plug& plug,
	WSHIfObj::List& params
	)
{
	WSHPlug& wshPlug = static_cast<WSHPlug&>(plug);
	WSHMacroManager* pWsh = NULL;

	if (!m_bUseCache || !wshPlug.m_Wsh) {
		FilePath path(plug.m_plugin.GetFilePath(to_tchar(plug.m_sHandler.c_str())).c_str());

		pWsh = (WSHMacroManager*)WSHMacroManager::Creator(path.GetExt(true));
		if (!pWsh) {
			return false;
		}

		bool bLoadResult = pWsh->LoadKeyMacro(G_AppInstance(), path);
		if (!bLoadResult) {
			ErrorMessage(NULL, LS(STR_WSHPLUG_LOADMACRO), static_cast<const TCHAR*>(path));
			delete pWsh;
			return false;
		}

	}else {
		pWsh = wshPlug.m_Wsh;
	}

	PluginIfObj cPluginIfo(*this);		// Plugin�I�u�W�F�N�g��ǉ�
	cPluginIfo.AddRef();
	cPluginIfo.SetPlugIndex(plug.m_id);	// ���s���v���O�ԍ����
	pWsh->AddParam(&cPluginIfo);
	pWsh->AddParam(params);			// �p�����[�^��ǉ�
	pWsh->ExecKeyMacro2(view, FA_NONRECORD | FA_FROMMACRO);
	pWsh->ClearParam();

	if (m_bUseCache) {
		wshPlug.m_Wsh = pWsh;
	}else {
		// �I�������J��
		delete pWsh;
	}

	return true;
}

