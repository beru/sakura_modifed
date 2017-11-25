#include "StdAfx.h"
#include "EditView_Paint.h"
#include "window/EditWnd.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@param flag [in] ���[�h(true:�o�^, false:����)
*/
void EditView::SetBracketPairPos(bool flag)
{
	if (bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!pTypeData->colorInfoArr[COLORIDX_BRACKET_PAIR].bDisp) {
		return;
	}

	// �Ί��ʂ̌���&�o�^
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	int	mode = 2;

	Point ptColLine;
	auto& caret = GetCaret();

	if (1
		&& flag
		&& !GetSelectionInfo().IsTextSelected()
		&& !GetSelectionInfo().bDrawSelectArea
		&& SearchBracket(caret.GetCaretLayoutPos(), &ptColLine, &mode)
	) {
		// �o�^�w��(flag=true)			&&
		// �e�L�X�g���I������Ă��Ȃ�	&&
		// �I��͈͂�`�悵�Ă��Ȃ�		&&
		// �Ή����銇�ʂ���������		�ꍇ
		if (1
			&& (ptColLine.x >= GetTextArea().GetViewLeftCol())
			&& (ptColLine.x <= GetTextArea().GetRightCol())
			&& (ptColLine.y >= GetTextArea().GetViewTopLine())
			&& (ptColLine.y <= GetTextArea().GetBottomLine())
		) {
			// �\���̈���̏ꍇ

			// ���C�A�E�g�ʒu���畨���ʒu�֕ϊ�(�����\���ʒu��o�^)
			ptBracketPairPos_PHY = pEditDoc->layoutMgr.LayoutToLogic(ptColLine);
			ptBracketCaretPos_PHY.y = caret.GetCaretLogicPos().y;
			if ((mode & 4) == 0) {
				// �J�[�\���̌�������ʒu
				ptBracketCaretPos_PHY.x = caret.GetCaretLogicPos().x;
			}else {
				// �J�[�\���̑O�������ʒu
				ptBracketCaretPos_PHY.x = caret.GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// ���ʂ̋����\���ʒu��񏉊���
	ptBracketPairPos_PHY.Set(-1, -1);
	ptBracketCaretPos_PHY.Set(-1, -1);

	return;
}

/*!
	�Ί��ʂ̋����\��
*/
void EditView::DrawBracketPair(bool bDraw)
{
	if (bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!pTypeData->colorInfoArr[COLORIDX_BRACKET_PAIR].bDisp) {
		return;
	}

	// ���ʂ̋����\���ʒu�����o�^�̏ꍇ�͏I��
	if (ptBracketPairPos_PHY.HasNegative() || ptBracketCaretPos_PHY.HasNegative()) {
		return;
	}

	// �`��w��(bDraw=true)				����
	// (�e�L�X�g���I������Ă���		����
	//   �I��͈͂�`�悵�Ă���			����
	//   �t�H�[�J�X�������Ă��Ȃ�		����
	//   �A�N�e�B�u�ȃy�C���ł͂Ȃ�)	�ꍇ�͏I��
	if (bDraw
		&& (0
			|| GetSelectionInfo().IsTextSelected()
			|| GetSelectionInfo().bDrawSelectArea
			|| !bDrawBracketPairFlag
			|| (editWnd.GetActivePane() != nMyIndex)
		)
	) {
		return;
	}
	
	Graphics gr;
	gr.Init(::GetDC(GetHwnd()));
	bool bCaretChange = false;
	gr.SetTextBackTransparent(true);
	auto& caret = GetCaret();

	for (int i=0; i<2; ++i) {
		// i=0:�Ί���,i=1:�J�[�\���ʒu�̊���
		Point ptColLine;
		if (i == 0) {
			ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptBracketPairPos_PHY);
		}else {
			ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptBracketCaretPos_PHY);
		}

		if (1
			&& (ptColLine.x >= GetTextArea().GetViewLeftCol())
			&& (ptColLine.x <= GetTextArea().GetRightCol())
			&& (ptColLine.y >= GetTextArea().GetViewTopLine())
			&& (ptColLine.y <= GetTextArea().GetBottomLine()) 
		) {	// �\���̈���̏ꍇ
			if (1
				&& !bDraw
				&& GetSelectionInfo().bDrawSelectArea
				&& (IsCurrentPositionSelected(ptColLine) == 0)
			) {	// �I��͈͕`��ς݂ŏ����Ώۂ̊��ʂ��I��͈͓��̏ꍇ
				continue;
			}
			const Layout* pLayout;
			size_t nLineLen;
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(ptColLine.y, &nLineLen, &pLayout);
			if (pLine) {
				EColorIndexType nColorIndex;
				size_t OutputX = LineColumnToIndex(pLayout, ptColLine.x);
				if (bDraw) {
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}else {
					if (IsBracket(pLine, OutputX, 1)) {
						DispPos pos(0, 0); // ���ӁF���̒l�̓_�~�[�BCheckChangeColor�ł̎Q�ƈʒu�͕s���m
						ColorStrategyInfo csInfo(*this);
						csInfo.pDispPos = &pos;

						// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
						Color3Setting cColor = GetColorIndex(pLayout, ptColLine.y, OutputX, csInfo);
						nColorIndex = cColor.eColorIndex2;
					}else {
						SetBracketPairPos(false);
						break;
					}
				}
				TypeSupport curetLineBg(*this, COLORIDX_CARETLINEBG);
				EColorIndexType nColorIndexBg = (curetLineBg.IsDisp() && ptColLine.y == caret.GetCaretLayoutPos().y
					? COLORIDX_CARETLINEBG
					: TypeSupport(*this, COLORIDX_EVENLINEBG).IsDisp() && ptColLine.y % 2 == 1
						? COLORIDX_EVENLINEBG
						: COLORIDX_TEXT);
				// 03/03/03 ai �J�[�\���̍��Ɋ��ʂ����芇�ʂ������\������Ă����Ԃ�Shift+���őI���J�n�����
				//             �I��͈͓��ɔ��]�\������Ȃ�������������̏C��
				int caretX = caret.GetCaretLayoutPos().x;
				bool bCaretHide = (!bCaretChange && (ptColLine.x == caretX || ptColLine.x + 1 == caretX) && caret.GetCaretShowFlag());
				if (bCaretHide) {
					bCaretChange = true;
					caret.HideCaret_(GetHwnd());	// �L�����b�g����u������̂�h�~
				}
				
				{
					int nWidth  = GetTextMetrics().GetHankakuDx();
					int nHeight = GetTextMetrics().GetHankakuDy();
					int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + ptColLine.x * nWidth;
					int nTop  = (ptColLine.y - GetTextArea().GetViewTopLine()) * nHeight + GetTextArea().GetAreaTop();
					size_t charsWidth = NativeW::GetKetaOfChar(pLine, nLineLen, OutputX);

					// �F�ݒ�
					TypeSupport textType(*this, COLORIDX_TEXT);
					textType.SetGraphicsState_WhileThisObj(gr);
					// 2013.05.24 �w�i�F���e�L�X�g�̔w�i�F�Ɠ����Ȃ�J�[�\���s�̔w�i�F��K�p
					TypeSupport colorIndexType(*this, nColorIndex);
					TypeSupport colorIndexBgType(*this, nColorIndexBg);
					TypeSupport* pColorBack = &colorIndexType;
					if (colorIndexType.GetBackColor() == textType.GetBackColor() && nColorIndexBg != COLORIDX_TEXT) {
						pColorBack = &colorIndexBgType;
					}

					SetCurrentColor(gr, nColorIndex, nColorIndex, nColorIndexBg);
					bool bTrans = false;
					// DEBUG_TRACE(_T("DrawBracket %d %d ") , ptColLine.y, ptColLine.x);
					if (1
						&& IsBkBitmap()
						&& textType.GetBackColor() == pColorBack->GetBackColor()
					) {
						bTrans = true;
						RECT rcChar;
						rcChar.left  = nLeft;
						rcChar.top = nTop;
						rcChar.right = nLeft + charsWidth * nWidth;
						rcChar.bottom = nTop + nHeight;
						HDC hdcBgImg = ::CreateCompatibleDC(gr);
						HBITMAP hBmpOld = (HBITMAP)::SelectObject(hdcBgImg, pEditDoc->hBackImg);
						DrawBackImage(gr, rcChar, hdcBgImg);
						::SelectObject(hdcBgImg, hBmpOld);
						::DeleteDC(hdcBgImg);
					}
					DispPos pos(nWidth, nHeight);
					pos.InitDrawPos(Point(nLeft, nTop));
					GetTextDrawer().DispText(gr, &pos,  &pLine[OutputX], 1, bTrans);
					GetTextDrawer().DispNoteLine(gr, nTop, nTop + nHeight, nLeft, nLeft + charsWidth * nWidth);
					// �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines(gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + charsWidth); // �����ʂ��S�p���ł���ꍇ���l��
					textType.RewindGraphicsState(gr);
				}

				if (1
					&& (editWnd.GetActivePane() == nMyIndex)
					&& (0
						|| (ptColLine.y == caret.GetCaretLayoutPos().GetY())
						|| (ptColLine.y - 1 == caret.GetCaretLayoutPos().GetY())
					) 
				) {	// 03/02/27 ai �s�̊Ԋu��"0"�̎��ɃA���_�[���C���������鎖������׏C��
					caret.underLine.CaretUnderLineON(true, false);
				}
			}
		}
	}
	if (bCaretChange) {
		caret.ShowCaret_(GetHwnd());	// �L�����b�g����u������̂�h�~
	}

	::ReleaseDC(GetHwnd(), gr);
}


//======================================================================
// �Ί��ʂ̑Ή��\
struct KAKKO_T {
	const wchar_t* sStr;
	const wchar_t* eStr;
};
static const KAKKO_T g_aKakkos[] = {
	// ���p
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"�", L"�", },
	// �S�p
	{ L"�y", L"�z", },
	{ L"�w", L"�x", },
	{ L"�u", L"�v", },
	{ L"��", L"��", },
	{ L"��", L"��", },
	{ L"�s", L"�t", },
	{ L"�i", L"�j", },
	{ L"�q", L"�r", },
	{ L"�o", L"�p", },
	{ L"�k", L"�l", },
	{ L"�m", L"�n", },
	{ L"�g", L"�h", },
	{ L"��", L"��", },
	// �I�[
	{ NULL, NULL, },
};


