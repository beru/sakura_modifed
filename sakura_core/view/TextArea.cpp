#include "StdAfx.h"
#include "TextArea.h"
#include "ViewFont.h"
#include "Ruler.h"
#include "EditView.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "doc/EditDoc.h"

// 2014.07.26 katze
//#define USE_LOG10			// ���̍s�̃R�����g���O���ƍs�ԍ��̍ŏ������̌v�Z��log10()��p����
#ifdef USE_LOG10
#include <math.h>
#endif

TextArea::TextArea(EditView& editView)
	:
	editView(editView)
{
	DllSharedData* pShareData = &GetDllShareData();

	nViewAlignLeft = 0;					// �\����̍��[���W
	nViewAlignLeftCols = 0;				// �s�ԍ���̌���
	nViewCx = 0;							// �\����̕�
	nViewCy = 0;							// �\����̍���
	nViewColNum = LayoutInt(0);			// �\����̌���
	nViewRowNum = LayoutInt(0);			// �\����̍s��
	nViewTopLine = LayoutInt(0);			// �\����̈�ԏ�̍s
	nViewLeftCol = LayoutInt(0);			// �\����̈�ԍ��̌�
	SetTopYohaku(pShareData->common.window.nRulerBottomSpace); 	// ���[���[�ƃe�L�X�g�̌���
	SetLeftYohaku(pShareData->common.window.nLineNumRightSpace);
	nViewAlignTop = GetTopYohaku();		// �\����̏�[���W
}

TextArea::~TextArea()
{
}

void TextArea::CopyTextAreaStatus(TextArea* pDst) const
{
	pDst->SetAreaLeft				(this->GetAreaLeft());		// �\����̍��[���W
	pDst->nViewAlignLeftCols		= this->nViewAlignLeftCols;	// �s�ԍ���̌���
	pDst->SetAreaTop				(this->GetAreaTop());			// �\����̏�[���W
//	pDst->nViewCx					= nViewCx;					// �\����̕�
//	pDst->nViewCy					= nViewCy;					// �\����̍���
//	pDst->nViewColNum				= this->nViewColNum;			// �\����̌���
//	pDst->nViewRowNum				= this->nViewRowNum;			// �\����̍s��
	pDst->SetViewTopLine			(this->GetViewTopLine());		// �\����̈�ԏ�̍s(0�J�n)
	pDst->SetViewLeftCol			(this->GetViewLeftCol());		// �\����̈�ԍ��̌�(0�J�n)
}

// �\����̍Čv�Z
void TextArea::UpdateViewColRowNums()
{
	auto& view = editView;
	// Note: �}�C�i�X�̊���Z�͏����n�ˑ��ł��B
	// 0���ƃJ�[�\����ݒ�ł��Ȃ��E�I���ł��Ȃ��ȂǓ���s�ǂɂȂ�̂�1�ȏ�ɂ���
	nViewColNum = LayoutInt(t_max(1, t_max(0, nViewCx - 1) / view.GetTextMetrics().GetHankakuDx()));	// �\����̌���
	nViewRowNum = LayoutInt(t_max(1, t_max(0, nViewCy - 1) / view.GetTextMetrics().GetHankakuDy()));	// �\����̍s��
}

//�t�H���g�ύX�̍ہA�e��p�����[�^���v�Z������
void TextArea::UpdateAreaMetrics(HDC hdc)
{
	auto& view = editView;
	auto& textMetrics = view.GetTextMetrics();
	if (view.bMiniMap) {
		// �����Ԋu
		textMetrics.SetHankakuDx(textMetrics.GetHankakuWidth());

		// �s�Ԋu
		textMetrics.SetHankakuDy(textMetrics.GetHankakuHeight());
	}else {
		// �����Ԋu
		textMetrics.SetHankakuDx(textMetrics.GetHankakuWidth() + view.pTypeData->nColumnSpace);
	
		// �s�Ԋu
		textMetrics.SetHankakuDy(textMetrics.GetHankakuHeight() + view.pTypeData->nLineSpace);
	}

	// �\����̍Čv�Z
	// 2010.08.24 Dx/Dy���g���̂Ō�Őݒ�
	UpdateViewColRowNums();
}

void TextArea::GenerateCharRect(RECT* rc, const DispPos& pos, int nHankakuNum) const
{
	auto& view = editView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = pos.GetDrawPos().x + view.GetTextMetrics().GetHankakuDx() * nHankakuNum;
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + view.GetTextMetrics().GetHankakuDy();
}

bool TextArea::TrimRectByArea(RECT* rc) const
{
	// ���͂ݏo������
	if (rc->left < GetAreaLeft()) {
		rc->left = GetAreaLeft();
	}

	if (rc->left >= rc->right) return false; // ���ƉE�����ׂ���
	if (rc->left >= GetAreaRight()) return false; // ��ʊO(�E)
	if (rc->right <= GetAreaLeft()) return false; // ��ʊO(��)

	//$ �����쓥�P�F��ʏ㉺�̂͂ݏo������͏ȗ�

	return true;
}

