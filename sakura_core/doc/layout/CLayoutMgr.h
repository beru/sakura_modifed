/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	@date 1998/03/06 新規作成
	@date 1998/04/14 データの削除を実装
	@date 1999/12/20 データの置換を実装
	@date 2009/08/28 nasukoji	CalTextWidthArg定義追加、DoLayout_Range()の引数変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>// 2002/2/10 aroka
#include <vector>
#include "doc/CDocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/CType.h"
#include "CLayoutExInfo.h"
#include "view/colors/EColorIndexType.h"
#include "COpe.h"
#include "util/container.h"
#include "util/design_template.h"

class Bregexp;// 2002/2/10 aroka
class Layout;// 2002/2/10 aroka
class DocLineMgr;// 2002/2/10 aroka
class DocLine;// 2002/2/10 aroka
class Memory;// 2002/2/10 aroka
class EditDoc;// 2003/07/20 genta
class SearchStringPattern;
class ColorStrategy;

// レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
enum EKinsokuType {
	KINSOKU_TYPE_NONE = 0,			// なし
	KINSOKU_TYPE_WORDWRAP,			// 英文ワードラップ中
	KINSOKU_TYPE_KINSOKU_HEAD,		// 行頭禁則中
	KINSOKU_TYPE_KINSOKU_TAIL,		// 行末禁則中
	KINSOKU_TYPE_KINSOKU_KUTO,		// 句読点ぶら下げ中
};

struct LayoutReplaceArg {
	LayoutRange	sDelRange;		// [in]削除範囲。レイアウト単位。
	OpeLineData*	pcmemDeleted;	// [out]削除されたデータ
	OpeLineData*	pInsData;		// [in/out]挿入するデータ
	LayoutInt		nAddLineNum;	// [out] 再描画ヒント レイアウト行の増減
	LayoutInt		nModLineFrom;	// [out] 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	LayoutInt		nModLineTo;		// [out] 再描画ヒント 変更されたレイアウト行To(レイアウト行の増減が0のとき使う)
	LayoutPoint	ptLayoutNew;	// [out]挿入された部分の次の位置の位置(レイアウト桁位置, レイアウト行)
	int				nDelSeq;		// [in]削除行のOpeシーケンス
	int				nInsSeq;		// [out]挿入行の元のシーケンス
};

// 編集時のテキスト最大幅算出用		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	LayoutPoint	ptLayout;		// 編集開始位置
	LayoutInt		nDelLines;		// 削除に関係する行数 - 1（負数の時削除なし）
	LayoutInt		nAllLinesOld;	// 編集前のテキスト行数
	BOOL			bInsData;		// 追加文字列あり
};

