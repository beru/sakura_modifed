#include "StdAfx.h"
#include "Cesu8.h"
#include "Eol.h"

// BOM�f�[�^�擾
void Cesu8::GetBom(Memory* pMemBom)
{
	static const BYTE UTF8_BOM[] = {0xEF, 0xBB, 0xBF};
	pMemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}

