#include "StdAfx.h"
#include "AppMode.h"
#include "window/EditWnd.h"
#include "env/SakuraEnvironment.h"

void AppMode::OnAfterSave(const SaveInfo& saveInfo)
{
	bViewMode = false;	// �r���[���[�h
	if (IsDebugMode()) {
		SetDebugModeOFF();	// �A�E�g�v�b�g�E�B���h�E�͒ʏ�E�B���h�E��
	}
}

// �f�o�b�O���j�^���[�h�ɐݒ�
void AppMode::SetDebugModeON()
{
	auto& shared = GetDllShareData();
	if (shared.handles.hwndDebug) {
		if (IsSakuraMainWindow(shared.handles.hwndDebug)) {
			return;
		}
	}
	shared.handles.hwndDebug = EditWnd::getInstance().GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);
	EditWnd::getInstance().UpdateCaption();
}

void AppMode::SetDebugModeOFF()
{
	auto& shared = GetDllShareData();
	if (shared.handles.hwndDebug == EditWnd::getInstance().GetHwnd()) {
		shared.handles.hwndDebug = NULL;
		this->_SetDebugMode(false);
		EditWnd::getInstance().UpdateCaption();
	}
}

