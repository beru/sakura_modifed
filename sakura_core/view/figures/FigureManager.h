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
	std::vector<Figure*>	m_vFigures;
	std::vector<Figure*>	m_vFiguresDisp;	// 色分け表示対象
};

