#pragma once

#include "charset/ESI.h"
class EditDoc;

class CodeMediator {
protected:
	// CESI.cpp �̔���֐��������Ɉڂ�
	static EncodingType DetectMBCode(ESI*);
	static EncodingType DetectUnicode(ESI*);

public:

	explicit CodeMediator(const EncodingConfig& ref) : pEncodingConfig(&ref) { }

	static EncodingType DetectUnicodeBom(const char* pS, size_t nLen);

	// ���{��R�[�h�Z�b�g����
	EncodingType CheckKanjiCode(const char*, size_t);
	// �t�@�C���̓��{��R�[�h�Z�b�g����
	EncodingType CheckKanjiCodeOfFile(const TCHAR*);

	static EncodingType CheckKanjiCode(ESI*);  // CESI �\���́i�H�j���O���ō\�z�����ꍇ�Ɏg�p

private:
	const EncodingConfig* pEncodingConfig;
};

