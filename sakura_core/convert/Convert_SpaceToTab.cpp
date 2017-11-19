#include "StdAfx.h"
#include "Convert_SpaceToTab.h"
#include "charset/charcode.h"
#include "Eol.h"
#include "util/string_ex2.h"

// 空白→TAB変換。単独のスペースは変換しない
bool Converter_SpaceToTab::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	size_t nLineLen;
	size_t nBgn;
	size_t nPosDes;
	int		nPosX;
	Eol	eol;

	bool	bSpace = false;	// スペースの処理中かどうか
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	// 変換後に必要なバイト数を調べる
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			nPosDes += nLineLen;
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
			nPosX = (pData->GetStringPtr() == pLine)? nStartColumn: 0;	// 処理中のiに対応する表示桁位置
			bSpace = false;	// 直前がスペースか
			nStartPos = 0;	// スペースの先頭
			size_t i;
			for (i=0; i<nLineLen; ++i) {
				if (pLine[i] == SPACE || pLine[i] == TAB) {
					if (!bSpace) {
						nStartPos = nPosX;
					}
					bSpace = true;
					if (pLine[i] == SPACE) {
						++nPosX;
					}else if (pLine[i] == TAB) {
						nPosX += nTabWidth - (nPosX % nTabWidth);
					}
				}else {
					if (bSpace) {
						if ((nPosX - nStartPos == 1) && (pLine[i - 1] == SPACE)) {
							pDes[nPosDes] = SPACE;
							++nPosDes;
						}else {
							for (j = nStartPos / nTabWidth; j < (nPosX / nTabWidth); ++j) {
								pDes[nPosDes] = TAB;
								++nPosDes;
								nStartPos += nTabWidth - (nStartPos % nTabWidth);
							}
							// 変換後にTABが1つも入らない場合にスペースを詰めすぎて
							// バッファをはみ出すのを修正
							for (j=nStartPos; j<nPosX; ++j) {
								pDes[nPosDes] = SPACE;
								++nPosDes;
							}
						}
					}
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// 全角文字ずれ対応
					pDes[nPosDes] = pLine[i];
					++nPosDes;
					bSpace = false;
				}
			}
			//for (; i<nLineLen; ++i) {
			//	pDes[nPosDes] = pLine[i];
			//	++nPosDes;
			//}
			if (bSpace) {
				if ((nPosX-nStartPos == 1) && (pLine[i - 1] == SPACE)) {
					pDes[nPosDes] = SPACE;
					++nPosDes;
				}else {
					//for (j = nStartPos - 1; (j + nTabWidth) <= nPosX + 1; j += nTabWidth) {
					for (j=nStartPos/nTabWidth; j<(nPosX/nTabWidth); ++j) {
						pDes[nPosDes] = TAB;
						++nPosDes;
						nStartPos += nTabWidth - (nStartPos % nTabWidth);
					}
					// 2003.08.05 Moca
					// 変換後にTABが1つも入らない場合にスペースを詰めすぎて
					// バッファをはみ出すのを修正
					for (j=nStartPos; j<nPosX; ++j) {
						pDes[nPosDes] = SPACE;
						++nPosDes;
					}
				}
			}
		}

		// 行末の処理
		auto_memcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
		nPosDes += eol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

