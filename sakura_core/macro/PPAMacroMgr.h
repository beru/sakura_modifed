/*!	@file
	@brief PPA.DLL�}�N��
*/

#pragma once

#include <Windows.h>
#include "KeyMacroMgr.h"
#include "PPA.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// PPA�}�N��
class PPAMacroMgr: public MacroManagerBase {
public:
	/*
	||  Constructors
	*/
	PPAMacroMgr();
	~PPAMacroMgr();

	/*
	||	PPA.DLL�ɈϏ����镔��
	*/
	virtual bool ExecKeyMacro(class EditView& editView, int flags) const;	// PPA�}�N���̎��s
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);		// �L�[�{�[�h�}�N�����t�@�C������ǂݍ��݁ACMacro�̗�ɕϊ�
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);	// �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ��݁ACMacro�̗�ɕϊ�

	static class PPA cPPA;

	// Apr. 29, 2002 genta
	static MacroManagerBase* Creator(EditView& view, const TCHAR* ext);
	static void Declare(void);

protected:
	NativeW buffer;
};

