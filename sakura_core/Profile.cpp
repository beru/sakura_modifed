/*!	@file
	@brief INIファイル入出力

	@author D.S.Koba
	@date 2003-10-21 D.S.Koba メンバ関数の名前と引数をそのままにしてメンバ変数，関数の中身を書き直し
	@date 2004-01-10 D.S.Koba 返値をBOOLからboolへ変更。IOProfileDataを型別の関数に分け，引数を減らす
	@date 2006-02-11 D.S.Koba 読み込み/書き出しを引数でなく，メンバで判別
	@date 2006-02-12 D.S.Koba IOProfileDataの中身の読み込みと書き出しを関数に分ける
*/
/*
	Copyright (C) 2003, D.S.Koba
	Copyright (C) 2004, D.S.Koba, MIK, genta
	Copyright (C) 2006, D.S.Koba, ryoji
	Copyright (C) 2009, ryoji

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
#include "Profile.h"
#include "io/TextStream.h"
#include "charset/Utf8.h"		// Resource読み込みに使用
#include "Eol.h"
#include "util/fileUtil.h"
#include "timer.h"
#include "debug/trace.h"

using namespace std;

/*! Profileを初期化
	
	@date 2003-10-21 D.S.Koba STLで書き直す
*/
void Profile::Init(void)
{
	m_strProfileName = _T("");
	m_profileData.clear();
	m_bRead = true;
	return;
}

/*!
	sakura.iniの1行を処理する．

	1行の読み込みが完了するごとに呼ばれる．
	
	@param line [in] 読み込んだ行
*/
void Profile::ReadOneline(
	const wstring& line
	)
{
	// 空行を読み飛ばす
	if (line.empty()) {
		return;
	}

	// コメント行を読みとばす
	if (line.compare(0, 2, LTEXT("//")) == 0) {
		return;
	}

	// セクション取得
	//	Jan. 29, 2004 genta compare使用
	if (1
		&& line.compare(0, 1, LTEXT("[")) == 0 
		&& line.find(LTEXT("=")) == line.npos
		&& line.find(LTEXT("]")) == ( line.size() - 1 )
	) {
		Section sec;
		sec.strSectionName = line.substr( 1, line.size() - 1 - 1 );
		m_profileData.push_back(sec);
	// エントリ取得
	}else if (!m_profileData.empty()) {	// 最初のセクション以前の行のエントリは無視
		wstring::size_type idx = line.find( LTEXT("=") );
		if (idx != line.npos) {
			m_profileData.back().mapEntries.insert( pair_str_str( line.substr(0,idx), line.substr(idx+1) ) );
		}
	}
}

/*!
sakura.iniの1行を処理する．

1行の読み込みが完了するごとに呼ばれる．

@param line [in] 読み込んだ行
*/
void Profile::ReadOneline(
	const wchar_t* line,
	size_t len
	)
{
	// 空行を読み飛ばす
	if (len == 0) {
		return;
	}

	wchar_t fl = line[0];
	// コメント行を読みとばす
	if (fl == ';') {
		return;
	}
	if (len >= 2 && fl == '/' && line[1] == '/') {
		return;
	}

	// セクション取得
	//	Jan. 29, 2004 genta compare使用
	const wchar_t* lineEnd = line + len;
	const wchar_t* eqPos = std::find(line, lineEnd, '=');

	if (fl == '['
		&& eqPos == lineEnd
		&& line[len - 1] == ']'
	) {
		Section sec;
		sec.strSectionName = std::wstring(line + 1, len - 1 - 1);
		m_profileData.push_back(sec);
		// エントリ取得
	}else if (!m_profileData.empty()) {	// 最初のセクション以前の行のエントリは無視
		if (eqPos != lineEnd) {
			m_profileData.back().mapEntries.emplace(
				std::wstring(line, eqPos - line),
				std::wstring(eqPos + 1, len - (eqPos - line) - 1)
			);
		}
	}
}

