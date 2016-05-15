/*!	@file
	@brief 編集操作要素ブロック

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "OpeBlk.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

OpeBlk::OpeBlk()
{
	refCount = 0;
}

OpeBlk::~OpeBlk()
{
	// 操作の配列を削除する
	int size = (int)ppCOpeArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(ppCOpeArr[i]);
	}
	ppCOpeArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 操作の追加
bool OpeBlk::AppendOpe(Ope* pOpe)
{
	if (pOpe->ptCaretPos_PHY_Before.HasNegative() || pOpe->ptCaretPos_PHY_After.HasNegative()) {
		TopErrorMessage(NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("Bug.\n")
			_T("pOpe->ptCaretPos_PHY_Before = %d,%d\n")
			_T("pOpe->ptCaretPos_PHY_After = %d,%d\n"),
			pOpe->ptCaretPos_PHY_Before.x,
			pOpe->ptCaretPos_PHY_Before.y,
			pOpe->ptCaretPos_PHY_After.x,
			pOpe->ptCaretPos_PHY_After.y
		);
	}

	// 配列のメモリサイズを調整
	ppCOpeArr.push_back(pOpe);
	return true;
}


// 操作を返す
Ope* OpeBlk::GetOpe(int nIndex)
{
	if (GetNum() <= nIndex) {
		return nullptr;
	}
	return ppCOpeArr[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 編集操作要素ブロックのダンプ
void OpeBlk::DUMP(void)
{
#ifdef _DEBUG
	int size = GetNum();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("\tCOpeBlk.ppCOpeArr[%d]----\n"), i);
		ppCOpeArr[i]->DUMP();
	}
#endif
}

