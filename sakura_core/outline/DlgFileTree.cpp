/*!	@file
	@brief ファイルツリー設定ダイアログ
*/

#include "StdAfx.h"
#include "outline/DlgFileTree.h"
#include "outline/DlgFuncList.h"
#include "dlg/DlgOpenFile.h"
#include "dlg/DlgInput1.h"
#include "env/DocTypeManager.h"
#include "typeprop/ImpExpManager.h"
#include "DataProfile.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/os.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13300
	IDC_CHECK_LOADINI,      HIDC_CHECK_FILETREE_LOADINI,
	IDC_EDIT_DEFINI,        HIDC_EDIT_FILETREE_DEFINI,
	IDC_BUTTON_REF1,        HIDC_BUTTON_FILETREE_REF1,
	IDC_BUTTON_LOAD,        HIDC_BUTTON_FILETREE_LOAD,
	IDC_RADIO_GREP,         HIDC_RADIO_FILETREE_GREP,
	IDC_RADIO_FILE,         HIDC_RADIO_FILETREE_FILE,
	IDC_RADIO_FOLDER,       HIDC_RADIO_FILETREE_FOLDER,
	IDC_STATIC_PATH,        HIDC_STATIC_FILETREE_PATH,
	IDC_EDIT_PATH,          HIDC_EDIT_FILETREE_PATH,
	IDC_BUTTON_REF2,        HIDC_BUTTON_FILETREE_REF2,
	IDC_BUTTON_PATH_MENU,   HIDC_BUTTON_FILETREE_PATH_MENU,
	IDC_EDIT_LABEL,         HIDC_EDIT_FILETREE_LABEL,
	IDC_STATIC_FILE,        HIDC_STATIC_FILETREE_FILE,
	IDC_EDIT_FILE,          HIDC_EDIT_FILETREE_FILE,
	IDC_CHECK_HIDDEN,       HIDC_CHECK_FILETREE_HIDDEN,
	IDC_CHECK_READONLY,     HIDC_CHECK_FILETREE_READONLY,
	IDC_CHECK_SYSTEM,       HIDC_CHECK_FILETREE_SYSTEM,
	IDC_BUTTON_DELETE,      HIDC_BUTTON_FILETREE_DELETE,
	IDC_BUTTON_INSERT,      HIDC_BUTTON_FILETREE_INSERT,
	IDC_BUTTON_INSERT_A,    HIDC_BUTTON_FILETREE_INSERT_A,
	IDC_BUTTON_ADD,         HIDC_BUTTON_FILETREE_ADD,
	IDC_BUTTON_UPDATE,      HIDC_BUTTON_FILETREE_UPDATE,
	IDC_BUTTON_FILEADD,     HIDC_BUTTON_FILETREE_FILEADD,
	IDC_BUTTON_REPLACE,     HIDC_BUTTON_FILETREE_REPLACE,
	IDC_BUTTON_UP,          HIDC_BUTTON_FILETREE_UP,
	IDC_BUTTON_DOWN,        HIDC_BUTTON_FILETREE_DOWN,
	IDC_BUTTON_RIGHT,       HIDC_BUTTON_FILETREE_RIGHT,
	IDC_BUTTON_LEFT,        HIDC_BUTTON_FILETREE_LEFT,
	IDC_TREE_FL,            HIDC_TREE_FILETREE_FL,
	IDC_BUTTON_IMPORT,      HIDC_BUTTON_FILETREE_IMPORT,
	HIDC_BUTTON_EXPORT,     HIDC_BUTTON_FILETREE_EXPORT,
	IDOK,                   HIDC_FILETREE_IDOK,
	IDCANCEL,               HIDC_FILETREE_IDCANCEL,
	IDC_BUTTON_HELP,        HIDC_BUTTON_FILETREE_HELP,
//	IDC_STATIC,				-1,
	0, 0
};

DlgFileTree::DlgFileTree()
{
	bInMove = false;
}

/*! モーダルダイアログの表示
	lParam は DlgFuncList*
	入力はlParam経由で取得。
	結果の設定はDlgFileTreeが直接共通設定・タイプ別・設定ファイルに書き込みをして
	呼び出し元は、再表示で設定される
*/
INT_PTR DlgFileTree::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
	)
{
	pDlgFuncList = reinterpret_cast<DlgFuncList*>(lParam);
	nDocType = pDlgFuncList->nDocType;
	return Dialog::DoModal(hInstance, hwndParent, IDD_FILETREE, lParam);
}


