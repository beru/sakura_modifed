// テキストのレイアウト情報管理

#pragma once

#include <Windows.h>
#include <vector>
#include "doc/DocListener.h"
#include "_main/global.h"
#include "basis/SakuraBasis.h"
#include "types/Type.h"
#include "LayoutExInfo.h"
#include "view/colors/EColorIndexType.h"
#include "Ope.h"
#include "util/container.h"
#include "util/design_template.h"

class Bregexp;
class Layout;
class DocLineMgr;
class DocLine;
class Memory;
class EditDoc;
class SearchStringPattern;
class ColorStrategy;

// レイアウト中の禁則タイプ
enum class KinsokuType {
	None = 0,	// なし
	WordWrap,	// 英文ワードラップ中
	Head,		// 行頭禁則中
	Tail,		// 行末禁則中
	Kuto,		// 句読点ぶら下げ中
};

struct LayoutReplaceArg {
	Range			delRange;		// [in]削除範囲。レイアウト単位。
	OpeLineData*	pMemDeleted;	// [out]削除されたデータ
	OpeLineData*	pInsData;		// [in/out]挿入するデータ
	size_t	nAddLineNum;	// [out] 再描画ヒント レイアウト行の増減
	int		nModLineFrom;	// [out] 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	int		nModLineTo;		// [out] 再描画ヒント 変更されたレイアウト行To(レイアウト行の増減が0のとき使う)
	Point	ptLayoutNew;	// [out]挿入された部分の次の位置の位置(レイアウト桁位置, レイアウト行)
	int				nDelSeq;		// [in]削除行のOpeシーケンス
	int				nInsSeq;		// [out]挿入行の元のシーケンス
};

// 編集時のテキスト最大幅算出用
struct CalTextWidthArg {
	Point ptLayout;		// 編集開始位置
	int	nDelLines;		// 削除に関係する行数 - 1（負数の時削除なし）
	size_t nAllLinesOld;	// 編集前のテキスト行数
	bool bInsData;		// 追加文字列あり
};

class PointEx: public Point {
public:
	size_t ext;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理 */
class LayoutMgr : public ProgressSubject {
private:
	typedef size_t (LayoutMgr::*CalcIndentProc)(Layout*);

public:
	// 生成と破棄
	LayoutMgr();
	~LayoutMgr();
	void Create(EditDoc*, DocLineMgr*);
	void Init();
	void _Empty();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        コンフィグ                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// タブ幅の取得
	size_t GetTabSpaceKetas() const { return nTabSpace; }
	size_t GetTabSpace() const { return nTabSpace; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          参照系                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:	
	size_t			GetLineCount() const { return nLines; }	// 全物理行数を返す
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen) const;	// 指定された物理行のデータへのポインタとその長さを返す
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen, const Layout** ppcLayoutDes) const;	// 指定された物理行のデータへのポインタとその長さを返す

	// 先頭と末尾
	Layout*			GetTopLayout()		{ return pLayoutTop; }
	Layout*			GetBottomLayout()	{ return pLayoutBot; }
	const Layout*	GetTopLayout() const { return pLayoutTop; }
	const Layout*	GetBottomLayout() const { return pLayoutBot; }

