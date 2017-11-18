// �����f�[�^1�s

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
	size_t			GetLengthWithoutEOL() const			{ return line.GetStringLength() - eol.GetLen(); } // �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr() const						{ return line.GetStringPtr(); }
	size_t			GetLengthWithEOL() const			{ return line.GetStringLength(); }	// CMemoryIterator�p
	const wchar_t*	GetDocLineStrWithEOL(size_t* pnLen) const {	//###���̖��O�A���̑Ώ�
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

