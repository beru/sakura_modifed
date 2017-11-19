#include "StdAfx.h"
#include "Layout.h"
#include "LayoutMgr.h"
#include "charset/charcode.h"
#include "extmodule/Bregexp.h" // LayoutMgr�̒�`�ŕK�v

Layout::~Layout()
{
	return;
}

void Layout::DUMP(void)
{
	DEBUG_TRACE(_T("\n\n��Layout::DUMP()======================\n"));
	DEBUG_TRACE(_T("ptLogicPos.y=%d\t\t�Ή�����_���s�ԍ�\n"), ptLogicPos.y);
	DEBUG_TRACE(_T("ptLogicPos.x=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n"), ptLogicPos.x);
	DEBUG_TRACE(_T("nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n"), (int)nLength);
	DEBUG_TRACE(_T("nTypePrev=%d\t\t�^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� \n"), nTypePrev);
	DEBUG_TRACE(_T("======================\n"));
	return;
}

// ���C�A�E�g�����v�Z�B���s�͊܂܂Ȃ��B
size_t Layout::CalcLayoutWidth(const LayoutMgr& layoutMgr) const
{
	// �\�[�X
	const wchar_t* pText = pDocLine->GetPtr();
	size_t nTextLen = pDocLine->GetLengthWithoutEOL();

	// �v�Z
	size_t nWidth = GetIndent();
	for (int i=ptLogicPos.x; i<ptLogicPos.x+(int)nLength; ++i) {
		if (pText[i] == WCODE::TAB) {
			nWidth += layoutMgr.GetActualTabSpace(nWidth);
		}else {
			nWidth += NativeW::GetKetaOfChar(pText, nTextLen, i);
		}
	}
	return nWidth;
}

// �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾
int Layout::CalcLayoutOffset(
	const LayoutMgr& layoutMgr,
	int nStartPos,
	int nStartOffset) const
{
	int nRet = nStartOffset;
	if (this->GetLogicOffset()) {
		const wchar_t* pLine = this->pDocLine->GetPtr();
		size_t nLineLen = this->pDocLine->GetLengthWithEOL();
		const int nOffset = GetLogicOffset();
		for (int i=nStartPos; i<nOffset; ++i) {
			if (pLine[i] == WCODE::TAB) {
				nRet += layoutMgr.GetActualTabSpace(nRet);
			}else {
				nRet += NativeW::GetKetaOfChar(pLine, nLineLen, i);
			}
		}
	}
	return nRet;
}

