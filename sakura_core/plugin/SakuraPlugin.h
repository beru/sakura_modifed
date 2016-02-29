/*!	@file
	@brief DLL�v���O�C��I/F
*/
/*
	Copyright (C) 2013-2014, Plugins developers

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
#include <OleAuto.h>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

/*
	�\���̂̃o�[�W����
		0x00010000=1.0

	  History:
	  0x00010000	�����o�[�W����(sakura2, UNICODE)
*/
#define SAKURA_DLL_PLUGIN_VERSION	0x00010000	// �\���̂̃o�[�W����

typedef BOOL (WINAPI *HandleFunctionCallback)(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result);
typedef void (WINAPI *HandleCommandCallback)(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, LPCWSTR Arguments[], const int* ArgLengths, const int ArgSize);

/*
	�}�N����`���
*/
typedef struct tagMACRO_FUNC_INFO_EX {
	int					m_nArgSize;			// ��������
	VARTYPE*			m_lpVarArgEx;		// VARTYPE�z��ւ̃|�C���^
} MACRO_FUNC_INFO_EX;

typedef struct tagMACRO_FUNC_INFO {
	DWORD				m_nFuncID;			// �@�\ID
	LPCWSTR				m_lpszFuncName;		// �֐���
	VARTYPE				m_varArguments[4];	// ����
	VARTYPE				m_varResult;		// �߂�l
	MACRO_FUNC_INFO_EX*	m_lpExData;			// 5�ڈȍ~�̈������
} MACRO_FUNC_INFO;

typedef struct tagSAKURA_DLL_PLUGIN_IF_OBJ {
	WCHAR						szName[64];			// ���ʎq
	LPVOID						m_lpIfObj;				// �v���O�C�����
	MACRO_FUNC_INFO*			m_pFunctionInfo;			// �}�N���֐����
	MACRO_FUNC_INFO*			m_pCommandInfo;			// �}�N���R�}���h���
} SAKURA_DLL_PLUGIN_IF_OBJ;

/*
	�T�N���G�f�B�^����DLL�v���O�C���ɓn�����\����(�G�f�B�^���v���O�C��)
	���̍\���̂��C�������ꍇ��CBasePluginInitialize::Copy()���C�����邱�ƁB
*/
typedef struct tagSAKURA_DLL_PLUGIN_OBJ {
	DWORD						m_dwVersion;			// DLL�v���O�C���\���̎��ʃo�[�W����
	DWORD						m_dwVersionMS;			// sakura�o�[�W����(MS)
	DWORD						m_dwVersionLS;			// sakura�o�[�W����(LS)
	DWORD						m_dwVersionShare;		// ���L�������o�[�W����
	LANGID						m_wLangId;				// ����ID
	WORD						m_wPadding1;			// �\��(�p�f�B���O)
	HWND						m_hParentHwnd;			// �e�E�B���h�E�n���h��
	LPVOID						m_lpDllPluginObj;		// �v���O�C�����(CWSHIfObj::List)
	LPVOID						m_lpEditView;			// EditView���
	HandleFunctionCallback		m_fnFunctionHandler;	// �֐��n���h��
	HandleCommandCallback		m_fnCommandHandler;		// �R�}���h�n���h��
	SAKURA_DLL_PLUGIN_IF_OBJ*	m_IfObjList;			// �v���O�C�����
	DWORD						m_dwIfObjListCount;		// �v���O�C������
	DWORD						m_wPadding2;			// �\��(�p�f�B���O)
	LPVOID						m_lpUserData[4];		// ���[�U���
	DWORD						m_dwReserve[8];			// �\��
// TODO: �\���̂��g������Ƃ���DLL_PLUGIN_INFO_VERSION��ύX��ifdef�Ŋg������
} SAKURA_DLL_PLUGIN_OBJ;

// DLL�v���O�C��API
typedef void (WINAPI *DllPlugHandler)(SAKURA_DLL_PLUGIN_OBJ* obj);

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

