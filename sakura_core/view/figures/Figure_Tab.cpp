#include "StdAfx.h"
#include "view/EditView.h"
#include "Figure_Tab.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "types/TypeSupport.h"

void _DispTab(Graphics& gr, DispPos* pDispPos, const EditView* pView);
// �^�u���`��֐�
void _DrawTabArrow(Graphics& gr, int nPosX, int nPosY, int nWidth, int nHeight, bool bBold, COLORREF pColor);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Figure_Tab                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_Tab::Match(const wchar_t* pText, int nTextLen) const
{
	return (pText[0] == WCODE::TAB);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TAB�`�� */
void Figure_Tab::DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const
{
	DispPos& sPos = *pDispPos;

	// �K�v�ȃC���^�[�t�F�[�X
	const TextMetrics* pMetrics = &view.GetTextMetrics();
	const TextArea* pArea = &view.GetTextArea();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();

	TypeSupport tabType(view, COLORIDX_TAB);

	// ���ꂩ��`�悷��^�u��
	size_t tabDispWidth = view.pEditDoc->layoutMgr.GetActualTabSpace(sPos.GetDrawCol());

	// �^�u�L���̈�
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if (rcClip2.left < pArea->GetAreaLeft()) {
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if (pArea->IsRectIntersected(rcClip2)) {
		if (tabType.IsDisp() && TabArrowType::String == pTypeData->bTabArrow) {	// �^�u�ʏ�\��
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				pTypeData->szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8,
				pMetrics->GetDxArray_AllHankaku()
			);
		}else {
			// �w�i
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				L"        ",
				tabDispWidth <= 8 ? tabDispWidth : 8,
				pMetrics->GetDxArray_AllHankaku()
			);

			// �^�u���\��
			if (tabType.IsDisp()) {
				// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�
				// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
				//	�������ǂ����ݒ������l�ɂ���
				// �����F�A������Graphics����擾

				if (TabArrowType::Short == pTypeData->bTabArrow) {
					if (rcClip2.left <= sPos.GetDrawPos().x) { // �s�ԍ��Əd�Ȃ�
						_DrawTabArrow(
							gr,
							sPos.GetDrawPos().x,
							sPos.GetDrawPos().y,
							pMetrics->GetHankakuWidth(),
							pMetrics->GetHankakuHeight(),
							gr.GetCurrentMyFontBold() || pTypeData->colorInfoArr[COLORIDX_TAB].fontAttr.bBoldFont,
							gr.GetCurrentTextForeColor()
						);
					}
				}else if (TabArrowType::Long == pTypeData->bTabArrow) {
					int	nPosLeft = rcClip2.left > sPos.GetDrawPos().x ? rcClip2.left : sPos.GetDrawPos().x;
					_DrawTabArrow(
						gr,
						nPosLeft,
						sPos.GetDrawPos().y,
						nCharWidth * tabDispWidth - (nPosLeft -  sPos.GetDrawPos().x),	// Tab Area��t��
						pMetrics->GetHankakuHeight(),
						gr.GetCurrentMyFontBold() || pTypeData->colorInfoArr[COLORIDX_TAB].fontAttr.bBoldFont,
						gr.GetCurrentTextForeColor()
					);
				}
			}
		}
	}

	// X��i�߂�
	sPos.ForwardDrawCol(tabDispWidth);
}



/*
	�^�u���`��֐�
*/
void _DrawTabArrow(
	Graphics&	gr,
	int			nPosX,   // �s�N�Z��X
	int			nPosY,   // �s�N�Z��Y
	int			nWidth,  // �s�N�Z��W
	int			nHeight, // �s�N�Z��H
	bool		bBold,
	COLORREF	pColor
)
{
	// �y���ݒ�
	gr.PushPen(pColor, 0);

	// ���̐擪
	int sx = nPosX + nWidth - 2;
	int sy = nPosY + (nHeight / 2);
	int sa = nHeight / 4;								// �V��size

	DWORD pp[] = { 3, 2 };
	POINT pt[5];
	pt[0].x = nPosX;	//�u���v���[����E�[
	pt[0].y = sy;
	pt[1].x = sx;		//�u�^�v�E�[����΂ߍ���
	pt[1].y = sy;
	pt[2].x = sx - sa;	//	���̐�[�ɖ߂�
	pt[2].y = sy + sa;
	pt[3].x = sx;		//�u�_�v�E�[����΂ߍ���
	pt[3].y = sy;
	pt[4].x = sx - sa;
	pt[4].y = sy - sa;
	::PolyPolyline(gr, pt, pp, _countof(pp));

	if (bBold) {
		pt[0].x += 0;	//�u���v���[����E�[
		pt[0].y += 1;
		pt[1].x += 0;	//�u�^�v�E�[����΂ߍ���
		pt[1].y += 1;
		pt[2].x += 0;	//	���̐�[�ɖ߂�
		pt[2].y += 1;
		pt[3].x += 0;	//�u�_�v�E�[����΂ߍ���
		pt[3].y += 1;
		pt[4].x += 0;
		pt[4].y += 1;
		::PolyPolyline(gr, pt, pp, _countof(pp));
	}

	gr.PopPen();
}


