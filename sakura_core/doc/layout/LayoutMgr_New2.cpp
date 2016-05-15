/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "LayoutMgr.h"
#include "Layout.h" // 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h" // 2002/2/10 aroka
#include "charset/charcode.h"
#include "SearchAgent.h"


// ������u��
void LayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg* pArg
	)
{
	LayoutInt nWork_nLines = nLines;	// �ύX�O�̑S�s���̕ۑ�	@@@ 2002.04.19 MIK

	// �u���擪�ʒu�̃��C�A�E�g���
	EColorIndexType	nCurrentLineType = COLORIDX_DEFAULT;
	LayoutColorInfo* colorInfo = nullptr;
	LayoutInt nLineWork = pArg->delRange.GetFrom().GetY2();

	Layout* pLayoutWork = SearchLineByLayoutY(pArg->delRange.GetFrom().GetY2());
	if (pLayoutWork) {
		while (pLayoutWork->GetLogicOffset() != 0) {
			pLayoutWork = pLayoutWork->GetPrevLayout();
			--nLineWork;
		}
		nCurrentLineType = pLayoutWork->GetColorTypePrev();
		colorInfo = pLayoutWork->GetLayoutExInfo()->DetachColorInfo();
	}else if (GetLineCount() == pArg->delRange.GetFrom().GetY2()) {
		// 2012.01.05 �ŏI�s��Redo/Undo�ł̐F�������������Ȃ��̂��C��
		nCurrentLineType = nLineTypeBot;
		colorInfo = layoutExInfoBot.DetachColorInfo();
	}

	/*
	||  �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu) ��
	||  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	LogicPoint ptFrom;
	LogicPoint ptTo;
	LayoutToLogic(pArg->delRange.GetFrom(), &ptFrom);
	LayoutToLogic(pArg->delRange.GetTo(), &ptTo);

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	DocLineReplaceArg dlra;
	dlra.delRange.SetFrom(ptFrom);	// �폜�͈�from
	dlra.delRange.SetTo(ptTo);		// �폜�͈�to
	dlra.pMemDeleted = pArg->pMemDeleted;	// �폜���ꂽ�f�[�^��ۑ�
	dlra.pInsData = pArg->pInsData;			// �}������f�[�^
	dlra.nDelSeq = pArg->nDelSeq;
	SearchAgent(*pDocLineMgr).ReplaceData(
		&dlra
	);
	pArg->nInsSeq = dlra.nInsSeq;

	// --- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---
	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜����
	// �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ�

	LayoutInt	nModifyLayoutLinesOld = LayoutInt(0);
	Layout* pLayoutPrev;
	LogicInt nWork = t_max(dlra.nDeletedLineNum, dlra.nInsLineNum);

	if (pLayoutWork) {
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			ptFrom.GetY2(),
			ptFrom.GetY2() + nWork,
			ptFrom,
			&nModifyLayoutLinesOld
		);

		// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����
		// �_���s���폜���ꂽ�ꍇ�͂O��菬�����s��
		// �_���s���}�����ꂽ�ꍇ�͂O���傫���s��
		if (dlra.nInsLineNum - dlra.nDeletedLineNum != 0) {
			ShiftLogicalLineNum(
				pLayoutPrev,
				dlra.nInsLineNum - dlra.nDeletedLineNum
			);
		}
	}else {
		pLayoutPrev = pLayoutBot;
	}

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	LogicInt nRowNum;
	if (!pLayoutPrev) {
		if (!pLayoutTop) {
			nRowNum = pDocLineMgr->GetLineCount();
		}else {
			nRowNum = pLayoutTop->GetLogicLineNo();
		}
	}else {
		if (!pLayoutPrev->GetNextLayout()) {
			nRowNum =
				pDocLineMgr->GetLineCount() -
				pLayoutPrev->GetLogicLineNo() - LogicInt(1);
		}else {
			nRowNum =
				pLayoutPrev->pNext->GetLogicLineNo() -
				pLayoutPrev->GetLogicLineNo() - LogicInt(1);
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout     = pArg->delRange.GetFrom();		// �ҏW�J�n�ʒu
	ctwArg.nDelLines    = pArg->delRange.GetTo().GetY2() - pArg->delRange.GetFrom().GetY2();	// �폜�s�� - 1
	ctwArg.nAllLinesOld = nWork_nLines;								// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = (pArg->pInsData && pArg->pInsData->size());			// �ǉ�������̗L��

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	LayoutInt nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		ptFrom,
		nCurrentLineType,
		colorInfo,
		ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = nLines - nWork_nLines;	// �ύX��̑S�s���Ƃ̍���	@@@ 2002.04.19 MIK
	if (pArg->nAddLineNum == 0) {
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	// �ĕ`��q���g ���C�A�E�g�s�̑���
	}
	pArg->nModLineFrom = pArg->delRange.GetFrom().GetY2();	// �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom
	pArg->nModLineTo += (pArg->nModLineFrom - LayoutInt(1)) ;	// �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo

	// 2007.10.18 kobake LayoutReplaceArg::ptLayoutNew�͂����ŎZ�o����̂�������
	LogicToLayout(dlra.ptNewPos, &pArg->ptLayoutNew); // �}�����ꂽ�����̎��̈ʒu
}

