/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス
*/

#include "StdAfx.h"
#include "dlg/DlgFileUpdateQuery.h"
#include "sakura_rc.h"

BOOL DlgFileUpdateQuery::OnInitDialog(
	HWND hWnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	::DlgItem_SetText(hWnd, IDC_UPDATEDFILENAME, pFilename);
	::DlgItem_SetText(hWnd, IDC_QUERYRELOADMSG, bModified ?
		LS(STR_ERR_DLGUPQRY1):LS(STR_ERR_DLGUPQRY2));

	return Dialog::OnInitDialog(hWnd, wParam, lParam);
}

/*!
	ボタンが押されたときの動作
*/
BOOL DlgFileUpdateQuery::OnBnClicked(int id)
{
	int result;
	switch (id) {
	case IDC_BTN_RELOAD: // 再読込
		result = 1;
		break;
	case IDC_BTN_CLOSE: // 閉じる
		result = 0;
		break;
	case IDC_BTN_NOTIFYONLY: // 以後通知メッセージのみ
		result = 2;
		break;
	case IDC_BTN_NOSUPERVISION: // 以後更新を監視しない
		result = 3;
		break;
	case IDC_BTN_AUTOLOAD:		// 以後未編集で再ロード
		result = 4;
		break;
	default:
		result = 0;
		break;
	}
	CloseDialog(result);

	return 0;
}

