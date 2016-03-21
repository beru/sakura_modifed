/*!	@file
	@brief マクロエンジン

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
#include "MacroManagerBase.h"
#include "view/EditView.h"
#include "cmd/ViewCommander_inline.h"
#include "OpeBlk.h"

// MacroBeforeAfter

void MacroBeforeAfter::ExecKeyMacroBefore(
	class EditView& editView,
	int flags
	)
{
	OpeBlk* opeBlk = editView.m_commander.GetOpeBlk();
	if (opeBlk) {
		m_nOpeBlkCount = opeBlk->GetRefCount();
	}else {
		m_nOpeBlkCount = 0;
	}
	m_bDrawSwitchOld = editView.GetDrawSwitch();
}

void MacroBeforeAfter::ExecKeyMacroAfter(
	class EditView& editView,
	int flags,
	bool bRet
	)
{
	OpeBlk* opeBlk = editView.m_commander.GetOpeBlk();
	if (0 < m_nOpeBlkCount) {
		if (!opeBlk) {
			editView.m_commander.SetOpeBlk(new OpeBlk());
		}
		if (editView.m_commander.GetOpeBlk()->GetRefCount() != m_nOpeBlkCount) {
			editView.m_commander.GetOpeBlk()->SetRefCount(m_nOpeBlkCount);
		}
	}else {
		if (opeBlk) {
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			editView.SetUndoBuffer();
		}
	}
	editView.m_editWnd.SetDrawSwitchOfAllViews(m_bDrawSwitchOld);
}

// MacroManagerBase
// デフォルトのコンストラクタ・デストラクタ

MacroManagerBase::MacroManagerBase()
	:
	m_nReady(false)
{}

MacroManagerBase::~MacroManagerBase()
{}

void MacroManagerBase::ExecKeyMacro2(
	class EditView& editView,
	int flags
	)
{
	ExecKeyMacroBefore(editView, flags);
	bool b = ExecKeyMacro(editView, flags);
	ExecKeyMacroAfter(editView, flags, b);
}

