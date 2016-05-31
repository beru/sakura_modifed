#include "StdAfx.h"
#include "Ruler.h"
#include "TextArea.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"

Ruler::Ruler(const EditView& editView, const EditDoc& editDoc)
	:
	editView(editView),
	editDoc(editDoc)
{
	nOldRulerDrawX = 0;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	nOldRulerWidth = 0;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
}

Ruler::~Ruler()
{
}

// 2007.08.26 kobake UNICODE�p��X�ʒu��ύX
void Ruler::_DrawRulerCaret(
	Graphics& gr,
	int nCaretDrawPosX,
	int nCaretWidth
	)
{
	// �`��̈� -> hRgn
	RECT rc;
	rc.left = nCaretDrawPosX + 1;	// 2012.07.27 Moca 1px�E�ɏC��
	rc.right = rc.left + editView.GetTextMetrics().GetHankakuDx() - 1;
	rc.top = 0;
	rc.bottom = editView.GetTextArea().GetAreaTop() - editView.GetTextArea().GetTopYohaku() - 1;
	HRGN hRgn = ::CreateRectRgnIndirect(&rc);

	// �u���V�쐬 -> hBrush
	HBRUSH hBrush;
	if (nCaretWidth == 0) {
		hBrush = ::CreateSolidBrush(RGB(128, 128, 128));
	}else {
		hBrush = ::CreateSolidBrush(RGB(0, 0, 0));
	}

	// �̈��`�� (�F�𔽓]������)
	int nROP_Old  = ::SetROP2(gr, R2_NOTXORPEN);
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(gr, hBrush);
	::SelectObject(gr, hBrush);
	::PaintRgn(gr, hRgn);
	::SelectObject(gr, hBrushOld);
	::SetROP2(gr, nROP_Old);

	// �`��I�u�W�F�N�g�j��
	::DeleteObject(hRgn);
	::DeleteObject(hBrush);
}

/*! 
	���[���[�̃L�����b�g���ĕ`��	2002.02.25 Add By KK
	@param hdc [in] �f�o�C�X�R���e�L�X�g
	DispRuler�̓��e�����ɍ쐬
*/
void Ruler::DrawRulerCaret(Graphics& gr)
{
	auto& textArea = editView.GetTextArea();
	auto& caret = editView.GetCaret();
	if (1
		&& textArea.GetViewLeftCol() <= caret.GetCaretLayoutPos().GetX()
		&& textArea.GetRightCol() + 2 >= caret.GetCaretLayoutPos().GetX()
	) {
		auto& ruler = editView.GetRuler();
		if (1
			&& ruler.nOldRulerDrawX == caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x
			&& caret.GetCaretSize().cx == ruler.nOldRulerWidth
		) {
			// �O�`�悵���ʒu�擯�� ���� ���[���[�̃L�����b�g�������� 
			return;
		}

		// ���ʒu���N���A nOldRulerWidth
		this->_DrawRulerCaret(gr, nOldRulerDrawX, nOldRulerWidth);

		// �V�����ʒu�ŕ`��   2007.08.26 kobake UNICODE�p��X�ʒu��ύX
		this->_DrawRulerCaret(
			gr,
			caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x,
			caret.GetCaretSize().cx
		);
	}
}

