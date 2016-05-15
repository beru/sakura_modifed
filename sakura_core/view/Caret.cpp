/*!	@file
	@brief �L�����b�g�̊Ǘ�

	@author	kobake
*/
/*
	Copyright (C) 2008, kobake, ryoji, Uchi
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji, Moca
	Copyright (C) 2011, Moca, syat
	Copyright (C) 2012, ryoji, Moca
	Copyright (C) 2013, Moca, Uchi

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
#include <algorithm>
#include "view/Caret.h"
#include "view/EditView.h"
#include "view/TextArea.h"
#include "view/TextMetrics.h"
#include "view/ViewFont.h"
#include "view/Ruler.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "mem/MemoryIterator.h"
#include "charset/charcode.h"
#include "charset/CodePage.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "window/EditWnd.h"

using namespace std;

#define SCROLLMARGIN_LEFT 4
#define SCROLLMARGIN_RIGHT 4
#define SCROLLMARGIN_NOMOVE 4

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �O���ˑ�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


inline int Caret::GetHankakuDx() const
{
	return editView.GetTextMetrics().GetHankakuDx();
}

inline int Caret::GetHankakuHeight() const
{
	return editView.GetTextMetrics().GetHankakuHeight();
}

inline int Caret::GetHankakuDy() const
{
	return editView.GetTextMetrics().GetHankakuDy();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CaretUnderLine                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �J�[�\���s�A���_�[���C����ON
void CaretUnderLine::CaretUnderLineON(bool bDraw, bool bPaintDraw)
{
	if (nLockCounter) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	editView.CaretUnderLineON(bDraw, bPaintDraw, nUnderLineLockCounter != 0);
}

// �J�[�\���s�A���_�[���C����OFF
void CaretUnderLine::CaretUnderLineOFF(bool bDraw, bool bDrawPaint, bool bResetFlag)
{
	if (nLockCounter) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	editView.CaretUnderLineOFF(bDraw, bDrawPaint, bResetFlag, nUnderLineLockCounter != 0);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Caret::Caret(EditView& editView, const EditDoc& editDoc)
	:
	editView(editView),
	editDoc(editDoc),
	ptCaretPos_Layout(0, 0),
	ptCaretPos_Logic(0, 0),			// �J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	sizeCaret(0, 0),					// �L�����b�g�̃T�C�Y
	underLine(editView)
{
	nCaretPosX_Prev = LayoutInt(0);	// �r���[���[����̃J�[�\�������O�̈ʒu(�O�I���W��)

	crCaret = -1;				// �L�����b�g�̐F				// 2006.12.16 ryoji
	hbmpCaret = NULL;			// �L�����b�g�p�r�b�g�}�b�v		// 2006.11.28 ryoji
	bClearStatus = true;
	ClearCaretPosInfoCache();
}

Caret::~Caret()
{
	// �L�����b�g�p�r�b�g�}�b�v	// 2006.11.28 ryoji
	if (hbmpCaret)
		DeleteObject(hbmpCaret);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief �s���w��ɂ��J�[�\���ړ�

	�K�v�ɉ����ďc/���X�N���[��������D
	�����X�N���[���������ꍇ�͂��̍s����Ԃ��i���^���j�D
	
	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �s���Ȉʒu���w�肳�ꂽ�ꍇ�ɂ͓K�؂ȍ��W�l��
		�ړ����邽�߁C�����ŗ^�������W�ƈړ���̍��W��
		�K��������v���Ȃ��D
	
	@note bScroll��false�̏ꍇ�ɂ̓J�[�\���ʒu�݈̂ړ�����D
		true�̏ꍇ�ɂ̓X�N���[���ʒu�����킹�ĕύX�����

	@note �����s�̍��E�ړ��̓A���_�[���C������x�����K�v�������̂�
		bUnderlineDoNotOFF���w�肷��ƍ������ł���.
		���l�ɓ������̏㉺�ړ���bVertLineDoNotOFF���w�肷���
		�J�[�\���ʒu�c���̏������Ȃ��č������ł���.

	@date 2001.10.20 deleted by novice AdjustScrollBar()���ĂԈʒu��ύX
	@date 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	@date 2004.09.11 genta bDraw�X�C�b�`�͓���Ɩ��̂���v���Ă��Ȃ��̂�
		�ĕ`��X�C�b�`����ʈʒu�����X�C�b�`�Ɩ��̕ύX
	@date 2009.08.28 nasukoji	�e�L�X�g�܂�Ԃ��́u�܂�Ԃ��Ȃ��v�Ή�
	@date 2010.11.27 syat �A���_�[���C���A�c�����������Ȃ��t���O��ǉ�
*/
LayoutInt Caret::MoveCursor(
	LayoutPoint	ptWk_CaretPos,		// [in] �ړ��惌�C�A�E�g�ʒu
	bool		bScroll,			// [in] true: ��ʈʒu�����L��  false: ��ʈʒu��������
	int			nCaretMarginRate,	// [in] �c�X�N���[���J�n�ʒu�����߂�l
	bool		bUnderLineDoNotOFF,	// [in] �A���_�[���C�����������Ȃ�
	bool		bVertLineDoNotOFF	// [in] �J�[�\���ʒu�c�����������Ȃ�
	)
{
	// �X�N���[������
	LayoutInt	nScrollRowNum = LayoutInt(0);
	LayoutInt	nScrollColNum = LayoutInt(0);
	int			nCaretMarginY;
	LayoutInt	nScrollMarginRight;
	LayoutInt	nScrollMarginLeft;

	auto& textArea = editView.GetTextArea();
	if (0 >= textArea.nViewColNum) {
		return LayoutInt(0);
	}

	if (editView.GetSelectionInfo().IsMouseSelecting()) {	// �͈͑I��
		nCaretMarginY = 0;
	}else {
		//	2001/10/20 novice
		nCaretMarginY = (Int)textArea.nViewRowNum / nCaretMarginRate;
		if (1 > nCaretMarginY) {
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	GetAdjustCursorPos(&ptWk_CaretPos);
	editDoc.layoutMgr.LayoutToLogic(
		ptWk_CaretPos,
		&ptCaretPos_Logic	// �J�[�\���ʒu�B���W�b�N�P�ʁB
	);
	// �L�����b�g�ړ�
	SetCaretLayoutPos(ptWk_CaretPos);

	// �J�[�\���s�A���_�[���C����OFF
	bool bDrawPaint = ptWk_CaretPos.GetY2() != editView.nOldUnderLineYBg;
	underLine.SetUnderLineDoNotOFF(bUnderLineDoNotOFF);
	underLine.SetVertLineDoNotOFF(bVertLineDoNotOFF);
	underLine.CaretUnderLineOFF(bScroll, bDrawPaint);	//	YAZAKI
	underLine.SetUnderLineDoNotOFF(false);
	underLine.SetVertLineDoNotOFF(false);
	
	// �����X�N���[���ʁi�������j�̎Z�o
	nScrollColNum = LayoutInt(0);
	nScrollMarginRight = LayoutInt(SCROLLMARGIN_RIGHT);
	nScrollMarginLeft = LayoutInt(SCROLLMARGIN_LEFT);

	// 2010.08.24 Moca ���������ꍇ�̃}�[�W���̒���
	{
		// �J�[�\�����^�񒆂ɂ���Ƃ��ɍ��E�ɂԂ�Ȃ��悤��
		int nNoMove = SCROLLMARGIN_NOMOVE;
		LayoutInt a = ((textArea.nViewColNum) - nNoMove) / 2;
		LayoutInt nMin = (2 <= a ? a : LayoutInt(0)); // 1���ƑS�p�ړ��Ɏx�Ⴊ����̂�2�ȏ�
		nScrollMarginRight = t_min(nScrollMarginRight, nMin);
		nScrollMarginLeft  = t_min(nScrollMarginLeft,  nMin);
	}
	
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if (editDoc.layoutMgr.GetMaxLineKetas() > textArea.nViewColNum
		&& ptWk_CaretPos.GetX() > textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight
	) {
		nScrollColNum = (textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight) - ptWk_CaretPos.GetX2();
	}else if (1
		&& 0 < textArea.GetViewLeftCol()
		&& ptWk_CaretPos.GetX() < textArea.GetViewLeftCol() + nScrollMarginLeft
	) {
		nScrollColNum = textArea.GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.GetX2();
		if (0 > textArea.GetViewLeftCol() - nScrollColNum) {
			nScrollColNum = textArea.GetViewLeftCol();
		}
	}

	// 2013.12.30 bScroll��OFF�̂Ƃ��͉��X�N���[�����Ȃ�
	if (bScroll) {
		textArea.SetViewLeftCol(textArea.GetViewLeftCol() - nScrollColNum);
	}else {
		nScrollColNum = 0;
	}

	//	From Here 2007.07.28 ���イ�� : �\���s����3�s�ȉ��̏ꍇ�̓�����P
	// �����X�N���[���ʁi�s���j�̎Z�o
	// ��ʂ��R�s�ȉ�
	if (textArea.nViewRowNum <= 3) {
		// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
		if (ptWk_CaretPos.y - textArea.GetViewTopLine() < nCaretMarginY) {
			if (ptWk_CaretPos.y < nCaretMarginY) {	// �P�s�ڂɈړ�
				nScrollRowNum = textArea.GetViewTopLine();
			}else if (textArea.nViewRowNum <= 1) {	// ��ʂ��P�s
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#if !(0)	// COMMENT�ɂ���ƁA�㉺�̋󂫂����炵�Ȃ��ׁA�c�ړ���good�����A���ړ��̏ꍇ�㉺�ɂԂ��
			else if (textArea.nViewRowNum <= 2) {	// ��ʂ��Q�s
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#endif
			else {						// ��ʂ��R�s
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
		}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= (textArea.nViewRowNum - nCaretMarginY - 2)) {
			LayoutInt ii = editDoc.layoutMgr.GetLineCount();
			if (1
				&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
				&& ii - textArea.GetViewTopLine() < textArea.nViewRowNum
			) {
			}else if (textArea.nViewRowNum <= 2) {	// ��ʂ��Q�s�A�P�s
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}else {						// ��ʂ��R�s
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}
	// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() < nCaretMarginY) {
		if (ptWk_CaretPos.y < nCaretMarginY) {	// �P�s�ڂɈړ�
			nScrollRowNum = textArea.GetViewTopLine();
		}else {
			nScrollRowNum = -(ptWk_CaretPos.y - textArea.GetViewTopLine()) + nCaretMarginY;
		}
	// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= textArea.nViewRowNum - nCaretMarginY - 2) {
		LayoutInt ii = editDoc.layoutMgr.GetLineCount();
		if (1
			&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
			&& ii - textArea.GetViewTopLine() < textArea.nViewRowNum
		) {
		}else {
			nScrollRowNum = -(ptWk_CaretPos.y - textArea.GetViewTopLine()) + (textArea.nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 ���イ��
	if (bScroll) {
		// �X�N���[��
		if (0
			|| t_abs(nScrollColNum) >= textArea.nViewColNum
			|| t_abs(nScrollRowNum) >= textArea.nViewRowNum
		) {
			textArea.OffsetViewTopLine(-nScrollRowNum);
			if (editView.GetDrawSwitch()) {
				editView.InvalidateRect(NULL);
				if (editView.editWnd.GetMiniMap().GetHwnd()) {
					editView.MiniMapRedraw(true);
				}
			}
		}else if (nScrollRowNum != 0 || nScrollColNum != 0) {
			RECT	rcClip;
			RECT	rcClip2;
			RECT	rcScroll;

			textArea.GenerateTextAreaRect(&rcScroll);
			if (nScrollRowNum > 0) {
				rcScroll.bottom = textArea.GetAreaBottom() - (Int)nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateTopRect(&rcClip, nScrollRowNum);
			}else if (nScrollRowNum < 0) {
				rcScroll.top = textArea.GetAreaTop() - (Int)nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateBottomRect(&rcClip, -nScrollRowNum);
			}

			if (nScrollColNum > 0) {
				rcScroll.left = textArea.GetAreaLeft();
				rcScroll.right = textArea.GetAreaRight() - (Int)nScrollColNum * GetHankakuDx();
				textArea.GenerateLeftRect(&rcClip2, nScrollColNum);
			}else if (nScrollColNum < 0) {
				rcScroll.left = textArea.GetAreaLeft() - (Int)nScrollColNum * GetHankakuDx();
				textArea.GenerateRightRect(&rcClip2, -nScrollColNum);
			}

			if (editView.GetDrawSwitch()) {
				editView.ScrollDraw(nScrollRowNum, nScrollColNum, rcScroll, rcClip, rcClip2);
				if (editView.editWnd.GetMiniMap().GetHwnd()) {
					editView.MiniMapRedraw(false);
				}
			}
		}

		// �X�N���[���o�[�̏�Ԃ��X�V����
		editView.AdjustScrollBars(); // 2001/10/20 novice
	}

	// ���X�N���[��������������A���[���[�S�̂��ĕ`�� 2002.02.25 Add By KK
	if (nScrollColNum != 0) {
		// ����DispRuler�Ăяo�����ɍĕ`��B�ibDraw=false�̃P�[�X���l�������B�j
		editView.GetRuler().SetRedrawFlag();
	}

	// �J�[�\���s�A���_�[���C����ON
	//CaretUnderLineON(bDraw); //2002.02.27 Del By KK �A���_�[���C���̂������ጸ
	if (bScroll) {
		// �L�����b�g�̕\���E�X�V
		ShowEditCaret();

		// ���[���̍ĕ`��
		HDC		hdc = editView.GetDC();
		editView.GetRuler().DispRuler(hdc);
		editView.ReleaseDC(hdc);

		// �A���_�[���C���̍ĕ`��
		underLine.CaretUnderLineON(true, bDrawPaint);

		// �L�����b�g�̍s���ʒu��\������
		ShowCaretPosInfo();

		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		//	bScroll == FALSE�̎��ɂ̓X�N���[�����Ȃ��̂ŁC���s���Ȃ�
		editView.SyncScrollV(-nScrollRowNum);	//	�������t�Ȃ̂ŕ������]���K�v
		editView.SyncScrollH(-nScrollColNum);	//	�������t�Ȃ̂ŕ������]���K�v

	}

// 02/09/18 �Ί��ʂ̋����\�� ai Start	03/02/18 ai mod S
	editView.DrawBracketPair(false);
	editView.SetBracketPairPos(true);
	editView.DrawBracketPair(true);
// 02/09/18 �Ί��ʂ̋����\�� ai End		03/02/18 ai mod E

	return nScrollRowNum;
}


LayoutInt Caret::MoveCursorFastMode(
	const LogicPoint& ptWk_CaretPosLogic	// [in] �ړ��惍�W�b�N�ʒu
	)
{
	// fastMode
	SetCaretLogicPos(ptWk_CaretPosLogic);
	return LayoutInt(0);
}

/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ�(���^��)
*/
// 2007.09.11 kobake �֐����ύX: MoveCursorToPoint��MoveCursorToClientPoint
LayoutInt Caret::MoveCursorToClientPoint(
	const POINT& ptClientPos,
	bool test,
	LayoutPoint* pCaretPosNew
	)
{
	LayoutPoint	ptLayoutPos;
	editView.GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - editView.GetTextArea().GetAreaLeft()) % (editView.GetTextMetrics().GetHankakuDx());
	LayoutInt nScrollRowNum = MoveCursorProperly(ptLayoutPos, true, test, pCaretPosNew, 1000, dx);
	if (!test) {
		nCaretPosX_Prev = GetCaretLayoutPos().GetX2();
	}
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE


/*! �������J�[�\���ʒu���Z�o����(EOF�ȍ~�̂�)
	@param pptPosXY [in/out] �J�[�\���̃��C�A�E�g���W
	@retval	TRUE ���W���C������
	@retval	FALSE ���W�͏C������Ȃ�����
	@note	EOF�̒��O�����s�łȂ��ꍇ�́A���̍s�Ɍ���EOF�ȍ~�ɂ��ړ��\
			EOF�����̍s�́A�擪�ʒu�̂ݐ������B
	@date 2004.04.02 Moca �֐���
*/
bool Caret::GetAdjustCursorPos(
	LayoutPoint* pptPosXY
	)
{
	// 2004.03.28 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	LayoutInt nLayoutLineCount = editDoc.layoutMgr.GetLineCount();

	LayoutPoint ptPosXY2 = *pptPosXY;
	bool ret = false;
	if (ptPosXY2.y >= nLayoutLineCount) {
		if (0 < nLayoutLineCount) {
			ptPosXY2.y = nLayoutLineCount - 1;
			const Layout* pLayout = editDoc.layoutMgr.SearchLineByLayoutY(ptPosXY2.GetY2());
			if (pLayout->GetLayoutEol() == EolType::None) {
				ptPosXY2.x = editView.LineIndexToColumn(pLayout, (LogicInt)pLayout->GetLengthWithEOL());
				// [EOF]�̂ݐ܂�Ԃ��̂͂�߂�	// 2009.02.17 ryoji
				// ��������Ȃ� ptPosXY2.x �ɐ܂�Ԃ��s�C���f���g��K�p����̂��悢

				// EOF�����܂�Ԃ���Ă��邩
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				//if (ptPosXY2.x >= editDoc.layoutMgr.GetMaxLineKetas()) {
				//	ptPosXY2.y++;
				//	ptPosXY2.x = LayoutInt(0);
				//}
			}else {
				// EOF�����̍s
				ptPosXY2.y++;
				ptPosXY2.x = LayoutInt(0);
			}
		}else {
			// ��̃t�@�C��
			ptPosXY2.Set(LayoutInt(0), LayoutInt(0));
		}
		if (*pptPosXY != ptPosXY2) {
			*pptPosXY = ptPosXY2;
			ret = true;
		}
	}
	return ret;
}

// �L�����b�g�̕\���E�X�V
void Caret::ShowEditCaret()
{
	if (editView.bMiniMap) {
		return;
	}
	// �K�v�ȃC���^�[�t�F�[�X
	auto& layoutMgr = editDoc.layoutMgr;
	auto& csGeneral = GetDllShareData().common.general;
	const TypeConfig* pTypes = &editDoc.docType.GetDocumentAttribute();

	using namespace WCODE;

/*
	�t�H�[�J�X�������Ƃ��ɓ����I�ɃL�����b�g�쐬����ƈÖٓI�ɃL�����b�g�j���i���j����Ă�
	�L�����b�g������inCaretWidth != 0�j�Ƃ������ƂɂȂ��Ă��܂��A�t�H�[�J�X���擾���Ă�
	�L�����b�g���o�Ă��Ȃ��Ȃ�ꍇ������
	�t�H�[�J�X�������Ƃ��̓L�����b�g���쐬�^�\�����Ȃ��悤�ɂ���

	���L�����b�g�̓X���b�h�ɂЂƂ����Ȃ̂ŗႦ�΃G�f�B�b�g�{�b�N�X���t�H�[�J�X�擾�����
	�@�ʌ`��̃L�����b�g�ɈÖٓI�ɍ����ւ����邵�t�H�[�J�X�������ΈÖٓI�ɔj�������

	2007.12.11 ryoji
	�h���b�O�A���h�h���b�v�ҏW���̓L�����b�g���K�v�ňÖٔj���̗v���������̂ŗ�O�I�ɕ\������
*/
	if (::GetFocus() != editView.GetHwnd() && !editView.bDragMode) {
		sizeCaret.cx = 0;
		return;
	}
	// 2014.07.02 GetDrawSwitch������
	if (!editView.GetDrawSwitch()) {
		return;
	}

	// CalcCaretDrawPos�̂��߂�Caret�T�C�Y�����ݒ�
	int	nCaretWidth = 0;
	int	nCaretHeight = 0;
	if (csGeneral.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();
		if (editView.IsInsMode()) {
			nCaretWidth = 2;
		}else {
			nCaretWidth = GetHankakuDx();
		}
	}else if (csGeneral.GetCaretType() == 1) {
		if (editView.IsInsMode()) {
			nCaretHeight = GetHankakuHeight() / 2;
		}else {
			nCaretHeight = GetHankakuHeight();
		}
		nCaretWidth = GetHankakuDx();
	}
	Size caretSizeOld = GetCaretSize();
	SetCaretSize(nCaretWidth, nCaretHeight);
	POINT ptDrawPos = CalcCaretDrawPos(GetCaretLayoutPos());
	SetCaretSize(caretSizeOld.cx, caretSizeOld.cy); // ��Ŕ�r����̂Ŗ߂�
	bool bShowCaret = false;
	auto& textArea = editView.GetTextArea();
	if (1
		&& textArea.GetAreaLeft() <= ptDrawPos.x
		&& textArea.GetAreaTop() <= ptDrawPos.y
		&& ptDrawPos.x < textArea.GetAreaRight()
		&& ptDrawPos.y < textArea.GetAreaBottom()
	) {
		// �L�����b�g�̕\��
		bShowCaret = true;
	}
	// �L�����b�g�̕��A����������
	// �J�[�\���̃^�C�v = win
	if (csGeneral.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();					// �L�����b�g�̍���
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretWidth = 2; // 2px
			// 2011.12.22 �V�X�e���̐ݒ�ɏ]��(����2px�ȏ�)
			DWORD dwWidth;
			if (::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &dwWidth, 0) && 2 < dwWidth) {
				nCaretWidth = t_min((int)dwWidth, GetHankakuDx());
			}
		}else {
			nCaretWidth = GetHankakuDx();

			const wchar_t*	pLine = NULL;
			LogicInt		nLineLen = LogicInt(0);
			const Layout*	pLayout = NULL;
			if (bShowCaret) {
				// ��ʊO�̂Ƃ���GetLineStr���Ă΂Ȃ�
				pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
			}

			if (pLine) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				int nIdxFrom = GetCaretLogicPos().GetX() - pLayout->GetLogicOffset();
				if (0
					|| nIdxFrom >= nLineLen
					|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
					|| pLine[nIdxFrom] == TAB
				) {
					nCaretWidth = GetHankakuDx();
				}else {
					LayoutInt nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
					if (0 < nKeta) {
						nCaretWidth = GetHankakuDx() * (Int)nKeta;
					}
				}
			}
		}
	// �J�[�\���̃^�C�v = dos
	}else if (csGeneral.GetCaretType() == 1) {
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretHeight = GetHankakuHeight() / 2;			// �L�����b�g�̍���
		}else {
			nCaretHeight = GetHankakuHeight();				// �L�����b�g�̍���
		}
		nCaretWidth = GetHankakuDx();

		const wchar_t*	pLine = NULL;
		LogicInt		nLineLen = LogicInt(0);
		const Layout*	pLayout = NULL;
		if (bShowCaret) {
			pLine= layoutMgr.GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
		}

		if (pLine) {
			// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
			int nIdxFrom = editView.LineColumnToIndex(pLayout, GetCaretLayoutPos().GetX2());
			if (0
				|| nIdxFrom >= nLineLen
				|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
				|| pLine[nIdxFrom] == TAB
			) {
				nCaretWidth = GetHankakuDx();
			}else {
				LayoutInt nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
				if (0 < nKeta) {
					nCaretWidth = GetHankakuDx() * (Int)nKeta;
				}
			}
		}
	}

	//	�L�����b�g�F�̎擾
	const ColorInfo* colorInfoArr = pTypes->colorInfoArr;
	int nCaretColor = (colorInfoArr[COLORIDX_CARET_IME].bDisp && editView.IsImeON())? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = colorInfoArr[nCaretColor].colorAttr.cTEXT;
	COLORREF crBack = colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	if (!ExistCaretFocus()) {
		// �L�����b�g���Ȃ������ꍇ
		// �L�����b�g�̍쐬
		CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);	// 2006.12.07 ryoji
		bCaretShowFlag = false; // 2002/07/22 novice
	}else {
		if (
			GetCaretSize() != Size(nCaretWidth, nCaretHeight)
			|| crCaret != crCaret
			|| editView.crBack != crBack
		) {
			// �L�����b�g�͂��邪�A�傫����F���ς�����ꍇ
			// ���݂̃L�����b�g���폜
			::DestroyCaret();

			// �L�����b�g�̍쐬
			CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);	// 2006.12.07 ryoji
			bCaretShowFlag = false; // 2002/07/22 novice
		}else {
			// �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ
			// �L�����b�g���B��
			HideCaret_(editView.GetHwnd()); // 2002/07/22 novice
		}
	}

	// �L�����b�g�T�C�Y
	SetCaretSize(nCaretWidth, nCaretHeight);

	// �L�����b�g�̈ʒu�𒲐�
	// 2007.08.26 kobake �L�����b�gX���W�̌v�Z��UNICODE�d�l�ɂ����B
	::SetCaretPos(ptDrawPos.x, ptDrawPos.y);
	if (bShowCaret) {
		// �L�����b�g�̕\��
		ShowCaret_(editView.GetHwnd()); // 2002/07/22 novice
	}

	crCaret = crCaret;	//	2006.12.07 ryoji
	editView.crBack2 = crBack;		//	2006.12.07 ryoji
	editView.SetIMECompFormPos();
}


/*! �L�����b�g�̍s���ʒu����уX�e�[�^�X�o�[�̏�ԕ\���̍X�V

	@note �X�e�[�^�X�o�[�̏�Ԃ̕��ѕ��̕ύX�̓��b�Z�[�W����M����
		CEditWnd::DispatchEvent()��WM_NOTIFY�ɂ��e�������邱�Ƃɒ���
	
	@note �X�e�[�^�X�o�[�̏o�͓��e�̕ύX��CEditWnd::OnSize()��
		�J�������v�Z�ɉe�������邱�Ƃɒ���
*/
// 2007.10.17 kobake �d������R�[�h�𐮗�
void Caret::ShowCaretPosInfo()
{
	// �K�v�ȃC���^�[�t�F�[�X
	auto& layoutMgr = editDoc.layoutMgr;
	const TypeConfig* pTypes = &editDoc.docType.GetDocumentAttribute();

	if (!editView.GetDrawSwitch()) {
		return;
	}

	// �X�e�[�^�X�o�[�n���h�����擾
	HWND hwndStatusBar = editDoc.pEditWnd->statusBar.GetStatusHwnd();

	// �J�[�\���ʒu�̕�������擾
	const Layout*	pLayout;
	LogicInt		nLineLen;
	const wchar_t*	pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

	// -- -- -- -- �����R�[�h��� -> pszCodeName -- -- -- -- //
	const TCHAR* pszCodeName;
	NativeT memCodeName;
	if (hwndStatusBar) {
		TCHAR szCodeName[100];
		CodePage::GetNameNormal(szCodeName, editDoc.GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (editDoc.GetDocumentBomExist()) {
			memCodeName.AppendString(LS(STR_CARET_WITHBOM));
		}
	}else {
		TCHAR szCodeName[100];
		CodePage::GetNameShort(szCodeName, editDoc.GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (editDoc.GetDocumentBomExist()) {
			memCodeName.AppendStringLiteral(_T("#"));		// BOM�t(���j���[�o�[�Ȃ̂ŏ�����)	// 2013/4/17 Uchi
		}
	}
	pszCodeName = memCodeName.GetStringPtr();


	// -- -- -- -- ���s���[�h -> szEolMode -- -- -- -- //
	//	May 12, 2000 genta
	//	���s�R�[�h�̕\����ǉ�
	Eol cNlType = editDoc.docEditor.GetNewLineCode();
	const TCHAR* szEolMode = cNlType.GetName();


	// -- -- -- -- �L�����b�g�ʒu -> ptCaret -- -- -- -- //
	//
	Point ptCaret;
	// �s�ԍ������W�b�N�P�ʂŕ\��
	if (pTypes->bLineNumIsCRLF) {
		ptCaret.x = 0;
		ptCaret.y = (Int)GetCaretLogicPos().y;
		if (pLayout) {
			// 2014.01.10 ���s�̂Ȃ��傫���s������ƒx���̂ŃL���b�V������
			LayoutInt offset;
			if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache == GetCaretLayoutPos().GetY2()
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				offset = nOffsetCache;
			}else if (
				nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache < GetCaretLayoutPos().GetY2()
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// ���ړ�
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffsetCache, nOffsetCache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().GetY2();
			}else if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNo50Cache <= GetCaretLayoutPos().GetY2()
				&& GetCaretLayoutPos().GetY2() <= nLineNo50Cache + 50
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// ��ړ�
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffset50Cache, nOffset50Cache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().GetY2();
			}else {
			// 2013.05.11 �܂�Ԃ��Ȃ��Ƃ��Čv�Z����
				const Layout* pLayout50 = pLayout;
				LayoutInt nLineNum = GetCaretLayoutPos().GetY2();
				for (;;) {
					if (pLayout50->GetLogicOffset() == 0) {
						break;
					}
					if (nLineNum + 50 == GetCaretLayoutPos().GetY2()) {
						break;
					}
					pLayout50 = pLayout50->GetPrevLayout();
					--nLineNum;
				}
				nOffset50Cache = pLayout50->CalcLayoutOffset(layoutMgr);
				nLogicOffset50Cache = pLayout50->GetLogicOffset();
				nLineNo50Cache = nLineNum;
				
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffset50Cache, nOffset50Cache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineLogicNoCache = pLayout->GetLogicLineNo();
				nLineNoCache = GetCaretLayoutPos().GetY2();
				nLineLogicModCache = ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() );
			}
			Layout layout(
				pLayout->GetDocLineRef(),
				pLayout->GetLogicPos(),
				pLayout->GetLengthWithEOL(),
				pLayout->GetColorTypePrev(),
				offset,
				NULL
			);
			ptCaret.x = (Int)editView.LineIndexToColumn(&layout, GetCaretLogicPos().x - pLayout->GetLogicPos().x);
		}
	// �s�ԍ������C�A�E�g�P�ʂŕ\��
	}else {
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLayoutPos().GetY();
	}
	// �\���l��1����n�܂�悤�ɕ␳
	ptCaret.x++;
	ptCaret.y++;

	// -- -- -- -- �L�����b�g�ʒu�̕������ -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32] = _T("");
	if (pLine) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		LogicInt nIdx = GetCaretLogicPos().GetX2() - pLayout->GetLogicOffset();
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				//auto_sprintf(szCaretChar, _T("%04x"),);
				// �C�ӂ̕����R�[�h����Unicode�֕ϊ�����		2008/6/9 Uchi
				CodeBase* pCode = CodeFactory::CreateCodeBase(editDoc.GetDocumentEncoding(), false);
				CommonSetting_StatusBar* psStatusbar = &GetDllShareData().common.statusBar;
				CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar, psStatusbar);
				delete pCode;
				if (ret != CodeConvertResult::Complete) {
					// ���܂��R�[�h�����Ȃ�����(Unicode�ŕ\��)
					pCode = CodeFactory::CreateCodeBase(CODE_UNICODE, false);
					/* CodeConvertResult ret = */ pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar, psStatusbar);
					delete pCode;
				}
			}else {
				_tcscpy_s(szCaretChar, _countof(szCaretChar), pLayout->GetLayoutEol().GetName());
			}
		}
	}


	// -- -- -- --  �X�e�[�^�X���������o�� -- -- -- -- //
	//
	// �E�B���h�E�E��ɏ����o��
	if (!hwndStatusBar) {
		TCHAR	szText[64];
		TCHAR	szFormat[64];
		TCHAR	szLeft[64];
		TCHAR	szRight[64];
		int		nLen;
		{	// ���b�Z�[�W�̍���������i�u�s:��v���������\���j
			nLen = _tcslen(pszCodeName) + _tcslen(szEolMode) + _tcslen(szCaretChar);
			// ����� %s(%s)%6s%s%s ���ɂȂ�B%6ts�\�L�͎g���Ȃ��̂Œ���
			auto_sprintf_s(
				szFormat,
				_T("%%s(%%s)%%%ds%%s%%s"),	// �u�L�����b�g�ʒu�̕������v���E�l�Ŕz�u�i����Ȃ��Ƃ��͍��l�ɂȂ��ĉE�ɐL�т�j
				(nLen < 15)? 15 - nLen: 1
			);
			auto_sprintf_s(
				szLeft,
				szFormat,
				pszCodeName,
				szEolMode,
				szCaretChar[0]? _T("["): _T(" "),	// ������񖳂��Ȃ犇�ʂ��ȗ��iEOF��t���[�J�[�\���ʒu�j
				szCaretChar,
				szCaretChar[0]? _T("]"): _T(" ")	// ������񖳂��Ȃ犇�ʂ��ȗ��iEOF��t���[�J�[�\���ʒu�j
			);
		}
		szRight[0] = _T('\0');
		nLen = MENUBAR_MESSAGE_MAX_LEN - _tcslen(szLeft);	// �E���Ɏc���Ă��镶����
		if (nLen > 0) {	// ���b�Z�[�W�̉E��������i�u�s:��v�\���j
			TCHAR szRowCol[32];
			auto_sprintf_s(
				szRowCol,
				_T("%d:%-4d"),	// �u��v�͍ŏ������w�肵�č��񂹁i����Ȃ��Ƃ��͉E�ɐL�т�j
				ptCaret.y,
				ptCaret.x
			);
			auto_sprintf_s(
				szFormat,
				_T("%%%ds"),	// �u�s:��v���E�l�Ŕz�u�i����Ȃ��Ƃ��͍��l�ɂȂ��ĉE�ɐL�т�j
				nLen
			);
			auto_sprintf_s(
				szRight,
				szFormat,
				szRowCol
			);
		}
		auto_sprintf_s(
			szText,
			_T("%s%s"),
			szLeft,
			szRight
		);
		editDoc.pEditWnd->PrintMenubarMessage(szText);
	// �X�e�[�^�X�o�[�ɏ�Ԃ������o��
	}else {
		TCHAR	szText_1[64];
		auto_sprintf_s(szText_1, LS(STR_STATUS_ROW_COL), ptCaret.y, ptCaret.x);	// Oct. 30, 2000 JEPRO �疜�s���v���

		TCHAR	szText_6[16];
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_INS));	// "�}��"
		}else {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_OVR));	// "�㏑"
		}
		if (bClearStatus) {
			::StatusBar_SetText(hwndStatusBar, 0 | SBT_NOBORDERS, _T(""));
		}
		::StatusBar_SetText(hwndStatusBar, 1 | 0,             szText_1);
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ��D���̔ԍ���1�����炷
		//	From Here
		::StatusBar_SetText(hwndStatusBar, 2 | 0,             szEolMode);
		//	To Here
		::StatusBar_SetText(hwndStatusBar, 3 | 0,             szCaretChar);
		::StatusBar_SetText(hwndStatusBar, 4 | 0,             pszCodeName);
		::StatusBar_SetText(hwndStatusBar, 5 | SBT_OWNERDRAW, _T(""));
		::StatusBar_SetText(hwndStatusBar, 6 | 0,             szText_6);
	}

}

