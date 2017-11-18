#pragma once

#include "DllSharedData.h"

// �h�L�������g�^�C�v�Ǘ�
class DocTypeManager {
public:
	DocTypeManager() {
		pShareData = &GetDllShareData();
	}
	TypeConfigNum GetDocumentTypeOfPath(const TCHAR* pszFilePath);	// �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾����
	TypeConfigNum GetDocumentTypeOfExt(const TCHAR* pszExt);		// �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾����
	TypeConfigNum GetDocumentTypeOfId(int id);

	bool GetTypeConfig(TypeConfigNum documentType, TypeConfig& type);
	bool SetTypeConfig(TypeConfigNum documentType, const TypeConfig& type);
	bool GetTypeConfigMini(TypeConfigNum documentType, const TypeConfigMini** type);
	bool AddTypeConfig(TypeConfigNum documentType);
	bool DelTypeConfig(TypeConfigNum documentType);

	static bool IsFileNameMatch(const TCHAR* pszTypeExts, const TCHAR* pszFileName);	// �^�C�v�ʊg���q�Ƀt�@�C�������}�b�`���邩
	static void GetFirstExt(const TCHAR* pszTypeExts, TCHAR szFirstExt[], int nBuffSize);	// �^�C�v�ʊg���q�̐擪�g���q���擾����
	static bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, const TCHAR* szExt, TCHAR *pszDstExt );	// �^�C�v�ʐݒ�̊g���q���X�g���_�C�A���O�p���X�g�ɕϊ�����

	static const TCHAR* typeExtSeps;			// �^�C�v�ʊg���q�̋�؂蕶��
	static const TCHAR* typeExtWildcards;		// �^�C�v�ʊg���q�̃��C���h�J�[�h

private:
	DllSharedData* pShareData;
};

