/*!	@file
	@brief RegexKeyword Library

	���K�\���L�[���[�h�������B
	BREGEXP.DLL�𗘗p����B

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "RegexKeyword.h"
#include "extmodule/Bregexp.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

#if 0
#include <stdio.h>
#define	MYDBGMSG(s) \
{\
	FILE* fp = fopen("debug.log", "a");\
	fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex);\
	fclose(fp);\
}
#else
#define	MYDBGMSG(a)
#endif

/*
 * �p�����[�^�錾
 */
#define RK_EMPTY          0      // �������
#define RK_CLOSE          1      // BREGEXP�N���[�Y
#define RK_OPEN           2      // BREGEXP�I�[�v��
#define RK_ACTIVE         3      // �R���p�C���ς�
#define RK_ERROR          9      // �R���p�C���G���[

#define RK_MATCH          4      // �}�b�`����
#define RK_NOMATCH        5      // ���̍s�ł̓}�b�`���Ȃ�

#define RK_SIZE           100    // �ő�o�^�\��

//#define RK_HEAD_CHAR      '^'    // �s�擪�̐��K�\��
#define RK_HEAD_STR1      L"/^"   // BREGEXP
#define RK_HEAD_STR1_LEN  2
#define RK_HEAD_STR2      L"m#^"  // BREGEXP
#define RK_HEAD_STR2_LEN  3
#define RK_HEAD_STR3      L"m/^"  // BREGEXP
#define RK_HEAD_STR3_LEN  3
//#define RK_HEAD_STR4      "#^"   // BREGEXP
//#define RK_HEAD_STR4_LEN  2

#define RK_KAKOMI_1_START "/"
#define RK_KAKOMI_1_END   "/k"
#define RK_KAKOMI_2_START "m#"
#define RK_KAKOMI_2_END   "#k"
#define RK_KAKOMI_3_START "m/"
#define RK_KAKOMI_3_END   "/k"
//#define RK_KAKOMI_4_START "#"
//#define RK_KAKOMI_4_END   "#k"


// �R���X�g���N�^
/*!	@brief �R���X�g���N�^

	BREGEXP.DLL �������A���K�\���L�[���[�h���������s���B

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2007.08.12 genta ���K�\��DLL�w��̂��߈����ǉ�
*/
RegexKeyword::RegexKeyword(LPCTSTR regexp_dll )
{
	InitDll(regexp_dll);	// 2007.08.12 genta �����ǉ�
	MYDBGMSG("RegexKeyword")

	m_pTypes    = NULL;
	m_nTypeIndex = -1;
	m_nTypeId = -1;

	RegexKeyInit();
}

// �f�X�g���N�^
/*!	@brief �f�X�g���N�^

	�R���p�C���ς݃f�[�^�̔j�����s���B
*/
RegexKeyword::~RegexKeyword()
{
	MYDBGMSG("~RegexKeyword")
	// �R���p�C���ς݂̃o�b�t�@���������B
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (m_info[i].pBregexp && IsAvailable()) {
			BRegfree(m_info[i].pBregexp);
		}
		m_info[i].pBregexp = NULL;
	}
	
	RegexKeyInit();

	m_nTypeIndex = -1;
	m_pTypes     = NULL;
}

// ���K�\���L�[���[�h����������
/*!	@brief ���K�\���L�[���[�h������

	 ���K�\���L�[���[�h�Ɋւ���ϐ��ނ�����������B

	@retval TRUE ����
*/
bool RegexKeyword::RegexKeyInit(void)
{
	MYDBGMSG("RegexKeyInit")
	m_nTypeIndex = -1;
	m_nTypeId = -1;
	m_nCompiledMagicNumber = 1;
	m_bUseRegexKeyword = false;
	m_nRegexKeyCount = 0;
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		m_info[i].pBregexp = NULL;
#ifdef USE_PARENT
#else
		m_info[i].sRegexKey.m_nColorIndex = COLORIDX_REGEX1;
#endif
	}
