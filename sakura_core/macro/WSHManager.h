/*!	@file
	@brief WSH Manager
*/
#pragma once

#include <Windows.h>
#include <string>
#include "macro/SMacroMgr.h"
#include "macro/MacroManagerBase.h"
#include "macro/WSHIfObj.h"
class EditView;

typedef void (*EngineCallback)(wchar_t* Ext, char* EngineName);

class WSHMacroManager : public MacroManagerBase {
public:
	WSHMacroManager(std::wstring const AEngineName);
	virtual ~WSHMacroManager();

	virtual bool ExecKeyMacro(EditView& editView, int flags) const;
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);

	static MacroManagerBase* Creator(EditView& editView, const TCHAR* FileExt);
	static void Declare();

	void AddParam(WSHIfObj* param);				// インタフェースオブジェクトを追加する
	void AddParam(WSHIfObj::List& params);		// インタフェースオブジェクト達を追加する
	void ClearParam();							// インタフェースオブジェクトを削除する
protected:
	std::wstring source;
	std::wstring engineName;
	WSHIfObj::List params;
};

