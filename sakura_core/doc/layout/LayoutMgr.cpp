/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, YAZAKI, genta, aroka
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "doc/EditDoc.h"
#include "doc/DocReader.h" // for _DEBUG
#include "doc/DocEditor.h"
#include "doc/logic/DocLine.h"/// 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/Memory.h"/// 2002/2/10 aroka
#include "mem/MemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "SearchAgent.h"
#include "debug/RunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	getIndentOffset(&LayoutMgr::getIndentOffset_Normal)	// Oct. 1, 2002 genta	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
{
	pDocLineMgr = nullptr;
	pTypeConfig = nullptr;
	nMaxLineKetas = LayoutInt(MAXLINEKETAS);
	nTabSpace = LayoutInt(4);
	pszKinsokuHead_1.clear();				// �s���֑�	//@@@ 2002.04.08 MIK
	pszKinsokuTail_1.clear();				// �s���֑�	//@@@ 2002.04.08 MIK
	pszKinsokuKuto_1.clear();				// ��Ǔ_�Ԃ炳��	//@@@ 2002.04.17 MIK

	nTextWidth = LayoutInt(0);			// �e�L�X�g�ő啝�̋L��		// 2009.08.28 nasukoji
	nTextWidthMaxLine = LayoutInt(0);	// �ő啝�̃��C�A�E�g�s		// 2009.08.28 nasukoji

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	pszKinsokuHead_1.clear();	// �s���֑�
	pszKinsokuTail_1.clear();	// �s���֑�			//@@@ 2002.04.08 MIK
	pszKinsokuKuto_1.clear();	// ��Ǔ_�Ԃ炳��	//@@@ 2002.04.17 MIK
}


/*
||
|| �s�f�[�^�Ǘ��N���X�̃|�C���^�����������܂�
||
*/
void LayoutMgr::Create(
	EditDoc* pEditDoc,
	DocLineMgr* pDocLineMgr
	)
{
	_Empty();
	Init();
	// Jun. 20, 2003 genta EditDoc�ւ̃|�C���^�ǉ�
	this->pEditDoc = pEditDoc;
	this->pDocLineMgr = pDocLineMgr;
}


void LayoutMgr::Init()
{
	pLayoutTop = nullptr;
	pLayoutBot = nullptr;
	nPrevReferLine = LayoutInt(0);
	pLayoutPrevRefer = nullptr;
	nLines = LayoutInt(0);			// �S�����s��
	nLineTypeBot = COLORIDX_DEFAULT;

	// EOF���C�A�E�g�ʒu�L��	// 2006.10.07 Moca
	nEOFLine = LayoutInt(-1);
	nEOFColumn = LayoutInt(-1);
}


void LayoutMgr::_Empty()
{
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		delete pLayout;
		pLayout = pLayoutNext;
	}
}


