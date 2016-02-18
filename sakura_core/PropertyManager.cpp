/*
	Copyright (C) 2008, kobake

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
#include "PropertyManager.h"
#include "env/DLLSHAREDATA.h"
#include "env/DocTypeManager.h"
#include <memory>

void PropertyManager::Create(
	HWND			hwndOwner,
	ImageListMgr*	pImageList,
	MenuDrawer*		pMenuDrawer
	)
{
	m_hwndOwner = hwndOwner;
	m_pImageList = pImageList;
	m_pMenuDrawer = pMenuDrawer;

	m_nPropComPageNum = -1;
	m_nPropTypePageNum = -1;
}

// 共通設定 プロパティシート
bool PropertyManager::OpenPropertySheet(
	HWND	hWnd,
	int		nPageNum,
	bool	bTrayProc
	)
{
	bool bRet;
	auto pcPropCommon = std::make_unique<PropCommon>();
	pcPropCommon->Create(m_hwndOwner, m_pImageList, m_pMenuDrawer);

	// 2002.12.11 Moca この部分で行われていたデータのコピーをPropCommonに移動・関数化
	// 共通設定の一時設定領域にSharaDataをコピーする
	pcPropCommon->InitData();

	if (nPageNum != -1) {
		m_nPropComPageNum = nPageNum;
	}

	// プロパティシートの作成
	if (pcPropCommon->DoPropertySheet(m_nPropComPageNum, bTrayProc)) {

		// 2002.12.11 Moca この部分で行われていたデータのコピーをPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		// 2007.06.20 ryoji グループ化に変更があったときはグループIDをリセットする
		auto& csTabBar = GetDllShareData().m_common.m_tabBar;
		bool bGroup = (csTabBar.m_bDispTabWnd && !csTabBar.m_bDispTabWndMultiWin);

		// 印刷中にキーワードを上書きしないように
		ShareDataLockCounter* pLock = NULL;
		ShareDataLockCounter::WaitLock(pcPropCommon->m_hwndParent, &pLock);

		pcPropCommon->ApplyData();
		// note: 基本的にここで適用しないで、MYWM_CHANGESETTINGからたどって適用してください。
		// 自ウィンドウには最後に通知されます。大抵は、OnChangeSetting にあります。
		// ここでしか適用しないと、ほかのウィンドウが変更されません。

		if (bGroup != (csTabBar.m_bDispTabWnd && !csTabBar.m_bDispTabWndMultiWin )) {
			AppNodeManager::getInstance()->ResetGroupId();
		}

		// アクセラレータテーブルの再作成
		::SendMessage(GetDllShareData().m_handles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

		// 設定変更を反映させる
		// 全編集ウィンドウへメッセージをポストする
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		bRet = true;
	}else {
		bRet = false;
	}

	// 最後にアクセスしたシートを覚えておく
	m_nPropComPageNum = pcPropCommon->GetPageNum();

	return bRet;
}


// タイプ別設定 プロパティシート
bool PropertyManager::OpenPropertySheetTypes(
	HWND		hWnd,
	int			nPageNum,
	TypeConfigNum	nSettingType
	)
{
	bool bRet;
	auto pcPropTypes = std::make_unique<PropTypes>();
	pcPropTypes->Create(G_AppInstance(), m_hwndOwner);

	auto pType = std::make_unique<TypeConfig>();
	DocTypeManager().GetTypeConfig(nSettingType, *pType);
	pcPropTypes->SetTypeData(*pType);
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように

	if (nPageNum != -1) {
		m_nPropTypePageNum = nPageNum;
	}

	// プロパティシートの作成
	if (pcPropTypes->DoPropertySheet(m_nPropTypePageNum)) {
		// 2013.06.10 Moca 印刷終了まで待機する
		ShareDataLockCounter* pLock = NULL;
		ShareDataLockCounter::WaitLock(pcPropTypes->GetHwndParent(), &pLock);

		pcPropTypes->GetTypeData(*pType);

		DocTypeManager().SetTypeConfig(nSettingType, *pType);

		// アクセラレータテーブルの再作成
		// ::SendMessage(GetDllShareData().m_handles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

		// 設定変更を反映させる
		// 全編集ウィンドウへメッセージをポストする
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nSettingType.GetIndex(),
			(LPARAM)PM_CHANGESETTING_TYPE,
			hWnd
		);
		if (pcPropTypes->GetChangeKeywordSet()) {
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				WM_COMMAND,
				(WPARAM)MAKELONG(F_REDRAW, 0),
				(LPARAM)0,
				hWnd
			);
		}

		delete pLock;
		bRet = true;
	}else {
		bRet = false;
	}

	// 最後にアクセスしたシートを覚えておく
	m_nPropTypePageNum = pcPropTypes->GetPageNum();

	return bRet;
}

