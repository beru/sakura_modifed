#pragma once

#include "view/colors/ColorStrategy.h"

class Color_Select : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_SELECT; }
	// 色替え
	virtual void InitStrategyStatus() { }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const StringRef& str, size_t nPos);

	virtual bool BeginColorEx(const StringRef& str, int nPos, int, const Layout*);

	// イベント
	virtual void OnStartScanLogic();

private:
	int nSelectLine;
	int nSelectStart;
	int nSelectEnd;
};

class Color_Found : public ColorStrategy {
public:
	Color_Found();
	virtual EColorIndexType GetStrategyColor() const {
		return
			this->validColorNum != 0
			? this->highlightColors[ (nSearchResult - 1) % this->validColorNum ]
			: COLORIDX_DEFAULT
		;
	}
	// 色替え
	virtual void InitStrategyStatus() { } //############要検証
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const StringRef& str, size_t nPos);
	// イベント
	virtual void OnStartScanLogic();

private:
	int nSearchResult;
	int nSearchStart;
	int nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< チェックが付いている検索文字列色の配列。
	unsigned validColorNum; ///< highlightColorsの何番目の要素までが有効か。
};

