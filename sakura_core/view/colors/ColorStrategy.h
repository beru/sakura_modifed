#pragma once

// 要先行定義
// #include "view/EditView.h"
#include "EColorIndexType.h"
#include "uiparts/Graphics.h"

class EditView;

bool _IsPosKeywordHead(const StringRef& str, size_t nPos);

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

/*! 色定数を色番号に変換する関数 */
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

// カラー名＜＞インデックス番号の変換
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
	ColorStrategyInfo(EditView& view)
		:
		view(view),
		dispPosBegin(0, 0),
		pStrategy(nullptr),
		pStrategyFound(nullptr),
		pStrategySelect(nullptr),
		colorIdxBackLine(COLORIDX_TEXT)
	{
		index.eColorIndex = COLORIDX_TEXT;
		index.eColorIndex2 = COLORIDX_TEXT;
		index.eColorIndexBg = COLORIDX_TEXT;
	}

	// 参照
	EditView&	view;
	Graphics	gr;	// (SColorInfoでは未使用)

	// スキャン位置
	LPCWSTR			pLineOfLogic;
	int				nPosInLogic;

	// 描画位置
	DispPos*		pDispPos;
	DispPos			dispPosBegin;

	// 色変え
	ColorStrategy*		pStrategy;
	Color_Found*		pStrategyFound;
	Color_Select*		pStrategySelect;
	EColorIndexType		colorIdxBackLine;
	Color3Setting		index;

	// 色の切り替え
	bool CheckChangeColor(const StringRef& lineStr);
	void DoChangeColor(Color3Setting *pcColor);
	EColorIndexType GetCurrentColor() const { return index.eColorIndex; }
	EColorIndexType GetCurrentColor2() const { return index.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const { return index.eColorIndexBg; }

	// 現在のスキャン位置
	int GetPosInLogic() const {
		return nPosInLogic;
	}
	
	const DocLine* GetDocLine() const {
		return pDispPos->GetLayoutRef()->GetDocLineRef();
	}
	
	const Layout* GetLayout() const {
		return pDispPos->GetLayoutRef();
	}
	
};

class ColorStrategy {
public:
	virtual ~ColorStrategy() {}
	// 色定義
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual LayoutColorInfo* GetStrategyColorInfo() const {
		return nullptr;
	}
	// 色切り替え開始を検出したら、その直前までの描画を行い、さらに色設定を行う。
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo(const LayoutColorInfo* = nullptr) {};
	virtual bool BeginColor(const StringRef& str, size_t nPos) { return false; }
	virtual bool EndColor(const StringRef& str, size_t nPos) { return true; }
	virtual bool Disp() const = 0;
	// イベント
	virtual void OnStartScanLogic() {}

	// 設定更新
	virtual void Update(void) {
		const EditDoc* pEditDoc = EditDoc::GetInstance(0);
		pTypeData = &pEditDoc->docType.GetDocumentAttribute();
	}

	//#######ラップ
	EColorIndexType GetStrategyColorSafe() const { if (this) return GetStrategyColor(); else return COLORIDX_TEXT; }
	LayoutColorInfo* GetStrategyColorInfoSafe() const {
		if (this) {
			return GetStrategyColorInfo();
		}
		return nullptr;
	}

protected:
	const TypeConfig* pTypeData;
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
	ColorStrategy*	GetStrategy(size_t nIndex) const { return vStrategiesDisp[nIndex]; }
	size_t			GetStrategyCount() const { return vStrategiesDisp.size(); }
	ColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	// 特定取得
	Color_Found*   GetFoundStrategy() const { return pcFoundStrategy; }
	Color_Select*  GetSelectStrategy() const { return pcSelectStrategy; }

	// イベント
	void NotifyOnStartScanLogic();

	/*
	|| 色分け
	*/
	void CheckColorMODE(ColorStrategy** ppColorStrategy, int nPos, const StringRef& lineStr);
	bool IsSkipBeforeLayout();	// レイアウトが行頭からチェックしなくていいか判定

	// 設定変更
	void OnChangeSetting(void);

	// ビューの設定・取得
	EditView* GetCurrentView(void) const { return pView; }
	void SetCurrentView(EditView* pView) { this->pView = pView; }

private:
	std::vector<ColorStrategy*>	vStrategies;
	std::vector<ColorStrategy*>	vStrategiesDisp;	// 色分け表示対象
	Color_Found*					pcFoundStrategy;
	Color_Select*					pcSelectStrategy;

	Color_LineComment*				pcLineComment;
	Color_BlockComment*				pcBlockComment1;
	Color_BlockComment*				pcBlockComment2;
	Color_SingleQuote*				pcSingleQuote;
	Color_DoubleQuote*				pcDoubleQuote;
	Color_Heredoc*					pcHeredoc;

	EditView*						pView;

	bool	bSkipBeforeLayoutGeneral;
	bool	bSkipBeforeLayoutFound;
};

