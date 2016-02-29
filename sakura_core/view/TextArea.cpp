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

TextArea::TextArea(EditView* pEditView)
	:
	m_pEditView(pEditView)
{
	DllSharedData* pShareData = &GetDllShareData();

	m_nViewAlignLeft = 0;					// �\����̍��[���W
	m_nViewAlignLeftCols = 0;				// �s�ԍ���̌���
	m_nViewCx = 0;							// �\����̕�
	m_nViewCy = 0;							// �\����̍���
	m_nViewColNum = LayoutInt(0);			// �\����̌���
	m_nViewRowNum = LayoutInt(0);			// �\����̍s��
	m_nViewTopLine = LayoutInt(0);			// �\����̈�ԏ�̍s
	m_nViewLeftCol = LayoutInt(0);			// �\����̈�ԍ��̌�
	SetTopYohaku(pShareData->common.window.nRulerBottomSpace); 	// ���[���[�ƃe�L�X�g�̌���
	SetLeftYohaku(pShareData->common.window.nLineNumRightSpace);
	m_nViewAlignTop = GetTopYohaku();		// �\����̏�[���W
}

TextArea::~TextArea()
{
}

void TextArea::CopyTextAreaStatus(TextArea* pDst) const
{
	pDst->SetAreaLeft				(this->GetAreaLeft());		// �\����̍��[���W
	pDst->m_nViewAlignLeftCols		= this->m_nViewAlignLeftCols;	// �s�ԍ���̌���
	pDst->SetAreaTop				(this->GetAreaTop());			// �\����̏�[���W
//	pDst->m_nViewCx					= m_nViewCx;					// �\����̕�
//	pDst->m_nViewCy					= m_nViewCy;					// �\����̍���
//	pDst->m_nViewColNum				= this->m_nViewColNum;			// �\����̌���
//	pDst->m_nViewRowNum				= this->m_nViewRowNum;			// �\����̍s��
	pDst->SetViewTopLine			(this->GetViewTopLine());		// �\����̈�ԏ�̍s(0�J�n)
	pDst->SetViewLeftCol			(this->GetViewLeftCol());		// �\����̈�ԍ��̌�(0�J�n)
}

// �\����̍Čv�Z
void TextArea::UpdateViewColRowNums()
{
	EditView* pView = m_pEditView;
	// Note: �}�C�i�X�̊���Z�͏����n�ˑ��ł��B
	// 0���ƃJ�[�\����ݒ�ł��Ȃ��E�I���ł��Ȃ��ȂǓ���s�ǂɂȂ�̂�1�ȏ�ɂ���
	m_nViewColNum = LayoutInt(t_max(1, t_max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx()));	// �\����̌���
	m_nViewRowNum = LayoutInt(t_max(1, t_max(0, m_nViewCy - 1) / pView->GetTextMetrics().GetHankakuDy()));	// �\����̍s��
}

//�t�H���g�ύX�̍ہA�e��p�����[�^���v�Z������
void TextArea::UpdateAreaMetrics(HDC hdc)
{
	EditView* pView = m_pEditView;

	if (pView->m_bMiniMap) {
		// �����Ԋu
		pView->GetTextMetrics().SetHankakuDx(pView->GetTextMetrics().GetHankakuWidth());

		// �s�Ԋu
		pView->GetTextMetrics().SetHankakuDy(pView->GetTextMetrics().GetHankakuHeight());
	}else {
		// �����Ԋu
		pView->GetTextMetrics().SetHankakuDx(pView->GetTextMetrics().GetHankakuWidth() + pView->m_pTypeData->nColumnSpace);
	
		// �s�Ԋu
		pView->GetTextMetrics().SetHankakuDy(pView->GetTextMetrics().GetHankakuHeight() + pView->m_pTypeData->nLineSpace);
	}

	// �\����̍Čv�Z
	// 2010.08.24 Dx/Dy���g���̂Ō�Őݒ�
	UpdateViewColRowNums();
}

