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
#include "FigureManager.h"
#include "Figure_Tab.h"
#include "Figure_HanSpace.h"
#include "Figure_ZenSpace.h"
#include "Figure_Eol.h"
#include "Figure_CtrlCode.h"

FigureManager::FigureManager()
{
	vFigures.push_back(new Figure_Tab());
	vFigures.push_back(new Figure_HanSpace());
	vFigures.push_back(new Figure_ZenSpace());
	vFigures.push_back(new Figure_Eol());
	vFigures.push_back(new Figure_CtrlCode());
	vFigures.push_back(new Figure_HanBinary());
	vFigures.push_back(new Figure_ZenBinary());
	vFigures.push_back(new Figure_Text());

	OnChangeSetting();
}

FigureManager::~FigureManager()
{
	vFiguresDisp.clear();

	int size = (int)vFigures.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vFigures[i]);
	}
	vFigures.clear();
}

//$$ 高速化可能
Figure& FigureManager::GetFigure(const wchar_t* pText, int nTextLen)
{
	int size = (int)vFiguresDisp.size();
	for (int i=0; i<size; ++i) {
		Figure* pFigure = vFiguresDisp[i];
		if (pFigure->Match(pText, nTextLen)) {
			return *pFigure;
		}
	}

	assert(0);
	return *vFiguresDisp.back();
}

// 設定更新
void FigureManager::OnChangeSetting(void)
{
	vFiguresDisp.clear();

	int size = (int)vFigures.size();
	for (int i=0; i<size; ++i) {
		vFigures[i]->Update();
		// 色分け表示対象のみを登録
		if (vFigures[i]->Disp()) {
			vFiguresDisp.push_back(vFigures[i]);
		}
	}
}

