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
	OpeBlk* opeBlk = editView.commander.GetOpeBlk();
	if (opeBlk) {
		nOpeBlkCount = opeBlk->GetRefCount();
	}else {
		nOpeBlkCount = 0;
	}
	bDrawSwitchOld = editView.GetDrawSwitch();
}

void MacroBeforeAfter::ExecKeyMacroAfter(
	class EditView& editView,
	int flags,
	bool bRet
	)
{
	OpeBlk* opeBlk = editView.commander.GetOpeBlk();
	if (0 < nOpeBlkCount) {
		if (!opeBlk) {
			editView.commander.SetOpeBlk(new OpeBlk());
		}
		if (editView.commander.GetOpeBlk()->GetRefCount() != nOpeBlkCount) {
			editView.commander.GetOpeBlk()->SetRefCount(nOpeBlkCount);
		}
	}else {
		if (opeBlk) {
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			editView.SetUndoBuffer();
		}
	}
	editView.editWnd.SetDrawSwitchOfAllViews(bDrawSwitchOld);
}

// MacroManagerBase
// �f�t�H���g�̃R���X�g���N�^�E�f�X�g���N�^

MacroManagerBase::MacroManagerBase()
	:
	nReady(false)
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

