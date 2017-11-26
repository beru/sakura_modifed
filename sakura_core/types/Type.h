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
	Perl,				// 
	VisualBasic,
	WZText,				// �K�w�t�e�L�X�g�A�E�g���C�����
	HTML,				// HTML�A�E�g���C�����
	TeX,				// TeX�A�E�g���C�����
	RuleFile,			// ���[���t�@�C���p
	Python,				// Python�A�E�g���C�����
	Erlang,				// Erlang�A�E�g���C�����
	//	�V�����A�E�g���C����͕͂K�����̒��O�֑}��
	CodeMax,
	BookMark,			// 
	PlugIn,				// �v���O�C���ɂ��A�E�g���C�����
	FileTree,			// �t�@�C���c���[
	Default = -1,		// 
	UnknownOutlineType	= 99,
	Tree = 100,			// �ėp�c���[
	TreeTagJump = 101,	// �ėp�c���[(�^�O�W�����v�t��)
	ClassTree = 200,	// �ėp�c���[(�N���X)
	List = 300,			// �ėp���X�g
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
	EolType			eDefaultEoltype;				// �f�t�H���g���s�R�[�h
	bool			bDefaultBom;					// �f�t�H���gBOM
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
	size_t				nIdx;
	int					id;
	TCHAR				szTypeName[64];				// �^�C�v�����F����
	TCHAR				szTypeExts[MAX_TYPES_EXTS];	// �^�C�v�����F�g���q���X�g
	TextWrappingMethod	nTextWrapMethod;			// �e�L�X�g�̐܂�Ԃ����@
	size_t				nMaxLineKetas;				// �܂�Ԃ�����
	size_t				nColumnSpace;				// �����ƕ����̌���
	int					nLineSpace;					// �s�Ԃ̂�����
	size_t				nTabSpace;					// TAB�̕�����
	TabArrowType		bTabArrow;					// �^�u���\��
	EDIT_CHAR			szTabViewString[8 + 1];		// TAB�\��������
	bool				bInsSpace;					// �X�y�[�X�̑}��
	int					nKeywordSetIdx[MAX_KEYWORDSET_PER_TYPE];	// �L�[���[�h�Z�b�g

	LineComment			lineComment;					// �s�R�����g�f���~�^
	BlockComment		blockComments[2];				// �u���b�N�R�����g�f���~�^

	StringLiteralType	stringType;						// �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	bool				bStringLineOnly;				// ������͍s���̂�
	bool				bStringEndLine;					// (�I�������񂪂Ȃ��ꍇ)�s���܂ŐF����
	HereDocType			nHeredocType;
	wchar_t				szIndentChars[64];				// ���̑��̃C���f���g�Ώە���

	size_t				nColorInfoArrNum;				// �F�ݒ�z��̗L����
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
	int					nVertLineIdx[MAX_VERTLINES];	// �w�茅�c��
	int 				nNoteLineOffset;				// �m�[�g���̃I�t�Z�b�g

	bool				bWordWrap;						// �p�����[�h���b�v������
	bool				bKinsokuHead;					// �s���֑�������
	bool				bKinsokuTail;					// �s���֑�������
	bool				bKinsokuRet;					// ���s�����̂Ԃ牺��
	bool				bKinsokuKuto;					// ��Ǔ_�̂Ԃ炳��
	bool				bKinsokuHide;					// �Ԃ牺�����B��
	wchar_t				szKinsokuHead[200];				// �s���֑�����
	wchar_t				szKinsokuTail[200];				// �s���֑�����
	wchar_t				szKinsokuKuto[200];				// ��Ǔ_�Ԃ炳������

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
	int					nImeState;						// ����IME���

	// �⊮�̃^�C�v�ʐݒ�
	SFilePath			szHokanFile;					// ���͕⊮ �P��t�@�C��
	int					nHokanType;						// ���͕⊮ ���(�v���O�C��)
	// �t�@�C��������̓��͕⊮�@�\
	bool				bUseHokanByFile;				// ���͕⊮ �J���Ă���t�@�C�����������T��
	bool				bUseHokanByKeyword;				// �����L�[���[�h������͕⊮
	
	bool				bHokanLoHiCase;					// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����

	SFilePath			szExtHelp;						// �O���w���v�P
	SFilePath			szExtHtmlHelp;					// �O��HTML�w���v
	bool				bHtmlHelpIsSingle;				// HtmlHelp�r���[�A�͂ЂƂ�

	bool				bChkEnterAtEnd;					// �ۑ����ɉ��s�R�[�h�̍��݂��x������

	EncodingConfig		encoding;						// �G���R�[�h�I�v�V����


	bool				bUseRegexKeyword;								// ���K�\���L�[���[�h���g����
	DWORD				nRegexKeyMagicNumber;							// ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[
	RegexKeywordInfo	regexKeywordArr[MAX_REGEX_KEYWORD];				// ���K�\���L�[���[�h
	wchar_t				regexKeywordList[MAX_REGEX_KEYWORDLISTLEN];		// ���K�\���L�[���[�h

	bool				bUseKeywordHelp;				// �L�[���[�h�����Z���N�g�@�\���g����
	int					nKeyHelpNum;					// �L�[���[�h�����̍���
	KeyHelpInfo			keyHelpArr[MAX_KEYHELP_FILE];	// �L�[���[�h�����t�@�C��
	bool				bUseKeyHelpAllSearch;			// �q�b�g�������̎���������(&A)
	bool				bUseKeyHelpKeyDisp;				// 1�s�ڂɃL�[���[�h���\������(&W)
	bool				bUseKeyHelpPrefix;				// �I��͈͂őO����v����(&P)

	bool				bAutoIndent;					// �I�[�g�C���f���g
	bool				bAutoIndent_ZENSPACE;			// ���{��󔒂��C���f���g
	bool				bRTrimPrevLine;					// ���s���ɖ����̋󔒂��폜
	int					nIndentLayout;					// �܂�Ԃ���2�s�ڈȍ~���������\��

	bool				bUseDocumentIcon;				// �t�@�C���Ɋ֘A�Â���ꂽ�A�C�R�����g��

	bool				bUseTypeFont;					// �^�C�v�ʃt�H���g�̎g�p
	LOGFONT				lf;								// �t�H���g
	INT					nPointSize;						// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj

	TypeConfig()
		:
		nMaxLineKetas(10) //	��ʐ܂�Ԃ�����TAB���ȉ��ɂȂ�Ȃ����Ƃ������l�ł��ۏ؂���
	{
	}

	int			nLineNumWidth;					// �s�ԍ��̍ŏ�����
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
	void InitTypeConfig(size_t nIdx, TypeConfig&);
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
bool C_IsSpace(wchar_t c, bool bExtEol)
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::IsLineDelimiter(c, bExtEol)
	);
}

