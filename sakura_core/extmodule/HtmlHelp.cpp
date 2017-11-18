/*!	@file
	@brief HtmpHelp���I���[�h
	
	HTML Help �R���|�[�l���g�ւ̓��I�A�N�Z�X�N���X
*/
#include "StdAfx.h"
#include "HtmlHelp.h"

HtmlHelpDll::~HtmlHelpDll(void)
{
}

/*!
	HTML Help �̃t�@�C������n��
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


