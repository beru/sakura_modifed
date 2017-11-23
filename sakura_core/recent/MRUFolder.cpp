#include "StdAfx.h"
#include "MRUFolder.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "uiparts/MenuDrawer.h"	//	これでいいのか？
#include "util/string_ex2.h"
#include "util/window.h"

MruFolder::MruFolder()
{
	// 初期化。
	pShareData = &GetDllShareData();
}

MruFolder::~MruFolder()
{
	recentFolder.Terminate();
}

/*!
	フォルダ履歴メニューの作成
	
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@return 生成したメニューのハンドル
*/
HMENU MruFolder::CreateMenu(MenuDrawer& menuDrawer) const
{
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, menuDrawer);
}

/*!
	フォルダ履歴メニューの作成
	
	@param 追加するメニューのハンドル
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@return メニューのハンドル
*/
HMENU MruFolder::CreateMenu(HMENU	hMenuPopUp, MenuDrawer& menuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	メニューキャプション

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	FileNameManager::getInstance().TransformFileName_MakeCache();
	for (size_t i=0; i<recentFolder.GetItemCount(); ++i) {
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= recentFolder.GetViewCount()) {
			break;
		}

		const TCHAR* pszFolder = recentFolder.GetItemText(i);
		bool bFavorite = recentFolder.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !pShareData->common.window.bMenuIcon;
		FileNameManager::getInstance().GetMenuFullLabel(szMenu, _countof(szMenu), true, pszFolder, -1, false, CODE_NONE, bFavoriteLabel, i, true, dcFont.GetHDC());

		// メニューに追加
		menuDrawer.MyAppendMenu(
			hMenuPopUp,
			MF_BYPOSITION | MF_STRING,
			IDM_SELOPENFOLDER + i,
			szMenu,
			_T(""),
			TRUE,
			bFavorite ? F_FAVORITE : -1
		);
	}
	return hMenuPopUp;
}

std::vector<LPCTSTR> MruFolder::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for (size_t i=0; i<recentFolder.GetItemCount(); ++i) {
		// 「共通設定」→「全般」→「フォルダの履歴MAX」を反映
		if (i >= recentFolder.GetViewCount()) {
			break;
		}
		ret.push_back(recentFolder.GetItemText(i));
	}
	return ret;
}

size_t MruFolder::Length() const
{
	return recentFolder.GetItemCount();
}

void MruFolder::ClearAll()
{
	recentFolder.DeleteAllItem();
}

/* @brief 開いたフォルダ リストへの登録 */
void MruFolder::Add(const TCHAR* pszFolder)
{
	if (!pszFolder
	 || pszFolder[0] == _T('\0')
	) {	//	長さが0なら排除。
		return;
	}

	// すでに登録されている場合は、除外指定を無視する
	if (recentFolder.FindItemByText(pszFolder) == -1) {
		size_t nSize = pShareData->history.aExceptMRU.size();
		for (size_t i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(pShareData->history.aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pszFolder, szExceptMRU)) {
				return;
			}
		}
	}

	recentFolder.AppendItem(pszFolder);
}

const TCHAR* MruFolder::GetPath(int num) const
{
	return recentFolder.GetItemText(num);
}

