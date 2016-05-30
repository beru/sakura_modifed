/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "doc/EditDoc.h" /// 2003/07/20 genta
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/MemoryIterator.h"
#include "util/window.h"


/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool LayoutMgr::IsKinsokuHead(wchar_t wc)
{
	return pszKinsokuHead_1.exist(wc);
}

/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool LayoutMgr::IsKinsokuTail(wchar_t wc)
{
	return pszKinsokuTail_1.exist(wc);
}


/*!
	�֑��Ώۋ�Ǔ_�ɊY�����邩�𒲂ׂ�D

	@param [in] pLine  ���ׂ镶���ւ̃|�C���^
	@param [in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool LayoutMgr::IsKinsokuKuto(wchar_t wc)
{
	return pszKinsokuKuto_1.exist(wc);
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool LayoutMgr::IsKinsokuPosHead(
	size_t nRest,		// [in] �s�̎c�蕶����
	size_t nCharKetas,	// [in] ���݈ʒu�̕����T�C�Y
	size_t nCharKetas2	// [in] ���݈ʒu�̎��̕����T�C�Y
	)
{
	switch (nRest) {
	//    321012  ���}�W�b�N�i���o�[
	// 3 "��j" : 22 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "Z�j" : 12 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "��j": 22 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 2  "��)" : 21 ")"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z�j": 12 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z)" : 11 ")"�Ő܂�Ԃ��̂Ƃ�
	// ���������O���H
	// ���������A"��Z"�������֑��Ȃ珈�����Ȃ��B
	case 3:	// 3�����O
		if (nCharKetas == 2 && nCharKetas2 == 2) {
			return true;
		}
		break;
	case 2:	// 2�����O
		if (nCharKetas == 2) {
			return true;
		}else if (nCharKetas == 1 && nCharKetas2 == 2) {
			return true;
		}
		break;
	case 1:	// 1�����O
		if (nCharKetas == 1) {
			return true;
		}
		break;
	}
	return false;
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool LayoutMgr::IsKinsokuPosTail(
	size_t nRest,		// [in] �s�̎c�蕶����
	size_t nCharKetas,	// [in] ���݈ʒu�̕����T�C�Y
	size_t nCharKetas2	// [in] ���݈ʒu�̎��̕����T�C�Y
	)
{
	switch (nRest) {
	case 3:	// 3�����O
		if (nCharKetas == 2 && nCharKetas2 == 2) {
			// "�i��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 2:	// 2�����O
		if (nCharKetas == 2) {
			// "�i��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}else if (nCharKetas == 1 && nCharKetas2 == 2) {
			// "(��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 1:	// 1�����O
		if (nCharKetas == 1) {
			// "(��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	}
	return false;
}


/*!
	@brief �s�̒������v�Z���� (2�s�ڈȍ~�̎���������)
	
	���������s��Ȃ��̂ŁC���0��Ԃ��D
	�����͎g��Ȃ��D
	
	@return 1�s�̕\�������� (���0)
	
	@author genta
	@date 2002.10.01
*/
size_t LayoutMgr::getIndentOffset_Normal(Layout*)
{
	return 0;
}

/*!
	@brief �C���f���g�����v�Z���� (Tx2x)
	
	�O�̍s�̍Ō��TAB�̈ʒu���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author Yazaki
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI ���̕ύX, ����������
*/
size_t LayoutMgr::getIndentOffset_Tx2x(Layout* pLayoutPrev)
{
	// �O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if (!pLayoutPrev) {
		return 0;
	}
	size_t nIpos = pLayoutPrev->GetIndent();

	// �O�̍s���܂�Ԃ��s�Ȃ�΂���ɍ��킹��
	if (pLayoutPrev->GetLogicOffset() > 0) {
		return nIpos;
	}
	
	MemoryIterator it(pLayoutPrev, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB) {
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	// 2010.07.06 Moca TAB=8�Ȃǂ̏ꍇ�ɐ܂�Ԃ��Ɩ������[�v����s��̏C��. 6�Œ�� nTabSpace + 2�ɕύX
	if (GetMaxLineKetas() - nIpos < GetTabSpace() + 2) {
		nIpos = t_max((size_t)0, GetMaxLineKetas() - (GetTabSpace() + 2)); // 2013.05.12 Chg:0�������̂��ő啝�ɕύX
	}
	return nIpos;	// �C���f���g
}

