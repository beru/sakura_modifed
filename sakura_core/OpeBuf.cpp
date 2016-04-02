/*!	@file
	@brief Undo, Redoバッファ

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "OpeBuf.h"
#include "OpeBlk.h"// 2002/2/10 aroka


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// OpeBufクラス構築
OpeBuf::OpeBuf()
{
	m_nCurrentPointer = 0;	// 現在位置
	m_nNoModifiedIndex = 0;	// 無変更な状態になった位置
}

// OpeBufクラス消滅
OpeBuf::~OpeBuf()
{
	// 操作ブロックの配列を削除する
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           状態                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Undo可能な状態か
bool OpeBuf::IsEnableUndo() const
{
	return 0 < m_vCOpeBlkArr.size() && 0 < m_nCurrentPointer;
}

// Redo可能な状態か
bool OpeBuf::IsEnableRedo() const
{
	return 0 < m_vCOpeBlkArr.size() && m_nCurrentPointer < (int)m_vCOpeBlkArr.size();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 操作の追加
bool OpeBuf::AppendOpeBlk(OpeBlk* pOpeBlk)
{
	// 現在位置より後ろ（Undo対象）がある場合は、消去
	int size = (int)m_vCOpeBlkArr.size();
	if (m_nCurrentPointer < size) {
		for (int i=m_nCurrentPointer; i<size; ++i) {
			SAFE_DELETE(m_vCOpeBlkArr[i]);
		}
		m_vCOpeBlkArr.resize(m_nCurrentPointer);
	}
	// 配列のメモリサイズを調整
	m_vCOpeBlkArr.push_back(pOpeBlk);
	++m_nCurrentPointer;
	return true;
}

// 全要素のクリア
void OpeBuf::ClearAll()
{
	// 操作ブロックの配列を削除する
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
	m_nCurrentPointer = 0;	// 現在位置
	m_nNoModifiedIndex = 0;	// 無変更な状態になった位置
}

// 現在位置で無変更な状態になったことを通知
void OpeBuf::SetNoModified()
{
	m_nNoModifiedIndex = m_nCurrentPointer;	// 無変更な状態になった位置
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
	--m_nCurrentPointer;
	if (m_nCurrentPointer == m_nNoModifiedIndex) {		// 無変更な状態になった位置
		*pbModified = false;
	}else {
		*pbModified = true;
	}
	return m_vCOpeBlkArr[m_nCurrentPointer];
}

// 現在のRedo対象の操作ブロックを返す
OpeBlk* OpeBuf::DoRedo(bool* pbModified)
{
	// Redo可能な状態か
	if (!IsEnableRedo()) {
		return nullptr;
	}
	OpeBlk* pOpeBlk = m_vCOpeBlkArr[m_nCurrentPointer];
	++m_nCurrentPointer;
	if (m_nCurrentPointer == m_nNoModifiedIndex) {		// 無変更な状態になった位置
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
	MYTRACE(_T("OpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer);
	int size = (int)m_vCOpeBlkArr.size();
	for (int i=0; i<size; ++i) {
		MYTRACE(_T("OpeBuf.m_vCOpeBlkArr[%d]----\n"), i);
		m_vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE(_T("OpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer);
#endif
}

