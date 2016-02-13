#include "StdAfx.h"
#include "CConvert_TabToSpace.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

// TAB����
bool Converter_TabToSpace::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	int	nLineLen;
	int	nBgn;
	int	nPosDes;
	int	nPosX;
	int	nWork;
	Eol cEol;
	nBgn = 0;
	nPosDes = 0;
	// CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			// �擪�s�ɂ��Ă͊J�n���ʒu���l������i����ɐ܂�Ԃ��֘A�̑΍􂪕K�v�H�j
			nPosX = (pData->GetStringPtr() == pLine)? m_nStartColumn: 0;
			for (int i=0; i<nLineLen; ++i) {
				if (TAB == pLine[i]) {
					nWork = m_nTabWidth - (nPosX % m_nTabWidth);
					nPosDes += nWork;
					nPosX += nWork;
				}else {
					++nPosDes;
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// �S�p��������Ή� 2008.10.15 matsumo
				}
			}
		}
		nPosDes += cEol.GetLen();
	}
	if (0 >= nPosDes) {
		return false;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			// �擪�s�ɂ��Ă͊J�n���ʒu���l������i����ɐ܂�Ԃ��֘A�̑΍􂪕K�v�H�j
			nPosX = (pData->GetStringPtr() == pLine)? m_nStartColumn: 0;
			for (int i=0; i<nLineLen; ++i) {
				if (TAB == pLine[i]) {
					nWork = m_nTabWidth - (nPosX % m_nTabWidth);
					auto_memset(&pDes[nPosDes], L' ', nWork);
					nPosDes += nWork;
					nPosX += nWork;
				}else {
					pDes[nPosDes] = pLine[i];
					++nPosDes;
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// �S�p��������Ή� 2008.10.15 matsumo
				}
			}
		}
		auto_memcpy(&pDes[nPosDes], cEol.GetValue2(), cEol.GetLen());
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

