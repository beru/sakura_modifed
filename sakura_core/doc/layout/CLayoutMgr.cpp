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
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/CEditDoc.h"
#include "doc/CDocReader.h" // for _DEBUG
#include "doc/CDocEditor.h"
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "CSearchAgent.h"
#include "debug/CRunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	m_getIndentOffset(&LayoutMgr::getIndentOffset_Normal)	// Oct. 1, 2002 genta	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
{
	m_pDocLineMgr = NULL;
	m_pTypeConfig = NULL;
	m_nMaxLineKetas = LayoutInt(MAXLINEKETAS);
	m_nTabSpace = LayoutInt(4);
	m_pszKinsokuHead_1.clear();				// �s���֑�	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();				// �s���֑�	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();				// ��Ǔ_�Ԃ炳��	//@@@ 2002.04.17 MIK

	m_nTextWidth = LayoutInt(0);			// �e�L�X�g�ő啝�̋L��		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = LayoutInt(0);	// �ő啝�̃��C�A�E�g�s		// 2009.08.28 nasukoji

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	m_pszKinsokuHead_1.clear();	// �s���֑�
	m_pszKinsokuTail_1.clear();	// �s���֑�			//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();	// ��Ǔ_�Ԃ炳��	//@@@ 2002.04.17 MIK
}


/*
||
|| �s�f�[�^�Ǘ��N���X�̃|�C���^�����������܂�
||
*/
void LayoutMgr::Create(
	EditDoc* pcEditDoc,
	DocLineMgr* pcDocLineMgr
	)
{
	_Empty();
	Init();
	// Jun. 20, 2003 genta EditDoc�ւ̃|�C���^�ǉ�
	m_pEditDoc = pcEditDoc;
	m_pDocLineMgr = pcDocLineMgr;
}


void LayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = LayoutInt(0);
	m_pLayoutPrevRefer = NULL;
	m_nLines = LayoutInt(0);			// �S�����s��
	m_nLineTypeBot = COLORIDX_DEFAULT;

	// EOF���C�A�E�g�ʒu�L��	// 2006.10.07 Moca
	m_nEOFLine = LayoutInt(-1);
	m_nEOFColumn = LayoutInt(-1);
}


