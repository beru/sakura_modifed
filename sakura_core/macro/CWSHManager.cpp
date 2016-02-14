/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cppから切り出し
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat

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
#include "macro/CWSHManager.h"
#include "macro/CWSH.h"
#include "macro/CEditorIfObj.h"
#include "view/CEditView.h"
#include "io/CTextStream.h"
#include "util/os.h"
#include "macro/CMacroFactory.h"

static void MacroError(
	BSTR Description,
	BSTR Source,
	void* Data
	)
{
	EditView *View = reinterpret_cast<EditView*>(Data);
	MessageBox(View->GetHwnd(), to_tchar(Description), to_tchar(Source), MB_ICONERROR);
}

WSHMacroManager::WSHMacroManager(std::wstring const AEngineName) : m_engineName(AEngineName)
{
}

WSHMacroManager::~WSHMacroManager()
{
}

/** WSHマクロの実行

	@param EditView [in] 操作対象EditView
	
	@date 2007.07.20 genta : flags追加
*/
bool WSHMacroManager::ExecKeyMacro(EditView* EditView, int flags) const
{
	auto engine = std::make_unique<WSHClient>(m_engineName.c_str(), MacroError, EditView);
	bool bRet = false;
	if (engine->m_Valid) {
		// インタフェースオブジェクトの登録
		WSHIfObj* objEditor = new EditorIfObj();
		objEditor->ReadyMethods(EditView, flags);
		engine->AddInterfaceObject(objEditor);
		for (auto it=m_params.begin(); it!=m_params.end(); ++it) {
			(*it)->ReadyMethods(EditView, flags);
			engine->AddInterfaceObject(*it);
		}

		bRet = engine->Execute(m_source.c_str());
	}
	return bRet;
}

/*!
	WSHマクロの読み込み（ファイルから）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszPath   [in] ファイルのパス
*/
bool WSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	// ソース読み込み -> m_source
	m_source = L"";
	
	TextInputStream in(pszPath);
	if (!in) {
		return false;
	}

	while (in) {
		m_source += in.ReadLineW() + L"\r\n";
	}
	return true;
}

/*!
	WSHマクロの読み込み（文字列から）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszCode   [in] マクロコード
*/
bool WSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// ソース読み込み -> m_source
	m_source = to_wchar(pszCode);
	return true;
}

MacroManagerBase* WSHMacroManager::Creator(const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; // 1024を超えたら後は知りません
	
	_tcscpy(FileExtWithDot, _T("."));
	_tcscat(FileExtWithDot, FileExt);

	if (ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024)) {
		lstrcat(FileType, _T("\\ScriptEngine"));
		if (ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024)) {
			wchar_t EngineNameW[1024];
			_tcstowcs(EngineNameW, EngineName, _countof(EngineNameW));
			return new WSHMacroManager(EngineNameW);
		}
	}
	return NULL;
}

void WSHMacroManager::declare()
{
	// 暫定
	MacroFactory::getInstance()->RegisterCreator(Creator);
}

// インタフェースオブジェクトを追加する
void WSHMacroManager::AddParam(WSHIfObj* param)
{
	m_params.push_back(param);
}

// インタフェースオブジェクト達を追加する
void WSHMacroManager::AddParam(WSHIfObj::List& params)
{
	m_params.insert(m_params.end(), params.begin(), params.end());
}

// インタフェースオブジェクトを削除する
void WSHMacroManager::ClearParam()
{
	m_params.clear();
}