/*!
	@brief �C���f���g�����v�Z���� (�X�y�[�X��������)
	
	�_���s�s���̃z���C�g�X�y�[�X�̏I���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author genta
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
*/
size_t LayoutMgr::getIndentOffset_LeftSpace(Layout* pLayoutPrev)
{
	// �O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if (!pLayoutPrev) {
		return 0;
	}
	// �C���f���g�̌v�Z
	size_t nIpos = pLayoutPrev->GetIndent();
	
	// Oct. 5, 2002 genta
	// �܂�Ԃ���3�s�ڈȍ~��1�O�̍s�̃C���f���g�ɍ��킹��D
	if (pLayoutPrev->GetLogicOffset() > 0) {
		return nIpos;
	}
	
	// 2002.10.07 YAZAKI �C���f���g�̌v�Z
	MemoryIterator it(pLayoutPrev, GetTabSpace());

	// Jul. 20, 2003 genta �����C���f���g�ɏ���������ɂ���
	bool bZenSpace = pTypeConfig->bAutoIndent_ZENSPACE;
	const wchar_t* szSpecialIndentChar = pTypeConfig->szIndentChars;
	while (!it.end()) {
		it.scanNext();
		if (it.getIndexDelta() == 1 && WCODE::IsIndentChar(it.getCurrentChar(), bZenSpace)) {
			// �C���f���g�̃J�E���g���p������
		// Jul. 20, 2003 genta �C���f���g�Ώە���
		}else if (szSpecialIndentChar[0] != L'\0') {
			wchar_t buf[3]; // �����̒�����1 or 2
			wmemcpy(buf, it.getCurrentPos(), it.getIndexDelta());
			buf[it.getIndexDelta()] = L'\0';
			if (wcsstr(szSpecialIndentChar, buf)) {
				// �C���f���g�̃J�E���g���p������
			}else {
				nIpos = it.getColumn();	// �I��
				break;
			}
		}else {
			nIpos = it.getColumn();	// �I��
			break;
		}
		it.addDelta();
	}
	if (it.end()) {
		nIpos = it.getColumn();	// �I��
	}
	// 2010.07.06 Moca TAB=8�Ȃǂ̏ꍇ�ɐ܂�Ԃ��Ɩ������[�v����s��̏C��. 6�Œ�� nTabSpace + 2�ɕύX
	if (GetMaxLineKetas() - nIpos < GetTabSpace() + 2) {
		nIpos = t_max((size_t)0, GetMaxLineKetas() - (GetTabSpace() + 2)); // 2013.05.12 Chg:0�������̂��ő啝�ɕύX
	}
	return nIpos;	// �C���f���g
}