//	Jun. 16, 2000 genta
/*!
	@brief �Ί��ʂ̌���

	�J�[�\���ʒu�̊��ʂɑΉ����銇�ʂ�T���B�J�[�\���ʒu�����ʂłȂ��ꍇ��
	�J�[�\���̌��̕��������ʂ��ǂ����𒲂ׂ�B

	�J�[�\���̑O�ア����������ʂłȂ��ꍇ�͉������Ȃ��B

	���ʂ����p���S�p���A�y�юn�܂肩�I��肩�ɂ���Ă���ɑ���4�̊֐���
	������ڂ��B

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param mode [in/out] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
						 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ� (����bit���Q��)
						 bit2(out) : ���������ʒu         0:���      1:�O     (����bit���X�V)

	@retval true ����
	@retval false ���s
*/
bool EditView::SearchBracket(
	const Point&	ptLayout,
	Point*		pptLayoutNew,
	int*				mode
	)
{
	size_t len;	//	�s�̒���
	Point ptPos = pEditDoc->layoutMgr.LayoutToLogic(ptLayout);
	const wchar_t* cline = pEditDoc->docLineMgr.GetLine(ptPos.y)->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if (!cline)	//	�Ō�̍s�ɖ{�����Ȃ��ꍇ
		return false;

	// ���ʏ���
	for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
		if (wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0) {
			return SearchBracketForward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
		}else if (wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0) {
			return SearchBracketBackward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
		}
	}

	// 02/09/18 ai Start
	if ((*mode & 2) == 0) {
		// �J�[�\���̑O���𒲂ׂȂ��ꍇ
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	���ʂ�������Ȃ�������C�J�[�\���̒��O�̕����𒲂ׂ�

	if (ptPos.x <= 0) {
		return false;	//	�O�̕����͂Ȃ�
	}

	const wchar_t* bPos = NativeW::GetCharPrev(cline, ptPos.x, cline + ptPos.x);
	ptrdiff_t nCharSize = cline + ptPos.x - bPos;
	// ���ʏ��� 2007.10.16 kobake
	if (nCharSize == 1) {
		ptPos.x = bPos - cline;
		for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
			if (wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0) {
				return SearchBracketForward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
			}else if (wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0) {
				return SearchBracketBackward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
			}
		}
	}
	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:������

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode   [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
// 03/01/08 ai
bool EditView::SearchBracketForward(
	Point	ptPos,
	Point*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int				mode
	)
{
	size_t len;
	int level = 0;


	// �����ʒu�̐ݒ�
	Point ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptPos);	// 02/09/19 ai
	int nSearchNum = (GetTextArea().GetBottomLine()) - ptColLine.y;					// 02/09/19 ai
	DocLine* ci = pEditDoc->docLineMgr.GetLine(ptPos.y);
	const wchar_t* cline = ci->GetDocLineStrWithEOL(&len);
	const wchar_t* lineend = cline + len;
	const wchar_t* cPos = cline + ptPos.x;

//	auto typeData = *pTypeData;
//	auto lineComment = typeData.lineComment;

	do {
		while (cPos < lineend) {
			const wchar_t* nPos = NativeW::GetCharNext(cline, len, cPos);
			if (nPos - cPos > 1) {
				// skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if (wcsncmp(upChar, cPos, 1) == 0) {
				++level;
			}else if (wcsncmp(dnChar, cPos, 1) == 0) {
				--level;
			}// 03/01/08 ai End

			if (level == 0) {	// ���������I
				ptPos.x = cPos - cline;
				*pptLayoutNew = pEditDoc->layoutMgr.LogicToLayout(ptPos);
				return true;
				// Happy Ending
			}
			cPos = nPos;	// ���̕�����
		}

		// 02/09/19 ai Start
		--nSearchNum;
		if (0 > nSearchNum && (mode & 1) == 0) {
			// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̏I�[�̏ꍇ
			//SendStatusMessage("�Ί��ʂ̌����𒆒f���܂���");
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y++;
		ci = ci->GetNextLine();	//	���̃A�C�e��
		if (!ci)
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL(&len);
		cPos = cline;
		lineend = cline + len;
	}while (cline);

	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:�t����

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
bool EditView::SearchBracketBackward(
	Point	ptPos,
	Point*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int				mode
	)
{
	size_t len;
	int level = 1;

	// �����ʒu�̐ݒ�
	Point ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptPos);	// 02/09/19 ai
	int nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	DocLine* ci = pEditDoc->docLineMgr.GetLine(ptPos.y);
	const wchar_t* cline = ci->GetDocLineStrWithEOL(&len);
	const wchar_t* cPos = cline + ptPos.x;

	do {
		while (cPos > cline) {
			const wchar_t* pPos = NativeW::GetCharPrev(cline, len, cPos);
			if (cPos - pPos > 1) {
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if (wcsncmp(upChar, pPos, 1) == 0) {
				++level;
			}else if (wcsncmp(dnChar, pPos, 1) == 0) {
				--level;
			}// 03/01/08 ai End

			if (level == 0) {	// ���������I
				ptPos.x = pPos - cline;
				*pptLayoutNew = pEditDoc->layoutMgr.LogicToLayout(ptPos);
				return true;
				// Happy Ending
			}
			cPos = pPos;	// ���̕�����
		}

		// 02/09/19 ai Start
		--nSearchNum;
		if (0 > nSearchNum && (mode & 1) == 0) {
			// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̐擪�̏ꍇ
			//SendStatusMessage("�Ί��ʂ̌����𒆒f���܂���");
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y--;
		ci = ci->GetPrevLine();	//	���̃A�C�e��
		if (!ci)
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL(&len);
		cPos = cline + len;
	}while (cline);
	
	return false;
}

//@@@ 2003.01.09 Start by ai:
/*!
	@brief ���ʔ���

	@param pLine [in] 
	@param x
	@param size

	@retval true ����
	@retval false �񊇌�
*/
bool EditView::IsBracket(
	const wchar_t* pLine,
	int x,
	int size
	)
{
	// ���ʏ���
	if (size == 1) {
		for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
			if (wcsncmp(p->sStr, &pLine[x], 1) == 0) {
				return true;
			}else if (wcsncmp(p->eStr, &pLine[x], 1) == 0) {
				return true;
			}
		}
	}
	return false;
}
//@@@ 2003.01.09 End