void Caret::ClearCaretPosInfoCache()
{
	nOffsetCache = LayoutInt(-1);
	nLineNoCache = LayoutInt(-1);
	nLogicOffsetCache = LogicInt(-1);
	nLineLogicNoCache = LogicInt(-1);
	nLineNo50Cache = LayoutInt(-1);
	nOffset50Cache = LayoutInt(-1);
	nLogicOffset50Cache = LogicInt(-1);
	nLineLogicModCache = -1;
}

/* �J�[�\���㉺�ړ����� */
LayoutInt Caret::Cursor_UPDOWN(LayoutInt nMoveLines, bool bSelect)
{
	// �K�v�ȃC���^�[�t�F�[�X
	auto& layoutMgr = editDoc.layoutMgr;
	auto& csGeneral = GetDllShareData().common.general;

	const LayoutPoint ptCaret = GetCaretLayoutPos();

	bool	bVertLineDoNotOFF = true;	// �J�[�\���ʒu�c�����������Ȃ�
	if (bSelect) {
		bVertLineDoNotOFF = false;		// �I����ԂȂ�J�[�\���ʒu�c���������s��
	}

	auto& selInfo = editView.GetSelectionInfo();

	// ���݂̃L�����b�gY���W + nMoveLines�����������C�A�E�g�s�͈͓̔��Ɏ��܂�悤�� nMoveLines�𒲐�����B
	if (nMoveLines > 0) { // ���ړ��B
		const bool existsEOFOnlyLine = layoutMgr.GetBottomLayout() && layoutMgr.GetBottomLayout()->GetLayoutEol() != EolType::None
			|| layoutMgr.GetLineCount() == 0;
		const LayoutInt maxLayoutLine = layoutMgr.GetLineCount() + (existsEOFOnlyLine ? 1 : 0) - 1;
		// �ړ��悪 EOF�݂̂̍s���܂߂����C�A�E�g�s�������ɂȂ�悤�Ɉړ��ʂ��K������B
		nMoveLines = t_min(nMoveLines,  maxLayoutLine - ptCaret.y);
		if (1
			&& ptCaret.y + nMoveLines == maxLayoutLine
			&& existsEOFOnlyLine // �ړ��悪 EOF�݂̂̍s
			&& selInfo.IsBoxSelecting()
			&& ptCaret.x != 0 // ����`�I�𒆂Ȃ�A
		) {
			// EOF�݂̂̍s�ɂ͈ړ����Ȃ��B���ړ��ŃL�����b�g�� X���W�𓮂��������Ȃ��̂ŁB
			nMoveLines = t_max(LayoutInt(0), nMoveLines - 1); // ���������ړ����Ȃ��悤�� 0�ȏ�����B
		}
	}else { // ��ړ��B
		// �ړ��悪 0�s�ڂ�菬�����Ȃ�Ȃ��悤�Ɉړ��ʂ��K���B
		nMoveLines = t_max(nMoveLines, - GetCaretLayoutPos().GetY());
	}

	if (bSelect && ! selInfo.IsTextSelected()) {
		// ���݂̃J�[�\���ʒu����I�����J�n����
		selInfo.BeginSelectArea();
	}
	if (!bSelect) {
		if (selInfo.IsTextSelected()) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			selInfo.DisableSelectArea(true);
		}else if (selInfo.IsBoxSelecting()) {
			selInfo.SetBoxSelect(false);
		}
	}

	// (���ꂩ�狁�߂�)�L�����b�g�̈ړ���B
	LayoutPoint ptTo(LayoutInt(0), ptCaret.y + nMoveLines);

	// �ړ���̍s�̃f�[�^���擾
	const Layout* const pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
	const LogicInt nLineLen = pLayout ? pLayout->GetLengthWithEOL() : LogicInt(0);
	int i = 0; ///< ���H
	if (pLayout) {
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if (it.getColumn() + it.getColumnDelta() > nCaretPosX_Prev) {
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		ptTo.x += it.getColumn();
		if (it.end()) {
			i = it.getIndex();
		}
	}
	if (i >= nLineLen) {
		// �t���[�J�[�\�����[�h�Ƌ�`�I�𒆂́A�L�����b�g�̈ʒu�����s�� EOF�̑O�ɐ������Ȃ�
		if (csGeneral.bIsFreeCursorMode
			|| selInfo.IsBoxSelecting()
		) {
			ptTo.x = nCaretPosX_Prev;
		}
	}
	if (ptTo.x != GetCaretLayoutPos().GetX()) {
		bVertLineDoNotOFF = false;
	}
	GetAdjustCursorPos(&ptTo);
	if (bSelect) {
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		selInfo.ChangeSelectAreaByCurrentCursor(ptTo);
	}
	const LayoutInt nScrollLines = MoveCursor(	ptTo,
								editView.GetDrawSwitch() /* TRUE */,
								_CARETMARGINRATE,
								false,
								bVertLineDoNotOFF);
	return nScrollLines;
}