bool TextArea::GenerateClipRect(RECT* rc, const DispPos& pos, int nHankakuNum) const
{
	GenerateCharRect(rc, pos, nHankakuNum);
	return TrimRectByArea(rc);
}

// �E�̎c���\����`�𐶐�����
bool TextArea::GenerateClipRectRight(RECT* rc, const DispPos& pos) const
{
	auto& view = editView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = GetAreaRight();
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + view.GetTextMetrics().GetHankakuDy();

	// ���͂ݏo������
	if (rc->left < GetAreaLeft()) {
		rc->left = GetAreaLeft();
	}

	if (rc->left >= rc->right) return false; // ���ƉE�����ׂ���
	if (rc->left >= GetAreaRight()) return false; // ��ʊO(�E)
	if (rc->right <= GetAreaLeft()) return false; // ��ʊO(��)

	//$ �����쓥�P�F��ʏ㉺�̂͂ݏo������͏ȗ�

	return true;
}

bool TextArea::GenerateClipRectLine(RECT* rc, const DispPos& pos) const
{
	rc->left   = 0;
	rc->right  = GetAreaRight();
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + editView.GetTextMetrics().GetHankakuDy();
	return true;
}


// �s�ԍ��\���ɕK�v�ȕ���ݒ�B�����ύX���ꂽ�ꍇ�� true ��Ԃ�
bool TextArea::DetectWidthOfLineNumberArea(bool bRedraw)
{
	auto& view = editView;

	int nViewAlignLeftNew;

	if (view.pTypeData->colorInfoArr[COLORIDX_GYOU].bDisp && !view.bMiniMap) {
		// �s�ԍ��\���ɕK�v�Ȍ������v�Z
		int i = DetectWidthOfLineNumberArea_calculate(&view.pEditDoc->layoutMgr);
		nViewAlignLeftNew = view.GetTextMetrics().GetHankakuDx() * (i + 1);	// �\����̍��[���W
		nViewAlignLeftCols = i + 1;
	}else if (view.bMiniMap) {
		nViewAlignLeftNew = 4;
		nViewAlignLeftCols = 0;
	}else {
		nViewAlignLeftNew = 8;
		nViewAlignLeftCols = 0;
	}

	//	Sep 18, 2002 genta
	nViewAlignLeftNew += GetDllShareData().common.window.nLineNumRightSpace;
	if (nViewAlignLeftNew != GetAreaLeft()) {
		Rect			rc;
		SetAreaLeft(nViewAlignLeftNew);
		view.GetClientRect(&rc);
		int nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);		// �����X�N���[���o�[�̉���
		nViewCx = rc.Width() - nCxVScroll - GetAreaLeft();	// �\����̕�
		// 2008.05.27 nasukoji	�\����̌������Z�o����i�E�[�J�[�\���ړ����̕\���ꏊ����ւ̑Ώ��j
		// nViewColNum = LayoutInt(t_max(0, nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// �\����̌���
		UpdateViewColRowNums();

		if (bRedraw && view.GetDrawSwitch()) {
			// �ĕ`��
			view.GetCaret().underLine.Lock();
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Ruler | (int)PaintAreaType::Body, false); // �������c�b���g�p���Ă�����̂Ȃ��ĕ`��
			// To Here 2007.09.09 Moca
			view.GetCaret().underLine.UnLock();
			view.GetCaret().ShowEditCaret();
			/*
			PAINTSTRUCT		ps;
			HDC hdc = ::GetDC(view.hWnd);
			ps.rcPaint.left   = 0;
			ps.rcPaint.right  = GetAreaRight();
			ps.rcPaint.top    = 0;
			ps.rcPaint.bottom = GetAreaBottom();
			view.GetCaret().underLine.Lock();
			view.OnPaint(hdc, &ps, TRUE);	
			GetCaret().underLine.UnLock();
			view.GetCaret().ShowEditCaret();
			::ReleaseDC(hWnd, hdc);
			*/
		}
		view.GetRuler().SetRedrawFlag();
		return true;
	}else {
		return false;
	}
}


