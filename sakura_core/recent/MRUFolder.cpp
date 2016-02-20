/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "MRUFolder.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "uiparts/MenuDrawer.h"	//	これでいいのか？
#include "util/string_ex2.h"
#include "util/window.h"

/*!	コンストラクタ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
MruFolder::MruFolder()
{
	// 初期化。
	m_pShareData = &GetDllShareData();
}

// デストラクタ
MruFolder::~MruFolder()
{
	m_recentFolder.Terminate();
}

/*!
	フォルダ履歴メニューの作成
	
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@return 生成したメニューのハンドル

	2010/5/21 Uchi 組み直し
*/
HMENU MruFolder::CreateMenu(MenuDrawer* pMenuDrawer) const
{
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, pMenuDrawer);
}

/*!
	フォルダ履歴メニューの作成
	
	@param 追加するメニューのハンドル
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return メニューのハンドル
*/
HMENU MruFolder::CreateMenu(HMENU	hMenuPopUp, MenuDrawer* pMenuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	メニューキャプション

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	FileNameManager::getInstance()->TransformFileName_MakeCache();
	for (int i=0; i<m_recentFolder.GetItemCount(); ++i) {
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= m_recentFolder.GetViewCount()) {
			break;
		}

		const TCHAR* pszFolder = m_recentFolder.GetItemText(i);
		bool bFavorite = m_recentFolder.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !m_pShareData->m_common.window.bMenuIcon;
		FileNameManager::getInstance()->GetMenuFullLabel(szMenu, _countof(szMenu), true, pszFolder, -1, false, CODE_NONE, bFavoriteLabel, i, true, dcFont.GetHDC());

		// メニューに追加
		pMenuDrawer->MyAppendMenu(
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
	for (int i=0; i<m_recentFolder.GetItemCount(); ++i) {
		// 「共通設定」→「全般」→「フォルダの履歴MAX」を反映
		if (i >= m_recentFolder.GetViewCount()) {
			break;
		}
		ret.push_back(m_recentFolder.GetItemText(i));
	}
	return ret;
}

int MruFolder::Length() const
{
	return m_recentFolder.GetItemCount();
}

void MruFolder::ClearAll()
{
	m_recentFolder.DeleteAllItem();
}

/*	@brief 開いたフォルダ リストへの登録

	@date 2001.12.26  CShareData::AddOPENFOLDERListから移動した。（YAZAKI）
*/
void MruFolder::Add(const TCHAR* pszFolder)
{
	if (!pszFolder
	 || pszFolder[0] == _T('\0')
	) {	//	長さが0なら排除。
		return;
	}

	// すでに登録されている場合は、除外指定を無視する
	if (m_recentFolder.FindItemByText(pszFolder) == -1) {
		int nSize = m_pShareData->m_history.m_aExceptMRU.size();
		for (int i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(m_pShareData->m_history.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pszFolder, szExceptMRU)) {
				return;
			}
		}
	}

	m_recentFolder.AppendItem(pszFolder);
}

const TCHAR* MruFolder::GetPath(int num) const
{
	return m_recentFolder.GetItemText(num);
}

