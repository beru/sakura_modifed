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

#include "Eol.h"
#include "env/CommonSetting.h"
#include "doc/DocTypeSetting.h"
#include "doc/LineComment.h"
#include "doc/BlockComment.h"
#include "charset/charset.h"	// EncodingType
#include "RegexKeyword.h"		// RegexKeywordInfo
#include "env/CommonSetting.h"

enum class DockSideType;

// �^�u�\�����@
enum class TabArrowType {
	String,		// �����w��
	Short,		// �Z�����
	Long,		// �������
};

// �A�E�g���C����͂̎��
enum class OutlineType {
	C,
	CPP,
	PLSQL,
	Text,
	Java,
	Cobol,
	Asm,
	Perl,				// Sep. 8, 2000 genta
	VisualBasic,		// June 23, 2001 N.Nakatani
	WZText,				// 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
	HTML,				// 2003.05.20 zenryaku HTML�A�E�g���C�����
	TeX,				// 2003.07.20 naoh TeX�A�E�g���C�����
	RuleFile,			// 2002.04.01 YAZAKI ���[���t�@�C���p
	Python,				// 2007.02.08 genta Python�A�E�g���C�����
	Erlang,				// 2009.08.10 genta Erlang�A�E�g���C�����
	//	�V�����A�E�g���C����͕͂K�����̒��O�֑}��
	CodeMax,
	BookMark,			// 2001.12.03 hor
	PlugIn,				// 2009.10.29 syat �v���O�C���ɂ��A�E�g���C�����
	FileTree,			//	2012.06.20 Moca �t�@�C���c���[
	Default = -1,		// 2001.12.03 hor
	UnknownOutlineType	= 99,
	Tree = 100,			// �ėp�c���[ 2010.03.28 syat
	TreeTagJump = 101,	// �ėp�c���[(�^�O�W�����v�t��) 2013.05.01 Moca
	ClassTree = 200,	// �ėp�c���[(�N���X) 2010.03.28 syat
	List = 300,			// �ėp���X�g 2010.03.28 syat
};

// �X�}�[�g�C���f���g���
enum class SmartIndentType {
	None,		// �Ȃ�
	Cpp			// C/C++
};

// �q�A�h�L�������g���
enum class HereDocType {
	PHP,			// PHP
	Ruby,			// Ruby
	Perl			// Perl
};

// �w�i�摜�\���ʒu
enum class BackgroundImagePosType {
	TopLeft,		// ����
	TopRight,		// �E��
	BottomLeft,		// ����
	BottomRight,	// �E��
	Center,			// ����
	TopCenter,		// ������
	BottomCenter,	// ������
	CenterLeft,		// ������
	CenterRight	,	// �����E
};

// �G���R�[�h�I�v�V����
struct EncodingConfig {
	bool			bPriorCesu8;					// �������ʎ��� CESU-8 ��D�悷�邩�ǂ���
	EncodingType	eDefaultCodetype;				// �f�t�H���g�����R�[�h
	EolType			eDefaultEoltype;				// �f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	bool			bDefaultBom;					// �f�t�H���gBOM			// 2011.01.24 ryoji
};

// �������؂�L���G�X�P�[�v���@
enum class StringLiteralType {
	CPP,	// C/C++���ꕗ
	PLSQL,	// PL/SQL��
	HTML,	// HTML/XML��
	CSharp,	// C#��
	Python,	// Python��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �^�C�v�ʐݒ�                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �^�C�v�ʐݒ�
struct TypeConfig {
	// 2007.09.07 �ϐ����ύX: m_nMaxLineSize��nMaxLineKetas
	int					nIdx;
	int					id;
	TCHAR				szTypeName[64];				// �^�C�v�����F����
	TCHAR				szTypeExts[MAX_TYPES_EXTS];	// �^�C�v�����F�g���q���X�g
	TextWrappingMethod	nTextWrapMethod;			// �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	LayoutInt			nMaxLineKetas;				// �܂�Ԃ�����
	int					nColumnSpace;				// �����ƕ����̌���
	int					nLineSpace;					// �s�Ԃ̂�����
	LayoutInt			nTabSpace;					// TAB�̕�����
	TabArrowType		bTabArrow;					// �^�u���\��		//@@@ 2003.03.26 MIK
	EDIT_CHAR			szTabViewString[8 + 1];		// TAB�\��������	// 2003.1.26 aroka �T�C�Y�g��	// 2009.02.11 ryoji �T�C�Y�߂�(17->8+1)
	bool				bInsSpace;					// �X�y�[�X�̑}��	// 2001.12.03 hor
	// 2005.01.13 MIK �z��
	int					nKeywordSetIdx[MAX_KEYWORDSET_PER_TYPE];	// �L�[���[�h�Z�b�g

	LineComment			lineComment;					// �s�R�����g�f���~�^				//@@@ 2002.09.22 YAZAKI
	BlockComment		blockComments[2];				// �u���b�N�R�����g�f���~�^		//@@@ 2002.09.22 YAZAKI

