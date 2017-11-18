/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス
*/
#pragma once

#include "dlg/Dialog.h"

class DlgFileUpdateQuery : public Dialog {
public:
	DlgFileUpdateQuery(const TCHAR* filename, bool IsModified)
		:
		pFilename(filename),
		bModified(IsModified)
	{
	}
	virtual BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int);

private:
	const TCHAR* pFilename;
	bool bModified;
};

