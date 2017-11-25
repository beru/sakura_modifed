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
	nCaretPosX_Prev = 0;	// �r���[���[����̃J�[�\�������O�̈ʒu(�O�I���W��)

	crCaret = -1;				// �L�����b�g�̐F
	hbmpCaret = NULL;			// �L�����b�g�p�r�b�g�}�b�v
	bClearStatus = true;
	ClearCaretPosInfoCache();
}

Caret::~Caret()
{
	// �L�����b�g�p�r�b�g�}�b�v
	if (hbmpCaret) {
		DeleteObject(hbmpCaret);
	}
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
*/
int Caret::MoveCursor(
	Point	ptWk_CaretPos,		// [in] �ړ��惌�C�A�E�g�ʒu
	bool	bScroll,			// [in] true: ��ʈʒu�����L��  false: ��ʈʒu��������
	int		nCaretMarginRate,	// [in] �c�X�N���[���J�n�ʒu�����߂�l
	bool	bUnderLineDoNotOFF,	// [in] �A���_�[���C�����������Ȃ�
	bool	bVertLineDoNotOFF	// [in] �J�[�\���ʒu�c�����������Ȃ�
	)
{
	// �X�N���[������
	int nScrollRowNum = 0;
	int nScrollColNum = 0;
	int nCaretMarginY;
	int	nScrollMarginRight;
	int	nScrollMarginLeft;

	auto& textArea = editView.GetTextArea();
	if (0 >= textArea.nViewColNum) {
		return 0;
	}

	if (editView.GetSelectionInfo().IsMouseSelecting()) {	// �͈͑I��
		nCaretMarginY = 0;
	}else {
		nCaretMarginY = textArea.nViewRowNum / nCaretMarginRate;
		if (1 > nCaretMarginY) {
			nCaretMarginY = 1;
		}
	}
	GetAdjustCursorPos(&ptWk_CaretPos);
	// �J�[�\���ʒu�B���W�b�N�P�ʁB
	ptCaretPos_Logic = editDoc.layoutMgr.LayoutToLogic(ptWk_CaretPos);
	// �L�����b�g�ړ�
	SetCaretLayoutPos(ptWk_CaretPos);

	// �J�[�\���s�A���_�[���C����OFF
	bool bDrawPaint = ptWk_CaretPos.y != editView.nOldUnderLineYBg;
	underLine.SetUnderLineDoNotOFF(bUnderLineDoNotOFF);
	underLine.SetVertLineDoNotOFF(bVertLineDoNotOFF);
	underLine.CaretUnderLineOFF(bScroll, bDrawPaint);
	underLine.SetUnderLineDoNotOFF(false);
	underLine.SetVertLineDoNotOFF(false);
	
	// �����X�N���[���ʁi�������j�̎Z�o
	nScrollColNum = 0;
	nScrollMarginRight = SCROLLMARGIN_RIGHT;
	nScrollMarginLeft = SCROLLMARGIN_LEFT;

	// ���������ꍇ�̃}�[�W���̒���
	{
		// �J�[�\�����^�񒆂ɂ���Ƃ��ɍ��E�ɂԂ�Ȃ��悤��
		int nNoMove = SCROLLMARGIN_NOMOVE;
		int a = (textArea.nViewColNum - nNoMove) / 2;
		int nMin = (2 <= a ? a : 0); // 1���ƑS�p�ړ��Ɏx�Ⴊ����̂�2�ȏ�
		nScrollMarginRight = t_min(nScrollMarginRight, nMin);
		nScrollMarginLeft  = t_min(nScrollMarginLeft,  nMin);
	}
	
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if ((int)editDoc.layoutMgr.GetMaxLineKetas() > textArea.nViewColNum
		&& ptWk_CaretPos.GetX() > textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight
	) {
		nScrollColNum = (textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight) - ptWk_CaretPos.x;
	}else if (1
		&& 0 < textArea.GetViewLeftCol()
		&& ptWk_CaretPos.GetX() < textArea.GetViewLeftCol() + nScrollMarginLeft
	) {
		nScrollColNum = textArea.GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.x;
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
			int ii = (int)editDoc.layoutMgr.GetLineCount();
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
			nScrollRowNum = -(ptWk_CaretPos.y - (int)textArea.GetViewTopLine()) + nCaretMarginY;
		}
	// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= textArea.nViewRowNum - nCaretMarginY - 2) {
		int ii = (int)editDoc.layoutMgr.GetLineCount();
		if (1
			&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
			&& ii - textArea.GetViewTopLine() < textArea.nViewRowNum
		) {
		}else {
			nScrollRowNum = -(ptWk_CaretPos.y - (int)textArea.GetViewTopLine()) + ((int)textArea.nViewRowNum - nCaretMarginY - 2);
		}
	}
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
				rcScroll.bottom = textArea.GetAreaBottom() - nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateTopRect(&rcClip, nScrollRowNum);
			}else if (nScrollRowNum < 0) {
				rcScroll.top = textArea.GetAreaTop() - nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateBottomRect(&rcClip, -nScrollRowNum);
			}

			if (nScrollColNum > 0) {
				rcScroll.left = textArea.GetAreaLeft();
				rcScroll.right = textArea.GetAreaRight() - nScrollColNum * GetHankakuDx();
				textArea.GenerateLeftRect(&rcClip2, nScrollColNum);
			}else if (nScrollColNum < 0) {
				rcScroll.left = textArea.GetAreaLeft() - nScrollColNum * GetHankakuDx();
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
		editView.AdjustScrollBars();
	}

	// ���X�N���[��������������A���[���[�S�̂��ĕ`��
	if (nScrollColNum != 0) {
		// ����DispRuler�Ăяo�����ɍĕ`��B�ibDraw=false�̃P�[�X���l�������B�j
		editView.GetRuler().SetRedrawFlag();
	}

	// �J�[�\���s�A���_�[���C����ON
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

	editView.DrawBracketPair(false);
	editView.SetBracketPairPos(true);
	editView.DrawBracketPair(true);

	return nScrollRowNum;
}


