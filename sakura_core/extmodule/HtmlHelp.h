/*!	@file
	@brief HtmpHelp���I���[�h

	HTML Help �R���|�[�l���g�ւ̓��I�A�N�Z�X�N���X
*/
#pragma once

#include "DllHandler.h"


/*!
	@brief HtmpHelp���I���[�h

	HTML�w���v�R���|�[�l���g�̓��I���[�h���T�|�[�g����N���X
*/
class HtmlHelpDll : public DllImp {
public:
	HtmlHelpDll() {}
	virtual ~HtmlHelpDll();

	// HtmlHelp ��Entry Point
	typedef HWND (WINAPI* Proc_HtmlHelp)(HWND, LPCTSTR, UINT, DWORD_PTR);
	Proc_HtmlHelp HtmlHelp;

protected:
	virtual bool InitDllImp();
	virtual LPCTSTR GetDllNameImp(int nIndex);
};

