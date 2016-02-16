#include "StdAfx.h"
#include "doc/EditDoc.h" /// 2003/07/20 genta
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "doc/logic/DocLine.h"/// 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "view/colors/ColorStrategy.h"
#include "util/window.h"
#include "debug/RunningTimer.h"

// 2008.07.27 kobake
static
bool _GetKeywordLength(
	const StringRef&	lineStr,		// [in]
	LogicInt			nPos,			// [in]
	LogicInt*			p_nWordBgn,		// [out]
	LogicInt*			p_nWordLen,		// [out]
	LayoutInt*			p_nWordKetas	// [out]
	)
{
	// キーワード長をカウントする
	LogicInt nWordBgn = nPos;
	LogicInt nWordLen = LogicInt(0);
	LayoutInt nWordKetas = LayoutInt(0);
	while (nPos < lineStr.GetLength() && IS_KEYWORD_CHAR(lineStr.At(nPos))) {
		LayoutInt k = NativeW::GetKetaOfChar(lineStr, nPos);
		if (k == 0) {
			k = LayoutInt(1);
		}
		nWordLen += 1;
		nWordKetas += k;
		++nPos;
	}
	// 結果
	if (nWordLen > 0) {
		*p_nWordBgn = nWordBgn;
		*p_nWordLen = nWordLen;
		*p_nWordKetas = nWordKetas;
		return true;
	}else {
		return false;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      部品ステータス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Layout* LayoutMgr::LayoutWork::_CreateLayout(LayoutMgr* mgr)
{
	return mgr->CreateLayout(
		this->pDocLine,
		LogicPoint(this->nBgn, this->nCurLine),
		this->nPos - this->nBgn,
		this->colorPrev,
		this->nIndent,
		this->nPosX,
		this->exInfoPrev.DetachColorInfo()
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           部品                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool LayoutMgr::_DoKinsokuSkip(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (KinsokuType::None != pWork->eKinsokuType) {
		// 禁則処理の最後尾に達したら禁則処理中を解除する
		if (pWork->nPos >= pWork->nWordBgn + pWork->nWordLen) {
			if (1
				&& pWork->eKinsokuType == KinsokuType::Kuto
				&& pWork->nPos == pWork->nWordBgn + pWork->nWordLen
			) {
				int	nEol = pWork->pDocLine->GetEol().GetLen();

				// 改行文字をぶら下げる		//@@@ 2002.04.14 MIK
				if (
					!(1
						&& m_pTypeConfig->m_bKinsokuRet
						&& (pWork->nPos == pWork->lineStr.GetLength() - nEol)
						&& nEol
					)
				) {
				
					(this->*pfOnLine)(pWork);
				}
			}
			pWork->nWordLen = LogicInt(0);
			pWork->eKinsokuType = KinsokuType::None;	//@@@ 2002.04.20 MIK
		}
		return true;
	}else {
		return false;
	}
}

void LayoutMgr::_DoWordWrap(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (pWork->eKinsokuType == KinsokuType::None) {
		// 英単語の先頭か
		if (1
			&& pWork->nPos >= pWork->nBgn
			&& IS_KEYWORD_CHAR(pWork->lineStr.At(pWork->nPos))
		) {
			// キーワード長を取得
			LayoutInt nWordKetas = LayoutInt(0);
			_GetKeywordLength(
				pWork->lineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->eKinsokuType = KinsokuType::WordWrap;	//@@@ 2002.04.20 MIK

			if (1
				&& pWork->nPosX + nWordKetas >= GetMaxLineKetas()
				&& pWork->nPos - pWork->nBgn > 0
			) {
				(this->*pfOnLine)(pWork);
			}
		}
	}
}

void LayoutMgr::_DoKutoBurasage(LayoutWork* pWork)
{
	if (1
		&& (GetMaxLineKetas() - pWork->nPosX < 2)
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {
		// 2007.09.07 kobake   レイアウトとロジックの区別
		LayoutInt nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);

		if (1
			&& IsKinsokuPosKuto(GetMaxLineKetas() - pWork->nPosX, nCharKetas)
			&& IsKinsokuKuto(pWork->lineStr.At(pWork->nPos))
		) {
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->eKinsokuType = KinsokuType::Kuto;
		}
	}
}

void LayoutMgr::_DoGyotoKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (1
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())	// 2007.02.17 ryoji 追加
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// 2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {
		// 2007.09.07 kobake   レイアウトとロジックの区別
		LayoutInt nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		LayoutInt nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);

		if (1
			&& IsKinsokuPosHead(GetMaxLineKetas() - pWork->nPosX, nCharKetas2, nCharKetas3)
			&& IsKinsokuHead(pWork->lineStr.At(pWork->nPos + 1))
			&& ! IsKinsokuHead(pWork->lineStr.At(pWork->nPos))	// 1文字前が行頭禁則でない
			&& ! IsKinsokuKuto(pWork->lineStr.At(pWork->nPos))
		) {	// 句読点でない
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 2;
			pWork->eKinsokuType = KinsokuType::Head;

			(this->*pfOnLine)(pWork);
		}
	}
}

void LayoutMgr::_DoGyomatsuKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	if (1
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())	// 2007.02.17 ryoji 追加
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// 2004.04.09 pWork->nPosXの解釈変更のため，行頭チェックも変更
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {	// 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう)
		LayoutInt nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		LayoutInt nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);

		if (1
			&& IsKinsokuPosTail(GetMaxLineKetas() - pWork->nPosX, nCharKetas2, nCharKetas3)
			&& IsKinsokuTail(pWork->lineStr.At(pWork->nPos))
		) {
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->eKinsokuType = KinsokuType::Tail;
			
			(this->*pfOnLine)(pWork);
		}
	}
}

