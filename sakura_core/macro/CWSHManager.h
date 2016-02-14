/*!	@file
	@brief WSH Manager

	@author �S, genta
	@date 2002�N4��28��,5��3��,5��5��,5��6��,5��13��,5��16��
	@date 2002.08.25 genta CWSH.h��蕪��

	@par TODO
	@li ���m�̃G���W���ɑΉ��ł���悤CMacroFactory��ύX �� �v�c�_
	@li EditView::HandleCommand���g�� �� CMacro::HandleCommand�ł��Ȃɂ�����Ă�悤�Ȃ̂ł�����Ȃ������H
	@li vector::reserve���g�� �� CSMacroMgr�Ō����錾����Ė����̂Ō�����
	@li �ĕ`��̑����ShowEditCaret �� protected�ł���[
*/
/*
	Copyright (C) 2002, �S, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#pragma once

#include <Windows.h>
#include <string>
#include "macro/CSMacroMgr.h"
#include "macro/CMacroManagerBase.h"
#include "macro/CWSHIfObj.h"
class EditView;

typedef void (*EngineCallback)(wchar_t* Ext, char* EngineName);

class WSHMacroManager : public MacroManagerBase {
public:
	WSHMacroManager(std::wstring const AEngineName);
	virtual ~WSHMacroManager();

	// 2007.07.20 genta : flags�ǉ�
	virtual bool ExecKeyMacro(EditView *EditView, int flags) const;
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);

	static MacroManagerBase* Creator(const TCHAR* FileExt);
	static void declare();

	void AddParam(WSHIfObj* param);				// �C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
	void AddParam(WSHIfObj::List& params);		// �C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
	void ClearParam();							// �C���^�t�F�[�X�I�u�W�F�N�g���폜����
protected:
	std::wstring m_source;
	std::wstring m_engineName;
	WSHIfObj::List m_params;
	// 2009.10.29 syat WSHIfObj�ֈړ�
	////	2007.07.20 genta : flags�ǉ�
	//static void ReadyCommands(CIfObj *Object, MacroFuncInfo *Info, int flags);
};

