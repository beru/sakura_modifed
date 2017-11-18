// 設定ファイル等のテキスト入出力を行うためのクラス群。
// .ini や .mac の入出力を扱うときに使うと良いです。
// ※「編集テキスト」を扱うためではなく、あくまでも、.iniや.macのような「設定ファイル」を扱う目的のクラス群です。
//
// 今のところはShiftJISで入出力を行うが、
// 将来はUTF-8等にすることにより、UNICODEデータの欠落が起こらないようにしたい。
#pragma once

#include <string>

#include "Stream.h"
class CodeBase;

// テキスト入力ストリーム (UTF-8, SJIS)
class TextInputStream : public Stream {
public:
	// コンストラクタ・デストラクタ
	TextInputStream(const TCHAR* tszPath);
	TextInputStream();
	virtual ~TextInputStream();

	// 操作
	std::wstring ReadLineW(); // 1行読込。改行は削る

private:
	bool bIsUtf8; // UTF-8ならtrue
};

// テキスト出力ストリーム
// 2008.01.26 kobake 出力文字コードを任意で指定できるように変更
class TextOutputStream : public OutputStream {
public:
	// コンストラクタ・デストラクタ
	TextOutputStream(const TCHAR* tszPath, EncodingType eCodeType = CODE_UTF8, bool bExceptionMode = false, bool bBom = true);
	virtual ~TextOutputStream();

	// 文字列書込。改行を入れたい場合は、文字列内に'\n'を含めること。(クラス側で適切な改行コードに変換して出力します)
	void WriteString(const wchar_t* szData, int nLen = -1);
	void WriteF(const wchar_t* format, ...);

	// 数値書込。(クラス側で適当に整形して出力します)
	void WriteInt(int n);

private:
	CodeBase* pCodeBase;
};


// テキスト入力ストリーム。相対パスの場合はINIファイルのパスからの相対パスとして開く。
class TextInputStream_AbsIni : public TextInputStream {
public:
	TextInputStream_AbsIni(const TCHAR* tszPath, bool bOrExedir = true);
};

