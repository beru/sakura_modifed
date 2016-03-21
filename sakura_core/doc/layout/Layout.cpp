/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
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
	DEBUG_TRACE(_T("m_ptLogicPos.y=%d\t\t�Ή�����_���s�ԍ�\n"), m_ptLogicPos.y);
	DEBUG_TRACE(_T("m_ptLogicPos.x=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n"), m_ptLogicPos.x);
	DEBUG_TRACE(_T("m_nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n"), (int)m_nLength);
	DEBUG_TRACE(_T("m_nTypePrev=%d\t\t�^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� \n"), m_nTypePrev);
	DEBUG_TRACE(_T("======================\n"));
	return;
}

// ���C�A�E�g�����v�Z�B���s�͊܂܂Ȃ��B
// 2007.10.11 kobake �쐬
// 2007.11.29 kobake �^�u�����v�Z����Ă��Ȃ������̂��C��
// 2011.12.26 Moca �C���f���g�͊܂ނ悤�ɕύX(���W�ϊ��o�O�C��)
LayoutInt Layout::CalcLayoutWidth(const LayoutMgr& layoutMgr) const
{
	// �\�[�X
	const wchar_t* pText    = m_pDocLine->GetPtr();
	LogicInt      nTextLen = m_pDocLine->GetLengthWithoutEOL();

	// �v�Z
	LayoutInt nWidth = GetIndent();
	for (LogicInt i=m_ptLogicPos.GetX2(); i<m_ptLogicPos.GetX2()+m_nLength; ++i) {
		if (pText[i] == WCODE::TAB) {
			nWidth += layoutMgr.GetActualTabSpace(nWidth);
		}else {
			nWidth += NativeW::GetKetaOfChar(pText, nTextLen, i);
		}
	}
	return nWidth;
}

// �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾�B2007.10.17 kobake
LayoutInt Layout::CalcLayoutOffset(
	const LayoutMgr& layoutMgr,
	LogicInt nStartPos,
	LayoutInt nStartOffset) const
{
	LayoutInt nRet = nStartOffset;
	if (this->GetLogicOffset()) {
		const wchar_t* pLine = this->m_pDocLine->GetPtr();
		int nLineLen = this->m_pDocLine->GetLengthWithEOL();
		const int nOffset = GetLogicOffset();
		for (int i=(Int)nStartPos; i<nOffset; ++i) {
			if (pLine[i] == WCODE::TAB) {
				nRet += layoutMgr.GetActualTabSpace(nRet);
			}else {
				nRet += NativeW::GetKetaOfChar(pLine, nLineLen, i);
			}
		}
	}
	return nRet;
}