int Caret::MoveCursorFastMode(
	const Point& ptWk_CaretPosLogic	// [in] �ړ��惍�W�b�N�ʒu
	)
{
	// fastMode
	SetCaretLogicPos(ptWk_CaretPosLogic);
	return 0;
}

/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ�(���^��)
*/
int Caret::MoveCursorToClientPoint(
	const POINT& ptClientPos,
	bool test,
	Point* pCaretPosNew
	)
{
	Point	ptLayoutPos;
	editView.GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - editView.GetTextArea().GetAreaLeft()) % (editView.GetTextMetrics().GetHankakuDx());
	int nScrollRowNum = MoveCursorProperly(ptLayoutPos, true, test, pCaretPosNew, 1000, dx);
	if (!test) {
		nCaretPosX_Prev = GetCaretLayoutPos().x;
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
*/
bool Caret::GetAdjustCursorPos(
	Point* pptPosXY
	)
{
	// EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	size_t nLayoutLineCount = editDoc.layoutMgr.GetLineCount();

	Point ptPosXY2 = *pptPosXY;
	bool ret = false;
	if (ptPosXY2.y >= (int)nLayoutLineCount) {
		if (0 < nLayoutLineCount) {
			ptPosXY2.y = (int)nLayoutLineCount - 1;
			const Layout* pLayout = editDoc.layoutMgr.SearchLineByLayoutY(ptPosXY2.y);
			if (pLayout->GetLayoutEol() == EolType::None) {
				ptPosXY2.x = (int)editView.LineIndexToColumn(pLayout, pLayout->GetLengthWithEOL());
			}else {
				// EOF�����̍s
				ptPosXY2.y++;
				ptPosXY2.x = 0;
			}
		}else {
			// ��̃t�@�C��
			ptPosXY2.Set(0, 0);
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
			size_t			nLineLen = 0;
			const Layout*	pLayout = nullptr;
			if (bShowCaret) {
				// ��ʊO�̂Ƃ���GetLineStr���Ă΂Ȃ�
				pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);
			}

			if (pLine) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				int nIdxFrom = GetCaretLogicPos().GetX() - pLayout->GetLogicOffset();
				if (0
					|| nIdxFrom >= (int)nLineLen
					|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
					|| pLine[nIdxFrom] == TAB
				) {
					nCaretWidth = GetHankakuDx();
				}else {
					size_t nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
					if (0 < nKeta) {
						nCaretWidth = GetHankakuDx() * (int)nKeta;
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
		size_t			nLineLen = 0;
		const Layout*	pLayout = nullptr;
		if (bShowCaret) {
			pLine= layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);
		}

		if (pLine) {
			// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
			size_t nIdxFrom = editView.LineColumnToIndex(pLayout, GetCaretLayoutPos().x);
			if (0
				|| nIdxFrom >= nLineLen
				|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
				|| pLine[nIdxFrom] == TAB
			) {
				nCaretWidth = GetHankakuDx();
			}else {
				size_t nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
				if (0 < nKeta) {
					nCaretWidth = GetHankakuDx() * (int)nKeta;
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
		CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);
		bCaretShowFlag = false;
	}else {
		if (
			GetCaretSize() != Size(nCaretWidth, nCaretHeight)
			|| this->crCaret != crCaret
			|| editView.crBack != crBack
		) {
			// �L�����b�g�͂��邪�A�傫����F���ς�����ꍇ
			// ���݂̃L�����b�g���폜
			::DestroyCaret();

			// �L�����b�g�̍쐬
			CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);
			bCaretShowFlag = false;
		}else {
			// �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ
			// �L�����b�g���B��
			HideCaret_(editView.GetHwnd());
		}
	}

	// �L�����b�g�T�C�Y
	SetCaretSize(nCaretWidth, nCaretHeight);

	// �L�����b�g�̈ʒu�𒲐�
	::SetCaretPos(ptDrawPos.x, ptDrawPos.y);
	if (bShowCaret) {
		// �L�����b�g�̕\��
		ShowCaret_(editView.GetHwnd());
	}

	this->crCaret = crCaret;
	editView.crBack2 = crBack;
	editView.SetIMECompFormPos();
}


/*! �L�����b�g�̍s���ʒu����уX�e�[�^�X�o�[�̏�ԕ\���̍X�V

	@note �X�e�[�^�X�o�[�̏�Ԃ̕��ѕ��̕ύX�̓��b�Z�[�W����M����
		CEditWnd::DispatchEvent()��WM_NOTIFY�ɂ��e�������邱�Ƃɒ���
	
	@note �X�e�[�^�X�o�[�̏o�͓��e�̕ύX��CEditWnd::OnSize()��
		�J�������v�Z�ɉe�������邱�Ƃɒ���
*/
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
	const Layout* pLayout;
	size_t nLineLen;
	const wchar_t*	pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);

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
		ptCaret.y = GetCaretLogicPos().y;
		if (pLayout) {
			// 2014.01.10 ���s�̂Ȃ��傫���s������ƒx���̂ŃL���b�V������
			int offset;
			if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache == GetCaretLayoutPos().y
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				offset = nOffsetCache;
			}else if (
				nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache < GetCaretLayoutPos().y
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// ���ړ�
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffsetCache, nOffsetCache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().y;
			}else if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNo50Cache <= GetCaretLayoutPos().y
				&& GetCaretLayoutPos().y <= nLineNo50Cache + 50
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// ��ړ�
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffset50Cache, nOffset50Cache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().y;
			}else {
			// 2013.05.11 �܂�Ԃ��Ȃ��Ƃ��Čv�Z����
				const Layout* pLayout50 = pLayout;
				int nLineNum = GetCaretLayoutPos().y;
				for (;;) {
					if (pLayout50->GetLogicOffset() == 0) {
						break;
					}
					if (nLineNum + 50 == GetCaretLayoutPos().y) {
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
				nLineNoCache = GetCaretLayoutPos().y;
				nLineLogicModCache = ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() );
			}
			Layout layout(
				pLayout->GetDocLineRef(),
				pLayout->GetLogicPos(),
				pLayout->GetLengthWithEOL(),
				pLayout->GetColorTypePrev(),
				offset,
				nullptr
			);
			ptCaret.x = (int)editView.LineIndexToColumn(&layout, GetCaretLogicPos().x - pLayout->GetLogicPos().x);
		}
	// �s�ԍ������C�A�E�g�P�ʂŕ\��
	}else {
		ptCaret.x = GetCaretLayoutPos().GetX();
		ptCaret.y = GetCaretLayoutPos().GetY();
	}
	// �\���l��1����n�܂�悤�ɕ␳
	ptCaret.x++;
	ptCaret.y++;

	// -- -- -- -- �L�����b�g�ʒu�̕������ -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32] = _T("");
	if (pLine) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		ASSERT_GE(GetCaretLogicPos().x, pLayout->GetLogicOffset());
		int nIdx = GetCaretLogicPos().x - pLayout->GetLogicOffset();
		if (nIdx < (int)nLineLen) {
			if (nIdx < (int)nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
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
		size_t	nLen;
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
		auto_sprintf_s(szText_1, LS(STR_STATUS_ROW_COL), ptCaret.y, ptCaret.x);

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
		::StatusBar_SetText(hwndStatusBar, 2 | 0,             szEolMode);
		::StatusBar_SetText(hwndStatusBar, 3 | 0,             szCaretChar);
		::StatusBar_SetText(hwndStatusBar, 4 | 0,             pszCodeName);
		::StatusBar_SetText(hwndStatusBar, 5 | SBT_OWNERDRAW, _T(""));
		::StatusBar_SetText(hwndStatusBar, 6 | 0,             szText_6);
	}

}

