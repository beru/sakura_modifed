#include "StdAfx.h"
#include "types/CType.h"

void CType_Other::InitTypeConfigImp(STypeConfig* pType)
{
	// –¼‘O‚ÆŠg’£Žq
	auto_sprintf_s(pType->m_szTypeName, _T("Ý’è%d"), pType->m_nIdx + 1);
	pType->m_szTypeExts[0] = 0;

}

