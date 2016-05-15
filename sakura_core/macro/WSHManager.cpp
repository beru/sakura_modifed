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
#include "macro/WSHManager.h"
#include "macro/WSH.h"
#include "macro/EditorIfObj.h"
#include "view/EditView.h"
#include "io/TextStream.h"
#include "util/os.h"
#include "macro/MacroFactory.h"

static void MacroError(
	BSTR Description,
	BSTR Source,
	void* Data
	)
{
	EditView *View = reinterpret_cast<EditView*>(Data);
	MessageBox(View->GetHwnd(), to_tchar(Description), to_tchar(Source), MB_ICONERROR);
}

WSHMacroManager::WSHMacroManager(std::wstring const AEngineName) : engineName(AEngineName)
{
}

WSHMacroManager::~WSHMacroManager()
{
}

/** WSH�}�N���̎��s

	@param EditView [in] ����Ώ�EditView
	
	@date 2007.07.20 genta : flags�ǉ�
*/
bool WSHMacroManager::ExecKeyMacro(EditView& editView, int flags) const
{
	auto engine = std::make_unique<WSHClient>(engineName.c_str(), MacroError, &editView);
	bool bRet = false;
	if (engine->isValid) {
		// �C���^�t�F�[�X�I�u�W�F�N�g�̓o�^
		WSHIfObj* objEditor = new EditorIfObj();
		objEditor->ReadyMethods(editView, flags);
		engine->AddInterfaceObject(objEditor);
		for (auto it=params.begin(); it!=params.end(); ++it) {
			(*it)->ReadyMethods(editView, flags);
			engine->AddInterfaceObject(*it);
		}

		bRet = engine->Execute(source.c_str());
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
	// �\�[�X�ǂݍ��� -> source
	source = L"";
	
	std::vector<wchar_t> buffW;
	ReadFileAndUnicodify(pszPath, buffW);

	buffW.push_back(0);
	source = &buffW[0];

	return true;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�����񂩂�j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszCode   [in] �}�N���R�[�h
*/
bool WSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// �\�[�X�ǂݍ��� -> source
	source = to_wchar(pszCode);
	return true;
}

MacroManagerBase* WSHMacroManager::Creator(EditView& editView, const TCHAR* FileExt)
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

void WSHMacroManager::Declare()
{
	// �b��
	MacroFactory::getInstance().RegisterCreator(Creator);
}

// �C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj* param)
{
	params.push_back(param);
}

// �C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj::List& params)
{
	params.insert(params.end(), params.begin(), params.end());
}

// �C���^�t�F�[�X�I�u�W�F�N�g���폜����
void WSHMacroManager::ClearParam()
{
	params.clear();
}

