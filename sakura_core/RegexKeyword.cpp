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
	fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &pTypes, s, &BMatch, BMatch, bUseRegexKeyword, nTypeIndex);\
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

	pTypes    = nullptr;
	nTypeIndex = -1;
	nTypeId = -1;

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
		auto& in = info[i];
		if (in.pBregexp && IsAvailable()) {
			BRegfree(in.pBregexp);
		}
		in.pBregexp = nullptr;
	}
	
	RegexKeyInit();

	nTypeIndex = -1;
	pTypes     = nullptr;
}

// ���K�\���L�[���[�h����������
/*!	@brief ���K�\���L�[���[�h������

	 ���K�\���L�[���[�h�Ɋւ���ϐ��ނ�����������B

	@retval true ����
*/
bool RegexKeyword::RegexKeyInit(void)
{
	MYDBGMSG("RegexKeyInit")
	nTypeIndex = -1;
	nTypeId = -1;
	nCompiledMagicNumber = 1;
	bUseRegexKeyword = false;
	nRegexKeyCount = 0;
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		auto& in = info[i];
		in.pBregexp = nullptr;
#ifdef USE_PARENT
#else
		in.sRegexKey.nColorIndex = COLORIDX_REGEX1;
#endif
	}
#ifdef USE_PARENT
#else
	wmemset(keywordList, _countof(keywordList), L'\0');
#endif

	return true;
}

// ���݃^�C�v�ݒ菈��
/*!	@brief ���݃^�C�v�ݒ�

	���݂̃^�C�v�ݒ��ݒ肷��B

	@param pTypesPtr [in] �^�C�v�ݒ�\���̂ւ̃|�C���^

	@retval true ����
	@retval false ���s

	@note �^�C�v�ݒ肪�ς������ă��[�h���R���p�C������B
*/
bool RegexKeyword::RegexKeySetTypes(const TypeConfig *pTypesPtr)
{
	MYDBGMSG("RegexKeySetTypes")
	if (!pTypesPtr)  {
		pTypes = nullptr;
		bUseRegexKeyword = false;
		return false;
	}

	if (!pTypesPtr->bUseRegexKeyword) {
		// OFF�ɂȂ����̂ɂ܂�ON�Ȃ�OFF�ɂ���B
		if (bUseRegexKeyword) {
			pTypes = nullptr;
			bUseRegexKeyword = false;
		}
		return false;
	}

	if (nTypeId == pTypesPtr->id
		&& nCompiledMagicNumber == pTypesPtr->nRegexKeyMagicNumber
		&& pTypes  // 2014.07.02 �����ǉ�
	) {
		return true;
	}

	pTypes = pTypesPtr;

	RegexKeyCompile();
	
	return true;
}

// ���K�\���L�[���[�h�R���p�C������
/*!	@brief ���K�\���L�[���[�h�R���p�C��

	���K�\���L�[���[�h���R���p�C������B

	@retval true ����
	@retval false ���s

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
		auto& in = info[i];
		if (in.pBregexp && IsAvailable()) {
			BRegfree(in.pBregexp);
		}
		in.pBregexp = nullptr;
	}

	// �R���p�C���p�^�[��������ϐ��Ɉڂ��B
	nRegexKeyCount = 0;
	const wchar_t* pKeyword = &pTypes->regexKeywordList[0];
#ifdef USE_PARENT
#else
	wmemcpy(keywordList,  pTypes->RegexKeywordList, _countof(RegexKeywordList));
#endif
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (pKeyword[0] == L'\0') {
			break;
		}
#ifdef USE_PARENT
#else
		info[i].sRegexKey.nColorIndex = pTypes->RegexKeywordArr[i].nColorIndex;
#endif
		++nRegexKeyCount;
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	nTypeIndex = pTypes->nIdx;
	nTypeId = pTypes->id;
	nCompiledMagicNumber = 1;	// Not Compiled.
	bUseRegexKeyword = pTypes->bUseRegexKeyword;
	if (!bUseRegexKeyword) {
		return false;
	}

	if (!IsAvailable()) {
		bUseRegexKeyword = false;
		return false;
	}

#ifdef USE_PARENT
	pKeyword = &pTypes->regexKeywordList[0];
#else
	pKeyword = &keywordList[0];
#endif
	// �p�^�[�����R���p�C������B
	for (int i=0; i<nRegexKeyCount; ++i) {
		auto& in = info[i];
#ifdef USE_PARENT
		rp = &pTypes->regexKeywordArr[i];
#else
		rp = &in.sRegexKey;
#endif

		if (RegexKeyCheckSyntax(pKeyword)) {
			szMsg[0] = '\0';
			BMatch(pKeyword, dummy, dummy + 1, &in.pBregexp, szMsg);

			if (szMsg[0] == '\0') {	// �G���[���Ȃ����`�F�b�N����
				// �擪�ȊO�͌������Ȃ��Ă悢
				if (wcsncmp(RK_HEAD_STR1, pKeyword, RK_HEAD_STR1_LEN) == 0
				 || wcsncmp(RK_HEAD_STR2, pKeyword, RK_HEAD_STR2_LEN) == 0
				 || wcsncmp(RK_HEAD_STR3, pKeyword, RK_HEAD_STR3_LEN) == 0
				) {
					in.nHead = 1;
				}else {
					in.nHead = 0;
				}

				if (COLORIDX_REGEX1  <= rp->nColorIndex
				 && COLORIDX_REGEX10 >= rp->nColorIndex
				) {
					// �F�w��Ń`�F�b�N�������ĂȂ���Ό������Ȃ��Ă��悢
					if (pTypes->colorInfoArr[rp->nColorIndex].bDisp) {
						in.nFlag = RK_EMPTY;
					}else {
						// ���K�\���ł͐F�w��̃`�F�b�N������B
						in.nFlag = RK_NOMATCH;
					}
				}else {
					// ���K�\���ȊO�ł́A�F�w��`�F�b�N�͌��Ȃ��B
					// �Ⴆ�΁A���p���l�͐��K�\�����g���A��{�@�\���g��Ȃ��Ƃ����w������蓾�邽��
					in.nFlag = RK_EMPTY;
				}
			}else {
				// �R���p�C���G���[�Ȃ̂Ō����Ώۂ���͂���
				in.nFlag = RK_NOMATCH;
			}
		}else {
			// �����G���[�Ȃ̂Ō����Ώۂ���͂���
			in.nFlag = RK_NOMATCH;
		}
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	nCompiledMagicNumber = pTypes->nRegexKeyMagicNumber;	// Compiled.

	return true;
}

// �s�����J�n����
/*!	@brief �s�����J�n

	�s�������J�n����B

	@retval true ����
	@retval false ���s�܂��͌������Ȃ��w�肠��

	@note ���ꂼ��̍s�����̍ŏ��Ɏ��s����B
	�^�C�v�ݒ蓙���ύX����Ă���ꍇ�̓����[�h����B
*/
bool RegexKeyword::RegexKeyLineStart(void)
{
	MYDBGMSG("RegexKeyLineStart")

	// ����ɕK�v�ȃ`�F�b�N������B
	if (!bUseRegexKeyword || !IsAvailable() || !pTypes) {
		return false;
	}

#if 0	// RegexKeySetTypes�Őݒ肳��Ă���͂��Ȃ̂Ŕp�~
	// ���s��v�Ȃ�}�X�^����擾���ăR���p�C������B
	if (0
		|| nCompiledMagicNumber != pTypes->nRegexKeyMagicNumber
	 	|| nTypeIndex           != pTypes->nIdx
	) {
		RegexKeyCompile();
	}
#endif

	// �����J�n�̂��߂ɃI�t�Z�b�g��񓙂��N���A����B
	for (int i=0; i<nRegexKeyCount; ++i) {
		auto& in = info[i];
		in.nOffset = -1;
		// info.nMatch  = RK_EMPTY;
		in.nMatch  = in.nFlag;
		in.nStatus = RK_EMPTY;
	}

	return true;
}

