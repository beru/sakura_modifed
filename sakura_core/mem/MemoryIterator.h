/*!	@file
	@brief Layout��DocLine�̃C�e���[�^
*/
#pragma once

//	sakura
#include "_main/global.h"
#include "charset/charcode.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"

// �u���b�N�R�����g�f���~�^���Ǘ�����
class MemoryIterator {
public:
	// DocLine�p�R���X�g���N�^
	MemoryIterator(const DocLine* pcT, size_t nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(0)
	{
		first();
	}

	// Layout�p�R���X�g���N�^
	MemoryIterator(const Layout* pcT, size_t nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(pcT ? pcT->GetIndent() : 0)
	{
		first();
	}

	// ���ʒu���s�̐擪�ɃZ�b�g
	void first() {
		nIndex = 0;
		nColumn = nIndent;
		nIndex_Delta = 0;
		nColumn_Delta = 0;
	}

	/*! �s�����ǂ���
		@return true: �s��, false: �s���ł͂Ȃ�
	 */
	bool end() const {
		return (nLineLen <= nIndex);
	}

	// ���̕������m�F���Ď��̕����Ƃ̍������߂�
	void scanNext() {

		// �f�[�^�������v�Z
		nIndex_Delta = NativeW::GetSizeOfChar(pLine, nLineLen, nIndex);
		if (nIndex_Delta == 0) {
			nIndex_Delta = 1;
		}

		// ���������v�Z
		if (pLine[nIndex] == WCODE::TAB) {
			nColumn_Delta = nTabSpace - (nColumn % nTabSpace);
		}else {
			nColumn_Delta = NativeW::GetKetaOfChar(pLine, nLineLen, nIndex);
		}
	}
	
	/*! �\�ߌv�Z�������������ʒu�ɉ�����D
		@sa scanNext()
	 */
	void addDelta() {
		nColumn += nColumn_Delta;
		nIndex += nIndex_Delta;
	}	// �|�C���^�����炷
	
	size_t	getIndex()			const {	return nIndex;	}
	size_t	getColumn()			const {	return nColumn;	}
	size_t	getIndexDelta()		const {	return nIndex_Delta;	}
	size_t	getColumnDelta()	const {	return nColumn_Delta;	}

	const wchar_t getCurrentChar() {	return pLine[nIndex];	}
	const wchar_t* getCurrentPos() {	return pLine + nIndex;	}


private:
	// �R���X�g���N�^�Ŏ󂯎�����p�����[�^ (�Œ�)
	const wchar_t*	pLine;
	const size_t	nLineLen;  // �f�[�^���B�����P�ʁB
	const size_t	nTabSpace;
	const size_t	nIndent;

	// ��ԕϐ�
	size_t	nIndex;        // �f�[�^�ʒu�B�����P�ʁB
	size_t	nColumn;       // ���C�A�E�g�ʒu�B��(���p��)�P�ʁB
	size_t	nIndex_Delta;  // index����
	size_t	nColumn_Delta; // column����

};

