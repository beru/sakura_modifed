#pragma once

class OpeBuf;

// Undo, Redoバッファ

#include <vector>
#include "_main/global.h"
class OpeBlk;/// 2002/2/10 aroka


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief Undo, Redoバッファ
*/
class OpeBuf {
public:
	// コンストラクタ・デストラクタ
	OpeBuf();
	~OpeBuf();

	// 状態
	bool IsEnableUndo() const;					// Undo可能な状態か
	bool IsEnableRedo() const;					// Redo可能な状態か
	int GetCurrentPointer(void) const { return nCurrentPointer; }	// 現在位置を返す	// 2007.12.09 ryoji
	int GetNextSeq() const { return nCurrentPointer + 1; }
	int GetNoModifiedSeq() const { return nNoModifiedIndex; }

	// 操作
	void ClearAll();							// 全要素のクリア
	bool AppendOpeBlk(OpeBlk* pOpeBlk);			// 操作ブロックの追加
	void SetNoModified();						// 現在位置で無変更な状態になったことを通知

	// 使用
	OpeBlk* DoUndo(bool* pbModified);			// 現在のUndo対象の操作ブロックを返す
	OpeBlk* DoRedo(bool* pbModified);			// 現在のRedo対象の操作ブロックを返す

	// デバッグ
	void DUMP();								// 編集操作要素ブロックのダンプ

private:
	std::vector<OpeBlk*>	vCOpeBlkArr;		// 操作ブロックの配列
	int						nCurrentPointer;	// 現在位置
	int						nNoModifiedIndex;	// 無変更な状態になった位置
};


