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

#include "StdAfx.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

// ���b�`�e�L�X�g
// JUl. 10, 2001 JEPRO WinHelp���̂ɂ���P����
// Jul. 10, 2001 JEPRO �ǉ�
void CType_Rich::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("���b�`�e�L�X�g"));
	_tcscpy(type.szTypeExts, _T("rtf"));

	// �ݒ�
	type.eDefaultOutline = OutlineType::Text;				// �A�E�g���C����͕��@
	type.nKeywordSetIdx[0]  = 15;							// �L�[���[�h�Z�b�g
	type.colorInfoArr[COLORIDX_DIGIT].bDisp = true;		// ���p���l��F�����\��
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	type.colorInfoArr[COLORIDX_URL].bDisp = false;		// URL�ɃA���_�[���C���������Ȃ�
}


// Jul. 10, 2001 JEPRO �ǉ�
const wchar_t* g_ppszKeywordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i",
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};
int g_nKeywordsRTF = _countof(g_ppszKeywordsRTF);

