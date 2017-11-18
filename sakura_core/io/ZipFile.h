/*!	@file
	@brief ZIP file����

*/
#pragma once

#include <ShlDisp.h>

class ZipFile {
private:
	IShellDispatch*	psd;
	Folder*			pZipFile;
	std::tstring	sZipName;

public:
	ZipFile();		// �R���X�g���N�^
	~ZipFile();	// �f�X�g���N�^

public:
	bool	IsOk() { return (psd != NULL); }			// Zip Folder���g�p�ł��邩?
	bool	SetZip(const std::tstring& sZipPath);		// Zip File�� �ݒ�
	bool	ChkPluginDef(const std::tstring& sDefFile, std::tstring& sFolderName);	// ZIP File �� �t�H���_���擾�ƒ�`�t�@�C������(Plugin�p)
	bool	Unzip(const std::tstring& sOutPath);			// Zip File ��
};

