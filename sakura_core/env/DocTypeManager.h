/*
	2008.05.18 kobake CShareData ���番��
*/
/*
	Copyright (C) 2008, kobake

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

#include "DllSharedData.h"

// �h�L�������g�^�C�v�Ǘ�
class DocTypeManager {
public:
	DocTypeManager() {
		m_pShareData = &GetDllShareData();
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

	static const TCHAR* m_typeExtSeps;			// �^�C�v�ʊg���q�̋�؂蕶��
	static const TCHAR* m_typeExtWildcards;		// �^�C�v�ʊg���q�̃��C���h�J�[�h

private:
	DllSharedData* m_pShareData;
};