// LS()を使用しているのですぐ使うこと
static TCHAR* GetFileTreeLabel(const FileTreeItem& item)
{
	const TCHAR* pszLabel;
	if (item.eFileTreeItemType != FileTreeItemType::Folder) {
		pszLabel = item.szLabelName;
		if (item.szLabelName[0] == _T('\0')) {
			pszLabel = item.szTargetPath;
			if (auto_strcmp(pszLabel, _T(".")) == 0
				|| auto_strcmp(pszLabel, _T(".\\")) == 0
				|| auto_strcmp(pszLabel, _T("./")) == 0
			) {
				pszLabel = LS(STR_FILETREE_CURDIR);
			}
		}
	}else {
		pszLabel = item.szLabelName;
		if (pszLabel[0] == _T('\0')) {
			pszLabel = _T("Folder");
		}
	}
	return const_cast<TCHAR*>(pszLabel);
}

// ダイアログデータの設定
void DlgFileTree::SetData()
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	std::vector<HTREEITEM> hParentTree;
	hParentTree.push_back(TVI_ROOT);
	HTREEITEM hSelect = NULL;
	aItemRemoveList.clear();
	TreeView_DeleteAllItems(hwndTree);
	bool bSaveShareData = (fileTreeSetting.szLoadProjectIni[0] == _T('\0'));
	for (int i=0; i<(int)fileTreeSetting.items.size(); ++i) {
		int nMaxCount = _countof(GetDllShareData().common.outline.fileTree.items);
		if (bSaveShareData && nMaxCount < i + 1) {
			::InfoMessage(GetHwnd(), LS(STR_FILETREE_MAXCOUNT), nMaxCount);
		}
		const FileTreeItem& item = fileTreeSetting.items[i];
		while (item.nDepth < (int)hParentTree.size() - 1) {
			hParentTree.resize(hParentTree.size() - 1);
		}
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParentTree.back();
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.lParam  = i;
		tvis.item.pszText = GetFileTreeLabel(item);
		tvis.item.cChildren = (item.eFileTreeItemType == FileTreeItemType::Folder) ? 1 : 0;
		HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
		if (item.eFileTreeItemType == FileTreeItemType::Folder) {
			hParentTree.push_back(hParent);
		}
		if (!hSelect) {
			hSelect = hParent;
		}
	}
	if (hSelect) {
		TreeView_SelectItem(hwndTree, hSelect);
	}
	int nIndex = (fileTreeSetting.items.size() == 0 ? -1: 0);
	SetDataItem(nIndex);
	ChangeEnableAddInsert();
	return;
}

void DlgFileTree::SetDataItem(int nItemIndex)
{
	HWND hwndDlg = GetHwnd();
	bool bDummy = false;
	if (nItemIndex < 0 || (int)fileTreeSetting.items.size() <= nItemIndex) {
		bDummy = true;
	}
	FileTreeItem itemDummy;
	const FileTreeItem& item = (bDummy ? itemDummy : fileTreeSetting.items[nItemIndex]);
	itemDummy.szTargetFile = _T("*.*");
	int nIDs[] ={IDC_RADIO_GREP, IDC_RADIO_FILE, IDC_RADIO_FOLDER};
	int nID1;
	int nID2, nID3;
	switch (item.eFileTreeItemType) {
	case FileTreeItemType::Grep:   nID1 = 0; nID2 = 1; nID3 = 2; break;
	case FileTreeItemType::File:   nID1 = 1; nID2 = 0; nID3 = 2; break;
	case FileTreeItemType::Folder: nID1 = 2; nID2 = 0; nID3 = 1; break;
	}
	::CheckDlgButton(hwndDlg, nIDs[nID1], TRUE);
	::CheckDlgButton(hwndDlg, nIDs[nID2], FALSE);
	::CheckDlgButton(hwndDlg, nIDs[nID3], FALSE);
	SetItemText(IDC_EDIT_PATH, item.szTargetPath);
	SetItemText(IDC_EDIT_LABEL, item.szLabelName);
	SetItemText(IDC_EDIT_FILE, item.szTargetFile);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_HIDDEN, item.bIgnoreHidden);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_READONLY, item.bIgnoreReadOnly);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_SYSTEM, item.bIgnoreSystem);
	ChangeEnableItemType();
	return;
}

void DlgFileTree::ChangeEnableItemType()
{
	bool bGrepEnable = false;
	bool bPathEnable = false;
	if (IsButtonChecked(IDC_RADIO_GREP)) { 
		bGrepEnable = true;
		bPathEnable = true;
	}else if (IsButtonChecked(IDC_RADIO_FILE)) { 
		bPathEnable = true;
	}
	EnableItem(IDC_STATIC_PATH, bPathEnable);
	EnableItem(IDC_EDIT_PATH, bPathEnable);
	EnableItem(IDC_BUTTON_REF2, bPathEnable);
	EnableItem(IDC_BUTTON_PATH_MENU, bPathEnable);
	EnableItem(IDC_STATIC_FILE, bGrepEnable);
	EnableItem(IDC_EDIT_FILE, bGrepEnable);
	EnableItem(IDC_CHECK_HIDDEN, bGrepEnable);
	EnableItem(IDC_CHECK_READONLY, bGrepEnable);
	EnableItem(IDC_CHECK_SYSTEM, bGrepEnable);
}

