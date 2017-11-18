#pragma once

class OpeBuf;

// Undo, Redo�o�b�t�@

#include <vector>
#include "_main/global.h"
class OpeBlk;/// 2002/2/10 aroka


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief Undo, Redo�o�b�t�@
*/
class OpeBuf {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	OpeBuf();
	~OpeBuf();

	// ���
	bool IsEnableUndo() const;					// Undo�\�ȏ�Ԃ�
	bool IsEnableRedo() const;					// Redo�\�ȏ�Ԃ�
	int GetCurrentPointer(void) const { return nCurrentPointer; }	// ���݈ʒu��Ԃ�	// 2007.12.09 ryoji
	int GetNextSeq() const { return nCurrentPointer + 1; }
	int GetNoModifiedSeq() const { return nNoModifiedIndex; }

	// ����
	void ClearAll();							// �S�v�f�̃N���A
	bool AppendOpeBlk(OpeBlk* pOpeBlk);			// ����u���b�N�̒ǉ�
	void SetNoModified();						// ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm

	// �g�p
	OpeBlk* DoUndo(bool* pbModified);			// ���݂�Undo�Ώۂ̑���u���b�N��Ԃ�
	OpeBlk* DoRedo(bool* pbModified);			// ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�

	// �f�o�b�O
	void DUMP();								// �ҏW����v�f�u���b�N�̃_���v

private:
	std::vector<OpeBlk*>	vCOpeBlkArr;		// ����u���b�N�̔z��
	int						nCurrentPointer;	// ���݈ʒu
	int						nNoModifiedIndex;	// ���ύX�ȏ�ԂɂȂ����ʒu
};


