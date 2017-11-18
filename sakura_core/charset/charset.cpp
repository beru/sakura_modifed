#include "StdAfx.h"
#include "charset.h"
#include "CodePage.h"
#include <vector>
#include <map>

struct CodeSet {
	EncodingType	eCodeSet;
	const wchar_t*	sNormal;
	const wchar_t*	sShort;
	const wchar_t*	sLong;			// for Combo
	bool			bUseBom;		// BOM���g���邩
	bool			bIsBomDefOn;	// BOM�̃f�t�H���g��On��
	bool			bCanDefault;	// �f�t�H���g�����R�[�h�ɂȂ�邩
};

// �����R�[�h�Z�b�g(�����f�[�^)
static CodeSet ASCodeSet[] = {
	{ CODE_AUTODETECT,	L"Auto",	L"Auto",	L"�����I��",	false,	false,	false },	// �����R�[�h��������	// map�ɂ͓���Ȃ�
	{ CODE_SJIS,		L"SJIS",	L"SJIS",	L"SJIS",		false,	false,	true  },	// SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	{ CODE_JIS,			L"JIS",		L"JIS",		L"JIS",			false,	false,	false },	// JIS				(MS-CP5022x(ISO-2022-JP-MS))
	{ CODE_EUC,			L"EUC",		L"EUC",		L"EUC-JP",		false,	false,	true  },	// EUC				(MS-CP51932)	// eucJP-ms(eucJP-open)�ł͂Ȃ�
	{ CODE_LATIN1,		L"Latin1",	L"Latin1",	L"Latin1",		false,	false,	true  },	// Latin1				(����, Windows-932, Windows Codepage 1252 West European)
	{ CODE_UNICODE,		L"Unicode",	L"Uni",		L"Unicode",		true,	true,	true  },	// Unicode			(UTF-16 LittleEndian)	// UCS-2
	{ CODE_UNICODEBE,	L"UniBE",	L"UniBE",	L"UnicodeBE",	true,	true,	true  },	// Unicode BigEndian	(UTF-16 BigEndian)		// UCS-2
	{ CODE_UTF8,		L"UTF-8",	L"UTF-8",	L"UTF-8",		true,	false,	true  },	// UTF-8
	{ CODE_CESU8,		L"CESU-8",	L"CESU-8",	L"CESU-8",		true,	false,	true  },	// CESU-8				(UCS-2����UTF-8��)
	{ CODE_UTF7,		L"UTF-7",	L"UTF-7",	L"UTF-7",		true,	false,	false },	// UTF-7
};

// �����R�[�h�Z�b�g
typedef	std::map<int, CodeSet>	MSCodeSet;
static MSCodeSet				msCodeSet;
// �\����
static std::vector<EncodingType>	vDispIdx;


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ������                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void InitCodeSet()
{
	if (msCodeSet.empty()) {
		for (size_t i=0; i<_countof(ASCodeSet); ++i) {
			vDispIdx.push_back(ASCodeSet[i].eCodeSet);
			if (i > 0) {
				msCodeSet[ASCodeSet[i].eCodeSet] = ASCodeSet[i];
			}
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool IsValidCodeType(int code)
{
	// ������
	InitCodeSet();
	return (msCodeSet.find(code) != msCodeSet.end());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR CodeTypeName::Normal() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar(msCodeSet[eCodeType].sNormal);
}

LPCTSTR CodeTypeName::Short() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar(msCodeSet[eCodeType].sShort);
}

LPCTSTR CodeTypeName::Bracket() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}

//	static	std::wstring	sWork = L"  [" + msCodeSet[eCodeType].sShort + L"]";
	static	std::wstring	sWork;
	sWork = std::wstring(L"  [") + msCodeSet[eCodeType].sShort + L"]";	// �ϐ��̒�`�ƒl�̐ݒ���ꏏ�ɂ��ƃo�O��l�Ȃ̂ŕ���

	return to_tchar(sWork.c_str());
}


bool CodeTypeName::UseBom()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		if (IsValidCodeOrCPType(eCodeType)) {
			CodePage encoding(eCodeType);
			Memory mem;
			encoding.GetBom(&mem);
			return 0 < mem.GetRawLength();
		}
		return false;
	}

	return msCodeSet[eCodeType].bUseBom;
}

bool CodeTypeName::IsBomDefOn()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return false;
	}

	return msCodeSet[eCodeType].bIsBomDefOn;
}

bool CodeTypeName::CanDefault()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		if (IsValidCodeOrCPType(eCodeType)) {
			return true;
		}
		return false;
	}

	return msCodeSet[eCodeType].bCanDefault;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �R���{�{�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

size_t CodeTypesForCombobox::GetCount() const
{
	return vDispIdx.size();
}

EncodingType CodeTypesForCombobox::GetCode(size_t nIndex) const
{
	return vDispIdx[nIndex];
}

LPCTSTR CodeTypesForCombobox::GetName(size_t nIndex) const
{
	if (nIndex == 0) {
		return LS(STR_ERR_GLOBAL01);
	}
	return to_tchar(msCodeSet[vDispIdx[nIndex]].sLong);
}
