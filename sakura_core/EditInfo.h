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

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "types/Type.h"


/*! ファイル情報

	@date 2002.03.07 genta szDocType追加
	@date 2003.01.26 aroka nWindowSizeX/Y nWindowOriginX/Y追加
*/
struct EditInfo {
	// ファイル
	TCHAR			szPath[_MAX_PATH];					// ファイル名
	EncodingType	nCharCode;							// 文字コード種別
	bool			bBom;								// BOM(GetFileInfo)
	TCHAR			szDocType[MAX_DOCTYPE_LEN + 1];		// 文書タイプ
	int 			nTypeId;							// 文書タイプ(MRU)

	// 表示域
	int nViewTopLine;							// 表示域の一番上の行(0開始)
	int nViewLeftCol;							// 表示域の一番左の桁(0開始)

	// キャレット
	LogicPoint ptCursor;								// キャレット位置

	// 各種状態
	bool		bIsModified;							// 変更フラグ

	// GREPモード
	bool		bIsGrep;								// Grepのウィンドウか
	wchar_t		szGrepKey[1024];

	// デバッグモニタ (アウトプットウィンドウ) モード
	bool		bIsDebug;								// デバッグモニタモード (アウトプットウィンドウ) か

	// ブックマーク情報
	wchar_t		szMarkLines[MAX_MARKLINES_LEN + 1];		// ブックマークの物理行リスト

	// ウィンドウ
	int			nWindowSizeX;							// ウィンドウ  幅(ピクセル数)
	int			nWindowSizeY;							// ウィンドウ  高さ(ピクセル数)
	int			nWindowOriginX;							// ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
	int			nWindowOriginY;							// ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
	
	// Mar. 7, 2002 genta
	// Constructor 確実に初期化するため
	EditInfo()
		:
		nCharCode(CODE_AUTODETECT),
		bBom(false),
		nTypeId(-1),
		nViewTopLine(-1),
		nViewLeftCol(-1),
		ptCursor(-1, -1),
		bIsModified(false),
		bIsGrep(false),
		bIsDebug(false),
		nWindowSizeX(-1),
		nWindowSizeY(-1),
		nWindowOriginX(CW_USEDEFAULT),	//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
		nWindowOriginY(CW_USEDEFAULT)
	{
		szPath[0] = '\0';
		szMarkLines[0] = L'\0';
		szDocType[0] = '\0';
	}
};

