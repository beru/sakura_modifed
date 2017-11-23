#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "env/DllSharedData.h"
#include "outline/FuncInfo.h"
#include "outline/FuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

// �e�L�X�g
void CType_Text::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("�e�L�X�g"));
	_tcscpy(type.szTypeExts, _T("txt,log,1st,err,ps"));

	// �ݒ�
	type.nMaxLineKetas = 120;					// �܂�Ԃ�����
	type.eDefaultOutline = OutlineType::Text;				// �A�E�g���C����͕��@
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	type.bKinsokuHead = false;								// �s���֑�
	type.bKinsokuTail = false;								// �s���֑�
	type.bKinsokuRet  = false;								// ���s�������Ԃ牺����
	type.bKinsokuKuto = false;								// ��Ǔ_���Ԃ牺����
	wcscpy_s(type.szKinsokuHead, L"!%),.:;?]}�����f�h�񁌁����A�B�X�r�t�v�x�z�l�J�K�T�U�E�R�S�I���j�C�D�F�G�H�n�p�����߁�");		/* �s���֑� */
	wcscpy_s(type.szKinsokuTail, L"$([{��\\�e�g�q�s�u�w�y�k���i�m�o�����");		/* �s���֑� */
	// type.szKinsokuKuto�i��Ǔ_�Ԃ牺�������j�͂����ł͂Ȃ��S�^�C�v�Ƀf�t�H���g�ݒ�

	// �������Ȑe�؂Ƃ��āAC:\�`�` �� \\�`�` �Ȃǂ̃t�@�C���p�X���N���b�J�u���ɂ���ݒ���u�e�L�X�g�v�Ɋ���Ŏd����
	// ��""�ŋ��܂��ݒ�͋��܂�Ȃ��ݒ������ɖ�����΂Ȃ�Ȃ�
	// ��""�ŋ��܂��ݒ�𕡐����Ă�����ƏC������΁A<>��[]�ɋ��܂ꂽ���̂ɂ��Ή��ł���i���[�U�ɔC����j

	// ���K�\���L�[���[�h
	size_t keywordPos = 0;
	wchar_t* pKeyword = type.regexKeywordList;
	type.bUseRegexKeyword = true;							// ���K�\���L�[���[�h���g����
	type.regexKeywordArr[0].nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn(&pKeyword[keywordPos],			// ���K�\���L�[���[�h
		L"/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k",			//   ""�ŋ��܂ꂽ C:\�`, \\�` �Ƀ}�b�`����p�^�[��
		_countof(type.regexKeywordList) - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	type.regexKeywordArr[1].nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn(&pKeyword[keywordPos],			// ���K�\���L�[���[�h
		L"/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k",		//   C:\�`, \\�` �Ƀ}�b�`����p�^�[��
		_countof(type.regexKeywordList) - keywordPos - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	pKeyword[keywordPos] = L'\0';
}


/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬 */
void DocOutline::MakeTopicList_txt(FuncInfoArr* pFuncInfoArr)
{
	using namespace WCODE;

	// ���o���L��
	const wchar_t*	pszStarts = GetDllShareData().common.format.szMidashiKigou;
	size_t			nStartsLen = wcslen(pszStarts);

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	size_t nLineCount;
	bool b278a = false;
	for (nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		// �s�擾
		size_t nLineLen;
		const wchar_t* pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}

		// �s���̋󔒔�΂�
		size_t i;
		for (i=0; i<nLineLen; ++i) {
			if (WCODE::IsBlank(pLine[i])) {
				continue;
			}
			break;
		}
		if (i >= nLineLen) {
			continue;
		}

		// �擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
		size_t nCharChars2;
		size_t j;
		for (j=0; j<nStartsLen; j+=nCharChars2) {
			nCharChars2 = NativeW::GetSizeOfChar(pszStarts, nStartsLen, j);
			if (nCharChars == nCharChars2) {
				if (wmemcmp(&pLine[i], &pszStarts[j], nCharChars) == 0) {
					break;
				}
			}
		}
		if (j >= nStartsLen) {
			continue;
		}

		// ���o����ނ̔��� -> szTitle
		if (pLine[i] == L'(') {
			     if (IsInRange(pLine[i + 1], L'0', L'9')) wcscpy_s(szTitle, L"(0)"); // ����
			else if (IsInRange(pLine[i + 1], L'A', L'Z')) wcscpy_s(szTitle, L"(A)"); // �p�啶��
			else if (IsInRange(pLine[i + 1], L'a', L'z')) wcscpy_s(szTitle, L"(a)"); // �p������
			else continue; // ���u(�v�̎����p�����Ŗ����ꍇ�A���o���Ƃ݂Ȃ��Ȃ�
		}
		else if (IsInRange(pLine[i], L'�O', L'�X')) wcscpy(szTitle, L"�O"); // �S�p����
		else if (0
			|| IsInRange(pLine[i], L'�@', L'�S')
			|| pLine[i] == L'\u24ea'
			|| IsInRange(pLine[i], L'\u3251', L'\u325f')
			|| IsInRange(pLine[i], L'\u32b1', L'\u32bf')
		) wcscpy(szTitle, L"�@"); // �@�`�S ��0�@��21��35�@��36��50
		else if (IsInRange(pLine[i], L'�T', L'\u216f')) wcscpy(szTitle, L"�T"); // �T�`�]�@XIXIILCDM
		else if (IsInRange(pLine[i], L'�@', L'\u217f')) wcscpy(szTitle, L"�T"); // �T�`�]�@xixiilcdm
		else if (IsInRange(pLine[i], L'\u2474', L'\u2487')) wcscpy(szTitle, L"\u2474"); // (1)-(20)
		else if (IsInRange(pLine[i], L'\u2488', L'\u249b')) wcscpy(szTitle, L"\u2488"); // 1.-20.
		else if (IsInRange(pLine[i], L'\u249c', L'\u24b5')) wcscpy(szTitle, L"\u249c"); // (a)-(z)
		else if (IsInRange(pLine[i], L'\u24b6', L'\u24cf')) wcscpy(szTitle, L"\u24b6"); // ��A-��Z
		else if (IsInRange(pLine[i], L'\u24d0', L'\u24e9')) wcscpy(szTitle, L"\u24d0"); // ��a-��z
		else if (IsInRange(pLine[i], L'\u24eb', L'\u24f4')) { // ��11-��20
			if (b278a) { wcscpy(szTitle, L"\u278a"); }
			else { wcscpy(szTitle, L"\u2776"); }
		}else if (IsInRange(pLine[i], L'\u24f5', L'\u24fe')) wcscpy(szTitle, L"\u24f5"); // ��1-��10
		else if (IsInRange(pLine[i], L'\u2776', L'\u277f')) wcscpy(szTitle, L"\u2776"); // ��1-��10
		else if (IsInRange(pLine[i], L'\u2780', L'\u2789')) wcscpy(szTitle, L"\u2780"); // ��1-��10
		else if (IsInRange(pLine[i], L'\u278a', L'\u2793')) { wcscpy(szTitle, L"\u278a"); b278a = true; } // ��1-��10(SANS-SERIF)
		else if (IsInRange(pLine[i], L'\u3220', L'\u3229')) wcscpy(szTitle, L"\ua3220"); // (��)-(�\)
		else if (IsInRange(pLine[i], L'\u3280', L'\u3289')) wcscpy(szTitle, L"\u3220"); // ����-���\
		else if (IsInRange(pLine[i], L'\u32d0', L'\u32fe')) wcscpy(szTitle, L"\u32d0"); // ���A-����
		else if (wcschr(L"�Z���O�l�ܘZ������\�S����Q��", pLine[i])) wcscpy(szTitle, L"��"); // ������
		else {
			wcsncpy(szTitle, &pLine[i], nCharChars);	//	�擪������szTitle�ɕێ��B
			szTitle[nCharChars] = L'\0';
		}

		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/

		// �s�����񂩂���s����菜�� pLine -> pszText
		std::vector<wchar_t> szText(nLineLen + 1);
		wchar_t* pszText = &szText[0];
		wmemcpy(pszText, &pLine[i], nLineLen);
		pszText[nLineLen] = L'\0';
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		for (i=0; i<nLineLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
		// nDepth���v�Z
		int k;
		bool bAppend = true;
		for (k=0; k<nDepth; ++k) {
			int nResult = wcscmp(pszStack[k], szTitle);
			if (nResult == 0) {
				break;
			}
		}
		if (k < nDepth) {
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}else if (nMaxStack > k) {
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy_s(pszStack[nDepth], szTitle);
		}else {
			// �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if (bAppend) {
			pFuncInfoArr->AppendData(
				nLineCount + 1,
				ptPos.y + 1,
				pszText,
				0,
				nDepth
			);
			++nDepth;
		}
	}
	return;
}


/*! �K�w�t���e�L�X�g �A�E�g���C����� */
void DocOutline::MakeTopicList_wztxt(FuncInfoArr* pFuncInfoArr)
{
	size_t levelPrev = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		const wchar_t*	pLine;
		size_t nLineLen;

		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}
		if (*pLine == L'.') {
			const wchar_t* pPos;
			int			nLength;
			wchar_t		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for (pPos=pLine+1; *pPos==L'.'; ++pPos)
				;

			Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
			size_t level = pPos - pLine;

			// �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if (levelPrev < level && level != levelPrev + 1) {
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for (size_t dummyLevel=levelPrev+1; dummyLevel<level; ++dummyLevel) {
					pFuncInfoArr->AppendData(
						nLineCount + 1,
						ptPos.y + 1,
						LSW(STR_NO_TITLE1),
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf_s(szTitle, L"%d - ", level);
			
			wchar_t* pDest = szTitle + nLength; // �������ݐ�
			wchar_t* pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while (pDest < pDestEnd) {
				if (WCODE::IsLineDelimiter(*pPos, bExtEol) || *pPos == L'\0') {
					break;
				}else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pFuncInfoArr->AppendData(nLineCount + 1, ptPos.y + 1, szTitle, 0, level - 1);
		}
	}
}