void TextArea::GenerateCharRect(RECT* rc, const DispPos& pos, int nHankakuNum) const
{
	const EditView* pView = m_pEditView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = pos.GetDrawPos().x + pView->GetTextMetrics().GetHankakuDx() * nHankakuNum;
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + pView->GetTextMetrics().GetHankakuDy();
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
	const EditView* pView = m_pEditView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = GetAreaRight();
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + pView->GetTextMetrics().GetHankakuDy();

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
	rc->bottom = pos.GetDrawPos().y + m_pEditView->GetTextMetrics().GetHankakuDy();
	return true;
}


/*
�s�ԍ��\���ɕK�v�ȕ���ݒ�B�����ύX���ꂽ�ꍇ��TRUE��Ԃ�
*/
bool TextArea::DetectWidthOfLineNumberArea(bool bRedraw)
{
	const EditView* pView = m_pEditView;
	EditView* pView2 = m_pEditView;

	int nViewAlignLeftNew;

	if (pView->m_pTypeData->colorInfoArr[COLORIDX_GYOU].bDisp && !pView->m_bMiniMap) {
		// �s�ԍ��\���ɕK�v�Ȍ������v�Z
		int i = DetectWidthOfLineNumberArea_calculate(&pView->m_pEditDoc->m_layoutMgr);
		nViewAlignLeftNew = pView->GetTextMetrics().GetHankakuDx() * (i + 1);	// �\����̍��[���W
		m_nViewAlignLeftCols = i + 1;
	}else if (pView->m_bMiniMap) {
		nViewAlignLeftNew = 4;
		m_nViewAlignLeftCols = 0;
	}else {
		nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}

	//	Sep 18, 2002 genta
	nViewAlignLeftNew += GetDllShareData().common.window.nLineNumRightSpace;
	if (nViewAlignLeftNew != GetAreaLeft()) {
		Rect			rc;
		SetAreaLeft(nViewAlignLeftNew);
		pView->GetClientRect(&rc);
		int nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL); // �����X�N���[���o�[�̉���
		m_nViewCx = rc.Width() - nCxVScroll - GetAreaLeft(); // �\����̕�
		// 2008.05.27 nasukoji	�\����̌������Z�o����i�E�[�J�[�\���ړ����̕\���ꏊ����ւ̑Ώ��j
		// m_nViewColNum = LayoutInt(t_max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// �\����̌���
		UpdateViewColRowNums();

		if (bRedraw && pView2->GetDrawSwitch()) {
			// �ĕ`��
			pView2->GetCaret().m_underLine.Lock();
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			pView2->Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Ruler | (int)PaintAreaType::Body, false); // �������c�b���g�p���Ă�����̂Ȃ��ĕ`��
			// To Here 2007.09.09 Moca
			pView2->GetCaret().m_underLine.UnLock();
			pView2->GetCaret().ShowEditCaret();
			/*
			PAINTSTRUCT		ps;
			HDC hdc = ::GetDC(pView->m_hWnd);
			ps.rcPaint.left   = 0;
			ps.rcPaint.right  = GetAreaRight();
			ps.rcPaint.top    = 0;
			ps.rcPaint.bottom = GetAreaBottom();
			pView2->GetCaret().m_underLine.Lock();
			pView2->OnPaint(hdc, &ps, TRUE);	
			GetCaret().m_underLine.UnLock();
			pView2->GetCaret().ShowEditCaret();
			::ReleaseDC(m_hWnd, hdc);
			*/
		}
		pView2->GetRuler().SetRedrawFlag();
		return true;
	}else {
		return false;
	}
}