	StringLiteralType	stringType;						// �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	bool				bStringLineOnly;				// ������͍s���̂�
	bool				bStringEndLine;					// (�I�������񂪂Ȃ��ꍇ)�s���܂ŐF����
	HereDocType			nHeredocType;
	wchar_t				szIndentChars[64];				// ���̑��̃C���f���g�Ώە���

	int					nColorInfoArrNum;				// �F�ݒ�z��̗L����
	ColorInfo			colorInfoArr[64];				// �F�ݒ�z��

	SFilePath			szBackImgPath;					// �w�i�摜
	BackgroundImagePosType backImgPos;					// �w�i�摜�\���ʒu
	bool				backImgRepeatX;					// �w�i�摜�\���������J��Ԃ�
	bool				backImgRepeatY;					// �w�i�摜�\���c�����J��Ԃ�
	bool				backImgScrollX;					// �w�i�摜�\���������X�N���[��
	bool				backImgScrollY;					// �w�i�摜�\���c�����X�N���[��
	POINT				backImgPosOffset;				// �w�i�摜�\���I�t�Z�b�g

	bool				bLineNumIsCRLF;					// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	int					nLineTermType;					// �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C��
	wchar_t				cLineTermChar;					// �s�ԍ���؂蕶��
	LayoutInt			nVertLineIdx[MAX_VERTLINES];	// �w�茅�c��
	int 				nNoteLineOffset;				// �m�[�g���̃I�t�Z�b�g

	bool				bWordWrap;						// �p�����[�h���b�v������
	bool				bKinsokuHead;					// �s���֑�������		//@@@ 2002.04.08 MIK
	bool				bKinsokuTail;					// �s���֑�������		//@@@ 2002.04.08 MIK
	bool				bKinsokuRet;					// ���s�����̂Ԃ牺��	//@@@ 2002.04.13 MIK
	bool				bKinsokuKuto;					// ��Ǔ_�̂Ԃ炳��	//@@@ 2002.04.17 MIK
	bool				bKinsokuHide;					// �Ԃ牺�����B��		// 2011/11/30 Uchi
	wchar_t				szKinsokuHead[200];				// �s���֑�����		//@@@ 2002.04.08 MIK
	wchar_t				szKinsokuTail[200];				// �s���֑����� 		//@@@ 2002.04.08 MIK
	wchar_t				szKinsokuKuto[200];				// ��Ǔ_�Ԃ炳������	// 2009.08.07 ryoji

	int					nCurrentPrintSetting;			// ���ݑI�����Ă������ݒ�

	bool				bOutlineDockDisp;				// �A�E�g���C����͕\���̗L��
	DockSideType		eOutlineDockSide;				// �A�E�g���C����̓h�b�L���O�z�u
	int					cxOutlineDockLeft;				// �A�E�g���C���̍��h�b�L���O��
	int					cyOutlineDockTop;				// �A�E�g���C���̏�h�b�L���O��
	int					cxOutlineDockRight;				// �A�E�g���C���̉E�h�b�L���O��
	int					cyOutlineDockBottom;			// �A�E�g���C���̉��h�b�L���O��
	OutlineType			nDockOutline;					// �h�b�L���O���̃A�E�g���C��/�u�b�N�}�[�N
	OutlineType			eDefaultOutline;				// �A�E�g���C����͕��@
	SFilePath			szOutlineRuleFilename;			// �A�E�g���C����̓��[���t�@�C��
	int					nOutlineSortCol;				// �A�E�g���C����̓\�[�g��ԍ�
	bool				bOutlineSortDesc;				// �A�E�g���C����̓\�[�g�~��
	int					nOutlineSortType;				// �A�E�g���C����̓\�[�g�
	FileTree			fileTree;						// �t�@�C���c���[�ݒ�

	SmartIndentType		eSmartIndent;					// �X�}�[�g�C���f���g���
	int					nImeState;						// ����IME���	Nov. 20, 2000 genta

	//	2001/06/14 asa-o �⊮�̃^�C�v�ʐݒ�
	SFilePath			szHokanFile;					// ���͕⊮ �P��t�@�C��
	int					nHokanType;						// ���͕⊮ ���(�v���O�C��)
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	bool				bUseHokanByFile;				// ���͕⊮ �J���Ă���t�@�C�����������T��
	bool				bUseHokanByKeyword;				// �����L�[���[�h������͕⊮
	
	//	2001/06/19 asa-o
	bool				bHokanLoHiCase;					// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����

	SFilePath			szExtHelp;						// �O���w���v�P
	SFilePath			szExtHtmlHelp;					// �O��HTML�w���v
	bool				bHtmlHelpIsSingle;				// HtmlHelp�r���[�A�͂ЂƂ�

	bool				bChkEnterAtEnd;					// �ۑ����ɉ��s�R�[�h�̍��݂��x������	2013/4/14 Uchi

