#include "StdAfx.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"
#include "doc/EditDoc.h"
#include "doc/DocReader.h"
#include "doc/DocEditor.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "charset/charcode.h"
#include "mem/Memory.h"
#include "mem/MemoryIterator.h"
#include "basis/SakuraBasis.h"
#include "SearchAgent.h"
#include "debug/RunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	getIndentOffset(&LayoutMgr::getIndentOffset_Normal)
{
	pDocLineMgr = nullptr;
	pTypeConfig = nullptr;
	nMaxLineKetas = MAXLINEKETAS;
	nTabSpace = 4;
	pszKinsokuHead_1.clear();				// �s���֑�
	pszKinsokuTail_1.clear();				// �s���֑�
	pszKinsokuKuto_1.clear();				// ��Ǔ_�Ԃ炳��

	nTextWidth = 0;			// �e�L�X�g�ő啝�̋L��
	nTextWidthMaxLine = 0;	// �ő啝�̃��C�A�E�g�s

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	pszKinsokuHead_1.clear();	// �s���֑�
	pszKinsokuTail_1.clear();	// �s���֑�
	pszKinsokuKuto_1.clear();	// ��Ǔ_�Ԃ炳��
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
	nPrevReferLine = 0;
	pLayoutPrevRefer = nullptr;
	nLines = 0;			// �S�����s��
	nLineTypeBot = COLORIDX_DEFAULT;

	// EOF���C�A�E�g�ʒu�L��
	nEOFLine = -1;
	nEOFColumn = -1;
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
	size_t				nTabSpace,
	size_t				nMaxLineKetas
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && nTabSpace == refType.nTabSpace) || bDoLayout);

	// �^�C�v�ʐݒ�
	pTypeConfig = &refType;
	nMaxLineKetas = nMaxLineKetas;
	nTabSpace = nTabSpace;

	// �^�C�v�ɂ���ď����֐���ύX����
	// ���������Ă�����e�[�u���ɂ��ׂ�
	switch (refType.nIndentLayout) {	// �܂�Ԃ���2�s�ڈȍ~���������\��
	case 1:
		getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// ��Ǔ_�Ԃ牺������
	pszKinsokuKuto_1.clear();
	if (refType.bKinsokuKuto) {	// bKinsokuKuto�ŐU�蕪����(Fix)
		for (const wchar_t* p=refType.szKinsokuKuto; *p; ++p) {
			pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
	pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.szKinsokuHead; *p; ++p) {
		if (pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// �s���֑�����
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
	size_t nLineLayout
	) const
{
	if (nLines == 0) {
		return nullptr;
	}
	size_t nLineNum = nLineLayout;
	Layout*	pLayout;
	size_t nCount;

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
	// pLayoutPrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	int nPrevToLineNumDiff = t_abs(nPrevReferLine - (int)nLineNum);
	if (0
		|| !pLayoutPrevRefer
		|| (int)nLineNum < nPrevToLineNumDiff
		|| (int)nLines - (int)nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (nLines / 2)) {
			nCount = 0;
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
			nCount = nLines - 1;
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
		}else if ((int)nLineNum > nPrevReferLine) {
			nCount = nPrevReferLine + 1;
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
			nCount = nPrevReferLine - 1;
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


void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (nLines == 0) {
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

Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (nLines == 0) {
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

/* Layout���쐬���� */
Layout* LayoutMgr::CreateLayout(
	DocLine*		pDocLine,
	Point			ptLogicPos,
	int				nLength,
	EColorIndexType	nTypePrev,
	int				nIndent,
	int				nPosX,
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
	auto& eol = pDocLine->GetEol();
	if (eol == EolType::None) {
		pLayout->eol.SetType(EolType::None);	// ���s�R�[�h�̎��
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pDocLine->GetLengthWithEOL() - eol.GetLen()
		) {
			pLayout->eol = eol;	// ���s�R�[�h�̎��
		}else {
			pLayout->eol = EolType::None;	// ���s�R�[�h�̎��
		}
	}

	// �u�܂�Ԃ��Ȃ��v�I�����̂݃��C�A�E�g�����L������
	// �u�܂�Ԃ��Ȃ��v�ȊO�Ōv�Z���Ȃ��̂̓p�t�H�[�}���X�ቺ��h���ړI�Ȃ̂ŁA
	// �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�S�Ă̐܂�Ԃ����@�Ōv�Z����
	// �悤�ɂ��Ă��ǂ��Ǝv���B
	// �i���̏ꍇLayoutMgr::CalculateTextWidth()�̌Ăяo���ӏ����`�F�b�N�j
	pLayout->SetLayoutWidth((pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) ? nPosX : 0);

	return pLayout;
}


/*
|| �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver0
*/
const wchar_t* LayoutMgr::GetLineStr(
	size_t nLine,
	size_t* pnLineLen
	) const //#####�������
{
	const Layout* pLayout;
	if (!(pLayout = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = pLayout->GetLengthWithEOL();
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	�w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver1 */
const wchar_t* LayoutMgr::GetLineStr(
	size_t nLine,
	size_t* pnLineLen,
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
*/
bool LayoutMgr::IsEndOfLine(
	const Point& ptLinePos
	)
{
	const Layout* pLayout;

	if (!(pLayout = SearchLineByLayoutY(ptLinePos.y))) {
		return false;
	}

	if (pLayout->GetLayoutEol().GetType() == EolType::None) {
		// ���̍s�ɉ��s�͂Ȃ�
		// ���̍s�̍Ōォ�H
		if (ptLinePos.x == pLayout->GetLengthWithEOL()) {
			return true; //$$ �P�ʍ���
		}
	}

	return false;
}

/*!	@brief �t�@�C�������̃��C�A�E�g�ʒu���擾����

	�t�@�C�������܂őI������ꍇ�ɐ��m�Ȉʒu����^���邽��

	�����̊֐��ł͕����s���烌�C�A�E�g�ʒu��ϊ�����K�v������C
	�����ɖ��ʂ��������߁C��p�֐����쐬
*/
void LayoutMgr::GetEndLayoutPos(
	Point* ptLayoutEnd // [out]
	)
{
	if (nEOFLine != -1) {
		ptLayoutEnd->x = nEOFColumn;
		ptLayoutEnd->y = nEOFLine;
		return;
	}

	if (nLines == 0 || !pLayoutBot) {
		// �f�[�^����
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = 0;
		nEOFColumn = 0;
		nEOFLine = 0;
		return;
	}

	Layout* btm = pLayoutBot;
	if (btm->eol != EolType::None) {
		// �����ɉ��s������
		ptLayoutEnd->Set(0, (int)GetLineCount());
	}else {
		MemoryIterator it(btm, GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set((int)it.getColumn(), (int)GetLineCount() - 1);
	}
	nEOFColumn = ptLayoutEnd->x;
	nEOFLine = ptLayoutEnd->y;
}


// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜����
// �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ�
Layout* LayoutMgr::DeleteLayoutAsLogical(
	Layout*	pLayoutInThisArea,
	int			nLineOf_pLayoutInThisArea,
	int			nLineFrom,
	int			nLineTo,
	Point		ptDelLogicalFrom,
	size_t*		pnDeleteLines
	)
{
	*pnDeleteLines = 0;
	if (nLines == 0) {	// �S�����s��
		return nullptr;
	}
	if (!pLayoutInThisArea) {
		return nullptr;
	}

	pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	nPrevReferLine = nLineOf_pLayoutInThisArea - 1;

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
		if (0
			|| (1
				&& ptDelLogicalFrom.y == pLayout->GetLogicLineNo()
				&& ptDelLogicalFrom.x < pLayout->GetLogicOffset() + (int)pLayout->GetLengthWithEOL()
			)
			|| (ptDelLogicalFrom.y < pLayout->GetLogicLineNo())
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
	int nShiftLines
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
	size_t nTabSize,
	size_t nMaxLineKetas
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
	size_t		nLineNum,
	size_t		nIdx,
	Range*		pSelect,		// [out]
	NativeW*	pcmcmWord,		// [out]
	NativeW*	pcmcmWordLeft	// [out]
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ� -> ���W�b�N�P��pSelect, pMemWord, pMemWordLeft
	size_t nFromX;
	size_t nToX;
	bool ret = SearchAgent(*pDocLineMgr).WhereCurrentWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if (ret) {
		// �_���ʒu�����C�A�E�g�ʒu�ϊ�
		Point ptFrom = LogicToLayout(Point((int)nFromX, pLayout->GetLogicLineNo()), nLineNum);
		Point ptTo = LogicToLayout(Point((int)nToX, pLayout->GetLogicLineNo()), nLineNum);
		pSelect->SetFrom(ptFrom);
		pSelect->SetTo(ptTo);
	}
	return ret;

}


// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
bool LayoutMgr::PrevOrNextWord(
	size_t	nLineNum,
	size_t	nIdx,
	Point*	pptLayoutNew,
	bool	bLeft,
	bool	bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	size_t nPosNew;
	bool ret = SearchAgent(*pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLeft,
		bStopsBothEnds
	);

	if (ret) {
		// �_���ʒu�����C�A�E�g�ʒu�ϊ�
		*pptLayoutNew = LogicToLayout(
			Point((int)nPosNew, pLayout->GetLogicLineNo()),
			nLineNum
		);
	}
	return ret;
}


// �P�ꌟ��
/*
	@retval 0 ������Ȃ�
*/
bool LayoutMgr::SearchWord(
	size_t nLine,						// [in] �����J�n���C�A�E�g�s
	size_t nIdx,						// [in] �����J�n�f�[�^�ʒu
	SearchDirection searchDirection,	// [in] ��������
	Range* pMatchRange,					// [out] �}�b�`���C�A�E�g�͈�
	const SearchStringPattern& pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return false;
	}

	// �P�ꌟ�� -> logicRange (�f�[�^�ʒu)
	Range logicRange;
	bool ret = SearchAgent(*pDocLineMgr).SearchWord(
		Point(pLayout->GetLogicOffset() + (int)nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&logicRange, //pMatchRange,
		pattern
	);

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	// logicRange -> pMatchRange
	if (ret) {
		LogicToLayout(
			logicRange,
			pMatchRange
		);
	}
	return ret;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �P�ʂ̕ϊ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief �J�[�\���ʒu�ϊ� ���������C�A�E�g

	�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	�����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
*/
Point LayoutMgr::LogicToLayout(
	const Point&	ptLogic,	// [in]  ���W�b�N�ʒu
	int				nLineHint	// [in]  ���C�A�E�gY�l�̃q���g�B���߂�l�ɋ߂��l��n���ƍ����Ɍ����ł���B
	)
{
	Point ptLayout;	// ���C�A�E�g�ʒu
	if (GetLineCount() == 0) {
		return ptLayout; // �ϊ��s��
	}
	// �T�[�`�J�n�n�_ -> pLayout, nCaretPosX, nCaretPosY
	size_t nCaretPosX = 0;
	size_t nCaretPosY;
	const Layout* pLayout;
	{
		nLineHint = t_min((int)GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max<int>(ptLogic.y, nLineHint);

		// pLayoutPrevRefer������
		if (1
			&& (int)nCaretPosY <= nPrevReferLine
			&& pLayoutPrevRefer
			&& pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// �q���g��茻�݈ʒu�̂ق�����납�������炢�ŋ߂�
			nCaretPosY = ptLogic.y - pLayoutPrevRefer->GetLogicLineNo() + nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			ptLayout.SetY((int)nLines);
			return ptLayout;
		}
		
		// ���W�b�NY���ł�������ꍇ�́A��v����܂Ńf�N�������g (
		while (pLayout && pLayout->GetLogicLineNo() > ptLogic.y) {
			pLayout = pLayout->GetPrevLayout();
			--nCaretPosY;
		}

		// ���W�b�NY��������Offset���s���߂��Ă���ꍇ�͖߂�
		if (pLayout->GetLogicLineNo() == ptLogic.y) {
			while (1
				&& pLayout->GetPrevLayout()
				&& pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.y
				&& ptLogic.x < pLayout->GetLogicOffset()
			) {
				pLayout = pLayout->GetPrevLayout();
				--nCaretPosY;
			}
		}
	}

	// Layout���P����ɐi�߂Ȃ���ptLogic.y�������s�Ɉ�v����Layout��T��
	do {
		if (ptLogic.y == pLayout->GetLogicLineNo()) {
			// 2013.05.10 Moca ������
			const Layout* pLayoutNext = pLayout->GetNextLayout();
			if (1
				&& pLayoutNext
				&& ptLogic.y == pLayoutNext->GetLogicLineNo()
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
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
			size_t nDataLen = pLayout->GetLengthWithEOL();

			size_t i;
			for (i=0; i<nDataLen; ++i) {
				if (pLayout->GetLogicOffset() + (int)i >= ptLogic.x) {
					break;
				}

				// �������W�b�N�� -> nCharChars
				size_t nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
				if (nCharChars == 0) {
					nCharChars = 1;
				}
				
				// �������C�A�E�g�� -> nCharKetas
				size_t nCharKetas;
				if (pData[i] == WCODE::TAB) {
					nCharKetas = GetActualTabSpace(nCaretPosX);
				}else {
					nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
				}
				// ���C�A�E�g���Z
				nCaretPosX += nCharKetas;

				// ���W�b�N���Z
				if (pData[i] == WCODE::TAB) {
					nCharChars = 1;
				}
				i += nCharChars - 1;
			}
			if (i < nDataLen) {
				// ptLogic.x, ptLogic.y�����̍s�̒��Ɍ��������烋�[�v�ł��؂�
				break;
			}

			if (!pLayout->GetNextLayout()) {
				// ���Y�ʒu�ɒB���Ă��Ȃ��Ă��C���C�A�E�g�����Ȃ�f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + ((pLayout->GetLayoutEol().GetLen() > 0) ? 1 : 0);
				break;
			}

			if (ptLogic.y < pLayout->pNext->GetLogicLineNo()) {
				// ����Layout�����Y�����s���߂��Ă��܂��ꍇ�̓f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + ((pLayout->GetLayoutEol().GetLen() > 0) ? 1 : 0);
				break;
			}
		}
		if (ptLogic.y < pLayout->GetLogicLineNo()) {
			// �ӂ��͂����ɂ͗��Ȃ��Ǝv����... (genta)
			// Layout�̎w�������s���T���Ă���s������w���Ă�����ł��؂�
			break;
		}

		// ���̍s�֐i��
		++nCaretPosY;
		pLayout = pLayout->GetNextLayout();
	} while (pLayout);

	// 2004.06.16 Moca �C���f���g�\���̍ۂ̈ʒu����C��
	ptLayout.Set(
		pLayout ? (int)nCaretPosX : 0,
		(int)nCaretPosY
	);
	return ptLayout;
}

/*!
	@brief �J�[�\���ʒu�ϊ�  ���C�A�E�g������

	���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	�������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
*/
PointEx LayoutMgr::LayoutToLogicEx(
	const Point& ptLayout	// [in]  ���C�A�E�g�ʒu
	) const
{
	PointEx ptLogic;	// [out] ���W�b�N�ʒu
	ptLogic.ext = 0;
	if (ptLayout.y > (int)nLines) {
		ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
		return ptLogic;
	}

	size_t nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �x�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bEOF = false;
	size_t nX;
	const Layout* pLayout = SearchLineByLayoutY(ptLayout.y);
	if (!pLayout) {
		if (0 < ptLayout.y) {
			pLayout = SearchLineByLayoutY(ptLayout.y - 1);
			if (!pLayout) {
				ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
				return ptLogic;
			}else {
				pData = GetLineStr(ptLayout.y - 1, &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
					ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
					return ptLogic;
				}else {
					ptLogic.y = pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr�ύX
					bEOF = true;
					// nX = MAXLINEKETAS;
					nX = pLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
		return ptLogic;
	}else {
		ptLogic.y = pLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �w�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.y, &nDataLen);
	nX = pLayout ? pLayout->GetIndent() : 0;

checkloop:;
	size_t i;
	for (i=0; i<nDataLen; ++i) {
		// �������W�b�N�� -> nCharChars
		size_t nCharChars;
		nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
		if (nCharChars == 0) {
			nCharChars = 1;
		}
		
		// �������C�A�E�g�� -> nCharKetas
		size_t nCharKetas;
		if (pData[i] == WCODE::TAB) {
			nCharKetas = GetActualTabSpace(nX);
		}else {
			nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
		}

		// ���C�A�E�g���Z
		if ((int)(nX + nCharKetas) > ptLayout.x && !bEOF) {
			break;
		}
		nX += nCharKetas;

		// ���W�b�N���Z
		if (pData[i] == WCODE::TAB) {
			nCharChars = 1;
		}
		i += nCharChars - 1;
	}
	i += pLayout->GetLogicOffset();
	ptLogic.x = i;
	ptLogic.ext = ptLayout.x - nX;
	return ptLogic;
}


Point LayoutMgr::LayoutToLogic(
	const Point& ptLayout
	) const
{
	return LayoutToLogicEx(ptLayout);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �e�X�g�p�Ƀ��C�A�E�g�����_���v
void LayoutMgr::DUMP()
{
#ifdef _DEBUG
	size_t nDataLen;
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

