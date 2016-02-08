/*
	Copyright (C) 2013, novice

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
#pragma once

#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "COpeBlk.h"

// �O���ˑ�
inline EditDoc* ViewCommander::GetDocument()
{
	return m_pCommanderView->m_pcEditDoc;
}

inline CEditWnd* ViewCommander::GetEditWindow()
{
	return m_pCommanderView->m_pcEditWnd;
}

inline HWND ViewCommander::GetMainWindow()
{
	return ::GetParent(m_pCommanderView->m_hwndParent);
}

inline OpeBlk* ViewCommander::GetOpeBlk()
{
	return GetDocument()->m_cDocEditor.m_pcOpeBlk;
}

inline void ViewCommander::SetOpeBlk(OpeBlk* p)
{
	GetDocument()->m_cDocEditor.m_pcOpeBlk = p;
	GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount = 0;
}

inline LayoutRange& ViewCommander::GetSelect()
{
	return m_pCommanderView->GetSelectionInfo().m_sSelect;
}

inline CCaret& ViewCommander::GetCaret()
{
	return m_pCommanderView->GetCaret();
}