// �s�ԍ��\���ɕK�v�Ȍ������v�Z
int TextArea::DetectWidthOfLineNumberArea_calculate(const LayoutMgr* pLayoutMgr, bool bLayout) const
{
	auto& view = editView;

	int nAllLines; //$$ �P�ʍ���

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (view.pTypeData->bLineNumIsCRLF && !bLayout) {
		nAllLines = view.pEditDoc->docLineMgr.GetLineCount();
	}else {
		nAllLines = (Int)pLayoutMgr->GetLineCount();
	}
	
	if (0 < nAllLines) {
		int nWork;
		int i;

		// �s�ԍ��̌��������߂� 2014.07.26 katze
		// nLineNumWidth�͏����ɐ����̌����������A�擪�̋󔒂��܂܂Ȃ��i�d�l�ύX�j 2014.08.02 katze
#ifdef USE_LOG10
		// �\�����Ă���s���̌��������߂�
		nWork = (int)(log10( (double)nAllLines) +1);	// 10���Ƃ���ΐ�(�����_�ȉ��؂�̂�)+1�Ō���
		// �ݒ�l�Ɣ�r���A�傫���������
		i = nWork > view.pTypeData->nLineNumWidth ?
			nWork : view.pTypeData->nLineNumWidth;
		// �擪�̋󔒕������Z����
		return (i +1);
#else
		// �ݒ肩��s�������߂�
		nWork = 10;
		for (i=1; i<view.pTypeData->nLineNumWidth; ++i) {
			nWork *= 10;
		}
		// �\�����Ă���s���Ɣ�r���A�傫�����̒l�����
		for (i= view.pTypeData->nLineNumWidth; i<LINENUMWIDTH_MAX; ++i) {
			if (nWork > nAllLines) {	// Oct. 18, 2003 genta ���𐮗�
				break;
			}
			nWork *= 10;
		}
		// �擪�̋󔒕������Z����
		return (i +1);
#endif
	}else {
		//	2003.09.11 wmlhq �s�ԍ���1���̂Ƃ��ƕ������킹��
		// �ŏ��������ςɕύX 2014.07.26 katze	// �擪�̋󔒕������Z���� 2014.07.31 katze
		return view.pTypeData->nLineNumWidth +1;
	}
}

void TextArea::TextArea_OnSize(
	const Size& sizeClient,		// �E�B���h�E�̃N���C�A���g�T�C�Y
	int nCxVScroll,				// �����X�N���[���o�[�̉���
	int nCyHScroll				// �����X�N���[���o�[�̏c��
	)
{
	nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // �\����̕�
	nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // �\����̍���
	UpdateViewColRowNums();
}


int TextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - (Int)GetViewLeftCol() * editView.GetTextMetrics().GetHankakuDx();
}

// �N���C�A���g���W���烌�C�A�E�g�ʒu�ɕϊ�����
void TextArea::ClientToLayout(Point ptClient, LayoutPoint* pptLayout) const
{
	auto& view = editView;
	pptLayout->Set(
		GetViewLeftCol() + LayoutInt((ptClient.x - GetAreaLeft()) / view.GetTextMetrics().GetHankakuDx()),
		GetViewTopLine() + LayoutInt((ptClient.y - GetAreaTop()) / view.GetTextMetrics().GetHankakuDy())
	);
}


// �s�ԍ��G���A���܂ޔ͈�
void TextArea::GenerateTopRect(RECT* rc, LayoutInt nLineCount) const
{
	rc->left   = 0; //nViewAlignLeft;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + (Int)nLineCount * editView.GetTextMetrics().GetHankakuDy();
}

// �s�ԍ��G���A���܂ޔ͈�
void TextArea::GenerateBottomRect(RECT* rc, LayoutInt nLineCount) const
{
	rc->left   = 0; //nViewAlignLeft;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop  + nViewCy - (Int)nLineCount * editView.GetTextMetrics().GetHankakuDy();
	rc->bottom = nViewAlignTop  + nViewCy;
}

void TextArea::GenerateLeftRect(RECT* rc, LayoutInt nColCount) const
{
	rc->left   = nViewAlignLeft;
	rc->right  = nViewAlignLeft + (Int)nColCount * editView.GetTextMetrics().GetHankakuDx();
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + nViewCy;
}

void TextArea::GenerateRightRect(RECT* rc, LayoutInt nColCount) const
{
	rc->left   = nViewAlignLeft + nViewCx - (Int)nColCount * editView.GetTextMetrics().GetHankakuDx(); // 2008.01.26 kobake �������t�ɂȂ��Ă��̂��C��
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop  + nViewCy;
}

void TextArea::GenerateLineNumberRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = nViewAlignLeft;
	rc->top    = 0;
	rc->bottom = nViewAlignTop + nViewCy;
}

void TextArea::GenerateTextAreaRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + nViewCy;
}


int TextArea::GenerateYPx(LayoutYInt nLineNum) const
{
	LayoutYInt nY = nLineNum - GetViewTopLine();
	int ret;
	if (nY < 0) {
		ret = GetAreaTop();
	}else if (nViewRowNum < nY) {
		ret = GetAreaBottom();
	}else {
		ret = GetAreaTop() + editView.GetTextMetrics().GetHankakuDy() * (Int)(nY);
	}
	return ret;
}

