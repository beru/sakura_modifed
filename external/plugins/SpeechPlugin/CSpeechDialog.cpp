/*
	Copyright (C) 2013, Plugins developers

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"
#include "CSpeechEngine.h"
#include "CSpeechDialog.h"
#include "CPluginService.h"
#include "CommonTools.h"

///////////////////////////////////////////////////////////////////////////////
CSpeechDialog::CSpeechDialog()
{
	m_nRetCode = IDCANCEL;
	m_strVoice = thePluginService.m_strVoice;
	m_nVolume = thePluginService.m_nVolume;
	m_nRate = thePluginService.m_nRate;
}

///////////////////////////////////////////////////////////////////////////////
CSpeechDialog::~CSpeechDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
int CSpeechDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_SPEECH_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CSpeechDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	HWND hSliderVolume = ::GetDlgItem(GetHwnd(), IDC_SLIDER_VOLUME);
	HWND hSliderRate   = ::GetDlgItem(GetHwnd(), IDC_SLIDER_SPEED);
	::SendMessage(hSliderVolume, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
	::SendMessage(hSliderRate,   TBM_SETRANGE, 0, (LPARAM)MAKELONG(-10, 10));
	::SendMessage(hSliderVolume, TBM_SETTICFREQ, (WPARAM)10, 0);
	::SendMessage(hSliderRate,   TBM_SETTICFREQ, (WPARAM)2, 0);

	m_strVoice = thePluginService.m_strVoice;
	m_nVolume = thePluginService.m_nVolume;
	m_nRate = thePluginService.m_nRate;

	SetData();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CSpeechDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_nRetCode = wID;
		if(GetData()){
			//設定を書き込む
			thePluginService.WriteProfile();
			thePluginService.m_SpeechEngine.Execute(m_strText.c_str(), m_strVoice, m_nVolume, m_nRate);
			//CloseDialog(IDOK);
		}
		break;

	case IDCANCEL:
		//設定を書き込む
		thePluginService.WriteProfile();
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CSpeechDialog::SetData(void)
{
	if(GetHwnd() == NULL) return;

	HWND hCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_VOICE);
	HWND hSliderVolume = ::GetDlgItem(GetHwnd(), IDC_SLIDER_VOLUME);
	HWND hSliderRate   = ::GetDlgItem(GetHwnd(), IDC_SLIDER_SPEED);

	std::list<std::wstring> lstTokens;
	thePluginService.m_SpeechEngine.GetSpeechEngine(lstTokens);
	int index = 0;
	for(std::list<std::wstring>::iterator it = lstTokens.begin(); it != lstTokens.end(); ++it){
		::SendMessage(hCombo, CB_INSERTSTRING, (WPARAM)index, (LPARAM)it->c_str());
		if((index == 0) || (m_strVoice.compare(it->c_str()) == 0)){
			::SendMessage(hCombo, CB_SETCURSEL, (WPARAM)index, 0L);
		}
		index++;
	}
	::SendMessage(hSliderVolume, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)m_nVolume);
	::SendMessage(hSliderRate, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)m_nRate);

	HWND hEdit = ::GetDlgItem(GetHwnd(), IDC_EDIT);
	::SetWindowText(hEdit, m_strText.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CSpeechDialog::GetData(void)
{
	HWND hCombo        = ::GetDlgItem(GetHwnd(), IDC_COMBO_VOICE);
	HWND hSliderVolume = ::GetDlgItem(GetHwnd(), IDC_SLIDER_VOLUME);
	HWND hSliderRate   = ::GetDlgItem(GetHwnd(), IDC_SLIDER_SPEED);

	switch(m_nRetCode){
	case IDOK:
		{
			HWND hEdit = ::GetDlgItem(GetHwnd(), IDC_EDIT);
			m_strText = GetWindowText(hEdit);
		}
		//fallthrough
	case IDCANCEL:
		{
			int index = ::SendMessage(hCombo, CB_GETCURSEL, 0L, 0L);
			wchar_t szVoice[256];
			::SendMessage(hCombo, CB_GETLBTEXT, (WPARAM)index, (LPARAM)szVoice);
			m_strVoice = szVoice;
			m_nVolume  = (int)::SendMessage(hSliderVolume, TBM_GETPOS, 0, 0);
			m_nRate    = (int)::SendMessage(hSliderRate, TBM_GETPOS, 0, 0);
			thePluginService.m_strVoice = m_strVoice;
			thePluginService.m_nVolume  = m_nVolume;
			thePluginService.m_nRate    = m_nRate;
		}
		break;

	default:
		break;
	}

	return TRUE;
}
