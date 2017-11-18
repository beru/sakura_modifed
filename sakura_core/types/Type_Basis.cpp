#include "StdAfx.h"
#include "types/Type.h"
#include "doc/DocOutline.h"
#include "view/colors/EColorIndexType.h"

void CType_Basis::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("基本"));
	_tcscpy(type.szTypeExts, _T(""));

	// 設定
	type.nMaxLineKetas = MAXLINEKETAS;			// 折り返し桁数
	type.eDefaultOutline = OutlineType::Text;				// アウトライン解析方法
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// シングルクォーテーション文字列を色分け表示しない	// Oct. 17, 2000 JEPRO
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない	// Sept. 4, 2000 JEPRO
}

