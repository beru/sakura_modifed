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
#include "Eol.h"
#include "mem/Memory.h"

#include "docplus/BookmarkManager.h"
#include "docplus/DiffManager.h"
#include "docplus/ModifyManager.h"
#include "docplus/FuncListManager.h"

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
	LogicInt		GetLengthWithoutEOL() const			{ return line.GetStringLength() - eol.GetLen(); } // �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr() const						{ return line.GetStringPtr(); }
	LogicInt		GetLengthWithEOL() const			{ return line.GetStringLength(); }	// CMemoryIterator�p
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
	const Eol& GetEol() const { return eol; }
	void SetEol(const Eol& eol, OpeBlk* pOpeBlk);
	void SetEol(); // ���݂̃o�b�t�@����ݒ�

	const NativeW& _GetDocLineDataWithEOL() const { return line; } //###��
	NativeW& _GetDocLineData() { return line; }

	// �f�[�^�ݒ�
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const NativeW& data);
	void SetDocLineStringMove(NativeW* pData);

	// �`�F�[������
	DocLine* GetPrevLine() { return pPrev; }
	const DocLine* GetPrevLine() const { return pPrev; }
	DocLine* GetNextLine() { return pNext; }
	const DocLine* GetNextLine() const { return pNext; }
	void _SetPrevLine(DocLine* pDocLine) { pPrev = pDocLine; }
	void _SetNextLine(DocLine* pDocLine) { pNext = pDocLine; }
	
private: //####
	DocLine*	pPrev;	// ��O�̗v�f
	DocLine*	pNext;	// ���̗v�f
private:
	NativeW	line;	// �f�[�^  2007.10.11 kobake �|�C���^�ł͂Ȃ��A���̂����悤�ɕύX
	Eol		eol;		// �s���R�[�h
public:
	// �g����� $$������
	struct MarkType {
		LineModified	modified;	// �ύX�t���O
		LineBookmarked	bookmarked;	// �u�b�N�}�[�N
		LineFuncList	funcList;	// �֐����X�g�}�[�N
		LineDiffed		diffMarked;	// DIFF�������
	};
	MarkType mark;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLine);
};

