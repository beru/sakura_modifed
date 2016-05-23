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

#include "doc/EditDoc.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"

struct DispPos {
public:
	DispPos(int nDx, int nDy)
		:
		nDx(nDx),
		nDy(nDy)
	{
		ptDrawOrigin.x = 0;
		ptDrawOrigin.y = 0;
		ptDrawLayout.x = 0;
		ptDrawLayout.y = 0;
		nLineRef = 0;
		// キャッシュ
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.GetTopLayout();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画位置                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 固定値
	void InitDrawPos(const POINT& pt) {
		ptDrawOrigin = pt;
		ptDrawLayout.x = ptDrawLayout.y = 0;
	}

	// 取得
	Point GetDrawPos() const {
		return Point(
			ptDrawOrigin.x + ptDrawLayout.x * nDx,
			ptDrawOrigin.y + ptDrawLayout.y * nDy
		);
	}

	// 進む
	void ForwardDrawCol (int nColOffset) { ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine) { ptDrawLayout.y += nOffsetLine; }

	// リセット
	void ResetDrawCol() { ptDrawLayout.x = 0; }

	// 取得
	int GetDrawCol() const { return ptDrawLayout.x; }
	int GetDrawLine() const { return ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     テキスト参照位置                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 変更
	void SetLayoutLineRef(int nOffsetLine) {
		nLineRef = nOffsetLine;
		// キャッシュ更新
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineRef);
	}
	void ForwardLayoutLineRef(int nOffsetLine);

	// 取得
	int				GetLayoutLineRef() const { return nLineRef; }
	const Layout*	GetLayoutRef() const { return pLayoutRef; }

private:
	// 固定要素
	int				nDx;			// 半角文字の文字間隔。固定。
	int				nDy;			// 半角文字の行間隔。固定。
	POINT			ptDrawOrigin;	// 描画位置基準。単位はピクセル。固定。

	// 描画位置
	LayoutPoint		ptDrawLayout;	// 描画位置。相対レイアウト単位。

	// テキスト参照位置
	int				nLineRef;		// 絶対レイアウト単位。

	// キャッシュ############
	const Layout*	pLayoutRef;
};

