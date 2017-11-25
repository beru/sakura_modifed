/*!	@file
	@brief �L�[�{�[�h�}�N��(���ڎ��s�p)
*/
#pragma once

#include <Windows.h>
#include <WTypes.h> // VARTYPE

#include "MacroManagerBase.h"
#include "env/DllSharedData.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class EditView;


const int STAND_KEYMACRO	= -1;	// �W���}�N��(�L�[�}�N��)
const int TEMP_KEYMACRO		= -2;	// �ꎞ�}�N��(���O���w�肵�ă}�N�����s)
const int INVALID_MACRO_IDX	= -3;	// �����ȃ}�N���̃C���f�b�N�X�ԍ�

struct MacroFuncInfoEx {
	int			nArgMinSize;
	int			nArgMaxSize;
	VARTYPE*	pVarArgEx;
};

// �}�N���֐����\����
// �֐�����SMacroMgr������
struct MacroFuncInfo {
	int				nFuncID;
	const wchar_t*	pszFuncName;
	VARTYPE			varArguments[4];	// �����̌^�̔z��
	VARTYPE			varResult;		// �߂�l�̌^ VT_EMPTY�Ȃ�procedure�Ƃ������Ƃ�
	MacroFuncInfoEx*	pData;
};
// �}�N���֐����\���̔z��
typedef MacroFuncInfo* MacroFuncInfoArray;

class SMacroMgr {
	// �f�[�^�̌^�錾
	MacroManagerBase* savedKeyMacros[MAX_CUSTMACRO];	// �L�[�}�N�����J�X�^�����j���[�̐������Ǘ�
	// �L�[�}�N���ɕW���}�N���ȊO�̃}�N����ǂݍ��߂�悤��
	MacroManagerBase* pKeyMacro;	// �W���́i�ۑ����ł���j�L�[�}�N�����Ǘ�

	// �ꎞ�}�N���i���O���w�肵�ă}�N�����s�j���Ǘ�
	MacroManagerBase* pTempMacro;

public:

	/*
	||  Constructors
	*/
	SMacroMgr();
	~SMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear(int idx);
	void ClearAll(void);	// �L�[�}�N���̃o�b�t�@���N���A����

	// �L�[�{�[�h�}�N���̎��s
	bool Exec(int idx, HINSTANCE hInstance, EditView& editView, int flags);
	
	//	���s�\���HCShareData�ɖ₢���킹
	bool IsEnabled(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].IsEnabled() : false;
	}
	
	//	�\�����閼�O�̎擾
	const TCHAR* GetTitle(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].GetTitle() : NULL;
	}
	
	//	�\�����̎擾
	const TCHAR* GetName(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].szName : NULL;
	}
	
	/*!	@brief �t�@�C�����̎擾
		@param idx [in] �}�N���ԍ�
	*/
	const TCHAR* GetFile(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].szFile : 
		((idx == STAND_KEYMACRO || idx == TEMP_KEYMACRO) && sMacroPath != _T("")) ?
		sMacroPath.c_str() : NULL;
	}

	// �L�[�{�[�h�}�N���̓ǂݍ���
	bool Load(EditView& view, int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType);
	bool Save(int idx, HINSTANCE hInstance, const TCHAR* pszPath);
	void UnloadAll(void);

	// �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	int Append(int idx, EFunctionCode nFuncID, const LPARAM* lParams, EditView& editView);

	/*
	||  Attributes & Operations
	*/
	static wchar_t* GetFuncInfoByID(HINSTANCE , int , wchar_t* , wchar_t*);	// �@�\ID���֐����C�@�\�����{��
	static EFunctionCode GetFuncInfoByName(HINSTANCE , const wchar_t* , wchar_t*);	// �֐������@�\ID�C�@�\�����{��
	static bool CanFuncIsKeyMacro(int);	// �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ�
	
	// Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID(int);
	
	bool IsSaveOk(void);

	// Sep. 15, 2005 FILE	���s���}�N���̃C���f�b�N�X�ԍ����� (INVALID_MACRO_IDX:����)
	int GetCurrentIdx(void) const {
		return currentIdx;
	}
	int SetCurrentIdx(int idx) {
		int oldIdx = currentIdx;
		currentIdx = idx;
		return oldIdx;
	}

	MacroManagerBase* SetTempMacro(MacroManagerBase* newMacro);

private:
	DllSharedData*	pShareData;
	MacroManagerBase** Idx2Ptr(int idx);

	/*!	���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:����) */
	int currentIdx;
	std::tstring sMacroPath;	// Load�����}�N����

public:
	static MacroFuncInfo	macroFuncInfoCommandArr[];	// �R�}���h���(�߂�l�Ȃ�)
	static MacroFuncInfo	macroFuncInfoArr[];		// �֐����(�߂�l����)

private:
	DISALLOW_COPY_AND_ASSIGN(SMacroMgr);
};