#ifdef USE_PARENT
#else
	wmemset(m_keywordList, _countof(m_keywordList), L'\0');
#endif

	return true;
}

// ���݃^�C�v�ݒ菈��
/*!	@brief ���݃^�C�v�ݒ�

	���݂̃^�C�v�ݒ��ݒ肷��B

	@param pTypesPtr [in] �^�C�v�ݒ�\���̂ւ̃|�C���^

	@retval TRUE ����
	@retval FALSE ���s

	@note �^�C�v�ݒ肪�ς������ă��[�h���R���p�C������B
*/
bool RegexKeyword::RegexKeySetTypes(const TypeConfig *pTypesPtr)
{
	MYDBGMSG("RegexKeySetTypes")
	if (!pTypesPtr)  {
		m_pTypes = NULL;
		m_bUseRegexKeyword = false;
		return false;
	}

	if (!pTypesPtr->bUseRegexKeyword) {
		// OFF�ɂȂ����̂ɂ܂�ON�Ȃ�OFF�ɂ���B
		if (m_bUseRegexKeyword) {
			m_pTypes = NULL;
			m_bUseRegexKeyword = false;
		}
		return false;
	}

	if (m_nTypeId == pTypesPtr->id
		&& m_nCompiledMagicNumber == pTypesPtr->nRegexKeyMagicNumber
		&& m_pTypes  // 2014.07.02 �����ǉ�
	) {
		return true;
	}

	m_pTypes = pTypesPtr;

	RegexKeyCompile();
	
	return true;
}

// ���K�\���L�[���[�h�R���p�C������
/*!	@brief ���K�\���L�[���[�h�R���p�C��

	���K�\���L�[���[�h���R���p�C������B

	@retval TRUE ����
	@retval FALSE ���s

	@note ���łɃR���p�C���ς݂̏ꍇ�͂����j������B
	�L�[���[�h�̓R���p�C���f�[�^�Ƃ��ē����ϐ��ɃR�s�[����B
	�擪�w��A�F�w�葤�̎g�p�E���g�p���`�F�b�N����B
*/
bool RegexKeyword::RegexKeyCompile(void)
{
	static const wchar_t dummy[2] = L"\0";
	const struct RegexKeywordInfo	*rp;

	MYDBGMSG("RegexKeyCompile")
	// �R���p�C���ς݂̃o�b�t�@���������B
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (m_info[i].pBregexp && IsAvailable()) {
			BRegfree(m_info[i].pBregexp);
		}
		m_info[i].pBregexp = NULL;
	}

	// �R���p�C���p�^�[��������ϐ��Ɉڂ��B
	m_nRegexKeyCount = 0;
	const wchar_t* pKeyword = &m_pTypes->regexKeywordList[0];
#ifdef USE_PARENT
#else
	wmemcpy(m_keywordList,  m_pTypes->m_RegexKeywordList, _countof(m_RegexKeywordList));
#endif
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (pKeyword[0] == L'\0') {
			break;
		}
#ifdef USE_PARENT
#else
		m_info[i].sRegexKey.m_nColorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#endif
		++m_nRegexKeyCount;
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	m_nTypeIndex = m_pTypes->nIdx;
	m_nTypeId = m_pTypes->id;
	m_nCompiledMagicNumber = 1;	// Not Compiled.
	m_bUseRegexKeyword = m_pTypes->bUseRegexKeyword;
	if (!m_bUseRegexKeyword) {
		return false;
	}

	if (!IsAvailable()) {
		m_bUseRegexKeyword = false;
		return false;
	}

#ifdef USE_PARENT
	pKeyword = &m_pTypes->regexKeywordList[0];
#else
	pKeyword = &m_keywordList[0];
