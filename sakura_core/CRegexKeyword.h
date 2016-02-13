/*!	@file
	@brief RegexKeyword Library

	���K�\���L�[���[�h�������B
	BREGEXP.DLL�𗘗p����B

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#pragma once

#include "_main/global.h"
#include "extmodule/CBregexp.h"
#include "config/maxdata.h" // MAX_REGEX_KEYWORD

struct TypeConfig;

#define USE_PARENT	// �e���g���ăL�[���[�h�i�[�̈���팸����B


//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	int	m_nColorIndex;		// �F�w��ԍ�
};
//@@@ 2001.11.17 add end MIK


// ���K�\���L�[���[�h�������\����
struct RegexInfo {
	BREGEXP_W* pBregexp;	// BREGEXP_W�\����
#ifdef USE_PARENT
#else
	struct RegexKeywordInfo	sRegexKey;	// �R���p�C���p�^�[����ێ�
#endif
	int    nStatus;		// ���(EMPTY,CLOSE,OPEN,ACTIVE,ERROR)
	int    nMatch;		// ���̃L�[���[�h�̃}�b�`���(EMPTY,MATCH,NOMATCH)
	int    nOffset;		// �}�b�`�����ʒu
	int    nLength;		// �}�b�`��������
	int    nHead;		// �擪�̂݃`�F�b�N���邩�H
	int    nFlag;		// �F�w��̃`�F�b�N�������Ă��邩�H YES=RK_EMPTY, NO=RK_NOMATCH
};


// ���K�\���L�[���[�h�N���X
/*!
	���K�\���L�[���[�h�������B
*/
class RegexKeyword : public Bregexp {
public:
	RegexKeyword(LPCTSTR);
	~RegexKeyword();

	// �s�����J�n
	bool RegexKeyLineStart(void);
	// �s����
	bool RegexIsKeyword(const StringRef& str, int nPos, int* nMatchLen, int* nMatchColor);
	// �^�C�v�ݒ�
	bool RegexKeySetTypes(const TypeConfig* pTypesPtr);

	// ����(�͂�)�`�F�b�N
	static bool RegexKeyCheckSyntax(const wchar_t* s);
	
	static DWORD GetNewMagicNumber();

protected:
	// �R���p�C��
	bool RegexKeyCompile(void);
	// �ϐ�������
	bool RegexKeyInit(void);

public:
	int				m_nTypeIndex;				// ���݂̃^�C�v�ݒ�ԍ�
	bool			m_bUseRegexKeyword;			// ���K�\���L�[���[�h���g�p����E���Ȃ�

private:
	const TypeConfig*	m_pTypes;				// �^�C�v�ݒ�ւ̃|�C���^(�Ăяo�����������Ă������)
	int				m_nTypeId;					// �^�C�v�ݒ�ID
	DWORD			m_nCompiledMagicNumber;		// �R���p�C���ς݂��H
	int				m_nRegexKeyCount;			// ���݂̃L�[���[�h��
	RegexInfo		m_sInfo[MAX_REGEX_KEYWORD];	// �L�[���[�h�ꗗ(BREGEXP�R���p�C���Ώ�)
#ifdef USE_PARENT
#else
	wchar_t			m_keywordList[MAX_REGEX_KEYWORDLISTLEN];
#endif
	wchar_t			m_szMsg[256];				// BREGEXP_W����̃��b�Z�[�W��ێ�����
};