class LogicPointEx: public LogicPoint {
public:
	LayoutInt ext;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
// 2007.10.15 XYLogicalToLayoutを廃止。LogicToLayoutに統合。
class LayoutMgr : public ProgressSubject {
private:
	typedef LayoutInt (LayoutMgr::*CalcIndentProc)(Layout*);

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
	KetaXInt GetTabSpaceKetas() const { return m_nTabSpace; }
	LayoutInt GetTabSpace() const { return m_nTabSpace; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          参照系                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake 関数名変更: Search → SearchLineByLayoutY
	LayoutInt		GetLineCount() const { return m_nLines; }	// 全物理行数を返す
	const wchar_t*	GetLineStr(LayoutInt nLine, LogicInt* pnLineLen) const;	// 指定された物理行のデータへのポインタとその長さを返す
	const wchar_t*	GetLineStr(LayoutInt nLine, LogicInt* pnLineLen, const Layout** ppcLayoutDes) const;	// 指定された物理行のデータへのポインタとその長さを返す

	// 先頭と末尾
	Layout*		GetTopLayout()		{ return m_pLayoutTop; }
	Layout*		GetBottomLayout()	{ return m_pLayoutBot; }
	const Layout*	GetTopLayout() const { return m_pLayoutTop; }
	const Layout*	GetBottomLayout() const { return m_pLayoutBot; }

	// レイアウトを探す
	const Layout*	SearchLineByLayoutY(LayoutInt nLineLayout) const;	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	Layout*		SearchLineByLayoutY(LayoutInt nLineLayout) { return const_cast<Layout*>(static_cast<const LayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	// ワードを探す
	bool			WhereCurrentWord(LayoutInt , LogicInt , LayoutRange* pSelect, NativeW*, NativeW*);	// 現在位置の単語の範囲を調べる

	// 判定
	bool			IsEndOfLine(const LayoutPoint& ptLinePos);	// 指定位置が行末(改行文字の直前)か調べる	//@@@ 2002.04.18 MIK

	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	LayoutInt GetActualTabSpace(LayoutInt pos) const { return m_nTabSpace - pos % m_nTabSpace; }

	// Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake 関数名変更 GetMaxLineSize→GetMaxLineKetas
	LayoutInt GetMaxLineKetas(void) const { return m_nMaxLineKetas; }

	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam(LayoutInt nTabSize, LayoutInt nMaxLineKetas);

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(LayoutPoint* ptLayoutEnd);

	LayoutInt GetMaxTextWidth(void) const { return m_nTextWidth; }		// 2009.08.28 nasukoji	テキスト最大幅を返す


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord(LayoutInt, LogicInt, LayoutPoint* pptLayoutNew, bool, bool bStopsBothEnds);	// 現在位置の左右の単語の先頭位置を調べる
public:
	int PrevWord(LayoutInt nLineNum, LogicInt nIdx, LayoutPoint* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, true, bStopsBothEnds); }	// 現在位置の左右の単語の先頭位置を調べる
	int NextWord(LayoutInt nLineNum, LogicInt nIdx, LayoutPoint* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, false, bStopsBothEnds); }	// 現在位置の左右の単語の先頭位置を調べる

	int SearchWord(LayoutInt nLine, LogicInt nIdx, eSearchDirection eSearchDirection, LayoutRange* pMatchRange, const SearchStringPattern&);	// 単語検索

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        単位の変換                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ロジック→レイアウト
	void LogicToLayoutEx(const LogicPointEx& ptLogicEx, LayoutPoint* pptLayout, LayoutInt nLineHint = LayoutInt(0)) {
		LogicToLayout(ptLogicEx, pptLayout, nLineHint);
		pptLayout->x += ptLogicEx.ext;
	}
	void LogicToLayout(const LogicPoint& ptLogic, LayoutPoint* pptLayout, LayoutInt nLineHint = LayoutInt(0));
	void LogicToLayout(const LogicRange& rangeLogic, LayoutRange* prangeLayout) {
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}
	
	// レイアウト→ロジック変換
	void LayoutToLogicEx(const LayoutPoint& ptLayout, LogicPointEx* pptLogicEx) const;
	void LayoutToLogic(const LayoutPoint& ptLayout, LogicPoint* pptLogic) const;
	void LayoutToLogic(const LayoutRange& rangeLayout, LogicRange* prangeLogic) const {
		LayoutToLogic(rangeLayout.GetFrom(), prangeLogic->GetFromPointer());
		LayoutToLogic(rangeLayout.GetTo(), prangeLogic->GetToPointer());
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
	/* レイアウト情報の変更
		@date Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
		@date 2002.04.13 MIK 禁則,改行文字をぶら下げる,句読点ぶらさげを追加
		@date 2002/04/27 YAZAKI TypeConfigを渡すように変更。
	*/
	void SetLayoutInfo(
		bool			bDoLayout,
		const TypeConfig&	refType,
		LayoutInt		nTabSpace,
		LayoutInt		nMaxLineKetas
	);

	// 文字列置換
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth(bool bCalLineLen = true, LayoutInt nStart = LayoutInt(-1), LayoutInt nEnd = LayoutInt(-1));	// テキスト最大幅を算出する		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth(void);				// 各行のレイアウト行長の記憶をクリアする		// 2009.08.28 nasukoji

protected:
	/*
	||  参照系
	*/
	const char* GetFirstLinrStr(int*);	// 順アクセスモード：先頭行を得る
	const char* GetNextLinrStr(int*);		// 順アクセスモード：次の行を得る

	/*
	|| 更新系
	*/
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
public:
	void _DoLayout();	// 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します
protected:
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	// 2009.08.28 nasukoji	テキスト最大幅算出用引数追加
	LayoutInt DoLayout_Range(Layout* , LogicInt, LogicPoint, EColorIndexType, LayoutColorInfo*, const CalTextWidthArg*, LayoutInt*);	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	void CalculateTextWidth_Range(const CalTextWidthArg* pctwArg);	// テキストが編集されたら最大幅を算出する	// 2009.08.28 nasukoji
	Layout* DeleteLayoutAsLogical(Layout*, LayoutInt, LogicInt , LogicInt, LogicPoint, LayoutInt*);	// 論理行の指定範囲に該当するレイアウト情報を削除
	void ShiftLogicalLineNum(Layout* , LogicInt);	// 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする

	// 部品
	struct LayoutWork {
		// 毎ループ初期化
		EKinsokuType	eKinsokuType;
		LogicInt		nPos;
		LogicInt		nBgn;
		StringRef		cLineStr;
		LogicInt		nWordBgn;
		LogicInt		nWordLen;
		LayoutInt		nPosX;
		LayoutInt		nIndent;
		Layout*		pLayoutCalculated;

		// ループ外
		DocLine*		pcDocLine;
		Layout*		pLayout;
		ColorStrategy*	pcColorStrategy;
		EColorIndexType	colorPrev;
		LayoutExInfo	exInfoPrev;
		LogicInt		nCurLine;

		// ループ外 (DoLayoutのみ)
//		LogicInt		nLineNum;

		// ループ外 (DoLayout_Rangeのみ)
		bool			bNeedChangeCOMMENTMODE;
		LayoutInt		nModifyLayoutLinesNew;
		
		// ループ外 (DoLayout_Range引数)
		LayoutInt*		pnExtInsLineNum;
		LogicPoint		ptDelLogicalFrom;

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
	bool _ExistKinsokuKuto(wchar_t wc) const { return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const { return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead(wchar_t wc);	// 行頭禁則文字をチェックする	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail(wchar_t wc);	// 行末禁則文字をチェックする	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto(wchar_t wc);	// 句読点文字をチェックする	//@@@ 2002.04.17 MIK
	// 2005-08-20 D.S.Koba 禁則関連処理の関数化
	/*! 句読点ぶら下げの処理位置か
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta 最適化
	*/
	bool IsKinsokuPosKuto(LayoutInt nRest, LayoutInt nCharChars) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(LayoutInt, LayoutInt, LayoutInt);	// 行頭禁則の処理位置か
	bool IsKinsokuPosTail(LayoutInt, LayoutInt, LayoutInt);	// 行末禁則の処理位置か
private:
	// Oct. 1, 2002 genta インデント幅計算関数群
	LayoutInt getIndentOffset_Normal(Layout* pLayoutPrev);
	LayoutInt getIndentOffset_Tx2x(Layout* pLayoutPrev);
	LayoutInt getIndentOffset_LeftSpace(Layout* pLayoutPrev);

protected:
	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX引数追加
	Layout* CreateLayout(DocLine* pCDocLine, LogicPoint ptLogicPos, LogicInt nLength, EColorIndexType nTypePrev, LayoutInt nIndent, LayoutInt nPosX, LayoutColorInfo*);
	Layout* InsertLineNext(Layout*, Layout*);
	void AddLineBottom(Layout*);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	DocLineMgr*			m_pcDocLineMgr;	// 行バッファ管理マネージャ

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	// 2007.10.08 kobake 変数名変更: getIndentOffset→m_getIndentOffset

	// 参照
	EditDoc*		m_pcEditDoc;

	// 実データ
	Layout*				m_pLayoutTop;
	Layout*				m_pLayoutBot;

	// タイプ別設定
	const TypeConfig*		m_pTypeConfig;
	LayoutInt				m_nMaxLineKetas;
	LayoutInt				m_nTabSpace;
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	CalcIndentProc			m_getIndentOffset;			// Oct. 1, 2002 genta インデント幅計算関数を保持

	// フラグ等
	EColorIndexType			m_nLineTypeBot;				// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	LayoutExInfo			m_cLayoutExInfoBot;
	LayoutInt				m_nLines;					// 全レイアウト行数

	mutable LayoutInt		m_nPrevReferLine;
	mutable Layout*		m_pLayoutPrevRefer;
	
	// EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)	//2006.10.01 Moca
	LayoutInt				m_nEOFLine;		// EOF行数
	LayoutInt				m_nEOFColumn;	// EOF幅位置

	// テキスト最大幅を記憶（折り返し位置算出に使用）	// 2009.08.28 nasukoji
	LayoutInt				m_nTextWidth;				// テキスト最大幅の記憶
	LayoutInt				m_nTextWidthMaxLine;		// 最大幅のレイアウト行

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutMgr);
};

