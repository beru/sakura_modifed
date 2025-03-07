#include "StdAfx.h"
#include "doc/EditDoc.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "charset/charcode.h"
#include "view/EditView.h"
#include "view/colors/ColorStrategy.h"
#include "util/window.h"
#include "debug/RunningTimer.h"

static
bool _GetKeywordLength(
	const StringRef&	lineStr,		// [in]
	size_t				nPos,			// [in]
	size_t*				p_nWordBgn,		// [out]
	size_t*				p_nWordLen,		// [out]
	size_t*				p_nWordKetas	// [out]
	)
{
	// キーワード長をカウントする
	size_t nWordBgn = nPos;
	size_t nWordLen = 0;
	size_t nWordKetas = 0;
	while (nPos < lineStr.GetLength() && IS_KEYWORD_CHAR(lineStr.At(nPos))) {
		size_t k = NativeW::GetKetaOfChar(lineStr, nPos);
		if (k == 0) {
			k = 1;
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
		Point(this->nBgn, this->nCurLine),
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
	if (pWork->eKinsokuType != KinsokuType::None) {
		// 禁則処理の最後尾に達したら禁則処理中を解除する
		if (pWork->nPos >= pWork->nWordBgn + pWork->nWordLen) {
			if (1
				&& pWork->eKinsokuType == KinsokuType::Kuto
				&& pWork->nPos == pWork->nWordBgn + pWork->nWordLen
			) {
				size_t nEol = pWork->pDocLine->GetEol().GetLen();
				// 改行文字をぶら下げる
				if (
					!(1
						&& pTypeConfig->bKinsokuRet
						&& (pWork->nPos == pWork->lineStr.GetLength() - nEol)
						&& nEol
					)
				) {
				
					(this->*pfOnLine)(pWork);
				}
			}
			pWork->nWordLen = 0;
			pWork->eKinsokuType = KinsokuType::None;
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
			size_t nWordKetas = 0;
			_GetKeywordLength(
				pWork->lineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->eKinsokuType = KinsokuType::WordWrap;

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
		// レイアウトとロジックの区別
		size_t nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
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
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// pWork->nPosXの解釈変更のため，行頭チェックも変更
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {
		// レイアウトとロジックの区別
		size_t nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		size_t nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);

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
		&& (pWork->nPos + 1 < pWork->lineStr.GetLength())
		&& (GetMaxLineKetas() - pWork->nPosX < 4)
		&& (pWork->nPosX > pWork->nIndent)	// pWork->nPosXの解釈変更のため，行頭チェックも変更
		&& (pWork->eKinsokuType == KinsokuType::None)
	) {	// 行末禁則する && 行末付近 && 行頭でないこと(無限に禁則してしまいそう)
		size_t nCharKetas2 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
		size_t nCharKetas3 = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos + 1);
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
	// せっかく作ったので関数を使う
	size_t nCharKetas = GetActualTabSpace(pWork->nPosX);
	if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
		(this->*pfOnLine)(pWork);
		return true;
	}
	pWork->nPosX += nCharKetas;
	pWork->nPos += 1;
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          準処理                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void LayoutMgr::_MakeOneLine(LayoutWork* pWork, PF_OnLine pfOnLine)
{
	size_t nEol = pWork->pDocLine->GetEol().GetLen(); //########そのうち不要になる
	size_t nEol_1 = (nEol == 0) ? 0 : (nEol - 1);
	ASSERT_GE(pWork->lineStr.GetLength(), nEol_1);
	size_t nLength = pWork->lineStr.GetLength() - nEol_1;

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
	}
	auto& color = ColorStrategyPool::getInstance();

	// 1ロジック行を消化するまでループ
	while (pWork->nPos < nLength) {
		// インデント幅は_OnLineで計算済みなのでここからは削除

		// 禁則処理中ならスキップする
		if (_DoKinsokuSkip(pWork, pfOnLine)) {
		}else {
			// 英文ワードラップをする
			if (pTypeConfig->bWordWrap) {
				_DoWordWrap(pWork, pfOnLine);
			}

			// 句読点のぶらさげ
			if (pTypeConfig->bKinsokuKuto) {
				_DoKutoBurasage(pWork);
			}

			// 行頭禁則
			if (pTypeConfig->bKinsokuHead) {
				_DoGyotoKinsoku(pWork, pfOnLine);
			}

			// 行末禁則
			if (pTypeConfig->bKinsokuTail) {
				_DoGyomatsuKinsoku(pWork, pfOnLine);
			}
		}

		color.CheckColorMODE(&pWork->pColorStrategy, pWork->nPos, pWork->lineStr);

		if (pWork->lineStr.At(pWork->nPos) == WCODE::TAB) {
			if (_DoTab(pWork, pfOnLine)) {
				continue;
			}
		}else {
			if (pWork->nPos >= pWork->lineStr.GetLength()) {
				break;
			}
			// ロジック幅とレイアウト幅を区別
			size_t nCharKetas = NativeW::GetKetaOfChar(pWork->lineStr, pWork->nPos);
			if (pWork->nPosX + nCharKetas > GetMaxLineKetas()) {
				if (pWork->eKinsokuType != KinsokuType::Kuto) {
					// 改行文字をぶら下げる
					if (!(1
						&& pTypeConfig->bKinsokuRet
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
	pWork->pLayout = pLayoutBot;
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	pWork->nBgn = pWork->nPos;
	// pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*getIndentOffset)(pWork->pLayout);
}

/*!
	現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します

	TABが使われると折り返し位置がずれるのを防ぐため，
	nPosXがインデントを含む幅を保持するように変更．m_nMaxLineKetasは
	固定値となったが，既存コードの置き換えは避けて最初に値を代入するようにした．
*/
void LayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::_DoLayout");

	/*	表示上のX位置
		nPosXはインデント幅を含むように変更(TAB位置調整のため)
	*/
	if (GetListenerCount() != 0) {
		NotifyProgress(0);
		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(NULL)) {
			return;
		}
	}

	_Empty();
	Init();
	
	// 折り返し幅 <= TAB幅のとき無限ループするのを避けるため，
	// TABが折り返し幅以上の時はTAB=4としてしまう
	// 折り返し幅の最小値=10なのでこの値は問題ない
	if (GetTabSpace() >= GetMaxLineKetas()) {
		nTabSpace = 4;
	}
	size_t nAllLineNum = pDocLineMgr->GetLineCount();
	LayoutWork	work;
	LayoutWork* pWork = &work;
	pWork->pDocLine		= pDocLineMgr->GetDocLineTop();
	pWork->pLayout			= nullptr;
	pWork->pColorStrategy	= nullptr;
	pWork->colorPrev		= COLORIDX_DEFAULT;
	pWork->nCurLine			= 0;

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;
		pWork->nBgn			= 0;
		pWork->nPos			= 0;
		pWork->nWordBgn		= 0;
		pWork->nWordLen		= 0;
		pWork->nPosX		= 0;	// 表示上のX位置
		pWork->nIndent		= 0;	// インデント幅

		_MakeOneLine(pWork, &LayoutMgr::_OnLine1);

		if (pWork->nPos - pWork->nBgn > 0) {
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
	}

	// Botの色分け情報は最後に設定
	nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
	layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	nPrevReferLine = 0;
	pLayoutPrevRefer = nullptr;

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
	if (pWork->bNeedChangeCOMMENTMODE) {
		pWork->pLayout = pWork->pLayout->GetNextLayout();
		pWork->pLayout->SetColorTypePrev(pWork->colorPrev);
		pWork->pLayout->GetLayoutExInfo()->SetColorInfo(pWork->exInfoPrev.DetachColorInfo());
	}else {
		pWork->pLayout = InsertLineNext(pWork->pLayout, pWork->_CreateLayout(this));
	}
	pWork->colorPrev = pWork->pColorStrategy->GetStrategyColorSafe();
	pWork->exInfoPrev.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());

	pWork->nBgn = pWork->nPos;
	// pWork->nPosXはインデント幅を含むように変更(TAB位置調整のため)
	pWork->nPosX = pWork->nIndent = (this->*getIndentOffset)(pWork->pLayout);
	if (0
		|| (1
			&& pWork->ptDelLogicalFrom.y == pWork->nCurLine
			&& pWork->ptDelLogicalFrom.x < (int)pWork->nPos
			)
		|| (pWork->ptDelLogicalFrom.y < pWork->nCurLine)
	) {
		(pWork->nModifyLayoutLinesNew)++;
	}
}

/*!
	指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	
		_DoLayoutとは違ってレイアウト情報がリスト中間に挿入されるため，
		挿入後にnLineTypeBotへコメントモードを指定してはならない
		代わりに最終行のコメントモードを終了間際に確認している．
*/
int LayoutMgr::DoLayout_Range(
	Layout*				pLayoutPrev,
	int					nLineNum,
	Point				_ptDelLogicalFrom,
	EColorIndexType		nCurrentLineType,
	LayoutColorInfo*	colorInfo,
	const CalTextWidthArg&	ctwArg
	)
{

	int nLineNumWork = 0;

	// 途中にまで再構築した場合にEOF位置がずれたまま
	// 更新されないので，範囲にかかわらず必ずリセットする．
	nEOFColumn = -1;
	nEOFLine = -1;

	LayoutWork work;
	LayoutWork* pWork = &work;
	pWork->pLayout					= pLayoutPrev;
	pWork->pColorStrategy			= ColorStrategyPool::getInstance().GetStrategyByColor(nCurrentLineType);
	pWork->colorPrev				= nCurrentLineType;
	pWork->exInfoPrev.SetColorInfo(colorInfo);
	pWork->bNeedChangeCOMMENTMODE	= false;
	if (!pWork->pLayout) {
		pWork->nCurLine = 0;
	}else {
		pWork->nCurLine = pWork->pLayout->GetLogicLineNo() + 1;
	}
	pWork->pDocLine					= pDocLineMgr->GetLine(pWork->nCurLine);
	pWork->nModifyLayoutLinesNew	= 0;
	// 引数
	pWork->ptDelLogicalFrom		= _ptDelLogicalFrom;

	if (pWork->pColorStrategy) {
		pWork->pColorStrategy->InitStrategyStatus();
		pWork->pColorStrategy->SetStrategyColorInfo(colorInfo);
	}

	while (pWork->pDocLine) {
		pWork->lineStr		= pWork->pDocLine->GetStringRefWithEOL();
		pWork->eKinsokuType	= KinsokuType::None;
		pWork->nBgn			= 0;
		pWork->nPos			= 0;
		pWork->nWordBgn		= 0;
		pWork->nWordLen		= 0;
		pWork->nPosX		= 0;			// 表示上のX位置
		pWork->nIndent		= 0;			// インデント幅

		_MakeOneLine(pWork, &LayoutMgr::_OnLine2);

		if (pWork->nPos - pWork->nBgn > 0) {
			_OnLine2(pWork);
		}

		++nLineNumWork;
		pWork->nCurLine++;

		// 目的の行数(nLineNum)に達したか、または通り過ぎた（＝行数が増えた）か確認
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
	}

	// EOFだけの論理行の直前の行の色分けが確認・更新された
	if (pWork->nCurLine == pDocLineMgr->GetLineCount()) {
		nLineTypeBot = pWork->pColorStrategy->GetStrategyColorSafe();
		layoutExInfoBot.SetColorInfo(pWork->pColorStrategy->GetStrategyColorInfoSafe());
	}

	// テキストが編集されたら最大幅を算出する
	CalculateTextWidth_Range(ctwArg);

	return pWork->nModifyLayoutLinesNew;
}

/*!
	@brief テキストが編集されたら最大幅を算出する

	@param[in] ctwArg テキスト最大幅算出用構造体

	@note 「折り返さない」選択時のみテキスト最大幅を算出する．
	      編集された行の範囲について算出する（下記を満たす場合は全行）
	      　削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
	      　削除行あり時：最大幅の行を含んで編集した
	      ctwArg.nDelLines が負数の時は削除行なし．
*/
void LayoutMgr::CalculateTextWidth_Range(const CalTextWidthArg& ctwArg)
{
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {	// 「折り返さない」
		int nCalTextWidthLinesFrom = 0;	// テキスト最大幅の算出開始レイアウト行
		int nCalTextWidthLinesTo = 0;	// テキスト最大幅の算出終了レイアウト行
		bool bCalTextWidth = true;		// テキスト最大幅の算出要求をON
		ASSERT_GE(nLines, ctwArg.nAllLinesOld);
		size_t nInsLineNum = nLines - ctwArg.nAllLinesOld;		// 追加削除行数

		// 削除行なし時：最大幅の行を行頭以外にて改行付きで編集した
		// 削除行あり時：最大幅の行を含んで編集した

		if (0
			|| (1
				&& ctwArg.nDelLines < 0
				&& nTextWidth
				&& nInsLineNum
				&& ctwArg.ptLayout.x
				&& nTextWidthMaxLine == ctwArg.ptLayout.y
			)
			|| (1
				&& ctwArg.nDelLines >= 0
				&& nTextWidth
				&& ctwArg.ptLayout.y <= (int)nTextWidthMaxLine
				&& (int)nTextWidthMaxLine <= ctwArg.ptLayout.y + ctwArg.nDelLines 
			)
		) {
			// 全ラインを走査する
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		// 追加削除行 または 追加文字列あり
		}else if (nInsLineNum || ctwArg.bInsData) {
			// 追加削除行のみを走査する
			nCalTextWidthLinesFrom = ctwArg.ptLayout.y;

			// 最終的に編集された行数（3行削除2行追加なら2行追加）
			// 1行がMAXLINEKETASを超える場合行数が合わなくなるが、超える場合はその先の計算自体が
			// 不要なので計算を省くためこのままとする。
			size_t nEditLines = nInsLineNum + ((ctwArg.nDelLines > 0) ? ctwArg.nDelLines : 0);
			nCalTextWidthLinesTo = ctwArg.ptLayout.y + ((nEditLines > 0) ? nEditLines : 0);

			// 最大幅の行が上下するのを計算
			if (1
				&& nTextWidth
				&& nInsLineNum
				&& (int)nTextWidthMaxLine >= ctwArg.ptLayout.y
			) {
				nTextWidthMaxLine += nInsLineNum;
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
//			MYTRACE_W(L"LayoutMgr::DoLayout_Range() nTextWidthMaxLine=%d\n", nTextWidthMaxLine);
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