/*!	�L�����b�g�̍쐬

	@param nCaretColor [in]	�L�����b�g�̐F��� (0:�ʏ�, 1:IME ON)
	@param nWidth [in]		�L�����b�g��
	@param nHeight [in]		�L�����b�g��

	@date 2006.12.07 ryoji �V�K�쐬
*/
void Caret::CreateEditCaret(COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight)
{
	//
	// �L�����b�g�p�̃r�b�g�}�b�v���쐬����
	//
	// Note: �E�B���h�E�݊��̃����� DC ��� PatBlt ��p���ăL�����b�g�F�Ɣw�i�F�� XOR ����
	//       ���邱�ƂŁC�ړI�̃r�b�g�}�b�v�𓾂�D
	//       �� 256 �F���ł� RGB �l��P���ɒ��ډ��Z���Ă��L�����b�g�F���o�����߂̐�����
	//          �r�b�g�}�b�v�F�͓����Ȃ��D
	//       �Q�l: [HOWTO] �L�����b�g�̐F�𐧌䂷����@
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// �L�����b�g�p�̃r�b�g�}�b�v

	HDC hdc = editView.GetDC();

	hbmpCaret = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);
	HDC hdcMem = ::CreateCompatibleDC(hdc);
	HBITMAP hbmpOld = (HBITMAP)::SelectObject(hdcMem, hbmpCaret);
	HBRUSH hbrCaret = ::CreateSolidBrush(crCaret);
	HBRUSH hbrBack = ::CreateSolidBrush(crBack);
	HBRUSH hbrOld = (HBRUSH)::SelectObject(hdcMem, hbrCaret);
	::PatBlt(hdcMem, 0, 0, nWidth, nHeight, PATCOPY);
	::SelectObject(hdcMem, hbrBack);
	::PatBlt(hdcMem, 0, 0, nWidth, nHeight, PATINVERT);
	::SelectObject(hdcMem, hbrOld);
	::SelectObject(hdcMem, hbmpOld);
	::DeleteObject(hbrCaret);
	::DeleteObject(hbrBack);
	::DeleteDC(hdcMem);

	editView.ReleaseDC(hdc);

	// �ȑO�̃r�b�g�}�b�v��j������
	if (hbmpCaret) {
		::DeleteObject(hbmpCaret);
	}
	hbmpCaret = hbmpCaret;

	// �L�����b�g���쐬����
	editView.CreateCaret(hbmpCaret, nWidth, nHeight);
	return;
}


