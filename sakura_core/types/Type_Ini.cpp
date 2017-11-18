#include "StdAfx.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

// 設定ファイル
// Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
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

