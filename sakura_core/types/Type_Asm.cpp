#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

// �A�Z���u��
//	2004.05.01 MIK/genta
// Mar. 10, 2001 JEPRO	���p���l��F�����\��
void CType_Asm::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("�A�Z���u��"));
	_tcscpy(type.szTypeExts, _T("asm"));

	// �ݒ�
	type.lineComment.CopyTo(0, L";", -1);				// �s�R�����g�f���~�^
	type.eDefaultOutline = OutlineType::Asm;			// �A�E�g���C����͕��@
	type.colorInfoArr[COLORIDX_DIGIT].bDisp = true;
}


/*! �A�Z���u�� �A�E�g���C�����

	@author MIK
	@date 2004.04.12 ��蒼��
*/
void DocOutline::MakeTopicList_asm(FuncInfoArr* pFuncInfoArr)
{
	size_t nTotalLine = doc.docLineMgr.GetLineCount();
	for (size_t nLineCount=0; nLineCount<nTotalLine; ++nLineCount) {
		const wchar_t* pLine;
		size_t nLineLen;
		wchar_t* pTmpLine;
		size_t length;
		size_t offset;
#define MAX_ASM_TOKEN 2
		wchar_t* token[MAX_ASM_TOKEN];
		wchar_t* p;

		// 1�s�擾����B
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) break;

		// ��Ɨp�ɃR�s�[���쐬����B�o�C�i�����������炻�̌��͒m��Ȃ��B
		pTmpLine = wcsdup(pLine);
		if (!pTmpLine) break;
		if (wcslen(pTmpLine) >= (unsigned int)nLineLen) {	// �o�C�i�����܂�ł�����Z���Ȃ�̂�...
			pTmpLine[nLineLen] = L'\0';	// �w�蒷�Ő؂�l��
		}

		// �s�R�����g�폜
		p = wcsstr(pTmpLine, L";");
		if (p) *p = L'\0';

		length = wcslen(pTmpLine);
		offset = 0;

		// �g�[�N���ɕ���
		for (size_t j=0; j<MAX_ASM_TOKEN; ++j) token[j] = NULL;
		for (size_t j=0; j<MAX_ASM_TOKEN; ++j) {
			token[j] = my_strtok<wchar_t>(pTmpLine, length, &offset, L" \t\r\n");
			if (!token[j]) break;
			// �g�[�N���Ɋ܂܂��ׂ������łȂ����H
			if (wcsstr(token[j], L"\"")
			 || wcsstr(token[j], L"\\")
			 || wcsstr(token[j], L"'")
			) {
				token[j] = NULL;
				break;
			}
		}

		if (token[0]) {	// �g�[�N����1�ȏ゠��
			int nFuncId = -1;
			wchar_t* entry_token = NULL;

			length = wcslen(token[0]);
			if (length >= 2
				&& token[0][length - 1] == L':'
			) {	// ���x��
				token[0][length - 1] = L'\0';
				nFuncId = 51;
				entry_token = token[0];
			}else if (token[1]) {	// �g�[�N����2�ȏ゠��
				if (wcsicmp(token[1], L"proc") == 0) {	// �֐�
					nFuncId = 50;
					entry_token = token[0];
				}else if (wcsicmp(token[1], L"endp") == 0) {	// �֐��I��
					nFuncId = 52;
					entry_token = token[0];
				//}else
				//if (my_stricmp(token[1], _T("macro")) == 0) {	// �}�N��
				//	nFuncId = -1;
				//	entry_token = token[0];
				//}else
				//if (my_stricmp(token[1], _T("struc")) == 0) {	// �\����
				//	nFuncId = -1;
				//	entry_token = token[0];
				}
			}

			if (nFuncId >= 0) {
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
				pFuncInfoArr->AppendData(nLineCount + 1, ptPos.y + 1, entry_token, nFuncId);
			}
		}

		free(pTmpLine);
	}

	return;
}

