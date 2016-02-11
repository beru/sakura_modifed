/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class FuncInfo;

#pragma once

#include "mem/CMemory.h"

// CDlgFuncList::SetTree()�p m_Info
#define FUNCINFO_INFOMASK	0xFFFF
//	2003.06.27 Moca
#define FUNCINFO_NOCLIPTEXT 0x10000

//! �A�E�g���C�����  �f�[�^�v�f
//@date 2002.04.01 YAZAKI �[������
class FuncInfo {
public:
	FuncInfo(LogicInt, LogicInt, LayoutInt, LayoutInt, const TCHAR*, const TCHAR*, int);	// FuncInfo�N���X�\�z
	~FuncInfo();	// FuncInfo�N���X����

	//! �N���b�v�{�[�h�ɒǉ�����v�f���H
	//	2003.06.27 Moca
	inline bool IsAddClipText(void) const {
		return (FUNCINFO_NOCLIPTEXT != (m_nInfo & FUNCINFO_NOCLIPTEXT));
	}

//	private:
	LogicInt	m_nFuncLineCRLF;	// �֐��̂���s(CRLF�P��)
	LayoutInt	m_nFuncLineLAYOUT;	// �֐��̂���s(�܂�Ԃ��P��)
	LogicInt	m_nFuncColCRLF;		// �֐��̂��錅(CRLF�P��)
	LayoutInt	m_nFuncColLAYOUT;	// �֐��̂��錅(�܂�Ԃ��P��)
	CNativeT	m_memFuncName;		// �֐���
	CNativeT	m_memFileName;		// �t�@�C����
	int			m_nInfo;			// �t�����
	int			m_nDepth;			// �[��
};

