#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�ݒ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �t�H���g����
struct FontAttr {
	bool		bBoldFont;		// ����
	bool		bUnderLine;		// ����
};

// �F����
struct ColorAttr {
	COLORREF	cTEXT;			// �����F
	COLORREF	cBACK;			// �w�i�F
};

// �F�ݒ�
struct ColorInfoBase {
	bool		bDisp;			// �\��
	FontAttr	fontAttr;		// �t�H���g����
	ColorAttr	colorAttr;		// �F����
};

// ���O�ƃC���f�b�N�X�t���F�ݒ�
struct ColorInfo : public ColorInfoBase {
	int			nColorIdx;		// �C���f�b�N�X
	TCHAR		szName[64];		// ���O
};


// �f�t�H���g�F�ݒ�
void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex);
void GetDefaultColorInfoName(ColorInfo* pColorInfo, int nIndex);
int GetDefaultColorInfoCount();


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
const int DICT_ABOUT_LEN = 50; // �����̐����̍ő咷 -1
struct KeyHelpInfo {
	bool		bUse;						// ������ �g�p����/���Ȃ�
	TCHAR		szAbout[DICT_ABOUT_LEN];	// �����̐���(�����t�@�C����1�s�ڂ��琶��)
	SFilePath	szPath;					// �t�@�C���p�X
};