/*! ���C�A�E�g���̕ύX
	@param bDoLayout [in] ���C�A�E�g���̍č쐬
	@param refType [in] �^�C�v�ʐݒ�
*/
void LayoutMgr::SetLayoutInfo(
	bool				bDoLayout,
	const TypeConfig&	refType,
	LayoutInt			nTabSpace,
	LayoutInt			nMaxLineKetas
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && nTabSpace == refType.nTabSpace) || bDoLayout);

	// �^�C�v�ʐݒ�
	pTypeConfig = &refType;
	nMaxLineKetas = nMaxLineKetas;
	nTabSpace = nTabSpace;

	// Oct. 1, 2002 genta �^�C�v�ɂ���ď����֐���ύX����
	// ���������Ă�����e�[�u���ɂ��ׂ�
	switch (refType.nIndentLayout) {	// �܂�Ԃ���2�s�ڈȍ~���������\��	//@@@ 2002.09.29 YAZAKI
	case 1:
		// Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
		getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// ��Ǔ_�Ԃ牺������	// 2009.08.07 ryoji
	// refType.szKinsokuKuto �� pszKinsokuKuto_1
	pszKinsokuKuto_1.clear();
	if (refType.bKinsokuKuto) {	// 2009.08.06 ryoji bKinsokuKuto�ŐU�蕪����(Fix)
		for (const wchar_t* p=refType.szKinsokuKuto; *p; ++p) {
			pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
	// refType.szKinsokuHead �� (��Ǔ_�ȊO) pszKinsokuHead_1
	pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.szKinsokuHead; *p; ++p) {
		if (pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
	// refType.szKinsokuTail �� pszKinsokuTail_1
	pszKinsokuTail_1.clear();
	for (const wchar_t* p=refType.szKinsokuTail; *p; ++p) {
		pszKinsokuTail_1.push_back_unique(*p);
	}

	// ���C�A�E�g
	if (bDoLayout) {
		_DoLayout();
	}
}


/*!
	@brief �w�肳�ꂽ�����s�̃��C�A�E�g�����擾

	@param nLineNum [in] �����s�ԍ� (0�`)
*/
const Layout* LayoutMgr::SearchLineByLayoutY(
	LayoutInt nLineLayout
	) const
{
	LayoutInt nLineNum = nLineLayout;

	Layout*	pLayout;
	LayoutInt	nCount;
	if (nLines == LayoutInt(0)) {
		return nullptr;
	}

	// Mar. 19, 2003 Moca nLineNum�����̏ꍇ�̃`�F�b�N��ǉ�
	if (LayoutInt(0) > nLineNum || nLineNum >= nLines) {
		if (LayoutInt(0) > nLineNum) {
			DEBUG_TRACE(_T("LayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum);
		}
		return nullptr;
	}
//	// +++++++ �ᑬ�� +++++++++
//	if (nLineNum < (nLines / 2)) {
//		nCount = 0;
//		pLayout = pLayoutTop;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				pLayoutPrevRefer = pLayout;
//				nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetNextLayout();
//			++nCount;
//		}
//	}else {
//		nCount = nLines - 1;
//		pLayout = pLayoutBot;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				pLayoutPrevRefer = pLayout;
//				nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetPrevLayout();
//			--nCount;
//		}
//	}


	// +++++++�킸���ɍ�����+++++++
	// 2004.03.28 Moca pLayoutPrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	LayoutInt nPrevToLineNumDiff = t_abs(nPrevReferLine - nLineNum);
	if (0
		|| !pLayoutPrevRefer
		|| nLineNum < nPrevToLineNumDiff
		|| nLines - nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (nLines / 2)) {
			nCount = LayoutInt(0);
			pLayout = pLayoutTop;
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = nLines - LayoutInt(1);
			pLayout = pLayoutBot;
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}else {
		if (nLineNum == nPrevReferLine) {
			return pLayoutPrevRefer;
		}else if (nLineNum > nPrevReferLine) {
			nCount = nPrevReferLine + LayoutInt(1);
			pLayout = pLayoutPrevRefer->GetNextLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = nPrevReferLine - LayoutInt(1);
			pLayout = pLayoutPrevRefer->GetPrevLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}
	return nullptr;
}


//@@@ 2002.09.23 YAZAKI Layout*���쐬����Ƃ���͕������āAInsertLineNext()�Ƌ��ʉ�
void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (nLines == LayoutInt(0)) {
		pLayoutBot = pLayoutTop = pLayout;
		pLayoutTop->pPrev = nullptr;
	}else {
		pLayoutBot->pNext = pLayout;
		pLayout->pPrev = pLayoutBot;
		pLayoutBot = pLayout;
	}
	pLayout->pNext = nullptr;
	++nLines;
	return;
}

//@@@ 2002.09.23 YAZAKI Layout*���쐬����Ƃ���͕������āAAddLineBottom()�Ƌ��ʉ�
Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (nLines == LayoutInt(0)) {
		// ��
		pLayoutBot = pLayoutTop = pLayout;
		pLayoutTop->pPrev = nullptr;
		pLayoutTop->pNext = nullptr;
	}else if (!pLayoutPrev) {
		// �擪�ɑ}��
		pLayoutTop->pPrev = pLayout;
		pLayout->pPrev = nullptr;
		pLayout->pNext = pLayoutTop;
		pLayoutTop = pLayout;
	}else
	if (!pLayoutPrev->GetNextLayout()) {
		// �Ō�ɑ}��
		pLayoutBot->pNext = pLayout;
		pLayout->pPrev = pLayoutBot;
		pLayout->pNext = nullptr;
		pLayoutBot = pLayout;
	}else {
		// �r���ɑ}��
		Layout* pLayoutNext = pLayoutPrev->GetNextLayout();
		pLayoutPrev->pNext = pLayout;
		pLayoutNext->pPrev = pLayout;
		pLayout->pPrev = pLayoutPrev;
		pLayout->pNext = pLayoutNext;
	}
	++nLines;
	return pLayout;
}

/* Layout���쐬����
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	���C�A�E�g���������ɒǉ�
*/
Layout* LayoutMgr::CreateLayout(
	DocLine*		pDocLine,
	LogicPoint		ptLogicPos,
	LogicInt		nLength,
	EColorIndexType	nTypePrev,
	LayoutInt		nIndent,
	LayoutInt		nPosX,
	LayoutColorInfo*	colorInfo
	)
{
	Layout* pLayout = new Layout(
		pDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent,
		colorInfo
	);

	if (pDocLine->GetEol() == EolType::None) {
		pLayout->eol.SetType(EolType::None);	// ���s�R�[�h�̎��
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pDocLine->GetLengthWithEOL() - pDocLine->GetEol().GetLen()
		) {
			pLayout->eol = pDocLine->GetEol();	// ���s�R�[�h�̎��
		}else {
			pLayout->eol = EolType::None;	// ���s�R�[�h�̎��
		}
	}

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̂݃��C�A�E�g�����L������
	// �u�܂�Ԃ��Ȃ��v�ȊO�Ōv�Z���Ȃ��̂̓p�t�H�[�}���X�ቺ��h���ړI�Ȃ̂ŁA
	// �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�S�Ă̐܂�Ԃ����@�Ōv�Z����
	// �悤�ɂ��Ă��ǂ��Ǝv���B
	// �i���̏ꍇLayoutMgr::CalculateTextWidth()�̌Ăяo���ӏ����`�F�b�N�j
	pLayout->SetLayoutWidth((pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) ? nPosX : LayoutInt(0));

	return pLayout;
}


/*
|| �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver0

	@date 2002/2/10 aroka CMemory�ύX
*/
const wchar_t* LayoutMgr::GetLineStr(
	LayoutInt nLine,
	LogicInt* pnLineLen
	) const //#####�������
{
	const Layout* pLayout;
	if (!(pLayout = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = LogicInt(pLayout->GetLengthWithEOL());
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	�w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver1
	@date 2002/03/24 YAZAKI GetLineStr(int nLine, int* pnLineLen)�Ɠ�������ɕύX�B
*/
const wchar_t* LayoutMgr::GetLineStr(
	LayoutInt nLine,
	LogicInt* pnLineLen,
	const Layout** ppcLayoutDes
	) const
{
	if (!((*ppcLayoutDes) = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->GetLengthWithEOL();
	return (*ppcLayoutDes)->pDocLine->GetPtr() + (*ppcLayoutDes)->GetLogicOffset();
}

/*
|| �w�肳�ꂽ�ʒu�����C�A�E�g�s�̓r���̍s�����ǂ������ׂ�

	@date 2002/4/27 MIK
*/
bool LayoutMgr::IsEndOfLine(
	const LayoutPoint& ptLinePos
	)
{
	const Layout* pLayout;

	if (!(pLayout = SearchLineByLayoutY(ptLinePos.GetY2()))) {
		return false;
	}

	if (pLayout->GetLayoutEol().GetType() == EolType::None) {
		// ���̍s�ɉ��s�͂Ȃ�
		// ���̍s�̍Ōォ�H
		if (ptLinePos.x == (Int)pLayout->GetLengthWithEOL()) {
			return true; //$$ �P�ʍ���
		}
	}

	return false;
}

/*!	@brief �t�@�C�������̃��C�A�E�g�ʒu���擾����

	�t�@�C�������܂őI������ꍇ�ɐ��m�Ȉʒu����^���邽��

	�����̊֐��ł͕����s���烌�C�A�E�g�ʒu��ϊ�����K�v������C
	�����ɖ��ʂ��������߁C��p�֐����쐬
	
	@date 2006.07.29 genta
	@date 2006.10.01 Moca �����o�ŕێ�����悤�ɁB�f�[�^�ύX���ɂ́A_DoLayout/DoLayout_Range�Ŗ����ɂ���B
*/
void LayoutMgr::GetEndLayoutPos(
	LayoutPoint* ptLayoutEnd // [out]
	)
{
	if (nEOFLine != -1) {
		ptLayoutEnd->x = nEOFColumn;
		ptLayoutEnd->y = nEOFLine;
		return;
	}

	if (nLines == LayoutInt(0) || !pLayoutBot) {
		// �f�[�^����
		ptLayoutEnd->x = LayoutInt(0);
		ptLayoutEnd->y = LayoutInt(0);
		nEOFColumn = ptLayoutEnd->x;
		nEOFLine = ptLayoutEnd->y;
		return;
	}

	Layout* btm = pLayoutBot;
	if (btm->eol != EolType::None) {
		// �����ɉ��s������
		ptLayoutEnd->Set(LayoutInt(0), GetLineCount());
	}else {
		MemoryIterator it(btm, GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set(it.getColumn(), GetLineCount() - LayoutInt(1));
		// [EOF]�̂ݐ܂�Ԃ��̂͂�߂�	// 2009.02.17 ryoji
		//// 2006.10.01 Moca Start [EOF]�݂̂̃��C�A�E�g�s�����������Ă����o�O���C��
		//if (GetMaxLineKetas() <= ptLayoutEnd->GetX2()) {
		//	ptLayoutEnd->SetX(LayoutInt(0));
		//	ptLayoutEnd->y++;
		//}
		//// 2006.10.01 Moca End
	}
	nEOFColumn = ptLayoutEnd->x;
	nEOFLine = ptLayoutEnd->y;
}


// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜����
// �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ�
Layout* LayoutMgr::DeleteLayoutAsLogical(
	Layout*	pLayoutInThisArea,
	LayoutInt	nLineOf_pLayoutInThisArea,
	LogicInt	nLineFrom,
	LogicInt	nLineTo,
	LogicPoint	ptDelLogicalFrom,
	LayoutInt*	pnDeleteLines
	)
{
	*pnDeleteLines = LayoutInt(0);
	if (nLines == LayoutInt(0)) {	// �S�����s��
		return nullptr;
	}
	if (!pLayoutInThisArea) {
		return nullptr;
	}

	// 1999.11.22
	pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	nPrevReferLine = nLineOf_pLayoutInThisArea - LayoutInt(1);

	// �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�`
	Layout* pLayoutWork = pLayoutInThisArea->GetPrevLayout();
	while (pLayoutWork && nLineFrom <= pLayoutWork->GetLogicLineNo()) {
		pLayoutWork = pLayoutWork->GetPrevLayout();
	}

	Layout* pLayout = pLayoutWork ? pLayoutWork->GetNextLayout() : pLayoutTop;
	while (pLayout) {
		if (pLayout->GetLogicLineNo() > nLineTo) {
			break;
		}
		Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutWork) {
			// �擪�s�̏���
			pLayoutTop = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->pNext->pPrev = nullptr;
			}
		}else {
			pLayoutWork->pNext = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->pNext->pPrev = pLayoutWork;
			}
		}
//		if (pLayoutPrevRefer == pLayout) {
//			// 1999.12.22 �O�ɂ��炷�����ł悢�̂ł�
//			pLayoutPrevRefer = pLayout->GetPrevLayout();
//			--nPrevReferLine;
//		}

		if (0
			|| (1
				&& ptDelLogicalFrom.GetY2() == pLayout->GetLogicLineNo()
				&& ptDelLogicalFrom.GetX2() < pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL()
			)
			|| (ptDelLogicalFrom.GetY2() < pLayout->GetLogicLineNo())
		) {
			(*pnDeleteLines)++;
		}

		if (pLayoutPrevRefer == pLayout) {
			DEBUG_TRACE(_T("�o�O�o�O\n"));
		}

		delete pLayout;

		--nLines;	// �S�����s��
		if (!pLayoutNext) {
			pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE(_T("(*pnDeleteLines)=%d\n"), (*pnDeleteLines));

	return pLayoutWork;
}


// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����
// �_���s���폜���ꂽ�ꍇ�͂O��菬�����s��
// �_���s���}�����ꂽ�ꍇ�͂O���傫���s��
void LayoutMgr::ShiftLogicalLineNum(
	Layout* pLayoutPrev,
	LogicInt nShiftLines
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::ShiftLogicalLineNum");

	if (nShiftLines == 0) {
		return;
	}
	Layout* pLayout = pLayoutPrev ? pLayoutPrev->GetNextLayout() : pLayoutTop;
	// ���C�A�E�g���S�̂��X�V����(�ȂȁA�Ȃ��!!!)
	while (pLayout) {
		pLayout->OffsetLogicLineNo(nShiftLines);	// �Ή�����_���s�ԍ�
		pLayout = pLayout->GetNextLayout();
	}
	return;
}

bool LayoutMgr::ChangeLayoutParam(
	LayoutInt	nTabSize,
	LayoutInt	nMaxLineKetas
	)
{
	if (nTabSize < 1 || nTabSize > 64) {
		return false;
	}
	if (nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS) {
		return false;
	}

	nTabSpace = nTabSize;
	nMaxLineKetas = nMaxLineKetas;

	_DoLayout();

	return true;
}


// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
bool LayoutMgr::WhereCurrentWord(
	LayoutInt		nLineNum,
	LogicInt		nIdx,
	LayoutRange*	pSelect,		// [out]
	NativeW*		pcmcmWord,		// [out]
	NativeW*		pcmcmWordLeft	// [out]
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ� -> ���W�b�N�P��pSelect, pMemWord, pMemWordLeft
	LogicInt nFromX;
	LogicInt nToX;
	bool nRetCode = SearchAgent(*pDocLineMgr).WhereCurrentWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + LogicInt(nIdx),
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if (nRetCode) {
		// �_���ʒu�����C�A�E�g�ʒu�ϊ�
		LayoutPoint ptFrom;
		LogicToLayout(LogicPoint(nFromX, pLayout->GetLogicLineNo()), &ptFrom, nLineNum);
		pSelect->SetFrom(ptFrom);

		LayoutPoint ptTo;
		LogicToLayout(LogicPoint(nToX, pLayout->GetLogicLineNo()), &ptTo, nLineNum);
		pSelect->SetTo(ptTo);
	}
	return nRetCode;

}


// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
int LayoutMgr::PrevOrNextWord(
	LayoutInt		nLineNum,
	LogicInt		nIdx,
	LayoutPoint*	pptLayoutNew,
	bool			bLeft,
	bool			bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return FALSE;
	}

	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	LogicInt nPosNew;
	int nRetCode = SearchAgent(*pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLeft,
		bStopsBothEnds
	);

	if (nRetCode) {
		// �_���ʒu�����C�A�E�g�ʒu�ϊ�
		LogicToLayout(
			LogicPoint(nPosNew, pLayout->GetLogicLineNo()),
			pptLayoutNew,
			nLineNum
		);
	}
	return nRetCode;
}


// �P�ꌟ��
/*
	@retval 0 ������Ȃ�
*/
int LayoutMgr::SearchWord(
	LayoutInt				nLine,				// [in] �����J�n���C�A�E�g�s
	LogicInt				nIdx,				// [in] �����J�n�f�[�^�ʒu
	SearchDirection			searchDirection,	// [in] ��������
	LayoutRange*			pMatchRange,		// [out] �}�b�`���C�A�E�g�͈�
	const SearchStringPattern&	pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return FALSE;
	}

	// �P�ꌟ�� -> logicRange (�f�[�^�ʒu)
	LogicRange logicRange;
	int nRetCode = SearchAgent(*pDocLineMgr).SearchWord(
		LogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&logicRange, //pMatchRange,
		pattern
	);

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	// logicRange -> pMatchRange
	if (nRetCode) {
		LogicToLayout(
			logicRange,
			pMatchRange
		);
	}
	return nRetCode;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �P�ʂ̕ϊ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief �J�[�\���ʒu�ϊ� ���������C�A�E�g

	�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	�����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)

	@date 2004.06.16 Moca �C���f���g�\���̍ۂ�TAB���܂ލs�̍��W����C��
	@date 2007.09.06 kobake �֐�����CaretPos_Phys2Log����LogicToLayout�ɕύX
*/
void LayoutMgr::LogicToLayout(
	const LogicPoint&	ptLogic,	// [in]  ���W�b�N�ʒu
	LayoutPoint*		pptLayout,	// [out] ���C�A�E�g�ʒu
	LayoutInt			nLineHint	// [in]  ���C�A�E�gY�l�̃q���g�B���߂�l�ɋ߂��l��n���ƍ����Ɍ����ł���B
	)
{
	pptLayout->Clear();

	if (GetLineCount() == 0) {
		return; // �ϊ��s��
	}
	// �T�[�`�J�n�n�_ -> pLayout, nCaretPosX, nCaretPosY
	LayoutInt		nCaretPosX = LayoutInt(0);
	LayoutInt		nCaretPosY;
	const Layout*	pLayout;
	// 2013.05.15 �q���g�A����Ȃ��̏����𓝍�
	{
		nLineHint = t_min(GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max(LayoutInt(ptLogic.y), nLineHint);

		// 2013.05.12 pLayoutPrevRefer������
		if (1
			&& nCaretPosY <= nPrevReferLine
			&& pLayoutPrevRefer
			&& pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// �q���g��茻�݈ʒu�̂ق�����납�������炢�ŋ߂�
			nCaretPosY = LayoutInt(ptLogic.y - pLayoutPrevRefer->GetLogicLineNo()) + nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			pptLayout->SetY(nLines);
			return;
		}
		
		// ���W�b�NY���ł�������ꍇ�́A��v����܂Ńf�N�������g (
		while (pLayout->GetLogicLineNo() > ptLogic.GetY2()) {
			pLayout = pLayout->GetPrevLayout();
			--nCaretPosY;
		}

		// ���W�b�NY��������Offset���s���߂��Ă���ꍇ�͖߂�
		if (pLayout->GetLogicLineNo() == ptLogic.GetY2()) {
			while (1
				&& pLayout->GetPrevLayout()
				&& pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()
				&& ptLogic.x < pLayout->GetLogicOffset()
			) {
				pLayout = pLayout->GetPrevLayout();
				--nCaretPosY;
			}
		}
	}

	// Layout���P����ɐi�߂Ȃ���ptLogic.y�������s�Ɉ�v����Layout��T��
	do {
		if (ptLogic.GetY2() == pLayout->GetLogicLineNo()) {
			// 2013.05.10 Moca ������
			const Layout* pLayoutNext = pLayout->GetNextLayout();
			if (1
				&& pLayoutNext
				&& ptLogic.GetY2() == pLayoutNext->GetLogicLineNo()
				&& pLayoutNext->GetLogicOffset() <= ptLogic.x
			) {
				++nCaretPosY;
				pLayout = pLayout->GetNextLayout();
				continue;
			}

			// 2004.06.16 Moca �C���f���g�\���̍ۂɈʒu�������(TAB�ʒu����ɂ��)
			// TAB���𐳊m�Ɍv�Z����ɂ͓�������C���f���g���������Ă����K�v������D
			nCaretPosX = pLayout->GetIndent();
			const wchar_t*	pData;
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset(); // 2002/2/10 aroka CMemory�ύX
			LogicInt	nDataLen = (LogicInt)pLayout->GetLengthWithEOL();

			LogicInt i;
			for (i=LogicInt(0); i<nDataLen; ++i) {
				if (pLayout->GetLogicOffset() + i >= ptLogic.x) {
					break;
				}

				// �������W�b�N�� -> nCharChars
				LogicInt nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
				if (nCharChars == 0) {
					nCharChars = LogicInt(1);
				}
				
				// �������C�A�E�g�� -> nCharKetas
				LayoutInt nCharKetas;
				if (pData[i] == WCODE::TAB) {
					// Sep. 23, 2002 genta �����o�[�֐����g���悤��
					nCharKetas = GetActualTabSpace(nCaretPosX);
				}else {
					nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
				}
//				if (nCharKetas == 0)				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//					nCharKetas = LayoutInt(1);

				// ���C�A�E�g���Z
				nCaretPosX += nCharKetas;

				// ���W�b�N���Z
				if (pData[i] == WCODE::TAB) {
					nCharChars = LogicInt(1);
				}
				i += nCharChars - LogicInt(1);
			}
			if (i < nDataLen) {
				// ptLogic.x, ptLogic.y�����̍s�̒��Ɍ��������烋�[�v�ł��؂�
				break;
			}

			if (!pLayout->GetNextLayout()) {
				// ���Y�ʒu�ɒB���Ă��Ȃ��Ă��C���C�A�E�g�����Ȃ�f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + LayoutInt(pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0);
				break;
			}

			if (ptLogic.y < pLayout->pNext->GetLogicLineNo()) {
				// ����Layout�����Y�����s���߂��Ă��܂��ꍇ�̓f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + LayoutInt(pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0);
				break;
			}
		}
		if (ptLogic.GetY2() < pLayout->GetLogicLineNo()) {
			// �ӂ��͂����ɂ͗��Ȃ��Ǝv����... (genta)
			// Layout�̎w�������s���T���Ă���s������w���Ă�����ł��؂�
			break;
		}

		// ���̍s�֐i��
		++nCaretPosY;
		pLayout = pLayout->GetNextLayout();
	} while (pLayout);

	// 2004.06.16 Moca �C���f���g�\���̍ۂ̈ʒu����C��
	pptLayout->Set(
		pLayout ? nCaretPosX : LayoutInt(0),
		nCaretPosY
	);
}

/*!
	@brief �J�[�\���ʒu�ϊ�  ���C�A�E�g������

	���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	�������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)

	@date 2007.09.06 kobake �֐�����CaretPos_Log2Phys��LayoutToLogic�ɕύX
*/
void LayoutMgr::LayoutToLogicEx(
	const LayoutPoint&	ptLayout,	// [in]  ���C�A�E�g�ʒu
	LogicPointEx*		pptLogic	// [out] ���W�b�N�ʒu
	) const
{
	pptLogic->Set(LogicInt(0), LogicInt(0));
	pptLogic->ext = 0;
	if (ptLayout.GetY2() > nLines) {
		// 2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, nLines);
		pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
		return;
	}

	LogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �x�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bEOF = false;
	LayoutInt nX;
	const Layout* pLayout = SearchLineByLayoutY(ptLayout.GetY2());
	if (!pLayout) {
		if (0 < ptLayout.y) {
			pLayout = SearchLineByLayoutY(ptLayout.GetY2() - LayoutInt(1));
			if (!pLayout) {
				pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
				return;
			}else {
				pData = GetLineStr(ptLayout.GetY2() - LayoutInt(1), &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
					pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
					return;
				}else {
					pptLogic->y = pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr�ύX
					bEOF = true;
					// nX = LayoutInt(MAXLINEKETAS);
					nX = pLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		// 2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, nLines);
		pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
		return;
	}else {
		pptLogic->y = pLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �w�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.GetY2(), &nDataLen);
	nX = pLayout ? pLayout->GetIndent() : LayoutInt(0);

checkloop:;
	LogicInt i;
	for (i=LogicInt(0); i<nDataLen; ++i) {
		// �������W�b�N�� -> nCharChars
		LogicInt	nCharChars;
		nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
		if (nCharChars == 0)
			nCharChars = LogicInt(1);
		
		// �������C�A�E�g�� -> nCharKetas
		LayoutInt	nCharKetas;
		if (pData[i] == WCODE::TAB) {
			nCharKetas = GetActualTabSpace(nX);
		}else {
			nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
		}
//		if (nCharKetas == 0)				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//			nCharKetas = LayoutInt(1);

		// ���C�A�E�g���Z
		if (nX + nCharKetas > ptLayout.GetX2() && !bEOF) {
			break;
		}
		nX += nCharKetas;

		// ���W�b�N���Z
		if (pData[i] == WCODE::TAB) {
			nCharChars = LogicInt(1);
		}
		i += nCharChars - LogicInt(1);
	}
	i += pLayout->GetLogicOffset();
	pptLogic->x = i;
	pptLogic->ext = ptLayout.GetX2() - nX;
	return;
}


void LayoutMgr::LayoutToLogic(
	const LayoutPoint& ptLayout,
	LogicPoint* pptLogic
	) const
{
	LogicPointEx ptEx;
	LayoutToLogicEx(ptLayout, &ptEx);
	*pptLogic = ptEx;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �e�X�g�p�Ƀ��C�A�E�g�����_���v
void LayoutMgr::DUMP()
{
#ifdef _DEBUG
	LogicInt nDataLen;
	MYTRACE(_T("------------------------\n"));
	MYTRACE(_T("nLines=%d\n"), nLines);
	MYTRACE(_T("pLayoutTop=%08lxh\n"), pLayoutTop);
	MYTRACE(_T("pLayoutBot=%08lxh\n"), pLayoutBot);
	MYTRACE(_T("nMaxLineKetas=%d\n"), nMaxLineKetas);

	MYTRACE(_T("nTabSpace=%d\n"), nTabSpace);
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"),		pLayout);
		MYTRACE(_T("\tpPrev =%08lxh\n"),	pLayout->GetPrevLayout());
		MYTRACE(_T("\tpNext =%08lxh\n"),	pLayout->GetNextLayout());
		MYTRACE(_T("\tnLinePhysical=%d\n"),	pLayout->GetLogicLineNo());
		MYTRACE(_T("\tnOffset=%d\n"),		pLayout->GetLogicOffset());
		MYTRACE(_T("\tnLength=%d\n"),		pLayout->GetLengthWithEOL());
		MYTRACE(_T("\tenumEOLType =%ls\n"),	pLayout->GetLayoutEol().GetName());
		MYTRACE(_T("\tnEOLLen =%d\n"),		pLayout->GetLayoutEol().GetLen());
		MYTRACE(_T("\tnTypePrev=%d\n"),		pLayout->GetColorTypePrev());
		const wchar_t* pData = DocReader(*pDocLineMgr).GetLineStr(pLayout->GetLogicLineNo(), &nDataLen);
		MYTRACE(_T("\t[%ls]\n"), pData);
		pLayout = pLayoutNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif
	return;
}

