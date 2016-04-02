/*!	@file
	@brief Undo, Redo�o�b�t�@

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "OpeBuf.h"
#include "OpeBlk.h"// 2002/2/10 aroka


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// OpeBuf�N���X�\�z
OpeBuf::OpeBuf()
{
	m_nCurrentPointer = 0;	// ���݈ʒu
	m_nNoModifiedIndex = 0;	// ���ύX�ȏ�ԂɂȂ����ʒu
}

// OpeBuf�N���X����
OpeBuf::~OpeBuf()
{
	// ����u���b�N�̔z����폜����
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo�\�ȏ�Ԃ�
bool OpeBuf::IsEnableUndo() const
{
	return 0 < m_vCOpeBlkArr.size() && 0 < m_nCurrentPointer;
}

// Redo�\�ȏ�Ԃ�
bool OpeBuf::IsEnableRedo() const
{
	return 0 < m_vCOpeBlkArr.size() && m_nCurrentPointer < (int)m_vCOpeBlkArr.size();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ����̒ǉ�
bool OpeBuf::AppendOpeBlk(OpeBlk* pOpeBlk)
{
	// ���݈ʒu�����iUndo�Ώہj������ꍇ�́A����
	int size = (int)m_vCOpeBlkArr.size();
	if (m_nCurrentPointer < size) {
		for (int i=m_nCurrentPointer; i<size; ++i) {
			SAFE_DELETE(m_vCOpeBlkArr[i]);
		}
		m_vCOpeBlkArr.resize(m_nCurrentPointer);
	}
	// �z��̃������T�C�Y�𒲐�
	m_vCOpeBlkArr.push_back(pOpeBlk);
	++m_nCurrentPointer;
	return true;
}

// �S�v�f�̃N���A
void OpeBuf::ClearAll()
{
	// ����u���b�N�̔z����폜����
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
	m_nCurrentPointer = 0;	// ���݈ʒu
	m_nNoModifiedIndex = 0;	// ���ύX�ȏ�ԂɂȂ����ʒu
}

// ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm
void OpeBuf::SetNoModified()
{
	m_nNoModifiedIndex = m_nCurrentPointer;	// ���ύX�ȏ�ԂɂȂ����ʒu
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �g�p                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���݂�Undo�Ώۂ̑���u���b�N��Ԃ�
OpeBlk* OpeBuf::DoUndo(bool* pbModified)
{
	// Undo�\�ȏ�Ԃ�
	if (!IsEnableUndo()) {
		return nullptr;
	}
	--m_nCurrentPointer;
	if (m_nCurrentPointer == m_nNoModifiedIndex) {		// ���ύX�ȏ�ԂɂȂ����ʒu
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return m_vCOpeBlkArr[m_nCurrentPointer];
}

// ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�
OpeBlk* OpeBuf::DoRedo(bool* pbModified)
{
	// Redo�\�ȏ�Ԃ�
	if (!IsEnableRedo()) {
		return nullptr;
	}
	OpeBlk* pOpeBlk = m_vCOpeBlkArr[m_nCurrentPointer];
	++m_nCurrentPointer;
	if (m_nCurrentPointer == m_nNoModifiedIndex) {		// ���ύX�ȏ�ԂɂȂ����ʒu
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return pOpeBlk;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo, Redo�o�b�t�@�̃_���v
void OpeBuf::DUMP()
{
#ifdef _DEBUG
	MYTRACE(_T("OpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer);
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("OpeBuf.m_vCOpeBlkArr[%d]----\n"), i);
		m_vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE(_T("OpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer);
#endif
}

