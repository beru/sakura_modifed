#include "StdAfx.h"
#include "Convert_TabToSpace.h"
#include "charset/charcode.h"
#include "Eol.h"
#include "util/string_ex2.h"

// TAB→空白
bool Converter_TabToSpace::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	int	nLineLen;
	int	nBgn;
	int	nPosDes;
	int	nPosX;
	int	nWork;
	Eol eol;
	nBgn = 0;
	nPosDes = 0;
	// CRLFで区切られる「行」を返す。CRLFは行長に加えない
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
			nPosX = (pData->GetStringPtr() == pLine)? nStartColumn: 0;
			for (int i=0; i<nLineLen; ++i) {
				if (pLine[i] == TAB) {
					nWork = nTabWidth - (nPosX % nTabWidth);
					nPosDes += nWork;
					nPosX += nWork;
				}else {
					++nPosDes;
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// 全角文字ずれ対応 2008.10.15 matsumo
				}
			}
		}
		nPosDes += eol.GetLen();
	}
	if (0 >= nPosDes) {
		return false;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// CRLFで区切られる「行」を返す。CRLFは行長に加えない
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
			nPosX = (pData->GetStringPtr() == pLine)? nStartColumn: 0;
			for (int i=0; i<nLineLen; ++i) {
				if (pLine[i] == TAB) {
					nWork = nTabWidth - (nPosX % nTabWidth);
					auto_memset(&pDes[nPosDes], L' ', nWork);
					nPosDes += nWork;
					nPosX += nWork;
				}else {
					pDes[nPosDes] = pLine[i];
					++nPosDes;
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// 全角文字ずれ対応 2008.10.15 matsumo
				}
			}
		}
		auto_memcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
		nPosDes += eol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