void DlgFileTree::ChangeEnableAddInsert()
{
	bool bSaveShareData = (fileTreeSetting.szLoadProjectIni[0] == _T('\0'));
	if (bSaveShareData) {
		int nCount = TreeView_GetCount(GetItemHwnd(IDC_TREE_FL));
		bool bEnable = true;
		int nMaxCount = _countof(GetDllShareData().common.outline.fileTree.items);
		if (nMaxCount < nCount) {
			bEnable = false;
		}
		EnableItem(IDC_BUTTON_ADD, bEnable);
		EnableItem(IDC_BUTTON_INSERT, bEnable);
		EnableItem(IDC_BUTTON_INSERT_A, bEnable);
	}
}

// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgFileTree::GetData()
{
	HWND hwndDlg = GetHwnd();
	FileTree* pFileTree;
	TypeConfig type;
	bool bTypeError = false;
	if (fileTreeSetting.eFileTreeSettingOrgType == FileTreeSettingFromType::Common) {
		pFileTree = &GetDllShareData().common.outline.fileTree;
	}else {
		if (!DocTypeManager().GetTypeConfig(TypeConfigNum(nDocType), type)) {
			bTypeError = true;
			pFileTree = NULL;
		}else {
			pFileTree = &type.fileTree;
		}
	}
	bool bSaveShareData = (fileTreeSetting.szLoadProjectIni[0] == _T('\0'));
	std::vector<FileTreeItem> items;
	if (!GetDataTree(items, TreeView_GetRoot(GetItemHwnd(IDC_TREE_FL)), 0, (bSaveShareData ? _countof(pFileTree->items) : 0))) {
		InfoMessage(GetHwnd(), LS(STR_FILETREE_MAXCOUNT));
	}
	if (pFileTree) {
		pFileTree->bProject = IsButtonChecked(IDC_CHECK_LOADINI);
		DlgItem_GetText(hwndDlg, IDC_EDIT_DEFINI, pFileTree->szProjectIni, pFileTree->szProjectIni.GetBufferCount());
		if (bSaveShareData) {
			pFileTree->nItemCount = (int)items.size();
			ASSERT_GE(_countof(pFileTree->items), pFileTree->nItemCount);
			for (int i=0; i<pFileTree->nItemCount; ++i) {
				pFileTree->items[i] = items[i];
			}
		}
		if (fileTreeSetting.eFileTreeSettingOrgType == FileTreeSettingFromType::Type) {
			DocTypeManager().SetTypeConfig(TypeConfigNum(nDocType), type);
		}
	}
	if (!bSaveShareData) {
		// 元のiniに保存
		ImpExpFileTree impExp(items);
		std::wstring strIni = to_wchar(fileTreeSetting.szLoadProjectIni);
		std::wstring strError;
		if (!impExp.Export(strIni, strError)) {
			ErrorMessage(hwndDlg, _T("%ls"), strError.c_str());
		}
	}
	return TRUE;
}

bool DlgFileTree::GetDataTree(
	std::vector<FileTreeItem>& data,
	HTREEITEM hItem,
	int nLevel,
	int nMaxCount
	)
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	for (HTREEITEM s=hItem; s!=NULL; s=TreeView_GetNextSibling(hwndTree, s)) {
		TV_ITEM	tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (!TreeView_GetItem(hwndTree, &tvi)) {
			return false;
		}
		if (0 < nMaxCount && nMaxCount <= (int)data.size()) {
			return false;
		}
		data.push_back(fileTreeSetting.items[tvi.lParam]);
		data.back().nDepth = nLevel;
		if (0 < tvi.cChildren) {
			HTREEITEM ts = TreeView_GetChild(hwndTree, s);
			if (ts) {
				if (!GetDataTree(data, ts, nLevel+1, nMaxCount)) {
					return false;
				}
			}
		}
	}
	return true;
}

