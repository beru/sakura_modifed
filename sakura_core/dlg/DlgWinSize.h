/*!	@file
	@brief ウィンドウの位置と大きさダイアログ
*/

#pragma once

#include "dlg/Dialog.h"
#include "env/CommonSetting.h"

/*!	@brief 位置と大きさの設定ダイアログ

	共通設定のウィンドウ設定で，ウィンドウ位置を指定するために補助的に
	使用されるダイアログボックス
*/
class DlgWinSize : public Dialog {
public:
	DlgWinSize();
	~DlgWinSize();
	INT_PTR DoModal(HINSTANCE, HWND, WinSizeMode&, WinSizeMode&, int&, RECT&);	// モーダルダイアログの表示

protected:

	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	int  GetData(void);
	void SetData(void);
	LPVOID GetHelpIdTable(void);

	void RenewItemState(void);

private:
	WinSizeMode	eSaveWinSize;	// ウィンドウサイズの保存: 0/デフォルト，1/継承，2/指定
	WinSizeMode	eSaveWinPos;	// ウィンドウ位置の保存: 0/デフォルト，1/継承，2/指定
	int			nWinSizeType;	// ウィンドウ表示方法: 0/標準，1/最大化，2/最小化
	RECT		rc;
};

