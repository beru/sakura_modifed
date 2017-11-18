/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f
*/

class FuncInfo;

#pragma once

#include "mem/Memory.h"

// CDlgFuncList::SetTree()�p Info
#define FUNCINFO_INFOMASK	0xFFFF
#define FUNCINFO_NOCLIPTEXT 0x10000

// �A�E�g���C�����  �f�[�^�v�f
class FuncInfo {
public:
	FuncInfo(size_t, size_t, size_t, size_t, const TCHAR*, const TCHAR*, int);	// FuncInfo�N���X�\�z
	~FuncInfo();	// FuncInfo�N���X����

	// �N���b�v�{�[�h�ɒǉ�����v�f���H
	inline bool IsAddClipText(void) const {
		return (FUNCINFO_NOCLIPTEXT != (nInfo & FUNCINFO_NOCLIPTEXT));
	}

//	private:
	size_t		nFuncLineCRLF;		// �֐��̂���s(CRLF�P��)
	size_t		nFuncLineLAYOUT;	// �֐��̂���s(�܂�Ԃ��P��)
	size_t		nFuncColCRLF;		// �֐��̂��錅(CRLF�P��)
	size_t		nFuncColLAYOUT;		// �֐��̂��錅(�܂�Ԃ��P��)
	NativeT		memFuncName;		// �֐���
	NativeT		memFileName;		// �t�@�C����
	int			nInfo;				// �t�����
	size_t		nDepth;				// �[��
};

