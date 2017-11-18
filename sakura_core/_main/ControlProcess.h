#pragma once

#include "global.h"
#include "Process.h"

class ControlTray;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �R���g���[���v���Z�X�N���X
	
	�R���g���[���v���Z�X��ControlTray�N���X�̃C���X�^���X�����B
*/
class ControlProcess : public Process {
public:
	ControlProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine) :
		Process(hInstance, lpCmdLine),
		hMutex(NULL),
		hMutexCP(NULL),
		hEventCPInitialized(NULL),
		pTray(0)
	{
	}

	virtual ~ControlProcess();
protected:
	ControlProcess();
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

private:
	HANDLE			hMutex;					// �A�v���P�[�V�������s���o�p�~���[�e�b�N�X
	HANDLE			hMutexCP;				// �R���g���[���v���Z�X�~���[�e�b�N�X
	HANDLE			hEventCPInitialized;	// �R���g���[���v���Z�X�����������C�x���g
	ControlTray*	pTray;
};

