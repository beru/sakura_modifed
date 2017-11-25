#include "StdAfx.h"
#include "PropertyManager.h"
#include "env/DllSharedData.h"
#include "env/DocTypeManager.h"
#include <memory>

void PropertyManager::Create(
	HWND			hwndOwner,
	ImageListMgr*	pImageList,
	MenuDrawer*		pMenuDrawer
	)
{
	this->hwndOwner = hwndOwner;
	this->pImageList = pImageList;
	this->pMenuDrawer = pMenuDrawer;

	nPropComPageNum = -1;
	nPropTypePageNum = -1;
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
	pcPropCommon->Create(hwndOwner, pImageList, pMenuDrawer);

	// 共通設定の一時設定領域にSharaDataをコピーする
	pcPropCommon->InitData();

	if (nPageNum != -1) {
		nPropComPageNum = nPageNum;
	}

	// プロパティシートの作成
	if (pcPropCommon->DoPropertySheet(nPropComPageNum, bTrayProc)) {

		// ShareData に 設定を適用・コピーする
		// グループ化に変更があったときはグループIDをリセットする
		auto& csTabBar = GetDllShareData().common.tabBar;
		bool bGroup = (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin);

		// 印刷中にキーワードを上書きしないように
		ShareDataLockCounter* pLock = nullptr;
		ShareDataLockCounter::WaitLock(pcPropCommon->hwndParent, &pLock);

		pcPropCommon->ApplyData();
		// note: 基本的にここで適用しないで、MYWM_CHANGESETTINGからたどって適用してください。
		// 自ウィンドウには最後に通知されます。大抵は、OnChangeSetting にあります。
		// ここでしか適用しないと、ほかのウィンドウが変更されません。

		if (bGroup != (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin )) {
			AppNodeManager::getInstance().ResetGroupId();
		}

		// アクセラレータテーブルの再作成
		::SendMessage(GetDllShareData().handles.hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

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
	nPropComPageNum = pcPropCommon->GetPageNum();

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
	pcPropTypes->Create(G_AppInstance(), hwndOwner);

	auto pType = std::make_unique<TypeConfig>();
	DocTypeManager().GetTypeConfig(nSettingType, *pType);
	pcPropTypes->SetTypeData(*pType);
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように

	if (nPageNum != -1) {
		nPropTypePageNum = nPageNum;
	}

	// プロパティシートの作成
	if (pcPropTypes->DoPropertySheet(nPropTypePageNum)) {
		ShareDataLockCounter* pLock = nullptr;
		ShareDataLockCounter::WaitLock(pcPropTypes->GetHwndParent(), &pLock);

		pcPropTypes->GetTypeData(*pType);

		DocTypeManager().SetTypeConfig(nSettingType, *pType);

		// アクセラレータテーブルの再作成
		// ::SendMessage(GetDllShareData().handles.hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

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
	nPropTypePageNum = pcPropTypes->GetPageNum();

	return bRet;
}

