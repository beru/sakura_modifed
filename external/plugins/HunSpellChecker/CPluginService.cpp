/*
	Copyright (C) 2013-2014, Plugins developers

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

#include "stdafx.h"
#include "CHunspellLoader.h"
#include "CPluginService.h"
#include <map>
#include "CDlgSpellCheck.h"
#include "resource.h"

typedef std::map<std::wstring, std::wstring> MapStringToString;
typedef std::map<std::wstring, int> MapStringToInteger;

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginHunspellChecker(SAKURA_DLL_PLUGIN_OBJ* obj, const DWORD dwMode)
{
	ReadOption();

	WideString strDllPath = m_strHunspellPath + L"\\" + HUNSPELL_DLL_NAME;
	if(Hunspell.Load(strDllPath.c_str()) == FALSE){
		WideString strMessage;
		LoadString(IDS_STR_MSG1, strMessage);	//Hunspell�̏������Ɏ��s���܂����B
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	Run(dwMode);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadOption()
{
	// �ݒ��ǂݍ��݂܂�
	WideString strPluginPath = Plugin.GetPluginDir();

	//HunspellPath
	m_strHunspellPath = Plugin.GetOption(GetPluginName(), L"HunspellPath");
	if(m_strHunspellPath.length() == 0){
		m_strHunspellPath = strPluginPath + L"\\Hunspell";
	}else{
		if(m_strHunspellPath.substr(0, 2) == L".\\"){
			m_strHunspellPath = strPluginPath + L"\\" + m_strHunspellPath;
		}
	}

	//Language
	m_strLanguage = Plugin.GetOption(GetPluginName(), L"Language");
	if(m_strLanguage.length() == 0){
		m_strLanguage = L"en_US";
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::Run(const DWORD dwMode)
{
	HINSTANCE hInstance = GetInstance();
	HWND hParentWnd = Editor.GetParentHwnd();
	MapStringToString m_Replace;
	MapStringToInteger m_Ignore;
	int x = -1;
	int y = -1;

	//����������
	WideString aff = m_strHunspellPath + L"\\" + m_strLanguage + L".aff";
	WideString dic = m_strHunspellPath + L"\\" + m_strLanguage + L".dic";

	AnsiString strAnsiAff = to_astr(aff);
	AnsiString strAnsiDic = to_astr(dic);
	Hunhandle* lpHunHandle = Hunspell.Hunspell_create(strAnsiAff.c_str(), strAnsiDic.c_str());
	if(lpHunHandle == NULL){
		WideString strMessage;
		LoadString(IDS_STR_MSG1, strMessage);	//Hunspell�̏������Ɏ��s���܂����B
		::MessageBox(hParentWnd, strMessage.c_str(), GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	BOOL LOOP = TRUE;
	switch(dwMode){
	case CDlgSpellCheck::RUNMODE_ONESHOT:
		LOOP = FALSE;
		break;
	case CDlgSpellCheck::RUNMODE_FILETOP:
		// �t�@�C���̐擪����`�F�b�N����
		Editor.S_GoFileTop();
		break;
	default:
		break;
	}

	BOOL bFirst = TRUE;
	do{
		//�ŏ������̓J�[�\���ʒu�̒P���I�����邽�߈ړ����Ȃ�
		if(bFirst == FALSE){
			Editor.WordRight();
		}

		//�P���������
		WideString word = L"";
		int result = GetNextEditorWord(word, x, y);
		bFirst = FALSE;
		if(result == 0){
			//word found
		}else if(result == 1){
			continue;
		}else{
			break;
		}
		AnsiString strAnsiWord = to_astr(word);

		//����݂̖̂������X�g�ɓo�^����Ă��邩
		MapStringToInteger::iterator it1 = m_Ignore.find(word.c_str());
		if(it1 != m_Ignore.end()){
			continue;
		}

		//����݂̂̒u�����X�g�ɓo�^����Ă��邩
		MapStringToString::iterator it2 = m_Replace.find(word.c_str());
		if(it2 != m_Replace.end()){
			Editor.InsText(it2->second.c_str());
			continue;
		}

		if(dwMode != CDlgSpellCheck::RUNMODE_ONESHOT){
			if (Hunspell.Hunspell_spell(lpHunHandle, strAnsiWord.c_str()) != 0){
				continue;
			}
		}

		CDlgSpellCheck dlg;
		dlg.m_strWord = word;

		//����T��
		char** lplpszSuggestList = NULL;
		int nCount = Hunspell.Hunspell_suggest(lpHunHandle, &lplpszSuggestList, strAnsiWord.c_str());
		for(int i = 0; i < nCount; i++){
			//��⃊�X�g�ɒǉ�����
			WideString strWideWord = to_wstr(lplpszSuggestList[i]);
			dlg.m_lstCorrection.push_back(strWideWord);
		}
		Hunspell.Hunspell_free_list(lpHunHandle, &lplpszSuggestList, nCount);

		dlg.DoModal(hInstance, hParentWnd, !LOOP);
		switch(dlg.m_nRetCode){
		case IDC_BUTTON_REPLACE_ALL:	//�ȍ~���ׂĒu��
			m_Replace[word.c_str()] = dlg.m_strReplace.c_str();
			break;

		case IDC_BUTTON_REPLACE:	//�u��
			Editor.InsText(dlg.m_strReplace);
			break;

		case IDC_BUTTON_IGNORE_ALL:	//�ȍ~���ׂĖ���
			m_Ignore[word.c_str()] = 1;
			break;

		case IDC_BUTTON_IGNORE:	//����
			//�������Ȃ�
			break;

		case IDC_BUTTON_ADD_DICT_REPLACE:	//�P��o�^���Ēu��
		case IDC_BUTTON_ADD_DICT_REPLACE_ALL:	//�P��o�^���Ĉȍ~���ׂĒu��
		case IDCANCEL:
		default:
			LOOP = 0;
			break;
		}
	}while(LOOP);

	Hunspell.Hunspell_destroy(lpHunHandle);
	lpHunHandle = NULL;

	Editor.ReDraw();

	if(dwMode != CDlgSpellCheck::RUNMODE_ONESHOT){
		WideString strMessage;
		LoadString(IDS_STR_MSG2, strMessage);	//�I�����܂����B
		::MessageBox(NULL, strMessage.c_str(), GetPluginName(), MB_ICONINFORMATION | MB_OK);
	}
}

///////////////////////////////////////////////////////////////////////////////
int CPluginService::GetNextEditorWord(WideString& word, int& prevX, int& prevY)
{
	Editor.SelectWord();
	word = Editor.GetSelectedString(0);
	int size = word.length();
	if((size == 1) || (size >= 256) || (isOnlyAlphabet(word.c_str()) == FALSE)){
		return 1;
	}else{
		//�t�@�C�������ňړ����Ȃ�
		int cx = Editor.ExpandParameter_x();
		int cy = Editor.ExpandParameter_y();
		if(prevX == cx && prevY == cy){
			return 2;
		}
		prevX = cx;
		prevY = cy;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginService::isOnlyAlphabet(LPCWSTR str)
{
	for(LPCWSTR p = str; *p != 0; p++){
		if(*p >= 256){
			return FALSE;
		}
		if(isalpha(*p) == 0){
			return FALSE;
		}
	}
	return TRUE;
}
