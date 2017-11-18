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
	size_t size = ppCOpeArr.size();
	for (size_t i=0; i<size; ++i) {
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
Ope* OpeBlk::GetOpe(size_t nIndex)
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
	size_t size = GetNum();
	for (size_t i=0; i<size; ++i) {
		MYTRACE(_T("\tCOpeBlk.ppCOpeArr[%d]----\n"), i);
		ppCOpeArr[i]->DUMP();
	}
#endif
}

