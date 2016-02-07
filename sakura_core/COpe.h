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
enum class eOpeCode {
	Unknown,	//!< 不明(未使用)
	Insert,		//!< 挿入
	Delete,		//!< 削除
	Replace,	//!< 置換
	MoveCaret,	//!< キャレット移動
};

class CLineData {
public:
	CNativeW cmemLine;
	int nSeq;
	void swap(CLineData& o) {
		std::swap(cmemLine, o.cmemLine);
		std::swap(nSeq, o.nSeq);
	}
};

namespace std {
template <>
	inline void swap(CLineData& n1, CLineData& n2) {
		n1.swap(n2);
	}
}

typedef std::vector<CLineData> COpeLineData;

/*!
	編集操作要素
	
	Undoのためにに操作手順を記録するために用いる。
	1オブジェクトが１つの操作を表す。
*/
// 2007.10.17 kobake 解放漏れを防ぐため、データをポインタではなくインスタンス実体で持つように変更
class COpe {
public:
	COpe(eOpeCode eCode);	// COpeクラス構築
	virtual ~COpe();		// COpeクラス消滅

	virtual void DUMP(void);	// 編集操作要素のダンプ

	eOpeCode GetCode() const { return m_nOpe; }

private:
	eOpeCode m_nOpe;						//!< 操作種別

public:
	CLogicPoint	m_ptCaretPos_PHY_Before;	//!< キャレット位置。文字単位。			[共通]
	CLogicPoint	m_ptCaretPos_PHY_After;		//!< キャレット位置。文字単位。			[共通]
};

//! 削除
class CDeleteOpe : public COpe {
public:
	CDeleteOpe() : COpe(eOpeCode::Delete) {
		m_ptCaretPos_PHY_To.Set(CLogicInt(0),CLogicInt(0));
	}
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置。文字単位。	[DELETE]
	COpeLineData	m_cOpeLineData;			//!< 操作に関連するデータ				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! 挿入
class CInsertOpe : public COpe {
public:
	CInsertOpe() : COpe(eOpeCode::Insert) { }
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	COpeLineData	m_cOpeLineData;			//!< 操作に関連するデータ				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//! 挿入
class CReplaceOpe : public COpe {
public:
	CReplaceOpe()
		:
		COpe(eOpeCode::Replace)
	{
		m_ptCaretPos_PHY_To.Set(CLogicInt(0), CLogicInt(0));
	}
public:
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置。文字単位。	[DELETE]
	COpeLineData	m_pcmemDataIns;			//!< 操作に関連するデータ				[INSERT]
	COpeLineData	m_pcmemDataDel;			//!< 操作に関連するデータ				[DELETE]
	int				m_nOrgInsSeq;
	int				m_nOrgDelSeq;
};

//! キャレット移動
class CMoveCaretOpe : public COpe {
public:
	CMoveCaretOpe() : COpe(eOpeCode::MoveCaret) { }
	CMoveCaretOpe(const CLogicPoint& ptBefore, const CLogicPoint& ptAfter)
		:
		COpe(eOpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptBefore;
		m_ptCaretPos_PHY_After = ptAfter;
	}
	CMoveCaretOpe(const CLogicPoint& ptCaretPos)
		:
		COpe(eOpeCode::MoveCaret)
	{
		m_ptCaretPos_PHY_Before = ptCaretPos;
		m_ptCaretPos_PHY_After = ptCaretPos;
	}
};

