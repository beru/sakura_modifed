/*!	@file
	@brief �}�N���G���W��

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta

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
#include "CMacroManagerBase.h"
#include "view/CEditView.h"
#include "cmd/CViewCommander_inline.h"
#include "COpeBlk.h"

// MacroBeforeAfter

void MacroBeforeAfter::ExecKeyMacroBefore(
	class EditView* pcEditView,
	int flags
	)
{
	OpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
	if (opeBlk) {
		m_nOpeBlkCount = opeBlk->GetRefCount();
	}else {
		m_nOpeBlkCount = 0;
	}
	m_bDrawSwitchOld = pcEditView->GetDrawSwitch();
}

void MacroBeforeAfter::ExecKeyMacroAfter(
	class EditView* pcEditView,
	int flags,
	bool bRet
	)
{
	OpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
	if (0 < m_nOpeBlkCount) {
		if (!opeBlk) {
			pcEditView->m_cCommander.SetOpeBlk(new OpeBlk());
		}
		if (pcEditView->m_cCommander.GetOpeBlk()->GetRefCount() != m_nOpeBlkCount) {
			pcEditView->m_cCommander.GetOpeBlk()->SetRefCount(m_nOpeBlkCount);
		}
	}else {
		if (opeBlk) {
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			pcEditView->SetUndoBuffer();
		}
	}
	pcEditView->m_pcEditWnd->SetDrawSwitchOfAllViews(m_bDrawSwitchOld);
}

// MacroManagerBase
// �f�t�H���g�̃R���X�g���N�^�E�f�X�g���N�^

MacroManagerBase::MacroManagerBase()
	:
	m_nReady(false)
{}

MacroManagerBase::~MacroManagerBase()
{}

void MacroManagerBase::ExecKeyMacro2(
	class EditView* pcEditView,
	int flags
	)
{
	ExecKeyMacroBefore(pcEditView, flags);
	bool b = ExecKeyMacro(pcEditView, flags);
	ExecKeyMacroAfter(pcEditView, flags, b);
}

