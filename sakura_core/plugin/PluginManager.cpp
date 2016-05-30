/*!	@file
	@brief �v���O�C���Ǘ��N���X

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
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "plugin/WSHPlugin.h"
#include "plugin/DllPlugin.h"
#include "util/module.h"
#include "io/ZipFile.h"

// �R���X�g���N�^
PluginManager::PluginManager()
{
	// plugins�t�H���_�̏ꏊ���擾
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir(szPluginPath, _T("plugins\\"));	// ini�Ɠ����K�w��plugins�t�H���_������
	sBaseDir.append(szPluginPath);

	// Exe�t�H���_�z��plugins�t�H���_�̃p�X���擾
	TCHAR	szPath[_MAX_PATH];
	TCHAR	szFolder[_MAX_PATH];
	TCHAR	szFname[_MAX_PATH];

	::GetModuleFileName(NULL, szPath, _countof(szPath));
	SplitPath_FolderAndFile(szPath, szFolder, szFname);
	Concat_FolderAndFile(szFolder, _T("plugins\\"), szPluginPath);

	sExePluginDir.append(szPluginPath);
}

// �S�v���O�C�����������
void PluginManager::UnloadAllPlugin()
{
	for (auto it=plugins.begin(); it!=plugins.end(); ++it) {
		UnRegisterPlugin(*it);
	}

	for (auto it=plugins.begin(); it!=plugins.end(); ++it) {
		delete *it;
	}
	
	// 2010.08.04 Moca plugins.claer����
	plugins.clear();
}

// �V�K�v���O�C����ǉ�����
bool PluginManager::SearchNewPlugin(
	CommonSetting& common,
	HWND hWndOwner
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPlugin\n"));
#endif

	HANDLE hFind;
	ZipFile	zipFile;

	// �v���O�C���t�H���_�̔z��������
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile((sBaseDir + _T("*")).c_str(), &wf);
	if (hFind == INVALID_HANDLE_VALUE) {
		// �v���O�C���t�H���_�����݂��Ȃ�
		if (!CreateDirectory(sBaseDir.c_str(), NULL)) {
			InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_FOLDER));
			return true;
		}
	}
	::FindClose(hFind);

	bool	bCancel = false;
	// �v���O�C���t�H���_�̔z��������
	bool bFindNewDir = SearchNewPluginDir(common, hWndOwner, sBaseDir, bCancel);
	if (!bCancel && sBaseDir != sExePluginDir) {
		bFindNewDir |= SearchNewPluginDir(common, hWndOwner, sExePluginDir, bCancel);
	}
	if (!bCancel && zipFile.IsOk()) {
		bFindNewDir |= SearchNewPluginZip(common, hWndOwner, sBaseDir, bCancel);
		if (!bCancel && sBaseDir != sExePluginDir) {
			bFindNewDir |= SearchNewPluginZip(common, hWndOwner, sExePluginDir, bCancel);
		}
	}

	if (bCancel) {
		InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_CANCEL));
	}else if (!bFindNewDir) {
		InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_NEWPLUGIN));
	}

	return true;
}


// �V�K�v���O�C����ǉ�����(������)
bool PluginManager::SearchNewPluginDir(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sSearchDir,
	bool& bCancel
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginDir\n"));
#endif

	PluginRec* pluginTable = common.plugin.pluginTable;
	HANDLE hFind;

	WIN32_FIND_DATA wf;
	hFind = FindFirstFile((sSearchDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		// �v���O�C���t�H���_�����݂��Ȃ�
		return false;
	}
	bool bFindNewDir = false;
	do {
		if ((wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			(wf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 &&
			_tcscmp(wf.cFileName, _T(".")) != 0 &&
			_tcscmp(wf.cFileName, _T("..")) != 0 &&
			auto_stricmp(wf.cFileName, _T("unuse")) != 0
		) {
			// �C���X�g�[���ς݃`�F�b�N�B�t�H���_�����v���O�C���e�[�u���̖��O�Ȃ�C���X�g�[�����Ȃ�
			// 2010.08.04 �啶�����������ꎋ�ɂ���
			bool isNotInstalled = true;
			for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
				if (auto_stricmp(wf.cFileName, to_tchar(pluginTable[iNo].szName)) == 0) {
					isNotInstalled = false;
					break;
				}
			}
			if (!isNotInstalled) { continue; }

			// 2011.08.20 syat plugin.def�����݂��Ȃ��t�H���_�͔�΂�
			if (!IsFileExists((sSearchDir + wf.cFileName + _T("\\") + PII_FILENAME).c_str(), true)) {
				continue;
			}

			bFindNewDir = true;
			int nRes = Select3Message(hWndOwner, LS(STR_PLGMGR_INSTALL), wf.cFileName);
			if (nRes == IDYES) {
				std::wstring errMsg;
				int pluginNo = InstallPlugin(common, wf.cFileName, hWndOwner, errMsg);
				if (pluginNo < 0) {
					WarningMessage(hWndOwner, LS(STR_PLGMGR_INSTALL_ERR),
						wf.cFileName, errMsg.c_str()
					);
				}
			}else if (nRes == IDCANCEL) {
				bCancel = true;
				break;	// for loop
			}
		}
	}while (FindNextFile(hFind, &wf));

	FindClose(hFind);
	return bFindNewDir;
}


// �V�K�v���O�C����ǉ�����(������)Zip File
bool PluginManager::SearchNewPluginZip(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sSearchDir,
	bool& bCancel
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginZip\n"));
#endif

	HANDLE hFind;

	WIN32_FIND_DATA wf;
	bool	bNewPlugin = false;
	bool	bFound;
	ZipFile	zipFile;

	hFind = INVALID_HANDLE_VALUE;

	// Zip File ������
	if (zipFile.IsOk()) {
		hFind = FindFirstFile((sSearchDir + _T("*.zip")).c_str(), &wf);

		for (bFound = (hFind != INVALID_HANDLE_VALUE); bFound; bFound = (FindNextFile(hFind, &wf) != 0)) {
			if ((wf.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) == 0) {
				bNewPlugin |= InstZipPluginSub(common, hWndOwner, sSearchDir + wf.cFileName, wf.cFileName, true, bCancel);
				if (bCancel) {
					break;
				}
			}
		}
	}

	FindClose(hFind);
	return bNewPlugin;
}


// Zip�v���O�C���𓱓�����
bool PluginManager::InstZipPlugin(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sZipFile,
	bool bInSearch
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Entry InstZipPlugin\n"));
#endif

	ZipFile	zipFile;
	TCHAR	msg[512];

	// ZIP�t�@�C���������邩
	if (!zipFile.IsOk()) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_ERR_ZIP));
		InfoMessage(hWndOwner, _T("%ts"), msg);
		return false;
	}

	// �v���O�C���t�H���_�̑��݂��m�F
	WIN32_FIND_DATA wf;
	HANDLE hFind;
	if ((hFind = ::FindFirstFile((sBaseDir + _T("*")).c_str(), &wf )) == INVALID_HANDLE_VALUE) {
		// �v���O�C���t�H���_�����݂��Ȃ�
		if (sBaseDir == sExePluginDir) {
			InfoMessage(hWndOwner, LS(STR_PLGMGR_ERR_FOLDER));
			::FindClose(hFind);
			return false;
		}else {
			if (!CreateDirectory(sBaseDir.c_str(), NULL)) {
				WarningMessage(hWndOwner, LS(STR_PLGMGR_ERR_CREATEDIR));
				::FindClose(hFind);
				return false;
			}
		}
	}
	::FindClose(hFind);

	bool bCancel;
	return PluginManager::InstZipPluginSub(common, hWndOwner, sZipFile, sZipFile, false, bCancel);
}

// Zip�v���O�C���𓱓�����(������)
bool PluginManager::InstZipPluginSub(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sZipFile,
	const tstring& sDispName,
	bool bInSearch,
	bool& bCancel
	)
{
	PluginRec*		pluginTable = common.plugin.pluginTable;
	ZipFile			zipFile;
	std::tstring	sFolderName;
	TCHAR			msg[512];
	std::wstring	errMsg;
	bool			bOk = true;
	bool			bSkip = false;
	bool			bNewPlugin = false;

	// Plugin �t�H���_���̎擾,��`�t�@�C���̊m�F
	if (bOk && !zipFile.SetZip(sZipFile)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ACCESS), sDispName.c_str());
		bOk = false;
		bSkip = bInSearch;
	}

	// Plgin �t�H���_���̎擾,��`�t�@�C���̊m�F
	if (bOk && !zipFile.ChkPluginDef(PII_FILENAME, sFolderName)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_DEF), sDispName.c_str());
		bOk = false;
		bSkip = bInSearch;
	}

	if (!bInSearch) {
		// �P�ƃC���X�g�[��
		// �C���X�g�[���ς݃`�F�b�N�B
		bool	isNotInstalled = true;
		int		iNo;
		if (bOk) {
			for (iNo=0; iNo<MAX_PLUGIN; ++iNo) {
				if (auto_stricmp(to_wchar(sFolderName.c_str()), to_wchar(pluginTable[iNo].szName)) == 0) {
					isNotInstalled = false;
					break;
				}
			}
			if (isNotInstalled) {
				bNewPlugin = true;
			}else {
				if (ConfirmMessage(
						hWndOwner,
						LS(STR_PLGMGR_INST_ZIP_ALREADY),
						sDispName.c_str()
					) != IDYES
				) {
					// Yes�Ŗ����Ȃ�I��
					return false;
				}
			}
		}
	}else {
		// plugins�t�H���_������
		// �t�H���_ �`�F�b�N�B���łɉ𓀂���Ă����Ȃ�C���X�g�[�����Ȃ�(�O�i�ŃC���X�g�[���ς݈��͉ۂ��m�F�ς�)
		if (bOk && fexist(to_tchar((sBaseDir + to_tchar(sFolderName.c_str())).c_str()))
			|| fexist(to_tchar((sExePluginDir + to_tchar(sFolderName.c_str())).c_str()))
		) {
			bOk = false;
			bSkip = true;
		}
		if (bOk) {
			bNewPlugin= true;
			int nRes = Select3Message(
				hWndOwner, LS(STR_PLGMGR_INST_ZIP_INST),
				sDispName.c_str(), sFolderName.c_str()
			);
			switch (nRes) {
			case IDCANCEL:
				bCancel = true;
				// through
			case IDNO:
				bOk = false;
				bSkip = true;
				break;
			}
		}
	}

	// Zip��
	if (bOk && !zipFile.Unzip(sBaseDir)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_UNZIP), sDispName.c_str());
		bOk = false;
	}
	if (bOk) {
		int pluginNo = InstallPlugin(common, to_tchar(sFolderName.c_str()), hWndOwner, errMsg, true);
		if (pluginNo < 0) {
			auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ERR), sDispName.c_str(), errMsg.c_str());
			bOk = false;
		}
	}

	if (!bOk && !bSkip) {
		// �G���[���b�Z�[�W�o��
		WarningMessage(hWndOwner, _T("%s"), msg);
	}

	return bNewPlugin;
}

// �v���O�C���̏�������������
//	common			���L�ݒ�ϐ�
//	pszPluginName	�v���O�C����
//	hWndOwner		
//	errorMsg		�G���[���b�Z�[�W��Ԃ�
//	bUodate			���łɓo�^���Ă����ꍇ�A�m�F�����㏑������
int PluginManager::InstallPlugin(
	CommonSetting& common,
	const TCHAR* pszPluginName,
	HWND hWndOwner,
	std::wstring& errorMsg,
	bool bUpdate
	)
{
	DataProfile profDef;				// �v���O�C����`�t�@�C��

	// �v���O�C����`�t�@�C����ǂݍ���
	profDef.SetReadingMode();
	if (!profDef.ReadProfile((sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str())
		&& !profDef.ReadProfile((sExePluginDir + pszPluginName + _T("\\") + PII_FILENAME).c_str()) 
	) {
		errorMsg = LSW(STR_PLGMGR_INST_DEF);
		return -1;
	}

	std::wstring sId;
	profDef.IOProfileData(PII_PLUGIN, PII_PLUGIN_ID, sId);
	if (sId.length() == 0) {
		errorMsg = LSW(STR_PLGMGR_INST_ID);
		return -1;
	}
	// 2010.08.04 ID�g�p�s�̕������m�F
	//  ��X�t�@�C������ini�Ŏg�����Ƃ��l���Ă��������ۂ���
	static const WCHAR szReservedChars[] = L"/\\,[]*?<>&|;:=\" \t";
	for (size_t x=0; x<_countof(szReservedChars); ++x) {
		if (sId.npos != sId.find(szReservedChars[x])) {
			errorMsg = std::wstring(LSW(STR_PLGMGR_INST_RESERVE1)) + szReservedChars + LSW(STR_PLGMGR_INST_RESERVE2);
			return -1;
		}
	}
	if (WCODE::Is09(sId[0])) {
		errorMsg = LSW(STR_PLGMGR_INST_IDNUM);
		return -1;
	}

	// ID�d���E�e�[�u���󂫃`�F�b�N
	PluginRec* pluginTable = common.plugin.pluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
		if (nEmpty == -1 && pluginTable[iNo].state == PLS_NONE) {
			nEmpty = iNo;
			// break ���Ă͂����Ȃ��B���œ���ID�����邩��
		}
		if (wcscmp(sId.c_str(), pluginTable[iNo].szId) == 0) {	// ID��v
			if (!bUpdate) {
				const TCHAR* msg = LS(STR_PLGMGR_INST_NAME);
				// 2010.08.04 �폜����ID�͌��̈ʒu�֒ǉ�(����������)
				if (pluginTable[iNo].state != PLS_DELETED &&
					ConfirmMessage(hWndOwner, msg, static_cast<const TCHAR*>(pszPluginName), static_cast<const WCHAR*>(pluginTable[iNo].szName)) != IDYES
				) {
					errorMsg = LSW(STR_PLGMGR_INST_USERCANCEL);
					return -1;
				}
			}
			nEmpty = iNo;
			isDuplicate = pluginTable[iNo].state != PLS_DELETED;
			break;
		}
	}

	if (nEmpty == -1) {
		errorMsg = LSW(STR_PLGMGR_INST_MAX);
		return -1;
	}

	wcsncpy(pluginTable[nEmpty].szName, to_wchar(pszPluginName), MAX_PLUGIN_NAME);
	pluginTable[nEmpty].szName[MAX_PLUGIN_NAME-1] = '\0';
	wcsncpy(pluginTable[nEmpty].szId, sId.c_str(), MAX_PLUGIN_ID);
	pluginTable[nEmpty].szId[MAX_PLUGIN_ID-1] = '\0';
	pluginTable[nEmpty].state = isDuplicate ? PLS_UPDATED : PLS_INSTALLED;

	// �R�}���h���̐ݒ�	2010/7/11 Uchi
	int			i;
	WCHAR		szPlugKey[10];
	wstring		sPlugCmd;

	pluginTable[nEmpty].nCmdNum = 0;
	for (i=1; i<MAX_PLUG_CMD; ++i) {
		auto_sprintf_s(szPlugKey, L"C[%d]", i);
		sPlugCmd.clear();
		profDef.IOProfileData(PII_COMMAND, szPlugKey, sPlugCmd);
		if (sPlugCmd == L"") {
			break;
		}
		pluginTable[nEmpty].nCmdNum = i;
	}

	return nEmpty;
}

// �S�v���O�C����ǂݍ���
bool PluginManager::LoadAllPlugin(CommonSetting* common)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter LoadAllPlugin\n"));
#endif
	CommonSetting_Plugin& pluginSetting = (common ? common->plugin : GetDllShareData().common.plugin);

	if (!pluginSetting.bEnablePlugin) {
		return true;
	}

	std::tstring szLangName;
	{
		std::tstring szDllName = GetDllShareData().common.window.szLanguageDll;
		if (szDllName == _T("")) {
			szLangName = _T("ja_JP");
		}else {
			// "sakura_lang_*.dll"
			size_t nStartPos = 0;
			size_t nEndPos = szDllName.length();
			if (szDllName.substr(0, 12) == _T("sakura_lang_")) {
				nStartPos = 12;
			}
			if (4 < szDllName.length() && szDllName.substr(szDllName.length() - 4, 4) == _T(".dll")) {
				nEndPos = szDllName.length() - 4;
			}
			szLangName = szDllName.substr(nStartPos, nEndPos - nStartPos);
		}
		DEBUG_TRACE(_T("lang = %ts\n"), szLangName.c_str());
	}

	// �v���O�C���e�[�u���ɓo�^���ꂽ�v���O�C����ǂݍ���
	PluginRec* pluginTable = pluginSetting.pluginTable;
	for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
		if (pluginTable[iNo].szName[0] == '\0') {
			continue;
		}
		// 2010.08.04 �폜��Ԃ�����(���̂Ƃ���ی�)
		if (pluginTable[iNo].state == PLS_DELETED) {
			continue;
		}
		if (GetPlugin(iNo)) {
			continue; // 2013.05.31 �ǂݍ��ݍς�
		}
		std::tstring name = to_tchar(pluginTable[iNo].szName);
		Plugin* plugin = LoadPlugin(sBaseDir.c_str(), name.c_str(), szLangName.c_str());
		if (!plugin) {
			plugin = LoadPlugin(sExePluginDir.c_str(), name.c_str(), szLangName.c_str());
		}
		if (plugin) {
			// �v�����Fplugin.def��id��sakuraw.ini��id�̕s��v����
			assert_warning(auto_strcmp(pluginTable[iNo].szId, plugin->sId.c_str()) == 0);
			plugin->id = iNo;		// �v���O�C���e�[�u���̍s�ԍ���ID�Ƃ���
			plugins.push_back(plugin);
			pluginTable[iNo].state = PLS_LOADED;
			// �R�}���h���ݒ�
			pluginTable[iNo].nCmdNum = plugin->GetCommandCount();
			RegisterPlugin(plugin);
		}
	}
	
	return true;
}

// �v���O�C����ǂݍ���
Plugin* PluginManager::LoadPlugin(
	const TCHAR* pszPluginDir,
	const TCHAR* pszPluginName,
	const TCHAR* pszLangName
	)
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	std::tstring strMlang;
	DataProfile profDef;				// �v���O�C����`�t�@�C��
	DataProfile profDefMLang;			// �v���O�C����`�t�@�C��(L10N)
	DataProfile* pProfDefMLang = &profDefMLang; 
	DataProfile profOption;			// �I�v�V�����t�@�C��
	Plugin* plugin = nullptr;

#ifdef _UNICODE
	DEBUG_TRACE(_T("Load Plugin %ts\n"),  pszPluginName );
#endif
	// �v���O�C����`�t�@�C����ǂݍ���
	Concat_FolderAndFile(pszPluginDir, pszPluginName, pszBasePath);
	Concat_FolderAndFile(pszBasePath, PII_FILENAME, pszPath);
	profDef.SetReadingMode();
	if (!profDef.ReadProfile(pszPath)) {
		// �v���O�C����`�t�@�C�������݂��Ȃ�
		return nullptr;
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  ��`�t�@�C���Ǎ� %ts\n"),  pszPath );
#endif

	// L10N��`�t�@�C����ǂ�
	// �v���O�C����`�t�@�C����ǂݍ��� base\pluginname\local\plugin_en_us.def
	strMlang = std::tstring(pszBasePath) + _T("\\") + PII_L10NDIR + _T("\\") + PII_L10NFILEBASE + pszLangName + PII_L10NFILEEXT;
	profDefMLang.SetReadingMode();
	if (!profDefMLang.ReadProfile(strMlang.c_str())) {
		// �v���O�C����`�t�@�C�������݂��Ȃ�
		pProfDefMLang = nullptr;
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N��`�t�@�C���Ǎ� %ts Not Found\n"),  strMlang.c_str() );
#endif
	}else {
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N��`�t�@�C���Ǎ� %ts\n"),  strMlang.c_str() );
#endif
	}

	std::wstring sPlugType;
	profDef.IOProfileData(PII_PLUGIN, PII_PLUGIN_PLUGTYPE, sPlugType);

	if (wcsicmp(sPlugType.c_str(), L"wsh") == 0) {
		plugin = new WSHPlugin(tstring(pszBasePath));
	}else if (wcsicmp(sPlugType.c_str(), L"dll") == 0) {
		plugin = new DllPlugin(tstring(pszBasePath));
	}else {
		return nullptr;
	}
	plugin->sOptionDir = sBaseDir + pszPluginName;
	plugin->sLangName = pszLangName;
	plugin->ReadPluginDef(profDef, pProfDefMLang);
#ifdef _UNICODE
	DEBUG_TRACE(_T("  �v���O�C���^�C�v %ls\n"), sPlugType.c_str() );
#endif

	// �I�v�V�����t�@�C����ǂݍ���
	profOption.SetReadingMode();
	if (profOption.ReadProfile(plugin->GetOptionPath().c_str())) {
		// �I�v�V�����t�@�C�������݂���ꍇ�A�ǂݍ���
		plugin->ReadPluginOption(profOption);
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  �I�v�V�����t�@�C���Ǎ� %ts\n"),  plugin->GetOptionPath().c_str() );
#endif

	return plugin;
}

// �v���O�C����JackManager�ɓo�^����
bool PluginManager::RegisterPlugin(Plugin* plugin)
{
	auto& jackMgr = JackManager::getInstance();
	Plug::Array plugs = plugin->GetPlugs();

	for (auto plug=plugs.begin(); plug!=plugs.end(); ++plug) {
		jackMgr.RegisterPlug((*plug)->sJack.c_str(), *plug);
	}

	return true;
}

// �v���O�C����JackManager�̓o�^����������
bool PluginManager::UnRegisterPlugin(Plugin* plugin)
{
	auto& jackMgr = JackManager::getInstance();
	Plug::Array plugs = plugin->GetPlugs();

	for (auto plug=plugs.begin(); plug!=plugs.end(); ++plug) {
		jackMgr.UnRegisterPlug((*plug)->sJack.c_str(), *plug);
	}

	return true;
}

// �v���O�C�����擾����
Plugin* PluginManager::GetPlugin(int id)
{
	for (auto plugin=plugins.begin(); plugin!=plugins.end(); ++plugin) {
		if ((*plugin)->id == id) {
			return *plugin;
		}
	}
	return nullptr;
}

// �v���O�C�����폜����
void PluginManager::UninstallPlugin(CommonSetting& common, int id)
{
	PluginRec* pluginTable = common.plugin.pluginTable;

	// 2010.08.04 �����ł�ID��ێ�����B��ōēx�ǉ�����Ƃ��ɓ����ʒu�ɒǉ�
	// PLS_DELETED��szId/szName��ini��ۑ�����ƍ폜����܂�
//	pluginTable[id].szId[0] = '\0';
	pluginTable[id].szName[0] = '\0';
	pluginTable[id].state = PLS_DELETED;
	pluginTable[id].nCmdNum = 0;
}

