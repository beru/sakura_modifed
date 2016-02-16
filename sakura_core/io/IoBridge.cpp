#include "StdAfx.h"
#include "io/IoBridge.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "Eol.h"

// 内部実装のエンコードへ変換
CodeConvertResult IoBridge::FileToImpl(
	const Memory&	src,		// [in]  変換元メモリ
	NativeW*		pDst,		// [out] 変換先メモリ(UNICODE)
	CodeBase*		pCode,		// [in]  変換元メモリの文字コード
	int				nFlag		// [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	)
{
	// 任意の文字コードからUnicodeへ変換する
	CodeConvertResult ret = pCode->CodeToUnicode(src, pDst);

	// 結果
	return ret;
}

CodeConvertResult IoBridge::ImplToFile(
	const NativeW&	src,		// [in]  変換元メモリ(UNICODE)
	Memory*			pDst,		// [out] 変換先メモリ
	CodeBase*		pCode		// [in]  変換先メモリの文字コード
	)
{
	// Unicodeから任意の文字コードへ変換する
	CodeConvertResult ret = pCode->UnicodeToCode(src, pDst);

	// 結果
	return ret;
}


