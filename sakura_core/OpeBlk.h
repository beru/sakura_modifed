/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class OpeBlk;

#pragma once

#include "Ope.h"
#include <vector>



/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �ҏW����v�f�u���b�N
	
	Ope �𕡐����˂邽�߂̂��́BUndo, Redo�͂��̃u���b�N�P�ʂōs����B
*/
class OpeBlk {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	OpeBlk();
	~OpeBlk();

	// �C���^�[�t�F�[�X
	size_t GetNum() const { return ppCOpeArr.size(); }	// ����̐���Ԃ�
	bool AppendOpe(Ope* pOpe);							// ����̒ǉ�
	Ope* GetOpe(size_t nIndex);								// �����Ԃ�
	void AddRef() { ++refCount; }	// �Q�ƃJ�E���^����
	int Release() { return refCount > 0 ? --refCount : 0; }	// �Q�ƃJ�E���^����
	int GetRefCount() const { return refCount; }	// �Q�ƃJ�E���^�擾
	int SetRefCount(int val) {  return refCount = val > 0? val : 0; }	// �Q�ƃJ�E���^�ݒ�

	// �f�o�b�O
	void DUMP();									// �ҏW����v�f�u���b�N�̃_���v

private:
	// �����o�ϐ�
	std::vector<Ope*>	ppCOpeArr;	// ����̔z��

	// �Q�ƃJ�E���^
	// HandleCommand������ċA�I��HandleCommand���Ă΂��ꍇ�A
	// ������HandleCommand�I������OpeBlk���j������Č㑱�̏����ɉe�����o��̂�h�����߁A
	// �Q�ƃJ�E���^��p���Ĉ�ԊO����HandleCommand�I�����̂�OpeBlk��j������B
	// OpeBlk��new�����Ƃ���AddRef()����̂���@�����A���Ȃ��Ă��g����B
	int refCount;
};