int DlgFileTree::GetDataItem(FileTreeItem& item)
{
	item = FileTreeItem(); // 初期化
	bool bGrepEnable = false;
	bool bPathEnable = false;
	if (IsButtonChecked(IDC_RADIO_GREP)) {
		bGrepEnable = true;
		bPathEnable = true;
		item.eFileTreeItemType = FileTreeItemType::Grep;
	}else if (IsButtonChecked(IDC_RADIO_FILE)) { 
		bPathEnable = true;
		item.eFileTreeItemType = FileTreeItemType::File;
	}else {
		item.eFileTreeItemType = FileTreeItemType::Folder;
	}
	if (bPathEnable) {
		GetItemText(IDC_EDIT_PATH, item.szTargetPath, item.szTargetPath.GetBufferCount());
	}
	GetItemText(IDC_EDIT_LABEL, item.szLabelName, item.szLabelName.GetBufferCount());
	if (bGrepEnable) {
		GetItemText(IDC_EDIT_FILE, item.szTargetFile, item.szTargetFile.GetBufferCount());
		item.bIgnoreHidden = IsButtonChecked(IDC_CHECK_HIDDEN);
		item.bIgnoreReadOnly = IsButtonChecked(IDC_CHECK_READONLY);
		item.bIgnoreSystem = IsButtonChecked(IDC_CHECK_SYSTEM);	
	}
	return TRUE;
}

BOOL DlgFileTree::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	FileTreeItem item;

	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_DEFINI), fileTreeSetting.szDefaultProjectIni.GetBufferCount() - 1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_PATH), item.szTargetPath.GetBufferCount() - 1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_LABEL), item.szLabelName.GetBufferCount() - 1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_FILE), item.szTargetFile.GetBufferCount() - 1);

	FilePath path;
	pDlgFuncList->LoadFileTreeSetting(fileTreeSetting, path);
	SetDataInit();

	// 基底クラスメンバ
	return Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
}

void DlgFileTree::SetDataInit()
{
	HWND hwndDlg = GetHwnd();
	bool bEnableDefIni = true;
	if (fileTreeSetting.eFileTreeSettingLoadType != FileTreeSettingFromType::File) {
		int id;
		if (fileTreeSetting.eFileTreeSettingLoadType == FileTreeSettingFromType::Common) {
			id = STR_FILETREE_FROM_COMMON;
		}else {
			id = STR_FILETREE_FROM_TYPE;
		}
		std::tstring str = LS(id);
		if (fileTreeSetting.szLoadProjectIni[0] != _T('\0')) {
			str += _T("+");
			str += LS(F_FILE_TOPMENU);
		}
		SetItemText(IDC_STATIC_SETTFING_FROM, str.c_str() );
	}else {
		TCHAR szMsg[_MAX_PATH+200];
		const TCHAR* pFile = fileTreeSetting.szLoadProjectIni;
		TCHAR szFilePath[_MAX_PATH];
		TextWidthCalc calc(GetHwnd(), IDC_STATIC_SETTFING_FROM);
		RECT rc;
		::GetWindowRect(GetItemHwnd(IDC_STATIC_SETTFING_FROM), &rc);
		const int xWidth = calc.GetTextWidth(_T("x"));
		const int ctrlWidth = rc.right - rc.left;
		int nMaxCch = ctrlWidth / xWidth;
		FileNameManager::getInstance().GetTransformFileNameFast(pFile, szFilePath, _countof(szFilePath), calc.GetDC(), true, nMaxCch);
		wsprintf(szMsg, LS(STR_FILETREE_FROM_FILE), szFilePath);
		SetItemText(IDC_STATIC_SETTFING_FROM, szMsg);
		bEnableDefIni = false;
	}
	EnableItem(IDC_BUTTON_LOAD, bEnableDefIni);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_LOADINI, fileTreeSetting.bProject);
	SetItemText(IDC_EDIT_DEFINI, fileTreeSetting.szDefaultProjectIni); 
}

HTREEITEM DlgFileTree::InsertTreeItem(
	FileTreeItem& item,
	HTREEITEM htiParent,
	HTREEITEM htiInsert
	)
{
	int nlParam;
	if (aItemRemoveList.empty()) {
		nlParam = fileTreeSetting.items.size();
		fileTreeSetting.items.push_back(item);
	}else {
		// 削除リストから復活させる
		nlParam = aItemRemoveList.back();
		aItemRemoveList.pop_back();
		fileTreeSetting.items[nlParam] = item;
	}
	TV_INSERTSTRUCT	tvis;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvis.hParent = htiParent;
	tvis.hInsertAfter = htiInsert;
	tvis.item.pszText = GetFileTreeLabel(item);
	tvis.item.lParam = nlParam;
	tvis.item.cChildren = (item.eFileTreeItemType == FileTreeItemType::Folder) ? 1 : 0;
	return TreeView_InsertItem(GetItemHwnd(IDC_TREE_FL), &tvis);
}


