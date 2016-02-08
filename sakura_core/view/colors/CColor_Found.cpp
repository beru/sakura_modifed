#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Found.h"
#include "types/CTypeSupport.h"
#include "view/CViewSelect.h"
#include <limits.h>


void CColor_Select::OnStartScanLogic()
{
	m_nSelectLine	= LayoutInt(-1);
	m_nSelectStart	= LogicInt(-1);
	m_nSelectEnd	= LogicInt(-1);
}

bool CColor_Select::BeginColor(const CStringRef& cStr, int nPos)
{
	assert(0);
	return false;
}

bool CColor_Select::BeginColorEx(const CStringRef& cStr, int nPos, LayoutInt nLineNum, const Layout* pcLayout)
{
	if (!cStr.IsValid()) return false;

	const CEditView& view = *(CColorStrategyPool::getInstance()->GetCurrentView());
	if (!view.GetSelectionInfo().IsTextSelected() || !CTypeSupport(&view, COLORIDX_SELECT).IsDisp()) {
		return false;
	}

	// 2011.12.27 ���C�A�E�g�s����1�񂾂��m�F���Ă��Ƃ̓����o�[�ϐ����݂�
	if (m_nSelectLine == nLineNum) {
		return (m_nSelectStart <= nPos && nPos < m_nSelectEnd);
	}
	m_nSelectLine = nLineNum;
	LayoutRange selectArea = view.GetSelectionInfo().GetSelectAreaLine(nLineNum, pcLayout);
	LayoutInt nSelectFrom = selectArea.GetFrom().x;
	LayoutInt nSelectTo = selectArea.GetTo().x;
	if (nSelectFrom == nSelectTo || nSelectFrom == -1) {
		m_nSelectStart = -1;
		m_nSelectEnd = -1;
		return false;
	}
	LogicInt nIdxFrom = view.LineColumnToIndex(pcLayout, nSelectFrom) + pcLayout->GetLogicOffset();
	LogicInt nIdxTo = view.LineColumnToIndex(pcLayout, nSelectTo) + pcLayout->GetLogicOffset();
	m_nSelectStart = nIdxFrom;
	m_nSelectEnd = nIdxTo;
	if (m_nSelectStart <= nPos && nPos < m_nSelectEnd) {
		return true;
	}
	return false;
}

bool CColor_Select::EndColor(const CStringRef& cStr, int nPos)
{
	// �}�b�`������I�����o
	return (m_nSelectEnd <= nPos);
}


CColor_Found::CColor_Found()
	:
	validColorNum(0)
{
}

void CColor_Found::OnStartScanLogic()
{
	m_nSearchResult	= 1;
	m_nSearchStart	= LogicInt(-1);
	m_nSearchEnd	= LogicInt(-1);

	this->validColorNum = 0;
	for (int color=COLORIDX_SEARCH; color<=COLORIDX_SEARCHTAIL; ++color) {
		if (m_pTypeData->m_ColorInfoArr[color].m_bDisp) {
			this->highlightColors[this->validColorNum++] = EColorIndexType(color);
		}
	}
}

bool CColor_Found::BeginColor(const CStringRef& cStr, int nPos)
{
	if (!cStr.IsValid()) return false;
	const CEditView* pcView = CColorStrategyPool::getInstance()->GetCurrentView();
	if (!pcView->m_bCurSrchKeyMark || this->validColorNum == 0) {
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor ���K�\���̌���������}�[�N������������
	if (pcView->m_curSearchOption.bWordOnly || (m_nSearchResult && m_nSearchStart < nPos)) {
		m_nSearchResult = pcView->IsSearchString(
			cStr,
			LogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	// �}�b�`�����񌟏o
	return (m_nSearchResult && m_nSearchStart == nPos);
}

bool CColor_Found::EndColor(const CStringRef& cStr, int nPos)
{
	// �}�b�`������I�����o
	return (m_nSearchEnd <= nPos); //+ == �ł͍s�������̏ꍇ�Am_nSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
}

