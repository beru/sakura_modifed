#include "StdAfx.h"
#include "types/Type.h"

void CType_Other::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	auto_sprintf_s(type.szTypeName, _T("�ݒ�%d"), type.nIdx + 1);
	type.szTypeExts[0] = 0;

}

