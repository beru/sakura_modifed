/*!	@file
	@brief DLL�v���O�C��I/F
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
	int					nArgSize;			// ��������
	VARTYPE*			lpVarArgEx;		// VARTYPE�z��ւ̃|�C���^
} MACRO_FUNC_INFO_EX;

typedef struct tagMACRO_FUNC_INFO {
	DWORD				nFuncID;			// �@�\ID
	LPCWSTR				lpszFuncName;		// �֐���
	VARTYPE				varArguments[4];	// ����
	VARTYPE				varResult;		// �߂�l
	MACRO_FUNC_INFO_EX*	lpExData;			// 5�ڈȍ~�̈������
} MACRO_FUNC_INFO;

typedef struct tagSAKURA_DLL_PLUGIN_IF_OBJ {
	wchar_t				szName[64];			// ���ʎq
	LPVOID				lpIfObj;				// �v���O�C�����
	MACRO_FUNC_INFO*	pFunctionInfo;			// �}�N���֐����
	MACRO_FUNC_INFO*	pCommandInfo;			// �}�N���R�}���h���
} SAKURA_DLL_PLUGIN_IF_OBJ;

/*
	�T�N���G�f�B�^����DLL�v���O�C���ɓn�����\����(�G�f�B�^���v���O�C��)
	���̍\���̂��C�������ꍇ��CBasePluginInitialize::Copy()���C�����邱�ƁB
*/
typedef struct tagSAKURA_DLL_PLUGIN_OBJ {
	DWORD						dwVersion;			// DLL�v���O�C���\���̎��ʃo�[�W����
	DWORD						dwVersionMS;			// sakura�o�[�W����(MS)
	DWORD						dwVersionLS;			// sakura�o�[�W����(LS)
	DWORD						dwVersionShare;		// ���L�������o�[�W����
	LANGID						wLangId;				// ����ID
	WORD						wPadding1;			// �\��(�p�f�B���O)
	HWND						hParentHwnd;			// �e�E�B���h�E�n���h��
	LPVOID						lpDllPluginObj;		// �v���O�C�����(CWSHIfObj::List)
	LPVOID						lpEditView;			// EditView���
	HandleFunctionCallback		fnFunctionHandler;	// �֐��n���h��
	HandleCommandCallback		fnCommandHandler;		// �R�}���h�n���h��
	SAKURA_DLL_PLUGIN_IF_OBJ*	IfObjList;			// �v���O�C�����
	DWORD						dwIfObjListCount;		// �v���O�C������
	DWORD						wPadding2;			// �\��(�p�f�B���O)
	LPVOID						lpUserData[4];		// ���[�U���
	DWORD						dwReserve[8];			// �\��
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

