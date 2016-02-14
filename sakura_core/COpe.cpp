/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "COpe.h"
#include "mem/CMemory.h"// 2002/2/10 aroka


// Opeクラス構築
Ope::Ope(OpeCode eCode)
{
	assert( eCode != OpeCode::Unknown );
	m_nOpe = eCode;		// 操作種別

	m_ptCaretPos_PHY_Before.Set(LogicInt(-1), LogicInt(-1));	// カーソル位置
	m_ptCaretPos_PHY_After.Set(LogicInt(-1), LogicInt(-1));	// カーソル位置

}

// Opeクラス消滅
Ope::~Ope()
{
}

// 編集操作要素のダンプ
void Ope::DUMP(void)
{
	DEBUG_TRACE(_T("\t\tm_nOpe                  = [%d]\n"), m_nOpe               );
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_Before = [%d,%d]\n"), m_ptCaretPos_PHY_Before.x, m_ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_After  = [%d,%d]\n"), m_ptCaretPos_PHY_After.x, m_ptCaretPos_PHY_After.y   );
	return;
}

// 編集操作要素のダンプ
void DeleteOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tm_ptCaretPos_PHY_To     = [%d,%d]\n"), m_ptCaretPos_PHY_To.x, m_ptCaretPos_PHY_To.y);
	DEBUG_TRACE(_T("\t\tm_opeLineData.size         = [%d]\n"), m_opeLineData.size());
	for (size_t i=0; i<m_opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].nSeq         = [%d]\n"), m_opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].cmemLine     = [%ls]\n"), m_opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

// 編集操作要素のダンプ
void InsertOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tm_cOpeLineData.size         = [%d]\n"), m_opeLineData.size());
	for (size_t i=0; i<m_opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].nSeq         = [%d]\n"), m_opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tm_cOpeLineData[%d].cmemLine     = [%ls]\n"), m_opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