void Caret::ClearCaretPosInfoCache()
{
	nOffsetCache = -1;
	nLineNoCache = -1;
	nLogicOffsetCache = -1;
	nLineLogicNoCache = -1;
	nLineNo50Cache = -1;
	nOffset50Cache = -1;
	nLogicOffset50Cache = -1;
	nLineLogicModCache = -1;
}

/* �J�[�\���㉺�ړ����� */
int Caret::Cursor_UPDOWN(int nMoveLines, bool bSelect)
{
	// �K�v�ȃC���^�[�t�F�[�X
	auto& layoutMgr = editDoc.layoutMgr;
	auto& csGeneral = GetDllShareData().common.general;

	const Point ptCaret = GetCaretLayoutPos();

	bool bVertLineDoNotOFF = true;	// �J�[�\���ʒu�c�����������Ȃ�
	if (bSelect) {
		bVertLineDoNotOFF = false;		// �I����ԂȂ�J�[�\���ʒu�c���������s��
	}

	auto& selInfo = editView.GetSelectionInfo();

	// ���݂̃L�����b�gY���W + nMoveLines�����������C�A�E�g�s�͈͓̔��Ɏ��܂�悤�� nMoveLines�𒲐�����B
	if (nMoveLines > 0) { // ���ړ��B
		const bool existsEOFOnlyLine = layoutMgr.GetBottomLayout() && layoutMgr.GetBottomLayout()->GetLayoutEol() != EolType::None
			|| layoutMgr.GetLineCount() == 0;
		const size_t maxLayoutLine = layoutMgr.GetLineCount() + (existsEOFOnlyLine ? 1 : 0) - 1;
		// �ړ��悪 EOF�݂̂̍s���܂߂����C�A�E�g�s�������ɂȂ�悤�Ɉړ��ʂ��K������B
		nMoveLines = t_min(nMoveLines, ((int)maxLayoutLine - (int)ptCaret.y));
		if (1
			&& ptCaret.y + nMoveLines == maxLayoutLine
			&& existsEOFOnlyLine // �ړ��悪 EOF�݂̂̍s
			&& selInfo.IsBoxSelecting()
			&& ptCaret.x != 0 // ����`�I�𒆂Ȃ�A
		) {
			// EOF�݂̂̍s�ɂ͈ړ����Ȃ��B���ړ��ŃL�����b�g�� X���W�𓮂��������Ȃ��̂ŁB
			nMoveLines = t_max(0, nMoveLines - 1); // ���������ړ����Ȃ��悤�� 0�ȏ�����B
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
	Point ptTo(0, ptCaret.y + nMoveLines);

	// �ړ���̍s�̃f�[�^���擾
	const Layout* const pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
	const size_t nLineLen = pLayout ? pLayout->GetLengthWithEOL() : 0;
	size_t i = 0; ///< ���H
	if (pLayout) {
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if ((int)(it.getColumn() + it.getColumnDelta()) > nCaretPosX_Prev) {
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		ptTo.x += (int)it.getColumn();
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
	const int nScrollLines = MoveCursor(	ptTo,
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
	if (this->hbmpCaret) {
		::DeleteObject(this->hbmpCaret);
	}
	this->hbmpCaret = hbmpCaret;

	// �L�����b�g���쐬����
	editView.CreateCaret(hbmpCaret, nWidth, nHeight);
	return;
}


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

	//�� �L�����b�g�̃T�C�Y�̓R�s�[���Ȃ��B
}


POINT Caret::CalcCaretDrawPos(const Point& ptCaretPos) const
{
	auto& textArea = editView.GetTextArea();
	int nPosX = textArea.GetAreaLeft() + (ptCaretPos.x - textArea.GetViewLeftCol()) * GetHankakuDx();
	int nY = ptCaretPos.y - textArea.GetViewTopLine();
	int nPosY;
	if (nY < 0) {
		nPosY = -1;
	}else if (textArea.nViewRowNum < nY) {
		nPosY = textArea.GetAreaBottom() + 1;
	}else {
		nPosY = textArea.GetAreaTop()
			+ nY * editView.GetTextMetrics().GetHankakuDy()
			+ editView.GetTextMetrics().GetHankakuHeight() - GetCaretSize().cy; // ����
	}

	return Point(nPosX, nPosY);
}


/*!
	�s���w��ɂ��J�[�\���ړ��i���W�����t���j

	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �}�E�X���ɂ��ړ��ŕs�K�؂Ȉʒu�ɍs���Ȃ��悤���W�������ăJ�[�\���ړ�����
*/
int Caret::MoveCursorProperly(
	Point	ptNewXY,			// [in] �J�[�\���̃��C�A�E�g���WX
	bool	bScroll,			// [in] true: ��ʈʒu�����L��/ false: ��ʈʒu�����L�薳��
	bool	test,				// [in] true: �J�[�\���ړ��͂��Ȃ�
	Point*	ptNewXYNew,			// [out] �V�������C�A�E�g���W
	int		nCaretMarginRate,	// [in] �c�X�N���[���J�n�ʒu�����߂�l
	int		dx					// [in] ptNewXY.x�ƃ}�E�X�J�[�\���ʒu�Ƃ̌덷(�J�����������̃h�b�g��)
	)
{
	size_t			nLineLen;
	const Layout*	pLayout;

	if (0 > ptNewXY.y) {
		ptNewXY.y = 0;
	}
	
	// EOF�ȉ��̍s�������ꍇ�ŋ�`�̂Ƃ��́A�ŏI���C�A�E�g�s�ֈړ�����
	auto& layoutMgr = editDoc.layoutMgr;
	auto& selectionInfo = editView.GetSelectionInfo();
	if (1
		&& ptNewXY.y >= (int)layoutMgr.GetLineCount()
		&& (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())
	) {
		const Layout* layoutEnd = layoutMgr.GetBottomLayout();
		bool bEofOnly = (layoutEnd && layoutEnd->GetLayoutEol() != EolType::None) || !layoutEnd;
	 	// �҂�����[EOF]�ʒu�ɂ���ꍇ�͈ʒu���ێ�(1��̍s�ɂ��Ȃ�)
	 	if (1
	 		&& bEofOnly
	 		&& ptNewXY.y == layoutMgr.GetLineCount()
	 		&& ptNewXY.x == 0
	 	) {
	 	}else {
			ptNewXY.y = t_max(0, (int)layoutMgr.GetLineCount() - 1);
		}
	}
	// �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩
	if (ptNewXY.y >= (int)layoutMgr.GetLineCount()) {
	// �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩
	}else if (ptNewXY.y < 0) {
		ptNewXY.Set(0, 0);
	}else {
		// �ړ���̍s�̃f�[�^���擾
		layoutMgr.GetLineStr(ptNewXY.y, &nLineLen, &pLayout);

		size_t nColWidth = editView.GetTextMetrics().GetHankakuDx();
		int nPosX = 0;
		size_t i = 0;
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if ((int)(it.getColumn() + it.getColumnDelta()) > ptNewXY.x) {
				if (1
					&& ptNewXY.x >= (int)(pLayout ? pLayout->GetIndent() : 0)
					&& ((ptNewXY.x - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth
				) {
				//if (ptNewXY.x >= (pLayout ? pLayout->GetIndent() : 0) && (it.getColumnDelta() > 1) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.x) <= it.getColumnDelta() / 2)) {
					nPosX += (int)it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += (int)it.getColumn();
		if (it.end()) {
			i = it.getIndex();
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
				int nMaxX = t_max(nPosX, (int)layoutMgr.GetMaxLineKetas());
				nPosX = ptNewXY.x;
				if (nPosX < 0) {
					nPosX = 0;
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
		return 0;
	}
	return MoveCursor(ptNewXY, bScroll, nCaretMarginRate);
}

