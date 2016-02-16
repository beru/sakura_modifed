/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "Ope.h"
#include "mem/Memory.h"// 2002/2/10 aroka


// Ope�N���X�\�z
Ope::Ope(OpeCode eCode)
{
	assert( eCode != OpeCode::Unknown );
	m_nOpe = eCode;		// ������

	m_ptCaretPos_PHY_Before.Set(LogicInt(-1), LogicInt(-1));	// �J�[�\���ʒu
	m_ptCaretPos_PHY_After.Set(LogicInt(-1), LogicInt(-1));	// �J�[�\���ʒu

}

// Ope�N���X����
Ope::~Ope()
{
}

// �ҏW����v�f�̃_���v
void Ope::DUMP(void)
{
	DEBUG_TRACE(_T("\t\tm_nOpe                  = [%d]\n"), m_nOpe               );
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_Before = [%d,%d]\n"), m_ptCaretPos_PHY_Before.x, m_ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_After  = [%d,%d]\n"), m_ptCaretPos_PHY_After.x, m_ptCaretPos_PHY_After.y   );
	return;
}

// �ҏW����v�f�̃_���v
void DeleteOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_To     = [%d,%d]\n"), m_ptCaretPos_PHY_To.x, m_ptCaretPos_PHY_To.y);
	DEBUG_TRACE(_T("\t\tm_opeLineData.size         = [%d]\n"), m_opeLineData.size());
	for (size_t i=0; i<m_opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].nSeq        = [%d]\n"), m_opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].memLine     = [%ls]\n"), m_opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

// �ҏW����v�f�̃_���v
void InsertOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tm_cOpeLineData.size         = [%d]\n"), m_opeLineData.size());
	for (size_t i=0; i<m_opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].nSeq        = [%d]\n"), m_opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].memLine     = [%ls]\n"), m_opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

