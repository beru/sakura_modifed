#include "StdAfx.h"
#include "CCodeFactory.h"
#include "CCodeMediator.h"
#include "CCodePage.h"

// move start	from CCodeMediator.h	2012/12/02 Uchi
#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
#include "CCesu8.h"
// move end
#include "CLatin1.h"

// eCodeTypeに適合する CodeBaseインスタンス を生成
CodeBase* CodeFactory::CreateCodeBase(
	ECodeType	eCodeType,		// 文字コード
	int			nFlag			// bit 0: MIME Encodeされたヘッダをdecodeするかどうか
)
{
  	switch (eCodeType) {
	case CODE_SJIS:			return new ShiftJis();
	case CODE_EUC:			return new Euc();
	case CODE_JIS:			return new Jis((nFlag&1) == 1);
	case CODE_UNICODE:		return new Unicode();
	case CODE_UTF8:			return new Utf8();
	case CODE_UTF7:			return new Utf7();
	case CODE_UNICODEBE:	return new UnicodeBe();
	case CODE_CESU8:		return new Cesu8();
	case CODE_LATIN1:		return new Latin1();	// 2010/3/20 Uchi
	case CODE_CPACP:		return new CodePage(eCodeType);
	case CODE_CPOEM:		return new CodePage(eCodeType);
	default:
		if (IsValidCodePageEx(eCodeType)) {
			return new CodePage(eCodeType);
		}
		assert_warning(0);
	}
	return NULL;
}

