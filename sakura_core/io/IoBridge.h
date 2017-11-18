#pragma once

#include "mem/Memory.h"
#include "charset/CodeBase.h"

class IoBridge {
public:
	// 内部実装のエンコードへ変換
	static CodeConvertResult FileToImpl(
		const Memory&	src,			// [in]  変換元メモリ
		NativeW*		pDst,			// [out] 変換先メモリ(UNICODE)
		CodeBase*		pCodeBase,		// [in]  変換元メモリの文字コードクラス
		int				nFlag			// [in]  bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);

	// ファイルのエンコードへ変更
	static CodeConvertResult ImplToFile(
		const NativeW&		src,		// [in]  変換元メモリ(UNICODE)
		Memory*				pDst,		// [out] 変換先メモリ
		CodeBase*			pCodeBase	// [in]  変換先メモリの文字コードクラス
	);
};

