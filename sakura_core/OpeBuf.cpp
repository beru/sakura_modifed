#include "StdAfx.h"
#include "OpeBuf.h"
#include "OpeBlk.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// OpeBufクラス構築
OpeBuf::OpeBuf()
{
	nCurrentPointer = 0;	// 現在位置
	nNoModifiedIndex = 0;	// 無変更な状態になった位置
}

// OpeBufクラス消滅
OpeBuf::~OpeBuf()
{
	// 操作ブロックの配列を削除する
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vCOpeBlkArr[i]);
	}
	vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           状態                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo可能な状態か
bool OpeBuf::IsEnableUndo() const
{
	return 0 < vCOpeBlkArr.size() && 0 < nCurrentPointer;
}

// Redo可能な状態か
bool OpeBuf::IsEnableRedo() const
{
	return 0 < vCOpeBlkArr.size() && nCurrentPointer < (int)vCOpeBlkArr.size();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 操作の追加
bool OpeBuf::AppendOpeBlk(OpeBlk* pOpeBlk)
{
	// 現在位置より後ろ（Undo対象）がある場合は、消去
	int size = (int)vCOpeBlkArr.size();
	if (nCurrentPointer < size) {
		for (int i=nCurrentPointer; i<size; ++i) {
			SAFE_DELETE(vCOpeBlkArr[i]);
		}
		vCOpeBlkArr.resize(nCurrentPointer);
	}
	// 配列のメモリサイズを調整
	vCOpeBlkArr.push_back(pOpeBlk);
	++nCurrentPointer;
	return true;
}

// 全要素のクリア
void OpeBuf::ClearAll()
{
	// 操作ブロックの配列を削除する
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vCOpeBlkArr[i]);
	}
	vCOpeBlkArr.clear();
	nCurrentPointer = 0;	// 現在位置
	nNoModifiedIndex = 0;	// 無変更な状態になった位置
}

// 現在位置で無変更な状態になったことを通知
void OpeBuf::SetNoModified()
{
	nNoModifiedIndex = nCurrentPointer;	// 無変更な状態になった位置
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           使用                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 現在のUndo対象の操作ブロックを返す
OpeBlk* OpeBuf::DoUndo(bool* pbModified)
{
	// Undo可能な状態か
	if (!IsEnableUndo()) {
		return nullptr;
	}
	--nCurrentPointer;
	if (nCurrentPointer == nNoModifiedIndex) {		// 無変更な状態になった位置
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return vCOpeBlkArr[nCurrentPointer];
}

// 現在のRedo対象の操作ブロックを返す
OpeBlk* OpeBuf::DoRedo(bool* pbModified)
{
	// Redo可能な状態か
	if (!IsEnableRedo()) {
		return nullptr;
	}
	OpeBlk* pOpeBlk = vCOpeBlkArr[nCurrentPointer];
	++nCurrentPointer;
	if (nCurrentPointer == nNoModifiedIndex) {		// 無変更な状態になった位置
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return pOpeBlk;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo, Redoバッファのダンプ
void OpeBuf::DUMP()
{
#ifdef _DEBUG
	MYTRACE(_T("OpeBuf.nCurrentPointer=[%d]----\n"), nCurrentPointer);
	int size = (int)vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("OpeBuf.vCOpeBlkArr[%d]----\n"), i);
		vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE(_T("OpeBuf.nCurrentPointer=[%d]----\n"), nCurrentPointer);
#endif
}

