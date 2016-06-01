/*!	@file
	@brief プラグインマクロマネージャクラス
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

#pragma once

#include <Windows.h>
#include "MacroManagerBase.h"
#include "plugin/Plugin.h"
#include <string>
#include "plugin/SakuraPlugin.h"

class EditView;

///////////////////////////////////////////////////////////////////////////////
// Plugin macro manager
class PluginMacroManager: public MacroManagerBase {
public:
	PluginMacroManager(const wchar_t* Ext, Plug* plug);
	virtual ~PluginMacroManager();

	virtual bool ExecKeyMacro(class EditView& editView, int flags) const;
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* Path);
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* Code);

	static MacroManagerBase* Creator(EditView& view, const TCHAR* Ext);
	static void Declare(void);

protected:
	std::wstring	source;	// マクロスクリプト
	std::wstring	ext;		// 拡張子
	Plug*			plug;		// プラグイン
};

