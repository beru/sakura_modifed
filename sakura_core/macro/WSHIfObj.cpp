/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス
*/
#include "StdAfx.h"

#include <memory>

#include "macro/WSHIfObj.h"
#include "macro/SMacroMgr.h" // MacroFuncInfo
#include "Funccode_enum.h" // EFunctionCode::FA_FROMMACRO


// コマンド・関数を準備する
void WSHIfObj::ReadyMethods(
	EditView& view,
	int flags
	)
{
	this->pView = &view;
	// コマンドに混ぜ込むフラグを渡す
	ReadyCommands(GetMacroCommandInfo(), flags | FA_FROMMACRO);
	ReadyCommands(GetMacroFuncInfo(), 0);
	/* WSHIfObjを継承したサブクラスからReadyMethodsを呼び出した場合、
	 * サブクラスのGetMacroCommandInfo,GetMacroFuncInfoが呼び出される。 */
}

/** WSHマクロエンジンへコマンド登録を行う */
void WSHIfObj::ReadyCommands(
	MacroFuncInfo* Info,
	int flags
	)
{
	while (Info->nFuncID != -1) {
		wchar_t FuncName[256];
		wcscpy(FuncName, Info->pszFuncName);

		int ArgCount = 0;
		if (Info->pData) {
			ArgCount = Info->pData->nArgMinSize;
		}else {
			for (int i=0; i<4; ++i) {
				if (Info->varArguments[i] != VT_EMPTY) {
					++ArgCount;
				}
			}
		}
		VARTYPE* varArgTmp = nullptr;
		VARTYPE* varArg = Info->varArguments;
		if (4 < ArgCount) {
			varArgTmp = varArg = new VARTYPE[ArgCount];
			for (int i=0; i<ArgCount; ++i) {
				if (i < 4) {
					varArg[i] = Info->varArguments[i];
				}else {
					varArg[i] = Info->pData->pVarArgEx[i-4];
				}
			}
		}
		// flagを加えた値を登録する
		this->AddMethod(
			FuncName,
			(Info->nFuncID | flags),
			varArg,
			ArgCount,
			Info->varResult,
			reinterpret_cast<CIfObjMethod>(&WSHIfObj::MacroCommand)
			/* WSHIfObjを継承したサブクラスからReadyCommandsを呼び出した場合、
			 * サブクラスのMacroCommandが呼び出される。 */
		);
		delete[] varArgTmp;
		++Info;
	}
}

/*!
	マクロコマンドの実行
*/
HRESULT WSHIfObj::MacroCommand(
	int index,
	DISPPARAMS* arguments,
	VARIANT* result,
	void* data
	)
{
	int argCount = arguments->cArgs;

	const EFunctionCode id = static_cast<EFunctionCode>(index);
	// コマンドは下位16ビットのみ
	if (LOWORD(id) >= F_FUNCTION_FIRST) {
		VARIANT ret; // 戻り値の受け取りが無くても関数を実行する
		VariantInit(&ret);

		auto rgvargParam = std::make_unique<VARIANTARG[]>(argCount);
		for (int i=0; i<argCount; ++i) {
			::VariantInit(&rgvargParam[argCount - i - 1]);
			::VariantCopy(&rgvargParam[argCount - i - 1], &arguments->rgvarg[i]);
		}

		// HandleFunctionはサブクラスでオーバーライドする
		bool r = HandleFunction(*pView, id, &rgvargParam[0], argCount, ret);
		if (result) {::VariantCopyInd(result, &ret);}
		VariantClear(&ret);
		for (int i=0; i<argCount; ++i) {
			::VariantClear(&rgvargParam[i]);
		}
		return r ? S_OK : E_FAIL;
	}else {
		// 最低4つは確保
		int argCountMin = t_max(4, argCount);
		// 引数を文字列で取得する
		auto strArgs = std::make_unique<LPWSTR[]>(argCountMin);
		auto strLengths = std::make_unique<int[]>(argCountMin);
		for (int i=argCount; i<argCountMin; ++i) {
			strArgs[i] = NULL;
			strLengths[i] = 0;
		}
		wchar_t* s = NULL;							// 初期化必須
		Variant varCopy;							// VT_BYREFだと困るのでコピー用
		int Len;
		for (int i=0; i<argCount; ++i) {
			if (VariantChangeType(&varCopy.data, &(arguments->rgvarg[i]), 0, VT_BSTR) == S_OK) {
				Wrap(&varCopy.data.bstrVal)->GetW(&s, &Len);
			}else {
				s = new wchar_t[1];
				s[0] = 0;
				Len = 0;
			}
			strArgs[argCount - i - 1] = s;			// DISPPARAMSは引数の順序が逆転しているため正しい順に直す
			strLengths[argCount - i - 1] = Len;
		}

		HandleCommand(*pView, id, const_cast<wchar_t const **>(&strArgs[0]), &strLengths[0], argCount);

		for (int j=0; j<argCount; ++j) {
			delete[] strArgs[j];
		}
		return S_OK;
	}
}

