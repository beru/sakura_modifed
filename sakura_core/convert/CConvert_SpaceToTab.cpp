#include "StdAfx.h"
#include "CConvert_SpaceToTab.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

// 空白→TAB変換。単独のスペースは変換しない
bool Converter_SpaceToTab::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	int		nLineLen;
	int		nBgn;
	int		i;
	int		nPosDes;
	int		nPosX;
	Eol	cEol;

	bool	bSpace = false;	// スペースの処理中かどうか
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	// 変換後に必要なバイト数を調べる
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			nPosDes += nLineLen;
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
	// CRLFで区切られる「行」を返す。CRLFは行長に加えない
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
			nPosX = (pData->GetStringPtr() == pLine)? m_nStartColumn: 0;	// 処理中のiに対応する表示桁位置
			bSpace = false;	// 直前がスペースか
			nStartPos = 0;	// スペースの先頭
			for (i=0; i<nLineLen; ++i) {
				if (SPACE == pLine[i] || TAB == pLine[i]) {
					if (!bSpace) {
						nStartPos = nPosX;
					}
					bSpace = true;
					if (SPACE == pLine[i]) {
						++nPosX;
					}else if (TAB == pLine[i]) {
						nPosX += m_nTabWidth - (nPosX % m_nTabWidth);
					}
				}else {
					if (bSpace) {
						if ((nPosX - nStartPos == 1) && (pLine[i - 1] == SPACE)) {
							pDes[nPosDes] = SPACE;
							++nPosDes;
						}else {
							for (j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); ++j) {
								pDes[nPosDes] = TAB;
								++nPosDes;
								nStartPos += m_nTabWidth - (nStartPos % m_nTabWidth);
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
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// 全角文字ずれ対応 2008.10.17 matsumo
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
					//for (j = nStartPos - 1; (j + m_nTabWidth) <= nPosX + 1; j += m_nTabWidth) {
					for (j=nStartPos/m_nTabWidth; j<(nPosX/m_nTabWidth); ++j) {
						pDes[nPosDes] = TAB;
						++nPosDes;
						nStartPos += m_nTabWidth - (nStartPos % m_nTabWidth);
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
		auto_memcpy(&pDes[nPosDes], cEol.GetValue2(), cEol.GetLen());
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

