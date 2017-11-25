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
	return editView.GetTextArea();
}

using namespace std;


/* �e�L�X�g�\�� */
void TextDrawer::DispText(
	HDC			hdc,
	DispPos*	pDispPos,
	const wchar_t* pData,
	size_t		nLength,
	bool		bTransparent
	) const
{
	if (nLength == 0) {
		return;
	}
	int x = pDispPos->GetDrawPos().x;
	int y = pDispPos->GetDrawPos().y;

	// �K�v�ȃC���^�[�t�F�[�X���擾
	const TextMetrics* pMetrics = &editView.GetTextMetrics();
	const TextArea& textArea = GetTextArea();

	// �����Ԋu�z��𐶐�
	static vector<int> vDxArray(1);
	const int* pDxArray = pMetrics->GenerateDxArray(&vDxArray, pData, nLength, editView.GetTextMetrics().GetHankakuDx());

	// ������̃s�N�Z����
	size_t nTextWidth = pMetrics->CalcTextWidth(pData, nLength, pDxArray);

	// �e�L�X�g�̕`��͈͂̋�`�����߂� -> rcClip
	Rect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + (int)nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + editView.GetTextMetrics().GetHankakuDy();
	if (rcClip.left < textArea.GetAreaLeft()) {
		rcClip.left = textArea.GetAreaLeft();
	}

	// �����Ԋu
	size_t nDx = editView.GetTextMetrics().GetHankakuDx();

	if (textArea.IsRectIntersected(rcClip) && rcClip.top >= textArea.GetAreaTop()) {

		if (rcClip.Width() > textArea.GetAreaWidth()) {
			rcClip.right = rcClip.left + textArea.GetAreaWidth();
		}

		// �E�B���h�E�̍��ɂ��ӂꂽ������ -> nBefore
		size_t nBeforeLogic = 0;
		size_t nBeforeLayout = 0;
		if (x < 0) {
			size_t nLeftLayout = (0 - x) / nDx - 1;
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
		int nDrawX = x + nBeforeLayout * nDx;

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
	@note Common::nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void TextDrawer::DispVerticalLines(
	Graphics&	gr,			// ��悷��E�B���h�E��DC
	int			nTop,		// ����������[�̃N���C�A���g���Wy
	int			nBottom,	// �����������[�̃N���C�A���g���Wy
	int			nLeftCol,	// ���������͈͂̍����̎w��
	int	nRightCol	// ���������͈͂̉E���̎w��(-1�Ŗ��w��)
	) const
{
	auto& view = editView;
	
	const TypeConfig& typeData = view.pEditDoc->docType.GetDocumentAttribute();
	
	TypeSupport vertType(view, COLORIDX_VERTLINE);
	TypeSupport textType(view, COLORIDX_TEXT);
	
	if (!vertType.IsDisp()) {
		return;
	}
	
	auto& textArea = view.GetTextArea();
	nLeftCol = t_max((int)textArea.GetViewLeftCol(), nLeftCol);
	
	const size_t nWrapKetas  = view.pEditDoc->layoutMgr.GetMaxLineKetas();
	const size_t nCharDx  = view.GetTextMetrics().GetHankakuDx();
	if (nRightCol < 0) {
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = GetDllShareData().common.window.nVertLineOffset + textArea.GetAreaLeft();
	const int nPosXLeft   = t_max(textArea.GetAreaLeft() + (nLeftCol - (int)textArea.GetViewLeftCol()) * (int)nCharDx, textArea.GetAreaLeft());
	const int nPosXRight  = t_min(textArea.GetAreaLeft() + (nRightCol - (int)textArea.GetViewLeftCol()) * (int)nCharDx, textArea.GetAreaRight());
	const size_t nLineHeight = view.GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? textArea.GetViewTopLine() : 0) + textArea.GetAreaTop() + nTop) % 2 == 1);

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
		int nXCol = typeData.nVertLineIdx[k];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
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
			if ((int)nWrapKetas < nXCol) {
				break;
			}
			int nPosX = nPosXOffset + (nXCol - 1 - textArea.GetViewLeftCol()) * nCharDx;
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
	auto& view = editView;

	TypeSupport noteLine(view, COLORIDX_NOTELINE);
	if (noteLine.IsDisp()) {
		gr.SetPen(noteLine.GetTextColor());
		const size_t nLineHeight = view.GetTextMetrics().GetHankakuDy();
		const int left = nLeft;
		const int right = nRight;
		int userOffset = view.pTypeData->nNoteLineOffset;
		int offset = view.GetTextArea().GetAreaTop() + userOffset - 1;
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

/*!	�܂�Ԃ����c���̕`�� */
void TextDrawer::DispWrapLine(
	Graphics&	gr,			// ��悷��E�B���h�E��DC
	int			nTop,		// ����������[�̃N���C�A���g���Wy
	int			nBottom		// �����������[�̃N���C�A���g���Wy
	) const
{
	auto& view = editView;
	TypeSupport wrapType(view, COLORIDX_WRAP);
	if (!wrapType.IsDisp()) {
		return;
	}

	const TextArea& textArea = GetTextArea();
	const size_t nWrapKetas = view.pEditDoc->layoutMgr.GetMaxLineKetas();
	const size_t nCharDx = view.GetTextMetrics().GetHankakuDx();
	int nXPos = textArea.GetAreaLeft() + (nWrapKetas - textArea.GetViewLeftCol()) * nCharDx;
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
	int				nLineNum,
	int				y
	) const
{
	//$$ �������FSearchLineByLayoutY�ɃL���b�V������������
	const Layout*	pLayout = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineNum);

	auto& view = editView;
	const TypeConfig& typeConfig = view.pEditDoc->docType.GetDocumentAttribute();

	unsigned int nLineHeight = (unsigned int)view.GetTextMetrics().GetHankakuDy();
	size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
	// �s�ԍ��\������X��	Sep. 23, 2002 genta ���ʎ��̂����肾��
	//int nLineNumAreaWidth = pView->GetTextArea().nViewAlignLeftCols * nCharWidth;
	int nLineNumAreaWidth = view.GetTextArea().GetAreaLeft() - GetDllShareData().common.window.nLineNumRightSpace;

	TypeSupport textType(view, COLORIDX_TEXT);
	TypeSupport caretLineBg(view, COLORIDX_CARETLINEBG);
	TypeSupport evenLineBg(view, COLORIDX_EVENLINEBG);
	// �s���Ȃ��Ƃ��E�s�̔w�i�������̂Ƃ��̐F
	TypeSupport& backType = (caretLineBg.IsDisp() &&
		view.GetCaret().GetCaretLayoutPos().GetY() == nLineNum
			? caretLineBg
			: evenLineBg.IsDisp() && nLineNum % 2 == 1
				? evenLineBg
				: textType);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndex������                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	// �s�ԍ�
	const DocLine* pDocLine = nullptr;
	bool bGyouMod = false;
	if (pLayout) {
		pDocLine = pLayout->GetDocLineRef();
		if (1
			&& view.GetDocument().docEditor.IsModified()
			&& ModifyVisitor().IsLineModified(
				pDocLine,
				view.GetDocument().docEditor.opeBuf.GetNoModifiedSeq()
			)
		) {
			// �ύX�t���O
			if (TypeSupport(view, COLORIDX_GYOU_MOD).IsDisp()) {
				nColorIndex = COLORIDX_GYOU_MOD;	// �s�ԍ��i�ύX�s�j
				bGyouMod = true;
			}
		}
	}

	if (pDocLine) {
		// DIFF�F�ݒ�
		DiffLineGetter(pDocLine).GetDiffColor(&nColorIndex);

		// �u�b�N�}�[�N�̕\��
		if (BookmarkGetter(pDocLine).IsBookmarked()) {
			if (TypeSupport(view, COLORIDX_MARK).IsDisp()) {
				nColorIndex = COLORIDX_MARK;
			}
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���肳�ꂽnColorIndex���g���ĕ`��               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	TypeSupport colorType(view, nColorIndex);
	TypeSupport markType(view, COLORIDX_MARK);

	// �Y���s�̍s�ԍ��G���A��`
	RECT rcLineNum;
	rcLineNum.left = 0;
	rcLineNum.right = nLineNumAreaWidth;
	rcLineNum.top = y;
	rcLineNum.bottom = y + nLineHeight;
	
	bool bTrans = view.IsBkBitmap() && textType.GetBackColor() == colorType.GetBackColor();
	bool bTransText = view.IsBkBitmap() && textType.GetBackColor() == backType.GetBackColor();
	bool bDispLineNumTrans = false;

	COLORREF fgcolor = colorType.GetTextColor();
	COLORREF bgcolor = colorType.GetBackColor();
	TypeSupport gyouType(view, COLORIDX_GYOU);
	TypeSupport gyouModType(view, COLORIDX_GYOU_MOD);
	if (bGyouMod && nColorIndex != COLORIDX_GYOU_MOD) {
		if (gyouType.GetTextColor() == colorType.GetTextColor()) {
			fgcolor = gyouModType.GetTextColor();
		}
		if (gyouType.GetBackColor() == colorType.GetBackColor()) {
			bgcolor = gyouModType.GetBackColor();
			bTrans = view.IsBkBitmap() && textType.GetBackColor() == gyouModType.GetBackColor();
		}
	}
	// �w�i�F���e�L�X�g�Ɠ����Ȃ�A���ߐF�Ƃ��čs�w�i�F��K�p
	if (bgcolor == textType.GetBackColor()) {
		bgcolor = backType.GetBackColor();
		bTrans = view.IsBkBitmap() && textType.GetBackColor() == bgcolor;
		bDispLineNumTrans = true;
	}
	if (!pLayout) {
		// �s�����݂��Ȃ��ꍇ�́A�e�L�X�g�`��F�œh��Ԃ�
		if (!bTransText) {
			textType.FillBack(gr, rcLineNum);
		}
		bDispLineNumTrans = true;
	}else if (TypeSupport(view, COLORIDX_GYOU).IsDisp()) { // �s�ԍ��\���^��\��
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
				font.hFont = view.GetFontset().ChooseFontHandle(font.fontAttr);
			}
		}
		gr.PushTextForeColor(fgcolor);	// �e�L�X�g�F�s�ԍ��̐F
		gr.PushTextBackColor(bgcolor);	// �e�L�X�g�F�s�ԍ��w�i�̐F
		gr.PushMyFont(font);	// �t�H���g�F�s�ԍ��̃t�H���g

		// �`�敶����
		wchar_t szLineNum[18];
		size_t nLineCols;
		size_t nLineNumCols;
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
				_itow(nLineNum + 1, szLineNum, 10);
			}
			nLineCols = wcslen(szLineNum);
			nLineNumCols = nLineCols;

			// �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C��
			if (typeConfig.nLineTermType == 2) {
				//	Sep. 22, 2002 genta
				szLineNum[nLineCols] = typeConfig.cLineTermChar;
				szLineNum[++nLineCols] = '\0';
			}
		}

		//	Sep. 23, 2002 genta
		int drawNumTop = (view.GetTextArea().nViewAlignLeftCols - nLineNumCols - 1) * (nCharWidth);
		::ExtTextOutW_AnyBuild(gr,
			drawNumTop,
			y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcLineNum,
			szLineNum,
			nLineCols,
			view.GetTextMetrics().GetDxArray_AllHankaku()
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
		// �Ƃ肠�����u�b�N�}�[�N�ɏc��
		if (BookmarkGetter(pDocLine).IsBookmarked() && !markType.IsDisp()) {
			gr.PushPen(colorType.GetTextColor(), 2);
			::MoveToEx(gr, 1, y, NULL);
			::LineTo(gr, 1, y + (int)nLineHeight);
			gr.PopPen();
		}

		// DIFF�}�[�N�`��
		DiffLineGetter(pDocLine).DrawDiffMark(gr, y, nLineHeight, fgcolor);
	}

	// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��
	if (!bTransText) {
		RECT rcRest;
		rcRest.left   = rcLineNum.right;
		rcRest.right  = view.GetTextArea().GetAreaLeft();
		rcRest.top    = y;
		rcRest.bottom = y + nLineHeight;
		textType.FillBack(gr, rcRest);
	}
	
	// �s�ԍ������̃m�[�g���`��
	if (!view.bMiniMap) {
		int left   = bDispLineNumTrans ? 0 : rcLineNum.right;
		int right  = view.GetTextArea().GetAreaLeft();
		int top    = y;
		int bottom = y + nLineHeight;
		DispNoteLine( gr, top, bottom, left, right );
	}
}

