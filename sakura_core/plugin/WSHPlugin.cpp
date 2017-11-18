/*!	@file
	@brief WSH�v���O�C���N���X
*/
#include "StdAfx.h"
#include "plugin/WSHPlugin.h"
#include "plugin/PluginIfObj.h"
#include "macro/WSHManager.h"

// �f�X�g���N�^
WSHPlugin::~WSHPlugin(void)
{
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		delete *it;
	}
}

// �v���O�C����`�t�@�C����ǂݍ���
bool WSHPlugin::ReadPluginDef(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// WSH�Z�N�V�����̓ǂݍ���
	profile.IOProfileData<bool>(PII_WSH, PII_WSH_USECACHE, bUseCache);

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
	DataProfile& profile
	)
{
	return true;
}

// �v���O�����s����
bool WSHPlugin::InvokePlug(
	EditView& view,
	Plug& plug,
	WSHIfObj::List& params
	)
{
	WSHPlug& wshPlug = static_cast<WSHPlug&>(plug);
	WSHMacroManager* pWsh = nullptr;

	if (!bUseCache || !wshPlug.wsh) {
		FilePath path(plug.plugin.GetFilePath(to_tchar(plug.sHandler.c_str())).c_str());

		pWsh = (WSHMacroManager*)WSHMacroManager::Creator(view, path.GetExt(true));
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
		pWsh = wshPlug.wsh;
	}

	PluginIfObj pluginIfo(*this);		// Plugin�I�u�W�F�N�g��ǉ�
	pluginIfo.AddRef();
	pluginIfo.SetPlugIndex(plug.id);	// ���s���v���O�ԍ����
	pWsh->AddParam(&pluginIfo);
	pWsh->AddParam(params);			// �p�����[�^��ǉ�
	pWsh->ExecKeyMacro2(view, FA_NONRECORD | FA_FROMMACRO);
	pWsh->ClearParam();

	if (bUseCache) {
		wshPlug.wsh = pWsh;
	}else {
		// �I�������J��
		delete pWsh;
	}

	return true;
}

