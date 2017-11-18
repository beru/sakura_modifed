/*!	@file
	@brief HtmpHelp動的ロード

	HTML Help コンポーネントへの動的アクセスクラス
*/
#pragma once

#include "DllHandler.h"


/*!
	@brief HtmpHelp動的ロード

	HTMLヘルプコンポーネントの動的ロードをサポートするクラス
*/
class HtmlHelpDll : public DllImp {
public:
	HtmlHelpDll() {}
	virtual ~HtmlHelpDll();

	// HtmlHelp のEntry Point
	typedef HWND (WINAPI* Proc_HtmlHelp)(HWND, LPCTSTR, UINT, DWORD_PTR);
	Proc_HtmlHelp HtmlHelp;

protected:
	virtual bool InitDllImp();
	virtual LPCTSTR GetDllNameImp(int nIndex);
};

