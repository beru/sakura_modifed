/*!	@file
	@brief Eol�N���X�̎���

	@author genta
	@date 2000/05/15 �V�K�쐬 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, Frozen, Moca

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
#include "Eol.h"

// �s�I�[�q�̔z��
const EolType g_pnEolTypeArr[EOL_TYPE_NUM] = {
	EolType::None			,	// == 0
	EolType::CRLF			,	// == 2
	EolType::LF				,	// == 1
	EolType::CR				,	// == 1
	EolType::NEL				,	// == 1
	EolType::LS				,	// == 1
	EolType::PS					// == 1
};


//-----------------------------------------------
//	�Œ�f�[�^
//-----------------------------------------------

struct EolDefinition {
	const TCHAR*	szName;
	const WCHAR*	szDataW;
	const ACHAR*	szDataA;
	size_t			nLen;

	bool StartsWith(const WCHAR* pData, size_t nLen) const { return this->nLen <= nLen && auto_memcmp(pData, szDataW, this->nLen) == 0; }
	bool StartsWith(const ACHAR* pData, size_t nLen) const { return this->nLen <= nLen && szDataA[0] != '\0' && auto_memcmp(pData, szDataA, this->nLen) == 0; }
};

static const EolDefinition g_aEolTable[] = {
	{ _T("���s��"),	L"",			"",			0 },
	{ _T("CRLF"),	L"\x0d\x0a",	"\x0d\x0a",	2 },
	{ _T("LF"),		L"\x0a",		"\x0a",		1 },
	{ _T("CR"),		L"\x0d",		"\x0d",		1 },
	{ _T("NEL"),	L"\x85",		"",			1 },
	{ _T("LS"),		L"\u2028",		"",			1 },
	{ _T("PS"),		L"\u2029",		"",			1 },
};

struct EolDefinitionForUniFile {
	const char*	szDataW;
	const char* szDataWB;
	size_t		nLen;

	bool StartsWithW(const char* pData, size_t nLen) const { return this->nLen <= nLen && memcmp(pData, szDataW, this->nLen) == 0; }
	bool StartsWithWB(const char* pData, size_t nLen) const { return this->nLen <= nLen && memcmp(pData, szDataWB, this->nLen) == 0; }
};
static const EolDefinitionForUniFile g_aEolTable_uni_file[] = {
	{ "",					"", 					0 },
	{ "\x0d\x00\x0a\x00",	"\x00\x0d\x00\x0a",		4 },
	{ "\x0a\x00",			"\x00\x0a",				2 },
	{ "\x0d\x00",			"\x00\x0d",				2 },
	{ "\x85\x00",			"\x00\x85",				2 },
	{ "\x28\x20",			"\x20\x28",				2 },
	{ "\x29\x20",			"\x20\x29",				2 },
};


//-----------------------------------------------
//	�����⏕
//-----------------------------------------------

/*!
	�s�I�[�q�̎�ނ𒲂ׂ�B
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EolType::None��Ԃ��B
*/
template <class T>
EolType GetEOLType(const T* pszData, size_t nDataLen)
{
	for (size_t i=1; i<EOL_TYPE_NUM; ++i) {
		if (g_aEolTable[i].StartsWith(pszData, nDataLen)) {
			return g_pnEolTypeArr[i];
		}
	}
	return EolType::None;
}


/*
	�t�@�C����ǂݍ��ނƂ��Ɏg�p�������
*/

EolType _GetEOLType_uni(const char* pszData, size_t nDataLen)
{
	for (int i=1; i<EOL_TYPE_NUM; ++i) {
		if (g_aEolTable_uni_file[i].StartsWithW(pszData, nDataLen)) {
			return g_pnEolTypeArr[i];
		}
	}
	return EolType::None;
}

EolType _GetEOLType_unibe(const char* pszData, size_t nDataLen)
{
	for (int i=1; i<EOL_TYPE_NUM; ++i) {
		if (g_aEolTable_uni_file[i].StartsWithWB(pszData, nDataLen)) {
			return g_pnEolTypeArr[i];
		}
	}
	return EolType::None;
}

//-----------------------------------------------
//	������
//-----------------------------------------------


// ���݂�EOL�����擾�B�����P�ʁB
size_t Eol::GetLen() const
{
	return g_aEolTable[(int)eEolType].nLen;
}

// ���݂�EOL�̖��̎擾
const TCHAR* Eol::GetName() const
{
	return g_aEolTable[(int)eEolType].szName;
}

// ���݂�EOL������擪�ւ̃|�C���^���擾
const wchar_t* Eol::GetValue2() const
{
	return g_aEolTable[(int)eEolType].szDataW;
}

/*!
	�s����ʂ̐ݒ�B
	@param t �s�����
	@retval true ����I���B�ݒ肪���f���ꂽ�B
	@retval false �ُ�I���B�����I��CRLF�ɐݒ�B
*/
bool Eol::SetType(EolType t)
{
	if (t < EolType::None || EolType::CodeMax <= t) {
		//	�ُ�l
		eEolType = EolType::CRLF;
		return false;
	}
	//	�������l
	eEolType = t;
	return true;
}

void Eol::SetTypeByString(const wchar_t* pszData, size_t nDataLen)
{
	SetType(GetEOLType(pszData, nDataLen));
}

void Eol::SetTypeByString(const char* pszData, size_t nDataLen)
{
	SetType(GetEOLType(pszData, nDataLen));
}

void Eol::SetTypeByStringForFile_uni(const char* pszData, size_t nDataLen)
{
	SetType(_GetEOLType_uni(pszData, nDataLen));
}

void Eol::SetTypeByStringForFile_unibe(const char* pszData, size_t nDataLen)
{
	SetType(_GetEOLType_unibe(pszData, nDataLen));
}