	// レイアウトを探す
	const Layout*	SearchLineByLayoutY(size_t nLineLayout) const;	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	Layout*			SearchLineByLayoutY(size_t nLineLayout) { return const_cast<Layout*>(static_cast<const LayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	// ワードを探す
	bool			WhereCurrentWord(size_t , size_t , Range* pSelect, NativeW*, NativeW*);	// 現在位置の単語の範囲を調べる

	// 判定
	bool			IsEndOfLine(const Point& ptLinePos);	// 指定位置が行末(改行文字の直前)か調べる

	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	size_t GetActualTabSpace(size_t pos) const { return nTabSpace - pos % nTabSpace; }

	size_t GetMaxLineKetas(void) const { return nMaxLineKetas; }

	bool ChangeLayoutParam(size_t nTabSize, size_t nMaxLineKetas);

	void GetEndLayoutPos(Point* ptLayoutEnd);

	size_t GetMaxTextWidth(void) const { return nTextWidth; }		// テキスト最大幅を返す


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	bool PrevOrNextWord(size_t, size_t, Point* pptLayoutNew, bool, bool bStopsBothEnds);	// 現在位置の左右の単語の先頭位置を調べる
public:
	bool PrevWord(size_t nLineNum, size_t nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, true, bStopsBothEnds); }	// 現在位置の左右の単語の先頭位置を調べる
	bool NextWord(size_t nLineNum, size_t nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, false, bStopsBothEnds); }	// 現在位置の左右の単語の先頭位置を調べる

	bool SearchWord(size_t nLine, size_t nIdx, SearchDirection SearchDirection, Range* pMatchRange, const SearchStringPattern&);	// 単語検索

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        単位の変換                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ロジック→レイアウト
	Point LogicToLayoutEx(const PointEx& ptLogicEx, int nLineHint = 0) {
		Point ptLayout = LogicToLayout(ptLogicEx, nLineHint);
		ptLayout.x += (int)ptLogicEx.ext;
		return ptLayout;
	}
	Point LogicToLayout(const Point& ptLogic, int nLineHint = 0);
	void LogicToLayout(const Range& rangeLogic, Range* prangeLayout) {
		prangeLayout->SetFrom(LogicToLayout(rangeLogic.GetFrom()));
		prangeLayout->SetTo(LogicToLayout(rangeLogic.GetTo()));
	}
	