static
bool findLine(
	const wchar_t* pData,
	size_t remainLen,
	size_t& lineLen,
	size_t& lineLenWithoutCrLf
	)
{
	if (remainLen == 0) {
		lineLen = 0;
		lineLenWithoutCrLf = 0;
		return false;
	}
	for (size_t i=0; i<remainLen; ++i) {
		wchar_t c = pData[i];
		if (c == L'\n') {
			lineLen = i + 1;
			lineLenWithoutCrLf = i;
			return true;
		}else if (c == L'\r') {
			lineLenWithoutCrLf = i;
			if (i + i >= remainLen) {
				lineLen = i + 1;
			}else {
				c = pData[i + 1];
				if (c == L'\n') {
					lineLen = i + 2;
				}else {
					lineLen = i + 1;
				}
			}
			return true;
		}
	}
	lineLen = remainLen;
	lineLenWithoutCrLf = remainLen;
	return true;
}

/*! Profileをファイルから読み出す
	
	@param pszProfileName [in] ファイル名

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba STLで書き直す
	@date 2003-10-26 D.S.Koba ReadProfile()から分離
	@date 2004-01-29 genta stream使用をやめてCライブラリ使用に．
	@date 2004-01-31 genta 行の解析の方を別関数にしてReadFileをReadProfileに
		
*/
bool Profile::ReadProfile(const TCHAR* pszProfileName)
{
	Timer t;

	m_strProfileName = pszProfileName;

#if 1
	std::vector<char> buff;
	if (!ReadFile(pszProfileName, buff) || buff.size() < 3) {
		return false;
	}

	const char* pBuff = &buff[0];
	size_t buffSize = buff.size();
	static const uint8_t utf8_bom[] = { 0xEF, 0xBB, 0xBF };
	bool isUtf8 = (memcmp(utf8_bom, pBuff, 3) == 0);
	if (isUtf8) {
		pBuff += 3;
		buffSize -= 3;
	}
	static const UINT cp_sjis = 932;
	UINT codePage = isUtf8 ? CP_UTF8 : cp_sjis;

	std::vector<wchar_t> wbuff(buffSize);
	wchar_t* pWBuff = &wbuff[0];
	int ret = MultiByteToWideChar(codePage, 0, pBuff, buffSize, pWBuff, buffSize);
	if (ret <= 0) {
		return false;
	}
	size_t remainLen = ret;
	size_t lineLen;
	size_t lineLenWithoutCrLf;
	size_t lineCount = 0;
	for (;;) {
		// 1行読込
		if (!findLine(pWBuff, remainLen, lineLen, lineLenWithoutCrLf)) {
			break;
		}
		++lineCount;
		// 解析
		ReadOneline(pWBuff, lineLenWithoutCrLf);

		pWBuff += lineLen;
		remainLen -= lineLen;
	}
#else

	TextInputStream in(m_strProfileName.c_str());
	if (!in) {
		return false;
	}

	try {
		while (in) {
			// 1行読込
			wstring line = in.ReadLineW();
			// 解析
			ReadOneline(line);
		}
	}catch (...) {
		return false;
	}
#endif

	TRACE(L"ReadProfile time %f\n", t.ElapsedSecond());

	return true;
}


