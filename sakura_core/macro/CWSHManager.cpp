/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cpp����؂�o��
*/
/*
	Copyright (C) 2002, �S, genta
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

/** WSH�}�N���̎��s

	@param EditView [in] ����Ώ�EditView
	
	@date 2007.07.20 genta : flags�ǉ�
*/
bool WSHMacroManager::ExecKeyMacro(EditView* EditView, int flags) const
{
	auto engine = std::make_unique<WSHClient>(m_engineName.c_str(), MacroError, EditView);
	bool bRet = false;
	if (engine->m_Valid) {
		// �C���^�t�F�[�X�I�u�W�F�N�g�̓o�^
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
	WSH�}�N���̓ǂݍ��݁i�t�@�C������j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszPath   [in] �t�@�C���̃p�X
*/
bool WSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	// �\�[�X�ǂݍ��� -> m_source
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
	WSH�}�N���̓ǂݍ��݁i�����񂩂�j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszCode   [in] �}�N���R�[�h
*/
bool WSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// �\�[�X�ǂݍ��� -> m_source
	m_source = to_wchar(pszCode);
	return true;
}

MacroManagerBase* WSHMacroManager::Creator(const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; // 1024�𒴂������͒m��܂���
	
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
	// �b��
	MacroFactory::getInstance()->RegisterCreator(Creator);
}

// �C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj* param)
{
	m_params.push_back(param);
}

// �C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj::List& params)
{
	m_params.insert(m_params.end(), params.begin(), params.end());
}

// �C���^�t�F�[�X�I�u�W�F�N�g���폜����
void WSHMacroManager::ClearParam()
{
	m_params.clear();
}

