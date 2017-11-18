#include "StdAfx.h"
#include "Eol.h"

// 行終端子の配列
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
//	固定データ
//-----------------------------------------------

struct EolDefinition {
	const TCHAR*	szName;
	const wchar_t*	szDataW;
	const char*		szDataA;
	size_t			nLen;

	bool StartsWith(const wchar_t* pData, size_t nLen) const { return this->nLen <= nLen && auto_memcmp(pData, szDataW, this->nLen) == 0; }
	bool StartsWith(const char* pData, size_t nLen) const { return this->nLen <= nLen && szDataA[0] != '\0' && auto_memcmp(pData, szDataA, this->nLen) == 0; }
};

static const EolDefinition g_aEolTable[] = {
	{ _T("改行無"),	L"",			"",			0 },
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
//	実装補助
//-----------------------------------------------

/*!
	行終端子の種類を調べる。
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ
	@return 改行コードの種類。終端子が見つからなかったときはEolType::Noneを返す。
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
	ファイルを読み込むときに使用するもの
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
//	実装部
//-----------------------------------------------


// 現在のEOL長を取得。文字単位。
size_t Eol::GetLen() const
{
	return g_aEolTable[(int)eEolType].nLen;
}

// 現在のEOLの名称取得
const TCHAR* Eol::GetName() const
{
	return g_aEolTable[(int)eEolType].szName;
}

// 現在のEOL文字列先頭へのポインタを取得
const wchar_t* Eol::GetValue2() const
{
	return g_aEolTable[(int)eEolType].szDataW;
}

/*!
	行末種別の設定。
	@param t 行末種別
	@retval true 正常終了。設定が反映された。
	@retval false 異常終了。強制的にCRLFに設定。
*/
bool Eol::SetType(EolType t)
{
	if (t < EolType::None || EolType::CodeMax <= t) {
		//	異常値
		eEolType = EolType::CRLF;
		return false;
	}
	//	正しい値
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

