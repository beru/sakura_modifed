#include "StdAfx.h"
#include "Ope.h"
#include "mem/Memory.h"// 2002/2/10 aroka


// Ope�N���X�\�z
Ope::Ope(OpeCode eCode)
{
	assert( eCode != OpeCode::Unknown );
	nOpe = eCode;		// ������

	ptCaretPos_PHY_Before.Set(-1, -1);	// �J�[�\���ʒu
	ptCaretPos_PHY_After.Set(-1, -1);	// �J�[�\���ʒu

}

// Ope�N���X����
Ope::~Ope()
{
}

// �ҏW����v�f�̃_���v
void Ope::DUMP(void)
{
	DEBUG_TRACE(_T("\t\tnOpe                  = [%d]\n"), nOpe               );
	DEBUG_TRACE(_T("\t\tptCaretPos_PHY_Before = [%d,%d]\n"), ptCaretPos_PHY_Before.x, ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE(_T("\t\tptCaretPos_PHY_After  = [%d,%d]\n"), ptCaretPos_PHY_After.x, ptCaretPos_PHY_After.y   );
	return;
}

// �ҏW����v�f�̃_���v
void DeleteOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tptCaretPos_PHY_To     = [%d,%d]\n"), ptCaretPos_PHY_To.x, ptCaretPos_PHY_To.y);
	DEBUG_TRACE(_T("\t\topeLineData.size         = [%d]\n"), opeLineData.size());
	for (size_t i=0; i<opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tcOpeLineData[%d].nSeq        = [%d]\n"), opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tcOpeLineData[%d].memLine     = [%ls]\n"), opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

// �ҏW����v�f�̃_���v
void InsertOpe::DUMP(void)
{
	Ope::DUMP();
	DEBUG_TRACE(_T("\t\tcOpeLineData.size         = [%d]\n"), opeLineData.size());
	for (size_t i=0; i<opeLineData.size(); ++i) {
		DEBUG_TRACE(_T("\t\tcOpeLineData[%d].nSeq        = [%d]\n"), opeLineData[i].nSeq);
		DEBUG_TRACE(_T("\t\tcOpeLineData[%d].memLine     = [%ls]\n"), opeLineData[i].memLine.GetStringPtr());		
	}
	return;
}