/*!
	@brief  �e�L�X�g�ő啝���Z�o����

	�w�肳�ꂽ���C���𑖍����ăe�L�X�g�̍ő啝���쐬����B
	�S�č폜���ꂽ���͖��Z�o��Ԃɖ߂��B

	@param bCalLineLen	[in] �e���C�A�E�g�s�̒����̎Z�o���s��
	@param nStart		[in] �Z�o�J�n���C�A�E�g�s
	@param nEnd			[in] �Z�o�I�����C�A�E�g�s

	@retval TRUE �ő啝���ω�����
	@retval FALSE �ő啝���ω����Ȃ�����

	@note nStart, nEnd�������Ƃ�-1�̎��A�S���C���𑖍�����
		  �͈͂��w�肳��Ă���ꍇ�͍ő啝�̊g��̂݃`�F�b�N����

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
BOOL LayoutMgr::CalculateTextWidth(bool bCalLineLen, int nStart, int nEnd)
{
	bool bRet = false;
	bool bOnlyExpansion = true;		// �ő啝�̊g��݂̂��`�F�b�N����
	size_t nMaxLen = 0;
	int nMaxLineNum = 0;

	size_t nLines = GetLineCount();	// �e�L�X�g�̃��C�A�E�g�s��

	// �J�n�E�I���ʒu���ǂ�����w�肳��Ă��Ȃ�
	if (nStart < 0 && nEnd < 0) {
		bOnlyExpansion = false;		// �ő啝�̊g��E�k�����`�F�b�N����
	}
	if (nStart < 0) {				// �Z�o�J�n�s�̎w��Ȃ�
		nStart = 0;
	}else if (nStart > (int)nLines) {	// �͈̓I�[�o�[
		nStart = (int)nLines;
	}
	if (nEnd < 0 || nEnd >= (int)nLines) {	// �Z�o�I���s�̎w��Ȃ� �܂��� �����s���ȏ�
		nEnd = (int)nLines;
	}else {
		++nEnd;						// �Z�o�I���s�̎��s
	}
	Layout* pLayout;
	// �Z�o�J�n���C�A�E�g�s��T��
	// 2013.05.13 SearchLineByLayoutY���g��
	if (nStart == 0) {
		pLayout = pLayoutTop;
	}else {
		pLayout = SearchLineByLayoutY(nStart);
	}
#if 0
	if (nStart * 2 < nLines) {
		// �O������T�[�`
		int nCount = 0;
		pLayout = pLayoutTop;
		while (pLayout) {
			if (nStart == nCount) {
				break;
			}
			pLayout = pLayout->GetNextLayout();
			++nCount;
		}
	}else {
		// �������T�[�`
		int nCount = nLines - 1;
		pLayout = pLayoutBot;
		while (pLayout) {
			if (nStart == nCount) {
				break;
			}
			pLayout = pLayout->GetPrevLayout();
			nCount--;
		}
	}
#endif

	// ���C�A�E�g�s�̍ő啝�����o��
	for (int i=nStart; i<nEnd; ++i) {
		if (!pLayout) {
			break;
		}
		// ���C�A�E�g�s�̒������Z�o����
		if (bCalLineLen) {
			int nWidth = pLayout->CalcLayoutWidth(*this) + pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0;
			pLayout->SetLayoutWidth(nWidth);
		}

		// �ő啝���X�V
		if (nMaxLen < pLayout->GetLayoutWidth()) {
			nMaxLen = pLayout->GetLayoutWidth();
			nMaxLineNum = i;		// �ő啝�̃��C�A�E�g�s

			// �A�v���P�[�V�����̍ő啝�ƂȂ�����Z�o�͒�~
			if (nMaxLen >= MAXLINEKETAS && !bCalLineLen) {
				break;
			}
		}

		// ���̃��C�A�E�g�s�̃f�[�^
		pLayout = pLayout->GetNextLayout();
	}

	// �e�L�X�g�̕��̕ω����`�F�b�N
	if (nMaxLen) {
		// �ő啝���g�債�� �܂��� �ő啝�̊g��̂݃`�F�b�N�łȂ�
		if (nTextWidth < nMaxLen || !bOnlyExpansion) {
			nTextWidthMaxLine = nMaxLineNum;
			if (nTextWidth != nMaxLen) {	// �ő啝�ω�����
				nTextWidth = nMaxLen;
				bRet = true;
			}
		}
	}else if (nTextWidth && !nLines) {
		// �S�폜���ꂽ�畝�̋L�����N���A
		nTextWidthMaxLine = 0;
		nTextWidth = 0;
		bRet = true;
	}
	
	return bRet;
}

/*!
	@brief  �e�s�̃��C�A�E�g�s���̋L�����N���A����
	
	@note �܂�Ԃ����@���u�܂�Ԃ��Ȃ��v�ȊO�̎��́A�p�t�H�[�}���X�̒ቺ��
		  �h�~����ړI�Ŋe�s�̃��C�A�E�g�s��(nLayoutWidth)���v�Z���Ă��Ȃ��B
		  ��Ő݌v����l������Ďg�p���Ă��܂���������Ȃ��̂Łu�܂�Ԃ��Ȃ��v
		  �ȊO�̎��̓N���A���Ă����B
		  �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�A�S�Ă̐܂�Ԃ����@�Ōv�Z
		  ����悤�ɂ��Ă��ǂ��Ǝv���B

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
void LayoutMgr::ClearLayoutLineWidth(void)
{
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		pLayout->nLayoutWidth = 0;			// ���C�A�E�g�s�����N���A
		pLayout = pLayout->GetNextLayout();		// ���̃��C�A�E�g�s�̃f�[�^
	}
}