#endif
	// �p�^�[�����R���p�C������B
	for (int i=0; i<m_nRegexKeyCount; ++i) {
#ifdef USE_PARENT
		rp = &m_pTypes->regexKeywordArr[i];
#else
		rp = &m_info[i].sRegexKey;
#endif

		if (RegexKeyCheckSyntax(pKeyword)) {
			m_szMsg[0] = '\0';
			BMatch(pKeyword, dummy, dummy + 1, &m_info[i].pBregexp, m_szMsg);

			if (m_szMsg[0] == '\0') {	// �G���[���Ȃ����`�F�b�N����
				// �擪�ȊO�͌������Ȃ��Ă悢
				if (wcsncmp(RK_HEAD_STR1, pKeyword, RK_HEAD_STR1_LEN) == 0
				 || wcsncmp(RK_HEAD_STR2, pKeyword, RK_HEAD_STR2_LEN) == 0
				 || wcsncmp(RK_HEAD_STR3, pKeyword, RK_HEAD_STR3_LEN) == 0
				) {
					m_info[i].nHead = 1;
				}else {
					m_info[i].nHead = 0;
				}

				if (COLORIDX_REGEX1  <= rp->m_nColorIndex
				 && COLORIDX_REGEX10 >= rp->m_nColorIndex
				) {
					// �F�w��Ń`�F�b�N�������ĂȂ���Ό������Ȃ��Ă��悢
					if (m_pTypes->colorInfoArr[rp->m_nColorIndex].bDisp) {
						m_info[i].nFlag = RK_EMPTY;
					}else {
						// ���K�\���ł͐F�w��̃`�F�b�N������B
						m_info[i].nFlag = RK_NOMATCH;
					}
				}else {
					// ���K�\���ȊO�ł́A�F�w��`�F�b�N�͌��Ȃ��B
					// �Ⴆ�΁A���p���l�͐��K�\�����g���A��{�@�\���g��Ȃ��Ƃ����w������蓾�邽��
					m_info[i].nFlag = RK_EMPTY;
				}
			}else {
				// �R���p�C���G���[�Ȃ̂Ō����Ώۂ���͂���
				m_info[i].nFlag = RK_NOMATCH;
			}
		}else {
			// �����G���[�Ȃ̂Ō����Ώۂ���͂���
			m_info[i].nFlag = RK_NOMATCH;
		}
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	m_nCompiledMagicNumber = m_pTypes->nRegexKeyMagicNumber;	// Compiled.

	return true;
}

// �s�����J�n����
/*!	@brief �s�����J�n

	�s�������J�n����B

	@retval TRUE ����
	@retval FALSE ���s�܂��͌������Ȃ��w�肠��

	@note ���ꂼ��̍s�����̍ŏ��Ɏ��s����B
	�^�C�v�ݒ蓙���ύX����Ă���ꍇ�̓����[�h����B
*/
bool RegexKeyword::RegexKeyLineStart(void)
{
	MYDBGMSG("RegexKeyLineStart")

	// ����ɕK�v�ȃ`�F�b�N������B
	if (!m_bUseRegexKeyword || !IsAvailable() || !m_pTypes) {
		return false;
	}

#if 0	// RegexKeySetTypes�Őݒ肳��Ă���͂��Ȃ̂Ŕp�~
	// ���s��v�Ȃ�}�X�^����擾���ăR���p�C������B
	if (0
		|| m_nCompiledMagicNumber != m_pTypes->m_nRegexKeyMagicNumber
	 	|| m_nTypeIndex           != m_pTypes->m_nIdx
	) {
		RegexKeyCompile();
	}
#endif

	// �����J�n�̂��߂ɃI�t�Z�b�g��񓙂��N���A����B
	for (int i=0; i<m_nRegexKeyCount; ++i) {
		m_info[i].nOffset = -1;
		// m_info[i].nMatch  = RK_EMPTY;
		m_info[i].nMatch  = m_info[i].nFlag;
		m_info[i].nStatus = RK_EMPTY;
	}

	return true;
}

