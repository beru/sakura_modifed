#pragma once

#include "parse/WordParse.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// SJIS�̃R�[�h�y�[�W(CP_ACP �ł͖���������g���΂����炭�p���Win�ł������͂��B)
#define CP_SJIS		932


// �萔�̑f (���ڎg�p�͍T���Ă�������)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"

// ANSI�萔
namespace ACODE {
	// ����
	static const char TAB   = TAB_;
	static const char SPACE = SPACE_;
	static const char CR	= CR_;
	static const char LF	= LF_;
	static const char ESC	= ESC_;

	// ������
	static const char CRLF[] = CRLF_;

	// ���� (BREGEXP)
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFF;
}

// UNICODE�萔
namespace WCODE {
	// ����
	static const wchar_t TAB   = LCHAR(TAB_);
	static const wchar_t SPACE = LCHAR(SPACE_);
	static const wchar_t CR    = LCHAR(CR_);
	static const wchar_t LF    = LCHAR(LF_);
	static const wchar_t ESC   = LCHAR(ESC_);

	// ������
	static const wchar_t CRLF[] = LTEXT(CRLF_);

	// ���� (BREGEXP)
	//$$ UNICODE�ł̉��f���~�^�Bbregonig�̎d�l���悭�킩��Ȃ��̂ŁA�Ƃ肠��������Ȓl�ɂ��Ă܂��B
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;

}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �L�[���[�h�L�����N�^
extern const unsigned char g_keyword_char[128];

inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	return (1
		&& 0 <= wc
		&& wc < _countof(g_keyword_char)
		&& (g_keyword_char[wc] == CK_CSYM||g_keyword_char[wc] == CK_UDEF)
	);
}


// UNICODE����֐��Q
namespace WCODE {
	inline bool IsAZ(wchar_t wc) {
		return (wc >= L'A' && wc <= L'Z') || (wc >= L'a' && wc <= L'z');
	}
	inline bool Is09(wchar_t wc) {
		return (wc >= L'0' && wc <= L'9');
	}
	inline bool IsInRange(wchar_t c, wchar_t front, wchar_t back) {
		return c >= front && c <= back;
	}

	bool CalcHankakuByFont(wchar_t c);

	// ���p����(�c�������`)���ǂ�������
	inline bool IsHankaku(wchar_t wc)
	{
		// ���قږ����؁B

		// �Q�l�Fhttp://www.swanq.co.jp/blog/archives/000783.html
		if (
			   wc <= 0x007E // ACODE�Ƃ�
//			|| wc == 0x00A5 // �~�}�[�N
//			|| wc == 0x203E // �ɂ��
			|| (wc >= 0xFF61 && wc <= 0xFF9f)	// ���p�J�^�J�i
		) return true;

		// 0x7F �` 0xA0 �����p�Ƃ݂Ȃ�
		// http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF �����āA�Ȃ�ƂȂ�
		if (wc >= 0x007F && wc <= 0x00A0) return true;	// Control Code ISO/IEC 6429

		// �����͂��ׂē��ꕝ�Ƃ݂Ȃ�
		if (wc >= 0x4E00 && wc <= 0x9FBB		// Unified Ideographs, CJK
		  || wc>= 0x3400 && wc <= 0x4DB5		// Unified Ideographs Extension A, CJK
		) {
			wc = 0x4E00; // '��'(0x4E00)�̕��ő�p
		}
		else
		// �n���O���͂��ׂē��ꕝ�Ƃ݂Ȃ�
		if (wc >= 0xAC00 && wc <= 0xD7A3) {		// Hangul Syllables
			wc = 0xAC00; // (0xAC00)�̕��ő�p
		}
		else
		// �O���͂��ׂē��ꕝ�Ƃ݂Ȃ�
		if (wc >= 0xE000 && wc <= 0xE8FF) { // Private Use Area
			wc = 0xE000; // (0xE000)�̕��ő�p
		}

		//$$ ���B�������I�Ɍv�Z�����Ⴆ�B(����̂�)
		return CalcHankakuByFont(wc);
	}

	// ���䕶���ł��邩�ǂ���
	inline bool IsControlCode(wchar_t wc)
	{
		//// ���s�͐��䕶���Ƃ݂Ȃ��Ȃ�
		//if (IsLineDelimiter(wc)) return false;

		//// �^�u�͐��䕶���Ƃ݂Ȃ��Ȃ�
		//if (wc == TAB) return false;

		//return iswcntrl(wc) != 0;
		return (wc < 128 && g_keyword_char[wc] == CK_CTRL);
	}

	// �S�p����(�����`)���ǂ�������
	inline bool IsZenkaku(wchar_t wc) {
		return !IsHankaku(wc);
	}

	// �S�p�X�y�[�X���ǂ�������
	inline bool IsZenkakuSpace(wchar_t wc) {
		return wc == 0x3000; // L'�@'
	}

	// ���s�����ł��邩�ǂ���
	inline bool IsLineDelimiter(wchar_t wc, bool ext) {
		return wc == CR || wc == LF || wc == 0x85 || wc == 0x2028 || wc == 0x2029;
	}
	inline bool IsLineDelimiterBasic(wchar_t wc) {
		return wc==CR || wc==LF;
	}
	inline bool IsLineDelimiterExt(wchar_t wc) {
		return wc==CR || wc==LF || wc==0x85 || wc==0x2028 || wc==0x2029;
	}

	// �P��̋�؂蕶���ł��邩�ǂ���
	inline bool IsWordDelimiter(wchar_t wc) {
		return wc == SPACE || wc == TAB || IsZenkakuSpace(wc);
	}

	// �C���f���g�\���v�f�ł��邩�ǂ����BbAcceptZenSpace: �S�p�X�y�[�X���܂߂邩�ǂ���
	inline bool IsIndentChar(wchar_t wc, bool bAcceptZenSpace) {
		if (wc == TAB || wc == SPACE) return true;
		if (bAcceptZenSpace && IsZenkakuSpace(wc)) return true;
		return false;
	}

	// �󔒂��ǂ���
	inline bool IsBlank(wchar_t wc) {
		return wc == TAB || wc == SPACE || IsZenkakuSpace(wc);
	}

	// �t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool IsValidFilenameChar(const wchar_t* pData, size_t nIndex) {
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
		return (wcschr(table, wc) == NULL); // table���̕������܂܂�Ă�����A�_���B
	}

	// �^�u�\���Ɏg���镶�����ǂ���
	inline bool IsTabAvailableCode(wchar_t wc) {
		//$$�v����
		if (wc == L'\0') return false;
		if (wc == L'\r') return false;
		if (wc == L'\n') return false;
		if (wc == L'\t') return false;
		return true;
	}

	// ���p�J�i���ǂ���
	inline bool IsHankakuKatakana(wchar_t c) {
		// �Q�l: http://ash.jp/code/unitbl1.htm
		return c >= 0xFF61 && c <= 0xFF9F;
	}

	// �S�p�L�����ǂ���
	inline bool IsZenkakuKigou(wchar_t c) {
		//$ ���ɂ��S�p�L���͂���Ǝv�����ǁA�Ƃ肠����ANSI�Ŏ���̔���𓥏P�B�p�t�H�[�}���X�����B
		// �u�T�U�i�Ђ炪�ȁj�v�u�R�S�i�J�^�J�i�j�v�u�J�K�i�S�p���_�j�v�u�W�X�Z�i�����j�v�u�[�i�����j�v�����O
		// ANSI�ł̏C���ɂ��킹�āu�Y�v���L���������ɂ���
		static const wchar_t* table = L"�@�A�B�C�D�E�F�G�H�I�L�M�N�O�P�Q�V�\�]�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�����������������������������������������������������������������������������������������������������������ȁɁʁˁ́́΁ځہ܁݁ށ߁�����������������������";
		return wcschr(table, c) != NULL;
	}

	// �Ђ炪�Ȃ��ǂ���
	inline bool IsHiragana(wchar_t c) {
		// �u�T�U�v��ǉ�
		return (c >= 0x3041 && c <= 0x3096) || (c >= 0x309D && c <= 0x309E);
	}

	// �J�^�J�i���ǂ���
	inline bool IsZenkakuKatakana(wchar_t c) {
		// �u�R�S�v��ǉ�
		return (c >= 0x30A1 && c <= 0x30FA) || (c >= 0x30FD && c <= 0x30FE);
	}

	// �M���V���������ǂ���
	inline bool IsGreek(wchar_t c) {
		return c >= 0x0391 && c <= 0x03C9;
	}

	// �L�����������ǂ���
	inline bool IsCyrillic(wchar_t c) {
		return c >= 0x0410 && c <= 0x044F;
	}

	// BOX DRAWING ���� ���ǂ���
	inline bool IsBoxDrawing(wchar_t c) {
		return c >= 0x2500 && c <= 0x257F;
	}

	// ��Ǔ_��
	//bool IsKutoten(wchar_t wc);

}


