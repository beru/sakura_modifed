#pragma once

#include "doc/DocListener.h"

class EditWnd;

class MainStatusBar : public DocListenerEx {
public:
	// 作成・破棄
	MainStatusBar(EditWnd& owner);
	void CreateStatusBar();		// ステータスバー作成
	void DestroyStatusBar();	// ステータスバー破棄
	void SendStatusMessage2(const TCHAR* msg);	//	Jul. 9, 2005 genta メニューバー右端には出したくない長めのメッセージを出す
	/*!	SendStatusMessage2()が効き目があるかを予めチェック
		@note もしSendStatusMessage2()でステータスバー表示以外の処理を追加
		する場合にはここを変更しないと新しい場所への出力が行われない．
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective() const {
		return hwndStatusBar != NULL;
	}

	// 取得
	HWND GetStatusHwnd() const { return hwndStatusBar; }
	HWND GetProgressHwnd() const { return hwndProgressBar; }

	// 設定
	void SetStatusText(int nIndex, int nOption, const TCHAR* pszText);
private:
	EditWnd&	owner;
	HWND		hwndStatusBar;
	HWND		hwndProgressBar;
};

