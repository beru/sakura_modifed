/*!	@file
	@brief Ruby Engine

	Rubyを利用するためのインターフェース

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
/*
	msvcrt-ruby200.dllをsakura.exeと同じフォルダに置いてください。
	Ruby2.0用です。Rub1.9以下はAPIが違うため利用できません。
	スクリプトでは、
		print Editor.InsText('hello')
	のようにEditorに続けてメソッドを記述してください。
	Editorをnewする必要はありません。
	"1"で始まるメソッド名は"One", "S_1"で始まるメソッド名にしています、

	TODO:
	self.nameのようにメソッド名が取得できればハンドラ数を減らせますが、方法がわかりません。
	ハンドラが多いのでビルド時に自動生成したほうがいいかもしれません。
*/

#include "StdAfx.h"
#include "CPluginService.h"
#include "CRuby.h"
#include "CRubyEngine.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyEngine::CRubyEngine()
{
	m_nSourceLine = 0;
}

///////////////////////////////////////////////////////////////////////////////
CRubyEngine::~CRubyEngine()
{
}

///////////////////////////////////////////////////////////////////////////////
bool CRubyEngine::Load(LPCWSTR lpszModulePath)
{
	return Ruby.Load(lpszModulePath) != FALSE;
}

///////////////////////////////////////////////////////////////////////////////
/*!	スクリプトを実行する
	@param[in]	code	スクリプト(UTF8)
*/
bool CRubyEngine::Execute(const char* code)
{
	int state = 0;
	m_strErrorMsg = L"";
	m_strSourceFile = L"";
	m_nSourceLine = 0;

	if(Ruby.IsAvailable() == false){
		return false;
	}

	//将来可変情報が入るときのため呼び出し毎に管理情報を用意しておく
	struct CRubyHandler::RubyExecInfo info;
	info.m_lpRuby          = &Ruby;
	info.m_lpPluginService = &thePluginService;
	info.m_argc            = 1;
	info.m_argv[0]         = "";
	info.m_argv[1]         = NULL;

	Ruby.ruby_sysinit(&info.m_argc, (char***)&info.m_argv);	//WinMain時は必要
	Ruby.ruby_init();
	CRubyHandler handler;
	handler.ReadyCommands(Ruby, &info);

	Ruby.rb_eval_string_protect(code, &state);
	if(state != 0){
		//エラー情報を格納する
		VALUE err = Ruby.rb_obj_as_string(Ruby.rb_errinfo());
		m_strErrorMsg = CPluginService::to_wstr(Ruby.StringValueCStr(err), CP_UTF8);
		m_strSourceFile = CPluginService::to_wstr(Ruby.rb_sourcefile(), CP_UTF8);
		m_nSourceLine = Ruby.rb_sourceline();
/*
		std::wstring strCaption = m_strErrorMsg + L"\n\nFile: " + m_strSourceFile + L"\n";
		int nLength = m_strErrorMsg.length() + m_strSourceFile.length() + 256;
		wchar_t* lpBuffer = new wchar_t[nLength];
		swprintf(lpBuffer, L"Line: %d", m_nSourceLine);
		strCaption += lpBuffer;
		delete[] lpBuffer;
		//::MessageBox(thePluginService.Editor.GetParentHwnd(), strCaption.c_str(), RUBY_SCRIPT_PLUGIN, MB_ICONEXCLAMATION | MB_OK);
*/
	}

	Ruby.ruby_cleanup(0);

	return (state == 0);
}