// ツリーのコピー
//		fChildがtrueの時はdstの子としてコピー, そうでなければdstの兄弟としてdstの後ろにコピー
//		fOnryOneがtrueの時は1つだけコピー（子があったらコピー）
static HTREEITEM FileTreeCopy(
	HWND hwndTree,
	HTREEITEM dst,
	HTREEITEM src,
	bool fChild,
	bool fOnryOne
	)
{
	HTREEITEM		s;
	HTREEITEM		ts;
	HTREEITEM		td = NULL;
	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	int				n = 0;
	TCHAR			szLabel[_MAX_PATH];

	for (s=src; s; s=fOnryOne ? NULL:TreeView_GetNextSibling(hwndTree, s)) {
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		tvi.pszText = szLabel;
		tvi.cchTextMax = _countof(szLabel);
		if (!TreeView_GetItem(hwndTree, &tvi)) {
			// Error
			break;
		}
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		if (fChild || n != 0) {
			// dstの子供として作成
			tvis.hParent = dst;
			tvis.hInsertAfter = TVI_LAST;
		}else {
			//	dstの兄弟として作成
			tvis.hParent = TreeView_GetParent(hwndTree, dst);
			tvis.hInsertAfter = dst;
		}
		tvis.item.pszText = szLabel;
		tvis.item.lParam = tvi.lParam;
		tvis.item.cChildren = tvi.cChildren;
		td = TreeView_InsertItem(hwndTree, &tvis);	//	Itemの作成

		if (tvi.cChildren) {
			ts = TreeView_GetChild(hwndTree, s);	//	子の取得
			if (ts) {
				FileTreeCopy(hwndTree, td, ts, true, false);
			}
			// 展開
			if (tvi.state & TVIS_EXPANDEDONCE) {
				TreeView_Expand(hwndTree, td, TVE_EXPAND);
			}
		}
		++n;
	}

	return td;
}


