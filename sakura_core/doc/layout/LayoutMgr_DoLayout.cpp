#include "StdAfx.h"
#include "doc/EditDoc.h" /// 2003/07/20 genta
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "doc/logic/DocLine.h"/// 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "view/colors/ColorStrategy.h"
#include "util/window.h"
#include "debug/RunningTimer.h"

// 2008.07.27 kobake
static
bool _GetKeywordLength(
	const StringRef&	lineStr,		// [in]
	uint32_t			nPos,			// [in]
	uint32_t*			p_nWordBgn,		// [out]
	uint32_t*			p_nWordLen,		// [out]
	uint32_t*			p_nWordKetas	// [out]
	)
{
	// �L�[���[�h�����J�E���g����
	size_t nWordBgn = nPos;
	size_t nWordLen = 0;
	size_t nWordKetas = 0;
	while (nPos < lineStr.GetLength() && IS_KEYWORD_CHAR(lineStr.At(nPos))) {
		size_t k = NativeW::GetKetaOfChar(lineStr, nPos);
		if (k == 0) {
			k = 1;
		}
		nWordLen += 1;
		nWordKetas += k;
		++nPos;
	}
	// ����
	if (nWordLen > 0) {
		*p_nWordBgn = nWordBgn;
		*p_nWordLen = nWordLen;
		*p_nWordKetas = nWordKetas;
		return true;
	}else {
		return false;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���i�X�e�[�^�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Layout* LayoutMgr::LayoutWork::_CreateLayout(LayoutMgr* mgr)
{
	return mgr->CreateLayout(
		this->pDocLine,
		Point(this->nBgn, this->nCurLine),
		this->nPos - this->nBgn,
		this->colorPrev,
		this->nIndent,
		this->nPosX,
		this->exInfoPrev.DetachColorInfo()
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���i                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool LayoutMgr::_DoKinsokuSkip(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (pWork->eKinsokuType != KinsokuType::None) {
		// �֑������̍Ō���ɒB������֑�����������������
		if (pWork->nPos >= pWork->nWordBgn + pWork->nWordLen) {
			if (1
				&& pWork->eKinsokuType == KinsokuType::Kuto
				&& pWork->nPos == pWork->nWordBgn + pWork->nWordLen
			) {
				int	nEol = pWork->pDocLine->GetEol().GetLen();

				// ���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
				if (
					!(1
						&& pTypeConfig->bKinsokuRet
						&& (pWork->nPos == pWork->lineStr.GetLength() - nEol)
						&& nEol
					)
				) {
				
					(this->*pfOnLine)(pWork);
				}
			}
			pWork->nWordLen = 0;
			pWork->eKinsokuType = KinsokuType::None;	//@@@ 2002.04.20 MIK
		}
		return true;
	}else {
		return false;
	}
}

void LayoutMgr::_DoWordWrap(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (pWork->eKinsokuType == KinsokuType::None) {
		// �p�P��̐擪��
		if (1
			&& pWork->nPos >= pWork->nBgn
			&& IS_KEYWORD_CHAR(pWork->lineStr.At(pWork->nPos))
		) {
			// �L�[���[�h�����擾
			uint32_t nWordKetas = 0;
			_GetKeywordLength(
				pWork->lineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->eKinsokuType = KinsokuType::WordWrap;	//@@@ 2002.04.20 MIK

			if (1
				&& pWork->nPosX + nWordKetas >= GetMaxLineKetas()
				&& pWork->nPos - pWork->nBgn > 0
			) {
				(this->*pfOnLine)(pWork);
			}
		}
	}
}

void LayoutMgr::_DoKutoBurasage(LayoutWork* pWork)
{
	if (1
		&& (GetMaxLineKetas() - pWork->nPosX < 2)
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {
		// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
		size_t nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		if (1
			&& IsKinsokuPosKuto(GetMaxLineKetas() - pWork->nPosX, nCharKetas)
			&& IsKinsokuKuto(pWork->lineStr.At(pWork->nPos))
		) {
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->eKinsokuType = KinsokuType::Kuto;
		}
	}
}

void LayoutMgr::_DoGyotoKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (1
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())	// 2007.02.17 ryoji �ǉ�
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// 2004.04.09 pWork->nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {
		// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
		size_t nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		size_t nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);

		if (1
			&& IsKinsokuPosHead(GetMaxLineKetas() - pWork->nPosX, nCharKetas2, nCharKetas3)
			&& IsKinsokuHead(pWork->lineStr.At(pWork->nPos + 1))
			&& ! IsKinsokuHead(pWork->lineStr.At(pWork->nPos))	// 1�����O���s���֑��łȂ�
			&& ! IsKinsokuKuto(pWork->lineStr.At(pWork->nPos))
		) {	// ��Ǔ_�łȂ�
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 2;
			pWork->eKinsokuType = KinsokuType::Head;

			(this->*pfOnLine)(pWork);
		}
	}
}

void LayoutMgr::_DoGyomatsuKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (1
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())	// 2007.02.17 ryoji �ǉ�
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// 2004.04.09 pWork->nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {	// �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����)
		int nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		int nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);

		if (1
			&& IsKinsokuPosTail(GetMaxLineKetas() - pWork->nPosX, nCharKetas2, nCharKetas3)
			&& IsKinsokuTail(pWork->lineStr.At(pWork->nPos))
		) {
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->eKinsokuType = KinsokuType::Tail;
			
			(this->*pfOnLine)(pWork);
		}
	}
}

// �܂�Ԃ��ꍇ��true��Ԃ�
bool LayoutMgr::_DoTab(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	// Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
	size_t nCharKetas = GetActualTabSpace(pWork->nPosX);
	if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
		(this->*pfOnLine)(pWork);
		return true;
	}
	pWork->nPosX += nCharKetas;
	pWork->nPos += 1;
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_MakeOneLine(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	size_t nEol = pWork->pDocLine->GetEol().GetLen(); //########���̂����s�v�ɂȂ�
	size_t nEol_1 = (nEol == 0) ? 0 : (nEol - 1);
	ASSERT_GE(pWork->lineStr.GetLength(), nEol_1);
	size_t nLength = pWork->lineStr.GetLength() - nEol_1;

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
	}
	auto& color = ColorStrategyPool::getInstance();

	// 1���W�b�N�s����������܂Ń��[�v
	while (pWork->nPos < nLength) {
		// �C���f���g����_OnLine�Ōv�Z�ς݂Ȃ̂ł�������͍폜

		// �֑��������Ȃ�X�L�b�v����	@@@ 2002.04.20 MIK
		if (_DoKinsokuSkip(pWork, pfOnLine)) {
		}else {
			// �p�����[�h���b�v������
			if (pTypeConfig->bWordWrap) {
				_DoWordWrap(pWork, pfOnLine);
			}

			// ��Ǔ_�̂Ԃ炳��
			if (pTypeConfig->bKinsokuKuto) {
				_DoKutoBurasage(pWork);
			}

			// �s���֑�
			if (pTypeConfig->bKinsokuHead) {
				_DoGyotoKinsoku(pWork, pfOnLine);
			}

			// �s���֑�
			if (pTypeConfig->bKinsokuTail) {
				_DoGyomatsuKinsoku(pWork, pfOnLine);
			}
		}

		//@@@ 2002.09.22 YAZAKI
		color.CheckColorMODE(&pWork->pColorStrategy, pWork->nPos, pWork->lineStr);

		if (pWork->lineStr.At(pWork->nPos) == WCODE::TAB) {
			if (_DoTab(pWork, pfOnLine)) {
				continue;
			}
		}else {
			if (pWork->nPos >= pWork->lineStr.GetLength()) {
				break;
			}
			// 2007.09.07 kobake   ���W�b�N���ƃ��C�A�E�g�������
			size_t nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
//			if (nCharKetas == 0) {				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//				nCharKetas = 1;
//			}

			if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
				if (pWork->eKinsokuType != KinsokuType::Kuto) {
					// ���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
					if (!(1
						&& pTypeConfig->bKinsokuRet
						&& (pWork->nPos == pWork->lineStr.GetLength() - nEol)
						&& nEol
						)
					) {
						(this->*pfOnLine)(pWork);
						continue;
					}
				}
			}
			pWork->nPos += 1;
			pWork->nPosX += nCharKetas;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �{����(�S��)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_OnLine1(LayoutWork* pWork)
{
	AddLineBottom(pWork->_CreateLayout(this));
	pWork->pLayout = pLayoutBot;
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	pWork->nPosX = pWork->nIndent = (this->*getIndentOffset)(pWork->pLayout);
}

/*!
	���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�

	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�Dm_nMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
*/
void LayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::_DoLayout");

	/*	�\�����X�ʒu
		2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	*/

	if (GetListenerCount() != 0) {
		NotifyProgress(0);
		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(NULL)) {
			return;
		}
	}

	_Empty();
	Init();
	
	// Nov. 16, 2002 genta
	// �܂�Ԃ��� <= TAB���̂Ƃ��������[�v����̂�����邽�߁C
	// TAB���܂�Ԃ����ȏ�̎���TAB=4�Ƃ��Ă��܂�
	// �܂�Ԃ����̍ŏ��l=10�Ȃ̂ł��̒l�͖��Ȃ�
	if (GetTabSpace() >= GetMaxLineKetas()) {
		nTabSpace = 4;
	}
	size_t nAllLineNum = pDocLineMgr->GetLineCount();
	LayoutWork	work;
	LayoutWork* pWork = &work;
	pWork->pDocLine		= pDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr�ύX
	pWork->pLayout			= nullptr;
	pWork->pColorStrategy	= nullptr;
	pWork->colorPrev		= COLORIDX_DEFAULT;
	pWork->nCurLine			= 0;

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= 0;
		pWork->nPos			= 0;
		pWork->nWordBgn		= 0;
		pWork->nWordLen		= 0;
		pWork->nPosX		= 0;	// �\�����X�ʒu
		pWork->nIndent		= 0;	// �C���f���g��

		_MakeOneLine(pWork, &LayoutMgr::_OnLine1);

		if (pWork->nPos - pWork->nBgn > 0) {
// 2002/03/13 novice
			AddLineBottom(pWork->_CreateLayout(this));
			pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
			pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
		}

		// ���̍s��
		pWork->nCurLine++;
		pWork->pDocLine = pWork->pDocLine->GetNextLine();
		
		// �������̃��[�U�[������\�ɂ���
		if (1
			&& GetListenerCount() != 0
			&& 0 < nAllLineNum
			&& (pWork->nCurLine % 1024) == 0
		) {
			NotifyProgress(pWork->nCurLine * 100 / nAllLineNum);
			if (!::BlockingHook(NULL)) return;
		}

// 2002/03/13 novice
	}

	// 2011.12.31 Bot�̐F�������͍Ō�ɐݒ�
	nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
	layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	nPrevReferLine = 0;
	pLayoutPrevRefer = nullptr;

	if (GetListenerCount() != 0) {
		NotifyProgress(0);
		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(NULL)) {
			return;
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �{����(�͈͎w��)                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_OnLine2(LayoutWork* pWork)
{
	//@@@ 2002.09.23 YAZAKI �œK��
	if (pWork->bNeedChangeCOMMENTMODE) {
		pWork->pLayout = pWork->pLayout->GetNextLayout();
		pWork->pLayout->SetColorTypePrev(pWork->colorPrev);
		pWork->pLayout->GetLayoutExInfo()->SetColorInfo(pWork->exInfoPrev.DetachColorInfo());
		(*pWork->pnExtInsLineNum)++;								// �ĕ`�悵�Ăق����s�� + 1
	}else {
		pWork->pLayout = InsertLineNext(pWork->pLayout, pWork->_CreateLayout(this));
	}
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	pWork->nPosX = pWork->nIndent = (this->*getIndentOffset)(pWork->pLayout);
	if (0
		|| (1
			&& pWork->ptDelLogicalFrom.y == pWork->nCurLine
			&& pWork->ptDelLogicalFrom.x < (int)pWork->nPos
			)
		|| (pWork->ptDelLogicalFrom.y < pWork->nCurLine)
	) {
		(pWork->nModifyLayoutLinesNew)++;
	}
}

/*!
	�w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		pWork->nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�Dm_nMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
	@date 2009.08.28 nasukoji	�e�L�X�g�ő啝�̎Z�o�ɑΉ�

	@note 2004.04.03 Moca
		_DoLayout�Ƃ͈���ă��C�A�E�g��񂪃��X�g���Ԃɑ}������邽�߁C
		�}�����nLineTypeBot�փR�����g���[�h���w�肵�Ă͂Ȃ�Ȃ�
		����ɍŏI�s�̃R�����g���[�h���I���ԍۂɊm�F���Ă���D
*/
int LayoutMgr::DoLayout_Range(
	Layout*				pLayoutPrev,
	int					nLineNum,
	Point				_ptDelLogicalFrom,
	EColorIndexType		nCurrentLineType,
	LayoutColorInfo*	colorInfo,
	const CalTextWidthArg&	ctwArg
	)
{

	int nLineNumWork = 0;

	// 2006.12.01 Moca �r���ɂ܂ōč\�z�����ꍇ��EOF�ʒu�����ꂽ�܂�
	// �X�V����Ȃ��̂ŁC�͈͂ɂ�����炸�K�����Z�b�g����D
	nEOFColumn = -1;
	nEOFLine = -1;

	LayoutWork work;
	LayoutWork* pWork = &work;
	pWork->pLayout					= pLayoutPrev;
	pWork->pColorStrategy			= ColorStrategyPool::getInstance().GetStrategyByColor(nCurrentLineType);
	pWork->colorPrev				= nCurrentLineType;
	pWork->exInfoPrev.SetColorInfo(colorInfo);
	pWork->bNeedChangeCOMMENTMODE	= false;
	if (!pWork->pLayout) {
		pWork->nCurLine = 0;
	}else {
		pWork->nCurLine = pWork->pLayout->GetLogicLineNo() + 1;
	}
	pWork->pDocLine					= pDocLineMgr->GetLine(pWork->nCurLine);
	pWork->nModifyLayoutLinesNew	= 0;
	// ����
	pWork->ptDelLogicalFrom		= _ptDelLogicalFrom;
	pWork->pnExtInsLineNum		= 0;

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
		pWork->pColorStrategy->SetStrategyColorInfo(colorInfo);
	}

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= 0;
		pWork->nPos			= 0;
		pWork->nWordBgn		= 0;
		pWork->nWordLen		= 0;
		pWork->nPosX		= 0;			// �\�����X�ʒu
		pWork->nIndent		= 0;			// �C���f���g��

		_MakeOneLine(pWork, &LayoutMgr::_OnLine2);

		if (pWork->nPos - pWork->nBgn > 0) {
// 2002/03/13 novice
			//@@@ 2002.09.23 YAZAKI �œK��
			_OnLine2(pWork);
		}

		++nLineNumWork;
		pWork->nCurLine++;

		// �ړI�̍s��(nLineNum)�ɒB�������A�܂��͒ʂ�߂����i���s�����������j���m�F
		//@@@ 2002.09.23 YAZAKI �œK��
		if (nLineNumWork >= nLineNum) {
			if (pWork->pLayout && pWork->pLayout->GetNextLayout()) {
				if (pWork->colorPrev != pWork->pLayout->GetNextLayout()->GetColorTypePrev()) {
					// COMMENTMODE���قȂ�s�������܂����̂ŁA���̍s�����̍s�ƍX�V���Ă����܂��B
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& pWork->exInfoPrev.GetColorInfo()
					&& pWork->pLayout->GetNextLayout()->GetColorInfo()
					&& !pWork->exInfoPrev.GetColorInfo()->IsEqual(pWork->pLayout->GetNextLayout()->GetColorInfo())
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& pWork->exInfoPrev.GetColorInfo()
					&& !pWork->pLayout->GetNextLayout()->GetColorInfo()
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& !pWork->exInfoPrev.GetColorInfo()
					&& pWork->pLayout->GetNextLayout()->GetColorInfo()
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else {
					break;
				}
			}else {
				break;	// while (pWork->pDocLine) �I��
			}
		}
		pWork->pDocLine = pWork->pDocLine->GetNextLine();
// 2002/03/13 novice
	}

	// 2004.03.28 Moca EOF�����̘_���s�̒��O�̍s�̐F�������m�F�E�X�V���ꂽ
	if (pWork->nCurLine == pDocLineMgr->GetLineCount()) {
		nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
		layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	}

	// 2009.08.28 nasukoji	�e�L�X�g���ҏW���ꂽ��ő啝���Z�o����
	CalculateTextWidth_Range(ctwArg);

// 1999.12.22 ���C�A�E�g��񂪂Ȃ��Ȃ��ł͂Ȃ��̂�
// nPrevReferLine = 0;
// pLayoutPrevRefer = nullptr;
// pLayoutCurrent = nullptr;

	return pWork->nModifyLayoutLinesNew;
}

/*!
	@brief �e�L�X�g���ҏW���ꂽ��ő啝���Z�o����

	@param[in] ctwArg �e�L�X�g�ő啝�Z�o�p�\����

	@note �u�܂�Ԃ��Ȃ��v�I�����̂݃e�L�X�g�ő啝���Z�o����D
	      �ҏW���ꂽ�s�͈̔͂ɂ��ĎZ�o����i���L�𖞂����ꍇ�͑S�s�j
	      �@�폜�s�Ȃ����F�ő啝�̍s���s���ȊO�ɂĉ��s�t���ŕҏW����
	      �@�폜�s���莞�F�ő啝�̍s���܂�ŕҏW����
	      ctwArg.nDelLines �������̎��͍폜�s�Ȃ��D

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
void LayoutMgr::CalculateTextWidth_Range(const CalTextWidthArg& ctwArg)
{
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {	// �u�܂�Ԃ��Ȃ��v
		int nCalTextWidthLinesFrom(0);	// �e�L�X�g�ő啝�̎Z�o�J�n���C�A�E�g�s
		int nCalTextWidthLinesTo(0);	// �e�L�X�g�ő啝�̎Z�o�I�����C�A�E�g�s
		bool bCalTextWidth = true;		// �e�L�X�g�ő啝�̎Z�o�v����ON
		int nInsLineNum = nLines - ctwArg.nAllLinesOld;		// �ǉ��폜�s��

		// �폜�s�Ȃ����F�ő啝�̍s���s���ȊO�ɂĉ��s�t���ŕҏW����
		// �폜�s���莞�F�ő啝�̍s���܂�ŕҏW����

		if (0
			|| (1
				&& ctwArg.nDelLines < 0
				&& nTextWidth
				&& nInsLineNum
				&& ctwArg.ptLayout.x
				&& nTextWidthMaxLine == ctwArg.ptLayout.y
			)
			|| (1
				&& ctwArg.nDelLines >= 0
				&& nTextWidth
				&& ctwArg.ptLayout.y <= (int)nTextWidthMaxLine
				&& (int)nTextWidthMaxLine <= ctwArg.ptLayout.y + ctwArg.nDelLines 
			)
		) {
			// �S���C���𑖍�����
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		// �ǉ��폜�s �܂��� �ǉ������񂠂�
		}else if (nInsLineNum || ctwArg.bInsData) {
			// �ǉ��폜�s�݂̂𑖍�����
			nCalTextWidthLinesFrom = ctwArg.ptLayout.y;

			// �ŏI�I�ɕҏW���ꂽ�s���i3�s�폜2�s�ǉ��Ȃ�2�s�ǉ��j
			// 1�s��MAXLINEKETAS�𒴂���ꍇ�s��������Ȃ��Ȃ邪�A������ꍇ�͂��̐�̌v�Z���̂�
			// �s�v�Ȃ̂Ōv�Z���Ȃ����߂��̂܂܂Ƃ���B
			int nEditLines = nInsLineNum + ((ctwArg.nDelLines > 0) ? ctwArg.nDelLines : 0);
			nCalTextWidthLinesTo   = ctwArg.ptLayout.y + ((nEditLines > 0) ? nEditLines : 0);

			// �ő啝�̍s���㉺����̂��v�Z
			if (1
				&& nTextWidth
				&& nInsLineNum
				&& (int)nTextWidthMaxLine >= ctwArg.ptLayout.y
			) {
				nTextWidthMaxLine += nInsLineNum;
			}
		}else {
			// �ő啝�ȊO�̍s�����s���܂܂��Ɂi1�s���Łj�ҏW����
			bCalTextWidth = false;		// �e�L�X�g�ő啝�̎Z�o�v����OFF
		}

#if defined(_DEBUG) && defined(_UNICODE)
		static int testcount = 0;
		++testcount;

		// �e�L�X�g�ő啝���Z�o����
		if (bCalTextWidth) {
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range(%d) nCalTextWidthLinesFrom=%d nCalTextWidthLinesTo=%d\n", testcount, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
			CalculateTextWidth(false, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range() nTextWidthMaxLine=%d\n", nTextWidthMaxLine);
		}else {
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range(%d) FALSE\n", testcount);
		}
#else
		// �e�L�X�g�ő啝���Z�o����
		if (bCalTextWidth)
			CalculateTextWidth(false, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
#endif
	}
}