	// レイアウト→ロジック変換
	PointEx LayoutToLogicEx(const Point& ptLayout) const;
	Point LayoutToLogic(const Point& ptLayout) const;
	void LayoutToLogic(const Range& rangeLayout, Range* prangeLogic) const {
		prangeLogic->SetFrom(LayoutToLogic(rangeLayout.GetFrom()));
		prangeLogic->SetTo(LayoutToLogic(rangeLayout.GetTo()));
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         デバッグ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void DUMP();	// テスト用にレイアウト情報をダンプ
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         編集とか                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*
	|| 更新系
	*/
	/* レイアウト情報の変更 */
	void SetLayoutInfo(
		bool			bDoLayout,
		const TypeConfig&	refType,
		size_t			nTabSpace,
		size_t			nMaxLineKetas
	);

	// 文字列置換
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth(bool bCalLineLen = true, int nStart = -1, int nEnd = -1);	// テキスト最大幅を算出する
	void ClearLayoutLineWidth(void);				// 各行のレイアウト行長の記憶をクリアする

protected:
	/*
	||  参照系
	*/
	const char* GetFirstLinrStr(int*);	// 順アクセスモード：先頭行を得る
	const char* GetNextLinrStr(int*);	// 順アクセスモード：次の行を得る

	/*
	|| 更新系
	*/
public:
	void _DoLayout();	// 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します
protected:
	int DoLayout_Range(Layout* , int, Point, EColorIndexType, LayoutColorInfo*, const CalTextWidthArg&);	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	void CalculateTextWidth_Range(const CalTextWidthArg& ctwArg);	// テキストが編集されたら最大幅を算出する
	Layout* DeleteLayoutAsLogical(Layout*, int, int , int, Point, size_t*);	// 論理行の指定範囲に該当するレイアウト情報を削除
	void ShiftLogicalLineNum(Layout* , int);	// 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする

	// 部品
	struct LayoutWork {
		// 毎ループ初期化
		KinsokuType	eKinsokuType;
		size_t		nPos;
		size_t		nBgn;
		StringRef	lineStr;
		size_t		nWordBgn;
		size_t		nWordLen;
		int			nPosX;
		int			nIndent;
		Layout*		pLayoutCalculated;

		// ループ外
		DocLine*		pDocLine;
		Layout*			pLayout;
		ColorStrategy*	pColorStrategy;
		EColorIndexType	colorPrev;
		LayoutExInfo	exInfoPrev;
		int				nCurLine;

		// ループ外 (DoLayoutのみ)
//		int		nLineNum;

		// ループ外 (DoLayout_Rangeのみ)
		bool			bNeedChangeCOMMENTMODE;
		int				nModifyLayoutLinesNew;
		
		// ループ外 (DoLayout_Range引数)
		Point			ptDelLogicalFrom;

		// 関数
		Layout* _CreateLayout(LayoutMgr* mgr);
	};
	// 関数ポインタ
	typedef void (LayoutMgr::*PF_OnLine)(LayoutWork*);
	// DoLayout用
	bool _DoKinsokuSkip(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoWordWrap(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoKutoBurasage(LayoutWork* pWork);
	void _DoGyotoKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoGyomatsuKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine);
	bool _DoTab(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _MakeOneLine(LayoutWork* pWork, PF_OnLine pfOnLine);
	// DoLayout用コア
	void _OnLine1(LayoutWork* pWork);
	// DoLayout_Range用コア
	void _OnLine2(LayoutWork* pWork);
	
private:
	bool _ExistKinsokuKuto(wchar_t wc) const { return pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const { return pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead(wchar_t wc);	// 行頭禁則文字をチェックする
	bool IsKinsokuTail(wchar_t wc);	// 行末禁則文字をチェックする
	bool IsKinsokuKuto(wchar_t wc);	// 句読点文字をチェックする
	/*! 句読点ぶら下げの処理位置か */
	bool IsKinsokuPosKuto(size_t nRest, size_t nCharChars) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(size_t, size_t, size_t);	// 行頭禁則の処理位置か
	bool IsKinsokuPosTail(size_t, size_t, size_t);	// 行末禁則の処理位置か
private:
	// インデント幅計算関数群
	size_t getIndentOffset_Normal(Layout* pLayoutPrev);
	size_t getIndentOffset_Tx2x(Layout* pLayoutPrev);
	size_t getIndentOffset_LeftSpace(Layout* pLayoutPrev);

protected:
	/*
	|| 実装ヘルパ系
	*/
	Layout* CreateLayout(DocLine* pDocLine, Point ptLogicPos, int nLength, EColorIndexType nTypePrev, int nIndent, int nPosX, LayoutColorInfo*);
	Layout* InsertLineNext(Layout*, Layout*);
	void AddLineBottom(Layout*);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	DocLineMgr*			pDocLineMgr;	// 行バッファ管理マネージャ

protected:
	// 参照
	EditDoc*			pEditDoc;

	// 実データ
	Layout*				pLayoutTop;
	Layout*				pLayoutBot;

	// タイプ別設定
	const TypeConfig*		pTypeConfig;
	size_t					nMaxLineKetas;
	size_t					nTabSpace;
	vector_ex<wchar_t>		pszKinsokuHead_1;			// 行頭禁則文字
	vector_ex<wchar_t>		pszKinsokuTail_1;			// 行末禁則文字
	vector_ex<wchar_t>		pszKinsokuKuto_1;			// 句読点ぶらさげ文字
	CalcIndentProc			getIndentOffset;			// インデント幅計算関数を保持

	// フラグ等
	EColorIndexType			nLineTypeBot;				// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	LayoutExInfo			layoutExInfoBot;
	size_t					nLines;					// 全レイアウト行数

	mutable int				nPrevReferLine;
	mutable Layout*			pLayoutPrevRefer;
	
	// EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)
	int						nEOFLine;		// EOF行数
	int						nEOFColumn;	// EOF幅位置

	// テキスト最大幅を記憶（折り返し位置算出に使用）
	size_t					nTextWidth;				// テキスト最大幅の記憶
	size_t					nTextWidthMaxLine;		// 最大幅のレイアウト行

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutMgr);
};