/*! Profileをリソースから読み出す
	
	@param pName [in] リソース名
	@param pType [in] リソースタイプ

	@retval true  成功
	@retval false 失敗

	@date 2010/5/19 MainMenu用に作成

	1行300文字までに制限
*/
bool Profile::ReadProfileRes(
	const TCHAR* pName,
	const TCHAR* pType,
	std::vector<std::wstring>* pData
	)
{
	static const BYTE utf8_bom[] = {0xEF, 0xBB, 0xBF};
	HRSRC		hRsrc;
	HGLOBAL		hGlobal;
	size_t		nSize;
	char*		psMMres;
	char*		p;
	char		sLine[300 + 1];
	char*		pn;
	size_t		lnsz;
	wstring		line;
	Memory mLine;
	NativeW mLineW;
	m_strProfileName = _T("-Res-");

	if (1
		&& (hRsrc = ::FindResource(0, pName, pType))
		&& (hGlobal = ::LoadResource(0, hRsrc))
		&& (psMMres = (char*)::LockResource(hGlobal))
		&& (nSize = (size_t)::SizeofResource(0, hRsrc)) != 0
	) {
		p = psMMres;
		if (nSize >= sizeof(utf8_bom) && memcmp(p, utf8_bom, sizeof(utf8_bom)) == 0) {
			// Skip BOM
			p += sizeof(utf8_bom);
		}
		for (; p < psMMres+nSize; p = pn) {
			// 1行切り取り（長すぎた場合切捨て）
			pn = strpbrk(p, "\n");
			if (!pn) {
				// 最終行
				pn = psMMres + nSize;
			}else {
				++pn;
			}
			lnsz = (pn - p) <= 300 ? (pn - p) : 300;
			memcpy(sLine, p, lnsz);
			sLine[lnsz] = '\0';
			if (sLine[lnsz - 1] == '\n') {
				sLine[--lnsz] = '\0';
			}
			if (sLine[lnsz - 1] == '\r') {
				sLine[--lnsz] = '\0';
			}
			
			// UTF-8 -> UNICODE
			mLine.SetRawDataHoldBuffer( sLine, lnsz );
			Utf8::UTF8ToUnicode( mLine, &mLineW );
			line = mLineW.GetStringPtr();

			if (pData) {
				pData->push_back(line);
			}else {
				// 解析
				ReadOneline(line);
			}
		}
	}
	return true;
}

/*! Profileをファイルへ書き出す
	
	@param pszProfileName [in] ファイル名(NULL=最後に読み書きしたファイル)
	@param pszComment [in] コメント文(NULL=コメント省略)

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba STLで書き直す
	@date 2004-01-28 D.S.Koba ファイル書き込み部を分離
	@date 2009.06.24 ryoji 別ファイルに書き込んでから置き換える処理を追加
*/
bool Profile::WriteProfile(
	const TCHAR* pszProfileName,
	const WCHAR* pszComment
	)
{
	Timer t;

	if (pszProfileName) {
		m_strProfileName = pszProfileName;
	}
    
	std::vector<wstring> vecLine;
	if (pszComment) {
		vecLine.push_back(LTEXT(";") + wstring(pszComment));		// //->;	2008/5/24 Uchi
		vecLine.push_back(LTEXT(""));
	}
	auto iterEnd = m_profileData.end();
	for (auto iter=m_profileData.begin(); iter!=iterEnd; ++iter) {
		// セクション名を書き込む
		vecLine.push_back(LTEXT("[") + iter->strSectionName + LTEXT("]"));
		auto mapiterEnd = iter->mapEntries.end();
		for (auto mapiter=iter->mapEntries.begin(); mapiter!=mapiterEnd; ++mapiter) {
			// エントリを書き込む
			vecLine.push_back(mapiter->first + LTEXT("=") + mapiter->second);
		}
		vecLine.push_back(LTEXT(""));
	}

	// 別ファイルに書き込んでから置き換える（プロセス強制終了などへの安全対策）
	TCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = _T('\0');
	TCHAR szPath[_MAX_PATH];
	LPTSTR lpszName;
	DWORD nLen = ::GetFullPathName(m_strProfileName.c_str(), _countof(szPath), szPath, &lpszName);
	if (0 < nLen && nLen < _countof(szPath)
		&& (lpszName - szPath + 11) < _countof(szMirrorFile) )	// path\preuuuu.TMP
	{
		*lpszName = _T('\0');
		::GetTempFileName(szPath, _T("sak"), 0, szMirrorFile);
	}

	if (!_WriteFile(szMirrorFile[0]? szMirrorFile: m_strProfileName, vecLine)) {
		return false;
	}

	if (szMirrorFile[0]) {
		BOOL (__stdcall *pfnReplaceFile)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);
		HMODULE hModule = ::GetModuleHandle(_T("KERNEL32"));
		pfnReplaceFile = (BOOL (__stdcall *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID))
#ifndef _UNICODE
			::GetProcAddress(hModule, "ReplaceFileA");
#else
			::GetProcAddress(hModule, "ReplaceFileW");
#endif
		if (!pfnReplaceFile || !pfnReplaceFile(m_strProfileName.c_str(), szMirrorFile, NULL, 0, NULL, NULL)) {
			if (fexist(m_strProfileName.c_str())) {
				if (!::DeleteFile(m_strProfileName.c_str())) {
					return false;
				}
			}
			if (!::MoveFile(szMirrorFile, m_strProfileName.c_str())) {
				return false;
			}
		}
	}

	TRACE(L"WriteProfile time %f\n", t.ElapsedSecond());
	return true;
}

