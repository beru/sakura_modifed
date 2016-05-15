/*!	@file
	@brief Layout��DocLine�̃C�e���[�^

	@author Yazaki
	@date 2002/09/25 �V�K�쐬
*/
/*
	Copyright (C) 2002, Yazaki
	Copyright (C) 2003, genta
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

//	sakura
#include "_main/global.h"
#include "charset/charcode.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// 2007.10.23 kobake �e���v���[�g�ł���K�v�������̂ŁA��e���v���[�g�ɕύX�B

#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"

// �u���b�N�R�����g�f���~�^���Ǘ�����
class MemoryIterator {
public:
	// DocLine�p�R���X�g���N�^
	MemoryIterator(const DocLine* pcT, LayoutInt nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(LayoutInt(0))
	{
		first();
	}

	// Layout�p�R���X�g���N�^
	MemoryIterator(const Layout* pcT, LayoutInt nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(pcT ? pcT->GetIndent() : LayoutInt(0))
	{
		first();
	}

	// ���ʒu���s�̐擪�ɃZ�b�g
	void first() {
		nIndex = LogicInt(0);
		nColumn = nIndent;
		nIndex_Delta = LogicInt(0);
		nColumn_Delta = LayoutInt(0);
	}

	/*! �s�����ǂ���
		@return true: �s��, false: �s���ł͂Ȃ�
	 */
	bool end() const {
		return (nLineLen <= nIndex);
	}

	// ���̕������m�F���Ď��̕����Ƃ̍������߂�
	void scanNext() {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		// 2007.09.04 kobake UNICODE���F�f�[�^�����ƌ�������ʁX�̒l�Ƃ��Čv�Z����B

		// �f�[�^�������v�Z
		nIndex_Delta = LogicInt(NativeW::GetSizeOfChar(pLine, nLineLen, nIndex));
		if (nIndex_Delta == 0) {
			nIndex_Delta = LogicInt(1);
		}

		// ���������v�Z
		if (pLine[nIndex] == WCODE::TAB) {
			nColumn_Delta = nTabSpace - (nColumn % nTabSpace);
		}else {
			nColumn_Delta = LayoutInt(NativeW::GetKetaOfChar(pLine, nLineLen, nIndex));
//			if (nColumn_Delta == 0) {				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//				nColumn_Delta = LayoutInt(1);
//			}
		}
	}
	
	/*! �\�ߌv�Z�������������ʒu�ɉ�����D
		@sa scanNext()
	 */
	void addDelta() {
		nColumn += nColumn_Delta;
		nIndex += nIndex_Delta;
	}	// �|�C���^�����炷
	
	LogicInt	getIndex()			const {	return nIndex;	}
	LayoutInt	getColumn()			const {	return nColumn;	}
	LogicInt	getIndexDelta()		const {	return nIndex_Delta;	}
	LayoutInt	getColumnDelta()	const {	return nColumn_Delta;	}

	// 2002.10.07 YAZAKI
	const wchar_t getCurrentChar() {	return pLine[nIndex];	}
	// Jul. 20, 2003 genta �ǉ�
	// memcpy������̂Ƀ|�C���^���Ƃ�Ȃ��Ɩʓ|
	const wchar_t* getCurrentPos() {	return pLine + nIndex;	}


private:
	// �R���X�g���N�^�Ŏ󂯎�����p�����[�^ (�Œ�)
	const wchar_t*		pLine;
	const int			nLineLen;  // �f�[�^���B�����P�ʁB
	const LayoutInt	nTabSpace;
	const LayoutInt	nIndent;

	// ��ԕϐ�
	LogicInt	nIndex;        // �f�[�^�ʒu�B�����P�ʁB
	LayoutInt	nColumn;       // ���C�A�E�g�ʒu�B��(���p��)�P�ʁB
	LogicInt	nIndex_Delta;  // index����
	LayoutInt	nColumn_Delta; // column����

};

