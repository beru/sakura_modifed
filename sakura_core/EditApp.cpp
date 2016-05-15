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
	this->hInst = hInst;

	// ヘルパ作成
	icons.Create(hInst);	//	CreateImage List

	// ドキュメントの作成
	pEditDoc = new EditDoc(*this);

	// IO管理
	pLoadAgent = new LoadAgent();
	pSaveAgent = new SaveAgent();
	pVisualProgress = new VisualProgress();

	// GREPモード管理
	pGrepAgent = new GrepAgent();

	// 編集モード
	AppMode::getInstance();	// ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	// マクロ
	pSMacroMgr = new SMacroMgr();

	// ウィンドウの作成
	pEditWnd = &EditWnd::getInstance();

	pEditDoc->Create(pEditWnd);
	pEditWnd->Create(pEditDoc, &icons, nGroupId);

	// MRU管理
	pMruListener = new MruListener();

	// プロパティ管理
	pPropertyManager = new PropertyManager();
	pPropertyManager->Create(
		pEditWnd->GetHwnd(),
		&GetIcons(),
		&pEditWnd->GetMenuDrawer()
	);
}

EditApp::~EditApp()
{
	delete pSMacroMgr;
	delete pPropertyManager;
	delete pMruListener;
	delete pGrepAgent;
	delete pVisualProgress;
	delete pSaveAgent;
	delete pLoadAgent;
	delete pEditDoc;
}

// 共通設定 プロパティシート
bool EditApp::OpenPropertySheet(int nPageNum)
{
	// プロパティシートの作成
	bool bRet = pPropertyManager->OpenPropertySheet(pEditWnd->GetHwnd(), nPageNum, false);
	if (bRet) {
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		pSMacroMgr->UnloadAll();
	}
	return bRet;
}

// タイプ別設定 プロパティシート
bool EditApp::OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType)
{
	bool bRet = pPropertyManager->OpenPropertySheetTypes(pEditWnd->GetHwnd(), nPageNum, nSettingType);

	return bRet;
}

