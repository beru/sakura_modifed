#pragma once

#include <vector>
#include "util/design_template.h"
#include "FigureStrategy.h"

class FigureManager : public TSingleton<FigureManager> {
	friend class TSingleton<FigureManager>;
	FigureManager();
	virtual ~FigureManager();

public:
	// 描画するFigureを取得
	//	@param	pText	対象文字列の先頭
	//	@param	nTextLen	pTextから行末までの長さ(ただしCRLF==2)
	Figure& GetFigure(const wchar_t* pText, int nTextLen);

	// 設定変更
	void OnChangeSetting(void);

private:
	std::vector<Figure*>	vFigures;
	std::vector<Figure*>	vFiguresDisp;	// 色分け表示対象
};

