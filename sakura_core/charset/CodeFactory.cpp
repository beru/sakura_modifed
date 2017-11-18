#include "StdAfx.h"
#include "CodeFactory.h"
#include "CodeMediator.h"
#include "CodePage.h"

#include "Euc.h"
#include "Jis.h"
#include "ShiftJis.h"
#include "Unicode.h"
#include "UnicodeBe.h"
#include "Utf7.h"
#include "Utf8.h"
#include "Cesu8.h"
#include "Latin1.h"

// codeTypeに適合する CodeBaseインスタンス を生成
CodeBase* CodeFactory::CreateCodeBase(
	EncodingType	codeType,		// 文字コード
	int			nFlag			// bit 0: MIME Encodeされたヘッダをdecodeするかどうか
)
{
  	switch (codeType) {
	case CODE_SJIS:			return new ShiftJis();
	case CODE_EUC:			return new Euc();
	case CODE_JIS:			return new Jis((nFlag&1) == 1);
	case CODE_UNICODE:		return new Unicode();
	case CODE_UTF8:			return new Utf8();
	case CODE_UTF7:			return new Utf7();
	case CODE_UNICODEBE:	return new UnicodeBe();
	case CODE_CESU8:		return new Cesu8();
	case CODE_LATIN1:		return new Latin1();	// 2010/3/20 Uchi
	case CODE_CPACP:		return new CodePage(codeType);
	case CODE_CPOEM:		return new CodePage(codeType);
	default:
		if (IsValidCodePageEx(codeType)) {
			return new CodePage(codeType);
		}
		assert_warning(0);
	}
	return NULL;
}

