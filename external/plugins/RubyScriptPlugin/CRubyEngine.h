/*!	@file
	@brief Ruby Engine

	Ruby Engineを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.26
*/
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

#ifndef _CRUBY_ENGINE_H_
#define _CRUBY_ENGINE_H_

#include <stdio.h>
#include <ObjIdl.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	Ruby Engineをサポートするクラス
*/
class CRubyEngine
{
public:
	CRubyEngine();
	virtual ~CRubyEngine();

	bool Load(LPCWSTR lpszModulePath);
	bool Execute(const char* code);
	LPCWSTR GetLastMessage() const { return m_strErrorMsg.c_str(); }

private:
	CRuby				Ruby;
	std::wstring		m_strErrorMsg;		//!< エラーメッセージ
	std::wstring		m_strSourceFile;	//!< エラー発生ソースファイル
	int					m_nSourceLine;		//!< エラー発生行
};

#endif	//_CRUBY_ENGINE_H_
