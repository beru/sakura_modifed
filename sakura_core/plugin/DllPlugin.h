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
		m_handler(NULL)
	{
	}
public:
	DllPlugHandler m_handler;
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
	bool ReadPluginDef(DataProfile* profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile* profile) {
		return true;
	}
	Plug* CreatePlug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel);
	Plug::Array GetPlugs() const {
		return m_plugs;
	}
	bool InvokePlug(EditView* view, Plug& plug, WSHIfObj::List& params);

	bool InitDllImp() {
		return true;
	}
	LPCTSTR GetDllNameImp(int nIndex) {
		return _T("");
	}

	// �����o�ϐ�
private:
	wstring m_sDllName;

};

