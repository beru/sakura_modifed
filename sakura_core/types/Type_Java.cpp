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
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

// Java
void CType_Java::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("Java"));
	_tcscpy(type.szTypeExts, _T("java,jav"));

	// �ݒ�
	type.lineComment.CopyTo(0, L"//", -1);						// �s�R�����g�f���~�^
	type.blockComments[0].SetBlockCommentRule(L"/*", L"*/");	// �u���b�N�R�����g�f���~�^
	type.nKeywordSetIdx[0] = 4;									// �L�[���[�h�Z�b�g
	type.eDefaultOutline = OutlineType::Java;						// �A�E�g���C����͕��@
	type.eSmartIndent = SmartIndentType::Cpp;						// �X�}�[�g�C���f���g���
	type.colorInfoArr[COLORIDX_DIGIT].bDisp = true;			// ���p���l��F�����\��			// Mar. 10, 2001 JEPRO
	type.colorInfoArr[COLORIDX_BRACKET_PAIR].bDisp = true;	// �Ί��ʂ̋������f�t�H���gON��	// Sep. 21, 2002 genta
	type.bStringLineOnly = true; // ������͍s���̂�
}


/* Java��̓��[�h */
enum class FuncListJavaMode {
	Normal = 0,
	Word = 1,
	Symbol = 2,
	Comment = 8,
	SingleQuote = 20,
	DoubleQuote = 21,
	TooLongWord = 999
};