// ���K�\����������
/*!	@brief ���K�\������

	���K�\���L�[���[�h����������B

	@retval TRUE ��v
	@retval FALSE �s��v

	@note RegexKeyLineStart�֐��ɂ���ď���������Ă��邱�ƁB
*/
bool RegexKeyword::RegexIsKeyword(
	const StringRef&	str,		// [in] �����Ώە�����
//	const wchar_t*		pLine,		// [in] �P�s�̃f�[�^
	int					nPos,		// [in] �����J�n�I�t�Z�b�g
//	int					nLineLen,	// [in] �P�s�̒���
	int*				nMatchLen,	// [out] �}�b�`��������
	int*				nMatchColor	// [out] �}�b�`�����F�ԍ�
	)
{
	MYDBGMSG("RegexIsKeyword")

	// ����ɕK�v�ȃ`�F�b�N������B
	if (0
		|| !m_bUseRegexKeyword
		|| !IsAvailable()
#ifdef USE_PARENT
		|| !m_pTypes
#endif
		// || (!pLine)
	) {
		return false;
	}

	for (int i=0; i<m_nRegexKeyCount; ++i) {
		if (m_info[i].nMatch != RK_NOMATCH) {  // ���̍s�ɃL�[���[�h���Ȃ��ƕ������Ă��Ȃ�
			if (m_info[i].nOffset == nPos) {  // �ȑO�����������ʂɈ�v����
				*nMatchLen   = m_info[i].nLength;
#ifdef USE_PARENT
				*nMatchColor = m_pTypes->regexKeywordArr[i].m_nColorIndex;
#else
				*nMatchColor = m_info[i].sRegexKey.m_nColorIndex;
#endif
				return true;  // �}�b�`����
			}

			// �ȑO�̌��ʂ͂����Â��̂ōČ�������
			if (m_info[i].nOffset < nPos) {
#ifdef USE_PARENT
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg)
					: BMatch(NULL,                  str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
#else
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
					: BMatch(NULL,                  str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
#endif
				if (matched) {
					m_info[i].nOffset = m_info[i].pBregexp->startp[0] - str.GetPtr();
					m_info[i].nLength = m_info[i].pBregexp->endp[0] - m_info[i].pBregexp->startp[0];
					m_info[i].nMatch  = RK_MATCH;
				
					// �w��̊J�n�ʒu�Ń}�b�`����
					if (m_info[i].nOffset == nPos) {
						if (m_info[i].nHead != 1 || nPos == 0) {
							*nMatchLen   = m_info[i].nLength;
#ifdef USE_PARENT
							*nMatchColor = m_pTypes->regexKeywordArr[i].m_nColorIndex;
#else
							*nMatchColor = m_info[i].sRegexKey.m_nColorIndex;
#endif
							return true;  // �}�b�`����
						}
					}

					// �s�擪��v�����鐳�K�\���ł͎��񂩂疳������
					if (m_info[i].nHead == 1) {
						m_info[i].nMatch = RK_NOMATCH;
					}
				}else {
					// ���̍s�ɂ��̃L�[���[�h�͂Ȃ�
					m_info[i].nMatch = RK_NOMATCH;
				}
			}
		}
	}  // for

	return false;
}

bool RegexKeyword::RegexKeyCheckSyntax(const wchar_t* s)
{
	static const wchar_t* kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		NULL, NULL,
	};

	int	length = wcslen(s);
	for (int i=0; kakomi[i]; i+=2) {
		// ���������m���߂�
		if (length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i + 1])) {
			// �n�܂���m���߂�
			if (wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0) {
				// �I�����m���߂�
				const wchar_t* p = &s[length - wcslen(kakomi[i+1])];
				if (wcscmp(p, kakomi[i + 1]) == 0) {
					// ����
					return true;
				}
			}
		}
	}
	return false;
}

//@@@ 2001.11.17 add end MIK

// static
DWORD RegexKeyword::GetNewMagicNumber()
{
	return ::GetTickCount();
}

