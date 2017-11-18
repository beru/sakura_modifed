/*!	@file
	@brief �v���O�C���Ǘ��N���X
*/
#pragma once

#include "plugin/Plugin.h"
#include <list>
#include <string>

class PluginManager : public TSingleton<PluginManager> {
	friend class TSingleton<PluginManager>;
	PluginManager();

	// �^��`
private:
	typedef std::wstring wstring;
	typedef std::string string;

	// ����
public:
	bool LoadAllPlugin(CommonSetting* common = nullptr);				// �S�v���O�C����ǂݍ���
	void UnloadAllPlugin();				// �S�v���O�C�����������
	bool SearchNewPlugin(CommonSetting& common, HWND hWndOwner);		// �V�K�v���O�C���𓱓�����
	int InstallPlugin(CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, wstring& errorMsg, bool bUpdate = false);	// �v���O�C���̏�������������
	bool InstZipPlugin(CommonSetting& common, HWND hWndOwner, const tstring& sZipName, bool bInSearch=false);		// Zip�v���O�C����ǉ�����
	Plugin* GetPlugin(int id);		// �v���O�C�����擾����
	void UninstallPlugin(CommonSetting& common, int id);		// �v���O�C�����폜����

private:
	Plugin* LoadPlugin(const TCHAR* pszPluginDir, const TCHAR* pszPluginName, const TCHAR* pszLangName);	// �v���O�C����ǂݍ���
	bool RegisterPlugin(Plugin* plugin);		// �v���O�C����CJackManager�ɓo�^����
	bool UnRegisterPlugin(Plugin* plugin);	// �v���O�C����CJackManager�̓o�^����������

	// ����
public:
	// plugins�t�H���_�̃p�X
	const tstring GetBaseDir() { return sBaseDir; }
	const tstring GetExePluginDir() { return sExePluginDir; }
	bool SearchNewPluginDir(CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel);		// �V�K�v���O�C����ǉ�����(������)
	bool SearchNewPluginZip(CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel);		// �V�K�v���O�C����ǉ�����(������)Zip File
	bool InstZipPluginSub(CommonSetting& common, HWND hWndOwner, const tstring& sZipName, const tstring& sDispName, bool bInSearch, bool& bCancel);		// Zip�v���O�C���𓱓�����(������)

	// �����o�ϐ�
private:
	Plugin::List plugins;
	tstring sBaseDir;					// plugins�t�H���_�̃p�X
	tstring sExePluginDir;			// Exe�t�H���_�z��plugins�t�H���_�̃p�X

};

