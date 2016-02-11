/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class FuncInfo;

#pragma once

#include "mem/CMemory.h"

// CDlgFuncList::SetTree()用 m_Info
#define FUNCINFO_INFOMASK	0xFFFF
//	2003.06.27 Moca
#define FUNCINFO_NOCLIPTEXT 0x10000

//! アウトライン解析  データ要素
//@date 2002.04.01 YAZAKI 深さ導入
class FuncInfo {
public:
	FuncInfo(LogicInt, LogicInt, LayoutInt, LayoutInt, const TCHAR*, const TCHAR*, int);	// FuncInfoクラス構築
	~FuncInfo();	// FuncInfoクラス消滅

	//! クリップボードに追加する要素か？
	//	2003.06.27 Moca
	inline bool IsAddClipText(void) const {
		return (FUNCINFO_NOCLIPTEXT != (m_nInfo & FUNCINFO_NOCLIPTEXT));
	}

//	private:
	LogicInt	m_nFuncLineCRLF;	// 関数のある行(CRLF単位)
	LayoutInt	m_nFuncLineLAYOUT;	// 関数のある行(折り返し単位)
	LogicInt	m_nFuncColCRLF;		// 関数のある桁(CRLF単位)
	LayoutInt	m_nFuncColLAYOUT;	// 関数のある桁(折り返し単位)
	CNativeT	m_memFuncName;		// 関数名
	CNativeT	m_memFileName;		// ファイル名
	int			m_nInfo;			// 付加情報
	int			m_nDepth;			// 深さ
};

