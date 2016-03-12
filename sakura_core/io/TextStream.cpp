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
		// BOM確認 -> m_bIsUtf8
		static const BYTE utf8_bom[] = {0xEF, 0xBB, 0xBF};
		BYTE buf[3];
		if (fread(&buf, 1, sizeof(utf8_bom), GetFp()) == sizeof(utf8_bom)) {
			m_bIsUtf8 = (memcmp(buf, utf8_bom, sizeof(utf8_bom)) == 0);
		}

		// UTF-8じゃなければ、ファイルポインタを元に戻す
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
	//$$ 非効率だけど今のところは許して。。
	NativeW line;
	line.AllocStringBuffer(60);
	for (;;) {
		int c = getc(GetFp());
		// EOFで終了
		if (c == EOF) {
			break;
		}
		// "\r" または "\r\n" で終了
		if (c == '\r') {
			c = getc(GetFp());
			if (c != '\n') {
				ungetc(c, GetFp());
			}
			break;
		}
		// "\n" で終了
		if (c == '\n') {
			break;
		}
		if (line._GetMemory()->capacity() < line._GetMemory()->GetRawLength() + 10) {
			line._GetMemory()->AllocBuffer( line._GetMemory()->GetRawLength() * 2 );
		}
		line._GetMemory()->AppendRawData(&c,sizeof(char));
	}

	NativeW line2;
	// UTF-8 → UNICODE
	if (m_bIsUtf8) {
		Utf8::UTF8ToUnicode(*(line._GetMemory()), &line2);
	// Shift_JIS → UNICODE
	}else {
		ShiftJis::SJISToUnicode(*(line._GetMemory()), &line2);
	}

	return wstring().assign( line2.GetStringPtr(), line2.GetStringLength() );	// EOL まで NULL 文字も含める
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
		// BOM付加
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
	const wchar_t*	szData,	// 書き込む文字列
	int				nLen	// 書き込む文字列長。-1を渡すと自動計算。
	)
{
	//$$メモ: 文字変換時にいちいちコピーを作ってるので効率が悪い。後々効率改善予定。

	int nDataLen = nLen;
	if (nDataLen < 0) {
		nDataLen = wcslen(szData);
	}
	const wchar_t* pData = szData;
	const wchar_t* pEnd = szData + nDataLen;

	// 1行毎にカキコ。"\n"は"\r\n"に変換しながら出力。ただし、"\r\n"は"\r\r\n"に変換しない。
	const wchar_t* p = pData;
	for (;;) {
		// \nを検出。ただし\r\nは除外。
		const wchar_t* q = p;
		while (q < pEnd) {
			if (*q == L'\n' && !((q-1) >= p && *(q-1) == L'\r')) {
				break;
			}
			++q;
		}
		const wchar_t* lf = (q < pEnd) ? q : NULL;
		if (lf) {
			// \nの前まで(p～lf)出力
			NativeW src(p, lf-p);
			Memory dst;
			m_pCodeBase->UnicodeToCode(src, &dst); // コード変換
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());

			// \r\nを出力
			src.SetString(L"\r\n");
			m_pCodeBase->UnicodeToCode(src, &dst);
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());

			// 次へ
			p = lf + 1;
		}else {
			// 残りぜんぶ出力
			NativeW src(p, pEnd - p);
			Memory dst;
			m_pCodeBase->UnicodeToCode(src, &dst); // コード変換
			fwrite(dst.GetRawPtr(), 1, dst.GetRawLength(), GetFp());
			break;
		}
	}
}

void TextOutputStream::WriteF(const wchar_t* format, ...)
{
	// テキスト整形 -> buf
	static wchar_t buf[16*1024]; //$$ 確保しすぎかも？
	va_list v;
	va_start(v, format);
	auto_vsprintf_s(buf, _countof(buf), format, v);
	va_end(v);

	// 出力
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

