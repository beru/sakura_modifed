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

class EditView;


// クリッピング領域を計算する際のフラグ
enum EPaintArea{
	PAINT_LINENUMBER = (1 << 0), // 行番号
	PAINT_RULER      = (1 << 1), // ルーラー
	PAINT_BODY       = (1 << 2), // 本文

	// 特殊
	PAINT_ALL        = PAINT_LINENUMBER | PAINT_RULER | PAINT_BODY, // ぜんぶ
};

class EditView_Paint {
public:
	virtual EditView* GetEditView() = 0;

public:
	virtual ~EditView_Paint() {}
	void Call_OnPaint(
		int nPaintFlag,   // 描画する領域を選択する
		bool bUseMemoryDC // メモリDCを使用する
	);
};

