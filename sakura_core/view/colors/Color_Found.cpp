#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Found.h"
#include "types/TypeSupport.h"
#include "view/ViewSelect.h"
#include <limits.h>


void Color_Select::OnStartScanLogic()
{
	nSelectLine	= -1;
	nSelectStart = -1;
	nSelectEnd = -1;
}

bool Color_Select::BeginColor(const StringRef& str, int nPos)
{
	assert(0);
	return false;
}

bool Color_Select::BeginColorEx(const StringRef& str, int nPos, int nLineNum, const Layout* pLayout)
{
	if (!str.IsValid()) return false;

	const EditView& view = *(ColorStrategyPool::getInstance().GetCurrentView());
	if (!view.GetSelectionInfo().IsTextSelected() || !TypeSupport(view, COLORIDX_SELECT).IsDisp()) {
		return false;
	}

	// 2011.12.27 ���C�A�E�g�s����1�񂾂��m�F���Ă��Ƃ̓����o�[�ϐ����݂�
	if (nSelectLine == nLineNum) {
		return (nSelectStart <= nPos && nPos < nSelectEnd);
	}
	nSelectLine = nLineNum;
	Range selectArea = view.GetSelectionInfo().GetSelectAreaLine(nLineNum, pLayout);
	int nSelectFrom = selectArea.GetFrom().x;
	int nSelectTo = selectArea.GetTo().x;
	if (nSelectFrom == nSelectTo || nSelectFrom == -1) {
		nSelectStart = -1;
		nSelectEnd = -1;
		return false;
	}
	int nIdxFrom = view.LineColumnToIndex(pLayout, nSelectFrom) + pLayout->GetLogicOffset();
	int nIdxTo = view.LineColumnToIndex(pLayout, nSelectTo) + pLayout->GetLogicOffset();
	nSelectStart = nIdxFrom;
	nSelectEnd = nIdxTo;
	if (nSelectStart <= nPos && nPos < nSelectEnd) {
		return true;
	}
	return false;
}

bool Color_Select::EndColor(const StringRef& str, int nPos)
{
	// �}�b�`������I�����o
	return (nSelectEnd <= nPos);
}


Color_Found::Color_Found()
	:
	validColorNum(0)
{
}

void Color_Found::OnStartScanLogic()
{
	nSearchResult = 1;
	nSearchStart = -1;
	nSearchEnd = -1;

	this->validColorNum = 0;
	for (int color=COLORIDX_SEARCH; color<=COLORIDX_SEARCHTAIL; ++color) {
		if (pTypeData->colorInfoArr[color].bDisp) {
			this->highlightColors[this->validColorNum++] = EColorIndexType(color);
		}
	}
}

bool Color_Found::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;
	const EditView* pView = ColorStrategyPool::getInstance().GetCurrentView();
	if (!pView->bCurSrchKeyMark || this->validColorNum == 0) {
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if (pView->curSearchOption.bWordOnly || (nSearchResult && nSearchStart < nPos)) {
		nSearchResult = pView->IsSearchString(
			str,
			nPos,
			&nSearchStart,
			&nSearchEnd
		);
	}
	// �}�b�`�����񌟏o
	return (nSearchResult && nSearchStart == nPos);
}

bool Color_Found::EndColor(const StringRef& str, int nPos)
{
	// �}�b�`������I�����o
	return (nSearchEnd <= nPos); //+ == �ł͍s�������̏ꍇ�AnSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
}

