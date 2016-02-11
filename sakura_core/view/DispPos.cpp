#include "StdAfx.h"
#include "DispPos.h"
#include "doc/layout/CLayout.h"

// $$$������
void DispPos::ForwardLayoutLineRef(int nOffsetLine)
{
	m_nLineRef += LayoutInt(nOffsetLine);
	// �L���b�V���X�V
	int n = nOffsetLine;
	if (m_pLayoutRef) {
		while (n > 0 && m_pLayoutRef) {
			m_pLayoutRef = m_pLayoutRef->GetNextLayout();
			--n;
		}
		while (n < 0 && m_pLayoutRef) {
			m_pLayoutRef = m_pLayoutRef->GetPrevLayout();
			++n;
		}
	}else {
		m_pLayoutRef = EditDoc::GetInstance(0)->m_layoutMgr.SearchLineByLayoutY(m_nLineRef);
	}
}