// 2002/07/22 novice
/*!
	�L�����b�g�̕\��
*/
void Caret::ShowCaret_(HWND hwnd)
{
	if (!bCaretShowFlag) {
		::ShowCaret(hwnd);
		bCaretShowFlag = true;
	}
}


/*!
	�L�����b�g�̔�\��
*/
void Caret::HideCaret_(HWND hwnd)
{
	if (bCaretShowFlag) {
		::HideCaret(hwnd);
		bCaretShowFlag = false;
	}
}

// �����̏�Ԃ𑼂�Caret�ɃR�s�[
void Caret::CopyCaretStatus(Caret* pCaret) const
{
	pCaret->SetCaretLayoutPos(GetCaretLayoutPos());
	pCaret->SetCaretLogicPos(GetCaretLogicPos());
	pCaret->nCaretPosX_Prev = nCaretPosX_Prev;	// �r���[���[����̃J�[�\�����ʒu�i�O�I���W��

	//�� �L�����b�g�̃T�C�Y�̓R�s�[���Ȃ��B2002/05/12 YAZAKI
}


POINT Caret::CalcCaretDrawPos(const LayoutPoint& ptCaretPos) const
{
	auto& textArea = editView.GetTextArea();
	int nPosX = textArea.GetAreaLeft() + (Int)(ptCaretPos.x - textArea.GetViewLeftCol()) * GetHankakuDx();
	LayoutYInt nY = ptCaretPos.y - textArea.GetViewTopLine();
	int nPosY;
	if (nY < 0) {
		nPosY = -1;
	}else if (textArea.nViewRowNum < nY) {
		nPosY = textArea.GetAreaBottom() + 1;
	}else {
		nPosY = textArea.GetAreaTop()
			+ (Int)(nY) * editView.GetTextMetrics().GetHankakuDy()
			+ editView.GetTextMetrics().GetHankakuHeight() - GetCaretSize().cy; // ����
	}

	return Point(nPosX, nPosY);
}


