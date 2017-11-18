/*!	@file
	@brief WSH�v���O�C���N���X
*/
#pragma once

#include "plugin/Plugin.h"
#include "macro/WSHManager.h"

#define PII_WSH						L"Wsh"			// WSH�Z�N�V����
#define PII_WSH_USECACHE			L"UseCache"		// �ǂݍ��񂾃X�N���v�g���ė��p����

class WSHPlug : public Plug {
public:
	WSHPlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) :
		Plug(plugin, id, sJack, sHandler, sLabel)
	{
		wsh = nullptr;
	}
	virtual ~WSHPlug() {
		if (wsh) {
			delete wsh;
			wsh = nullptr;
		}
	}
	WSHMacroManager* wsh;
};

class WSHPlugin : public Plugin {
	// �R���X�g���N�^
public:
	WSHPlugin(const tstring& sBaseDir) : Plugin(sBaseDir) {
		bUseCache = false;
	}

	// �f�X�g���N�^
public:
	~WSHPlugin(void);

	// ����
	// Plug�C���X�^���X�̍쐬�BReadPluginDefPlug/Command ����Ă΂��B
	virtual Plug* CreatePlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) {
		return new WSHPlug(plugin, id, sJack, sHandler, sLabel);
	}

	// ����
public:
	bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile& profile);
	Plug::Array GetPlugs() const {
		return plugs;
	}
	bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& params);

	// �����o�ϐ�
private:
	bool bUseCache;

};

