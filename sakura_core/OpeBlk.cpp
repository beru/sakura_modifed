#include "StdAfx.h"
#include <stdlib.h>
#include "OpeBlk.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

OpeBlk::OpeBlk()
{
	refCount = 0;
}

OpeBlk::~OpeBlk()
{
	// ����̔z����폜����
	size_t size = ppCOpeArr.size();
	for (size_t i=0; i<size; ++i) {
		SAFE_DELETE(ppCOpeArr[i]);
	}
	ppCOpeArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ����̒ǉ�
bool OpeBlk::AppendOpe(Ope* pOpe)
{
	if (pOpe->ptCaretPos_PHY_Before.HasNegative() || pOpe->ptCaretPos_PHY_After.HasNegative()) {
		TopErrorMessage(NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("Bug.\n")
			_T("pOpe->ptCaretPos_PHY_Before = %d,%d\n")
			_T("pOpe->ptCaretPos_PHY_After = %d,%d\n"),
			pOpe->ptCaretPos_PHY_Before.x,
			pOpe->ptCaretPos_PHY_Before.y,
			pOpe->ptCaretPos_PHY_After.x,
			pOpe->ptCaretPos_PHY_After.y
		);
	}

	// �z��̃������T�C�Y�𒲐�
	ppCOpeArr.push_back(pOpe);
	return true;
}


// �����Ԃ�
Ope* OpeBlk::GetOpe(size_t nIndex)
{
	if (GetNum() <= nIndex) {
		return nullptr;
	}
	return ppCOpeArr[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ҏW����v�f�u���b�N�̃_���v
void OpeBlk::DUMP(void)
{
#ifdef _DEBUG
	size_t size = GetNum();
	for (size_t i=0; i<size; ++i) {
		MYTRACE(_T("\tCOpeBlk.ppCOpeArr[%d]----\n"), i);
		ppCOpeArr[i]->DUMP();
	}
#endif
}

