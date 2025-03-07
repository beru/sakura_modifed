#include "StdAfx.h"
#include "charset/CodeMediator.h"
#include "charset/charcode.h"
#include "charset/ESI.h"
#include "io/BinaryStream.h"
#include "types/Type.h"


/*!
	文字列の先頭にUnicode系BOMが付いているか？

	@retval CODE_UNICODE   UTF-16 LE
	@retval CODE_UTF8      UTF-8
	@retval CODE_UNICODEBE UTF-16 BE
	@retval CODE_NONE      未検出
*/
EncodingType CodeMediator::DetectUnicodeBom(const char* pS, size_t nLen)
{
	uchar_t* pBuf;

	if (!pS) { return CODE_NONE; }

	pBuf = (uchar_t*) pS;
	if (2 <= nLen) {
		if (pBuf[0] == 0xff && pBuf[1] == 0xfe) {
			return CODE_UNICODE;
		}
		if (pBuf[0] == 0xfe && pBuf[1] == 0xff) {
			return CODE_UNICODEBE;
		}
		if (3 <= nLen) {
			if (pBuf[0] == 0xef && pBuf[1] == 0xbb && pBuf[2] == 0xbf) {
				return CODE_UTF8;
			}
		}
	}
	if (4 <= nLen) {
		if (memcmp(pBuf, "+/v", 3) == 0
			&& (pBuf[3] == '8' || pBuf[3] == '9' || pBuf[3] == '+' || pBuf[3] == '/')
		) {
			return CODE_UTF7;
		}
	}
	return CODE_NONE;
}


/*!
	SJIS, JIS, EUCJP, UTF-8, UTF-7 を判定 (改)
	@return SJIS, JIS, EUCJP, UTF-8, UTF-7 の何れかの ID を返す．
	@note 適切な検出が行われた場合は、dwStatus に CESI_MB_DETECTED フラグが格納される。
*/
EncodingType CodeMediator::DetectMBCode(ESI* pEsi)
{
//	pEsi->dwStatus = ESI_NOINFORMATION;
	if ((int)pEsi->GetDataLen() < (pEsi->apMbcInfo[0]->nSpecific - pEsi->apMbcInfo[0]->nPoints) * 2000) {
		// 不正バイトの割合が、全体の 0.05% 未満であることを確認。
		// 全体の0.05%ほどの不正バイトは、無視する。
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}
	if (pEsi->apMbcInfo[0]->nPoints <= 0) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	// 判定状況を確認
	pEsi->SetStatus(ESI_MBC_DETECTED);
	return pEsi->apMbcInfo[0]->eCodeID;
}


/*!
	UTF-16 LE/BE を判定.

	@retval CODE_UNICODE    UTF-16 LE が検出された
	@retval CODE_UNICODEBE  UTF-16 BE が検出された
	@retval 0               UTF-16 LE/BE ともに検出されなかった
*/
EncodingType CodeMediator::DetectUnicode(ESI* pEsi)
{
//	pEsi->dwStatus = ESI_NOINFORMATION;

	BOMType bomType = pEsi->GetBOMType();
	if (bomType == BOMType::Unknown) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	// 1行の平均桁数が200を超えている場合はUnicode未検出とする
	size_t nDataLen = pEsi->GetDataLen();
	int nLineBreak = pEsi->aWcInfo[(int)bomType].nSpecific;  // 改行数を nLineBreakに取得
	if (static_cast<double>(nDataLen) / nLineBreak > 200) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	pEsi->SetStatus(ESI_WC_DETECTED);
	return pEsi->aWcInfo[(int)bomType].eCodeID;
}


/*
	日本語コードセット判定
*/
EncodingType CodeMediator::CheckKanjiCode(ESI* pEsi)
{
	/*
		判定状況は、
		DetectMBCode(), DetectUnicode() 内で
		esi.dwStatus に記録する。
	*/

	if (!pEsi) {
		return CODE_DEFAULT;
	}
	if (pEsi->GetMetaName() != CODE_NONE) {
		return pEsi->GetMetaName();
	}
	EncodingType nret;
	nret = DetectUnicode( pEsi );
	if (nret != CODE_NONE && pEsi->GetStatus() != ESI_NODETECTED) {
		return nret;
	}
	nret = DetectMBCode(pEsi);
	if (nret != CODE_NONE && pEsi->GetStatus() != ESI_NODETECTED) {
		return nret;
	}

	// デフォルト文字コードを返す
	return pEsi->pEncodingConfig->eDefaultCodetype;
}


/*
	日本語コードセット判別

	戻り値
	SJIS		CODE_SJIS
	JIS			CODE_JIS
	EUC			CODE_EUC
	Unicode		CODE_UNICODE
	UTF-8		CODE_UTF8
	UTF-7		CODE_UTF7
	UnicodeBE	CODE_UNICODEBE
*/
EncodingType CodeMediator::CheckKanjiCode(const char* pBuf, size_t nBufLen)
{
	ESI esi(*pEncodingConfig);

	/*
		判定状況は、
		DetectMBCode(), DetectUnicode() 内で
		esi.dwStatus に記録する。
	*/

	esi.SetInformation(pBuf, nBufLen/*, CODE_SJIS*/);
	return CheckKanjiCode(&esi);
}



/*
|| ファイルの日本語コードセット判別
||
|| 【戻り値】
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
||	エラー		CODE_ERROR
*/
EncodingType CodeMediator::CheckKanjiCodeOfFile(const TCHAR* pszFile)
{
	// オープン
	BinaryInputStream in(pszFile);
	if (!in) {
		return CODE_ERROR;
	}

	// データ長取得
	size_t nBufLen = in.GetLength();
	if (nBufLen > CheckKanjiCode_MAXREADLENGTH) {
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}

	// 0バイトならタイプ別のデフォルト設定
	if (nBufLen == 0) {
		return pEncodingConfig->eDefaultCodetype;
	}

	// データ確保
	Memory mem;
	mem.AllocBuffer(nBufLen);
	void* pBuf = mem.GetRawPtr();

	// 読み込み
	nBufLen = in.Read(pBuf, nBufLen);

	// クローズ
	in.Close();

	// 日本語コードセット判別
	EncodingType nCodeType = DetectUnicodeBom(reinterpret_cast<const char*>(pBuf), nBufLen);
	if (nCodeType == CODE_NONE) {
		// Unicode BOM は検出されませんでした．
		nCodeType = CheckKanjiCode(reinterpret_cast<const char*>(pBuf), nBufLen);
	}

	return nCodeType;
}

