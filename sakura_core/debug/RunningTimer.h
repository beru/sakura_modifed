#pragma once

// �������v���Ԃ̌v���N���X

#include <windows.h>
// RunningTimer�Ōo�ߎ��Ԃ̑�����s���ꍇ�ɂ̓R�����g���O���Ă�������
//#define TIME_MEASURE

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �������v���Ԃ̌v���N���X

	��`�̐؂�ւ��݂̂Ń^�C�}�[��ON/OFF���s����悤�ɂ��邽�߁C
	���̃N���X�𒼐ڎg�킸�C���ɂ���MY_RUNNINGTIMER��MY_TRACETIME��
	�g�����ƁD
*/
class RunningTimer {
public:
	/*
	||  Constructors
	*/
	RunningTimer(const char* Text = NULL);
	~RunningTimer();

	/*
	|| �֐�
	*/
	void Reset();
	DWORD Read();
	
	void WriteTrace(const char* msg = "") const;

protected:
	DWORD	nStartTime;
	char	szText[100];	// �^�C�}�[��
	int		nDepth;		// ���̃I�u�W�F�N�g�̃l�X�g�̐[��

#if 1 //def _DEBUG
	static int nNestCount;
#endif
};

// Oct. 16, 2002 genta
//#ifdef _DEBUG�`#endif�Œ���͂܂Ȃ��Ă��ȒP�Ƀ^�C�}�[��ON/OFF���s�����߂̃}�N��
#if defined(_DEBUG) && defined(TIME_MEASURE)
  #define MY_TRACETIME(c, m) (c).WriteTrace(m)
  #define MY_RUNNINGTIMER(c, m) RunningTimer c(m)
#else
  #define MY_TRACETIME(c, m)
  #define MY_RUNNINGTIMER(c, m)
#endif

