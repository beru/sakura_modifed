/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "view/CEditView.h" // ColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CColor_Comment.h"
#include "CColor_Quote.h"
#include "CColor_RegexKeyword.h"
#include "CColor_Found.h"
#include "CColor_Url.h"
#include "CColor_Numeric.h"
#include "CColor_KeywordSet.h"
#include "CColor_Found.h"
#include "CColor_Heredoc.h"
#include "doc/layout/CLayout.h"
#include "window/CEditWnd.h"
#include "types/CTypeSupport.h"


bool _IsPosKeywordHead(const StringRef& cStr, int nPos)
{
	return (nPos == 0 || !IS_KEYWORD_CHAR(cStr.At(nPos - 1)));
}

/*! 色の切り替え判定
	@retval true 色の変更あり
	@retval false 色の変更なし
*/
bool ColorStrategyInfo::CheckChangeColor(const StringRef& lineStr)
{
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pView);
	Color_Found*  pcFound  = pool->GetFoundStrategy();
	Color_Select* pcSelect = pool->GetSelectStrategy();
	bool bChange = false;

	// 選択範囲色終了
	if (m_pStrategySelect) {
		if (m_pStrategySelect->EndColor(lineStr, this->GetPosInLogic())) {
			m_pStrategySelect = NULL;
			bChange = true;
		}
	}
	// 選択範囲色開始
	if (!m_pStrategySelect) {
		if (pcSelect->BeginColorEx(lineStr, this->GetPosInLogic(), m_pDispPos->GetLayoutLineRef(), this->GetLayout())) {
			m_pStrategySelect = pcSelect;
			bChange = true;
		}
	}

	// 検索色終了
	if (m_pStrategyFound) {
		if (m_pStrategyFound->EndColor(lineStr, this->GetPosInLogic())) {
			m_pStrategyFound = NULL;
			bChange = true;
		}
	}

	// 検索色開始
	if (!m_pStrategyFound) {
		if (pcFound->BeginColor(lineStr, this->GetPosInLogic())) {
			m_pStrategyFound = pcFound;
			bChange = true;
		}
	}

	// 色終了
	if (m_pStrategy) {
		if (m_pStrategy->EndColor(lineStr, this->GetPosInLogic())) {
			m_pStrategy = NULL;
			bChange = true;
		}
	}

	// 色開始
	if (!m_pStrategy) {
		int size = pool->GetStrategyCount();
		for (int i=0; i<size; ++i) {
			if (pool->GetStrategy(i)->BeginColor(lineStr, this->GetPosInLogic())) {
				m_pStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}

	// カーソル行背景色
	TypeSupport cCaretLineBg(m_pView, COLORIDX_CARETLINEBG);
	if (cCaretLineBg.IsDisp() && !m_pView->m_bMiniMap) {
		if (m_colorIdxBackLine == COLORIDX_CARETLINEBG) {
			if (m_pDispPos->GetLayoutLineRef() != m_pView->GetCaret().GetCaretLayoutPos().GetY2()) {
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else {
			if (m_pDispPos->GetLayoutLineRef() == m_pView->GetCaret().GetCaretLayoutPos().GetY2()) {
				m_colorIdxBackLine = COLORIDX_CARETLINEBG;
				bChange = true;
			}
		}
	}
	// 偶数行の背景色
	TypeSupport cEvenLineBg(m_pView, COLORIDX_EVENLINEBG);
	if (cEvenLineBg.IsDisp() && !m_pView->m_bMiniMap && m_colorIdxBackLine != COLORIDX_CARETLINEBG) {
		if (m_colorIdxBackLine == COLORIDX_EVENLINEBG) {
			if (m_pDispPos->GetLayoutLineRef() % 2 == 0) {
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else {
			if (m_pDispPos->GetLayoutLineRef() % 2 == 1) {
				m_colorIdxBackLine = COLORIDX_EVENLINEBG;
				bChange = true;
			}
		}
	}
	if (m_pView->m_bMiniMap) {
		TypeSupport cPageViewBg(m_pView, COLORIDX_PAGEVIEW);
		if (cPageViewBg.IsDisp()) {
			EditView& cActiveView = m_pView->m_pEditWnd->GetActiveView();
			LayoutInt curLine = m_pDispPos->GetLayoutLineRef();
			if (m_colorIdxBackLine == COLORIDX_PAGEVIEW) {
				if (cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine()) {
				}else {
					m_colorIdxBackLine = COLORIDX_TEXT;
					bChange = true;
				}
			}else if (m_colorIdxBackLine == COLORIDX_TEXT) {
				if (cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine()) {
					m_colorIdxBackLine = COLORIDX_PAGEVIEW;
					bChange = true;
				}
			}
		}
	}

	return bChange;
}

/*! 色の切り替え

	@date 2013.05.11 novice 実際の変更は呼び出し側で行う
*/
void ColorStrategyInfo::DoChangeColor(Color3Setting *pcColor)
{
	if (m_pStrategySelect) {
		m_cIndex.eColorIndex = m_pStrategySelect->GetStrategyColor();
	}else if (m_pStrategyFound) {
		m_cIndex.eColorIndex = m_pStrategyFound->GetStrategyColor();
	}else {
		m_cIndex.eColorIndex = m_pStrategy->GetStrategyColorSafe();
	}

	if (m_pStrategyFound) {
		m_cIndex.eColorIndex2 = m_pStrategyFound->GetStrategyColor();
	}else {
		m_cIndex.eColorIndex2 = m_pStrategy->GetStrategyColorSafe();
	}

	m_cIndex.eColorIndexBg = m_colorIdxBackLine;

	*pcColor = m_cIndex;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          プール                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

ColorStrategyPool::ColorStrategyPool()
{
	m_pView = &(EditWnd::getInstance()->GetView(0));
	m_pcSelectStrategy = new Color_Select();
	m_pcFoundStrategy = new Color_Found();
//	m_vStrategies.push_back(new Color_Found);				// マッチ文字列
	m_vStrategies.push_back(new Color_RegexKeyword);		// 正規表現キーワード
	m_vStrategies.push_back(new Color_Heredoc);			// ヒアドキュメント
	m_vStrategies.push_back(new Color_BlockComment(COLORIDX_BLOCK1));	// ブロックコメント
	m_vStrategies.push_back(new Color_BlockComment(COLORIDX_BLOCK2));	// ブロックコメント2
	m_vStrategies.push_back(new Color_LineComment);		// 行コメント
	m_vStrategies.push_back(new Color_SingleQuote);		// シングルクォーテーション文字列
	m_vStrategies.push_back(new Color_DoubleQuote);		// ダブルクォーテーション文字列
	m_vStrategies.push_back(new Color_Url);				// URL
	m_vStrategies.push_back(new Color_Numeric);			// 半角数字
	m_vStrategies.push_back(new Color_KeywordSet);			// キーワードセット

	// 設定更新
	OnChangeSetting();
}

ColorStrategyPool::~ColorStrategyPool()
{
	SAFE_DELETE(m_pcSelectStrategy);
	SAFE_DELETE(m_pcFoundStrategy);
	m_vStrategiesDisp.clear();
	int size = (int)m_vStrategies.size();
	for (int i=0; i<size; ++i) {
		delete m_vStrategies[i];
	}
	m_vStrategies.clear();
}

ColorStrategy*	ColorStrategyPool::GetStrategyByColor(EColorIndexType eColor) const
{
	if (COLORIDX_SEARCH <= eColor && eColor <= COLORIDX_SEARCHTAIL) {
		return m_pcFoundStrategy;
	}
	int size = (int)m_vStrategiesDisp.size();
	for (int i=0; i<size; ++i) {
		if (m_vStrategiesDisp[i]->GetStrategyColor() == eColor) {
			return m_vStrategiesDisp[i];
		}
	}
	return NULL;
}

void ColorStrategyPool::NotifyOnStartScanLogic()
{
	m_pcSelectStrategy->OnStartScanLogic();
	m_pcFoundStrategy->OnStartScanLogic();
	int size = GetStrategyCount();
	for (int i=0; i<size; ++i) {
		GetStrategy(i)->OnStartScanLogic();
	}
}

// 2005.11.20 Mocaコメントの色分けがON/OFF関係なく行われていたバグを修正
void ColorStrategyPool::CheckColorMODE(
	ColorStrategy**	ppcColorStrategy,	// [in/out]
	int					nPos,
	const StringRef&	lineStr
)
{
	// 色終了
	if (*ppcColorStrategy) {
		if ((*ppcColorStrategy)->EndColor(lineStr, nPos)) {
			*ppcColorStrategy = NULL;
		}
	}

	// 色開始
	if (!*ppcColorStrategy) {
		// CheckColorMODE はレイアウト処理全体のボトルネックになるくらい頻繁に呼び出される
		// 基本クラスからの動的仮想関数呼び出しを使用すると無視できないほどのオーバヘッドになる模様
		// ここはエレガントさよりも性能優先で個々の派生クラスから BeginColor() を呼び出す
		if (m_pcHeredoc && m_pcHeredoc->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcHeredoc; return; }
		if (m_pcBlockComment1 && m_pcBlockComment1->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcBlockComment1; return; }
		if (m_pcBlockComment2 && m_pcBlockComment2->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcBlockComment2; return; }
		if (m_pcLineComment && m_pcLineComment->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcLineComment; return; }
		if (m_pcSingleQuote && m_pcSingleQuote->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcSingleQuote; return; }
		if (m_pcDoubleQuote && m_pcDoubleQuote->BeginColor(lineStr, nPos)) { *ppcColorStrategy = m_pcDoubleQuote; return; }
	}
}

/*! 設定更新
*/
void ColorStrategyPool::OnChangeSetting(void)
{
	m_vStrategiesDisp.clear();

	m_pcSelectStrategy->Update();
	m_pcFoundStrategy->Update();
	int size = (int)m_vStrategies.size();
	for (int i=0; i<size; ++i) {
		m_vStrategies[i]->Update();

		// 色分け表示対象であれば登録
		if (m_vStrategies[i]->Disp()) {
			m_vStrategiesDisp.push_back(m_vStrategies[i]);
		}
	}

	// CheckColorMODE 用
	m_pcHeredoc = static_cast<Color_Heredoc*>(GetStrategyByColor(COLORIDX_HEREDOC));
	m_pcBlockComment1 = static_cast<Color_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK1));	// ブロックコメント
	m_pcBlockComment2 = static_cast<Color_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK2));	// ブロックコメント2
	m_pcLineComment = static_cast<Color_LineComment*>(GetStrategyByColor(COLORIDX_COMMENT));	// 行コメント
	m_pcSingleQuote = static_cast<Color_SingleQuote*>(GetStrategyByColor(COLORIDX_SSTRING));	// シングルクォーテーション文字列
	m_pcDoubleQuote = static_cast<Color_DoubleQuote*>(GetStrategyByColor(COLORIDX_WSTRING));	// ダブルクォーテーション文字列

	// 色分けをしない場合に、処理をスキップできるように確認する
	const TypeConfig& type = EditDoc::GetInstance(0)->m_docType.GetDocumentAttribute();
	EColorIndexType bSkipColorTypeTable[] = {
		COLORIDX_DIGIT,
		COLORIDX_COMMENT,
		COLORIDX_SSTRING,
		COLORIDX_WSTRING,
		COLORIDX_HEREDOC,
		COLORIDX_URL,
		COLORIDX_KEYWORD1,
		COLORIDX_KEYWORD2,
		COLORIDX_KEYWORD3,
		COLORIDX_KEYWORD4,
		COLORIDX_KEYWORD5,
		COLORIDX_KEYWORD6,
		COLORIDX_KEYWORD7,
		COLORIDX_KEYWORD8,
		COLORIDX_KEYWORD9,
		COLORIDX_KEYWORD10,
	};
	m_bSkipBeforeLayoutGeneral = true;
	int nKeyword1;
	int bUnuseKeyword = false;
	for (int n=0; n<_countof(bSkipColorTypeTable); ++n) {
		if (bSkipColorTypeTable[n] == COLORIDX_KEYWORD1) {
			nKeyword1 = n;
		}
		if (COLORIDX_KEYWORD1 <= bSkipColorTypeTable[n]
			&& bSkipColorTypeTable[n] <= COLORIDX_KEYWORD10
		) {
			if (type.m_nKeyWordSetIdx[n - nKeyword1] == -1) {
				bUnuseKeyword = true; // -1以降は無効
			}
			if (!bUnuseKeyword && type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp) {
				m_bSkipBeforeLayoutGeneral = false;
				break;
			}
		}else if (type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp) {
			m_bSkipBeforeLayoutGeneral = false;
			break;
		}
	}
	if (m_bSkipBeforeLayoutGeneral) {
		if (type.m_bUseRegexKeyword) {
			m_bSkipBeforeLayoutGeneral = false;
		}
	}
	m_bSkipBeforeLayoutFound = true;
	for (int n=COLORIDX_SEARCH; n<=COLORIDX_SEARCHTAIL; ++n) {
		if (type.m_ColorInfoArr[n].m_bDisp) {
			m_bSkipBeforeLayoutFound = false;
			break;
		}
	}
}

bool ColorStrategyPool::IsSkipBeforeLayout()
{
	if (!m_bSkipBeforeLayoutGeneral) {
		return false;
	}
	if (!m_bSkipBeforeLayoutFound && m_pView->m_bCurSrchKeyMark) {
		return false;
	}
	return true;
}

/*!
  iniの色設定を番号でなく文字列で書き出す。(added by Stonee, 2001/01/12, 2001/01/15)
  配列の順番は共有メモリ中のデータの順番と一致している。

  @note 数値による内部的対応は EColorIndexType EColorIndexType.h
  日本語名などは  ColorInfo_DEFAULT CDocTypeSetting.cpp
  CShareDataからglobalに移動
*/
const ColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// キャレット		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IMEキャレット	// 2006.12.07 ryoji
	{_T("CBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | (COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD)}, // 2007.09.09 Moca カーソル位置縦線
	{_T("NOT"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("EBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	// 2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca 指定桁縦線
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK 半角数値の強調
	{_T("BRC"), 0},	// 対括弧	// 02/09/18 ai Add
	{_T("SEL"), 0},
	{_T("FND"), 0},
	{_T("FN2"), 0},
	{_T("FN3"), 0},
	{_T("FN4"), 0},
	{_T("FN5"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("HDC"), 0},
	{_T("URL"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK 強調キーワード3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	// DIFF追加	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	// DIFF変更	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	// DIFF削除	//@@@ 2002.06.01 MIK
	{_T("MRK"), 0},	// ブックマーク	// 02/10/16 ai Add
	{_T("PGV"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LAST"), 0}	// Not Used
};


/*
 * カラー名からインデックス番号に変換する
 */
int GetColorIndexByName(const TCHAR* name)
{
	for (int i=0; i<COLORIDX_LAST; ++i) {
		if (_tcscmp(name, g_ColorAttributeArr[i].szName) == 0) return i;
	}
	return -1;
}

/*
 * インデックス番号からカラー名に変換する
 */
const TCHAR* GetColorNameByIndex(int index)
{
	return g_ColorAttributeArr[index].szName;
}

