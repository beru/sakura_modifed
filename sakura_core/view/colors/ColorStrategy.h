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
#pragma once

// 要先行定義
// #include "view/EditView.h"
#include "EColorIndexType.h"
#include "uiparts/Graphics.h"

class EditView;

bool _IsPosKeywordHead(const StringRef& str, int nPos);

// 正規表現キーワードのEColorIndexType値を作る関数
inline
EColorIndexType ToColorIndexType_RegularExpression(const int nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

// 正規表現キーワードのEColorIndexType値を色番号に戻す関数
inline
int ToColorInfoArrIndex_RegularExpression(const EColorIndexType eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

/*! 色定数を色番号に変換する関数

	@date 2013.05.08 novice 範囲外のときはテキストを選択する
*/
inline
int ToColorInfoArrIndex(const EColorIndexType eColorIndex)
{
	if (eColorIndex >= 0 && eColorIndex < COLORIDX_LAST)
		return eColorIndex;
	else if (eColorIndex & COLORIDX_BLOCK_BIT)
		return COLORIDX_COMMENT;
	else if (eColorIndex & COLORIDX_REGEX_BIT)
		return ToColorInfoArrIndex_RegularExpression(eColorIndex);

	assert(0); // ここには来ない
	return COLORIDX_TEXT;
}

// カラー名＜＞インデックス番号の変換	//@@@ 2002.04.30
int GetColorIndexByName(const TCHAR* name);
const TCHAR* GetColorNameByIndex(int index);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           基底                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class ColorStrategy;
#include "view/DispPos.h"

class Color_Found;
class Color_Select;

// 色設定
struct Color3Setting {
	EColorIndexType eColorIndex;    // 選択を含む現在の色
	EColorIndexType eColorIndex2;   // 選択以外の現在の色
	EColorIndexType eColorIndexBg;  // 背景色
};

struct ColorStrategyInfo {
	ColorStrategyInfo()
		:
		m_dispPosBegin(0, 0),
		m_pStrategy(NULL),
		m_pStrategyFound(NULL),
		m_pStrategySelect(NULL),
		m_colorIdxBackLine(COLORIDX_TEXT)
	{
		m_cIndex.eColorIndex = COLORIDX_TEXT;
		m_cIndex.eColorIndex2 = COLORIDX_TEXT;
		m_cIndex.eColorIndexBg = COLORIDX_TEXT;
	}

	// 参照
	EditView*	m_pView;
	Graphics	m_gr;	// (SColorInfoでは未使用)

	// スキャン位置
	LPCWSTR			m_pLineOfLogic;
	LogicInt		m_nPosInLogic;

	// 描画位置
	DispPos*		m_pDispPos;
	DispPos			m_dispPosBegin;

	// 色変え
	ColorStrategy*		m_pStrategy;
	Color_Found*		m_pStrategyFound;
	Color_Select*		m_pStrategySelect;
	EColorIndexType		m_colorIdxBackLine;
	Color3Setting		m_cIndex;

	// 色の切り替え
	bool CheckChangeColor(const StringRef& lineStr);
	void DoChangeColor(Color3Setting *pcColor);
	EColorIndexType GetCurrentColor() const { return m_cIndex.eColorIndex; }
	EColorIndexType GetCurrentColor2() const { return m_cIndex.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const { return m_cIndex.eColorIndexBg; }

	// 現在のスキャン位置
	LogicInt GetPosInLogic() const {
		return m_nPosInLogic;
	}
	
	const DocLine* GetDocLine() const {
		return m_pDispPos->GetLayoutRef()->GetDocLineRef();
	}
	
	const Layout* GetLayout() const {
		return m_pDispPos->GetLayoutRef();
	}
	
};

class ColorStrategy {
public:
	virtual ~ColorStrategy() {}
	// 色定義
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual LayoutColorInfo* GetStrategyColorInfo() const {
		return NULL;
	}
	// 色切り替え開始を検出したら、その直前までの描画を行い、さらに色設定を行う。
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo(const LayoutColorInfo* = NULL) {};
	virtual bool BeginColor(const StringRef& str, int nPos) { return false; }
	virtual bool EndColor(const StringRef& str, int nPos) { return true; }
	virtual bool Disp() const = 0;
	// イベント
	virtual void OnStartScanLogic() {}

	// 設定更新
	virtual void Update(void) {
		const EditDoc* pEditDoc = EditDoc::GetInstance(0);
		m_pTypeData = &pEditDoc->m_docType.GetDocumentAttribute();
	}

	//#######ラップ
	EColorIndexType GetStrategyColorSafe() const { if (this) return GetStrategyColor(); else return COLORIDX_TEXT; }
	LayoutColorInfo* GetStrategyColorInfoSafe() const {
		if (this) {
			return GetStrategyColorInfo();
		}
		return NULL;
	}

protected:
	const TypeConfig* m_pTypeData;
};

#include "util/design_template.h"
#include <vector>
class Color_LineComment;
class Color_BlockComment;
class Color_BlockComment;
class Color_SingleQuote;
class Color_DoubleQuote;
class Color_Heredoc;

class ColorStrategyPool : public TSingleton<ColorStrategyPool> {
	friend class TSingleton<ColorStrategyPool>;
	ColorStrategyPool();
	virtual ~ColorStrategyPool();

public:

	// 取得
	ColorStrategy*	GetStrategy(int nIndex) const { return m_vStrategiesDisp[nIndex]; }
	int				GetStrategyCount() const { return (int)m_vStrategiesDisp.size(); }
	ColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	// 特定取得
	Color_Found*   GetFoundStrategy() const { return m_pcFoundStrategy; }
	Color_Select*  GetSelectStrategy() const { return m_pcSelectStrategy; }

	// イベント
	void NotifyOnStartScanLogic();

	/*
	|| 色分け
	*/
	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void CheckColorMODE(ColorStrategy** ppColorStrategy, int nPos, const StringRef& lineStr);
	bool IsSkipBeforeLayout();	// レイアウトが行頭からチェックしなくていいか判定

	// 設定変更
	void OnChangeSetting(void);

	// ビューの設定・取得
	EditView* GetCurrentView(void) const { return m_pView; }
	void SetCurrentView(EditView* pView) { m_pView = pView; }

private:
	std::vector<ColorStrategy*>	m_vStrategies;
	std::vector<ColorStrategy*>	m_vStrategiesDisp;	// 色分け表示対象
	Color_Found*					m_pcFoundStrategy;
	Color_Select*					m_pcSelectStrategy;

	Color_LineComment*				m_pcLineComment;
	Color_BlockComment*				m_pcBlockComment1;
	Color_BlockComment*				m_pcBlockComment2;
	Color_SingleQuote*				m_pcSingleQuote;
	Color_DoubleQuote*				m_pcDoubleQuote;
	Color_Heredoc*					m_pcHeredoc;

	EditView*						m_pView;

	bool	m_bSkipBeforeLayoutGeneral;
	bool	m_bSkipBeforeLayoutFound;
};

