#include "StdAfx.h"
#include "types/CType.h"

void CType_Other::InitTypeConfigImp(STypeConfig* pType)
{
	// ���O�Ɗg���q
	auto_sprintf_s(pType->m_szTypeName, _T("�ݒ�%d"), pType->m_nIdx + 1);
	pType->m_szTypeExts[0] = 0;

}

