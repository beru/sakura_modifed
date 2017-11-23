/*!	@file
	@brief ファイルオープンダイアログボックス
*/
#pragma once

#include <CommDlg.h>
#include <vector>
#include "util/design_template.h"
#include "Eol.h"
#include "basis/MyString.h"
#include "dlg/Dialog.h"

struct LoadInfo;	// doc/DocListener.h
struct SaveInfo;	// doc/DocListener.h
struct OPENFILENAMEZ;
class DlgOpenFileMem;


/*!	ファイルオープンダイアログボックス */
class DlgOpenFile
{
public:
	// コンストラクタ・デストラクタ
	DlgOpenFile();
	~DlgOpenFile();
	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU			= std::vector<LPCTSTR>(),
		const std::vector<LPCTSTR>& vOPENFOLDER		= std::vector<LPCTSTR>()
	);

	// 操作
	bool DoModal_GetOpenFileName(TCHAR*, bool bSetCurDir = false);	// 開くダイアログ モーダルダイアログの表示
	bool DoModal_GetSaveFileName(TCHAR*, bool bSetCurDir = false);	// 保存ダイアログ モーダルダイアログの表示
	bool DoModalOpenDlg(LoadInfo* pLoadInfo, std::vector<std::tstring>*, bool bOptions = true);	// 開くダイアグ モーダルダイアログの表示
	bool DoModalSaveDlg(SaveInfo* pSaveInfo, bool bSimpleMode);		// 保存ダイアログ モーダルダイアログの表示

protected:
	DlgOpenFileMem*	mem;

	/*
	||  実装ヘルパ関数
	*/

	void DlgOpenFail(void);

	// OS バージョン対応の OPENFILENAME 初期化用関数
	void InitOfn(OPENFILENAMEZ*);

	// 初期レイアウト設定処理
	static void InitLayout(HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl);

	// リトライ機能付き GetOpenFileName
	bool _GetOpenFileNameRecover(OPENFILENAMEZ* ofn);
	// リトライ機能付き GetOpenFileName
	bool GetSaveFileNameRecover(OPENFILENAMEZ* ofn);

	friend UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	DISALLOW_COPY_AND_ASSIGN(DlgOpenFile);
};