// ���[���[�̔w�i�̂ݕ`�� 2007.08.29 kobake �ǉ�
void Ruler::DrawRulerBg(Graphics& gr)
{
	// �K�v�ȃC���^�[�t�F�[�X
	CommonSetting* pCommon = &GetDllShareData().common;

	// �T�|�[�g
	TypeSupport rulerType(editView, COLORIDX_RULER);

	// �t�H���g�ݒ� (���[���[��̐����p)
	LOGFONT	lf = {0};
	lf.lfHeight			= 1 - pCommon->window.nRulerHeight;	//	2002/05/13 ai
	lf.lfWidth			= 5;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0;
	lf.lfUnderline		= 0;
	lf.lfStrikeOut		= 0;
	lf.lfCharSet		= 0;
	lf.lfOutPrecision	= 3;
	lf.lfClipPrecision	= 2;
	lf.lfQuality		= 1;
	lf.lfPitchAndFamily	= 34;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	HFONT hFont = ::CreateFontIndirect(&lf);
	HFONT hFontOld = (HFONT)::SelectObject(gr, hFont);
	::SetBkMode(gr, TRANSPARENT);
	
	auto& textArea = editView.GetTextArea();
	// �w�i�h��Ԃ�
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = textArea.GetAreaRight();
	rc.bottom = textArea.GetAreaTop() - textArea.GetTopYohaku();
	rulerType.FillBack(gr, rc);
	
	// ���[���[�F�ݒ�
	gr.PushPen(rulerType.GetTextColor(), 0);
	gr.PushTextForeColor(rulerType.GetTextColor());
	
	// �`��J�n�ʒu
	int nX = textArea.GetAreaLeft();
	int nY = textArea.GetRulerHeight() - 2;
	
	// ���� (���[���[�Ɩ{���̋��E)
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	//	2005.11.10 Moca 1dot����Ȃ�
	size_t nMaxLineKetas = editDoc.layoutMgr.GetMaxLineKetas();
	int nToX = textArea.GetAreaLeft() + (nMaxLineKetas - textArea.GetViewLeftCol()) * editView.GetTextMetrics().GetHankakuDx() + 1;
	if (nToX > textArea.GetAreaRight()) {
		nToX = textArea.GetAreaRight();
	}
	::MoveToEx(gr, textArea.GetAreaLeft(), nY + 1, NULL);
	::LineTo(gr, nToX, nY + 1);

	// �ڐ���`��
	int i = textArea.GetViewLeftCol();
	while (i <= textArea.GetRightCol() + 1 && i <= nMaxLineKetas) {
		// ���[���[�I�[�̋�؂�(��)
		if (i == nMaxLineKetas) {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, 0);
		// 10�ڐ������̋�؂�(��)�Ɛ���
		}else if (i % 10 == 0) {
			wchar_t szColumn[32];
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, 0);
			_itow(i / 10, szColumn, 10);
			::TextOutW_AnyBuild(gr, nX + 2 + 0, -1 + 0, szColumn, wcslen(szColumn));
		// 5�ڐ������̋�؂�(��)
		}else if (i % 5 == 0) {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, nY - 6);
		// ���ڐ��̋�؂�(��)
		}else {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, nY - 3);
		}

		nX += (int)editView.GetTextMetrics().GetHankakuDx();
		++i;
	}

	// �F�߂�
	gr.PopTextForeColor();
	gr.PopPen();

	// �t�H���g�߂�
	::SelectObject(gr, hFontOld);
	::DeleteObject(hFont);
}

/*! ���[���[�`��

	@date 2005.08.14 genta �܂�Ԃ�����LayoutMgr����擾����悤��
*/
void Ruler::DispRuler(HDC hdc)
{
	// �T�|�[�g
	TypeSupport rulerType(editView, COLORIDX_RULER);

	if (!editView.GetDrawSwitch()) {
		return;
	}
	if (!rulerType.IsDisp()) {
		return;
	}

	// �`��Ώ�
	Graphics gr(hdc);
	auto& caret = editView.GetCaret();
	// 2002.02.25 Add By KK ���[���[�S�̂�`�������K�v���Ȃ��ꍇ�́A���[����̃L�����b�g�̂ݕ`���Ȃ��� 
	if (!bRedrawRuler) {
		DrawRulerCaret(gr);
	}else {
		// �w�i�`��
		DrawRulerBg(gr);
		auto& textArea = editView.GetTextArea();
		// �L�����b�g�`��
		if (1
			&& textArea.GetViewLeftCol() <= caret.GetCaretLayoutPos().GetX()
			&& textArea.GetRightCol() + 2 >= caret.GetCaretLayoutPos().GetX()
		) {
			_DrawRulerCaret(gr, caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x, caret.GetCaretSize().cx);
		}

		bRedrawRuler = false;	// bRedrawRuler = true �Ŏw�肳���܂ŁA���[���̃L�����b�g�݂̂��ĕ`�� 2002.02.25 Add By KK
	}

	// �`�悵�����[���[�̃L�����b�g�ʒu�E����ۑ� 2002.02.25 Add By KK
	nOldRulerDrawX = caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x;
	nOldRulerWidth = caret.GetCaretSize().cx ;
}

