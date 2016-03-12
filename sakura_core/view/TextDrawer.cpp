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
#include "TextDrawer.h"
#include <vector>
#include "TextMetrics.h"
#include "TextArea.h"
#include "ViewFont.h"
#include "Eol.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"
#include "charset/charcode.h"
#include "doc/layout/Layout.h"

const TextArea& TextDrawer::GetTextArea() const
{
	return m_pEditView->GetTextArea();
}

using namespace std;


/*
�e�L�X�g�\��
@@@ 2002.09.22 YAZAKI    const unsigned char* pData���Aconst char* pData�ɕύX
@@@ 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
*/
void TextDrawer::DispText(
	HDC			hdc,
	DispPos*	pDispPos,
	const wchar_t* pData,
	int			nLength,
	bool		bTransparent
	) const
{
	if (0 >= nLength) {
		return;
	}
	int x = pDispPos->GetDrawPos().x;
	int y = pDispPos->GetDrawPos().y;

	// �K�v�ȃC���^�[�t�F�[�X���擾
	const TextMetrics* pMetrics = &m_pEditView->GetTextMetrics();
	const TextArea& textArea = GetTextArea();

	// �����Ԋu�z��𐶐�
	static vector<int> vDxArray(1);
	const int* pDxArray = pMetrics->GenerateDxArray(&vDxArray, pData, nLength, this->m_pEditView->GetTextMetrics().GetHankakuDx());

	// ������̃s�N�Z����
	int nTextWidth = pMetrics->CalcTextWidth(pData, nLength, pDxArray);

	// �e�L�X�g�̕`��͈͂̋�`�����߂� -> rcClip
	Rect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + m_pEditView->GetTextMetrics().GetHankakuDy();
	if (rcClip.left < textArea.GetAreaLeft()) {
		rcClip.left = textArea.GetAreaLeft();
	}

	// �����Ԋu
	int nDx = m_pEditView->GetTextMetrics().GetHankakuDx();

	if (textArea.IsRectIntersected(rcClip) && rcClip.top >= textArea.GetAreaTop()) {

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW_AnyBuild�̐������
		if (rcClip.Width() > textArea.GetAreaWidth()) {
			rcClip.right = rcClip.left + textArea.GetAreaWidth();
		}

		// �E�B���h�E�̍��ɂ��ӂꂽ������ -> nBefore
		// 2007.09.08 kobake�� �u�E�B���h�E�̍��v�ł͂Ȃ��u�N���b�v�̍��v�����Ɍv�Z�����ق����`��̈��ߖ�ł��邪�A
		//                        �o�O���o��̂��|���̂łƂ肠�������̂܂܁B
		int nBeforeLogic = 0;
		LayoutInt nBeforeLayout = LayoutInt(0);
		if (x < 0) {
			int nLeftLayout = (0 - x) / nDx - 1;
			while (nBeforeLayout < nLeftLayout) {
				nBeforeLayout += NativeW::GetKetaOfChar(pData, nLength, nBeforeLogic);
				nBeforeLogic  += NativeW::GetSizeOfChar(pData, nLength, nBeforeLogic);
			}
		}

		/*
		// �E�B���h�E�̉E�ɂ��ӂꂽ������ -> nAfter
		int nAfterLayout = 0;
		if (rcClip.right < x + nTextWidth) {
			//	-1���Ă��܂����i������͂�����ˁH�j
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// �`��J�n�ʒu
		int nDrawX = x + (Int)nBeforeLayout * nDx;

		// ���ۂ̕`�敶����|�C���^
		const wchar_t*	pDrawData			= &pData[nBeforeLogic];
		int				nDrawDataMaxLength	= nLength - nBeforeLogic;

		// ���ۂ̕����Ԋu�z��
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// �`�悷�镶���񒷂����߂� -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; // ���߂�ׂ��s�N�Z����
		if (nRequiredWidth <= 0) {
			goto end;
		}
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while (nWorkWidth < nRequiredWidth) {
			if (nDrawLength >= nDrawDataMaxLength) {
				break;
			}
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}
		// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi	Update 7/8 Uchi
		if (nDrawLength < nDrawDataMaxLength && pDrawDxArray[nDrawLength] == 0) {
			++nDrawLength;
		}

		// �`��
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					// X
			y,						// Y
			ExtTextOutOption() & ~(bTransparent? ETO_OPAQUE: 0),
			&rcClip,
			pDrawData,				// ������
			nDrawLength,			// ������
			pDrawDxArray			// �����Ԋu�̓������z��
		);
	}

end:
	// �`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �w�茅�c��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�w�茅�c���̕`��
	@date 2005.11.08 Moca �V�K�쐬
	@date 2006.04.29 Moca �����E�_���̃T�|�[�g�B�I�𒆂̔��]�΍�ɍs���Ƃɍ�悷��悤�ɕύX
	    �c���̐F���e�L�X�g�̔w�i�F�Ɠ����ꍇ�́A�c���̔w�i�F��EXOR�ō�悷��
	@note Common::m_nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void TextDrawer::DispVerticalLines(
	Graphics&	gr,			// ��悷��E�B���h�E��DC
	int			nTop,		// ����������[�̃N���C�A���g���Wy
	int			nBottom,	// �����������[�̃N���C�A���g���Wy
	LayoutInt	nLeftCol,	// ���������͈͂̍����̎w��
	LayoutInt	nRightCol	// ���������͈͂̉E���̎w��(-1�Ŗ��w��)
	) const
{
	const EditView* pView = m_pEditView;
	
	const TypeConfig& typeData = pView->m_pEditDoc->m_docType.GetDocumentAttribute();
	
	TypeSupport vertType(pView, COLORIDX_VERTLINE);
	TypeSupport textType(pView, COLORIDX_TEXT);
	
	if (!vertType.IsDisp()) {
		return;
	}
	
	auto& textArea = pView->GetTextArea();
	nLeftCol = t_max(textArea.GetViewLeftCol(), nLeftCol);
	
	const LayoutInt nWrapKetas  = pView->m_pEditDoc->m_layoutMgr.GetMaxLineKetas();
	const int nCharDx  = pView->GetTextMetrics().GetHankakuDx();
	if (nRightCol < 0) {
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = GetDllShareData().common.window.nVertLineOffset + textArea.GetAreaLeft();
	const int nPosXLeft   = t_max(textArea.GetAreaLeft() + (Int)(nLeftCol  - textArea.GetViewLeftCol()) * nCharDx, textArea.GetAreaLeft());
	const int nPosXRight  = t_min(textArea.GetAreaLeft() + (Int)(nRightCol - textArea.GetViewLeftCol()) * nCharDx, textArea.GetAreaRight());
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)textArea.GetViewTopLine() : 0) + textArea.GetAreaTop() + nTop) % 2 == 1);

	// ����
	const bool bBold = vertType.IsBoldFont();
	// �h�b�g��(����������]�p/�e�X�g�p)
	const bool bDot = vertType.HasUnderLine();
	const bool bExorPen = (vertType.GetTextColor() == textType.GetBackColor());
	int nROP_Old = 0;
	if (bExorPen) {
		gr.SetPen(vertType.GetBackColor());
		nROP_Old = ::SetROP2(gr, R2_NOTXORPEN);
	}else {
		gr.SetPen(vertType.GetTextColor());
	}

	for (int k=0; k<MAX_VERTLINES && typeData.nVertLineIdx[k]!=0; ++k) {
		// nXCol��1�J�n�BGetTextArea().GetViewLeftCol()��0�J�n�Ȃ̂Œ��ӁB
		LayoutInt nXCol = typeData.nVertLineIdx[k];
		LayoutInt nXColEnd = nXCol;
		LayoutInt nXColAdd = LayoutInt(1);
		// nXCol���}�C�i�X���ƌJ��Ԃ��Bk+1���I���l�Ak+2���X�e�b�v���Ƃ��ė��p����
		if (nXCol < 0) {
			if (k < MAX_VERTLINES - 2) {
				nXCol = -nXCol;
				nXColEnd = typeData.nVertLineIdx[++k];
				nXColAdd = typeData.nVertLineIdx[++k];
				if (nXColEnd < nXCol || nXColAdd <= 0) {
					continue;
				}
				// ���͈͂̎n�߂܂ŃX�L�b�v
				if (nXCol < textArea.GetViewLeftCol()) {
					nXCol = textArea.GetViewLeftCol() + nXColAdd - (textArea.GetViewLeftCol() - nXCol) % nXColAdd;
				}
			}else {
				k += 2;
				continue;
			}
		}
		for (; nXCol<=nXColEnd; nXCol+=nXColAdd) {
			if (nWrapKetas < nXCol) {
				break;
			}
			int nPosX = nPosXOffset + (Int)(nXCol - 1 - textArea.GetViewLeftCol()) * nCharDx;
			// 2006.04.30 Moca ���̈����͈́E���@��ύX
			// �����̏ꍇ�A����������悷��\��������B
			int nPosXBold = nPosX;
			if (bBold) {
				nPosXBold -= 1;
			}
			if (nPosXRight <= nPosXBold) {
				break;
			}
			if (nPosXLeft <= nPosX) {
				if (bDot) {
					// �_���ō��B1�h�b�g�̐����쐬
					int y = nTop;
					// �X�N���[�����Ă������؂�Ȃ��悤�ɍ��W�𒲐�
					if (bOddLine) {
						++y;
					}
					for (; y<nBottom; y+=2) {
						if (nPosX < nPosXRight) {
							::MoveToEx(gr, nPosX, y, NULL);
							::LineTo(gr, nPosX, y + 1);
						}
						if (bBold && nPosXLeft <= nPosXBold) {
							::MoveToEx(gr, nPosXBold, y, NULL);
							::LineTo(gr, nPosXBold, y + 1);
						}
					}
				}else {
					if (nPosX < nPosXRight) {
						::MoveToEx(gr, nPosX, nTop, NULL);
						::LineTo(gr, nPosX, nBottom);
					}
					if (bBold && nPosXLeft <= nPosXBold) {
						::MoveToEx(gr, nPosXBold, nTop, NULL);
						::LineTo(gr, nPosXBold, nBottom);
					}
				}
			}
		}
	}
	if (bExorPen) {
		::SetROP2(gr, nROP_Old);
	}
}

void TextDrawer::DispNoteLine(
	Graphics&	gr,			// ��悷��E�B���h�E��DC
	int			nTop,		// ����������[�̃N���C�A���g���Wy
	int			nBottom,	// �����������[�̃N���C�A���g���Wy
	int			nLeft,		// �����������[
	int			nRight		// ���������E�[
	) const
{
	const EditView* pView = m_pEditView;

	TypeSupport noteLine(pView, COLORIDX_NOTELINE);
	if (noteLine.IsDisp()) {
		gr.SetPen(noteLine.GetTextColor());
		const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
		const int left = nLeft;
		const int right = nRight;
		int userOffset = pView->m_pTypeData->nNoteLineOffset;
		int offset = pView->GetTextArea().GetAreaTop() + userOffset - 1;
		while (offset < 0) {
			offset += nLineHeight;
		}
		int offsetMod = offset % nLineHeight;
		int y = ((nTop - offset) / nLineHeight * nLineHeight) + offsetMod;
		for (; y<nBottom; y+=nLineHeight) {
			if (nTop <= y) {
				::MoveToEx( gr, left, y, NULL );
				::LineTo( gr, right, y );
			}
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �܂�Ԃ����c��                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�܂�Ԃ����c���̕`��
	@date 2009.10.24 ryoji �V�K�쐬
*/
void TextDrawer::DispWrapLine(
	Graphics&	gr,			// ��悷��E�B���h�E��DC
	int			nTop,		// ����������[�̃N���C�A���g���Wy
	int			nBottom		// �����������[�̃N���C�A���g���Wy
	) const
{
	const EditView* pView = m_pEditView;
	TypeSupport wrapType(pView, COLORIDX_WRAP);
	if (!wrapType.IsDisp()) return;

	const TextArea& textArea = GetTextArea();
	const LayoutInt nWrapKetas = pView->m_pEditDoc->m_layoutMgr.GetMaxLineKetas();
	const int nCharDx = pView->GetTextMetrics().GetHankakuDx();
	int nXPos = textArea.GetAreaLeft() + (Int)(nWrapKetas - textArea.GetViewLeftCol()) * nCharDx;
	//	2005.11.08 Moca �������ύX
	if (textArea.GetAreaLeft() < nXPos && nXPos < textArea.GetAreaRight()) {
		/// �܂�Ԃ��L���̐F�̃y����ݒ�
		gr.PushPen(wrapType.GetTextColor(), 0);

		::MoveToEx(gr, nXPos, nTop, NULL);
		::LineTo(gr, nXPos, nBottom);

		gr.PopPen();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �s�ԍ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void TextDrawer::DispLineNumber(
	Graphics&		gr,
	LayoutInt		nLineNum,
	int				y
	) const
{
	//$$ �������FSearchLineByLayoutY�ɃL���b�V������������
	const Layout*	pLayout = EditDoc::GetInstance(0)->m_layoutMgr.SearchLineByLayoutY(nLineNum);

	const EditView* pView = m_pEditView;
	const TypeConfig& typeConfig = pView->m_pEditDoc->m_docType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	// �s�ԍ��\������X��	Sep. 23, 2002 genta ���ʎ��̂����肾��
	//int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;
	int				nLineNumAreaWidth = pView->GetTextArea().GetAreaLeft() - GetDllShareData().common.window.nLineNumRightSpace;	// 2009.03.26 ryoji

	TypeSupport textType(pView, COLORIDX_TEXT);
	TypeSupport caretLineBg(pView, COLORIDX_CARETLINEBG);
	TypeSupport evenLineBg(pView, COLORIDX_EVENLINEBG);
	// �s���Ȃ��Ƃ��E�s�̔w�i�������̂Ƃ��̐F
	TypeSupport& backType = (caretLineBg.IsDisp() &&
		pView->GetCaret().GetCaretLayoutPos().GetY() == nLineNum
			? caretLineBg
			: evenLineBg.IsDisp() && nLineNum % 2 == 1
				? evenLineBg
				: textType);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndex������                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	// �s�ԍ�
	const DocLine*	pDocLine = NULL;
	bool bGyouMod = false;
	if (pLayout) {
		pDocLine = pLayout->GetDocLineRef();

		if (1
			&& pView->GetDocument()->m_docEditor.IsModified()
			&& ModifyVisitor().IsLineModified(
				pDocLine,
				pView->GetDocument()->m_docEditor.m_opeBuf.GetNoModifiedSeq()
			)
		) {
			// �ύX�t���O
			if (TypeSupport(pView, COLORIDX_GYOU_MOD).IsDisp()) {	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	// �s�ԍ��i�ύX�s�j
				bGyouMod = true;
			}
		}
	}

	if (pDocLine) {
		// DIFF�F�ݒ�
		DiffLineGetter(pDocLine).GetDiffColor(&nColorIndex);

		// 02/10/16 ai
		// �u�b�N�}�[�N�̕\��
		if (BookmarkGetter(pDocLine).IsBookmarked()) {
			if (TypeSupport(pView, COLORIDX_MARK).IsDisp()) {
				nColorIndex = COLORIDX_MARK;
			}
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���肳�ꂽnColorIndex���g���ĕ`��               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	TypeSupport colorType(pView, nColorIndex);
	TypeSupport markType(pView, COLORIDX_MARK);

	// �Y���s�̍s�ԍ��G���A��`
	RECT	rcLineNum;
	rcLineNum.left = 0;
	rcLineNum.right = nLineNumAreaWidth;
	rcLineNum.top = y;
	rcLineNum.bottom = y + nLineHeight;
	
	bool bTrans = pView->IsBkBitmap() && textType.GetBackColor() == colorType.GetBackColor();
	bool bTransText = pView->IsBkBitmap() && textType.GetBackColor() == backType.GetBackColor();
	bool bDispLineNumTrans = false;

	COLORREF fgcolor = colorType.GetTextColor();
	COLORREF bgcolor = colorType.GetBackColor();
	TypeSupport gyouType(pView, COLORIDX_GYOU);
	TypeSupport gyouModType(pView, COLORIDX_GYOU_MOD);
	if (bGyouMod && nColorIndex != COLORIDX_GYOU_MOD) {
		if (gyouType.GetTextColor() == colorType.GetTextColor()) {
			fgcolor = gyouModType.GetTextColor();
		}
		if (gyouType.GetBackColor() == colorType.GetBackColor()) {
			bgcolor = gyouModType.GetBackColor();
			bTrans = pView->IsBkBitmap() && textType.GetBackColor() == gyouModType.GetBackColor();
		}
	}
	// 2014.01.29 Moca �w�i�F���e�L�X�g�Ɠ����Ȃ�A���ߐF�Ƃ��čs�w�i�F��K�p
	if (bgcolor == textType.GetBackColor()) {
		bgcolor = backType.GetBackColor();
		bTrans = pView->IsBkBitmap() && textType.GetBackColor() == bgcolor;
		bDispLineNumTrans = true;
	}
	if (!pLayout) {
		// �s�����݂��Ȃ��ꍇ�́A�e�L�X�g�`��F�œh��Ԃ�
		if (!bTransText) {
			textType.FillBack(gr, rcLineNum);
		}
		bDispLineNumTrans = true;
	}else if (TypeSupport(pView, COLORIDX_GYOU).IsDisp()) { // �s�ԍ��\���^��\��
		Font font = colorType.GetTypeFont();
	 	// 2013.12.30 �ύX�s�̐F�E�t�H���g������DIFF�u�b�N�}�[�N�s�Ɍp������悤��
		if (bGyouMod && nColorIndex != COLORIDX_GYOU_MOD) {
			bool bChange = true;
			if (gyouType.IsBoldFont() == colorType.IsBoldFont()) {
		 		font.fontAttr.bBoldFont = gyouModType.IsBoldFont();
				bChange = true;
			}
			if (gyouType.HasUnderLine() == colorType.HasUnderLine()) {
				font.fontAttr.bUnderLine = gyouModType.HasUnderLine();
				bChange = true;
			}
			if (bChange) {
				font.hFont = pView->GetFontset().ChooseFontHandle(font.fontAttr);
			}
		}
		gr.PushTextForeColor(fgcolor);	// �e�L�X�g�F�s�ԍ��̐F
		gr.PushTextBackColor(bgcolor);	// �e�L�X�g�F�s�ԍ��w�i�̐F
		gr.PushMyFont(font);	// �t�H���g�F�s�ԍ��̃t�H���g

		// �`�敶����
		wchar_t szLineNum[18];
		int nLineCols;
		int nLineNumCols;
		{
			// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
			if (typeConfig.bLineNumIsCRLF) {
				// �_���s�ԍ��\�����[�h
				if (!pLayout || pLayout->GetLogicOffset() != 0) { // �܂�Ԃ����C�A�E�g�s
					wcscpy(szLineNum, L" ");
				}else {
					_itow(pLayout->GetLogicLineNo() + 1, szLineNum, 10);	// �Ή�����_���s�ԍ�
//###�f�o�b�O�p
//					_itow(ModifyVisitor().GetLineModifiedSeq(pDocLine), szLineNum, 10);	// �s�̕ύX�ԍ�
				}
			}else {
				// �����s�i���C�A�E�g�s�j�ԍ��\�����[�h
				_itow((Int)nLineNum + 1, szLineNum, 10);
			}
			nLineCols = wcslen(szLineNum);
			nLineNumCols = nLineCols; // 2010.08.17 Moca �ʒu����ɍs�ԍ���؂�͊܂߂Ȃ�

			// �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C��
			if (typeConfig.nLineTermType == 2) {
				//	Sep. 22, 2002 genta
				szLineNum[nLineCols] = typeConfig.cLineTermChar;
				szLineNum[++nLineCols] = '\0';
			}
		}

		//	Sep. 23, 2002 genta
		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineNumCols - 1) * (nCharWidth);
		::ExtTextOutW_AnyBuild(gr,
			drawNumTop,
			y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcLineNum,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C��
		if (typeConfig.nLineTermType == 1) {
			RECT rc;
			rc.left = nLineNumAreaWidth - 2;
			rc.top = y;
			rc.right = nLineNumAreaWidth - 1;
			rc.bottom = y + nLineHeight;
			gr.FillSolidMyRect(rc, fgcolor);
		}

		gr.PopTextForeColor();
		gr.PopTextBackColor();
		gr.PopMyFont();
	}else {
		// �s�ԍ��G���A�̔w�i�`��
		if (!bTrans) {
			gr.FillSolidMyRect(rcLineNum, bgcolor);
		}
		bDispLineNumTrans = true;
	}

	// �s�����`�� ($$$�����\��)
	if (pDocLine) {
		// 2001.12.03 hor
		// �Ƃ肠�����u�b�N�}�[�N�ɏc��
		if (BookmarkGetter(pDocLine).IsBookmarked() && !markType.IsDisp()) {
			gr.PushPen(colorType.GetTextColor(), 2);
			::MoveToEx(gr, 1, y, NULL);
			::LineTo(gr, 1, y + nLineHeight);
			gr.PopPen();
		}

		// DIFF�}�[�N�`��
		DiffLineGetter(pDocLine).DrawDiffMark(gr, y, nLineHeight, fgcolor);
	}

	// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��
	if (!bTransText) {
		RECT rcRest;
		rcRest.left   = rcLineNum.right;
		rcRest.right  = pView->GetTextArea().GetAreaLeft();
		rcRest.top    = y;
		rcRest.bottom = y + nLineHeight;
		textType.FillBack(gr, rcRest);
	}
	
	// �s�ԍ������̃m�[�g���`��
	if (!pView->m_bMiniMap) {
		int left   = bDispLineNumTrans ? 0 : rcLineNum.right;
		int right  = pView->GetTextArea().GetAreaLeft();
		int top    = y;
		int bottom = y + nLineHeight;
		DispNoteLine( gr, top, bottom, left, right );
	}
}

