#include "StdAfx.h"
#include "Profile.h"
#include "io/TextStream.h"
#include "charset/Utf8.h"		// Resource�ǂݍ��݂Ɏg�p
#include "Eol.h"
#include "util/fileUtil.h"
#include "timer.h"
#include "debug/trace.h"

using namespace std;

/*! Profile��������
*/
void Profile::Init(void)
{
	strProfileName = _T("");
	profileData.clear();
	bRead = true;
	return;
}

/*!
	sakura.ini��1�s����������D

	1�s�̓ǂݍ��݂��������邲�ƂɌĂ΂��D
	
	@param line [in] �ǂݍ��񂾍s
*/
void Profile::ReadOneline(
	const wstring& line
	)
{
	// ��s��ǂݔ�΂�
	if (line.empty()) {
		return;
	}

	// �R�����g�s��ǂ݂Ƃ΂�
	if (line.compare(0, 2, LTEXT("//")) == 0) {
		return;
	}

	// �Z�N�V�����擾
	//	Jan. 29, 2004 genta compare�g�p
	if (1
		&& line.compare(0, 1, LTEXT("[")) == 0 
		&& line.find(LTEXT("=")) == line.npos
		&& line.find(LTEXT("]")) == ( line.size() - 1 )
	) {
		profileData.emplace_back(line.substr(1, line.size() - 1 - 1));
	// �G���g���擾
	}else if (!profileData.empty()) {	// �ŏ��̃Z�N�V�����ȑO�̍s�̃G���g���͖���
		wstring::size_type idx = line.find( LTEXT("=") );
		if (idx != line.npos) {
			profileData.back().mapEntries.emplace(
				line.substr(0,idx),
				line.substr(idx+1)
			);
		}
	}
}

