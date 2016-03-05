/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <vector>
#include <limits.h>
#include "view/EditView_Paint.h"
#include "view/EditView.h"
#include "view/ViewFont.h"
#include "view/Ruler.h"
#include "view/colors/ColorStrategy.h"
#include "view/colors/Color_Found.h"
#include "view/figures/FigureManager.h"
#include "types/TypeSupport.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"
#ifdef USE_SSE2
#ifdef __MINGW32__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif
#endif

void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView* pView, LayoutYInt nLineNum);

/*
	PaintAreaType::LineNumber = (1<<0), // �s�ԍ�
	PaintAreaType::Ruler      = (1<<1), // ���[���[
	PaintAreaType::Body       = (1<<2), // �{��
*/

void EditView_Paint::Call_OnPaint(
	int nPaintFlag,   // �`�悷��̈��I������
	bool bUseMemoryDC // ������DC���g�p����
	)
{
	EditView* pView = GetEditView();

	auto& textArea = pView->GetTextArea();
	// �e�v�f
	Rect rcLineNumber(0, textArea.GetAreaTop(), textArea.GetAreaLeft(), textArea.GetAreaBottom());
	Rect rcRuler(textArea.GetAreaLeft(), 0, textArea.GetAreaRight(), textArea.GetAreaTop());
	Rect rcBody(textArea.GetAreaLeft(), textArea.GetAreaTop(), textArea.GetAreaRight(), textArea.GetAreaBottom());

	// �̈���쐬 -> rc
	std::vector<Rect> rcs;
	if (nPaintFlag & (int)PaintAreaType::LineNumber) rcs.push_back(rcLineNumber);
	if (nPaintFlag & (int)PaintAreaType::Ruler) rcs.push_back(rcRuler);
	if (nPaintFlag & (int)PaintAreaType::Body) rcs.push_back(rcBody);
	if (rcs.size() == 0) return;
	Rect rc = rcs[0];
	int nSize = (int)rcs.size();
	for (int i=1; i<nSize; ++i) {
		rc = MergeRect(rc, rcs[i]);
	}

	// �`��
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = pView->GetDC();
	pView->OnPaint(hdc, &ps, bUseMemoryDC);
	pView->ReleaseDC(hdc);
}


