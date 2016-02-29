#include "StdAfx.h"
#include "AppMode.h"
#include "window/EditWnd.h"
#include "env/SakuraEnvironment.h"

void AppMode::OnAfterSave(const SaveInfo& saveInfo)
{
	m_bViewMode = false;	// �r���[���[�h
	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
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
	shared.handles.hwndDebug = EditWnd::getInstance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// �r���[���[�h	// 2001/06/23 N.Nakatani �A�E�g�v�b�g���ւ̏o�̓e�L�X�g�̒ǉ�F_ADDTAIL_W���}�~�����̂łƂ肠�����r���[���[�h�͎��߂܂���
	EditWnd::getInstance()->UpdateCaption();
}

// 2005.06.24 Moca
// �f�o�b�N���j�^���[�h�̉���
void AppMode::SetDebugModeOFF()
{
	auto& shared = GetDllShareData();
	if (shared.handles.hwndDebug == EditWnd::getInstance()->GetHwnd()) {
		shared.handles.hwndDebug = NULL;
		this->_SetDebugMode(false);
		EditWnd::getInstance()->UpdateCaption();
	}
}

