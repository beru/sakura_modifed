/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "global.h"
#include "Process.h"
#include "extmodule/Migemo.h"
#include "EditApp.h"
#include "util/design_template.h"
class EditWnd;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �G�f�B�^�v���Z�X�N���X
	
	�G�f�B�^�v���Z�X��EditWnd�N���X�̃C���X�^���X�����B
*/
class NormalProcess : public Process {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	NormalProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine);
	virtual ~NormalProcess();

protected:
	// �v���Z�X�n���h��
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

protected:
	// �����⏕
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka

private:
	EditApp* pEditApp;	// 2007.10.23 kobake
};

