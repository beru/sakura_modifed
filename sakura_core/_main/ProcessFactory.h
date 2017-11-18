/*!	@file
	@brief �v���Z�X�����N���X�w�b�_�t�@�C��
*/

#pragma once

#include "global.h"

class Process;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �v���Z�X�����N���X

	�^����ꂽ�R�}���h���C���������琶�����ׂ��v���Z�X�̎�ʂ𔻒肵�C
	�Ή�����I�u�W�F�N�g��Ԃ�Factory�N���X�D

	�ʏ�̃G�f�B�^�v���Z�X�̋N�����w�肳�ꂽ�ꍇ�ɂ́C�K�v�ɉ����ăR���g���[���v���Z�X
	�N���̋N�����G�f�B�^�̋N���ɐ旧���čs���D
*/
class ProcessFactory {
public:
	Process* Create(HINSTANCE hInstance, LPCTSTR lpCmdLine);
protected:
private:
	bool IsValidVersion();
	bool ProfileSelect(HINSTANCE, LPCTSTR);
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
	bool WaitForInitializedControlProcess();
	bool TestWriteQuit();
};

