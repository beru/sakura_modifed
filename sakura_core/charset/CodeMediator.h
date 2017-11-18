#pragma once

#include "charset/ESI.h"
class EditDoc;

class CodeMediator {
protected:
	// CESI.cpp の判定関数をここに移す
	static EncodingType DetectMBCode(ESI*);
	static EncodingType DetectUnicode(ESI*);

public:

	explicit CodeMediator(const EncodingConfig& ref) : pEncodingConfig(&ref) { }

	static EncodingType DetectUnicodeBom(const char* pS, size_t nLen);

	// 日本語コードセット判別
	EncodingType CheckKanjiCode(const char*, size_t);
	// ファイルの日本語コードセット判別
	EncodingType CheckKanjiCodeOfFile(const TCHAR*);

	static EncodingType CheckKanjiCode(ESI*);  // CESI 構造体（？）を外部で構築した場合に使用

private:
	const EncodingConfig* pEncodingConfig;
};

