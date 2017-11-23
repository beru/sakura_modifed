#include "StdAfx.h"
#include "docplus/DiffManager.h"
#include "types/TypeSupport.h"
#include "window/EditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DiffMark DiffLineGetter::GetLineDiffMark() const { return (DiffMark)pDocLine->mark.diffMarked; }

/*! �s�̍����}�[�N�ɑΉ������F��Ԃ� -> pColorIndex
	
	�F�ݒ肪�����ꍇ�� pColorIndex ��ύX������ false ��Ԃ��B	
*/
bool DiffLineGetter::GetDiffColor(EColorIndexType* pColorIndex) const
{
	DiffMark type = GetLineDiffMark();
	EditView& view = EditWnd::getInstance().GetActiveView();

	// DIFF�����}�[�N�\��	
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
	unsigned int nLineHeight,
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

void DiffLineSetter::SetLineDiffMark(DiffMark mark) { pDocLine->mark.diffMarked = mark; }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�����\���̑S���� */
void DiffLineMgr::ResetAllDiffMark()
{
	DocLine* pDocLine = docLineMgr.GetDocLineTop();
	while (pDocLine) {
		pDocLine->mark.diffMarked = DiffMark::None;
		pDocLine = pDocLine->GetNextLine();
	}

	DiffManager::getInstance().SetDiffUse(false);
}

/*! �������� */
bool DiffLineMgr::SearchDiffMark(
	size_t			nLineNum,		// �����J�n�s
	SearchDirection	bPrevOrNext,	// 0==�O������ 1==�������
	size_t*			pnLineNum 		// �}�b�`�s
	)
{
	size_t nLinePos = nLineNum;

	// �O������
	if (bPrevOrNext == SearchDirection::Backward) {
		--nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
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
		const DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
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

/*!	���������s�͈͎w��œo�^���� */
void DiffLineMgr::SetDiffMarkRange(DiffMark nMode, size_t nStartLine, size_t nEndLine)
{
	DiffManager::getInstance().SetDiffUse(true);

	if (nStartLine < 0) {
		nStartLine = 0;
	}
	// �ŏI�s����ɍ폜�s����
	size_t nLines = docLineMgr.GetLineCount();
	if (nLines <= nEndLine) {
		nEndLine = nLines - 1;
		DocLine* pDocLine = docLineMgr.GetLine(nEndLine);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(DiffMark::DeleteEx);
		}
	}

	// �s�͈͂Ƀ}�[�N������
	for (size_t i=nStartLine; i<=nEndLine; ++i) {
		DocLine* pDocLine = docLineMgr.GetLine(i);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(nMode);
		}
	}

	return;
}

