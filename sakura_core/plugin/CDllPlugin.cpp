/*!	@file
	@brief DLL�v���O�C���N���X

*/
/*
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
#include "plugin/CDllPlugin.h"
#include "plugin/SakuraPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CEditorIfObj.h"
#include "view/CEditView.h"
#include "util/module.h"
#include "CSelectLang.h"

// �f�X�g���N�^
CDllPlugin::~CDllPlugin(void)
{
	for (auto it = m_plugs.begin(); it != m_plugs.end(); it++) {
		delete (CDllPlug*)(*it);
	}
}

// �v���O�̐���
// CPlug�̑����CDllPlug���쐬����
CPlug* CDllPlugin::CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
{
	CDllPlug* newPlug = new CDllPlug( plugin, id, sJack, sHandler, sLabel );
	return newPlug;
}

// �v���O�C����`�t�@�C���̓ǂݍ���
bool CDllPlugin::ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang )
{
	ReadPluginDefCommon( cProfile, cProfileMlang );

	// DLL���̓ǂݍ���
	cProfile->IOProfileData( PII_DLL, PII_DLL_NAME, m_sDllName );

	// �v���O�̓ǂݍ���
	ReadPluginDefPlug( cProfile, cProfileMlang );

	// �R�}���h�̓ǂݍ���
	ReadPluginDefCommand( cProfile, cProfileMlang );

	// �I�v�V������`�̓ǂݍ���	// 2010/3/24 Uchi
	ReadPluginDefOption( cProfile, cProfileMlang );

	// �������`�̓ǂݍ���
	ReadPluginDefString( cProfile, cProfileMlang );

	return true;
}

// �v���O���s
bool CDllPlugin::InvokePlug( CEditView* view, CPlug& plug_raw, CWSHIfObj::List& params )
{
	tstring dllPath = GetFilePath( to_tchar(m_sDllName.c_str()) );
	EDllResult resInit = InitDll( to_tchar( dllPath.c_str() ) );
	if (resInit != DLL_SUCCESS) {
		::MYMESSAGEBOX( view->m_hwndParent, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR1), dllPath.c_str(), m_sName.c_str() );
		return false;
	}

	CDllPlug& plug = *(static_cast<CDllPlug*>(&plug_raw));
	if (!plug.m_handler) {
		//DLL�֐��̎擾
		ImportTable imp[2] = {
			{ &plug.m_handler, to_achar( plug.m_sHandler.c_str() ) },
			{ NULL, 0 }
		};
		if (!RegisterEntries( imp )) {
//			DWORD err = GetLastError();
			::MYMESSAGEBOX( NULL, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR2) );
			return false;
		}
	}
	CMacroBeforeAfter ba;
	int flags = FA_NONRECORD | FA_FROMMACRO;
	if (view != NULL) {
		ba.ExecKeyMacroBefore(view, flags);
	}

	CPluginIfObj* objPlugin = new CPluginIfObj(*this);
	objPlugin->AddRef();
	objPlugin->SetPlugIndex(plug.m_id);	// ���s���v���O�ԍ����
	params.push_back(objPlugin);
	CEditorIfObj* objEditor = new CEditorIfObj();
	objEditor->AddRef();
	params.push_back(objEditor);
	SAKURA_DLL_PLUGIN_OBJ* obj = CreateIfObj(view, params);

	// DLL�֐��̌Ăяo��
	plug.m_handler(obj);

	EraseIfObj(obj);
	params.remove(objEditor);
	objEditor->Release();
	params.remove(objPlugin);
	objPlugin->Release();

	if (view != NULL) {
		ba.ExecKeyMacroAfter(view, flags, true);
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// DLL�v���O�C���ɓn�������쐬����
SAKURA_DLL_PLUGIN_OBJ* CDllPlugin::CreateIfObj(CEditView* view, CWSHIfObj::List& params)
{
	SAKURA_DLL_PLUGIN_OBJ* obj = new SAKURA_DLL_PLUGIN_OBJ;
	if (obj != NULL) {
		memset(obj, 0, sizeof(SAKURA_DLL_PLUGIN_OBJ));
		obj->m_dwVersion         = SAKURA_DLL_PLUGIN_VERSION;
		GetAppVersionInfo(NULL, VS_VERSION_INFO, &(obj->m_dwVersionMS), &(obj->m_dwVersionLS));
		obj->m_dwVersionShare    = N_SHAREDATA_VERSION;
		obj->m_wLangId           = CSelectLang::getDefaultLangId();
		obj->m_lpEditView        = view;
		obj->m_hParentHwnd       = (view != NULL) ? view->GetHwnd() : NULL;
		obj->m_lpDllPluginObj    = &params;
		obj->m_fnCommandHandler  = &HandleCommandCallback;
		obj->m_fnFunctionHandler = &HandleFunctionCallback;
		obj->m_dwIfObjListCount  = params.size();
		obj->m_IfObjList         = NULL;
		if (obj->m_dwIfObjListCount > 0) {
			obj->m_IfObjList     = new SAKURA_DLL_PLUGIN_IF_OBJ[obj->m_dwIfObjListCount];
			int i = 0;
			for (auto it = params.begin(); it != params.end(); it++) {
				SAKURA_DLL_PLUGIN_IF_OBJ* ifobj = &(obj->m_IfObjList[i++]);
				memset(ifobj, 0, sizeof(SAKURA_DLL_PLUGIN_IF_OBJ));
				if (wcslen((*it)->Name()) >= _countof(ifobj->m_szName)) {
					continue;
				}
				wcscpy_s(ifobj->m_szName, (*it)->Name());
				ifobj->m_lpIfObj      = (*it);
				ifobj->m_FunctionInfo = (MACRO_FUNC_INFO*)(*it)->GetMacroFuncInfo();
				ifobj->m_CommandInfo  = (MACRO_FUNC_INFO*)(*it)->GetMacroCommandInfo();
			}
		}
	}
	return obj;
}

///////////////////////////////////////////////////////////////////////////////
void CDllPlugin::EraseIfObj(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	if (obj != NULL) {
		if (obj->m_IfObjList != NULL) {
			delete[] obj->m_IfObjList;
		}
		delete obj;
	}
}

///////////////////////////////////////////////////////////////////////////////
// DLL�v���O�C������̃R�[���o�b�N(�֐�)
BOOL WINAPI CDllPlugin::HandleFunctionCallback(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result)
{
	CWSHIfObj* obj = reinterpret_cast<CWSHIfObj*>(lpIfObj);
	CEditView* view = reinterpret_cast<CEditView*>(lpEditView);
	if (obj != NULL) {
		//�p���N���X���Ăяo�����
		return obj->HandleFunction(view, (EFunctionCode)ID, Arguments, ArgSize, *Result);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// DLL�v���O�C������̃R�[���o�b�N(�R�}���h)
void WINAPI CDllPlugin::HandleCommandCallback(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, LPCWSTR Arguments[], const int ArgLengths[], const int ArgSize)
{
	CWSHIfObj* obj = reinterpret_cast<CWSHIfObj*>(lpIfObj);
	CEditView* view = reinterpret_cast<CEditView*>(lpEditView);
	if (obj != NULL) {
		obj->HandleCommand(view, (EFunctionCode)ID, Arguments, ArgLengths, ArgSize);
	}
}