/* Java�֐����X�g�쐬 */
void DocOutline::MakeFuncList_Java(FuncInfoArr* pFuncInfoArr)
{
	const wchar_t*	pLine;
	size_t		nLineLen;
	size_t		i;
	int			nNestLevel;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	FuncListJavaMode	mode;
	wchar_t		szFuncName[100];
	int	nFuncLine = 0;
	int			nFuncId;
	int			nFuncNum;
	wchar_t		szClass[1024];

	int			nClassNestArrNum;
	std::vector<int>	nClassNestArr(0);
	std::vector<int>	nNestLevel2Arr(0);

	nNestLevel = 0;
	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	mode = FuncListJavaMode::Normal;
	//nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = L'\0';
	nClassNestArrNum = 0;
	const wchar_t*	szJavaKigou = L"!\"#%&'()=-^|\\`@[{+;*}]<,>?/";	// ���ʎq�Ɏg�p�ł��Ȃ����p�L���B_:~.$�͋���
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for (i=0; i<nLineLen; i+=nCharChars) {
			nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if (L'\\' == pLine[i]) {
				++i;
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			}else if (mode == FuncListJavaMode::SingleQuote) {
				if (L'\'' == pLine[i]) {
					mode = FuncListJavaMode::Normal;
					continue;
				}else {
				}
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			}else if (mode == FuncListJavaMode::DoubleQuote) {
				if (L'"' == pLine[i]) {
					mode = FuncListJavaMode::Normal;
					continue;
				}else {
				}
			/* �R�����g�ǂݍ��ݒ� */
			}else if (mode == FuncListJavaMode::Comment) {
				if (i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1]) {
					++i;
					mode = FuncListJavaMode::Normal;
					continue;
				}else {
				}
			/* �P��ǂݍ��ݒ� */
			}else if (mode == FuncListJavaMode::Word) {
				// 2011.09.16 syat �A�E�g���C����͂œ��{�ꂪ�܂܂�Ă��镔�����\������Ȃ�
				if (! WCODE::IsBlank(pLine[i]) &&
					! WCODE::IsLineDelimiter(pLine[i], bExtEol) &&
					! WCODE::IsControlCode(pLine[i]) &&
					! wcschr(szJavaKigou, pLine[i])
				) {
					if (nWordIdx + nCharChars >= nMaxWordLeng) {
						mode = FuncListJavaMode::TooLongWord;
						continue;
					}else {
						memcpy(&szWord[nWordIdx], &pLine[i], sizeof(wchar_t)*nCharChars);
						szWord[nWordIdx + nCharChars] = '\0';
					}
					nWordIdx += nCharChars;
				}else {
					/* �N���X�錾������������ */
					//	Oct. 10, 2002 genta interface���Ώۂ�
					if (wcscmp(L"class", szWordPrev) == 0 ||
						wcscmp(L"interface", szWordPrev) == 0
					) {
						nClassNestArr.push_back(nNestLevel);
						nNestLevel2Arr.push_back(0);
						++nClassNestArrNum;
						if (0 < nNestLevel) {
							wcscat(szClass, L"\\");
						}
						wcscat(szClass, szWord);

						nFuncId = FL_OBJ_DEFINITION;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						Point ptPosXY_Logic(0, nLineCount);
						Point ptPosXY_Layout = doc.layoutMgr.LogicToLayout(ptPosXY_Logic);
						wchar_t szWork[256];
						if (0 < auto_snprintf_s(szWork, _countof(szWork), L"%ls::%ls", szClass, LSW(STR_OUTLINE_JAVA_DEFPOS))) {
							pFuncInfoArr->AppendData(ptPosXY_Logic.y + 1, ptPosXY_Layout.y + 1, szWork, nFuncId); //2007.10.09 kobake ���C�A�E�g�E���W�b�N�̍��݃o�O�C��
						}
					}

					mode = FuncListJavaMode::Normal;
					--i;
					continue;
				}
			/* �L����ǂݍ��ݒ� */
			}else if (mode == FuncListJavaMode::Symbol) {
				if (L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z')||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z')||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9')||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'"' == pLine[i] ||
					L'/' == pLine[i] ||
					L'.' == pLine[i]
				) {
					mode = FuncListJavaMode::Normal;
					--i;
					continue;
				}else {
				}
			/* ���߂���P�ꖳ���� */
			}else if (mode == FuncListJavaMode::TooLongWord) {
				/* �󔒂�^�u�L�������΂� */
				if (L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				) {
					mode = FuncListJavaMode::Normal;
					continue;
				}
			/* �m�[�}�����[�h */
			}else if (mode == FuncListJavaMode::Normal) {
				/* �󔒂�^�u�L�������΂� */
				if (L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				) {
					continue;
				}else if (i < nLineLen - 1 && L'/' == pLine[i] &&  L'/' == pLine[i + 1]) {
					break;
				}else if (i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1]) {
					++i;
					mode = FuncListJavaMode::Comment;
					continue;
				}else if (L'\'' == pLine[i]) {
					mode = FuncListJavaMode::SingleQuote;
					continue;
				}else if (L'"' == pLine[i]) {
					mode = FuncListJavaMode::DoubleQuote;
					continue;
				}else if (L'{' == pLine[i]) {
					if (0 < nClassNestArrNum && nNestLevel2Arr[nClassNestArrNum - 1] == 2) {
						//	Oct. 10, 2002 genta
						//	���\�b�h���ł���Ƀ��\�b�h���`���邱�Ƃ͂Ȃ��̂�
						//	�l�X�g���x������ǉ� class/interface�̒����̏ꍇ�̂ݔ��肷��
						if (nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && wcscmp(L"sizeof", szFuncName) != 0
						 && wcscmp(L"if", szFuncName) != 0
						 && wcscmp(L"for", szFuncName) != 0
						 && wcscmp(L"do", szFuncName) != 0
						 && wcscmp(L"while", szFuncName) != 0
						 && wcscmp(L"catch", szFuncName) != 0
						 && wcscmp(L"switch", szFuncName) != 0
						 && wcscmp(L"return", szFuncName) != 0
						) {
							nFuncId = FL_OBJ_FUNCTION;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							Point ptPosXY = doc.layoutMgr.LogicToLayout(Point(0, nFuncLine - 1));
							wchar_t szWork[256];
							if (0 < auto_snprintf_s(szWork, _countof(szWork), L"%ls::%ls", szClass, szFuncName)) {
								pFuncInfoArr->AppendData(nFuncLine, ptPosXY.y + 1, szWork, nFuncId);
							}
						}
					}
					if (0 < nClassNestArrNum) {
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					mode = FuncListJavaMode::Normal;
					continue;
				}else if (L'}' == pLine[i]) {
					if (0 < nClassNestArrNum) {
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					--nNestLevel;
					if (0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					) {
						nClassNestArr.pop_back();
						nNestLevel2Arr.pop_back();
						--nClassNestArrNum;
						int k;
						for (k=wcslen(szClass)-1; k>=0; --k) {
							if (L'\\' == szClass[k]) {
								break;
							}
						}
						if (0 > k) {
							k = 0;
						}
						szClass[k] = L'\0';
					}
					mode = FuncListJavaMode::Normal;
					continue;
				}else if (L'(' == pLine[i]) {
					if (0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						wcscmp(L"new", szWordPrev) != 0
					) {
						wcscpy_s(szFuncName, szWord);
						nFuncLine = nLineCount + 1;
						if (0 < nClassNestArrNum) {
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					mode = FuncListJavaMode::Normal;
					continue;
				}else if (L')' == pLine[i]) {
					size_t k;
					const wchar_t*	pLine2;
					size_t		nLineLen2;
					int	nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for (; k<nLineLen2; ++k) {
						if (!bCommentLoop) {
							if (pLine2[k] != L' ' && pLine2[k] != WCODE::TAB && !WCODE::IsLineDelimiter(pLine2[k], bExtEol)) {
								if (k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'*') {
									bCommentLoop = TRUE;
									++k;
								}else if (k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'/') {
									k = nLineLen2 + 1;
									break;
								}else {
									break;
								}
							}
						}else {
							if (k + 1 < nLineLen2 && pLine2[k] == L'*' && pLine2[k + 1] == L'/') {
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if (k >= nLineLen2) {
						k = 0;
						++nLineCount2;
						pLine2 = doc.docLineMgr.GetLine(nLineCount2)->GetDocLineStrWithEOL(&nLineLen2);
						if (pLine2) {
							goto loop_is_func;
						}
						if (0 < nClassNestArrNum) {
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else {
						//	Oct. 10, 2002 genta
						//	abstract �ɂ��Ή�
						if (pLine2[k] == L'{' || pLine2[k] == L';' ||
							__iscsym(pLine2[k])
						) {
							if (0 < nClassNestArrNum) {
								if (nNestLevel2Arr[nClassNestArrNum - 1] == 1) {
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else {
							if (0 < nClassNestArrNum) {
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					mode = FuncListJavaMode::Normal;
					continue;
				}else if (L';' == pLine[i]) {
					if (0 < nClassNestArrNum && nNestLevel2Arr[nClassNestArrNum - 1] == 2) {
						//	Oct. 10, 2002 genta
						// �֐��̒��ŕʂ̊֐��̐錾�����g�����Ƃ��āCJava�ł���́H
						if (1
							&& nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
							&& wcscmp(L"sizeof", szFuncName) != 0
							&& wcscmp(L"if", szFuncName) != 0
							&& wcscmp(L"for", szFuncName) != 0
							&& wcscmp(L"do", szFuncName) != 0
							&& wcscmp(L"while", szFuncName) != 0
							&& wcscmp(L"catch", szFuncName) != 0
							&& wcscmp(L"switch", szFuncName) != 0
							&& wcscmp(L"return", szFuncName) != 0
						) {
							nFuncId = FL_OBJ_DECLARE;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							Point ptPosXY = doc.layoutMgr.LogicToLayout(Point(0, nFuncLine - 1));
							wchar_t szWork[256];
							if (0 < auto_snprintf_s(szWork, _countof(szWork), L"%ls::%ls", szClass, szFuncName)) {
								pFuncInfoArr->AppendData(nFuncLine, ptPosXY.y + 1, szWork, nFuncId);
							}
						}
					}
					if (0 < nClassNestArrNum) {
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					mode = FuncListJavaMode::Normal;
					continue;
				}else {
					if (! WCODE::IsBlank(pLine[i]) &&
						! WCODE::IsLineDelimiter(pLine[i], bExtEol) &&
						! WCODE::IsControlCode(pLine[i]) &&
						! wcschr(szJavaKigou, pLine[i])
					) {
						wcscpy_s(szWordPrev, szWord);
						nWordIdx = 0;
						memcpy(&szWord[nWordIdx], &pLine[i], sizeof(wchar_t)*nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += nCharChars;
						mode = FuncListJavaMode::Word;
					}else {
						mode = FuncListJavaMode::Normal;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pFuncInfoArr->DUMP();
#endif
	return;
}

const wchar_t* g_ppszKeywordsJAVA[] = {
	L"abstract",
	L"assert",	// Mar. 8, 2003 genta
	L"boolean",
	L"break",
	L"byte",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"continue",
	L"default",
	L"do",
	L"double",
	L"else",
	L"extends",
	L"final",
	L"finally",
	L"float",
	L"for",
	L"goto",
	L"if",
	L"implements",
	L"import",
	L"instanceof",
	L"int",
	L"interface",
	L"long",
	L"native",
	L"new",
	L"package",
	L"private",
	L"protected",
	L"public",
	L"return",
	L"short",
	L"static",
	L"strictfp",	// Mar. 8, 2003 genta
	L"super",
	L"switch",
	L"synchronized",
	L"this",
	L"throw",
	L"throws",
	L"transient",
	L"try",
	L"void",
	L"volatile",
	L"while"
};
size_t g_nKeywordsJAVA = _countof(g_ppszKeywordsJAVA);

