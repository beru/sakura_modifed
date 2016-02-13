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
#include "StdAfx.h"
#include <stdlib.h>
#include "COpeBlk.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

OpeBlk::OpeBlk()
{
	m_refCount = 0;
}

OpeBlk::~OpeBlk()
{
	// ����̔z����폜����
	int size = (int)m_ppCOpeArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(m_ppCOpeArr[i]);
	}
	m_ppCOpeArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ����̒ǉ�
bool OpeBlk::AppendOpe(Ope* pOpe)
{
	if (pOpe->m_ptCaretPos_PHY_Before.HasNegative() || pOpe->m_ptCaretPos_PHY_After.HasNegative()) {
		TopErrorMessage(NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("Bug.\n")
			_T("pOpe->m_ptCaretPos_PHY_Before = %d,%d\n")
			_T("pOpe->m_ptCaretPos_PHY_After = %d,%d\n"),
			pOpe->m_ptCaretPos_PHY_Before.x,
			pOpe->m_ptCaretPos_PHY_Before.y,
			pOpe->m_ptCaretPos_PHY_After.x,
			pOpe->m_ptCaretPos_PHY_After.y
		);
	}

	// �z��̃������T�C�Y�𒲐�
	m_ppCOpeArr.push_back(pOpe);
	return true;
}


// �����Ԃ�
Ope* OpeBlk::GetOpe(int nIndex)
{
	if (GetNum() <= nIndex) {
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ҏW����v�f�u���b�N�̃_���v
void OpeBlk::DUMP(void)
{
#ifdef _DEBUG
	int size = GetNum();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i);
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}

