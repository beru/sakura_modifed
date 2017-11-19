#pragma once

#include <Windows.h>
#include "_main/global.h"
#include "doc/DocListener.h"

// �����~���b�ɕϊ����邽�߂̌W��
const int MSec2Min = 1000 * 60;

/*! @class PassiveTimer CAutoSave.h
	���������̌o�ߎ��Ԃ��ݒ�Ԋu���߂������ǂ����𔻒肷��B
	�p�ɂɌĂяo�����^�C�}�[�����ɕʂ̏ꏊ�ɂ���Ƃ��A��������Ԋu���L����
	�Ԋu�̌��������v������Ȃ��p�r�ɗ��p�\�B
	�t�@�C���̎����ۑ��Ŏg���Ă���B
*/
class PassiveTimer {
public:
	/*!
		�����l�͊Ԋu1msec�Ń^�C�}�[�͖����B
	*/
	PassiveTimer() : nInterval(1), bEnabled(false) { Reset(); }

	// ���ԊԊu
	void SetInterval(int m);	// ���ԊԊu�̐ݒ�
	int GetInterval(void) const { return nInterval / MSec2Min; }	// ���ԊԊu�̎擾
	void Reset(void) { nLastTick = ::GetTickCount(); }			// ������̃��Z�b�g

	// �L���^����
	void Enable(bool flag);							// �L���^�����̐ݒ�
	bool IsEnabled(void) const { return bEnabled; }	// �L���^�����̓ǂݏo��

	// �K�莞�ԂɒB�������ǂ����̔���
	bool CheckAction(void);

private:
	DWORD	nLastTick;	// �Ō�Ƀ`�F�b�N�����Ƃ��̎��� (GetTickCount()�Ŏ擾��������)
	int		nInterval;	// Action�Ԋu (��)
	bool	bEnabled;	// �L�����ǂ���
};


class AutoSaveAgent : public DocListenerEx {
public:
	void CheckAutoSave();
	void ReloadAutoSaveParam();	// �ݒ��SharedArea����ǂݏo��

private:
	PassiveTimer passiveTimer;
};

