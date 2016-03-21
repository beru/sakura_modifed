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

#include "StdAfx.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

// 設定ファイル
// Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
void CType_Ini::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("設定ファイル"));
	_tcscpy(type.szTypeExts, _T("ini,inf,cnf,kwd,col"));
	
	// 設定
	type.lineComment.CopyTo(0, L"//", -1);				// 行コメントデリミタ
	type.lineComment.CopyTo(1, L";", -1);					// 行コメントデリミタ2
	type.eDefaultOutline = OutlineType::Text;				// アウトライン解析方法
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// シングルクォーテーション文字列を色分け表示しない
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない
}