// ANSI����֐��Q
namespace ACODE
{
	inline bool IsAZ(char c) {
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	}

	// ���䕶���ł��邩�ǂ���
	inline bool IsControlCode(char c) {
		unsigned char n = (unsigned char)c;
		if (c == TAB) return false;
		if (c == CR) return false;
		if (c == LF) return false;
		if (n <= 0x1F) return true;
		if (n >= 0x7F && n <= 0x9F) return true;
		if (n >= 0xE0) return true;
		return false;
	}

	// �^�u�\���Ɏg���镶�����ǂ���
	inline bool IsTabAvailableCode(char c) {
		if (c == '\0') return false;
		if (c <= 0x1f) return false;
		if (c >= 0x7f) return false;
		return true;
	}

	// �t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool IsValidFilenameChar(const char* pData, size_t nIndex) {
		static const char* table = "<>?\"|*";
		char c = pData[nIndex];
		// table���̕������܂܂�Ă���
		return (strchr(table, c) == NULL);
	}
}

// TCHAR����֐��Q
namespace TCODE {
	using namespace WCODE;
}

// �������̓��I�v�Z�p�L���b�V���֘A
struct CharWidthCache {
	// �������p�S�p�L���b�V��
	TCHAR		lfFaceName[LF_FACESIZE];
	BYTE		bCharWidthCache[0x10000/4];		// 16KB �������p�S�p�L���b�V��
	int			nCharWidthCacheTest;				// cache��ꌟ�o
};

enum class CharWidthFontMode {
	Edit,
	Print,
	MiniMap,
	Max,
};

enum class CharWidthCacheMode {
	Neutral,
	Share,
	Local,
};

// �L���b�V���̏������֐��Q
void SelectCharWidthCache(CharWidthFontMode fMode, CharWidthCacheMode cMode);  // ���[�h��ύX�������Ƃ�
void InitCharWidthCache(const LOGFONT& lf, CharWidthFontMode fMode = CharWidthFontMode::Edit); // �t�H���g��ύX�����Ƃ�

