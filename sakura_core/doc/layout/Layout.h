/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "util/design_template.h"
#include "Eol.h"// 2002/2/10 aroka
#include "doc/logic/DocLine.h"// 2002/4/21 YAZAKI
#include "mem/Memory.h"// 2002/4/21 YAZAKI
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
	// 2007.08.23 kobake �R���X�g���N�^�Ń����o�ϐ�������������悤�ɂ���
	Layout(
		const DocLine*	pDocLine,		// ���f�[�^�ւ̎Q��
		LogicPoint		ptLogicPos,		// ���f�[�^�Q�ƈʒu
		LogicInt		nLength,		// ���f�[�^���f�[�^��
		EColorIndexType	nTypePrev,
		LayoutInt		nTypeIndent,
		LayoutColorInfo*	pColorInfo
		)
	{
		this->pPrev			= nullptr;
		this->pNext			= nullptr;
		this->pDocLine		= pDocLine;
		this->ptLogicPos	= ptLogicPos;	// ���f�[�^�Q�ƈʒu
		this->nLength		= nLength;		// ���f�[�^���f�[�^��
		this->nTypePrev		= nTypePrev;	// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
		nIndent		= nTypeIndent;	// ���̃��C�A�E�g�s�̃C���f���g�� @@@ 2002.09.23 YAZAKI
		exInfo.SetColorInfo(pColorInfo);
	}
	~Layout();
	void DUMP(void);
	
	// ptLogicPos.x�ŕ␳�������Ƃ̕�����𓾂�
	const wchar_t* GetPtr() const { return pDocLine->GetPtr() + ptLogicPos.x; }
	LogicInt GetLengthWithEOL() const { return nLength;	}	//	������EOL�͏��1�����ƃJ�E���g�H�H
	LogicInt GetLengthWithoutEOL() const { return nLength - (eol.GetLen() ? 1 : 0);	}
	//LogicInt GetLength() const { return nLength; }	// CMemoryIterator�p�iEOL�܂ށj
	LayoutInt GetIndent() const { return nIndent; }	// ���̃��C�A�E�g�s�̃C���f���g�T�C�Y���擾�B�P�ʂ͔��p�����B	CMemoryIterator�p

	// �擾�C���^�[�t�F�[�X
	LogicInt GetLogicLineNo() const { if (this) return ptLogicPos.GetY2(); else return LogicInt(-1); } //$$$������
	LogicInt GetLogicOffset() const { return ptLogicPos.GetX2(); }
	LogicPoint GetLogicPos() const { return ptLogicPos; }
	EColorIndexType GetColorTypePrev() const { return nTypePrev; } //#########����
	LayoutInt GetLayoutWidth() const { return nLayoutWidth; }		// 2009.08.28 nasukoji	���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g����Ԃ�

	// �ύX�C���^�[�t�F�[�X
	void OffsetLogicLineNo(LogicInt n) { ptLogicPos.y += n; }
	void SetColorTypePrev(EColorIndexType n) {
		nTypePrev = n;
	}
	void SetLayoutWidth(LayoutInt nWidth) { nLayoutWidth = nWidth; }

	// ���C�A�E�g�����v�Z�B���s�͊܂܂Ȃ��B2007.10.11 kobake
	LayoutInt CalcLayoutWidth(const LayoutMgr& layoutMgr) const;

	// �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾�B2007.10.17 kobake
	LayoutInt CalcLayoutOffset(const LayoutMgr& layoutMgr,
								LogicInt nStartPos = LogicInt(0),
								LayoutInt nStartOffset = LayoutInt(0)) const;

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
	const DocLine* GetDocLineRef() const { if (this) return pDocLine; else return NULL; } //$$note:������

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
	const DocLine*		pDocLine;			// ���f�[�^�ւ̎Q��
	LogicPoint			ptLogicPos;		// �Ή����郍�W�b�N�Q�ƈʒu
	LogicInt			nLength;			// ���̃��C�A�E�g�s�̒����B�����P�ʁB
	
	// ���̑�����
	EColorIndexType		nTypePrev;		// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	LayoutInt			nIndent;			// ���̃��C�A�E�g�s�̃C���f���g�� @@@ 2002.09.23 YAZAKI
	Eol					eol;
	LayoutInt			nLayoutWidth;		// ���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g���i�u�܂�Ԃ��Ȃ��v�I�����̂݁j	// 2009.08.28 nasukoji
	LayoutExInfo		exInfo;			// �F�����ڍ׏��

private:
	DISALLOW_COPY_AND_ASSIGN(Layout);
};

