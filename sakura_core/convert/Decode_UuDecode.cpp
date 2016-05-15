// この行は文字化け対策のものです。
#include "StdAfx.h"
#include "Decode_UuDecode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "util/string_ex2.h"
#include "Eol.h"

// Uudecode (デコード）
bool Decode_UuDecode::DoDecode(const NativeW& pSrc, Memory* pDst)
{
	const WCHAR *psrc, *pline;
	int nsrclen;
	char *pw, *pw_base;
	int nlinelen, ncuridx;
	Eol eol;
	bool bsuccess = false;

	pDst->Clear();
	psrc = pSrc.GetStringPtr();
	nsrclen = pSrc.GetStringLength();

	if (nsrclen < 1) {
		pDst->_AppendSz("");
		return false;
	}
	pDst->AllocBuffer((nsrclen / 4) * 3 + 10);
	pw_base = pw = static_cast<char*>(pDst->GetRawPtr());

	// 先頭の改行・空白文字をスキップ
	for (ncuridx=0; ncuridx<nsrclen; ++ncuridx) {
		WCHAR c = psrc[ncuridx];
		if (!WCODE::IsLineDelimiterBasic(c) && c != L' ' && c != L'\t') {
			break;
		}
	}

	// ヘッダーを解析
	pline = GetNextLineW( psrc, nsrclen, &nlinelen, &ncuridx, &eol, false );
	if (!CheckUUHeader(pline, nlinelen, aFilename)) {
		pDst->_AppendSz("");
		return false;
	}

	// ボディーを処理
	while ((pline = GetNextLineW(psrc, nsrclen, &nlinelen, &ncuridx, &eol, false))) {
		if (eol.GetType() != EolType::CRLF) {
			pDst->_AppendSz("");
			return false;
		}
		if (nlinelen < 1) {
			pDst->_AppendSz("");
			return false;
		}
		if (nlinelen == 1) {
			// データの最後である場合
			if (pline[0] == L' ' || pline[0] == L'`' || pline[0] == L'~') {
				bsuccess = true;
				break;
			}
		}
		pw += _DecodeUU_line(pline, nlinelen, pw);
	}
	if (!bsuccess) {
		return false;
	}

	pline += 3;  // '`' 'CR' 'LF' の分をスキップ

	// フッターを解析
	if (!CheckUUFooter(pline, nsrclen - ncuridx)) {
		pDst->_AppendSz("");
		return false;
	}

	pDst->_SetRawLength(pw - pw_base);
	return true;
}

