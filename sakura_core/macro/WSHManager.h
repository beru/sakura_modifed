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

	void AddParam(WSHIfObj* param);				// �C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
	void AddParam(WSHIfObj::List& params);		// �C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
	void ClearParam();							// �C���^�t�F�[�X�I�u�W�F�N�g���폜����
protected:
	std::wstring source;
	std::wstring engineName;
	WSHIfObj::List params;
};