// ���K�\����������
/*!	@brief ���K�\������

	���K�\���L�[���[�h����������B

	@retval true ��v
	@retval false �s��v

	@note RegexKeyLineStart�֐��ɂ���ď���������Ă��邱�ƁB
*/
bool RegexKeyword::RegexIsKeyword(
	const StringRef&	str,		// [in] �����Ώە�����
//	const wchar_t*		pLine,		// [in] �P�s�̃f�[�^
	int					nPos,		// [in] �����J�n�I�t�Z�b�g
//	int					nLineLen,	// [in] �P�s�̒���
	size_t*				nMatchLen,	// [out] �}�b�`��������
	int*				nMatchColor	// [out] �}�b�`�����F�ԍ�
	)
{
	MYDBGMSG("RegexIsKeyword")

	// ����ɕK�v�ȃ`�F�b�N������B
	if (0
		|| !bUseRegexKeyword
		|| !IsAvailable()
#ifdef USE_PARENT
		|| !pTypes
#endif
		// || (!pLine)
	) {
		return false;
	}

	for (int i=0; i<nRegexKeyCount; ++i) {
		auto& in = info[i];
		if (in.nMatch != RK_NOMATCH) {  // ���̍s�ɃL�[���[�h���Ȃ��ƕ������Ă��Ȃ�
			if (in.nOffset == nPos) {  // �ȑO�����������ʂɈ�v����
				*nMatchLen   = in.nLength;
#ifdef USE_PARENT
				*nMatchColor = pTypes->regexKeywordArr[i].nColorIndex;
#else
				*nMatchColor = in.sRegexKey.nColorIndex;
#endif
				return true;  // �}�b�`����
			}

			// �ȑO�̌��ʂ͂����Â��̂ōČ�������
			if (in.nOffset < nPos) {
#ifdef USE_PARENT
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &in.pBregexp, szMsg)
					: BMatch(NULL,                 str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &in.pBregexp, szMsg);
#else
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &in.pBregexp, szMsg);
					: BMatch(NULL,                 str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &in.pBregexp, szMsg);
#endif
				if (matched) {
					in.nOffset = in.pBregexp->startp[0] - str.GetPtr();
					in.nLength = in.pBregexp->endp[0] - in.pBregexp->startp[0];
					in.nMatch  = RK_MATCH;
				
					// �w��̊J�n�ʒu�Ń}�b�`����
					if (in.nOffset == nPos) {
						if (in.nHead != 1 || nPos == 0) {
							*nMatchLen   = in.nLength;
#ifdef USE_PARENT
							*nMatchColor = pTypes->regexKeywordArr[i].nColorIndex;
#else
							*nMatchColor = info.sRegexKey.nColorIndex;
#endif
							return true;  // �}�b�`����
						}
					}

					// �s�擪��v�����鐳�K�\���ł͎��񂩂疳������
					if (in.nHead == 1) {
						in.nMatch = RK_NOMATCH;
					}
				}else {
					// ���̍s�ɂ��̃L�[���[�h�͂Ȃ�
					in.nMatch = RK_NOMATCH;
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

	size_t length = wcslen(s);
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