// �s�ԍ��\���ɕK�v�Ȍ������v�Z
int TextArea::DetectWidthOfLineNumberArea_calculate(const LayoutMgr* pLayoutMgr, bool bLayout) const
{
	const EditView* pView = m_pEditView;

	int nAllLines; //$$ �P�ʍ���

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (pView->m_pTypeData->bLineNumIsCRLF && !bLayout) {
		nAllLines = pView->m_pEditDoc->m_docLineMgr.GetLineCount();
	}else {
		nAllLines = (Int)pLayoutMgr->GetLineCount();
	}
	
	if (0 < nAllLines) {
		int nWork;
		int i;

		// �s�ԍ��̌��������߂� 2014.07.26 katze
		// m_nLineNumWidth�͏����ɐ����̌����������A�擪�̋󔒂��܂܂Ȃ��i�d�l�ύX�j 2014.08.02 katze
#ifdef USE_LOG10
		// �\�����Ă���s���̌��������߂�
		nWork = (int)(log10( (double)nAllLines) +1);	// 10���Ƃ���ΐ�(�����_�ȉ��؂�̂�)+1�Ō���
		// �ݒ�l�Ɣ�r���A�傫���������
		i = nWork > pView->m_pTypeData->m_nLineNumWidth ?
			nWork : pView->m_pTypeData->m_nLineNumWidth;
		// �擪�̋󔒕������Z����
		return (i +1);
#else
		// �ݒ肩��s�������߂�
		nWork = 10;
		for (i=1; i<pView->m_pTypeData->nLineNumWidth; ++i) {
			nWork *= 10;
		}
		// �\�����Ă���s���Ɣ�r���A�傫�����̒l�����
		for (i=pView->m_pTypeData->nLineNumWidth; i<LINENUMWIDTH_MAX; ++i) {
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
		return pView->m_pTypeData->nLineNumWidth +1;
	}
}

void TextArea::TextArea_OnSize(
	const Size& sizeClient, // �E�B���h�E�̃N���C�A���g�T�C�Y
	int nCxVScroll,            // �����X�N���[���o�[�̉���
	int nCyHScroll             // �����X�N���[���o�[�̏c��
	)
{
	m_nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // �\����̕�
	m_nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // �\����̍���
	UpdateViewColRowNums();
}


int TextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - (Int)GetViewLeftCol() * m_pEditView->GetTextMetrics().GetHankakuDx();
}

// �N���C�A���g���W���烌�C�A�E�g�ʒu�ɕϊ�����
void TextArea::ClientToLayout(Point ptClient, LayoutPoint* pptLayout) const
{
	const EditView* pView = m_pEditView;
	pptLayout->Set(
		GetViewLeftCol() + LayoutInt((ptClient.x - GetAreaLeft()) / pView->GetTextMetrics().GetHankakuDx()),
		GetViewTopLine() + LayoutInt((ptClient.y - GetAreaTop()) / pView->GetTextMetrics().GetHankakuDy())
	);
}


// �s�ԍ��G���A���܂ޔ͈�
void TextArea::GenerateTopRect(RECT* rc, LayoutInt nLineCount) const
{
	rc->left   = 0; //m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + (Int)nLineCount * m_pEditView->GetTextMetrics().GetHankakuDy();
}

// �s�ԍ��G���A���܂ޔ͈�
void TextArea::GenerateBottomRect(RECT* rc, LayoutInt nLineCount) const
{
	rc->left   = 0; //m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop  + m_nViewCy - (Int)nLineCount * m_pEditView->GetTextMetrics().GetHankakuDy();
	rc->bottom = m_nViewAlignTop  + m_nViewCy;
}

void TextArea::GenerateLeftRect(RECT* rc, LayoutInt nColCount) const
{
	rc->left   = m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + (Int)nColCount * m_pEditView->GetTextMetrics().GetHankakuDx();
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}

void TextArea::GenerateRightRect(RECT* rc, LayoutInt nColCount) const
{
	rc->left   = m_nViewAlignLeft + m_nViewCx - (Int)nColCount * m_pEditView->GetTextMetrics().GetHankakuDx(); // 2008.01.26 kobake �������t�ɂȂ��Ă��̂��C��
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop  + m_nViewCy;
}

void TextArea::GenerateLineNumberRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = m_nViewAlignLeft;
	rc->top    = 0;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}

void TextArea::GenerateTextAreaRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}


int TextArea::GenerateYPx(LayoutYInt nLineNum) const
{
	LayoutYInt nY = nLineNum - GetViewTopLine();
	int ret;
	if (nY < 0) {
		ret = GetAreaTop();
	}else if (m_nViewRowNum < nY) {
		ret = GetAreaBottom();
	}else {
		ret = GetAreaTop() + m_pEditView->GetTextMetrics().GetHankakuDy() * (Int)(nY);
	}
	return ret;
}

