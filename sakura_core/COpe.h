/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#pragma once

// アンドゥバッファ用 操作コード
enum class OpeCode {
	Unknown,	//!< 不明(未使用)
	Insert,		//!< 挿入
	Delete,		//!< 削除
	Replace,	//!< 置換
	MoveCaret,	//!< キャレット移動
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
	編集操作要素
	
	Undoのためにに操作手順を記録するために用いる。
	1オブジェクトが１つの操作を表す。
*/
// 2007.10.17 kobake 解放漏れを防ぐため、データをポインタではなくインスタンス実体で持つように変更
class Ope {
public:
	Ope(OpeCode eCode);	// Opeクラス構築
	virtual ~Ope();		// Opeクラス消滅

	virtual void DUMP(void);	// 編集操作要素のダンプ

	OpeCode GetCode() const { return m_nOpe; }

private:
	OpeCode m_nOpe;						//!< 操作種別

public:
	LogicPoint	m_ptCaretPos_PHY_Before;	//!< キャレット位置。文字単位。			[共通]
	LogicPoint	m_ptCaretPos_PHY_After;		//!< キャレット位置。文字単位。			[共通]
};

//! 削除
class DeleteOpe : public Ope {
public:
	DeleteOpe() : Ope(OpeCode::Delete) {
		m_ptCaretPos_PHY_To.Set(LogicInt(0),LogicInt(0));
	}
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	LogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置。文字単位。	[DELETE]
	OpeLineData	m_cOpeLineData;			//!< 操作に関連するデータ				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! 挿入
class InsertOpe : public Ope {
public:
	InsertOpe() : Ope(OpeCode::Insert) { }
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	OpeLineData	m_cOpeLineData;			//!< 操作に関連するデータ				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! 挿入
class ReplaceOpe : public Ope {
public:
	ReplaceOpe()
		:
		Ope(OpeCode::Replace)
	{
		m_ptCaretPos_PHY_To.Set(LogicInt(0), LogicInt(0));
	}
public:
	LogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置。文字単位。	[DELETE]
	OpeLineData	m_pcmemDataIns;			//!< 操作に関連するデータ				[INSERT]
	OpeLineData	m_pcmemDataDel;			//!< 操作に関連するデータ				[DELETE]
	int				m_nOrgInsSeq;
	int				m_nOrgDelSeq;
};

//! キャレット移動
class MoveCaretOpe : public Ope {
public:
	MoveCaretOpe() : Ope(OpeCode::MoveCaret) { }
	MoveCaretOpe(const LogicPoint& ptBefore, const LogicPoint& ptAfter)
		:
		Ope(OpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptBefore;
		m_ptCaretPos_PHY_After = ptAfter;
	}
	MoveCaretOpe(const LogicPoint& ptCaretPos)
		:
		Ope(OpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptCaretPos;
		m_ptCaretPos_PHY_After = ptCaretPos;
	}
};