// 折り返す場合はtrueを返す
bool LayoutMgr::_DoTab(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	// Sep. 23, 2002 genta せっかく作ったので関数を使う
	LayoutInt nCharKetas = GetActualTabSpace(pWork->nPosX);
	if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
		(this->*pfOnLine)(pWork);
		return true;
	}
	pWork->nPosX += nCharKetas;
	pWork->nPos += LogicInt(1);
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          準処理                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_MakeOneLine(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	int	nEol = pWork->pDocLine->GetEol().GetLen(); //########そのうち不要になる
	int nEol_1 = nEol - 1;
	if (0 >	nEol_1) {
		nEol_1 = 0;
	}
	LogicInt nLength = pWork->lineStr.GetLength() - LogicInt(nEol_1);

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
	}
	ColorStrategyPool& color = *ColorStrategyPool::getInstance();

	// 1ロジック行を消化するまでループ
	while (pWork->nPos < nLength) {
		// インデント幅は_OnLineで計算済みなのでここからは削除

		// 禁則処理中ならスキップする	@@@ 2002.04.20 MIK
		if (_DoKinsokuSkip(pWork, pfOnLine)) {
		}else {
			// 英文ワードラップをする
			if (m_pTypeConfig->m_bWordWrap) {
				_DoWordWrap(pWork, pfOnLine);
			}

			// 句読点のぶらさげ
			if (m_pTypeConfig->m_bKinsokuKuto) {
				_DoKutoBurasage(pWork);
			}

			// 行頭禁則
			if (m_pTypeConfig->m_bKinsokuHead) {
				_DoGyotoKinsoku(pWork, pfOnLine);
			}

			// 行末禁則
			if (m_pTypeConfig->m_bKinsokuTail) {
				_DoGyomatsuKinsoku(pWork, pfOnLine);
			}
		}

		//@@@ 2002.09.22 YAZAKI
		color.CheckColorMODE(&pWork->pColorStrategy, pWork->nPos, pWork->lineStr);

		if (pWork->lineStr.At(pWork->nPos) == WCODE::TAB) {
			if (_DoTab(pWork, pfOnLine)) {
				continue;
			}
		}else {
			if (pWork->nPos >= pWork->lineStr.GetLength()) {
				break;
			}
			// 2007.09.07 kobake   ロジック幅とレイアウト幅を区別
			LayoutInt nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
//			if (nCharKetas == 0) {				// 削除 サロゲートペア対策	2008/7/5 Uchi
//				nCharKetas = LayoutInt(1);
//			}

			if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
				if (pWork->eKinsokuType != KinsokuType::Kuto) {
					// 改行文字をぶら下げる		//@@@ 2002.04.14 MIK
					if (!(1
						&& m_pTypeConfig->m_bKinsokuRet
						&& (pWork->nPos == pWork->lineStr.GetLength() - nEol)
						&& nEol
						)
					) {
						(this->*pfOnLine)(pWork);
						continue;
					}
				}
			}
			pWork->nPos += 1;
			pWork->nPosX += nCharKetas;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       本処理(全体)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_OnLine1(LayoutWork* pWork)
{
	AddLineBottom(pWork->_CreateLayout(this));
	pWork->pLayout = m_pLayoutBot;
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)(pWork->pLayout);
}

