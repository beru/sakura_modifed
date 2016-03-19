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
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_Eol.h"
#include "types/TypeSupport.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"

// �܂�Ԃ��`��
void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView* pView);

// EOF�`��֐�
// ���ۂɂ� pX �� nX ���X�V�����B
// 2004.05.29 genta
// 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
// 2007.08.25 kobake �������� nCharWidth, nLineHeight ���폜
// 2007.08.28 kobake ���� fuOptions ���폜
//void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView* pView, bool bTrans);

// ���s�L���`��
// 2007.08.30 kobake �ǉ�
void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView* pView, bool bTrans);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        Figure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_Eol::Match(const wchar_t* pText, int nTextLen) const
{
	// 2014.06.18 �܂�Ԃ��E�ŏI�s����DrawImp��eol.GetLen()==0�ɂȂ薳�����[�v����̂�
	// �������s�̓r���ɉ��s�R�[�h���������ꍇ��Match�����Ȃ�
	if (nTextLen == 2 && pText[0] == L'\r' && pText[1] == L'\n') return true;
	if (nTextLen == 1 && WCODE::IsLineDelimiterExt(pText[0])) return true;
	return false;
}

// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
//$$ �������\�B
bool Figure_Eol::DrawImp(ColorStrategyInfo* pInfo)
{
	EditView* pView = pInfo->pView;

	// ���s�擾
	const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
	Eol eol = pLayout->GetLayoutEol();
	if (eol.GetLen()) {
		// CFigureSpace::DrawImp_StyleSelect���ǂ��B�I���E�����F��D�悷��
		TypeSupport currentType(pView, pInfo->GetCurrentColor());	// ���ӂ̐F�i���݂̎w��F/�I��F�j
		TypeSupport currentType2(pView, pInfo->GetCurrentColor2());	// ���ӂ̐F�i���݂̎w��F�j
		TypeSupport textType(pView, COLORIDX_TEXT);				// �e�L�X�g�̎w��F
		TypeSupport spaceType(pView, GetDispColorIdx());	// �󔒂̎w��F
		TypeSupport searchType(pView, COLORIDX_SEARCH);	// �����F(EOL�ŗL)
		TypeSupport currentTypeBg(pView, pInfo->GetCurrentColorBg());
		TypeSupport& currentType3 = (currentType2.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType2);
		COLORREF crText;
		COLORREF crBack;
		bool bSelecting = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2();
		bool blendColor = bSelecting && currentType.GetTextColor() == currentType.GetBackColor(); // �I�������F
		TypeSupport& currentStyle = blendColor ? currentType2 : currentType;
		TypeSupport *pcText, *pcBack;
		if (bSelecting && !blendColor) {
			// �I�𕶎��F�Œ�w��
			pcText = &currentType;
			pcBack = &currentType;
		}else if (pInfo->GetCurrentColor2() == COLORIDX_SEARCH) {
			// �����F�D��
			pcText = &searchType;
			pcBack = &searchType;
		}else {
			pcText = spaceType.GetTextColor() == textType.GetTextColor() ? &currentType2 : &spaceType;
			pcBack = spaceType.GetBackColor() == textType.GetBackColor() ? &currentType3 : &spaceType;
		}
		if (blendColor) {
			// �����F(�����F��D�悵��)
			crText = pView->GetTextColorByColorInfo2(currentType.GetColorInfo(), pcText->GetColorInfo());
			crBack = pView->GetBackColorByColorInfo2(currentType.GetColorInfo(), pcBack->GetColorInfo());
		}else {
			crText = pcText->GetTextColor();
			crBack = pcBack->GetBackColor();
		}
		pInfo->gr.PushTextForeColor(crText);
		pInfo->gr.PushTextBackColor(crBack);
		bool bTrans = pView->IsBkBitmap() && textType.GetBackColor() == crBack;
		Font font;
		font.fontAttr.bBoldFont = spaceType.IsBoldFont() || currentStyle.IsBoldFont();
		font.fontAttr.bUnderLine = spaceType.HasUnderLine();
		font.hFont = pInfo->pView->GetFontset().ChooseFontHandle(font.fontAttr);
		pInfo->gr.PushMyFont(font);

		DispPos pos(*pInfo->pDispPos);	// ���݈ʒu���o���Ă���
		_DispEOL(pInfo->gr, pInfo->pDispPos, eol, pView, bTrans);
		DrawImp_StylePop(pInfo);
		DrawImp_DrawUnderline(pInfo, pos);

		pInfo->nPosInLogic += eol.GetLen();
	}else {
		// �������[�v�΍�
		pInfo->nPosInLogic += 1;
		assert_warning( 1 );
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �܂�Ԃ��`�����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �܂�Ԃ��`��
void _DispWrap(
	Graphics&		gr,
	DispPos*		pDispPos,
	const EditView*	pView,
	LayoutYInt		nLineNum
	)
{
	RECT rcClip2;
	if (pView->GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 1)) {
		// �T�|�[�g�N���X
		TypeSupport wrapType(pView, COLORIDX_WRAP);
		TypeSupport textType(pView, COLORIDX_TEXT);
		TypeSupport bgLineType(pView, COLORIDX_CARETLINEBG);
		TypeSupport evenBgLineType(pView, COLORIDX_EVENLINEBG);
		TypeSupport pageViewBgLineType(pView,COLORIDX_PAGEVIEW);
		bool bBgcolor = wrapType.GetBackColor() == textType.GetBackColor();
		EColorIndexType bgColorOverwrite = COLORIDX_WRAP;
		bool bTrans = pView->IsBkBitmap();
		if (wrapType.IsDisp()) {
			EditView& activeView = pView->m_pEditWnd->GetActiveView();
			if (bgLineType.IsDisp() && pView->GetCaret().GetCaretLayoutPos().GetY2() == nLineNum) {
				if (bBgcolor) {
					bgColorOverwrite = COLORIDX_CARETLINEBG;
					bTrans = bTrans && bgLineType.GetBackColor() == textType.GetBackColor();
				}
			}else if (evenBgLineType.IsDisp() && nLineNum % 2 == 1) {
				if (bBgcolor) {
					bgColorOverwrite = COLORIDX_EVENLINEBG;
					bTrans = bTrans && evenBgLineType.GetBackColor() == textType.GetBackColor();
				}
			}else if (
				pView->m_bMiniMap
				&& activeView.GetTextArea().GetViewTopLine() <= nLineNum
				&& nLineNum < activeView.GetTextArea().GetBottomLine()
			) {
				bgColorOverwrite = COLORIDX_PAGEVIEW;
				bTrans = bTrans && pageViewBgLineType.GetBackColor() == textType.GetBackColor();
			}
		}
		bool bChangeColor = false;

		// �`�敶����ƐF�̌���
		const wchar_t* szText;
		if (wrapType.IsDisp()) {
			szText = L"<";
			wrapType.SetGraphicsState_WhileThisObj(gr);
			if (bgColorOverwrite != COLORIDX_WRAP) {
				bChangeColor = true;
				gr.PushTextBackColor(TypeSupport(pView, bgColorOverwrite).GetBackColor());
			}
		}else {
			szText = L" ";
		}

		// �`��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			szText,
			wcslen(szText),
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		if (bChangeColor) {
			gr.PopTextBackColor();
		}
	}
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       EOF�`�����                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
EOF�L���̕`��
@date 2004.05.29 genta  MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
@date 2007.08.28 kobake ���� nCharWidth �폜
@date 2007.08.28 kobake ���� fuOptions �폜
@date 2007.08.30 kobake ���� EofColInfo �폜
*/
void _DispEOF(
	Graphics&			gr,			// [in] �`��Ώۂ�Device Context
	DispPos*			pDispPos,	// [in] �\�����W
	const EditView*		pView
	)
{
	// �`��Ɏg���F���
	TypeSupport eofType(pView, COLORIDX_EOF);
	if (!eofType.IsDisp()) {
		return;
	}
	TypeSupport textType(pView, COLORIDX_TEXT);
	bool bTrans = pView->IsBkBitmap() && eofType.GetBackColor() == textType.GetBackColor();

	// �K�v�ȃC���^�[�t�F�[�X���擾
	const TextMetrics* pMetrics = &pView->GetTextMetrics();
	const TextArea* pArea = &pView->GetTextArea();

	// �萔
	static const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	// �N���b�s���O�̈���v�Z
	RECT rcClip;
	if (pArea->GenerateClipRect(&rcClip, *pDispPos, nEofLen)) {
		// �F�ݒ�
		eofType.SetGraphicsState_WhileThisObj(gr);

		// �`��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip,
			szEof,
			nEofLen,
			pMetrics->GetDxArray_AllHankaku()
		);
	}

	// �`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nEofLen);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���s�`�����                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ��ʕ`��⏕�֐�
// May 23, 2000 genta
//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
void _DrawEOL(
	Graphics&		gr,
	const Rect&		rcEol,
	Eol				eol,
	bool			bBold,
	COLORREF		pColor
);

// 2007.08.30 kobake �ǉ�
void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView* pView, bool bTrans)
{
	RECT rcClip2;
	if (pView->GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 2)) {
		// 2003.08.17 ryoji ���s�����������Ȃ��悤��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			L"  ",
			2,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// ���s�L���̕\��
		if (TypeSupport(pView, COLORIDX_EOL).IsDisp()) {
			// From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��

			// ���[�W�����쐬�A�I���B
			gr.SetClipping(rcClip2);
			
			// �`��̈�
			Rect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(pView->GetTextMetrics().GetHankakuWidth(), pView->GetTextMetrics().GetHankakuHeight());

			// �`��
			// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�	// 2009.05.29 ryoji 
			// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
			// 2013.06.21 novice �����F�A������Graphics����擾
			_DrawEOL(gr, rcEol, eol, gr.GetCurrentMyFontBold(), gr.GetCurrentTextForeColor());

			// ���[�W�����j��
			gr.ClearClipping();

			// To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
		}
	}

	// �`��ʒu��i�߂�(2��)
	pDispPos->ForwardDrawCol(2);
}