void LayoutMgr::_Empty()
{
	Layout* pLayout = m_pLayoutTop;
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
	MY_RUNNINGTIMER(cRunningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && m_nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && m_nTabSpace == refType.m_nTabSpace) || bDoLayout);

	// �^�C�v�ʐݒ�
	m_pTypeConfig = &refType;
	m_nMaxLineKetas = nMaxLineKetas;
	m_nTabSpace = nTabSpace;

	// Oct. 1, 2002 genta �^�C�v�ɂ���ď����֐���ύX����
	// ���������Ă�����e�[�u���ɂ��ׂ�
	switch (refType.m_nIndentLayout) {	// �܂�Ԃ���2�s�ڈȍ~���������\��	//@@@ 2002.09.29 YAZAKI
	case 1:
		// Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
		m_getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		m_getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		m_getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// ��Ǔ_�Ԃ牺������	// 2009.08.07 ryoji
	// refType.m_szKinsokuKuto �� m_pszKinsokuKuto_1
	m_pszKinsokuKuto_1.clear();
	if (refType.m_bKinsokuKuto) {	// 2009.08.06 ryoji m_bKinsokuKuto�ŐU�蕪����(Fix)
		for (const wchar_t* p=refType.m_szKinsokuKuto; *p; ++p) {
			m_pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
	// refType.m_szKinsokuHead �� (��Ǔ_�ȊO) m_pszKinsokuHead_1
	m_pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.m_szKinsokuHead; *p; ++p) {
		if (m_pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			m_pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
	// refType.m_szKinsokuTail �� m_pszKinsokuTail_1
	m_pszKinsokuTail_1.clear();
	for (const wchar_t* p=refType.m_szKinsokuTail; *p; ++p) {
		m_pszKinsokuTail_1.push_back_unique(*p);
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
	if (m_nLines == LayoutInt(0)) {
		return NULL;
	}

	// Mar. 19, 2003 Moca nLineNum�����̏ꍇ�̃`�F�b�N��ǉ�
	if (LayoutInt(0) > nLineNum || nLineNum >= m_nLines) {
		if (LayoutInt(0) > nLineNum) {
			DEBUG_TRACE(_T("LayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum);
		}
		return NULL;
	}
//	// +++++++ �ᑬ�� +++++++++
//	if (nLineNum < (m_nLines / 2)) {
//		nCount = 0;
//		pLayout = m_pLayoutTop;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetNextLayout();
//			++nCount;
//		}
//	}else {
//		nCount = m_nLines - 1;
//		pLayout = m_pLayoutBot;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetPrevLayout();
//			--nCount;
//		}
//	}


	// +++++++�킸���ɍ�����+++++++
	// 2004.03.28 Moca m_pLayoutPrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	LayoutInt nPrevToLineNumDiff = t_abs(m_nPrevReferLine - nLineNum);
	if (0
		|| !m_pLayoutPrevRefer
		|| nLineNum < nPrevToLineNumDiff
		|| m_nLines - nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (m_nLines / 2)) {
			nCount = LayoutInt(0);
			pLayout = m_pLayoutTop;
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = m_nLines - LayoutInt(1);
			pLayout = m_pLayoutBot;
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}else {
		if (nLineNum == m_nPrevReferLine) {
			return m_pLayoutPrevRefer;
		}else if (nLineNum > m_nPrevReferLine) {
			nCount = m_nPrevReferLine + LayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetNextLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = m_nPrevReferLine - LayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetPrevLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}
	return NULL;
}


//@@@ 2002.09.23 YAZAKI Layout*���쐬����Ƃ���͕������āAInsertLineNext()�Ƌ��ʉ�
void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (m_nLines == LayoutInt(0)) {
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
	}else {
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		m_pLayoutBot = pLayout;
	}
	pLayout->m_pNext = NULL;
	++m_nLines;
	return;
}

//@@@ 2002.09.23 YAZAKI Layout*���쐬����Ƃ���͕������āAAddLineBottom()�Ƌ��ʉ�
Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (m_nLines == LayoutInt(0)) {
		// ��
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}else if (!pLayoutPrev) {
		// �擪�ɑ}��
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if (!pLayoutPrev->GetNextLayout()) {
		// �Ō�ɑ}��
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else {
		// �r���ɑ}��
		Layout* pLayoutNext = pLayoutPrev->GetNextLayout();
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
	++m_nLines;
	return pLayout;
}

/* Layout���쐬����
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	���C�A�E�g���������ɒǉ�
*/
Layout* LayoutMgr::CreateLayout(
	DocLine*		pCDocLine,
	LogicPoint		ptLogicPos,
	LogicInt		nLength,
	EColorIndexType	nTypePrev,
	LayoutInt		nIndent,
	LayoutInt		nPosX,
	LayoutColorInfo*	colorInfo
	)
{
	Layout* pLayout = new Layout(
		pCDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent,
		colorInfo
	);

	if (pCDocLine->GetEol() == EolType::None) {
		pLayout->m_eol.SetType(EolType::None);	// ���s�R�[�h�̎��
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pCDocLine->GetLengthWithEOL() - pCDocLine->GetEol().GetLen()
		) {
			pLayout->m_eol = pCDocLine->GetEol();	// ���s�R�[�h�̎��
		}else {
			pLayout->m_eol = EolType::None;	// ���s�R�[�h�̎��
		}
	}

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̂݃��C�A�E�g�����L������
	// �u�܂�Ԃ��Ȃ��v�ȊO�Ōv�Z���Ȃ��̂̓p�t�H�[�}���X�ቺ��h���ړI�Ȃ̂ŁA
	// �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�S�Ă̐܂�Ԃ����@�Ōv�Z����
	// �悤�ɂ��Ă��ǂ��Ǝv���B
	// �i���̏ꍇLayoutMgr::CalculateTextWidth()�̌Ăяo���ӏ����`�F�b�N�j
	pLayout->SetLayoutWidth((m_pEditDoc->m_nTextWrapMethodCur == (int)TextWrappingMethod::NoWrapping) ? nPosX : LayoutInt(0));

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
	return (*ppcLayoutDes)->m_pDocLine->GetPtr() + (*ppcLayoutDes)->GetLogicOffset();
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
	if (m_nEOFLine != -1) {
		ptLayoutEnd->x = m_nEOFColumn;
		ptLayoutEnd->y = m_nEOFLine;
		return;
	}

	if (m_nLines == LayoutInt(0) || !m_pLayoutBot) {
		// �f�[�^����
		ptLayoutEnd->x = LayoutInt(0);
		ptLayoutEnd->y = LayoutInt(0);
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	Layout* btm = m_pLayoutBot;
	if (btm->m_eol != EolType::None) {
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
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
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
	if (m_nLines == LayoutInt(0)) {	// �S�����s��
		return NULL;
	}
	if (!pLayoutInThisArea) {
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - LayoutInt(1);

	// �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�`
	Layout* pLayoutWork = pLayoutInThisArea->GetPrevLayout();
	while (pLayoutWork && nLineFrom <= pLayoutWork->GetLogicLineNo()) {
		pLayoutWork = pLayoutWork->GetPrevLayout();
	}

	Layout* pLayout = pLayoutWork ? pLayoutWork->GetNextLayout() : m_pLayoutTop;
	while (pLayout) {
		if (pLayout->GetLogicLineNo() > nLineTo) {
			break;
		}
		Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutWork) {
			// �擪�s�̏���
			m_pLayoutTop = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else {
			pLayoutWork->m_pNext = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if (m_pLayoutPrevRefer == pLayout) {
//			// 1999.12.22 �O�ɂ��炷�����ł悢�̂ł�
//			m_pLayoutPrevRefer = pLayout->GetPrevLayout();
//			--m_nPrevReferLine;
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

		if (m_pLayoutPrevRefer == pLayout) {
			DEBUG_TRACE(_T("�o�O�o�O\n"));
		}

		delete pLayout;

		--m_nLines;	// �S�����s��
		if (!pLayoutNext) {
			m_pLayoutBot = pLayoutWork;
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
	MY_RUNNINGTIMER(cRunningTimer, "LayoutMgr::ShiftLogicalLineNum");

	if (nShiftLines == 0) {
		return;
	}
	Layout* pLayout = pLayoutPrev ? pLayoutPrev->GetNextLayout() : m_pLayoutTop;
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

	m_nTabSpace = nTabSize;
	m_nMaxLineKetas = nMaxLineKetas;

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

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ� -> ���W�b�N�P��pSelect, pcmemWord, pcmemWordLeft
	LogicInt nFromX;
	LogicInt nToX;
	bool nRetCode = SearchAgent(m_pDocLineMgr).WhereCurrentWord(
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
	bool			bLEFT,
	bool			bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return FALSE;
	}

	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	LogicInt nPosNew;
	int nRetCode = SearchAgent(m_pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLEFT,
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
	SearchDirection		searchDirection,	// [in] ��������
	LayoutRange*			pMatchRange,		// [out] �}�b�`���C�A�E�g�͈�
	const SearchStringPattern&	pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return FALSE;
	}

	// �P�ꌟ�� -> cLogicRange (�f�[�^�ʒu)
	LogicRange cLogicRange;
	int nRetCode = SearchAgent(m_pDocLineMgr).SearchWord(
		LogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&cLogicRange, //pMatchRange,
		pattern
	);

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	// cLogicRange -> pMatchRange
	if (nRetCode) {
		LogicToLayout(
			cLogicRange,
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

		// 2013.05.12 m_pLayoutPrevRefer������
		if (1
			&& nCaretPosY <= m_nPrevReferLine
			&& m_pLayoutPrevRefer
			&& m_pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// �q���g��茻�݈ʒu�̂ق�����납�������炢�ŋ߂�
			nCaretPosY = LayoutInt(ptLogic.y - m_pLayoutPrevRefer->GetLogicLineNo()) + m_nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			pptLayout->SetY(m_nLines);
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

			if (ptLogic.y < pLayout->m_pNext->GetLogicLineNo()) {
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
	if (ptLayout.GetY2() > m_nLines) {
		// 2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
		return;
	}

	LogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �x�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	BOOL bEOF = FALSE;
	LayoutInt nX;
	const Layout* pcLayout = SearchLineByLayoutY(ptLayout.GetY2());
	if (!pcLayout) {
		if (0 < ptLayout.y) {
			pcLayout = SearchLineByLayoutY(ptLayout.GetY2() - LayoutInt(1));
			if (!pcLayout) {
				pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
				return;
			}else {
				pData = GetLineStr(ptLayout.GetY2() - LayoutInt(1), &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().m_common.m_edit.m_bEnableExtEol)) {
					pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
					return;
				}else {
					pptLogic->y = m_pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr�ύX
					bEOF = TRUE;
					// nX = LayoutInt(MAXLINEKETAS);
					nX = pcLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		// 2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
		return;
	}else {
		pptLogic->y = pcLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �w�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.GetY2(), &nDataLen);
	nX = pcLayout ? pcLayout->GetIndent() : LayoutInt(0);

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
	i += pcLayout->GetLogicOffset();
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
	MYTRACE(_T("m_nLines=%d\n"), m_nLines);
	MYTRACE(_T("m_pLayoutTop=%08lxh\n"), m_pLayoutTop);
	MYTRACE(_T("m_pLayoutBot=%08lxh\n"), m_pLayoutBot);
	MYTRACE(_T("m_nMaxLineKetas=%d\n"), m_nMaxLineKetas);

	MYTRACE(_T("m_nTabSpace=%d\n"), m_nTabSpace);
	Layout* pLayout = m_pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"), pLayout);
		MYTRACE(_T("\tm_pPrev =%08lxh\n"),		pLayout->GetPrevLayout());
		MYTRACE(_T("\tm_pNext =%08lxh\n"),		pLayout->GetNextLayout());
		MYTRACE(_T("\tm_nLinePhysical=%d\n"),	pLayout->GetLogicLineNo());
		MYTRACE(_T("\tm_nOffset=%d\n"),		pLayout->GetLogicOffset());
		MYTRACE(_T("\tm_nLength=%d\n"),		pLayout->GetLengthWithEOL());
		MYTRACE(_T("\tm_enumEOLType =%ls\n"),	pLayout->GetLayoutEol().GetName());
		MYTRACE(_T("\tm_nEOLLen =%d\n"),		pLayout->GetLayoutEol().GetLen());
		MYTRACE(_T("\tm_nTypePrev=%d\n"),		pLayout->GetColorTypePrev());
		const wchar_t* pData = DocReader(*m_pDocLineMgr).GetLineStr(pLayout->GetLogicLineNo(), &nDataLen);
		MYTRACE(_T("\t[%ls]\n"), pData);
		pLayout = pLayoutNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif
	return;
}