/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void LayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::_DoLayout");

	/*	表示上のX位置
		2004.03.28 Moca nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	int nAllLineNum;

	if (GetListenerCount() != 0) {
		NotifyProgress(0);
		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(NULL)) {
			return;
		}
	}

	_Empty();
	Init();
	
	// Nov. 16, 2002 genta
	// 折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	// TABが折り返し幅以上の時はTAB=4としてしまう
	// 折り返し幅の最小値=10なのでこの値は問題ない
	if (GetTabSpace() >= GetMaxLineKetas()) {
		m_nTabSpace = LayoutInt(4);
	}

	nAllLineNum = m_pDocLineMgr->GetLineCount();

	LayoutWork	work;
	LayoutWork* pWork = &work;
	pWork->pDocLine		= m_pDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr変更
	pWork->pLayout			= NULL;
	pWork->pColorStrategy	= NULL;
	pWork->colorPrev		= COLORIDX_DEFAULT;
	pWork->nCurLine			= LogicInt(0);

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= LogicInt(0);
		pWork->nPos			= LogicInt(0);
		pWork->nWordBgn		= LogicInt(0);
		pWork->nWordLen		= LogicInt(0);
		pWork->nPosX		= LayoutInt(0);	// 表示上のX位置
		pWork->nIndent		= LayoutInt(0);	// インデント幅

		_MakeOneLine(pWork, &LayoutMgr::_OnLine1);

		if (pWork->nPos - pWork->nBgn > 0) {
// 2002/03/13 novice
			AddLineBottom(pWork->_CreateLayout(this));
			pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
			pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
		}

		// 次の行へ
		pWork->nCurLine++;
		pWork->pDocLine = pWork->pDocLine->GetNextLine();
		
		// 処理中のユーザー操作を可能にする
		if (1
			&& GetListenerCount() != 0
			&& 0 < nAllLineNum
			&& (pWork->nCurLine % 1024) == 0
		) {
			NotifyProgress(pWork->nCurLine * 100 / nAllLineNum);
			if (!::BlockingHook(NULL)) return;
		}

// 2002/03/13 novice
	}

	// 2011.12.31 Botの色分け情報は最後に設定
	m_nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
	m_layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	m_nPrevReferLine = LayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if (GetListenerCount() != 0) {
		NotifyProgress(0);
		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(NULL)) {
			return;
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     本処理(範囲指定)                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_OnLine2(LayoutWork* pWork)
{
	//@@@ 2002.09.23 YAZAKI 最適化
	if (pWork->bNeedChangeCOMMENTMODE) {
		pWork->pLayout = pWork->pLayout->GetNextLayout();
		pWork->pLayout->SetColorTypePrev(pWork->colorPrev);
		pWork->pLayout->GetLayoutExInfo()->SetColorInfo(pWork->exInfoPrev.DetachColorInfo());
		(*pWork->pnExtInsLineNum)++;								// 再描画してほしい行数 + 1
	}else {
		pWork->pLayout = InsertLineNext(pWork->pLayout, pWork->_CreateLayout(this));
	}
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)(pWork->pLayout);
	if (0
		|| (1
			&& pWork->ptDelLogicalFrom.GetY2() == pWork->nCurLine
			&& pWork->ptDelLogicalFrom.GetX2() < pWork->nPos
			)
		|| (pWork->ptDelLogicalFrom.GetY2() < pWork->nCurLine)
	) {
		(pWork->nModifyLayoutLinesNew)++;
	}
}

/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TABが使われると折り返し位置がずれるのを防ぐため，
		pWork->nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
		固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
	@date 2009.08.28 nasukoji	テキスト最大幅の算出に対応

	@note 2004.04.03 Moca
		_DoLayoutとは違ってレイアウト情報がリスト中間に挿入されるため，
		挿入後にm_nLineTypeBotへコメントモードを指定してはならない
		代わりに最終行のコメントモードを終了間際に確認している．
*/
LayoutInt LayoutMgr::DoLayout_Range(
	Layout*			pLayoutPrev,
	LogicInt			nLineNum,
	LogicPoint			_ptDelLogicalFrom,
	EColorIndexType		nCurrentLineType,
	LayoutColorInfo*	colorInfo,
	const CalTextWidthArg*	pctwArg,
	LayoutInt*			_pnExtInsLineNum
	)
{
	*_pnExtInsLineNum = LayoutInt(0);

	LogicInt nLineNumWork = LogicInt(0);

	// 2006.12.01 Moca 途中にまで再構築した場合にEOF位置がずれたまま
	// 更新されないので，範囲にかかわらず必ずリセットする．
	m_nEOFColumn = LayoutInt(-1);
	m_nEOFLine = LayoutInt(-1);

	LayoutWork _sWork;
	LayoutWork* pWork = &_sWork;
	pWork->pLayout					= pLayoutPrev;
	pWork->pColorStrategy			= ColorStrategyPool::getInstance()->GetStrategyByColor(nCurrentLineType);
	pWork->colorPrev				= nCurrentLineType;
	pWork->exInfoPrev.SetColorInfo(colorInfo);
	pWork->bNeedChangeCOMMENTMODE	= false;
	if (!pWork->pLayout) {
		pWork->nCurLine = LogicInt(0);
	}else {
		pWork->nCurLine = pWork->pLayout->GetLogicLineNo() + LogicInt(1);
	}
	pWork->pDocLine				= m_pDocLineMgr->GetLine(pWork->nCurLine);
	pWork->nModifyLayoutLinesNew	= LayoutInt(0);
	// 引数
	pWork->ptDelLogicalFrom		= _ptDelLogicalFrom;
	pWork->pnExtInsLineNum		= _pnExtInsLineNum;

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
		pWork->pColorStrategy->SetStrategyColorInfo(colorInfo);
	}

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= LogicInt(0);
		pWork->nPos			= LogicInt(0);
		pWork->nWordBgn		= LogicInt(0);
		pWork->nWordLen		= LogicInt(0);
		pWork->nPosX		= LayoutInt(0);			// 表示上のX位置
		pWork->nIndent		= LayoutInt(0);			// インデント幅

		_MakeOneLine(pWork, &LayoutMgr::_OnLine2);

		if (pWork->nPos - pWork->nBgn > 0) {
// 2002/03/13 novice
			//@@@ 2002.09.23 YAZAKI 最適化
			_OnLine2(pWork);
		}

		++nLineNumWork;
		pWork->nCurLine++;

		// 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認
		//@@@ 2002.09.23 YAZAKI 最適化
		if (nLineNumWork >= nLineNum) {
			if (pWork->pLayout && pWork->pLayout->GetNextLayout()) {
				if (pWork->colorPrev != pWork->pLayout->GetNextLayout()->GetColorTypePrev()) {
					// COMMENTMODEが異なる行が増えましたので、次の行→次の行と更新していきます。
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& pWork->exInfoPrev.GetColorInfo()
					&& pWork->pLayout->GetNextLayout()->GetColorInfo()
					&& !pWork->exInfoPrev.GetColorInfo()->IsEqual(pWork->pLayout->GetNextLayout()->GetColorInfo())
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& pWork->exInfoPrev.GetColorInfo()
					&& !pWork->pLayout->GetNextLayout()->GetColorInfo()
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else if (1
					&& !pWork->exInfoPrev.GetColorInfo()
					&& pWork->pLayout->GetNextLayout()->GetColorInfo()
				) {
					pWork->bNeedChangeCOMMENTMODE = true;
				}else {
					break;
				}
			}else {
				break;	// while (pWork->pDocLine) 終了
			}
		}
		pWork->pDocLine = pWork->pDocLine->GetNextLine();
// 2002/03/13 novice
	}

	// 2004.03.28 Moca EOFだけの論理行の直前の行の色分けが確認・更新された
	if (pWork->nCurLine == m_pDocLineMgr->GetLineCount()) {
		m_nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
		m_layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	}

	// 2009.08.28 nasukoji	テキストが編集されたら最大幅を算出する
	CalculateTextWidth_Range(pctwArg);

