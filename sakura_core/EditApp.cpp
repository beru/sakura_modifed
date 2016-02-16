/*
	Copyright (C) 2007, kobake

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
#include "EditApp.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "LoadAgent.h"
#include "SaveAgent.h"
#include "uiparts/VisualProgress.h"
#include "recent/MruListener.h"
#include "macro/SMacroMgr.h"
#include "PropertyManager.h"
#include "GrepAgent.h"
#include "_main/AppMode.h"
#include "_main/CommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void EditApp::Create(HINSTANCE hInst, int nGroupId)
{
	m_hInst = hInst;

	// ヘルパ作成
	m_icons.Create(m_hInst);	//	CreateImage List

	// ドキュメントの作成
	m_pEditDoc = new EditDoc(this);

	// IO管理
	m_pLoadAgent = new LoadAgent();
	m_pSaveAgent = new SaveAgent();
	m_pVisualProgress = new VisualProgress();

	// GREPモード管理
	m_pGrepAgent = new GrepAgent();

	// 編集モード
	AppMode::getInstance();	// ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	// マクロ
	m_pSMacroMgr = new SMacroMgr();

	// ウィンドウの作成
	m_pEditWnd = EditWnd::getInstance();

	m_pEditDoc->Create(m_pEditWnd);
	m_pEditWnd->Create(m_pEditDoc, &m_icons, nGroupId);

	// MRU管理
	m_pMruListener = new MruListener();

	// プロパティ管理
	m_pPropertyManager = new PropertyManager();
	m_pPropertyManager->Create(
		m_pEditWnd->GetHwnd(),
		&GetIcons(),
		&m_pEditWnd->GetMenuDrawer()
	);
}

EditApp::~EditApp()
{
	delete m_pSMacroMgr;
	delete m_pPropertyManager;
	delete m_pMruListener;
	delete m_pGrepAgent;
	delete m_pVisualProgress;
	delete m_pSaveAgent;
	delete m_pLoadAgent;
	delete m_pEditDoc;
}

// 共通設定 プロパティシート
bool EditApp::OpenPropertySheet(int nPageNum)
{
	// プロパティシートの作成
	bool bRet = m_pPropertyManager->OpenPropertySheet(m_pEditWnd->GetHwnd(), nPageNum, false);
	if (bRet) {
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		m_pSMacroMgr->UnloadAll();
	}
	return bRet;
}

// タイプ別設定 プロパティシート
bool EditApp::OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType)
{
	bool bRet = m_pPropertyManager->OpenPropertySheetTypes(m_pEditWnd->GetHwnd(), nPageNum, nSettingType);

	return bRet;
}

