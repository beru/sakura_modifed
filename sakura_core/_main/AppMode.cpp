#include "StdAfx.h"
#include "AppMode.h"
#include "window/EditWnd.h"
#include "env/SakuraEnvironment.h"

void AppMode::OnAfterSave(const SaveInfo& saveInfo)
{
	bViewMode = false;	// ビューモード
	if (IsDebugMode()) {
		SetDebugModeOFF();	// アウトプットウィンドウは通常ウィンドウ化
	}
}

// デバッグモニタモードに設定
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