/*!
sakura.ini��1�s����������D

1�s�̓ǂݍ��݂��������邲�ƂɌĂ΂��D

@param line [in] �ǂݍ��񂾍s
*/
void Profile::ReadOneline(
	const wchar_t* line,
	size_t len
	)
{
	// ��s��ǂݔ�΂�
	if (len == 0) {
		return;
	}

	wchar_t fl = line[0];
	// �R�����g�s��ǂ݂Ƃ΂�
	if (fl == ';') {
		return;
	}
	if (len >= 2 && fl == '/' && line[1] == '/') {
		return;
	}

	// �Z�N�V�����擾
	//	Jan. 29, 2004 genta compare�g�p
	const wchar_t* lineEnd = line + len;
	const wchar_t* eqPos = std::find(line, lineEnd, '=');

	if (fl == '['
		&& eqPos == lineEnd
		&& line[len - 1] == ']'
	) {
		profileData.emplace_back(std::wstring(line + 1, len - 1 - 1));
		// �G���g���擾
	}else if (!profileData.empty()) {	// �ŏ��̃Z�N�V�����ȑO�̍s�̃G���g���͖���
		if (eqPos != lineEnd) {
			profileData.back().mapEntries.emplace(
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

/*! Profile���t�@�C������ǂݏo��
	
	@param pszProfileName [in] �t�@�C����

	@retval true  ����
	@retval false ���s
*/
bool Profile::ReadProfile(const TCHAR* pszProfileName)
{
	Timer t;
	strProfileName = pszProfileName;

	std::vector<wchar_t> buff;
	if (!ReadFileAndUnicodify(pszProfileName, buff)) {
		return false;
	}
	wchar_t* pBuff = &buff[0];
	size_t remainLen = buff.size();
	size_t lineLen;
	size_t lineLenWithoutCrLf;
	size_t lineCount = 0;
	for (;;) {
		// 1�s�Ǎ�
		if (!findLine(pBuff, remainLen, lineLen, lineLenWithoutCrLf)) {
			break;
		}
		++lineCount;
		// ���
		ReadOneline(pBuff, lineLenWithoutCrLf);

		pBuff += lineLen;
		remainLen -= lineLen;
	}

	TRACE(L"ReadProfile time %f\n", t.ElapsedSecond());
	return true;
}


/*! Profile�����\�[�X����ǂݏo��
	
	@param pName [in] ���\�[�X��
	@param pType [in] ���\�[�X�^�C�v

	@retval true  ����
	@retval false ���s

	1�s300�����܂łɐ���
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
	strProfileName = _T("-Res-");

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
			// 1�s�؂���i���������ꍇ�؎̂āj
			pn = strpbrk(p, "\n");
			if (!pn) {
				// �ŏI�s
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
				// ���
				ReadOneline(line);
			}
		}
	}
	return true;
}

/*! Profile���t�@�C���֏����o��
	
	@param pszProfileName [in] �t�@�C����(NULL=�Ō�ɓǂݏ��������t�@�C��)
	@param pszComment [in] �R�����g��(NULL=�R�����g�ȗ�)

	@retval true  ����
	@retval false ���s
*/
bool Profile::WriteProfile(
	const TCHAR* pszProfileName,
	const wchar_t* pszComment
	)
{
	Timer t;

	if (pszProfileName) {
		strProfileName = pszProfileName;
	}
    
	std::vector<wstring> vecLine;
	if (pszComment) {
		vecLine.push_back(LTEXT(";") + wstring(pszComment));		// //->;	2008/5/24 Uchi
		vecLine.push_back(LTEXT(""));
	}
	auto iterEnd = profileData.end();
	for (auto iter=profileData.begin(); iter!=iterEnd; ++iter) {
		// �Z�N�V����������������
		vecLine.push_back(LTEXT("[") + iter->strSectionName + LTEXT("]"));
		auto mapiterEnd = iter->mapEntries.end();
		for (auto mapiter=iter->mapEntries.begin(); mapiter!=mapiterEnd; ++mapiter) {
			// �G���g������������
			vecLine.push_back(mapiter->first + LTEXT("=") + mapiter->second);
		}
		vecLine.push_back(LTEXT(""));
	}

	// �ʃt�@�C���ɏ�������ł���u��������i�v���Z�X�����I���Ȃǂւ̈��S�΍�j
	TCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = _T('\0');
	TCHAR szPath[_MAX_PATH];
	LPTSTR lpszName;
	DWORD nLen = ::GetFullPathName(strProfileName.c_str(), _countof(szPath), szPath, &lpszName);
	if (0 < nLen && nLen < _countof(szPath)
		&& (lpszName - szPath + 11) < _countof(szMirrorFile) )	// path\preuuuu.TMP
	{
		*lpszName = _T('\0');
		::GetTempFileName(szPath, _T("sak"), 0, szMirrorFile);
	}

	if (!_WriteFile(szMirrorFile[0]? szMirrorFile: strProfileName, vecLine)) {
		return false;
	}

	if (szMirrorFile[0]) {
		BOOL (__stdcall *pfnReplaceFile)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);
		HMODULE hModule = ::GetModuleHandle(_T("KERNEL32"));
		pfnReplaceFile = (BOOL (__stdcall *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID))
			::GetProcAddress(hModule, "ReplaceFileW");
		if (!pfnReplaceFile || !pfnReplaceFile(strProfileName.c_str(), szMirrorFile, NULL, 0, NULL, NULL)) {
			if (fexist(strProfileName.c_str())) {
				if (!::DeleteFile(strProfileName.c_str())) {
					return false;
				}
			}
			if (!::MoveFile(szMirrorFile, strProfileName.c_str())) {
				return false;
			}
		}
	}

	TRACE(L"WriteProfile time %f\n", t.ElapsedSecond());
	return true;
}

/*! �t�@�C���֏�������
	
	@retval true  ����
	@retval false ���s
*/
bool Profile::_WriteFile(
	const tstring&			strFilename,	// [in]  �t�@�C����
	const vector<wstring>&	vecLine			// [out] ������i�[��
	)
{
	TextOutputStream out(strFilename.c_str());
	if (!out) {
		return false;
	}

	int nSize = (int)vecLine.size();
	for (int i=0; i<nSize; ++i) {
		// �o��
		out.WriteString(vecLine[i].c_str());
		out.WriteString(L"\n");
	}

	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                            Imp                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �G���g���l��Profile����ǂݍ���
	
	@retval true ����
	@retval false ���s
*/
bool Profile::GetProfileDataImp(
	const wstring&	strSectionName,	// [in] �Z�N�V������
	const wstring&	strEntryKey,	// [in] �G���g����
	wstring&		strEntryValue	// [out] �G���g���l
	)
{
	auto iterEnd = profileData.end();
	for (auto iter = profileData.begin(); iter != iterEnd; ++iter) {
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

/*! �G���g����Profile�֏�������
	
	@retval true  ����
	@retval false ���s(���������Ă��Ȃ��̂�false�͕Ԃ�Ȃ�)
*/
bool Profile::SetProfileDataImp(
	const wstring&	strSectionName,	// [in] �Z�N�V������
	const wstring&	strEntryKey,	// [in] �G���g����
	const wstring&	strEntryValue	// [in] �G���g���l
	)
{
	auto iterEnd = profileData.end();
	auto iter = profileData.begin();
	for (; iter != iterEnd; ++iter) {
		if (iter->strSectionName == strSectionName) {
			// �����̃Z�N�V�����̏ꍇ
			auto mapiter = iter->mapEntries.find(strEntryKey);
			if (iter->mapEntries.end() != mapiter) {
				// �����̃G���g���̏ꍇ�͒l���㏑��
				mapiter->second = strEntryValue;
				break;
			}else {
				// �����̃G���g����������Ȃ��ꍇ�͒ǉ�
				iter->mapEntries.emplace(strEntryKey, strEntryValue);
				break;
			}
		}
	}
	// �����̃Z�N�V�����ł͂Ȃ��ꍇ�C�Z�N�V�����y�уG���g����ǉ�
	if (iter == iterEnd) {
		profileData.emplace_back(strSectionName);
		Section& section = profileData.back();
		section.mapEntries.emplace(strEntryKey, strEntryValue);
	}
	return true;
}


void Profile::Dump(void)
{
#ifdef _DEBUG
	auto iterEnd = profileData.end();
	// 2006.02.20 ryoji: map_str_str_iter�폜���̏C���R��ɂ��R���p�C���G���[�C��
	MYTRACE(_T("\n\nCProfile::DUMP()======================"));
	for (auto iter=profileData.begin(); iter!=iterEnd; ++iter) {
		MYTRACE(_T("\n��strSectionName=%ls"), iter->strSectionName.c_str());
		auto mapiterEnd = iter->mapEntries.end();
		for (auto mapiter=iter->mapEntries.begin(); mapiter!=mapiterEnd; ++mapiter) {
			MYTRACE(_T("\"%ls\" = \"%ls\"\n"), mapiter->first.c_str(), mapiter->second.c_str());
		}
	}
	MYTRACE(_T("========================================\n"));
#endif
	return;
}