	EncodingConfig		encoding;						// �G���R�[�h�I�v�V����


//@@@ 2001.11.17 add start MIK
	bool				bUseRegexKeyword;								// ���K�\���L�[���[�h���g����
	DWORD				nRegexKeyMagicNumber;							// ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[
	RegexKeywordInfo	regexKeywordArr[MAX_REGEX_KEYWORD];				// ���K�\���L�[���[�h
	wchar_t				regexKeywordList[MAX_REGEX_KEYWORDLISTLEN];		// ���K�\���L�[���[�h
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				bUseKeywordHelp;				// �L�[���[�h�����Z���N�g�@�\���g����
	int					nKeyHelpNum;					// �L�[���[�h�����̍���
	KeyHelpInfo			keyHelpArr[MAX_KEYHELP_FILE];	// �L�[���[�h�����t�@�C��
	bool				bUseKeyHelpAllSearch;			// �q�b�g�������̎���������(&A)
	bool				bUseKeyHelpKeyDisp;				// 1�s�ڂɃL�[���[�h���\������(&W)
	bool				bUseKeyHelpPrefix;				// �I��͈͂őO����v����(&P)
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Common����ړ��B
	bool				bAutoIndent;					// �I�[�g�C���f���g
	bool				bAutoIndent_ZENSPACE;			// ���{��󔒂��C���f���g
	bool				bRTrimPrevLine;					// 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜
	int					nIndentLayout;					// �܂�Ԃ���2�s�ڈȍ~���������\��

	//	Sep. 10, 2002 genta
	bool				bUseDocumentIcon;				// �t�@�C���Ɋ֘A�Â���ꂽ�A�C�R�����g��

	bool				bUseTypeFont;					// �^�C�v�ʃt�H���g�̎g�p
	LOGFONT				lf;								// �t�H���g // 2013.03.18 aroka
	INT					nPointSize;						// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj

	TypeConfig()
		:
		nMaxLineKetas(10) //	��ʐ܂�Ԃ�����TAB���ȉ��ɂȂ�Ȃ����Ƃ������l�ł��ۏ؂���	//	2004.04.03 Moca
	{
	}

	int			nLineNumWidth;					// �s�ԍ��̍ŏ����� 2014.08.02 katze
}; // TypeConfig

// �^�C�v�ʐݒ�(mini)
struct TypeConfigMini {
	int			id;
	TCHAR		szTypeName[64];				// �^�C�v�����F����
	TCHAR		szTypeExts[MAX_TYPES_EXTS];	// �^�C�v�����F�g���q���X�g
	EncodingConfig		encoding;			// �G���R�[�h�I�v�V����
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �^�C�v�ʐݒ�A�N�Z�T                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �h�L�������g��ށB���L�f�[�^�� TypeConfig �ւ̃A�N�Z�T�����˂�B
// 2007.12.13 kobake �쐬
class TypeConfigNum {
public:
	TypeConfigNum()
	{
#ifdef _DEBUG
		// ����int�������̂ŁA���������Ŏg���Ɩ�肪��������悤�ɁA�����āA�ςȒl�����Ă����B
		type = 1234;
#else
		// �����[�X���́A���������ł���肪�N����ɂ����悤�ɁA�[���N���A���Ă���
		type = 0;
#endif
	}
	
	explicit TypeConfigNum(int n) {
		type = n;
	}
	bool IsValidType() const { return type >= 0 && type < MAX_TYPES; }
	int GetIndex() const { /*assert(IsValid());*/ return type; }

	// ���L�f�[�^�ւ̊ȈՃA�N�Z�T
//	TypeConfig* operator->() { return GetTypeConfig(); }
//	TypeConfig* GetTypeConfig();
private:
	int type;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �^�C�v�ݒ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class Type {
public:
	virtual ~Type() { }
	void InitTypeConfig(int nIdx, TypeConfig&);
protected:
	virtual void InitTypeConfigImp(TypeConfig& type) = 0;
};

#define GEN_CTYPE(CLASS_NAME) \
class CLASS_NAME : public Type { \
protected: \
	void InitTypeConfigImp(TypeConfig& type); \
};

GEN_CTYPE(CType_Asm)
GEN_CTYPE(CType_Awk)
GEN_CTYPE(CType_Basis)
GEN_CTYPE(CType_Cobol)
GEN_CTYPE(CType_Cpp)
GEN_CTYPE(CType_Dos)
GEN_CTYPE(CType_Html)
GEN_CTYPE(CType_Ini)
GEN_CTYPE(CType_Java)
GEN_CTYPE(CType_Pascal)
GEN_CTYPE(CType_Perl)
GEN_CTYPE(CType_Rich)
GEN_CTYPE(CType_Sql)
GEN_CTYPE(CType_Tex)
GEN_CTYPE(CType_Text)
GEN_CTYPE(CType_Vb)
GEN_CTYPE(CType_Other)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�X�y�[�X�̔���
*/
inline
bool C_IsSpace( wchar_t c, bool bExtEol )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::IsLineDelimiter(c, bExtEol)
	);
}

