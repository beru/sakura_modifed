#include "StdAfx.h"
#include "types/Type.h"

void CType_Other::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	auto_sprintf_s(type.szTypeName, _T("設定%d"), type.nIdx + 1);
	type.szTypeExts[0] = 0;

}

