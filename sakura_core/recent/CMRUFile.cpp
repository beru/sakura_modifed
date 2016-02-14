/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <ShlObj.h>
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"
#include "uiparts/CMenuDrawer.h"	//	これでいいのか？
#include "window/CEditWnd.h"
#include "util/string_ex2.h"
#include "util/window.h"

/*!	コンストラクタ
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
MRUFile::MRUFile()
{
	//	初期化。
	m_pShareData = &GetDllShareData();
}

// デストラクタ
MRUFile::~MRUFile()
{
	m_recentFile.Terminate();
}

/*!
	ファイル履歴メニューの作成
	
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return 生成したメニューのハンドル

	2010/5/21 Uchi 組み直し
*/
HMENU MRUFile::CreateMenu(MenuDrawer* pMenuDrawer) const
{
	//	空メニューを作る
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, pMenuDrawer);
}
/*!
	ファイル履歴メニューの作成
	
	@param 追加するメニューのハンドル
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return 生成したメニューのハンドル

	2010/5/21 Uchi 組み直し
*/
HMENU MRUFile::CreateMenu(HMENU hMenuPopUp, MenuDrawer* pMenuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	メニューキャプション
	const BOOL bMenuIcon = m_pShareData->m_common.m_window.m_bMenuIcon;
	FileNameManager::getInstance()->TransformFileName_MakeCache();

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	for (int i=0; i<m_recentFile.GetItemCount(); ++i) {
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= m_recentFile.GetViewCount()) {
			break;
		}
		
		// MRUリストの中にある開かれていないファイル

		const EditInfo* p = m_recentFile.GetItem(i);
		bool bFavorite = m_recentFile.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !bMenuIcon;
		FileNameManager::getInstance()->GetMenuFullLabel_MRU(szMenu, _countof(szMenu), p, -1, bFavoriteLabel, i, dcFont.GetHDC());

		// メニューに追加。
		pMenuDrawer->MyAppendMenu(
			hMenuPopUp,
			MF_BYPOSITION | MF_STRING,
			IDM_SELMRU + i,
			szMenu,
			_T(""),
			TRUE,
			bFavorite ? F_FAVORITE : -1
		);
	}
	return hMenuPopUp;
}

BOOL MRUFile::DestroyMenu(HMENU hMenuPopUp) const
{
	return ::DestroyMenu(hMenuPopUp);
}

/*!
	ファイル履歴の一覧を返す
	
	@param ppszMRU [out] 文字列へのポインタリストを格納する．
	最後の要素の次にはNULLが入る．
	予め呼び出す側で最大値+1の領域を確保しておくこと．
*/
std::vector<LPCTSTR> MRUFile::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for (int i=0; i<m_recentFile.GetItemCount(); ++i) {
		//「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= m_recentFile.GetViewCount()) {
			break;
		}
		ret.push_back(m_recentFile.GetItemText(i));
	}
	return ret;
}

// アイテム数を返す
int MRUFile::Length(void) const
{
	return m_recentFile.GetItemCount();
}

/*!
	ファイル履歴のクリア
*/
void MRUFile::ClearAll(void)
{
	m_recentFile.DeleteAllItem();
}

/*!
	ファイル情報の取得
	
	@param num [in] 履歴番号(0~)
	@param pfi [out] 構造体へのポインタ格納先
	
	@retval TRUE データが格納された
	@retval FALSE 正しくない番号が指定された．データは格納されなかった．
*/
bool MRUFile::GetEditInfo(int num, EditInfo* pfi) const
{
	const EditInfo*	p = m_recentFile.GetItem(num);
	if (!p) {
		return false;
	}

	*pfi = *p;

	return true;
}

/*!
	指定された名前のファイルがMRUリストに存在するか調べる。存在するならばファイル情報を返す。

	@param pszPath [in] 検索するファイル名
	@param pfi [out] データが見つかったときにファイル情報を格納する領域。
		呼び出し側で領域をあらかじめ用意する必要がある。
	@retval TRUE  ファイルが見つかった。pfiにファイル情報が格納されている。
	@retval FALSE 指定されたファイルはMRU Listに無い。

	@date 2001.12.26 CShareData::IsExistInMRUListから移動した。（YAZAKI）
*/
bool MRUFile::GetEditInfo(const TCHAR* pszPath, EditInfo* pfi) const
{
	const EditInfo*	p = m_recentFile.GetItem(m_recentFile.FindItemByPath(pszPath));
	if (!p) {
		return false;
	}

	*pfi = *p;

	return true;
}

/*!	@brief MRUリストへの登録

	@param pEditInfo [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。

	@date 2001.03.29 MIK リムーバブルディスク上のファイルを登録しないようにした。
	@date 2001.12.26 YAZAKI CShareData::AddMRUListから移動
*/
void MRUFile::Add(EditInfo* pEditInfo)
{
	//	ファイル名が無ければ無視
	if (!pEditInfo || pEditInfo->m_szPath[0] == L'\0') {
		return;
	}
	
	// すでに登録されている場合は、除外指定を無視する
	if (m_recentFile.FindItemByPath(pEditInfo->m_szPath) == -1) {
		int nSize = m_pShareData->m_history.m_aExceptMRU.size();
		for (int i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(m_pShareData->m_history.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pEditInfo->m_szPath,  szExceptMRU)) {
				return;
			}
		}
	}
	EditInfo tmpEditInfo = *pEditInfo;
	tmpEditInfo.m_bIsModified = FALSE; // 変更フラグを無効に

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFolder[_MAX_PATH + 1];	//	ドライブ＋フォルダ
	
	_tsplitpath(pEditInfo->m_szPath, szDrive, szDir, NULL, NULL);	//	ドライブとフォルダを取り出す。

	//	Jan.  10, 2006 genta USBメモリはRemovable mediaと認識されるようなので，
	//	一応無効化する．
	//	リムーバブルなら非登録？
	//if (/* 「リムーバブルなら登録しない」オン && */ ! IsLocalDrive(szDrive)) {
	//	return;
	//}

	//	szFolder作成
	_tcscpy(szFolder, szDrive);
	_tcscat(szFolder, szDir);

	//	Folderを、MRUFolderに登録
	MRUFolder mruFolder;
	mruFolder.Add(szFolder);

	m_recentFile.AppendItem(&tmpEditInfo);
	
	::SHAddToRecentDocs(SHARD_PATH, to_wchar(pEditInfo->m_szPath));
}

// EOF
