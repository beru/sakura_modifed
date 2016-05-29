/*
	Copyright (C) 2007, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include <vector>
#include "TextMetrics.h"
#include "charset/codecheck.h"

using namespace std;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

TextMetrics::TextMetrics()
{
	//$ 適当な仮値で初期化。実際には使う側でSet～を呼ぶので、これらの仮値が参照されることは無い。
	SetHankakuWidth(10);
	SetHankakuHeight(18);
	SetHankakuDx(12);
	SetHankakuDy(24);
}

TextMetrics::~TextMetrics()
{
}

void TextMetrics::CopyTextMetricsStatus(TextMetrics* pDst) const
{
	pDst->SetHankakuWidth(GetHankakuWidth());		// 半角文字の幅
	pDst->SetHankakuHeight(GetHankakuHeight());		// 文字の高さ
}

/*
	文字の大きさを調べる
	
	※ビルド種により、微妙にサイズが変わるようでした。
	　サイズを合わせるため、適当な文字で調整。
*/
void TextMetrics::Update(HFONT hFont)
{
	HDC hdc = GetDC(NULL);
	{
		HFONT hFontOld = (HFONT)::SelectObject(hdc, hFont);
		SIZE  sz;
#ifdef _UNICODE
		::GetTextExtentPoint32(hdc, L"xx", 2, &sz);
#else
		::GetTextExtentPoint32(hdc, LS(STR_ERR_DLGEDITVW2), 2, &sz);
#endif
		this->SetHankakuHeight(sz.cy);
		this->SetHankakuWidth(sz.cx / 2);
		::SelectObject(hdc, hFontOld);
	}
	ReleaseDC(NULL, hdc);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void TextMetrics::SetHankakuWidth(int nHankakuWidth)
{
	nCharWidth = nHankakuWidth;
}

// 半角文字の縦幅を設定。単位はピクセル。
void TextMetrics::SetHankakuHeight(int nHankakuHeight)
{
	nCharHeight = nHankakuHeight;
}


//文字間隔基準設定。nDxBasisは半角文字の基準ピクセル幅。SetHankakuDx
void TextMetrics::SetHankakuDx(int nDxBasis)
{
	this->nDxBasis = nDxBasis;
	for (size_t i=0; i<_countof(anHankakuDx); ++i) anHankakuDx[i] = GetHankakuDx();
	for (size_t i=0; i<_countof(anZenkakuDx); ++i) anZenkakuDx[i] = GetZenkakuDx();
}

void TextMetrics::SetHankakuDy(int nDyBasis)
{
	this->nDyBasis = nDyBasis;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           取得                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 指定した文字列により文字間隔配列を生成する。
const int* TextMetrics::GenerateDxArray(
	std::vector<int>* vResultArray, // [out] 文字間隔配列の受け取りコンテナ
	const wchar_t* pText,           // [in]  文字列
	size_t nLength,                    // [in]  文字列長
	int	nHankakuDx,					// [in]  半角文字の文字間隔
	int	nTabSpace,					// [in]  TAB幅
	int	nIndent						// [in]  インデント(TAB対応用)
	)
{
	bool bHigh;				// サロゲートペア（上位）

	vResultArray->resize(nLength);
	if (!pText || nLength <= 0) return NULL;

	int* p = &(*vResultArray)[0];
	int	 nLayoutCnt = nIndent;
	const wchar_t* q = pText;
	bHigh = false;
	for (size_t i=0; i<nLength; ++i, ++p, ++q) {
		if (*q == WCODE::TAB) {
			// TAB対応	2013/5/7 Uchi
			if (i > 0 && *(q-1) == WCODE::TAB) {
				*p = nTabSpace * nHankakuDx;
				nLayoutCnt += nTabSpace;
			}else {
				*p = (nTabSpace - nLayoutCnt % nTabSpace) * nHankakuDx;
				nLayoutCnt += (nTabSpace - nLayoutCnt % nTabSpace);
			}
			bHigh = false;
		// サロゲートチェック BMP 以外は全角扱い	2008/7/5 Uchi
		}else if (IsUTF16High(*q)) {
			*p = nHankakuDx*2;
			nLayoutCnt += 2;
			bHigh = true;
		}else if (IsUTF16Low(*q)) {
			// サロゲートペア（下位）単独の場合は全角扱い
			//*p = (bHigh) ? 0 : nHankakuDx*2;
			if (bHigh) {
				*p = 0;
			}else {
				if (IsBinaryOnSurrogate(*q)) {
					*p = nHankakuDx;
					++nLayoutCnt;
				}else {
					*p = nHankakuDx*2;
					nLayoutCnt += 2;
				}
			}
			bHigh = false;
		}else if (WCODE::IsHankaku(*q)) {
			*p = nHankakuDx;
			++nLayoutCnt;
			bHigh = false;				// サロゲートペア対策	2008/7/5 Uchi
		}else {
			*p = nHankakuDx*2;
			nLayoutCnt += 2;
			bHigh = false;				// サロゲートペア対策	2008/7/5 Uchi
		}
	}

	if (vResultArray->size()) {
		return &(*vResultArray)[0];
	}else {
		return nullptr;
	}
}

// 文字列のピクセル幅を返す。
size_t TextMetrics::CalcTextWidth(
	const wchar_t* pText,	// 文字列
	size_t nLength,			// 文字列長
	const int* pnDx			// 文字間隔の入った配列
	)
{
	// ANSI時代の動作 ※pnDxにはすべて同じ値が入っていた
	// return pnDx[0] * nLength;

	// UNICODE時代の動作
	int w = 0;
	for (size_t i=0; i<nLength; ++i) {
		w += pnDx[i];
	}
	return (size_t)w;
}

// 文字列のピクセル幅を返す。
size_t TextMetrics::CalcTextWidth2(
	const wchar_t* pText,	// 文字列
	size_t nLength,			// 文字列長
	int nHankakuDx		// 半角文字の文字間隔
	)
{
	// 文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray = TextMetrics::GenerateDxArray(
		&vDxArray,
		pText,
		nLength,
		nHankakuDx
	);

	// ピクセル幅を計算
	return CalcTextWidth(pText, nLength, pDxArray);
}

