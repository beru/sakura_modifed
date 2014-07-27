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

#ifndef _CPLUGINMACROMANAGER_H_
#define _CPLUGINMACROMANAGER_H_

#include <Windows.h>
#include "CMacroManagerBase.h"
#include "plugin/CPlugin.h"
#include <string>
#include "plugin/SakuraPlugin.h"

class CEditView;

///////////////////////////////////////////////////////////////////////////////
/*!	Plugin macro manager
*/
class CPluginMacroManager: public CMacroManagerBase
{
public:
	CPluginMacroManager(const WCHAR* Ext, CPlug* plug);
	virtual ~CPluginMacroManager();

	virtual bool ExecKeyMacro(class CEditView* pcEditView, int flags) const;
	virtual BOOL LoadKeyMacro(HINSTANCE hInstance, const TCHAR* Path);
	virtual BOOL LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* Code);

	static CMacroManagerBase* Creator(const TCHAR* Ext);
	static void declare(void);

protected:
	std::wstring	m_Source;	//!< マクロスクリプト
	std::wstring	m_Ext;		//!< 拡張子
	CPlug*			m_Plug;		//!< プラグイン
};

#endif	//_CPLUGINMACROMANAGER_H_
