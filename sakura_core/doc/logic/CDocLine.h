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
	LogicInt		GetLengthWithoutEOL() const			{ return m_line.GetStringLength() - m_eol.GetLen(); } // �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr() const						{ return m_line.GetStringPtr(); }
	LogicInt		GetLengthWithEOL() const			{ return m_line.GetStringLength(); }	// CMemoryIterator�p
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
	const Eol& GetEol() const { return m_eol; }
	void SetEol(const Eol& cEol, OpeBlk* pcOpeBlk);
	void SetEol(); // ���݂̃o�b�t�@����ݒ�

	const NativeW& _GetDocLineDataWithEOL() const { return m_line; } //###��
	NativeW& _GetDocLineData() { return m_line; }

	// �f�[�^�ݒ�
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const NativeW& cData);
	void SetDocLineStringMove(NativeW* pcData);

	// �`�F�[������
	DocLine* GetPrevLine() { return m_pPrev; }
	const DocLine* GetPrevLine() const { return m_pPrev; }
	DocLine* GetNextLine() { return m_pNext; }
	const DocLine* GetNextLine() const { return m_pNext; }
	void _SetPrevLine(DocLine* pDocLine) { m_pPrev = pDocLine; }
	void _SetNextLine(DocLine* pDocLine) { m_pNext = pDocLine; }
	
private: //####
	DocLine*	m_pPrev;	// ��O�̗v�f
	DocLine*	m_pNext;	// ���̗v�f
private:
	NativeW	m_line;	// �f�[�^  2007.10.11 kobake �|�C���^�ł͂Ȃ��A���̂����悤�ɕύX
	Eol		m_eol;		// �s���R�[�h
public:
	// �g����� $$������
	struct MarkType {
		LineModified	m_modified;		// �ύX�t���O
		LineBookmarked	m_bookmarked;	// �u�b�N�}�[�N
		LineFuncList	m_funcList;		// �֐����X�g�}�[�N
		LineDiffed		m_diffMarked;	// DIFF�������
	};
	MarkType m_mark;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLine);
};

