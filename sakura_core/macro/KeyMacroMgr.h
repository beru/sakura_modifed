/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "MacroManagerBase.h"
#include "Funccode_enum.h"

class Macro;

//#define MAX_STRLEN			70
//#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// �L�[�{�[�h�}�N��
/*!
	�L�[�{�[�h�}�N���N���X
*/
class KeyMacroMgr : public MacroManagerBase {
public:
	/*
	||  Constructors
	*/
	KeyMacroMgr();
	~KeyMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll(void);				// �L�[�}�N���̃o�b�t�@���N���A����
	void Append(EFunctionCode, const LPARAM*, class EditView* pEditView);		// �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	void Append(class Macro* macro);		// �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	
	// �L�[�{�[�h�}�N�����܂Ƃ߂Ď�舵��
	bool SaveKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) const;	// Macro�̗���A�L�[�{�[�h�}�N���ɕۑ�
	//@@@2002.2.2 YAZAKI PPA.DLL�A��/�i�V�����̂���virtual�ɁB
	// 2007.07.20 genta flags�ǉ�
	virtual bool ExecKeyMacro(class EditView* pEditView, int flags) const;	// �L�[�{�[�h�}�N���̎��s
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);		// �L�[�{�[�h�}�N�����t�@�C������ǂݍ���
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);	// �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ���
	
	// Apr. 29, 2002 genta
	static MacroManagerBase* Creator(const TCHAR* ext);
	static void declare(void);

protected:
	Macro*	m_pTop;	// �擪�ƏI�[��ێ�
	Macro*	m_pBot;
};


