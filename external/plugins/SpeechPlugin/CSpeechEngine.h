/*!	@file
	@brief Speech Engine

	Speech Engineを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.29
*/
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

#ifndef _CSPEECH_ENGINE_H_
#define _CSPEECH_ENGINE_H_

#include <stdio.h>
#include <objbase.h>
#include <sapi.h>
#include <list>

///////////////////////////////////////////////////////////////////////////////
/*!	Speech Engineをサポートするクラス
*/
class CSpeechEngine
{
public:
	CSpeechEngine();
	virtual ~CSpeechEngine();

	bool Initialize();
	void UnInitialize();
	bool Execute(LPCWSTR code, std::wstring& strVoice, const int nVolume, const int nRate);
	bool Purge();
	bool Wait();
	bool GetSpeechEngine(std::list<std::wstring>& lstTokens);
	bool SetSpeechEngine(std::wstring& strVoice);
	LPCWSTR GetLastMessage() const { return m_strErrorMsg.c_str(); }

private:
	bool					m_bReady;
	std::wstring			m_strErrorMsg;	//!< エラーメッセージ
	ISpVoice*				m_pVoice;		//Text-to-speech engine
    ISpObjectToken*			m_pToken;		//Voice token
};

#endif	//_CSPEECH_ENGINE_H_
