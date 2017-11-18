/*!	@file
	@brief 機能番号定義
*/

#pragma once

/*
引数の扱いは
  CEditView::HandleCommand
  CMacro::HandleFunction
  MacroFuncInfo CSMacroMgr::macroFuncInfoArr[]
を参照
*/

#include "Funccode_enum.h"

#ifndef UINT16
#define UINT16 WORD
#endif
#ifndef uint16_t
typedef UINT16 uint16_t;
#endif

// 機能一覧に関するデータ宣言
namespace nsFuncCode {
	extern const uint16_t		ppszFuncKind[];
	extern const size_t			nFuncKindNum;
	extern const int			pnFuncListNumArr[];
	extern const EFunctionCode*	ppnFuncListArr[];
	extern const size_t			nFincListNumArrNum;

	extern const EFunctionCode	pnFuncList_Special[];
	extern const size_t			nFuncList_Special_Num;
};
///////////////////////////////////////////////////////////////////////


// 機能番号に対応したヘルプトピックIDを返す
int FuncID_To_HelpContextID(EFunctionCode nFuncID);

class EditDoc;
struct DllSharedData;

bool IsFuncEnable(const EditDoc&, const DllSharedData&, EFunctionCode);	// 機能が利用可能か調べる
bool IsFuncChecked(const EditDoc&, const DllSharedData&, EFunctionCode);	// 機能がチェック状態か調べる

