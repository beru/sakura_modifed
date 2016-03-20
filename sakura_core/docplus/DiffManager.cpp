#include "StdAfx.h"
#include "docplus/DiffManager.h"
#include "types/TypeSupport.h"
#include "window/EditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DiffMark DiffLineGetter::GetLineDiffMark() const { return (DiffMark)m_pDocLine->m_mark.m_diffMarked; }

/*! �s�̍����}�[�N�ɑΉ������F��Ԃ� -> pColorIndex
	
	�F�ݒ肪�����ꍇ�� pColorIndex ��ύX������ false ��Ԃ��B	
*/
bool DiffLineGetter::GetDiffColor(EColorIndexType* pColorIndex) const
{
	DiffMark type = GetLineDiffMark();
	EditView& view = EditWnd::getInstance().GetActiveView();

	// DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	if (type != DiffMark::None) {
		switch (type) {
		case DiffMark::Append:	// �ǉ�
			if (TypeSupport(view, COLORIDX_DIFF_APPEND).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_APPEND;
				return true;
			}
			break;
		case DiffMark::Change:	// �ύX
			if (TypeSupport(view, COLORIDX_DIFF_CHANGE).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_CHANGE;
				return true;
			}
			break;
		case DiffMark::Delete:	// �폜
		case DiffMark::DeleteEx:	// �폜
			if (TypeSupport(view, COLORIDX_DIFF_DELETE).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_DELETE;
				return true;
			}
			break;
		}
	}
	return false;
}


/*! DIFF�}�[�N�`��

	�����͉��B�i���ʂȈ������肻���j
*/
bool DiffLineGetter::DrawDiffMark(
	Graphics& gr,
	int y,
	int nLineHeight,
	COLORREF color
	) const
{
	DiffMark type = GetLineDiffMark();

	if (type != DiffMark::None) {	// DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
		int	cy = y + nLineHeight / 2;

		gr.PushPen(color, 0);

		switch (type) {
		case DiffMark::Append:	// �ǉ�
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 6, cy);
			::MoveToEx(gr, 4, cy - 2, NULL);
			::LineTo  (gr, 4, cy + 3);
			break;

		case DiffMark::Change:	// �ύX
			::MoveToEx(gr, 3, cy - 4, NULL);
			::LineTo  (gr, 3, cy);
			::MoveToEx(gr, 3, cy + 2, NULL);
			::LineTo  (gr, 3, cy + 3);
			break;

		case DiffMark::Delete:	// �폜
			cy -= 3;
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 5, cy);
			::LineTo  (gr, 3, cy + 2);
			::LineTo  (gr, 3, cy);
			::LineTo  (gr, 7, cy + 4);
			break;
		
		case DiffMark::DeleteEx:	// �폜(EOF)
			cy += 3;
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 5, cy);
			::LineTo  (gr, 3, cy - 2);
			::LineTo  (gr, 3, cy);
			::LineTo  (gr, 7, cy - 4);
			break;
		}

		gr.PopPen();

		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineSetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DiffLineSetter::SetLineDiffMark(DiffMark mark) { m_pDocLine->m_mark.m_diffMarked = mark; }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�����\���̑S����
	@author	MIK
	@date	2002.05.25
*/
void DiffLineMgr::ResetAllDiffMark()
{
	DocLine* pDocLine = m_docLineMgr.GetDocLineTop();
	while (pDocLine) {
		pDocLine->m_mark.m_diffMarked = DiffMark::None;
		pDocLine = pDocLine->GetNextLine();
	}

	DiffManager::getInstance().SetDiffUse(false);
}

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
bool DiffLineMgr::SearchDiffMark(
	LogicInt			nLineNum,		// �����J�n�s
	SearchDirection		bPrevOrNext,	// 0==�O������ 1==�������
	LogicInt*			pnLineNum 		// �}�b�`�s
	)
{
	LogicInt nLinePos = nLineNum;

	// �O������
	if (bPrevOrNext == SearchDirection::Backward) {
		--nLinePos;
		const DocLine* pDocLine = m_docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != DiffMark::None) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	// �������
	}else {
		++nLinePos;
		const DocLine* pDocLine = m_docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != DiffMark::None) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	���������s�͈͎w��œo�^����B
	@author	MIK
	@date	2002/05/25
*/
void DiffLineMgr::SetDiffMarkRange(DiffMark nMode, LogicInt nStartLine, LogicInt nEndLine)
{
	DiffManager::getInstance().SetDiffUse(true);

	if (nStartLine < LogicInt(0)) {
		nStartLine = LogicInt(0);
	}
	// �ŏI�s����ɍ폜�s����
	LogicInt nLines = m_docLineMgr.GetLineCount();
	if (nLines <= nEndLine) {
		nEndLine = nLines - LogicInt(1);
		DocLine* pDocLine = m_docLineMgr.GetLine(nEndLine);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(DiffMark::DeleteEx);
		}
	}

	// �s�͈͂Ƀ}�[�N������
	for (LogicInt i=nStartLine; i<=nEndLine; ++i) {
		DocLine* pDocLine = m_docLineMgr.GetLine(i);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(nMode);
		}
	}

	return;
}

