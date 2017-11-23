/*!	@file
	@brief �}�N���G���W��
*/

#pragma once

#include <Windows.h>
class EditView;

class MacroBeforeAfter {
public:
	MacroBeforeAfter() : nOpeBlkCount(0), bDrawSwitchOld(true) {};
	virtual ~MacroBeforeAfter() {};
	virtual void ExecKeyMacroBefore(class EditView& editView, int flags);
	virtual void ExecKeyMacroAfter(class EditView& editView, int flags, bool bRet);
private:
	int nOpeBlkCount;
	bool bDrawSwitchOld;
};

/*!
	@brief �}�N������������G���W�������̊��N���X
*/
class MacroManagerBase : MacroBeforeAfter {
public:

	/*! �L�[�{�[�h�}�N���̎��s
	
		@param[in] pcEditView �}�N�����s�Ώۂ̕ҏW�E�B���h�E
		@param[in] flags �}�N�����s�����D
	*/
	virtual bool ExecKeyMacro(class EditView& editView, int flags) const = 0;
	virtual void ExecKeyMacro2(class EditView& editView, int flags);
	
	/*! �L�[�{�[�h�}�N�����t�@�C������ǂݍ���

		@param hInstance [in]
		@param pszPath [in] �t�@�C����
	*/
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) = 0;

	/*! �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ���

		@param hInstance [in]
		@param pszCode [in] �}�N���R�[�h
	*/
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode) = 0;

	// static MacroManagerBase* Creator(const char* str);
	// �������z�N���X�͎��̉��ł��Ȃ��̂�Factory�͕s�v�B
	// �p����N���X�ł͕K�v�B
	
	// �f�X�g���N�^��virtual��Y�ꂸ��
	virtual ~MacroManagerBase();
	

protected:
	// Load�ς݂��ǂ�����\���t���O true...Load�ς݁Afalse...��Load�B
	bool nReady;

public:
	/*!	Load�ς݂��ǂ���

		@retval true Load�ς�
		@retval false ��Load
	*/
	bool IsReady() { return nReady; }

	// Constructor
	MacroManagerBase();

};

