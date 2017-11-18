/*!	@file
	@brief ファイルツリー設定
*/

#pragma once

#include "dlg/Dialog.h"
#include "outline/DlgFuncList.h"

/*!
	@brief ファイルツリー設定ダイアログ
*/
class DlgFileTree : public Dialog
{
public:
	DlgFileTree();

	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);

private:
	BOOL	OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM, LPARAM);
	LPVOID	GetHelpIdTable();
	void	SetData();
	int		GetData();

	void	SetDataInit();
	void	SetDataItem(int);
	void	ChangeEnableItemType();
	void	ChangeEnableAddInsert();
	int		GetDataItem(FileTreeItem&);
	bool	GetDataTree(std::vector<FileTreeItem>&, HTREEITEM, int, int);
	HTREEITEM InsertTreeItem(FileTreeItem&, HTREEITEM, HTREEITEM);

private:
	DlgFuncList*		pDlgFuncList;
	FileTreeSetting		fileTreeSetting;
	std::vector<int>	aItemRemoveList;
	int					nlParamCount;
	int					nDocType;

	int					bInMove;
};

