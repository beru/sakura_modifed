#pragma once

#include "util/design_template.h"
#include "Eol.h"
#include "doc/logic/DocLine.h"
#include "mem/Memory.h"
#include "LayoutExInfo.h"
#include "view/colors/EColorIndexType.h"

class Layout;
class LayoutMgr;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class Layout {
protected:
	friend class LayoutMgr; //####��
public:
	/*
	||  Constructors
	*/
	Layout(
		const DocLine*	pDocLine,		// ���f�[�^�ւ̎Q��
		Point			ptLogicPos,		// ���f�[�^�Q�ƈʒu
		size_t			nLength,		// ���f�[�^���f�[�^��
		EColorIndexType	nTypePrev,
		int				nTypeIndent,
		LayoutColorInfo*	pColorInfo
		)
		:
		pPrev(nullptr),
		pNext(nullptr),
		pDocLine(pDocLine),
		ptLogicPos(ptLogicPos),
		nLength(nLength),
		nTypePrev(nTypePrev),
		nIndent(nTypeIndent)
	{
		exInfo.SetColorInfo(pColorInfo);
	}
	~Layout();
	void DUMP(void);
	
	// ptLogicPos.x�ŕ␳�������Ƃ̕�����𓾂�
	const wchar_t* GetPtr() const { return pDocLine->GetPtr() + ptLogicPos.x; }
	size_t GetLengthWithEOL() const { return nLength;	}	//	������EOL�͏��1�����ƃJ�E���g�H�H
	size_t GetLengthWithoutEOL() const { return nLength - (eol.GetLen() ? 1 : 0);	}
	//int GetLength() const { return nLength; }	// CMemoryIterator�p�iEOL�܂ށj
	size_t GetIndent() const { return nIndent; }	// ���̃��C�A�E�g�s�̃C���f���g�T�C�Y���擾�B�P�ʂ͔��p�����B	CMemoryIterator�p

	// �擾�C���^�[�t�F�[�X
	int GetLogicLineNo() const { return this ? ptLogicPos.y : -1; } //$$$������
	int GetLogicOffset() const { return ptLogicPos.x; }
	Point GetLogicPos() const { return ptLogicPos; }
	EColorIndexType GetColorTypePrev() const { return nTypePrev; } //#########����
	size_t GetLayoutWidth() const { return nLayoutWidth; }		// ���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g����Ԃ�

	// �ύX�C���^�[�t�F�[�X
	void OffsetLogicLineNo(int n) { ptLogicPos.y += n; }
	void SetColorTypePrev(EColorIndexType n) {
		nTypePrev = n;
	}
	void SetLayoutWidth(size_t nWidth) { nLayoutWidth = nWidth; }

	// ���C�A�E�g�����v�Z�B���s�͊܂܂Ȃ�
	size_t CalcLayoutWidth(const LayoutMgr& layoutMgr) const;

	// �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾
	int CalcLayoutOffset(const LayoutMgr& layoutMgr,
							int nStartPos = 0,
							int nStartOffset = 0) const;

	// ������Q�Ƃ��擾
	StringRef GetStringRef() const { return StringRef(GetPtr(), GetLengthWithEOL()); }

	// �`�F�[������
	Layout* GetPrevLayout() { return pPrev; }
	const Layout* GetPrevLayout() const { return pPrev; }
	Layout* GetNextLayout() { return pNext; }
	const Layout* GetNextLayout() const { return pNext; }
	void _SetPrevLayout(Layout* pLayout) { pPrev = pLayout; }
	void _SetNextLayout(Layout* pLayout) { pNext = pLayout; }

	// ���f�[�^�Q��
	const DocLine* GetDocLineRef() const { if (this) return pDocLine; else return nullptr; } //$$note:������

	// ���̑������Q��
	const Eol& GetLayoutEol() const { return eol; }
	const LayoutColorInfo* GetColorInfo() const { return exInfo.GetColorInfo(); }
	LayoutExInfo* GetLayoutExInfo() {
		return &exInfo;
	}
	
private:
	Layout*			pPrev;
	Layout*			pNext;

	// �f�[�^�Q�Ɣ͈�
	const DocLine*		pDocLine;		// ���f�[�^�ւ̎Q��
	Point				ptLogicPos;		// �Ή����郍�W�b�N�Q�ƈʒu
	size_t				nLength;		// ���̃��C�A�E�g�s�̒����B�����P�ʁB
	
	// ���̑�����
	EColorIndexType		nTypePrev;		// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	size_t				nIndent;		// ���̃��C�A�E�g�s�̃C���f���g��
	Eol					eol;
	size_t				nLayoutWidth;	// ���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g���i�u�܂�Ԃ��Ȃ��v�I�����̂݁j
	LayoutExInfo		exInfo;			// �F�����ڍ׏��

private:
	DISALLOW_COPY_AND_ASSIGN(Layout);
};

