/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "charset/CESI.h"
class EditDoc;

class CodeMediator {
protected:
	// CESI.cpp �̔���֐��������Ɉڂ�
	static ECodeType DetectMBCode(ESI*);
	static ECodeType DetectUnicode(ESI*);

public:

	explicit CodeMediator(const EncodingConfig& ref) : m_pEncodingConfig(&ref) { }

	static ECodeType DetectUnicodeBom(const char* pS, const int nLen);

	// ���{��R�[�h�Z�b�g����
	ECodeType CheckKanjiCode(const char*, int);
	// �t�@�C���̓��{��R�[�h�Z�b�g����
	ECodeType CheckKanjiCodeOfFile(const TCHAR*);

	static ECodeType CheckKanjiCode(ESI*);  // CESI �\���́i�H�j���O���ō\�z�����ꍇ�Ɏg�p

private:
	const EncodingConfig* m_pEncodingConfig;
};

