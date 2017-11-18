/*!	@file
	@brief アウトライン解析  データ要素
*/

class FuncInfo;

#pragma once

#include "mem/Memory.h"

// CDlgFuncList::SetTree()用 Info
#define FUNCINFO_INFOMASK	0xFFFF
#define FUNCINFO_NOCLIPTEXT 0x10000

// アウトライン解析  データ要素
class FuncInfo {
public:
	FuncInfo(size_t, size_t, size_t, size_t, const TCHAR*, const TCHAR*, int);	// FuncInfoクラス構築
	~FuncInfo();	// FuncInfoクラス消滅

	// クリップボードに追加する要素か？
	inline bool IsAddClipText(void) const {
		return (FUNCINFO_NOCLIPTEXT != (nInfo & FUNCINFO_NOCLIPTEXT));
	}

//	private:
	size_t		nFuncLineCRLF;		// 関数のある行(CRLF単位)
	size_t		nFuncLineLAYOUT;	// 関数のある行(折り返し単位)
	size_t		nFuncColCRLF;		// 関数のある桁(CRLF単位)
	size_t		nFuncColLAYOUT;		// 関数のある桁(折り返し単位)
	NativeT		memFuncName;		// 関数名
	NativeT		memFileName;		// ファイル名
	int			nInfo;				// 付加情報
	size_t		nDepth;				// 深さ
};

