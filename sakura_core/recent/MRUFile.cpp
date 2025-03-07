#include "StdAfx.h"
#include <ShlObj.h>
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/FileNameManager.h"
#include "uiparts/MenuDrawer.h"	//	これでいいのか？
#include "window/EditWnd.h"
#include "util/string_ex2.h"
#include "util/window.h"

MruFile::MruFile()
{
	//	初期化。
	pShareData = &GetDllShareData();
}

MruFile::~MruFile()
{
	recentFile.Terminate();
}

/*!
	ファイル履歴メニューの作成
	
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@return 生成したメニューのハンドル
*/
HMENU MruFile::CreateMenu(MenuDrawer& menuDrawer) const
{
	//	空メニューを作る
	HMENU hMenuPopUp = ::CreatePopupMenu();
	return CreateMenu(hMenuPopUp, menuDrawer);
}
/*!
	ファイル履歴メニューの作成
	
	@param 追加するメニューのハンドル
	@param pMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@return 生成したメニューのハンドル
*/
HMENU MruFile::CreateMenu(HMENU hMenuPopUp, MenuDrawer& menuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	メニューキャプション
	const BOOL bMenuIcon = pShareData->common.window.bMenuIcon;
	FileNameManager::getInstance().TransformFileName_MakeCache();

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	for (size_t i=0; i<recentFile.GetItemCount(); ++i) {
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= recentFile.GetViewCount()) {
			break;
		}
		
		// MRUリストの中にある開かれていないファイル

		const EditInfo* p = recentFile.GetItem(i);
		bool bFavorite = recentFile.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !bMenuIcon;
		FileNameManager::getInstance().GetMenuFullLabel_MRU(szMenu, _countof(szMenu), p, -1, bFavoriteLabel, i, dcFont.GetHDC());

		// メニューに追加。
		menuDrawer.MyAppendMenu(
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

BOOL MruFile::DestroyMenu(HMENU hMenuPopUp) const
{
	return ::DestroyMenu(hMenuPopUp);
}

/*!
	ファイル履歴の一覧を返す
	
	@param ppszMRU [out] 文字列へのポインタリストを格納する．
	最後の要素の次にはNULLが入る．
	予め呼び出す側で最大値+1の領域を確保しておくこと．
*/
std::vector<LPCTSTR> MruFile::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for (size_t i=0; i<recentFile.GetItemCount(); ++i) {
		//「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if (i >= recentFile.GetViewCount()) {
			break;
		}
		ret.push_back(recentFile.GetItemText(i));
	}
	return ret;
}

// アイテム数を返す
size_t MruFile::Length(void) const
{
	return recentFile.GetItemCount();
}

/*!
	ファイル履歴のクリア
*/
void MruFile::ClearAll(void)
{
	recentFile.DeleteAllItem();
}

/*!
	ファイル情報の取得
	
	@param num [in] 履歴番号(0~)
	@param pfi [out] 構造体へのポインタ格納先
	
	@retval TRUE データが格納された
	@retval FALSE 正しくない番号が指定された．データは格納されなかった．
*/
bool MruFile::GetEditInfo(size_t num, EditInfo* pfi) const
{
	const EditInfo*	p = recentFile.GetItem(num);
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
*/
bool MruFile::GetEditInfo(const TCHAR* pszPath, EditInfo* pfi) const
{
	const EditInfo*	p = recentFile.GetItem(recentFile.FindItemByPath(pszPath));
	if (!p) {
		return false;
	}

	*pfi = *p;

	return true;
}

/*!	@brief MRUリストへの登録

	@param pEditInfo [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。
*/
void MruFile::Add(EditInfo* pEditInfo)
{
	//	ファイル名が無ければ無視
	if (!pEditInfo || pEditInfo->szPath[0] == L'\0') {
		return;
	}
	
	// すでに登録されている場合は、除外指定を無視する
	if (recentFile.FindItemByPath(pEditInfo->szPath) == -1) {
		size_t nSize = pShareData->history.aExceptMRU.size();
		for (size_t i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(pShareData->history.aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pEditInfo->szPath,  szExceptMRU)) {
				return;
			}
		}
	}
	EditInfo tmpEditInfo = *pEditInfo;
	tmpEditInfo.bIsModified = false; // 変更フラグを無効に

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFolder[_MAX_PATH + 1];	//	ドライブ＋フォルダ
	
	_tsplitpath(pEditInfo->szPath, szDrive, szDir, NULL, NULL);	//	ドライブとフォルダを取り出す。

	//	USBメモリはRemovable mediaと認識されるようなので，
	//	一応無効化する．
	//	リムーバブルなら非登録？
	//if (/* 「リムーバブルなら登録しない」オン && */ ! IsLocalDrive(szDrive)) {
	//	return;
	//}

	//	szFolder作成
	_tcscpy(szFolder, szDrive);
	_tcscat(szFolder, szDir);

	//	Folderを、MruFolderに登録
	MruFolder mruFolder;
	mruFolder.Add(szFolder);

	recentFile.AppendItem(&tmpEditInfo);
	
	::SHAddToRecentDocs(SHARD_PATH, to_wchar(pEditInfo->szPath));
}

