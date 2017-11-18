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


/*!	ファイルオープンダイアログボックス

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
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
	bool DoModal_GetOpenFileName(TCHAR*, bool bSetCurDir = false);	// 開くダイアログ モーダルダイアログの表示	// 2002/08/21 moca	引数追加
	bool DoModal_GetSaveFileName(TCHAR*, bool bSetCurDir = false);	// 保存ダイアログ モーダルダイアログの表示	// 2002/08/21 30,2002 moca	引数追加
	bool DoModalOpenDlg(LoadInfo* pLoadInfo, std::vector<std::tstring>*, bool bOptions = true);	// 開くダイアグ モーダルダイアログの表示
	bool DoModalSaveDlg(SaveInfo* pSaveInfo, bool bSimpleMode);		// 保存ダイアログ モーダルダイアログの表示

protected:
	DlgOpenFileMem*	mem;

	/*
	||  実装ヘルパ関数
	*/

	// May 29, 2004 genta エラー処理をまとめる (advised by MIK)
	void DlgOpenFail(void);

	// 2005.11.02 ryoji OS バージョン対応の OPENFILENAME 初期化用関数
	void InitOfn(OPENFILENAMEZ*);

	// 2005.11.02 ryoji 初期レイアウト設定処理
	static void InitLayout(HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl);

	// 2006.09.03 Moca ファイルダイアログのエラー回避
	// リトライ機能付き GetOpenFileName
	bool _GetOpenFileNameRecover(OPENFILENAMEZ* ofn);
	// リトライ機能付き GetOpenFileName
	bool GetSaveFileNameRecover(OPENFILENAMEZ* ofn);

	friend UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	DISALLOW_COPY_AND_ASSIGN(DlgOpenFile);
};



