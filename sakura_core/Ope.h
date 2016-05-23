/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#pragma once

// �A���h�D�o�b�t�@�p ����R�[�h
enum class OpeCode {
	Unknown,	// �s��(���g�p)
	Insert,		// �}��
	Delete,		// �폜
	Replace,	// �u��
	MoveCaret,	// �L�����b�g�ړ�
};

class LineData {
public:
	NativeW memLine;
	int nSeq;
	void swap(LineData& o) {
		std::swap(memLine, o.memLine);
		std::swap(nSeq, o.nSeq);
	}
};

namespace std {
template <>
	inline void swap(LineData& n1, LineData& n2) {
		n1.swap(n2);
	}
}

typedef std::vector<LineData> OpeLineData;

/*!
	�ҏW����v�f
	
	Undo�̂��߂ɂɑ���菇���L�^���邽�߂ɗp����B
	1�I�u�W�F�N�g���P�̑����\���B
*/
// 2007.10.17 kobake ����R���h�����߁A�f�[�^���|�C���^�ł͂Ȃ��C���X�^���X���̂Ŏ��悤�ɕύX
class Ope {
public:
	Ope(OpeCode eCode);	// Ope�N���X�\�z
	virtual ~Ope();		// Ope�N���X����

	virtual void DUMP(void);	// �ҏW����v�f�̃_���v

	OpeCode GetCode() const { return nOpe; }

private:
	OpeCode nOpe;						// ������

public:
	LogicPoint	ptCaretPos_PHY_Before;	// �L�����b�g�ʒu�B�����P�ʁB			[����]
	LogicPoint	ptCaretPos_PHY_After;	// �L�����b�g�ʒu�B�����P�ʁB			[����]
};

// �폜
class DeleteOpe : public Ope {
public:
	DeleteOpe() : Ope(OpeCode::Delete) {
		ptCaretPos_PHY_To.Set(0, 0);
	}
	virtual void DUMP(void);	// �ҏW����v�f�̃_���v
public:
	LogicPoint	ptCaretPos_PHY_To;	// ����O�̃L�����b�g�ʒu�B�����P�ʁB	[DELETE]
	OpeLineData	opeLineData;		// ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int			nOrgSeq;
};

// �}��
class InsertOpe : public Ope {
public:
	InsertOpe() : Ope(OpeCode::Insert) { }
	virtual void DUMP(void);	// �ҏW����v�f�̃_���v
public:
	OpeLineData	opeLineData;	// ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int			nOrgSeq;
};

// �}��
class ReplaceOpe : public Ope {
public:
	ReplaceOpe()
		:
		Ope(OpeCode::Replace)
	{
		ptCaretPos_PHY_To.Set(0, 0);
	}
public:
	LogicPoint	ptCaretPos_PHY_To;	// ����O�̃L�����b�g�ʒu�B�����P�ʁB	[DELETE]
	OpeLineData	pMemDataIns;			// ����Ɋ֘A����f�[�^				[INSERT]
	OpeLineData	pMemDataDel;			// ����Ɋ֘A����f�[�^				[DELETE]
	int			nOrgInsSeq;
	int			nOrgDelSeq;
};

// �L�����b�g�ړ�
class MoveCaretOpe : public Ope {
public:
	MoveCaretOpe() : Ope(OpeCode::MoveCaret) { }
	MoveCaretOpe(const LogicPoint& ptBefore, const LogicPoint& ptAfter)
		:
		Ope(OpeCode::MoveCaret)
	{
		ptCaretPos_PHY_Before = ptBefore;
		ptCaretPos_PHY_After = ptAfter;
	}
	MoveCaretOpe(const LogicPoint& ptCaretPos)
		:
		Ope(OpeCode::MoveCaret)
	{
		ptCaretPos_PHY_Before = ptCaretPos;
		ptCaretPos_PHY_After = ptCaretPos;
	}
};

