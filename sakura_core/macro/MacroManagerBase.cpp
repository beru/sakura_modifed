/*!	@file
	@brief �}�N���G���W��
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

