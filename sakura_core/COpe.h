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
enum class eOpeCode {
	Unknown,	//!< �s��(���g�p)
	Insert,		//!< �}��
	Delete,		//!< �폜
	Replace,	//!< �u��
	MoveCaret,	//!< �L�����b�g�ړ�
};

class LineData {
public:
	NativeW cmemLine;
	int nSeq;
	void swap(LineData& o) {
		std::swap(cmemLine, o.cmemLine);
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
	Ope(eOpeCode eCode);	// Ope�N���X�\�z
	virtual ~Ope();		// Ope�N���X����

	virtual void DUMP(void);	// �ҏW����v�f�̃_���v

	eOpeCode GetCode() const { return m_nOpe; }

private:
	eOpeCode m_nOpe;						//!< ������

public:
	LogicPoint	m_ptCaretPos_PHY_Before;	//!< �L�����b�g�ʒu�B�����P�ʁB			[����]
	LogicPoint	m_ptCaretPos_PHY_After;		//!< �L�����b�g�ʒu�B�����P�ʁB			[����]
};

//! �폜
class DeleteOpe : public Ope {
public:
	DeleteOpe() : Ope(eOpeCode::Delete) {
		m_ptCaretPos_PHY_To.Set(LogicInt(0),LogicInt(0));
	}
	virtual void DUMP(void);	// �ҏW����v�f�̃_���v
public:
	LogicPoint	m_ptCaretPos_PHY_To;		//!< ����O�̃L�����b�g�ʒu�B�����P�ʁB	[DELETE]
	OpeLineData	m_cOpeLineData;			//!< ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! �}��
class InsertOpe : public Ope {
public:
	InsertOpe() : Ope(eOpeCode::Insert) { }
	virtual void DUMP(void);	// �ҏW����v�f�̃_���v
public:
	OpeLineData	m_cOpeLineData;			//!< ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! �}��
class ReplaceOpe : public Ope {
public:
	ReplaceOpe()
		:
		Ope(eOpeCode::Replace)
	{
		m_ptCaretPos_PHY_To.Set(LogicInt(0), LogicInt(0));
	}
public:
	LogicPoint	m_ptCaretPos_PHY_To;		//!< ����O�̃L�����b�g�ʒu�B�����P�ʁB	[DELETE]
	OpeLineData	m_pcmemDataIns;			//!< ����Ɋ֘A����f�[�^				[INSERT]
	OpeLineData	m_pcmemDataDel;			//!< ����Ɋ֘A����f�[�^				[DELETE]
	int				m_nOrgInsSeq;
	int				m_nOrgDelSeq;
};

//! �L�����b�g�ړ�
class MoveCaretOpe : public Ope {
public:
	MoveCaretOpe() : Ope(eOpeCode::MoveCaret) { }
	MoveCaretOpe(const LogicPoint& ptBefore, const LogicPoint& ptAfter)
		:
		Ope(eOpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptBefore;
		m_ptCaretPos_PHY_After = ptAfter;
	}
	MoveCaretOpe(const LogicPoint& ptCaretPos)
		:
		Ope(eOpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptCaretPos;
		m_ptCaretPos_PHY_After = ptCaretPos;
	}
};

