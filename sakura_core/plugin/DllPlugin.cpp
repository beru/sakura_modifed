/*!	@file
	@brief DLL�v���O�C���N���X
*/
#include "StdAfx.h"
#include "plugin/DllPlugin.h"
#include "view/EditView.h"

// �f�X�g���N�^
DllPlugin::~DllPlugin(void)
{
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
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
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// DLL���̓ǂݍ���
	profile.IOProfileData(PII_DLL, PII_DLL_NAME, sDllName);

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
	EditView& view,
	Plug& plug_raw,
	WSHIfObj::List& params
	)
{
	tstring dllPath = GetFilePath(to_tchar(sDllName.c_str()));
	InitDllResultType resInit = InitDll(to_tchar(dllPath.c_str()));
	if (resInit != InitDllResultType::Success) {
		::MYMESSAGEBOX(view.hwndParent, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR1), dllPath.c_str(), sName.c_str());
		return false;
	}

	DllPlug& plug = *(static_cast<DllPlug*>(&plug_raw));
	if (!plug.handler) {
		// DLL�֐��̎擾
		ImportTable imp[2] = {
			{ &plug.handler, to_achar(plug.sHandler.c_str()) },
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
	plug.handler();
	ba.ExecKeyMacroAfter(view, flags, true);
	
	return true;
}
