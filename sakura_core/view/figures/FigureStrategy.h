#pragma once

#include <vector>
#include "view/colors/ColorStrategy.h" // ColorStrategyInfo


//$$レイアウト構築フロー(DoLayout)も Figure で行うと整理しやすい
class Figure {
public:
	virtual ~Figure() {}
	virtual bool DrawImp(ColorStrategyInfo& csInfo) = 0;
	virtual bool Match(const wchar_t* pText, int nTextLen) const = 0;

	// 色分け表示対象判定
	virtual bool Disp(void) const = 0;

	// 設定更新
	virtual void Update(void) {
		EditDoc* pEditDoc = EditDoc::GetInstance(0);
		pTypeData = &pEditDoc->docType.GetDocumentAttribute();
	}
protected:
	const TypeConfig* pTypeData;
};

// 通常テキスト描画
class Figure_Text : public Figure {
public:
	bool DrawImp(ColorStrategyInfo& csInfo) override;
	bool Match(const wchar_t* pText, int nTextLen) const {
		return true;
	}

	// 色分け表示対象判定
	virtual bool Disp(void) const {
		return true;
	}
};

// 各種空白（半角空白／全角空白／タブ／改行）描画用の基本クラス
class FigureSpace : public Figure {
public:
	virtual bool DrawImp(ColorStrategyInfo& csInfo) override;
protected:
	virtual void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	// 色分け表示対象判定
	virtual bool Disp(void) const {
		EColorIndexType nColorIndex = GetColorIdx();
		return pTypeData->colorInfoArr[nColorIndex].bDisp;
	}

	virtual void Update(void) {
		Figure::Update();

		EColorIndexType nColorIndex = GetColorIdx();
		if (pTypeData->colorInfoArr[nColorIndex].bDisp) {
			nDispColorIndex = nColorIndex;
		}else {
			nDispColorIndex = COLORIDX_TEXT;
		}
	}

	EColorIndexType GetDispColorIdx(void) const { return nDispColorIndex; }

	// 実装補助
	bool DrawImp_StyleSelect(ColorStrategyInfo& csInfo);
	void DrawImp_StylePop(ColorStrategyInfo& csInfo);
	void DrawImp_DrawUnderline(ColorStrategyInfo& csInfo, DispPos&);

protected:
	EColorIndexType nDispColorIndex;
};

