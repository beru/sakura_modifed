/*!	@file
	@brief �����f�[�^1�s

	@author Norio Nakatani

	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ��������o�[�ǉ�
	@date 2001/12/18 hor bookmark, �C���t���O�̃A�N�Z�X�֐���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#pragma once

#include "util/design_template.h"
#include "CEol.h"
#include "mem/CMemory.h"

#include "docplus/CBookmarkManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CModifyManager.h"
#include "docplus/CFuncListManager.h"

class DocLine;
class OpeBlk;

// �����f�[�^1�s
class DocLine {
protected:
	friend class DocLineMgr; //######��
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	DocLine();
	~DocLine();

	// ����
	bool			IsEmptyLine() const;		// ����DocLine����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ����B

	// �f�[�^�擾
	LogicInt		GetLengthWithoutEOL() const			{ return m_cLine.GetStringLength() - m_cEol.GetLen(); } // �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr() const						{ return m_cLine.GetStringPtr(); }
	LogicInt		GetLengthWithEOL() const			{ return m_cLine.GetStringLength(); }	// CMemoryIterator�p
#ifdef USE_STRICT_INT
	const wchar_t*	GetDocLineStrWithEOL(int* pnLen) const {	//###���̖��O�A���̑Ώ�
		LogicInt n;
		const wchar_t* p = GetDocLineStrWithEOL(&n);
		*pnLen = n;
		return p;
	}
#endif
	const wchar_t*	GetDocLineStrWithEOL(LogicInt* pnLen) const {	//###���̖��O�A���̑Ώ�
		if (this) {
			*pnLen = GetLengthWithEOL();
			return GetPtr();
		}else {
			*pnLen = 0;
			return NULL;
		}
	}
	StringRef GetStringRefWithEOL() const {	//###���̖��O�A���̑Ώ�
		if (this) {
			return StringRef(GetPtr(), GetLengthWithEOL());
		}else {
			return StringRef(NULL, 0);
		}
	}
	const Eol& GetEol() const { return m_cEol; }
	void SetEol(const Eol& cEol, OpeBlk* pcOpeBlk);
	void SetEol(); // ���݂̃o�b�t�@����ݒ�

	const NativeW& _GetDocLineDataWithEOL() const { return m_cLine; } //###��
	NativeW& _GetDocLineData() { return m_cLine; }

	// �f�[�^�ݒ�
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const NativeW& cData);
	void SetDocLineStringMove(NativeW* pcData);

	// �`�F�[������
	DocLine* GetPrevLine() { return m_pPrev; }
	const DocLine* GetPrevLine() const { return m_pPrev; }
	DocLine* GetNextLine() { return m_pNext; }
	const DocLine* GetNextLine() const { return m_pNext; }
	void _SetPrevLine(DocLine* pcDocLine) { m_pPrev = pcDocLine; }
	void _SetNextLine(DocLine* pcDocLine) { m_pNext = pcDocLine; }
	
private: //####
	DocLine*	m_pPrev;	// ��O�̗v�f
	DocLine*	m_pNext;	// ���̗v�f
private:
	NativeW	m_cLine;	// �f�[�^  2007.10.11 kobake �|�C���^�ł͂Ȃ��A���̂����悤�ɕύX
	Eol		m_cEol;		// �s���R�[�h
public:
	// �g����� $$������
	struct MarkType {
		LineModified	m_cModified;	// �ύX�t���O
		LineBookmarked	m_cBookmarked;	// �u�b�N�}�[�N
		LineFuncList	m_cFuncList;	//�֐����X�g�}�[�N
		LineDiffed		m_cDiffmarked;	// DIFF�������
	};
	MarkType m_sMark;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLine);
};