/*!
	�s���w��ɂ��J�[�\���ړ��i���W�����t���j

	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �}�E�X���ɂ��ړ��ŕs�K�؂Ȉʒu�ɍs���Ȃ��悤���W�������ăJ�[�\���ړ�����

	@date 2007.08.23 ryoji �֐����iMoveCursorToPoint()���珈���𔲂��o���j
	@date 2007.09.26 ryoji ���p�����ł������ō��E�ɃJ�[�\����U�蕪����
	@date 2007.10.23 kobake ���������̌����C�� ([in/out]��[in])
	@date 2009.02.17 ryoji ���C�A�E�g�s���Ȍ�̃J�����ʒu�w��Ȃ疖�������̑O�ł͂Ȃ����������̌�Ɉړ�����
*/
LayoutInt Caret::MoveCursorProperly(
	LayoutPoint		ptNewXY,			// [in] �J�[�\���̃��C�A�E�g���WX
	bool			bScroll,			// [in] true: ��ʈʒu�����L��/ false: ��ʈʒu�����L�薳��
	bool			test,				// [in] true: �J�[�\���ړ��͂��Ȃ�
	LayoutPoint*	ptNewXYNew,			// [out] �V�������C�A�E�g���W
	int				nCaretMarginRate,	// [in] �c�X�N���[���J�n�ʒu�����߂�l
	int				dx					// [in] ptNewXY.x�ƃ}�E�X�J�[�\���ʒu�Ƃ̌덷(�J�����������̃h�b�g��)
	)
{
	LogicInt		nLineLen;
	const Layout*	pLayout;

	if (0 > ptNewXY.y) {
		ptNewXY.y = LayoutInt(0);
	}
	
	// 2011.12.26 EOF�ȉ��̍s�������ꍇ�ŋ�`�̂Ƃ��́A�ŏI���C�A�E�g�s�ֈړ�����
	auto& layoutMgr = editDoc.layoutMgr;
	auto& selectionInfo = editView.GetSelectionInfo();
	if (1
		&& ptNewXY.y >= layoutMgr.GetLineCount()
		&& (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())
	) {
		const Layout* layoutEnd = layoutMgr.GetBottomLayout();
		bool bEofOnly = (layoutEnd && layoutEnd->GetLayoutEol() != EolType::None) || !layoutEnd;
	 	// 2012.01.09 �҂�����[EOF]�ʒu�ɂ���ꍇ�͈ʒu���ێ�(1��̍s�ɂ��Ȃ�)
	 	if (1
	 		&& bEofOnly
	 		&& ptNewXY.y == layoutMgr.GetLineCount()
	 		&& ptNewXY.x == 0
	 	) {
	 	}else {
			ptNewXY.y = t_max(LayoutInt(0), layoutMgr.GetLineCount() - 1);
		}
	}
	// �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩
	if (ptNewXY.y >= layoutMgr.GetLineCount()) {
		// 2004.04.03 Moca EOF�����̍��W�����́AMoveCursor���ł���Ă��炤�̂ŁA�폜
	// �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩
	}else if (ptNewXY.y < 0) {
		ptNewXY.Set(LayoutInt(0), LayoutInt(0));
	}else {
		// �ړ���̍s�̃f�[�^���擾
		layoutMgr.GetLineStr(ptNewXY.GetY2(), &nLineLen, &pLayout);

		int nColWidth = editView.GetTextMetrics().GetHankakuDx();
		LayoutInt nPosX = LayoutInt(0);
		int i = 0;
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > LogicInt(pLayout->GetLengthWithoutEOL())) {
				i = nLineLen;
				break;
			}
			if (it.getColumn() + it.getColumnDelta() > ptNewXY.GetX2()) {
				if (1
					&& ptNewXY.GetX2() >= (pLayout ? pLayout->GetIndent() : LayoutInt(0))
					&& ((ptNewXY.GetX2() - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth
				) {
				//if (ptNewXY.GetX2() >= (pLayout ? pLayout->GetIndent() : LayoutInt(0)) && (it.getColumnDelta() > LayoutInt(1)) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.GetX2()) <= it.getColumnDelta() / 2)) {
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if (it.end()) {
			i = it.getIndex();
			//nPosX -= it.getColumnDelta();	// 2009.02.17 ryoji �R�����g�A�E�g�i���������̌�Ɉړ�����j
		}

		if (i >= nLineLen) {
			// 2011.12.26 �t���[�J�[�\��/��`�Ńf�[�^�t��EOF�̉E���ֈړ��ł���悤��
			// �t���[�J�[�\�����[�h��
			if (0
				|| GetDllShareData().common.general.bIsFreeCursorMode
				|| (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())	/* �}�E�X�͈͑I�� && ��`�͈͑I�� */
				|| (editView.bDragMode && editView.bDragBoxData) /* OLE DropTarget && ��`�f�[�^ */
			) {
				// �܂�Ԃ����ƃ��C�A�E�g�s�����i�Ԃ牺�����܂ށj�̂ǂ��炩�傫���ق��܂ŃJ�[�\���ړ��\
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				LayoutInt nMaxX = t_max(nPosX, layoutMgr.GetMaxLineKetas());
				nPosX = ptNewXY.GetX2();
				if (nPosX < LayoutInt(0)) {
					nPosX = LayoutInt(0);
				}else if (nPosX > nMaxX) {
					nPosX = nMaxX;
				}
			}
		}
		ptNewXY.SetX(nPosX);
	}
	
	if (ptNewXYNew) {
		*ptNewXYNew = ptNewXY;
		GetAdjustCursorPos(ptNewXYNew);
	}
	if (test) {
		return LayoutInt(0);
	}
	return MoveCursor(ptNewXY, bScroll, nCaretMarginRate);
}

