#pragma once

// アンドゥバッファ用 操作コード
enum class OpeCode {
	Unknown,	// 不明(未使用)
	Insert,		// 挿入
	Delete,		// 削除
	Replace,	// 置換
	MoveCaret,	// キャレット移動
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
	編集操作要素
	
	Undoのためにに操作手順を記録するために用いる。
	1オブジェクトが１つの操作を表す。
*/
class Ope {
public:
	Ope(OpeCode eCode);	// Opeクラス構築
	virtual ~Ope();		// Opeクラス消滅

	virtual void DUMP(void);	// 編集操作要素のダンプ

	OpeCode GetCode() const { return nOpe; }

private:
	OpeCode nOpe;						// 操作種別

public:
	Point	ptCaretPos_PHY_Before;	// キャレット位置。文字単位。			[共通]
	Point	ptCaretPos_PHY_After;	// キャレット位置。文字単位。			[共通]
};

// 削除
class DeleteOpe : public Ope {
public:
	DeleteOpe() : Ope(OpeCode::Delete) {
		ptCaretPos_PHY_To.Set(0, 0);
	}
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	Point	ptCaretPos_PHY_To;	// 操作前のキャレット位置。文字単位。	[DELETE]
	OpeLineData	opeLineData;		// 操作に関連するデータ				[DELETE/INSERT]
	int			nOrgSeq;
};

// 挿入
class InsertOpe : public Ope {
public:
	InsertOpe() : Ope(OpeCode::Insert) { }
	virtual void DUMP(void);	// 編集操作要素のダンプ
public:
	OpeLineData	opeLineData;	// 操作に関連するデータ				[DELETE/INSERT]
	int			nOrgSeq;
};

// 挿入
class ReplaceOpe : public Ope {
public:
	ReplaceOpe()
		:
		Ope(OpeCode::Replace)
	{
		ptCaretPos_PHY_To.Set(0, 0);
	}
public:
	Point	ptCaretPos_PHY_To;	// 操作前のキャレット位置。文字単位。	[DELETE]
	OpeLineData	pMemDataIns;			// 操作に関連するデータ				[INSERT]
	OpeLineData	pMemDataDel;			// 操作に関連するデータ				[DELETE]
	int			nOrgInsSeq;
	int			nOrgDelSeq;
};

// キャレット移動
class MoveCaretOpe : public Ope {
public:
	MoveCaretOpe() : Ope(OpeCode::MoveCaret) { }
	MoveCaretOpe(const Point& ptBefore, const Point& ptAfter)
		:
		Ope(OpeCode::MoveCaret)
	{
		ptCaretPos_PHY_Before = ptBefore;
		ptCaretPos_PHY_After = ptAfter;
	}
	MoveCaretOpe(const Point& ptCaretPos)
		:
		Ope(OpeCode::MoveCaret)
	{
		ptCaretPos_PHY_Before = ptCaretPos;
		ptCaretPos_PHY_After = ptCaretPos;
	}
};