//	May 23, 2000 genta
/*!
��ʕ`��⏕�֐�:
�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

@note bBold��true�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
���܂葾�������Ȃ��B

@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
						���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void _DrawEOL(
	Graphics&		gr,			// Device Context Handle
	const Rect&		rcEol,		// �`��̈�
	Eol				eol,		// �s���R�[�h���
	bool			bBold,		// true: ����
	COLORREF		pColor		// �F
	)
{
	int sx, sy;	// ���̐擪
	gr.SetPen(pColor);

	switch (eol.GetType()) {
	case EolType::CRLF:	// �������
		{
			sx = rcEol.left;						// X���[
			sy = rcEol.top + (rcEol.Height() / 2);	// Y���S
			DWORD pp[] = { 3, 3 };
			POINT pt[6];
			pt[0].x = sx + rcEol.Width();	// ���
			pt[0].y = sy - rcEol.Height() / 4;
			pt[1].x = sx + rcEol.Width();	// ����
			pt[1].y = sy;
			pt[2].x = sx;	// �擪��
			pt[2].y = sy;
			pt[3].x = sx + rcEol.Height() / 4;	// �擪���牺��
			pt[3].y = sy + rcEol.Height() / 4;
			pt[4].x = sx;	// �擪�֖߂�
			pt[4].y = sy;
			pt[5].x = sx + rcEol.Height() / 4;	// �擪������
			pt[5].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 1;	// ��ցi�E�ւ��炷�j
				pt[0].y += 0;
				pt[1].x += 1;	// �E�ցi�E�ɂЂƂ���Ă���j
				pt[1].y += 1;
				pt[2].x += 0;	// �擪��
				pt[2].y += 1;
				pt[3].x += 0;	// �擪���牺��
				pt[3].y += 1;
				pt[4].x += 0;	// �擪�֖߂�
				pt[4].y += 1;
				pt[5].x += 0;	// �擪������
				pt[5].y += 1;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::CR:	// ���������	// 2007.08.17 ryoji EolType::LF -> EolType::CR
		{
			sx = rcEol.left;
			sy = rcEol.top + (rcEol.Height() / 2);
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx + rcEol.Width();	// �E��
			pt[0].y = sy;
			pt[1].x = sx;	// �擪��
			pt[1].y = sy;
			pt[2].x = sx + rcEol.Height() / 4;	// �擪���牺��
			pt[2].y = sy + rcEol.Height() / 4;
			pt[3].x = sx;	// �擪�֖߂�
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	// �擪������
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 0;	// �E��
				pt[0].y += 1;
				pt[1].x += 0;	// �擪��
				pt[1].y += 1;
				pt[2].x += 0;	// �擪���牺��
				pt[2].y += 1;
				pt[3].x += 0;	// �擪�֖߂�
				pt[3].y += 1;
				pt[4].x += 0;	// �擪������
				pt[4].y += 1;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::LF:	// ���������	// 2007.08.17 ryoji EolType::CR -> EolType::LF
	// 2013.04.22 Moca NEL,LS,PS�Ή��B�b���LF�Ɠ����ɂ���
		{
			sx = rcEol.left + (rcEol.Width() / 2);
			sy = rcEol.top + (rcEol.Height() * 3 / 4);
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx;	// ���
			pt[0].y = rcEol.top + rcEol.Height() / 4 + 1;
			pt[1].x = sx;	// �ォ�牺��
			pt[1].y = sy;
			pt[2].x = sx - rcEol.Height() / 4;	// ���̂܂܍����
			pt[2].y = sy - rcEol.Height() / 4;
			pt[3].x = sx;	// ���̐�[�ɖ߂�
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	// �����ĉE���
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 1;	// ���
				pt[0].y += 0;
				pt[1].x += 1;	// �ォ�牺��
				pt[1].y += 0;
				pt[2].x += 1;	// ���̂܂܍����
				pt[2].y += 0;
				pt[3].x += 1;	// ���̐�[�ɖ߂�
				pt[3].y += 0;
				pt[4].x += 1;	// �����ĉE���
				pt[4].y += 0;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::NEL:
	case EolType::LS:
	case EolType::PS:
		{
			// �������(�܂�Ȃ���Ȃ�)
			sx = rcEol.left;			//X���[
			sy = rcEol.top + ( rcEol.Height() * 3 / 4 );	//Y�ォ��3/4
			DWORD pp[] = { 2, 3 };
			POINT pt[5];
			int nWidth = t_min(rcEol.Width(), rcEol.Height() / 2);
			pt[0].x = sx + nWidth;	//	�E�ォ��
			pt[0].y = sy - nWidth;
			pt[1].x = sx;	//	�擪��
			pt[1].y = sy;
			pt[2].x = sx + nWidth;	//	�E����
			pt[2].y = sy;
			pt[3].x = sx;	//	�擪�֖߂�
			pt[3].y = sy;
			pt[4].x = sx;	//	�擪������
			pt[4].y = sy - nWidth;
			::PolyPolyline( gr, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	�E�ォ��
				pt[0].y += 1;
				pt[1].x += 0;	//	�擪��
				pt[1].y += 1;
				pt[2].x += 0;	//	�E����
				pt[2].y -= 1;
				pt[3].x += 1;	//	�擪�֖߂�
				pt[3].y -= 1;
				pt[4].x += 1;	//	�擪������
				pt[4].y += 0;
				::PolyPolyline( gr, pt, pp, _countof(pp));
			}
		}
		break;
	}
}