/*! ファイルへ書き込む
	
	@retval true  成功
	@retval false 失敗

	@date 2004-01-28 D.S.Koba WriteProfile()から分離
	@date 2004-01-29 genta stream使用をやめてCライブラリ使用に．
*/
bool Profile::_WriteFile(
	const tstring&			strFilename,	// [in]  ファイル名
	const vector<wstring>&	vecLine			// [out] 文字列格納先
	)
{
	TextOutputStream out(strFilename.c_str());
	if (!out) {
		return false;
	}

	int nSize = (int)vecLine.size();
	for (int i=0; i<nSize; ++i) {
		// 出力
		out.WriteString(vecLine[i].c_str());
		out.WriteString(L"\n");
	}

	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                            Imp                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! エントリ値をProfileから読み込む
	
	@retval true 成功
	@retval false 失敗

	@date 2003-10-22 D.S.Koba 作成
*/
bool Profile::GetProfileDataImp(
	const wstring&	strSectionName,	// [in] セクション名
	const wstring&	strEntryKey,	// [in] エントリ名
	wstring&		strEntryValue	// [out] エントリ値
	)
{
	auto iterEnd = m_profileData.end();
	for (auto iter = m_profileData.begin(); iter != iterEnd; ++iter) {
		if (iter->strSectionName == strSectionName) {
			auto mapiter = iter->mapEntries.find(strEntryKey);
			if (iter->mapEntries.end() != mapiter) {
				strEntryValue = mapiter->second;
				return true;
			}
		}
	}
	return false;
}

/*! エントリをProfileへ書き込む
	
	@retval true  成功
	@retval false 失敗(処理を入れていないのでfalseは返らない)

	@date 2003-10-21 D.S.Koba 作成
*/
bool Profile::SetProfileDataImp(
	const wstring&	strSectionName,	// [in] セクション名
	const wstring&	strEntryKey,	// [in] エントリ名
	const wstring&	strEntryValue	// [in] エントリ値
	)
{
	auto iterEnd = m_profileData.end();
	auto iter = m_profileData.begin();
	for (; iter != iterEnd; ++iter) {
		if (iter->strSectionName == strSectionName) {
			// 既存のセクションの場合
			auto mapiter = iter->mapEntries.find(strEntryKey);
			if (iter->mapEntries.end() != mapiter) {
				// 既存のエントリの場合は値を上書き
				mapiter->second = strEntryValue;
				break;
			}else {
				// 既存のエントリが見つからない場合は追加
				iter->mapEntries.insert(pair_str_str(strEntryKey, strEntryValue));
				break;
			}
		}
	}
	// 既存のセクションではない場合，セクション及びエントリを追加
	if (iter == iterEnd) {
		Section buffer;
		buffer.strSectionName = strSectionName;
		buffer.mapEntries.insert(pair_str_str(strEntryKey, strEntryValue));
		m_profileData.push_back(buffer);
	}
	return true;
}


void Profile::Dump(void)
{
#ifdef _DEBUG
	auto iterEnd = m_profileData.end();
	// 2006.02.20 ryoji: map_str_str_iter削除時の修正漏れによるコンパイルエラー修正
	MYTRACE(_T("\n\nCProfile::DUMP()======================"));
	for (auto iter=m_profileData.begin(); iter!=iterEnd; ++iter) {
		MYTRACE(_T("\n■strSectionName=%ls"), iter->strSectionName.c_str());
		auto mapiterEnd = iter->mapEntries.end();
		for (auto mapiter=iter->mapEntries.begin(); mapiter!=mapiterEnd; ++mapiter) {
			MYTRACE(_T("\"%ls\" = \"%ls\"\n"), mapiter->first.c_str(), mapiter->second.c_str());
		}
	}
	MYTRACE(_T("========================================\n"));
#endif
	return;
}