// 1999.12.22 レイアウト情報がなくなる訳ではないので
// m_nPrevReferLine = 0;
// m_pLayoutPrevRefer = NULL;
// m_pLayoutCurrent = NULL;

	return pWork->nModifyLayoutLinesNew;
}

/*!
	@brief テキストが編集されたら最大幅を算出する

	@param[in] pctwArg テキスト最大幅算出用構造体

	@note 「折り返さない」選択時のみテキスト最大幅を算出する．
	      編集された行の範囲について算出する（下記を満たす場合は全行）
	      　削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
	      　削除行あり時：最大幅の行を含んで編集した
	      pctwArg->nDelLines が負数の時は削除行なし．

	@date 2009.08.28 nasukoji	新規作成
*/
void LayoutMgr::CalculateTextWidth_Range(const CalTextWidthArg* pctwArg)
{
	if (m_pEditDoc->m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {	// 「折り返さない」
		LayoutInt nCalTextWidthLinesFrom(0);	// テキスト最大幅の算出開始レイアウト行
		LayoutInt nCalTextWidthLinesTo(0);	// テキスト最大幅の算出終了レイアウト行
		bool bCalTextWidth = true;		// テキスト最大幅の算出要求をON
		LayoutInt nInsLineNum = m_nLines - pctwArg->nAllLinesOld;		// 追加削除行数

		// 削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
		// 削除行あり時：最大幅の行を含んで編集した

		if (0
			|| (1
				&& pctwArg->nDelLines < LayoutInt(0)
				&& Int(m_nTextWidth)
				&& Int(nInsLineNum)
				&& Int(pctwArg->ptLayout.x)
				&& m_nTextWidthMaxLine == pctwArg->ptLayout.y
			)
			|| (1
				&& pctwArg->nDelLines >= LayoutInt(0)
				&& Int(m_nTextWidth)
				&& pctwArg->ptLayout.y <= m_nTextWidthMaxLine
				&& m_nTextWidthMaxLine <= pctwArg->ptLayout.y + pctwArg->nDelLines 
			)
		) {
			// 全ラインを走査する
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		// 追加削除行 または 追加文字列あり
		}else if (Int(nInsLineNum) || Int(pctwArg->bInsData)) {
			// 追加削除行のみを走査する
			nCalTextWidthLinesFrom = pctwArg->ptLayout.y;

			// 最終的に編集された行数（3行削除2行追加なら2行追加）
			// 1行がMAXLINEKETASを超える場合行数が合わなくなるが、超える場合はその先の計算自体が
			// 不要なので計算を省くためこのままとする。
			LayoutInt nEditLines = nInsLineNum + ((pctwArg->nDelLines > 0) ? pctwArg->nDelLines : LayoutInt(0));
			nCalTextWidthLinesTo   = pctwArg->ptLayout.y + ((nEditLines > 0) ? nEditLines : LayoutInt(0));

			// 最大幅の行が上下するのを計算
			if (1
				&& Int(m_nTextWidth)
				&& Int(nInsLineNum)
				&& m_nTextWidthMaxLine >= pctwArg->ptLayout.y
			) {
				m_nTextWidthMaxLine += nInsLineNum;
			}
		}else {
			// 最大幅以外の行を改行を含まずに（1行内で）編集した
			bCalTextWidth = false;		// テキスト最大幅の算出要求をOFF
		}

#if defined(_DEBUG) && defined(_UNICODE)
		static int testcount = 0;
		++testcount;

		// テキスト最大幅を算出する
		if (bCalTextWidth) {
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range(%d) nCalTextWidthLinesFrom=%d nCalTextWidthLinesTo=%d\n", testcount, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
			CalculateTextWidth(false, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range() m_nTextWidthMaxLine=%d\n", m_nTextWidthMaxLine);
		}else {
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range(%d) FALSE\n", testcount);
		}
#else
		// テキスト最大幅を算出する
		if (bCalTextWidth)
			CalculateTextWidth(false, nCalTextWidthLinesFrom, nCalTextWidthLinesTo);
#endif
	}
}

