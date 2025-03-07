/*!	@file
	@brief Complementオブジェクト
*/
#pragma once

#include "macro/WSHIfObj.h"
#include "util/ole_convert.h"

class ComplementIfObj : public WSHIfObj {
	// 型定義
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					// ↓コマンドは以下に追加する
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ↓関数は以下に追加する
		F_CM_GETCURRENTWORD,					// 補完対象の文字列を取得
		F_CM_GETOPTION,							// オプションを取得
		F_CM_ADDLIST,							// 候補に追加
	};

	// コンストラクタ
public:
	ComplementIfObj(std::wstring& curWord, HokanMgr& mgr, int option)
		:
		WSHIfObj(L"Complement", false),
		currentWord(curWord),
		hokanMgr(mgr),
		nOption(option)
	{
	}

	// デストラクタ
public:
	~ComplementIfObj() {}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const { return macroFuncInfoCommandArr; }
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const { return macroFuncInfoArr; };
	// 関数を処理する
	bool HandleFunction(
		EditView&		view,
		EFunctionCode	index,
		const VARIANT*	arguments,
		const int		argSize,
		VARIANT&		result
	) {
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch (LOWORD(index)) {
		case F_CM_GETCURRENTWORD:	// 補完対象の文字列を取得
			{
				SysString s(currentWord.c_str(), currentWord.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_CM_GETOPTION:	// オプションを取得
			{
				Wrap(&result)->Receive(nOption);
			}
			return true;
		case F_CM_ADDLIST:		// 候補に追加する
			{
				std::wstring keyword;
				if (!variant_to_wstr(arguments[0], keyword)) {
					return false;
				}
				const wchar_t* word = keyword.c_str();
				size_t nWordLen = keyword.length();
				if (nWordLen <= 0) {
					return false;
				}
				std::wstring strWord = std::wstring(word, nWordLen);
				if (HokanMgr::AddKouhoUnique(hokanMgr.vKouho, strWord)) {
					Wrap(&result)->Receive((int)hokanMgr.vKouho.size());
				}else {
					Wrap(&result)->Receive(-1);
				}
				return true;
			}
		}
		return false;
	}
	
	// コマンドを処理する
	bool HandleCommand(
		EditView& view,
		EFunctionCode index,
		const wchar_t* arguments[],
		const int argLengths[],
		const int argSize
		)
	{
		return false;
	}

	// メンバ変数
private:
	std::wstring currentWord;
	HokanMgr& hokanMgr;
	int nOption; // 0x01 == IgnoreCase

private:
	static MacroFuncInfo macroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo macroFuncInfoArr[];			// 関数情報(戻り値あり)
};

// コマンド情報
MacroFuncInfo ComplementIfObj::macroFuncInfoCommandArr[] = {
	//ID									関数名							引数										戻り値の型	pszData
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// 関数情報
MacroFuncInfo ComplementIfObj::macroFuncInfoArr[] = {
	//ID								関数名				引数										戻り値の型	pszData
	{EFunctionCode(F_CM_GETCURRENTWORD),L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // 補完対象の文字列を取得
	{EFunctionCode(F_CM_GETOPTION),		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 補完対象の文字列を取得
	{EFunctionCode(F_CM_ADDLIST),		L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // 候補に追加する
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

