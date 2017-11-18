/*!	@file
	@brief DLL�v���O�C���N���X
*/
#pragma once

#include "Plugin.h"

#define	PII_DLL							L"Dll"			// DLL���
#define	PII_DLL_NAME					L"Name"			// ���O

typedef void (*DllPlugHandler)();

class DllPlug : public Plug {
public:
	DllPlug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel)
		:
		Plug(plugin, id, sJack, sHandler, sLabel),
		handler(NULL)
	{
	}
public:
	DllPlugHandler handler;
};

class DllPlugin :
	public Plugin,
	public DllImp
{
	// �R���X�g���N�^
public:
	DllPlugin(const tstring& sBaseDir)
		:
		Plugin(sBaseDir),
		DllImp()
	{
	}

	// �f�X�g���N�^
public:
	~DllPlugin(void);

	// ����
public:
	bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile& profile) {
		return true;
	}
	Plug* CreatePlug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel);
	Plug::Array GetPlugs() const {
		return plugs;
	}
	bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& params);

	bool InitDllImp() {
		return true;
	}
	LPCTSTR GetDllNameImp(int nIndex) {
		return _T("");
	}

	// �����o�ϐ�
private:
	wstring sDllName;

};

