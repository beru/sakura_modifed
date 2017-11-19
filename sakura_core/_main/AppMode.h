#pragma once

#include "util/design_template.h"
#include "doc/DocListener.h"

class AppMode
	:
	public TSingleton<AppMode>,
	public DocListenerEx
{ // ###��
	friend class TSingleton<AppMode>;
	AppMode()
		:
		bViewMode(false),	// �r���[���[�h
		bDebugMode(false)	// �f�o�b�O���j�^���[�h
	{
		szGrepKey[0] = 0;
	}

public:
	// �C���^�[�t�F�[�X
	bool	IsViewMode() const				{ return bViewMode; }				// �r���[���[�h���擾
	void	SetViewMode(bool bViewMode)		{ this->bViewMode = bViewMode; }	// �r���[���[�h��ݒ�
	bool	IsDebugMode() const				{ return bDebugMode; }
	void	SetDebugModeON();	// �f�o�b�O���j�^���[�h�ݒ�
	void	SetDebugModeOFF();	// �f�o�b�O���j�^���[�h����

	// �C�x���g
	void OnAfterSave(const SaveInfo& saveInfo);

protected:
	void _SetDebugMode(bool bDebugMode) { this->bDebugMode = bDebugMode; }

private:
	bool	bViewMode;				// �r���[���[�h
	bool	bDebugMode;				// �f�o�b�O���j�^���[�h
public:
	wchar_t	szGrepKey[1024];		// Grep���[�h�̏ꍇ�A���̌����L�[
};

