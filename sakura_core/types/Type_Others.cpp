#include "StdAfx.h"
#include "types/Type.h"

void CType_Other::InitTypeConfigImp(TypeConfig& type)
{
	// ñºëOÇ∆ägí£éq
	auto_sprintf_s(type.szTypeName, _T("ê›íË%d"), type.nIdx + 1);
	type.szTypeExts[0] = 0;

}

