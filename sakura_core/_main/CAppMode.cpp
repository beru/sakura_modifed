#include "StdAfx.h"
#include "CAppMode.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"

void AppMode::OnAfterSave(const SaveInfo& sSaveInfo)
{
	m_bViewMode = false;	// ビューモード
	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	if (IsDebugMode()) {
		SetDebugModeOFF();	// アウトプットウィンドウは通常ウィンドウ化
	}
}

// デバッグモニタモードに設定
void AppMode::SetDebugModeON()
{
	auto& shared = GetDllShareData();
	if (shared.m_handles.m_hwndDebug) {
		if (IsSakuraMainWindow(shared.m_handles.m_hwndDebug)) {
			return;
		}
	}
	shared.m_handles.m_hwndDebug = EditWnd::getInstance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// ビューモード	// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAIL_Wが抑止されるのでとりあえずビューモードは辞めました
	EditWnd::getInstance()->UpdateCaption();
}

// 2005.06.24 Moca
// デバックモニタモードの解除
void AppMode::SetDebugModeOFF()
{
	auto& shared = GetDllShareData();
	if (shared.m_handles.m_hwndDebug == EditWnd::getInstance()->GetHwnd()) {
		shared.m_handles.m_hwndDebug = NULL;
		this->_SetDebugMode(false);
		EditWnd::getInstance()->UpdateCaption();
	}
}

