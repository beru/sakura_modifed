/*!	@file
	@brief HtmpHelp動的ロード
	
	HTML Help コンポーネントへの動的アクセスクラス
*/
#include "StdAfx.h"
#include "HtmlHelp.h"

HtmlHelpDll::~HtmlHelpDll(void)
{
}

/*!
	HTML Help のファイル名を渡す
*/
LPCTSTR HtmlHelpDll::GetDllNameImp(int nIndex)
{
	return _T("HHCTRL.OCX");
}

bool HtmlHelpDll::InitDllImp()
{
	HtmlHelp = (Proc_HtmlHelp) ::GetProcAddress(
		GetInstance(),
		"HtmlHelpW"
	);
	return HtmlHelp != nullptr; 
}


