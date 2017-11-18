#pragma once

// 現在行のマークを管理する

#include <vector>


/*!
	行マークを管理するクラス。
	純粋仮想関数を含むので、実際にはサブクラスを作って使う。

	@par 通常操作（共通）
	Add()で追加．場所と名前を登録できる．操作そのものはカスタマイズ可能．
	[番号]で該当番号の要素を取得できる．

	@par 内部動作
	最大値を超えた場合はprotectedな関数で処理する．（カスタマイズ可能）
	Add()の処理はサブクラスに任せる．

	@par 現在位置の管理
	現在位置はManager内で管理する．

	削除操作はサブクラスにまかせる

*/
class MarkMgr {
public:

	//	項目のクラス
	class Mark {
	public:
		//	constructor
		Mark(const Point& pt) : ptLogic(pt) { }

		Point GetPosition() const { return ptLogic; }
		void SetPosition(const Point& pt) { ptLogic = pt; }

		bool IsValid(void) const { return true; }

		bool operator == (Mark &r) const { return ptLogic.y == r.ptLogic.y; }
		bool operator != (Mark &r) const { return ptLogic.y != r.ptLogic.y; }

	private:
		Point ptLogic;
	};

	// GENERATE_FACTORY(Mark, CMarkFactory);	//	Mark用Factory class

	//	型宣言
	typedef std::vector<Mark> MarkChain;
	typedef std::vector<Mark>::const_iterator	MarkIterator;

	//	Interface
	//	constructor
	MarkMgr() : nCurpos(0), nMaxitem(10) {}
	// MarkMgr(const CDocLineMgr *p) : doc(p) {}

	size_t Count(void) const { return markChain.size(); }	//	項目数を返す
	int GetMax(void) const { return nMaxitem; }	//	最大項目数を返す
	void SetMax(int max);	//	最大項目数を設定

	virtual void Add(const Mark& m) = 0;	//	要素の追加

	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//	要素の全消去

	// 要素の取得
	const Mark& GetCurrent(void) const { return markChain[nCurpos]; }

	// 有効性の確認
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;

	// 現在位置の移動
	bool NextValid(void);
	bool PrevValid(void);

	const Mark& operator[](int index) const { return markChain[index]; }

	//	連続取得インターフェース
//	MarkIterator CurrentPos(void) const { return (MarkIterator)markChain.begin() + nCurpos; }
//	MarkIterator Begin(void) const { return (MarkIterator)markChain.begin(); }
//	MarkIterator End(void) const { return (MarkIterator)markChain.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory factory;	//	Factory Class (マクロで生成される）
	MarkChain markChain;	//	マークデータ本体
	int nCurpos;	//	現在位置（番号）

	int nMaxitem;	//	保管可能アイテムの最大数
private:
	//MarkMgr(const MarkMgr&);	//	Copy禁止

};

// ----------------------------------------------------
/*!
	@brief 移動履歴の管理クラス

	MarkMgr を継承し、動作が規定されていない部分を実装する。
*/
class AutoMarkMgr : public MarkMgr {
public:
	virtual void Add(const Mark& m);	// 要素の追加
	virtual void Expire(void);			// 要素数の調整
};