/* �t�H�[�J�X�ړ����̍ĕ`��

	@date 2001/06/21 asa-o �u�X�N���[���o�[�̏�Ԃ��X�V����v�u�J�[�\���ړ��v�폜
*/
void EditView::RedrawAll()
{
	if (!GetHwnd()) {
		return;
	}
	
	if (GetDrawSwitch()) {
		// �E�B���h�E�S�̂��ĕ`��
		PAINTSTRUCT	ps;
		HDC hdc = ::GetDC(GetHwnd());
		::GetClientRect(GetHwnd(), &ps.rcPaint);
		OnPaint(hdc, &ps, FALSE);
		::ReleaseDC(GetHwnd(), hdc);
	}

	// �L�����b�g�̕\��
	GetCaret().ShowEditCaret();

	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();

	// �e�E�B���h�E�̃^�C�g�����X�V
	m_pEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	�I��͈͂̏����X�e�[�^�X�o�[�֕\��
	GetSelectionInfo().PrintSelectionInfoMsg();

	// �X�N���[���o�[�̏�Ԃ��X�V����
	AdjustScrollBars();
}

// 2001/06/21 Start by asa-o �ĕ`��
void EditView::Redraw()
{
	if (!GetHwnd()) {
		return;
	}
	if (!GetDrawSwitch()) {
		return;
	}

	HDC hdc = ::GetDC(GetHwnd());
	PAINTSTRUCT	ps;
	::GetClientRect(GetHwnd(), &ps.rcPaint);
	OnPaint(hdc, &ps, FALSE);
	::ReleaseDC(GetHwnd(), hdc);
}
// 2001/06/21 End

void EditView::RedrawLines(LayoutYInt top, LayoutYInt bottom)
{
	if (!GetHwnd()) {
		return;
	}
	if (!GetDrawSwitch()) {
		return;
	}

	if (bottom < GetTextArea().GetViewTopLine()) {
		return;
	}
	if (GetTextArea().GetBottomLine() <= top) {
		return;
	}
	HDC hdc = GetDC();
	PAINTSTRUCT	ps;
	ps.rcPaint.left = 0;
	ps.rcPaint.right = GetTextArea().GetAreaRight();
	ps.rcPaint.top = GetTextArea().GenerateYPx(top);
	ps.rcPaint.bottom = GetTextArea().GenerateYPx(bottom);
	OnPaint(hdc, &ps, FALSE);
	ReleaseDC(hdc);
}

void MyFillRect(HDC hdc, RECT& re)
{
	::ExtTextOut(hdc, re.left, re.top, ETO_OPAQUE|ETO_CLIPPED, &re, _T(""), 0, NULL);
}

void EditView::DrawBackImage(HDC hdc, RECT& rcPaint, HDC hdcBgImg)
{
#if 0
//	�e�X�g�w�i�p�^�[��
	static int testColorIndex = 0;
	testColorIndex = testColorIndex % 7;
	COLORREF cols[7] = {RGB(255,255,255),
		RGB(200,255,255),RGB(255,200,255),RGB(255,255,200),
		RGB(200,200,255),RGB(255,200,200),RGB(200,255,200),
	};
	COLORREF colorOld = ::SetBkColor(hdc, cols[testColorIndex]);
	MyFillRect(hdc, rcPaint);
	::SetBkColor(hdc, colorOld);
	++testColorIndex;
#else
	TypeSupport textType(this, COLORIDX_TEXT);
	COLORREF colorOld = ::SetBkColor(hdc, textType.GetBackColor());
	const TextArea& area = GetTextArea();
	const EditDoc& doc  = *m_pEditDoc;
	const TypeConfig& typeConfig = doc.m_docType.GetDocumentAttribute();

	Rect rcImagePos;
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::CenterLeft:
		rcImagePos.left = area.GetAreaLeft();
		break;
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::CenterRight:
		rcImagePos.left = area.GetAreaRight() - doc.m_nBackImgWidth;
		break;
	case BackgroundImagePosType::TopCenter:
	case BackgroundImagePosType::BottomCenter:
	case BackgroundImagePosType::Center:
		rcImagePos.left = area.GetAreaLeft() + area.GetAreaWidth()/2 - doc.m_nBackImgWidth/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::TopCenter:
		rcImagePos.top  = area.GetAreaTop();
		break;
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::BottomCenter:
		rcImagePos.top  = area.GetAreaBottom() - doc.m_nBackImgHeight;
		break;
	case BackgroundImagePosType::CenterLeft:
	case BackgroundImagePosType::CenterRight:
	case BackgroundImagePosType::Center:
		rcImagePos.top  = area.GetAreaTop() + area.GetAreaHeight()/2 - doc.m_nBackImgHeight/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	rcImagePos.left += typeConfig.backImgPosOffset.x;
	rcImagePos.top  += typeConfig.backImgPosOffset.y;
	// �X�N���[�����̉�ʂ̒[����悷��Ƃ��̈ʒu������ֈړ�
	if (typeConfig.backImgScrollX) {
		int tile = typeConfig.backImgRepeatX ? doc.m_nBackImgWidth : INT_MAX;
		Int posX = (area.GetViewLeftCol() % tile) * GetTextMetrics().GetHankakuDx();
		rcImagePos.left -= posX % tile;
	}
	if (typeConfig.backImgScrollY) {
		int tile = typeConfig.backImgRepeatY ? doc.m_nBackImgHeight : INT_MAX;
		Int posY = (area.GetViewTopLine() % tile) * GetTextMetrics().GetHankakuDy();
		rcImagePos.top -= posY % tile;
	}
	if (typeConfig.backImgRepeatX) {
		if (0 < rcImagePos.left) {
			// rcImagePos.left = rcImagePos.left - (rcImagePos.left / doc.m_nBackImgWidth + 1) * doc.m_nBackImgWidth;
			rcImagePos.left = rcImagePos.left % doc.m_nBackImgWidth - doc.m_nBackImgWidth;
		}
	}
	if (typeConfig.backImgRepeatY) {
		if (0 < rcImagePos.top) {
			// rcImagePos.top = rcImagePos.top - (rcImagePos.top / doc.m_nBackImgHeight + 1) * doc.m_nBackImgHeight;
			rcImagePos.top = rcImagePos.top % doc.m_nBackImgHeight - doc.m_nBackImgHeight;
		}
	}
	rcImagePos.SetSize(doc.m_nBackImgWidth, doc.m_nBackImgHeight);
	
	RECT rc = rcPaint;
	// rc.left = t_max((int)rc.left, area.GetAreaLeft());
	rc.top  = t_max((int)rc.top,  area.GetRulerHeight()); // ���[���[�����O
	const int nXEnd = area.GetAreaRight();
	const int nYEnd = area.GetAreaBottom();
	Rect rcBltAll;
	rcBltAll.SetLTRB(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
	Rect rcImagePosOrg = rcImagePos;
	for (; rcImagePos.top<=nYEnd; ) {
		for (; rcImagePos.left<=nXEnd; ) {
			Rect rcBlt;
			if (::IntersectRect(&rcBlt, &rc, &rcImagePos)) {
				::BitBlt(
					hdc,
					rcBlt.left,
					rcBlt.top,
					rcBlt.right  - rcBlt.left,
					rcBlt.bottom - rcBlt.top,
					hdcBgImg,
					rcBlt.left - rcImagePos.left,
					rcBlt.top - rcImagePos.top,
					SRCCOPY
				);
				rcBltAll.left   = t_min(rcBltAll.left,   rcBlt.left);
				rcBltAll.top    = t_min(rcBltAll.top,    rcBlt.top);
				rcBltAll.right  = t_max(rcBltAll.right,  rcBlt.right);
				rcBltAll.bottom = t_max(rcBltAll.bottom, rcBlt.bottom);
			}
			rcImagePos.left  += doc.m_nBackImgWidth;
			rcImagePos.right += doc.m_nBackImgWidth;
			if (!typeConfig.backImgRepeatX) {
				break;
			}
		}
		rcImagePos.left  = rcImagePosOrg.left;
		rcImagePos.right = rcImagePosOrg.right;
		rcImagePos.top    += doc.m_nBackImgHeight;
		rcImagePos.bottom += doc.m_nBackImgHeight;
		if (!typeConfig.backImgRepeatY) {
			break;
		}
	}
	if (rcBltAll.left != INT_MAX) {
		// �㉺���E�ȂȂ߂̌��Ԃ𖄂߂�
		Rect rcFill;
		LONG& x1 = rc.left;
		LONG& x2 = rcBltAll.left;
		LONG& x3 = rcBltAll.right;
		LONG& x4 = rc.right;
		LONG& y1 = rc.top;
		LONG& y2 = rcBltAll.top;
		LONG& y3 = rcBltAll.bottom;
		LONG& y4 = rc.bottom;
		if (y1 < y2) {
			rcFill.SetLTRB(x1,y1, x4,y2); MyFillRect(hdc, rcFill);
		}
		if (x1 < x2) {
			rcFill.SetLTRB(x1,y2, x2,y3); MyFillRect(hdc, rcFill);
		}
		if (x3 < x4) {
			rcFill.SetLTRB(x3,y2, x4,y3); MyFillRect(hdc, rcFill);
		}
		if (y3 < y4) {
			rcFill.SetLTRB(x1,y3, x4,y4); MyFillRect(hdc, rcFill);
		}
	}else {
		MyFillRect(hdc, rc);
	}
	::SetBkColor(hdc, colorOld);
#endif
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�ݒ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �w��ʒu��ColorIndex�̎擾
	EditView::DrawLogicLine�����ɂ�������EditView::DrawLogicLine��
	�C�����������ꍇ�́A�������C�����K�v�B
*/
Color3Setting EditView::GetColorIndex(
	const Layout*			pLayout,
	LayoutYInt				nLineNum,
	int						nIndex,
	ColorStrategyInfo*	 	pInfo,			// 2010.03.31 ryoji �ǉ�
	bool					bPrev			// �w��ʒu�̐F�ύX���O�܂�	2010.06.19 ryoji �ǉ�
	)
{
	EColorIndexType eRet = COLORIDX_TEXT;

	if (!pLayout) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}
	// 2014.12.30 Skip���[�h�̎���COLORIDX_TEXT
	if (ColorStrategyPool::getInstance()->IsSkipBeforeLayout()) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}

	const LayoutColorInfo* colorInfo;
	const Layout* pLayoutLineFirst = pLayout;
	LayoutYInt nLineNumFirst = nLineNum;
	{
		// 2002/2/10 aroka CMemory�ύX
		pInfo->pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();

		// �_���s�̍ŏ��̃��C�A�E�g�����擾 -> pLayoutLineFirst
		while (pLayoutLineFirst->GetLogicOffset() != 0) {
			pLayoutLineFirst = pLayoutLineFirst->GetPrevLayout();
			--nLineNumFirst;

			// �_���s�̐擪�܂Ŗ߂�Ȃ��Ɗm���ɂ͐��m�ȐF�͓����Ȃ�
			// �i���K�\���L�[���[�h�Ƀ}�b�`�������������\�������̈ʒu�̃��C�A�E�g�s�����܂����ł���ꍇ�Ȃǁj
			//if (pLayout->GetLogicOffset() - pLayoutLineFirst->GetLogicOffset() > 260)
			//	break;
		}

		// 2005.11.20 Moca �F���������Ȃ����Ƃ�������ɑΏ�
		eRet = pLayoutLineFirst->GetColorTypePrev();	// ���݂̐F���w��	// 02/12/18 ai
		colorInfo = pLayoutLineFirst->GetColorInfo();
		pInfo->nPosInLogic = pLayoutLineFirst->GetLogicOffset();

		// ColorStrategyPool������
		ColorStrategyPool* pool = ColorStrategyPool::getInstance();
		pool->SetCurrentView(this);
		pool->NotifyOnStartScanLogic();

		// 2009.02.07 ryoji ���̊֐��ł� pInfo->CheckChangeColor() �ŐF�𒲂ׂ邾���Ȃ̂ňȉ��̏����͕s�v
		//
		////############�����B�{����Visitor���g���ׂ�
		//class TmpVisitor{
		//public:
		//	static int CalcLayoutIndex(const Layout* pLayout)
		//	{
		//		int n = -1;
		//		while (pLayout) {
		//			pLayout = pLayout->GetPrevLayout(); // prev or null
		//			++n;
		//		}
		//		return n;
		//	}
		//};
		//pInfo->pDispPos->SetLayoutLineRef(LayoutInt(TmpVisitor::CalcLayoutIndex(pLayout)));
		// 2013.12.11 Moca �J�����g�s�̐F�ւ��ŕK�v�ɂȂ�܂���
		pInfo->pDispPos->SetLayoutLineRef(nLineNumFirst);
	}

	// ������Q��
	const DocLine* pDocLine = pLayout->GetDocLineRef();
	StringRef lineStr(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());

	// color strategy
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pInfo->pStrategy = pool->GetStrategyByColor(eRet);
	if (pInfo->pStrategy) {
		pInfo->pStrategy->InitStrategyStatus();
		pInfo->pStrategy->SetStrategyColorInfo(colorInfo);
	}

	const Layout* pLayoutNext = pLayoutLineFirst->GetNextLayout();
	LayoutYInt nLineNumScan = nLineNumFirst;
	int nPosTo = pLayout->GetLogicOffset() + t_min(nIndex, (int)pLayout->GetLengthWithEOL() - 1);
	while (pInfo->nPosInLogic <= nPosTo) {
		if (bPrev && pInfo->nPosInLogic == nPosTo)
			break;

		// �F�ؑ�
		pInfo->CheckChangeColor(lineStr);

		// 1�����i��
		pInfo->nPosInLogic += NativeW::GetSizeOfChar(
									lineStr.GetPtr(),
									lineStr.GetLength(),
									pInfo->nPosInLogic
								);
		if (pLayoutNext && pLayoutNext->GetLogicOffset() <= pInfo->nPosInLogic) {
			++nLineNumScan;
			pInfo->pDispPos->SetLayoutLineRef(nLineNumScan);
			pLayoutNext = pLayoutNext->GetNextLayout();
		}
	}

	Color3Setting cColor;
	pInfo->DoChangeColor(&cColor);

	return cColor;
}

/*! ���݂̐F���w��
	@param eColorIndex   �I�����܂ތ��݂̐F
	@param eColorIndex2  �I���ȊO�̌��݂̐F
	@param eColorIndexBg �w�i�F

	@date 2013.05.08 novice �͈͊O�`�F�b�N�폜
*/
void EditView::SetCurrentColor(
	Graphics& gr,
	EColorIndexType eColorIndex,
	EColorIndexType eColorIndex2,
	EColorIndexType eColorIndexBg
	)
{
	// �C���f�b�N�X����
	int nColorIdx = ToColorInfoArrIndex(eColorIndex);
	int nColorIdx2 = ToColorInfoArrIndex(eColorIndex2);
	int nColorIdxBg = ToColorInfoArrIndex(eColorIndexBg);

	// ���ۂɐF��ݒ�
	const ColorInfo& info  = m_pTypeData->colorInfoArr[nColorIdx];
	const ColorInfo& info2 = m_pTypeData->colorInfoArr[nColorIdx2];
	const ColorInfo& infoBg = m_pTypeData->colorInfoArr[nColorIdxBg];
	COLORREF fgcolor = GetTextColorByColorInfo2(info, info2);
	gr.SetTextForeColor(fgcolor);
	// 2012.11.21 �w�i�F���e�L�X�g�Ƃ��Ȃ��Ȃ�w�i�F�̓J�[�\���s�w�i
	const ColorInfo& info3 = (info2.colorAttr.cBACK == m_crBack ? infoBg : info2);
	COLORREF bkcolor = (nColorIdx == nColorIdx2) ? info3.colorAttr.cBACK : GetBackColorByColorInfo2(info, info3);
	gr.SetTextBackColor(bkcolor);
	Font font;
	font.fontAttr = (info.colorAttr.cTEXT != info.colorAttr.cBACK) ? info.fontAttr : info2.fontAttr;
	font.hFont = GetFontset().ChooseFontHandle(font.fontAttr);
	gr.SetMyFont(font);
}

inline COLORREF MakeColor2(COLORREF a, COLORREF b, int alpha)
{
#ifdef USE_SSE2
	// (a * alpha + b * (256 - alpha)) / 256 -> ((a - b) * alpha) / 256 + b
	__m128i xmm0, xmm1, xmm2, xmm3;
	COLORREF color;
	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_cvtsi32_si128(a);
	xmm2 = _mm_cvtsi32_si128(b);
	xmm3 = _mm_cvtsi32_si128(alpha);

	xmm1 = _mm_unpacklo_epi8(xmm1, xmm0); // a:a:a:a
	xmm2 = _mm_unpacklo_epi8(xmm2, xmm0); // b:b:b:b
	xmm3 = _mm_shufflelo_epi16(xmm3, 0); // alpha:alpha:alpha:alpha

	xmm1 = _mm_sub_epi16(xmm1, xmm2); // (a - b)
	xmm1 = _mm_mullo_epi16(xmm1, xmm3); // (a - b) * alpha
	xmm1 = _mm_srli_epi16(xmm1, 8); // ((a - b) * alpha) / 256
	xmm1 = _mm_add_epi8(xmm1, xmm2); // ((a - b) * alpha) / 256 + b

	xmm1 = _mm_packus_epi16(xmm1, xmm0);
	color = _mm_cvtsi128_si32(xmm1);

	return color;
#else
	const int ap = alpha;
	const int bp = 256 - ap;
	BYTE valR = (BYTE)((GetRValue(a) * ap + GetRValue(b) * bp) / 256);
	BYTE valG = (BYTE)((GetGValue(a) * ap + GetGValue(b) * bp) / 256);
	BYTE valB = (BYTE)((GetBValue(a) * ap + GetBValue(b) * bp) / 256);
	return RGB(valR, valG, valB);
#endif
}

COLORREF EditView::GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2)
{
	if (info.colorAttr.cTEXT != info.colorAttr.cBACK) {
		return info.colorAttr.cTEXT;
	}
	// ���]�\��
	if (info.colorAttr.cBACK == m_crBack) {
		return  info2.colorAttr.cTEXT ^ 0x00FFFFFF;
	}
	int alpha = 255*30/100; // 30%
	return MakeColor2(info.colorAttr.cTEXT, info2.colorAttr.cTEXT, alpha);
}

COLORREF EditView::GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2)
{
	if (info.colorAttr.cTEXT != info.colorAttr.cBACK) {
		return info.colorAttr.cBACK;
	}
	// ���]�\��
	if (info.colorAttr.cBACK == m_crBack) {
		return  info2.colorAttr.cBACK ^ 0x00FFFFFF;
	}
	int alpha = 255*30/100; // 30%
	return MakeColor2(info.colorAttr.cBACK, info2.colorAttr.cBACK, alpha);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �`��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnPaint(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp)
{
	bool bChangeFont = m_bMiniMap;
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local);
	}
	OnPaint2(_hdc, pPs, bDrawFromComptibleBmp);
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::Edit, m_pEditWnd->GetLogfontCacheMode());
	}
}

/*! �ʏ�̕`�揈�� new 
	@param pPs  pPs.rcPaint �͐������K�v������
	@param bDrawFromComptibleBmp  TRUE ��ʃo�b�t�@����hdc�ɍ�悷��(�R�s�[���邾��)�B
			TRUE�̏ꍇ�ApPs.rcPaint�̈�O�͍�悳��Ȃ����AFALSE�̏ꍇ�͍�悳��鎖������B
			�݊�DC/BMP�������ꍇ�́A���ʂ̍�揈��������B

	@date 2007.09.09 Moca ���X����������Ă�����O�p�����[�^��bUseMemoryDC��bDrawFromComptibleBmp�ɕύX�B
	@date 2009.03.26 ryoji �s�ԍ��̂ݕ`���ʏ�̍s�`��ƕ����i�������j
*/
void EditView::OnPaint2(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp)
{
//	MY_RUNNINGTIMER(runningTimer, "EditView::OnPaint");
	Graphics gr(_hdc);

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if (!GetHwnd() || !_hdc) {
		return;
	}

	if (!GetDrawSwitch()) return;
	//@@@
#if 0
	::MYTRACE(_T("OnPaint(%d,%d)-(%d,%d) : %d\n"),
		pPs->rcPaint.left,
		pPs->rcPaint.top,
		pPs->rcPaint.right,
		pPs->rcPaint.bottom,
		bDrawFromComptibleBmp
		);
#endif
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �݊�BMP����̓]���݂̂ɂ����
	if (bDrawFromComptibleBmp
		&& m_hdcCompatDC
		&& m_hbmpCompatBMP
	) {
		::BitBlt(
			gr,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			m_hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if (m_pEditWnd->GetActivePane() == m_nMyIndex) {
			// �A�N�e�B�u�y�C���́A�A���_�[���C���`��
			GetCaret().m_underLine.CaretUnderLineON(true, false);
		}
		return;
	}
	if (m_hdcCompatDC && !m_hbmpCompatBMP
		 || m_nCompatBMPWidth < (pPs->rcPaint.right - pPs->rcPaint.left)
		 || m_nCompatBMPHeight < (pPs->rcPaint.bottom - pPs->rcPaint.top)
	) {
		RECT rect;
		::GetWindowRect(this->GetHwnd(), &rect);
		CreateOrUpdateCompatibleBitmap(rect.right - rect.left, rect.bottom - rect.top);
	}
	// To Here 2007.09.09 Moca

	// �L�����b�g���B��
	bool bCaretShowFlag_Old = GetCaret().GetCaretShowFlag();	// 2008.06.09 ryoji
	GetCaret().HideCaret_(this->GetHwnd()); // 2002/07/22 novice

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();

	// �T�|�[�g
	TypeSupport textType(this, COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	// �ύX������΃^�C�v�ݒ���s���B
	if (m_pTypeData->bUseRegexKeyword || m_pRegexKeyword->m_bUseRegexKeyword) { // OFF�Ȃ̂ɑO��̃f�[�^���c���Ă�
		// �^�C�v�ʐݒ������B�ݒ�ς݂��ǂ����͌Ăѐ�Ń`�F�b�N����B
		m_pRegexKeyword->RegexKeySetTypes(m_pTypeData);
	}
//@@@ 2001.11.17 add end MIK

	bool bTransText = IsBkBitmap();
	// �������c�b�𗘗p�����ĕ`��̏ꍇ�͕`���̂c�b��؂�ւ���
	HDC hdcOld = 0;
	// 2007.09.09 Moca bUseMemoryDC��L�����B
	// bUseMemoryDC = FALSE;
	bool bUseMemoryDC = (m_hdcCompatDC != NULL);
	assert_warning(gr != m_hdcCompatDC);
	if (bUseMemoryDC) {
		hdcOld = gr;
		gr = m_hdcCompatDC;
	}else {
		if (bTransText || pPs->rcPaint.bottom - pPs->rcPaint.top <= 2 || pPs->rcPaint.left - pPs->rcPaint.right <= 2) {
			// ���ߏ����̏ꍇ�t�H���g�̗֊s���d�˓h��ɂȂ邽�ߎ����ŃN���b�s���O�̈��ݒ�
			// 2�ȉ��͂��Ԃ�A���_�[���C���E�J�[�\���s�c���̍��
			// MemoryDC�̏ꍇ�͓]������`�N���b�s���O�̑���ɂȂ��Ă���
			gr.SetClipping(pPs->rcPaint);
		}
	}

	// 03/02/18 �Ί��ʂ̋����\��(����) ai
	if (!bUseMemoryDC) {
		// MemoryDC���ƃX�N���[�����ɐ�Ɋ��ʂ����\������ĕs���R�Ȃ̂Ō�ł��B
		DrawBracketPair(false);
	}

	EditView& activeView = m_pEditWnd->GetActiveView();
	m_nPageViewTop = activeView.GetTextArea().GetViewTopLine();
	m_nPageViewBottom = activeView.GetTextArea().GetBottomLine();

	// �w�i�̕\��
	if (bTransText) {
		HDC hdcBgImg = CreateCompatibleDC(gr);
		HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, m_pEditDoc->m_hBackImg);
		DrawBackImage(gr, pPs->rcPaint, hdcBgImg);
		SelectObject(hdcBgImg, hOldBmp);
		DeleteObject(hdcBgImg);
	}

	// ���[���[�ƃe�L�X�g�̊Ԃ̗]��
	//@@@ 2002.01.03 YAZAKI �]����0�̂Ƃ��͖��ʂł����B
	if (GetTextArea().GetTopYohaku()) {
		if (!bTransText) {
			rc.left   = 0;
			rc.top    = GetTextArea().GetRulerHeight();
			rc.right  = GetTextArea().GetAreaRight();
			rc.bottom = GetTextArea().GetAreaTop();
			textType.FillBack(gr, rc);
		}
	}

	// �s�ԍ��̕\��
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta �s�ԍ���\���ł��s�ԍ��F�̑т�����̂Ō��Ԃ𖄂߂�
	if (GetTextArea().GetTopYohaku()) {
		if (bTransText && m_pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK == textType.GetBackColor()) {
		}else {
			rc.left   = 0;
			rc.top    = GetTextArea().GetRulerHeight();
			rc.right  = GetTextArea().GetLineNumberWidth(); //	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
			rc.bottom = GetTextArea().GetAreaTop();
			gr.SetTextBackColor(m_pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK);
			gr.FillMyRectTextBackColor(rc);
		}
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode(gr, TRANSPARENT);

	textType.SetGraphicsState_WhileThisObj(gr);


	int nTop = pPs->rcPaint.top;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           �`��J�n���C�A�E�g��΍s -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt nLayoutLine;
	if (0 > nTop - GetTextArea().GetAreaTop()) {
		nLayoutLine = GetTextArea().GetViewTopLine(); // �r���[�㕔����`��
	}else {
		nLayoutLine = GetTextArea().GetViewTopLine() + LayoutInt((nTop - GetTextArea().GetAreaTop()) / nLineHeight); // �r���[�r������`��
	}

	// �� �����ɂ������`��͈͂� 260 �������[���o�b�N������ GetColorIndex() �ɋz��	// 2009.02.11 ryoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �`��I�����C�A�E�g��΍s -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ LayoutInt((pPs->rcPaint.bottom - GetTextArea().GetAreaTop() + (nLineHeight - 1)) / nLineHeight) - 1;	// 2007.02.17 ryoji �v�Z�𐸖���


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �`����W                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos pos(GetTextMetrics().GetHankakuDx(), GetTextMetrics().GetHankakuDy());
	pos.InitDrawPos(Point(
		GetTextArea().GetAreaLeft() - (Int)GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (Int)(nLayoutLine - GetTextArea().GetViewTopLine()) * nLineHeight
	));
	pos.SetLayoutLineRef(nLayoutLine);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �S���̍s��`��                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �K�v�ȍs��`�悷��	// 2009.03.26 ryoji �s�ԍ��̂ݕ`���ʏ�̍s�`��ƕ����i�������j
	if (pPs->rcPaint.right <= GetTextArea().GetAreaLeft()) {
		while (pos.GetLayoutLineRef() <= nLayoutLineTo) {
			if (!pos.GetLayoutRef())
				break;

			// 1�s�`��i�s�ԍ��̂݁j
			GetTextDrawer().DispLineNumber(
				gr,
				pos.GetLayoutLineRef(),
				pos.GetDrawPos().y
			);
			// �s��i�߂�
			pos.ForwardDrawLine(1);		// �`��Y���W�{�{
			pos.ForwardLayoutLineRef(1);	// ���C�A�E�g�s�{�{
		}
	}else {
		while (pos.GetLayoutLineRef() <= nLayoutLineTo) {
			// �`��X�ʒu���Z�b�g
			pos.ResetDrawCol();

			// 1�s�`��
			bool bDispResult = DrawLogicLine(
				gr,
				&pos,
				nLayoutLineTo
			);

//			if (bDispResult) {
//				pPs->rcPaint.bottom += nLineHeight;	// EOF�ĕ`��Ή�
//				break;
//			}
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              �e�L�X�g�̖��������̓h��Ԃ�                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (!bTransText && pos.GetDrawPos().y < pPs->rcPaint.bottom) {
		RECT rcBack;
		rcBack.left   = pPs->rcPaint.left;
		rcBack.right  = pPs->rcPaint.right;
		rcBack.top    = pos.GetDrawPos().y;
		rcBack.bottom = pPs->rcPaint.bottom;

		textType.FillBack(gr, rcBack);
	}
	
	{
		if (!m_bMiniMap) {
			GetTextDrawer().DispNoteLine(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, pPs->rcPaint.left, pPs->rcPaint.right);
		}
		// 2006.04.29 �s�����͍s���Ƃɍ�悵�A�����ł͏c���̎c������
		GetTextDrawer().DispVerticalLines(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, LayoutInt(0), LayoutInt(-1));
		GetTextDrawer().DispWrapLine(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom);	// 2009.10.24 ryoji
	}

	textType.RewindGraphicsState(gr);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ���[���[�`��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pPs->rcPaint.top < GetTextArea().GetRulerHeight()) { // ���[���[���ĕ`��͈͂ɂ���Ƃ��̂ݍĕ`�悷�� 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); // 2002.02.25 Add By KK ���[���[�S�̂�`��B
		GetRuler().DispRuler(gr);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     ���̑���n���Ȃ�                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �������c�b�𗘗p�����ĕ`��̏ꍇ�̓������c�b�ɕ`�悵�����e����ʂփR�s�[����
	if (bUseMemoryDC) {
		// 2010.10.11 ��ɕ`���Ɣw�i�Œ�̃X�N���[���Ȃǂł̕\�����s���R�ɂȂ�
		DrawBracketPair(false);

		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			gr,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	//     �A���_�[���C���`���������DC����̃R�s�[�O���������Ɉړ�
	if (m_pEditWnd->GetActivePane() == m_nMyIndex) {
		// �A�N�e�B�u�y�C���́A�A���_�[���C���`��
		GetCaret().m_underLine.CaretUnderLineON(true, false);
	}
	// To Here 2007.09.09 Moca

	// 03/02/18 �Ί��ʂ̋����\��(�`��) ai
	DrawBracketPair(true);

	// �L�����b�g�����݈ʒu�ɕ\�����܂�
	if (bCaretShowFlag_Old)	// 2008.06.09 ryoji
		GetCaret().ShowCaret_(this->GetHwnd()); // 2002/07/22 novice
	return;
}

/*!
	�s�̃e�L�X�g�^�I����Ԃ̕`��
	1���1���W�b�N�s������悷��B

	@return EOF����悵����true

	@date 2001.02.17 MIK
	@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX
	@date 2007.08.31 kobake ���� bDispBkBitmap ���폜
*/
bool EditView::DrawLogicLine(
	HDC				_hdc,			// [in]     ���Ώ�
	DispPos*		_pDispPos,		// [in/out] �`�悷��ӏ��A�`�挳�\�[�X
	LayoutInt		nLineTo			// [in]     ���I�����郌�C�A�E�g�s�ԍ�
	)
{
//	MY_RUNNINGTIMER(runningTimer, "EditView::DrawLogicLine");
	bool bDispEOF = false;
	ColorStrategyInfo _sInfo;
	ColorStrategyInfo* pInfo = &_sInfo;
	pInfo->gr.Init(_hdc);
	pInfo->pDispPos = _pDispPos;
	pInfo->pView = this;

	// ColorStrategyPool������
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pool->SetCurrentView(this);
	pool->NotifyOnStartScanLogic();
	bool bSkipBeforeLayout = pool->IsSkipBeforeLayout();

	// DispPos��ۑ����Ă���
	pInfo->dispPosBegin = *pInfo->pDispPos;

	// �������镶���ʒu
	pInfo->nPosInLogic = LogicInt(0); // ���J�n

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �_���s�f�[�^�̎擾 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �O�s�̍ŏI�ݒ�F
	{
		const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
		if (bSkipBeforeLayout) {
			EColorIndexType eRet = COLORIDX_TEXT;
			const LayoutColorInfo* colorInfo = NULL;
			if (pLayout) {
				eRet = pLayout->GetColorTypePrev(); // COLORIDX_TEXT�̂͂�
				colorInfo = pLayout->GetColorInfo();
			}
			pInfo->pStrategy = pool->GetStrategyByColor(eRet);
			if (pInfo->pStrategy) {
				pInfo->pStrategy->InitStrategyStatus();
				pInfo->pStrategy->SetStrategyColorInfo(colorInfo);
			}
		}else {
			Color3Setting cColor = GetColorIndex(pLayout, pInfo->pDispPos->GetLayoutLineRef(), 0, pInfo, true);
			SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
		}
	}

	// �J�n���W�b�N�ʒu���Z�o
	{
		const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
		pInfo->nPosInLogic = pLayout ? pLayout->GetLogicOffset() : LogicInt(0);
	}

	for (;;) {
		// �Ώۍs���`��͈͊O��������I��
		if (GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef()) {
			pInfo->pDispPos->SetLayoutLineRef(nLineTo + LayoutInt(1));
			break;
		}
		if (nLineTo < pInfo->pDispPos->GetLayoutLineRef()) {
			break;
		}

		// ���C�A�E�g�s��1�s�`��
		bDispEOF = DrawLayoutLine(pInfo);

		// �s��i�߂�
		LogicInt nOldLogicLineNo = pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo();
		pInfo->pDispPos->ForwardDrawLine(1);		// �`��Y���W�{�{
		pInfo->pDispPos->ForwardLayoutLineRef(1);	// ���C�A�E�g�s�{�{

		// ���W�b�N�s��`�悵�I������甲����
		if (pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo() != nOldLogicLineNo) {
			break;
		}

		// nLineTo�𒴂����甲����
		if (pInfo->pDispPos->GetLayoutLineRef() >= nLineTo + LayoutInt(1)) {
			break;
		}
	}

	return bDispEOF;
}

/*!
	���C�A�E�g�s��1�s�`��
*/
// ���s�L����`�悵���ꍇ��true��Ԃ��H
bool EditView::DrawLayoutLine(ColorStrategyInfo* pInfo)
{
	bool bDispEOF = false;
	TypeSupport textType(this, COLORIDX_TEXT);

	const Layout* pLayout = pInfo->pDispPos->GetLayoutRef(); //m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(pInfo->pDispPos->GetLayoutLineRef());

	// ���C�A�E�g���
	if (pLayout) {
		pInfo->pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();
	}else {
		pInfo->pLineOfLogic = NULL;
	}

	// ������Q��
	const DocLine* pDocLine = pInfo->GetDocLine();
	StringRef lineStr = pDocLine->GetStringRefWithEOL();

	// �`��͈͊O�̏ꍇ�͐F�ؑւ����Ŕ�����
	if (pInfo->pDispPos->GetDrawPos().y < GetTextArea().GetAreaTop()) {
		if (pLayout) {
			bool bChange = false;
			int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
			Color3Setting cColor;
			while (pInfo->nPosInLogic < nPosTo) {
				// �F�ؑ�
				bChange |= pInfo->CheckChangeColor(lineStr);

				// 1�����i��
				pInfo->nPosInLogic += NativeW::GetSizeOfChar(
											lineStr.GetPtr(),
											lineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
			if (bChange) {
				pInfo->DoChangeColor(&cColor);
				SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}
		}
		return false;
	}

	// �R���t�B�O
	int nLineHeight = GetTextMetrics().GetHankakuDy();  // �s�̏c���H
	TypeSupport	caretLineBg(this, COLORIDX_CARETLINEBG);
	TypeSupport	evenLineBg(this, COLORIDX_EVENLINEBG);
	TypeSupport	pageViewBg(this, COLORIDX_PAGEVIEW);
	EditView& activeView = m_pEditWnd->GetActiveView();
	TypeSupport&	backType = (caretLineBg.IsDisp() &&
		GetCaret().GetCaretLayoutPos().GetY() == pInfo->pDispPos->GetLayoutLineRef() && !m_bMiniMap
			? caretLineBg
			: evenLineBg.IsDisp() && pInfo->pDispPos->GetLayoutLineRef() % 2 == 1 && !m_bMiniMap
				? evenLineBg
				: (pageViewBg.IsDisp() && m_bMiniMap
					&& activeView.GetTextArea().GetViewTopLine() <= pInfo->pDispPos->GetLayoutLineRef()
					&& pInfo->pDispPos->GetLayoutLineRef() < activeView.GetTextArea().GetBottomLine())
						? pageViewBg
						: textType);
	bool bTransText = IsBkBitmap();
	if (bTransText) {
		bTransText = backType.GetBackColor() == textType.GetBackColor();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �s�ԍ��`��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	GetTextDrawer().DispLineNumber(
		pInfo->gr,
		pInfo->pDispPos->GetLayoutLineRef(),
		pInfo->pDispPos->GetDrawPos().y
	);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �{���`��J�n                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pDispPos->ResetDrawCol();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 �s��(�C���f���g)�w�i�`��                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pLayout && pLayout->GetIndent() != 0) {
		RECT rcClip;
		if (!bTransText && GetTextArea().GenerateClipRect(&rcClip, *pInfo->pDispPos,(Int)pLayout->GetIndent())) {
			backType.FillBack(pInfo->gr, rcClip);
		}
		// �`��ʒu�i�߂�
		pInfo->pDispPos->ForwardDrawCol((Int)pLayout->GetIndent());
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �{���`��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bSkipRight = false; // ������`�悵�Ȃ��Ă����ꍇ�̓X�L�b�v����
	if (pLayout) {
		const Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutNext) {
			bSkipRight = true;
		}else if (pLayoutNext->GetLogicOffset() == 0) {
			bSkipRight = true; // ���̍s�͕ʂ̃��W�b�N�s�Ȃ̂ŃX�L�b�v�\
		}
		if (!bSkipRight) {
			bSkipRight = ColorStrategyPool::getInstance()->IsSkipBeforeLayout();
		}
	}
	// �s�I�[�܂��͐܂�Ԃ��ɒB����܂Ń��[�v
	if (pLayout) {
		int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
		FigureManager* pFigureManager = FigureManager::getInstance();
		while (pInfo->nPosInLogic < nPosTo) {
			// �F�ؑ�
			if (pInfo->CheckChangeColor(lineStr)) {
				Color3Setting cColor;
				pInfo->DoChangeColor(&cColor);
				SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}

			// 1�������擾 $$�������\
			Figure& figure = pFigureManager->GetFigure(&lineStr.GetPtr()[pInfo->GetPosInLogic()],
				lineStr.GetLength() - pInfo->GetPosInLogic());

			// 1�����`��
			figure.DrawImp(pInfo);
			if (bSkipRight && GetTextArea().GetAreaRight() < pInfo->pDispPos->GetDrawPos().x) {
				pInfo->nPosInLogic = nPosTo;
				break;
			}
		}
	}

	// �K�v�Ȃ�EOF�`��
	void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView* pView);
	if (pLayout && !pLayout->GetNextLayout() && pLayout->GetLayoutEol().GetLen() == 0) {
		// �L�����s��EOF
		_DispEOF(pInfo->gr, pInfo->pDispPos, this);
		bDispEOF = true;
	}else if (!pLayout && pInfo->pDispPos->GetLayoutLineRef() == m_pEditDoc->m_layoutMgr.GetLineCount()) {
		// ��s��EOF
		const Layout* pBottom = m_pEditDoc->m_layoutMgr.GetBottomLayout();
		if (!pBottom || (pBottom && pBottom->GetLayoutEol().GetLen())) {
			_DispEOF(pInfo->gr,pInfo->pDispPos,this);
			bDispEOF = true;
		}
	}

	// �K�v�Ȃ�܂�Ԃ��L���`��
	if (pLayout && pLayout->GetLayoutEol().GetLen() == 0 && pLayout->GetNextLayout()) {
		_DispWrap(pInfo->gr, pInfo->pDispPos, this, pInfo->pDispPos->GetLayoutLineRef());
	}

	// �s���w�i�`��
	RECT rcClip;
	bool rcClipRet = GetTextArea().GenerateClipRectRight(&rcClip, *pInfo->pDispPos);
	if (rcClipRet) {
		if (!bTransText) {
			backType.FillBack(pInfo->gr, rcClip);
		}
		TypeSupport selectType(this, COLORIDX_SELECT);
		if (GetSelectionInfo().IsTextSelected() && selectType.IsDisp()) {
			// �I��͈͂̎w��F�F�K�v�Ȃ�e�L�X�g�̂Ȃ������̋�`�I�������
			LayoutRange selectArea = GetSelectionInfo().GetSelectAreaLine(pInfo->pDispPos->GetLayoutLineRef(), pLayout);
			// 2010.10.04 �X�N���[�����̑����Y��
			int nSelectFromPx = GetTextMetrics().GetHankakuDx() * (Int)(selectArea.GetFrom().x - GetTextArea().GetViewLeftCol());
			int nSelectToPx   = GetTextMetrics().GetHankakuDx() * (Int)(selectArea.GetTo().x - GetTextArea().GetViewLeftCol());
			if (nSelectFromPx < nSelectToPx && selectArea.GetTo().x != INT_MAX) {
				RECT rcSelect; // Pixel
				rcSelect.top    = pInfo->pDispPos->GetDrawPos().y;
				rcSelect.bottom = pInfo->pDispPos->GetDrawPos().y + GetTextMetrics().GetHankakuDy();
				rcSelect.left   = GetTextArea().GetAreaLeft() + nSelectFromPx;
				rcSelect.right  = GetTextArea().GetAreaLeft() + nSelectToPx;
				RECT rcDraw;
				if (::IntersectRect(&rcDraw, &rcClip, &rcSelect)) {
					COLORREF color = GetBackColorByColorInfo2(selectType.GetColorInfo(), backType.GetColorInfo());
					if (color != backType.GetBackColor()) {
						pInfo->gr.FillSolidMyRect(rcDraw, color);
					}
				}
			}
		}
	}

	// �m�[�g���`��
	if (!m_bMiniMap) {
		GetTextDrawer().DispNoteLine(
			pInfo->gr,
			pInfo->pDispPos->GetDrawPos().y,
			pInfo->pDispPos->GetDrawPos().y + nLineHeight,
			GetTextArea().GetAreaLeft(),
			GetTextArea().GetAreaRight()
		);
	}

	// �w�茅�c���`��
	GetTextDrawer().DispVerticalLines(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		LayoutInt(0),
		LayoutInt(-1)
	);

	// �܂�Ԃ����c���`��
	if (!m_bMiniMap) {
		GetTextDrawer().DispWrapLine(
			pInfo->gr,
			pInfo->pDispPos->GetDrawPos().y,
			pInfo->pDispPos->GetDrawPos().y + nLineHeight
		);
	}

	// ���]�`��
	if (pLayout && GetSelectionInfo().IsTextSelected()) {
		DispTextSelected(
			pInfo->gr,
			pInfo->pDispPos->GetLayoutLineRef(),
			Point(pInfo->dispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
			pLayout->CalcLayoutWidth(EditDoc::GetInstance(0)->m_layoutMgr) + LayoutInt(pLayout->GetLayoutEol().GetLen() ? 1 : 0)
		);
	}

	return bDispEOF;
}



/* �e�L�X�g���]

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DrawLogicLine() �ł̍��(WM_PAINT)���ɁA1���C�A�E�g�s���܂Ƃ߂Ĕ��]�������邽�߂̊֐��B
	�͈͑I���̐����X�V�́AEditView::DrawSelectArea() ���I���E���]�������s���B
	
*/
void EditView::DispTextSelected(
	HDC				hdc,		// ���Ώۃr�b�g�}�b�v���܂ރf�o�C�X
	LayoutInt		nLineNum,	// ���]�����Ώۃ��C�A�E�g�s�ԍ�(0�J�n)
	const Point&	ptXY,		// (���΃��C�A�E�g0���ڂ̍��[���W, �Ώۍs�̏�[���W)
	LayoutInt		nX_Layout	// �Ώۍs�̏I�����ʒu�B�@[ABC\n]�Ȃ���s�̌���4
)
{
	LayoutInt	nSelectFrom;
	LayoutInt	nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(nLineNum);
	LayoutRange& select = GetSelectionInfo().m_select;

	// �I��͈͓��̍s����
//	if (IsTextSelected()) {
		if (nLineNum >= select.GetFrom().y && nLineNum <= select.GetTo().y) {
			LayoutRange selectArea = GetSelectionInfo().GetSelectAreaLine(nLineNum, pLayout);
			nSelectFrom = selectArea.GetFrom().x;
			nSelectTo   = selectArea.GetTo().x;
			if (nSelectFrom == INT_MAX) {
				nSelectFrom = nX_Layout;
			}
			if (nSelectTo == INT_MAX) {
				nSelectTo = nX_Layout;
			}

			// 2006.03.28 Moca �\����O�Ȃ牽�����Ȃ�
			if (GetTextArea().GetRightCol() < nSelectFrom) {
				return;
			}
			if (nSelectTo < GetTextArea().GetViewLeftCol()) {	// nSelectTo == GetTextArea().GetViewLeftCol()�̃P�[�X�͌�łO�����}�b�`�łȂ����Ƃ��m�F���Ă��甲����
				return;
			}

			if (nSelectFrom < GetTextArea().GetViewLeftCol()) {
				nSelectFrom = GetTextArea().GetViewLeftCol();
			}
			rcClip.left   = ptXY.x + (Int)nSelectFrom * nCharWidth;
			rcClip.right  = ptXY.x + (Int)nSelectTo   * nCharWidth;
			rcClip.top    = ptXY.y;
			rcClip.bottom = ptXY.y + nLineHeight;

			bool bOMatch = false;

			// 2005/04/02 ����� �O�����}�b�`���Ɣ��]�����O�ƂȂ蔽�]����Ȃ��̂ŁA1/3�������������]������
			// 2005/06/26 zenryaku �I�������ŃL�����b�g�̎c�[���c������C��
			// 2005/09/29 ryoji �X�N���[�����ɃL�����b�g�̂悤�ȃS�~���\�����������C��
			if (GetSelectionInfo().IsTextSelected() && rcClip.right == rcClip.left &&
				select.IsLineOne() &&
				select.GetFrom().x >= GetTextArea().GetViewLeftCol()
			) {
				HWND hWnd = ::GetForegroundWindow();
				if (hWnd && (hWnd == m_pEditWnd->m_dlgFind.GetHwnd() || hWnd == m_pEditWnd->m_dlgReplace.GetHwnd())) {
					rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
					bOMatch = true;
				}
			}
			if (rcClip.right == rcClip.left) {
				return;	// �O�����}�b�`�ɂ�锽�]���g���Ȃ�
			}

			// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
			if (rcClip.right > GetTextArea().GetAreaRight()) {
				rcClip.right = GetTextArea().GetAreaRight();
			}
			
			// �I��F�\���Ȃ甽�]���Ȃ�
			if (!bOMatch && TypeSupport(this, COLORIDX_SELECT).IsDisp()) {
				return;
			}
			
			HBRUSH hBrush    = ::CreateSolidBrush(SELECTEDAREA_RGB);

			int    nROP_Old  = ::SetROP2(hdc, SELECTEDAREA_ROP2);
			HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
			hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			::DeleteObject(hrgnDraw);

			SetROP2(hdc, nROP_Old);
			SelectObject(hdc, hBrushOld);
			DeleteObject(hBrush);
		}
//	}
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ��ʃo�b�t�@                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	��ʂ̌݊��r�b�g�}�b�v���쐬�܂��͍X�V����B
		�K�v�̖����Ƃ��͉������Ȃ��B
	
	@param cx �E�B���h�E�̍���
	@param cy �E�B���h�E�̕�
	@return true: �r�b�g�}�b�v�𗘗p�\ / false: �r�b�g�}�b�v�̍쐬�E�X�V�Ɏ��s

	@date 2007.09.09 Moca EditView::OnSize���番���B
		�P���ɐ������邾�����������̂��A�d�l�ύX�ɏ]�����e�R�s�[��ǉ��B
		�T�C�Y�������Ƃ��͉������Ȃ��悤�ɕύX

	@par �݊�BMP�ɂ̓L�����b�g�E�J�[�\���ʒu���c���E�Ί��ʈȊO�̏���S�ď������ށB
		�I��͈͕ύX���̔��]�����́A��ʂƌ݊�BMP�̗�����ʁX�ɕύX����B
		�J�[�\���ʒu���c���ύX���ɂ́A�݊�BMP�����ʂɌ��̏��𕜋A�����Ă���B

*/
bool EditView::CreateOrUpdateCompatibleBitmap(int cx, int cy)
{
	if (!m_hdcCompatDC) {
		return false;
	}
	// �T�C�Y��64�̔{���Ő���
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if (nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight) {
#if 0
	MYTRACE(_T("EditView::CreateOrUpdateCompatibleBitmap(%d, %d): resized\n"), cx, cy);
#endif
		HDC	hdc = ::GetDC(GetHwnd());
		HBITMAP hBitmapNew = NULL;
		if (m_hbmpCompatBMP) {
			// BMP�̍X�V
			HDC hdcTemp = ::CreateCompatibleDC(hdc);
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
			if (hBitmapNew) {
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdcTemp, hBitmapNew);
				// �O�̉�ʓ��e���R�s�[����
				::BitBlt(hdcTemp, 0, 0,
					t_min(nBmpWidthNew, m_nCompatBMPWidth),
					t_min(nBmpHeightNew, m_nCompatBMPHeight),
					m_hdcCompatDC, 0, 0, SRCCOPY);
				::SelectObject(hdcTemp, hBitmapOld);
				::SelectObject(m_hdcCompatDC, m_hbmpCompatBMPOld);
				::DeleteObject(m_hbmpCompatBMP);
			}
			::DeleteDC(hdcTemp);
		}else {
			// BMP�̐V�K�쐬
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
		}
		if (hBitmapNew) {
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject(m_hdcCompatDC, m_hbmpCompatBMP);
		}else {
			// �݊�BMP�̍쐬�Ɏ��s
			// ��������s���J��Ԃ��\���������̂�
			// m_hdcCompatDC��NULL�ɂ��邱�Ƃŉ�ʃo�b�t�@�@�\�����̃E�B���h�E�̂ݖ����ɂ���B
			//	2007.09.29 genta �֐����D������BMP�����
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC(GetHwnd(), hdc);
	}
	return m_hbmpCompatBMP != NULL;
}


/*!
	�݊�������BMP���폜

	@note �����r���[����\���ɂȂ����ꍇ��
		�e�E�B���h�E����\���E�ŏ������ꂽ�ꍇ�ɍ폜�����B
	@date 2007.09.09 Moca �V�K�쐬 
*/
void EditView::DeleteCompatibleBitmap()
{
	if (m_hbmpCompatBMP) {
		::SelectObject(m_hdcCompatDC, m_hbmpCompatBMPOld);
		::DeleteObject(m_hbmpCompatBMP);
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}


/** ��ʃL���b�V���pCompatibleDC��p�ӂ���

	@param[in] TRUE: ��ʃL���b�V��ON

	@date 2007.09.30 genta �֐���
*/
void EditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if (fCache) {
		if (!m_hdcCompatDC) {
			HDC hdc = ::GetDC(GetHwnd());
			m_hdcCompatDC = ::CreateCompatibleDC(hdc);
			::ReleaseDC(GetHwnd(), hdc);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Created\n"), fCache);
		}else {
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}else {
		//	CompatibleBitmap���c���Ă��邩������Ȃ��̂ōŏ��ɍ폜
		DeleteCompatibleBitmap();
		if (m_hdcCompatDC) {
			::DeleteDC(m_hdcCompatDC);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Deleted.\n"));
			m_hdcCompatDC = NULL;
		}
	}
}

