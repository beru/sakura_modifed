#include "StdAfx.h"
#include "OpeBuf.h"
#include "OpeBlk.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// OpeBuf�N���X�\�z
OpeBuf::OpeBuf()
{
	nCurrentPointer = 0;	// ���݈ʒu
	nNoModifiedIndex = 0;	// ���ύX�ȏ�ԂɂȂ����ʒu
}

// OpeBuf�N���X����
OpeBuf::~OpeBuf()
{
	// ����u���b�N�̔z����폜����
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vCOpeBlkArr[i]);
	}
	vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo�\�ȏ�Ԃ�
bool OpeBuf::IsEnableUndo() const
{
	return 0 < vCOpeBlkArr.size() && 0 < nCurrentPointer;
}

// Redo�\�ȏ�Ԃ�
bool OpeBuf::IsEnableRedo() const
{
	return 0 < vCOpeBlkArr.size() && nCurrentPointer < (int)vCOpeBlkArr.size();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ����̒ǉ�
bool OpeBuf::AppendOpeBlk(OpeBlk* pOpeBlk)
{
	// ���݈ʒu�����iUndo�Ώہj������ꍇ�́A����
	int size = (int)vCOpeBlkArr.size();
	if (nCurrentPointer < size) {
		for (int i=nCurrentPointer; i<size; ++i) {
			SAFE_DELETE(vCOpeBlkArr[i]);
		}
		vCOpeBlkArr.resize(nCurrentPointer);
	}
	// �z��̃������T�C�Y�𒲐�
	vCOpeBlkArr.push_back(pOpeBlk);
	++nCurrentPointer;
	return true;
}

// �S�v�f�̃N���A
void OpeBuf::ClearAll()
{
	// ����u���b�N�̔z����폜����
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vCOpeBlkArr[i]);
	}
	vCOpeBlkArr.clear();
	nCurrentPointer = 0;	// ���݈ʒu
	nNoModifiedIndex = 0;	// ���ύX�ȏ�ԂɂȂ����ʒu
}

// ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm
void OpeBuf::SetNoModified()
{
	nNoModifiedIndex = nCurrentPointer;	// ���ύX�ȏ�ԂɂȂ����ʒu
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
	--nCurrentPointer;
	if (nCurrentPointer == nNoModifiedIndex) {		// ���ύX�ȏ�ԂɂȂ����ʒu
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return vCOpeBlkArr[nCurrentPointer];
}

// ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�
OpeBlk* OpeBuf::DoRedo(bool* pbModified)
{
	// Redo�\�ȏ�Ԃ�
	if (!IsEnableRedo()) {
		return nullptr;
	}
	OpeBlk* pOpeBlk = vCOpeBlkArr[nCurrentPointer];
	++nCurrentPointer;
	if (nCurrentPointer == nNoModifiedIndex) {		// ���ύX�ȏ�ԂɂȂ����ʒu
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
	MYTRACE(_T("OpeBuf.nCurrentPointer=[%d]----\n"), nCurrentPointer);
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("OpeBuf.vCOpeBlkArr[%d]----\n"), i);
		vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE(_T("OpeBuf.nCurrentPointer=[%d]----\n"), nCurrentPointer);
#endif
}

