/*!	@file
	@brief �\���p�����񓙂̎擾

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D
*/

#pragma once

#include <Windows.h>
#include "_main/global.h"
#include "config/maxdata.h"
#include "func/Funccode.h"

struct MacroRec;
struct CommonSetting;

// �}�N�����
struct MacroRec {
	TCHAR	szName[MACRONAME_MAX];	// �\����
	TCHAR	szFile[_MAX_PATH + 1];	// �t�@�C����(�f�B���N�g�����܂܂Ȃ�)
	bool	bReloadWhenExecute;		// ���s���ɓǂݍ��݂Ȃ������i�f�t�H���gon�j
	
	bool IsEnabled() const { return szFile[0] != _T('\0'); }
	const TCHAR* GetTitle() const { return szName[0] == _T('\0') ? szFile: szName; }
};

/*!
	@brief �\���p�����񓙂̎擾

	�@�\�C�@�\���ނƈʒu�C�@�\�ԍ��C������Ȃǂ̑Ή����W�񂷂�D
*/
class FuncLookup {

public:
	FuncLookup() : pMacroRec(nullptr) {}

	void Init(MacroRec* pMacroRec, CommonSetting* pCom) {
		this->pMacroRec = pMacroRec;
		pCommon = pCom;
	}

	EFunctionCode Pos2FuncCode(int category, int position, bool bGetUnavailable = true) const;
	bool Pos2FuncName(int category, int position, wchar_t* ptr, int bufsize) const;
	bool Funccode2Name(int funccode, wchar_t* ptr, int bufsize) const ;
	const TCHAR* Category2Name(int category) const;
	const wchar_t* Custmenu2Name(int index, wchar_t buf[], int bufSize) const;

	void SetCategory2Combo(HWND hComboBox) const ;
	void SetListItem(HWND hListBox, size_t category) const;
	
	size_t GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 3;	// ���ށ{�O���}�N���{�J�X�^�����j���[�{�v���O�C��
	}
	
	size_t GetItemCount(size_t category) const;


private:
	MacroRec* pMacroRec;	// �}�N�����
	CommonSetting* pCommon;	// ���ʐݒ�f�[�^�̈�ւ̃|�C���^

};

