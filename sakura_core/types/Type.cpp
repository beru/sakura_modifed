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
#include "Type.h"
#include "view/Colors/EColorIndexType.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"

void _DefaultConfig(TypeConfig* pType);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          Type                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void Type::InitTypeConfig(int nIdx, TypeConfig& type)
{
	// �K��l���R�s�[
	static TypeConfig sDefault;
	static bool bLoadedDefault = false;
	if (!bLoadedDefault) {
		_DefaultConfig(&sDefault);
		bLoadedDefault=true;
	}
	type = sDefault;
	
	// �C���f�b�N�X��ݒ�
	type.nIdx = nIdx;
	type.id = nIdx;
	
	// �ʐݒ�
	InitTypeConfigImp(type);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief ���L������������/�^�C�v�ʐݒ�

	�^�C�v�ʐݒ�̏���������

	@date 2005.01.30 genta ShareData::Init()���番���D
*/
void ShareData::InitTypeConfigs(
	DllSharedData* pShareData,
	std::vector<TypeConfig*>& types
	)
{
	Type* table[] = {
		new CType_Basis(),	// ��{
		new CType_Text(),	// �e�L�X�g
		new CType_Cpp(),	// C/C++
		new CType_Html(),	// HTML
		new CType_Sql(),	// PL/SQL
		new CType_Cobol(),	// COBOL
		new CType_Java(),	// Java
		new CType_Asm(),	// �A�Z���u��
		new CType_Awk(),	// awk
		new CType_Dos(),	// MS-DOS�o�b�`�t�@�C��
		new CType_Pascal(),	// Pascal
		new CType_Tex(),	// TeX
		new CType_Perl(),	// Perl
		new CType_Vb(),		// Visual Basic
		new CType_Rich(),	// ���b�`�e�L�X�g
		new CType_Ini(),	// �ݒ�t�@�C��
	};
	types.clear();
	assert(_countof(table) <= MAX_TYPES);
	for (int i=0; i<_countof(table) && i<MAX_TYPES; ++i) {
		TypeConfig* type = new TypeConfig;
		types.push_back(type);
		table[i]->InitTypeConfig(i, *type);
		auto& typeMini = pShareData->typesMini[i];
		auto_strcpy(typeMini.szTypeExts, type->szTypeExts);
		auto_strcpy(typeMini.szTypeName, type->szTypeName);
		typeMini.encoding = type->encoding;
		typeMini.id = type->id;
		SAFE_DELETE(table[i]);
	}
	pShareData->typeBasis = *types[0];
	pShareData->nTypesCount = (int)types.size();
}

/*!	@brief ���L������������/�����L�[���[�h

	�����L�[���[�h�֘A�̏���������

	@date 2005.01.30 genta ShareData::Init()���番���D
		�L�[���[�h��`���֐��̊O�ɏo���C�o�^���}�N�������ĊȌ��ɁD
*/
void ShareData::InitKeyword(DllSharedData* pShareData)
{
	// �����L�[���[�h�̃e�X�g�f�[�^
	pShareData->common.specialKeyword.keywordSetMgr.m_nCurrentKeywordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name, case_sensitive, aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	pShareData->common.specialKeyword.keywordSetMgr.AddKeywordSet((name), (case_sensitive));	\
	pShareData->common.specialKeyword.keywordSetMgr.SetKeywordArr(++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname);
	
	PopulateKeyword(L"C/C++",			true,	CPP);			// �Z�b�g 0�̒ǉ�
	PopulateKeyword(L"HTML",			false,	HTML);			// �Z�b�g 1�̒ǉ�
	PopulateKeyword(L"PL/SQL",			false,	PLSQL);			// �Z�b�g 2�̒ǉ�
	PopulateKeyword(L"COBOL",			true,	COBOL);			// �Z�b�g 3�̒ǉ�
	PopulateKeyword(L"Java",			true,	JAVA);			// �Z�b�g 4�̒ǉ�
	PopulateKeyword(L"CORBA IDL",		true,	CORBA_IDL);		// �Z�b�g 5�̒ǉ�
	PopulateKeyword(L"AWK"	,			true,	AWK);			// �Z�b�g 6�̒ǉ�
	PopulateKeyword(L"MS-DOS batch",	false,	BAT);			// �Z�b�g 7�̒ǉ�	// Oct. 31, 2000 JEPRO '�o�b�`�t�@�C��'��'batch' �ɒZ�k
	PopulateKeyword(L"Pascal",			false,	PASCAL);		// �Z�b�g 8�̒ǉ�	// Nov. 5, 2000 JEPRO ��E�������̋�ʂ�'���Ȃ�'�ɕύX
	PopulateKeyword(L"TeX",				true,	TEX);			// �Z�b�g 9�̒ǉ�	// Sept. 2, 2000 jepro Tex ��TeX �ɏC�� Bool�l�͑�E�������̋��
	PopulateKeyword(L"TeX2",			true,	TEX2);			// �Z�b�g10�̒ǉ�	// Jan. 19, 2001 JEPRO �ǉ�
	PopulateKeyword(L"Perl",			true,	PERL);			// �Z�b�g11�̒ǉ�
	PopulateKeyword(L"Perl2",			true,	PERL2);			// �Z�b�g12�̒ǉ�	// Jul. 10, 2001 JEPRO Perl����ϐ��𕪗��E�Ɨ�
	PopulateKeyword(L"Visual Basic",	false,	VB);			// �Z�b�g13�̒ǉ�	// Jul. 10, 2001 JEPRO
	PopulateKeyword(L"Visual Basic2",	false,	VB2);			// �Z�b�g14�̒ǉ�	// Jul. 10, 2001 JEPRO
	PopulateKeyword(L"Rich Text",		true,	RTF);			// �Z�b�g15�̒ǉ�	// Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �f�t�H���g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DefaultConfig(TypeConfig* pType)
{
// �L�[���[�h�F�f�t�H���g�J���[�ݒ�
/************************/
/* �^�C�v�ʐݒ�̋K��l */
/************************/

	pType->nTextWrapMethod = TextWrappingMethod::SettingWidth;		// �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	pType->nMaxLineKetas = LayoutInt(MAXLINEKETAS);	// �܂�Ԃ�����
	pType->nColumnSpace = 0;							// �����ƕ����̌���
	pType->nLineSpace = 1;							// �s�Ԃ̂�����
	pType->nTabSpace = LayoutInt(4);					// TAB�̕�����
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		pType->nKeywordSetIdx[i] = -1;
	}
	wcscpy_s(pType->szTabViewString, _EDITL("^       "));	// TAB�\��������
	pType->bTabArrow = TabArrowType::String;	// �^�u���\��	// 2001.12.03 hor	// default on 2013/4/11 Uchi
	pType->bInsSpace = false;				// �X�y�[�X�̑}��	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI �ȉ��AlineComment��blockComments���g���悤�ɏC��
	pType->lineComment.CopyTo(0, L"", -1);	// �s�R�����g�f���~�^
	pType->lineComment.CopyTo(1, L"", -1);	// �s�R�����g�f���~�^2
	pType->lineComment.CopyTo(2, L"", -1);	// �s�R�����g�f���~�^3	//Jun. 01, 2001 JEPRO �ǉ�
	pType->blockComments[0].SetBlockCommentRule(L"", L"");	// �u���b�N�R�����g�f���~�^
	pType->blockComments[1].SetBlockCommentRule(L"", L"");	// �u���b�N�R�����g�f���~�^2

	pType->stringType = StringLiteralType::CPP;					// �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""]['']
	pType->bStringLineOnly = false;
	pType->bStringEndLine  = false;
	pType->nHeredocType = HereDocType::PHP;
	pType->szIndentChars[0] = 0;		// ���̑��̃C���f���g�Ώە���

	pType->nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	pType->szHokanFile[0] = 0;		// ���͕⊮ �P��t�@�C��
	// 2001/06/14 End

	pType->nHokanType = 0;

	// 2001/06/19 asa-o
	pType->bHokanLoHiCase = false;			// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����

	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	pType->bUseHokanByFile = true;			//! ���͕⊮ �J���Ă���t�@�C�����������T��
	pType->bUseHokanByKeyword = false;		// �����L�[���[�h������͕⊮

	// �����R�[�h�ݒ�
	auto& encoding = pType->encoding;
	encoding.bPriorCesu8 = false;
	encoding.eDefaultCodetype = CODE_SJIS;
	encoding.eDefaultEoltype = EolType::CRLF;
	encoding.bDefaultBom = false;

	//@@@2002.2.4 YAZAKI
	pType->szExtHelp[0] = L'\0';
	pType->szExtHtmlHelp[0] = L'\0';
	pType->bHtmlHelpIsSingle = true;

	pType->bAutoIndent = true;			// �I�[�g�C���f���g
	pType->bAutoIndent_ZENSPACE = true;	// ���{��󔒂��C���f���g
	pType->bRTrimPrevLine = false;		// 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜

	pType->nIndentLayout = 0;	// �܂�Ԃ���2�s�ڈȍ~���������\��

	assert(COLORIDX_LAST <= _countof(pType->colorInfoArr));
	for (int i=0; i<COLORIDX_LAST; ++i) {
		GetDefaultColorInfo(&pType->colorInfoArr[i], i);
	}
	pType->szBackImgPath[0] = '\0';
	pType->backImgPos = BackgroundImagePosType::TopLeft;
	pType->backImgRepeatX = true;
	pType->backImgRepeatY = true;
	pType->backImgScrollX = true;
	pType->backImgScrollY = true;
	{
		POINT pt ={0, 0};
		pType->backImgPosOffset = pt;
	}
	pType->bLineNumIsCRLF = true;					// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	pType->nLineTermType = 1;						// �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C��
	pType->cLineTermChar = L':';					// �s�ԍ���؂蕶��
	pType->bWordWrap = false;						// �p�����[�h���b�v������
	pType->nCurrentPrintSetting = 0;				// ���ݑI�����Ă������ݒ�
	pType->bOutlineDockDisp = false;				// �A�E�g���C����͕\���̗L��
	pType->eOutlineDockSide = DockSideType::Float;	// �A�E�g���C����̓h�b�L���O�z�u
	pType->cxOutlineDockLeft = 0;					// �A�E�g���C���̍��h�b�L���O��
	pType->cyOutlineDockTop = 0;					// �A�E�g���C���̏�h�b�L���O��
	pType->cxOutlineDockRight = 0;					// �A�E�g���C���̉E�h�b�L���O��
	pType->cyOutlineDockBottom = 0;					// �A�E�g���C���̉��h�b�L���O��
	pType->eDefaultOutline = OutlineType::Text;		// �A�E�g���C����͕��@
	pType->nOutlineSortCol = 0;						// �A�E�g���C����̓\�[�g��ԍ�
	pType->bOutlineSortDesc = false;				// �A�E�g���C����̓\�[�g�~��
	pType->nOutlineSortType = 0;					// �A�E�g���C����̓\�[�g�
	pType->eSmartIndent = SmartIndentType::None;		// �X�}�[�g�C���f���g���
	pType->nImeState = IME_CMODE_NOCONVERSION;	// IME����

	pType->szOutlineRuleFilename[0] = L'\0';		//Dec. 4, 2000 MIK
	pType->bKinsokuHead = false;					// �s���֑�				//@@@ 2002.04.08 MIK
	pType->bKinsokuTail = false;					// �s���֑�				//@@@ 2002.04.08 MIK
	pType->bKinsokuRet  = false;					// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	pType->bKinsokuKuto = false;					// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	pType->szKinsokuHead[0] = 0;					// �s���֑�				//@@@ 2002.04.08 MIK
	pType->szKinsokuTail[0] = 0;					// �s���֑�				//@@@ 2002.04.08 MIK
	wcscpy_s(pType->szKinsokuKuto, L"�A�B�C�D��,.");	// ��Ǔ_�Ԃ牺������	// 2009.08.07 ryoji

	pType->bUseDocumentIcon = false;				// �����Ɋ֘A�Â���ꂽ�A�C�R�����g��

//@@@ 2001.11.17 add start MIK
	for (int i=0; i<_countof(pType->regexKeywordArr); ++i) {
		pType->regexKeywordArr[i].nColorIndex = COLORIDX_REGEX1;
	}
	pType->regexKeywordList[0] = L'\0';
	pType->bUseRegexKeyword = false;
//@@@ 2001.11.17 add end MIK
	pType->nRegexKeyMagicNumber = 0;

//@@@ 2006.04.10 fon ADD-start
	for (int i=0; i<MAX_KEYHELP_FILE; ++i) {
		auto& attr = pType->keyHelpArr[i];
		attr.bUse = false;
		attr.szAbout[0] = _T('\0');
		attr.szPath[0] = _T('\0');
	}
	pType->bUseKeywordHelp = false;		// �����I���@�\�̎g�p��
	pType->nKeyHelpNum = 0;				// �o�^������
	pType->bUseKeyHelpAllSearch = false;	// �q�b�g�������̎���������(&A)
	pType->bUseKeyHelpKeyDisp = false;	// 1�s�ڂɃL�[���[�h���\������(&W)
	pType->bUseKeyHelpPrefix = false;		// �I��͈͂őO����v����(&P)
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca �w��ʒu�c���̐ݒ�
	for (int i=0; i<MAX_VERTLINES; ++i) {
		pType->nVertLineIdx[i] = LayoutInt(0);
	}
	pType->nNoteLineOffset = 0;

	//  �ۑ����ɉ��s�R�[�h�̍��݂��x������	2013/4/14 Uchi
	pType->bChkEnterAtEnd = true;

	pType->bUseTypeFont = false;				// �^�C�v�ʃt�H���g�̎g�p

	pType->nLineNumWidth = LINENUMWIDTH_MIN;	// �s�ԍ��ŏ����� 2014.08.02 katze
}

