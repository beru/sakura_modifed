/*!	@file
	@brief �v���O�C���}�N���}�l�[�W���N���X
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
	std::wstring	source;	// �}�N���X�N���v�g
	std::wstring	ext;		// �g���q
	Plug*			plug;		// �v���O�C��
};