BOOL DlgFileTree::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_REF1:
		{
			SelectFile(GetHwnd(), GetItemHwnd(IDC_EDIT_DEFINI), _T("*.ini"), true);
		}
		return TRUE;
	case IDC_BUTTON_LOAD:
		{
			GetItemText(IDC_EDIT_DEFINI, fileTreeSetting.szDefaultProjectIni, fileTreeSetting.szDefaultProjectIni.GetBufferCount() );
			if (fileTreeSetting.szDefaultProjectIni[0] != _T('\0')) {
				DataProfile profile;
				profile.SetReadingMode();
				const TCHAR* pszIniFileName;
				TCHAR szDir[_MAX_PATH * 2];
				if (_IS_REL_PATH(fileTreeSetting.szDefaultProjectIni)) {
					// sakura.iniからの相対パス
					GetInidirOrExedir(szDir, fileTreeSetting.szDefaultProjectIni);
					pszIniFileName = szDir;
				}else {
					pszIniFileName = fileTreeSetting.szDefaultProjectIni;
				}
				if (profile.ReadProfile(pszIniFileName)) {
					ImpExpFileTree::IO_FileTreeIni(profile, fileTreeSetting.items);
					fileTreeSetting.szLoadProjectIni = pszIniFileName;
				}
			}
			SetDataInit();
			SetData();
		}
		return TRUE;
	case IDC_RADIO_GREP:
	case IDC_RADIO_FILE:
	case IDC_RADIO_FOLDER:
		{
			ChangeEnableItemType();
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			BOOL bEnableUpdate = TRUE;
			if (htiItem && !IsButtonChecked(IDC_RADIO_FOLDER)) {
				TV_ITEM tvi;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (TreeView_GetItem(hwndTree, &tvi)
					&& fileTreeSetting.items[tvi.lParam].eFileTreeItemType ==  FileTreeItemType::Folder
					&& TreeView_GetChild(hwndTree, htiItem)
				) {
					// [Folder]以外を子がいるFolderに上書きするの禁止
					bEnableUpdate = FALSE;
				}
			}
			EnableWindow( GetItemHwnd(IDC_BUTTON_UPDATE), bEnableUpdate );
		}
		return TRUE;
	case IDC_BUTTON_REF2:
		{
			HWND hwndDlg = GetHwnd();
			if (IsButtonChecked(IDC_RADIO_GREP)) {
				// folder == RADIO_GREP
				TCHAR szDir[MAX_PATH];
				GetItemText(IDC_EDIT_PATH, szDir, _countof(szDir) );
				if (SelectDir(hwndDlg, LS(STR_DLGGREP1), szDir, szDir)) {
					SetItemText(IDC_EDIT_PATH, szDir);
				}
			}else {
				// file == RADIO_FILE
				DlgOpenFile dlg;
				TCHAR szDir[_MAX_PATH];
				GetInidir(szDir);
				dlg.Create( G_AppInstance(), hwndDlg, _T("*.*"), szDir,
					std::vector<LPCTSTR>(), std::vector<LPCTSTR>() );
				TCHAR szFile[_MAX_PATH];
				if (dlg.DoModal_GetOpenFileName(szFile)) {
					NativeT memFile = szFile;
					memFile.ReplaceT(_T("%"), _T("%%"));
					SetItemText(IDC_EDIT_PATH, memFile.GetStringPtr() );
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_PATH_MENU:
		{
			const int MENU_ROOT = 0x100;
			const int MENU_MYDOC = 0x101;
			const int MENU_MYMUSIC = 0x102;
			const int MENU_MYVIDEO = 0x103;
			const int MENU_DESK = 0x104;
			const int MENU_TEMP = 0x105;
			const int MENU_SAKURA = 0x106;
			const int MENU_SAKURADATA = 0x107;
			HMENU hMenu = ::CreatePopupMenu();
			int iPos = 0;
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_ROOT, LS(STR_FILETREE_MENU_ROOT) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYDOC, LS(STR_FILETREE_MENU_MYDOC) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYMUSIC, LS(STR_FILETREE_MENU_MYMUSIC) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYVIDEO, LS(STR_FILETREE_MENU_MYVIDEO) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_DESK, LS(STR_FILETREE_MENU_DESK) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_TEMP, LS(STR_FILETREE_MENU_TEMP) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_SAKURA, LS(STR_FILETREE_MENU_SAKURA) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_SAKURADATA, LS(STR_FILETREE_MENU_SAKURADATA) );
			POINT pt;
			RECT rc;
			::GetWindowRect( GetItemHwnd(IDC_BUTTON_PATH_MENU), &rc );
			pt.x = rc.left;
			pt.y = rc.bottom;
			RECT rcWork;
			GetMonitorWorkRect(pt, &rcWork);	// モニタのワークエリア
			int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										( pt.x > rcWork.left )? pt.x: rcWork.left,
										( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
										0, GetHwnd(), NULL);
			::DestroyMenu( hMenu );
			if (nId != 0) {
				int index = nId - MENU_ROOT;
				const TCHAR* pszPaths[] = { _T("<iniroot>"), _T("%MYDOC%"), _T("%MYMUSIC%"), _T("%MYVIDEO%"),
					_T("%DESKTOP%"), _T("%TEMP%"), _T("%SAKURA%"), _T("%SAKURADATA%") };
				EditCtl_ReplaceSel(GetItemHwnd(IDC_EDIT_PATH), pszPaths[index]);
			}
		}
		return TRUE;
	case IDC_BUTTON_DELETE:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if (htiItem) {
				if (TreeView_GetChild(hwndTree, htiItem)
					&& ::MYMESSAGEBOX(GetHwnd(), MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMMAINMENU_DEL)) == IDCANCEL
				) {
					return TRUE;
				}
				HTREEITEM htiTemp = TreeView_GetNextSibling(hwndTree, htiItem);
				if (htiTemp) {
					// 末尾ならば、前を取る
					htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
				}
				TreeView_DeleteItem(hwndTree, htiItem);
				if (htiTemp) {
					TreeView_SelectItem(hwndTree, htiTemp);
				}
			}
			ChangeEnableAddInsert();
		}
		return TRUE;
	case IDC_BUTTON_INSERT:
	case IDC_BUTTON_INSERT_A:
	case IDC_BUTTON_ADD:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiInsert = NULL;
			HTREEITEM htiParent = NULL;
			// 挿入位置検索
			HTREEITEM htiTemp = TreeView_GetSelection(hwndTree);
			TV_ITEM tvi;
			if (!htiTemp) {
			}else if (wID == IDC_BUTTON_ADD) {
				// 追加
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiTemp;
				if (TreeView_GetItem(hwndTree, &tvi)) {
					if (fileTreeSetting.items[tvi.lParam].eFileTreeItemType ==  FileTreeItemType::Folder) {
						// ノード
						htiParent = htiTemp;
					}else {
						// 子を付けられないので親に付ける（選択アイテムの下に付く）
						htiParent = TreeView_GetParent(hwndTree, htiTemp);
					}
				}
			}else if (wID == IDC_BUTTON_INSERT_A) {
				// ノード挿入、挿入(下)
				// 追加先を探る
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiTemp;
				if (TreeView_GetItem(hwndTree, &tvi)) {
					if (fileTreeSetting.items[tvi.lParam].eFileTreeItemType ==  FileTreeItemType::Folder) {
						// ノード
						htiParent = htiTemp;
						htiInsert = TVI_FIRST;
					}else {
						// 子を付けられないので親に付ける（選択アイテムの下に付く）
						htiParent = TreeView_GetParent(hwndTree, htiTemp);
						htiInsert = htiTemp;
					}
				}
			}else {
				assert(wID == IDC_BUTTON_INSERT);
				// 挿入(上)
				// 挿入先を探る
				htiParent = TreeView_GetParent(hwndTree, htiTemp);
				if (!htiParent) {
					htiInsert = TVI_FIRST;
				}else {
					htiInsert = TreeView_GetPrevSibling(hwndTree, htiTemp);
					if (!htiInsert) {
						htiInsert = TVI_FIRST;
					}
				}
			}
			if (!htiParent) {
				htiParent = TVI_ROOT;
			}
			if (!htiInsert) {
				htiInsert = TVI_LAST;
			}
			FileTreeItem item;
			GetDataItem(item);
			HTREEITEM htiItem = InsertTreeItem(item, htiParent, htiInsert);
			// 展開
			if (htiParent != TVI_ROOT) {
				TreeView_Expand(hwndTree, htiParent, TVE_EXPAND);
			}
			TreeView_SelectItem(hwndTree, htiItem);
			ChangeEnableAddInsert();
		}
		return TRUE;
	case IDC_BUTTON_UPDATE:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiSelect = TreeView_GetSelection(hwndTree);
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiSelect;
			if (TreeView_GetItem(hwndTree, &tvi)) {
				FileTreeItem item;
				GetDataItem(item);
				fileTreeSetting.items[tvi.lParam] = item;
				
				tvi.mask = TVIF_HANDLE | TVIF_TEXT;
				tvi.hItem = htiSelect;
				tvi.pszText = GetFileTreeLabel(item);
				TreeView_SetItem(hwndTree, &tvi);
			}
		}
		return TRUE;
	case IDC_BUTTON_FILEADD:
		{
			DlgOpenFile dlg;
			LoadInfo loadInfo;
			std::vector<std::tstring> aFileNames;
			dlg.Create( G_AppInstance(), GetHwnd(), _T("*.*"), _T("."),
				std::vector<LPCTSTR>(), std::vector<LPCTSTR>() );
			if (dlg.DoModalOpenDlg(&loadInfo, &aFileNames, false)) {
				if (0 < aFileNames.size()) {
					HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
					HTREEITEM htiInsert = NULL;
					HTREEITEM htiParent = NULL;
					// 挿入位置検索
					HTREEITEM htiTemp = TreeView_GetSelection(hwndTree);
					TV_ITEM tvi;
					if (!htiTemp) {
					}else {
						// ノード挿入、挿入(下)
						// 追加先を探る
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						tvi.hItem = htiTemp;
						if (TreeView_GetItem(hwndTree, &tvi)) {
							if (fileTreeSetting.items[tvi.lParam].eFileTreeItemType ==  FileTreeItemType::Folder) {
								// ノード
								htiParent = htiTemp;
								htiInsert = TVI_FIRST;
							}else {
								// 子を付けられないので親に付ける（選択アイテムの下に付く）
								htiParent = TreeView_GetParent(hwndTree, htiTemp);
							}
						}
					}
					if (!htiParent) {
						htiParent = TVI_ROOT;
					}
					if (!htiInsert) {
						htiInsert = TVI_LAST;
					}
					HTREEITEM htiItemFirst = NULL;
					for (int i=0; i<(int)aFileNames.size(); ++i) {
						NativeT memFile = aFileNames[i].c_str();
						memFile.ReplaceT(_T("%"), _T("%%"));
						FileTreeItem item;
						item.eFileTreeItemType = FileTreeItemType::File;
						item.szTargetPath = memFile.GetStringPtr();
						item.szLabelName = GetFileTitlePointer(aFileNames[i].c_str());
						htiInsert = InsertTreeItem(item, htiParent, htiInsert);
						if (!htiItemFirst) {
							htiItemFirst = htiInsert;
						}
					}
					// 展開
					if (htiParent != TVI_ROOT) {
						TreeView_Expand(hwndTree, htiParent, TVE_EXPAND);
					}
					TreeView_SelectItem(hwndTree, htiItemFirst);
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_REPLACE:
		{
			DlgInput1 dlgInput;
			std::tstring strMsg = LS(STR_FILETREE_REPLACE_PATH_FROM);
			std::tstring strTitle = LS(STR_DLGREPLC_STR);
			TCHAR szPathFrom[_MAX_PATH];
			szPathFrom[0] = _T('\0');
			if (dlgInput.DoModal(G_AppInstance(), GetHwnd(), strTitle.c_str(), strMsg.c_str(), _countof(szPathFrom), szPathFrom)) {
				TCHAR szPathTo[_MAX_PATH];
				szPathTo[0] = _T('\0');
				strMsg = LS(STR_FILETREE_REPLACE_PATH_TO);
				if (dlgInput.DoModal( G_AppInstance(), GetHwnd(), strTitle.c_str(), strMsg.c_str(), _countof(szPathTo), szPathTo)) {
					int nItemsCount = (int)fileTreeSetting.items.size();
					for (int i=0; i<nItemsCount; ++i) {
						FileTreeItem& item =  fileTreeSetting.items[i];
						NativeT str(item.szTargetPath);
						str.Replace(szPathFrom, szPathTo);
						if (str.GetStringLength() < (int)item.szTargetPath.GetBufferCount()) {
							item.szTargetPath = str.GetStringPtr();
						}
					}
				}
				HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
				HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
				if (htiItem) {
					TV_ITEM tvi;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = htiItem;
					if (TreeView_GetItem( hwndTree, &tvi)) {
						SetDataItem(tvi.lParam);
					}
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_UP:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if (!htiItem) {
				break;
			}
			HTREEITEM htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
			if (!htiTemp) {
				// そのエリアで最初
				break;
			}
			// コピー
			bInMove = true;
			FileTreeCopy(hwndTree, htiItem, htiTemp, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiTemp);
			bInMove = false;
		}
		return TRUE;
	case IDC_BUTTON_DOWN:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if (!htiItem) {
				break;
			}
			HTREEITEM htiTemp = TreeView_GetNextSibling(hwndTree, htiItem);
			if (!htiTemp) {
				// そのエリアで最後
				break;
			}
			// コピー
			bInMove = true;
			FileTreeCopy(hwndTree, htiTemp, htiItem, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiItem);
			bInMove = false;
			// 選択
			htiItem = TreeView_GetNextSibling(hwndTree, htiTemp);
			if (htiItem) {
				TreeView_SelectItem(hwndTree, htiItem);
			}
		}
		return TRUE;
	case IDC_BUTTON_RIGHT:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if (!htiItem) {
				break;
			}
			HTREEITEM htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
			if (!htiTemp) {
				// そのエリアで最初
				break;
			}
			// ノード確認
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiTemp;
			if (!TreeView_GetItem(hwndTree, &tvi)) {
				// エラー
				break;
			}
			if (fileTreeSetting.items[tvi.lParam].eFileTreeItemType == FileTreeItemType::Folder) {
				// 直前がノード
				// コピー
				bInMove = true;
				HTREEITEM htiTemp2 = FileTreeCopy(hwndTree, htiTemp, htiItem, true, true);
				// 削除
				TreeView_DeleteItem(hwndTree, htiItem);
				bInMove = false;
				// 選択
				TreeView_SelectItem(hwndTree, htiTemp2);
			}
		}
		return TRUE;
	case IDC_BUTTON_LEFT:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if (!htiItem) {
				break;
			}
			HTREEITEM htiParent = TreeView_GetParent(hwndTree, htiItem);
			if (!htiParent) {
				// Root
				break;
			}
			// コピー
			bInMove = true;
			HTREEITEM htiTemp2 = FileTreeCopy(hwndTree, htiParent, htiItem, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiItem);
			bInMove = false;
			// 選択
			TreeView_SelectItem(hwndTree, htiTemp2);
		}
		return TRUE;
	case IDC_BUTTON_IMPORT:
		{
			ImpExpFileTree impExp(fileTreeSetting.items);
			impExp.ImportUI(G_AppInstance(), GetHwnd());
			SetData();
		}
		return TRUE;
	case IDC_BUTTON_EXPORT:
		{
			std::vector<FileTreeItem> items;
			GetDataTree(items, TreeView_GetRoot(GetItemHwnd(IDC_TREE_FL)), 0, 0);
			ImpExpFileTree impExp(items);
			impExp.ExportUI(G_AppInstance(), GetHwnd());
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_FILETREE) );
		return TRUE;

	case IDOK:
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

BOOL DlgFileTree::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	TV_DISPINFO* ptdi = (TV_DISPINFO*)lParam;
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	HTREEITEM htiItem;

	switch (pNMHDR->code) {
	case TVN_DELETEITEM:
		if (!bInMove
			&& pNMHDR->hwndFrom == hwndTree
			&& (htiItem = TreeView_GetSelection( hwndTree ))
		) {
			//付属情報を削除
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			if (TreeView_GetItem( hwndTree, &tvi)) {
				// リストから削除する代わりに番号を覚えて後で再利用
				aItemRemoveList.push_back(tvi.lParam);
			}
		}
		break;
	case TVN_SELCHANGED:
		if (!bInMove
			&& pNMHDR->hwndFrom == hwndTree
		) {
			htiItem = TreeView_GetSelection( hwndTree );
			if (htiItem) {
				TV_ITEM tvi;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (TreeView_GetItem( hwndTree, &tvi)) {
					SetDataItem(tvi.lParam);
				}
			}else {
				SetDataItem(-1);
			}
		}
	}
	// 基底クラスメンバ
	return Dialog::OnNotify(wParam, lParam);
}

LPVOID DlgFileTree::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


