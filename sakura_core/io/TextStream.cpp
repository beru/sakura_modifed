#include "StdAfx.h"
#include "TextStream.h"
#include "charset/CodeFactory.h"
#include "charset/ShiftJis.h"	// move from CodeMediator.h	2010/6/14 Uchi
#include "charset/Utf8.h"		// move from CodeMediator.h	2010/6/14 Uchi
#include "Eol.h"
#include "util/fileUtil.h"			// _IS_REL_PATH
#include "util/module.h"

using namespace std;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     TextInputStream                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

TextInputStream::TextInputStream(const TCHAR* tszPath)
	:
	Stream(tszPath, _T("rb"))
{
	m_bIsUtf8 = false;

	if (Good()) {
		// BOM�m�F -> m_bIsUtf8
		static const BYTE utf8_bom[] = {0xEF, 0xBB, 0xBF};
		BYTE buf[3];
		if (fread(&buf, 1, sizeof(utf8_bom), GetFp()) == sizeof(utf8_bom)) {
			m_bIsUtf8 = (memcmp(buf, utf8_bom, sizeof(utf8_bom)) == 0);
		}

		// UTF-8����Ȃ���΁A�t�@�C���|�C���^�����ɖ߂�
		if (!m_bIsUtf8) {
			fseek(GetFp(), 0, SEEK_SET);
		}
	}else {
		m_bIsUtf8 = false;
	}
}

/*
TextInputStream::TextInputStream()
: Stream()
{
	m_bIsUtf8=false;
}
*/

TextInputStream::~TextInputStream()
{
}


wstring TextInputStream::ReadLineW()
{
	//$$ ����������Ǎ��̂Ƃ���͋����āB�B
	NativeW line;
	line.AllocStringBuffer(60);
	for (;;) {
		int c = getc(GetFp());
		// EOF�ŏI��
		if (c == EOF) {
			break;
		}
		// "\r" �܂��� "\r\n" �ŏI��
		if (c == '\r') {
			c = getc(GetFp());
			if (c != '\n') {
				ungetc(c, GetFp());
			}
			break;
		}
		// "\n" �ŏI��
		if (c == '\n') {
			break;
		}
		if (line._GetMemory()->capacity() < line._GetMemory()->GetRawLength() + 10) {
			line._GetMemory()->AllocBuffer( line._GetMemory()->GetRawLength() * 2 );
		}
		line._GetMemory()->AppendRawData(&c,sizeof(char));
	}

	NativeW line2;
	// UTF-8 �� UNICODE
	if (m_bIsUtf8) {
		Utf8::UTF8ToUnicode(*(line._GetMemory()), &line2);
	// Shift_JIS �� UNICODE
	}else {
		ShiftJis::SJISToUnicode(*(line._GetMemory()), &line2);
	}

	return wstring().assign( line2.GetStringPtr(), line2.GetStringLength() );	// EOL �܂� NULL �������܂߂�
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     TextOutputStream                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

TextOutputStream::TextOutputStream(
	const TCHAR* tszPath,
	EncodingType eCodeType,
	bool bExceptionMode,
	bool bBom
	)
	: OutputStream(tszPath, _T("wb"), bExceptionMode)
{
	m_pCodeBase = CodeFactory::CreateCodeBase(eCodeType, 0);
	if (Good() && bBom) {
		// BOM�t��
		Memory memBom;
		m_pCodeBase->GetBom(&memBom);
		if (memBom.GetRawLength() > 0) {
			fwrite(memBom.GetRawPtr(), memBom.GetRawLength(), 1, GetFp());
		}
	}
}

TextOutputStream::~TextOutputStream()
{
	delete m_pCodeBase;
}

void TextOutputStream::WriteString(
	const wchar_t*	szData,	// �������ޕ�����
	int				nLen	// �������ޕ����񒷁B-1��n���Ǝ����v�Z�B
	)
{
	//$$����: �����ϊ����ɂ��������R�s�[������Ă�̂Ō����������B��X�������P�\��B

	int nDataLen = nLen;
	if (nDataLen < 0) {
		nDataLen = wcslen(szData);
	}
	const wchar_t* pData = szData;
	const wchar_t* pEnd = szData + nDataLen;

	// 1�s���ɃJ�L�R�B"\n"��"\r\n"�ɕϊ����Ȃ���o�́B�������A"\r\n"��"\r\r\n"�ɕϊ����Ȃ��B
	const wchar_t* p = pData;
	for (;;) {
		// \n�����o�B������\r\n�͏��O�B
		const wchar_t* q = p;
		while (q < pEnd) {
			if (*q == L'\n' && !((q-1) >= p && *(q-1) == L'\r')) {
				break;
			}
			++q;
		}
		const wchar_t* lf = (q < pEnd) ? q : NULL;
		if (lf) {
			// \n�̑O�܂�(p�`lf)�o��
			NativeW src(p, lf-p);
			Memory dst;
			m_pCodeBase->UnicodeToCode(src, &dst); // �R�[�h�ϊ�
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());

			// \r\n���o��
			src.SetString(L"\r\n");
			m_pCodeBase->UnicodeToCode(src, &dst);
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());

			// ����
			p = lf + 1;
		}else {
			// �c�肺��ԏo��
			NativeW src(p, pEnd - p);
			Memory dst;
			m_pCodeBase->UnicodeToCode(src, &dst); // �R�[�h�ϊ�
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());
			break;
		}
	}
}

void TextOutputStream::WriteF(const wchar_t* format, ...)
{
	// �e�L�X�g���` -> buf
	static wchar_t buf[16*1024]; //$$ �m�ۂ����������H
	va_list v;
	va_start(v, format);
	auto_vsprintf_s(buf, _countof(buf), format, v);
	va_end(v);

	// �o��
	WriteString(buf);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  TextInputStream_AbsIni                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

static
const TCHAR* _Resolve(
	const TCHAR* fname,
	bool bOrExedir
	)
{
	if (_IS_REL_PATH(fname)) {
		static TCHAR path[_MAX_PATH];
		if (bOrExedir) {
			GetInidirOrExedir(path, fname);
		}else {
			GetInidir(path, fname);
		}
		return path;
	}
	return fname;
}


TextInputStream_AbsIni::TextInputStream_AbsIni(
	const TCHAR* fname,
	bool bOrExedir
	)
	:
	TextInputStream(_Resolve(fname, bOrExedir))
{
}

