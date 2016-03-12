/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/3/5  �V�K�쐬
	@date 2001/06/23 N.Nakatani WhereCurrentWord_2()�ǉ� static�����o
	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ����֐��ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, aroka, MIK, hor
	Copyright (C) 2003, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "_main/global.h" // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "util/design_template.h"
#include "Ope.h"

class DocLine; // 2002/2/10 aroka
class Bregexp; // 2002/2/10 aroka

struct DocLineReplaceArg {
	LogicRange		delRange;			// [in] �폜�͈́B���W�b�N�P�ʁB
	OpeLineData*	pMemDeleted;		// [out] �폜���ꂽ�f�[�^��ۑ�
	OpeLineData*	pInsData;			// [in/out] �}������f�[�^(���g���ړ�����)
	LogicInt		nDeletedLineNum;	// [out] �폜�����s�̑���
	LogicInt		nInsLineNum;		// [out] �}���ɂ���đ������s�̐�
	LogicPoint		ptNewPos;			// [out] �}�����ꂽ�����̎��̈ʒu
	int				nDelSeq;			// [in] �폜�s��Ope�V�[�P���X
	int				nInsSeq;			// [out] �}���s�̌��̃V�[�P���X
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// 2007.09.30 kobake WhereCurrentWord_2 �� CWordParse �Ɉړ�
class DocLineMgr {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	DocLineMgr();
	~DocLineMgr();
	
	// ���
	LogicInt GetLineCount() const { return m_nLines; }	// �S�s����Ԃ�
	
	// �s�f�[�^�ւ̃A�N�Z�X
	const DocLine* GetLine(LogicInt nLine) const;		// �w��s���擾
	DocLine* GetLine(LogicInt nLine) {
		return const_cast<DocLine*>(const_cast<DocLine*>(static_cast<const DocLineMgr*>(this)->GetLine( nLine )));
	}
	const DocLine* GetDocLineTop() const { return m_pDocLineTop; }		// �擪�s���擾
	DocLine* GetDocLineTop() { return m_pDocLineTop; }					// �擪�s���擾
	const DocLine* GetDocLineBottom() const { return m_pDocLineBot; }	// �ŏI�s���擾
	DocLine* GetDocLineBottom() { return m_pDocLineBot; }				// �ŏI�s���擾
	
	// �s�f�[�^�̊Ǘ�
	DocLine* InsertNewLine(DocLine* pPos);		// pPos�̒��O�ɐV�����s��}��
	DocLine* AddNewLine();						// �ŉ����ɐV�����s��}��
	void DeleteAllLine();						// �S�Ă̍s���폜����
	void DeleteLine(DocLine*);					// �s�̍폜
	
	// �f�o�b�O
	void DUMP();
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	void _Init();
	// -- -- �`�F�[���֐� -- -- // 2007.10.11 kobake �쐬
	void _PushBottom(DocLine* pDocLineNew);						// �ŉ����ɑ}��
	void _InsertBeforePos(DocLine* pDocLineNew, DocLine* pPos);	// pPos�̒��O�ɑ}��
	void _InsertAfterPos(DocLine* pDocLineNew, DocLine* pPos);	// pPos�̒���ɑ}��
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	DocLine*	m_pDocLineTop;		// �ŏ��̍s
	DocLine*	m_pDocLineBot;		// �Ō�̍s(��1�s�����Ȃ��ꍇ��m_pDocLineTop�Ɠ������Ȃ�)
	LogicInt	m_nLines;			// �S�s��
	
public:
	//$$ kobake��: �ȉ��A��΂ɐ؂藣�������i�Œ�؂藣���Ȃ��Ă��A�ϐ��̈Ӗ����R�����g�Ŗ��m�ɋL���ׂ��j�ϐ��Q
	mutable DocLine*	m_pDocLineCurrent;	// ���A�N�Z�X���̌��݈ʒu
	mutable LogicInt	m_nPrevReferLine;
	mutable DocLine*	m_pCodePrevRefer;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLineMgr);
};

