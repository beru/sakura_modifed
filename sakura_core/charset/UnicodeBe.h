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

#include "CodeBase.h"
#include "Unicode.h"
#include "Eol.h"

class UnicodeBe : public CodeBase {
public:
	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UnicodeBEToUnicode(src, pDst); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUnicodeBE(src, pDst); }	// UNICODE    �� ����R�[�h �ϊ�
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eolType);	// ���s�f�[�^�擾

public:

	inline static CodeConvertResult UnicodeBEToUnicode(const Memory& src, NativeW* pDst)
		{ return Unicode::_UnicodeToUnicode_in(src, pDst, true); }	// UnicodeBE �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	inline static CodeConvertResult UnicodeToUnicodeBE(const NativeW& src, Memory* pDst)
		{ return Unicode::_UnicodeToUnicode_out(src, pDst, true); }	// Unicode   �� UnicodeBE�R�[�h�ϊ�

};

