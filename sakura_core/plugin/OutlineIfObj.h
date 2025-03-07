/*!	@file
	@brief Outlineオブジェクト
*/
#pragma once

#include "macro/WSHIfObj.h"
#include "outline/FuncInfo.h"	// FUNCINFO_INFOMASK

class OutlineIfObj : public WSHIfObj {
	// 型定義
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					// ↓コマンドは以下に追加する
		F_OL_ADDFUNCINFO,						// アウトライン解析に追加する
		F_OL_ADDFUNCINFO2,						// アウトライン解析に追加する（深さ指定）
		F_OL_SETTITLE,							// アウトラインダイアログタイトルを指定
		F_OL_SETLISTTYPE,						// アウトラインリスト種別を指定
		F_OL_SETLABEL,							// ラベル文字列を指定
		F_OL_ADDFUNCINFO3,						//アウトライン解析に追加する（ファイル名）
		F_OL_ADDFUNCINFO4,						//アウトライン解析に追加する（深さ指定、ファイル名）
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST	//↓関数は以下に追加する
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	OutlineIfObj(FuncInfoArr& funcInfoArr)
		:
		WSHIfObj(L"Outline", false),
		nListType(OutlineType::PlugIn),
		funcInfoArr(funcInfoArr)
	{
	}

	// デストラクタ
public:
	~OutlineIfObj() {}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const { return macroFuncInfoCommandArr; }
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const { return macroFuncInfoArr; }
	// 関数を処理する
	bool HandleFunction(
		EditView& view,
		EFunctionCode index,
		const VARIANT* arguments,
		const int argSize,
		VARIANT& result
	) {
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
		switch (index) {
		case F_OL_ADDFUNCINFO:			// アウトライン解析に追加する
		case F_OL_ADDFUNCINFO2:			// アウトライン解析に追加する（深さ指定）
		case F_OL_ADDFUNCINFO3:			//アウトライン解析に追加する（ファイル名）
		case F_OL_ADDFUNCINFO4:			//アウトライン解析に追加する（ファイル名/深さ指定）
			{
				if (!arguments[0]) return false;
				if (!arguments[1]) return false;
				if (!arguments[2]) return false;
				if (!arguments[3]) return false;
				Point ptLogic( _wtoi(arguments[1])-1, _wtoi(arguments[0])-1 );
				Point ptLayout;
				if (ptLogic.x < 0 || ptLogic.y < 0) {
					ptLayout.x = ptLogic.x;
					ptLayout.y = ptLogic.y;
				}else {
					ptLayout = view.GetDocument().layoutMgr.LogicToLayout(ptLogic);
				}
				int nParam = _wtoi(arguments[3]);
				if (index == F_OL_ADDFUNCINFO) {
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1, 
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						NULL,
						nParam
					);
				}else if (index == F_OL_ADDFUNCINFO2) {
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						NULL,
						nParam,
						nDepth
					);
				}else if (index == F_OL_ADDFUNCINFO3) {
					if (argSize < 5 || arguments[4] == NULL) { return false; }
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						arguments[4],
						nParam
					);
				}else if (index == F_OL_ADDFUNCINFO4) {
					if (argSize < 5 || arguments[4] == NULL) { return false; }
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						arguments[4],
						nParam,
						nDepth
					);
				}

			}
			break;
		case F_OL_SETTITLE:				// アウトラインダイアログタイトルを指定
			if (!arguments[0]) return false;
			sOutlineTitle = to_tchar(arguments[0]);
			break;
		case F_OL_SETLISTTYPE:			// アウトラインリスト種別を指定
			if (!arguments[0]) return false;
			nListType = (OutlineType)_wtol(arguments[0]);
			break;
		case F_OL_SETLABEL:				// ラベル文字列を指定
			if (!arguments[0] || !arguments[1]) {
				return false;
			}
			{
				std::wstring sLabel = arguments[1];
				funcInfoArr.SetAppendText(_wtol(arguments[0]), sLabel, true);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	// メンバ変数
public:
	tstring sOutlineTitle;
	OutlineType nListType;
private:
	FuncInfoArr& funcInfoArr;
	static MacroFuncInfo macroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo macroFuncInfoArr[];	// 関数情報(戻り値あり)
};

VARTYPE g_OutlineIfObj_MacroArgEx_s[] = {VT_BSTR};
MacroFuncInfoEx g_OutlineIfObj_FuncInfoEx_s = {5, 5, g_OutlineIfObj_MacroArgEx_s};

// コマンド情報
MacroFuncInfo OutlineIfObj::macroFuncInfoCommandArr[] = {
	// ID									関数名							引数										戻り値の型	pszData
	{EFunctionCode(F_OL_ADDFUNCINFO),		LTEXT("AddFuncInfo"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	nullptr }, // アウトライン解析に追加する
	{EFunctionCode(F_OL_ADDFUNCINFO2),		LTEXT("AddFuncInfo2"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	nullptr }, // アウトライン解析に追加する（深さ指定）
	{EFunctionCode(F_OL_SETTITLE),			LTEXT("SetTitle"),				{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr },	// アウトラインダイアログタイトルを指定
	{EFunctionCode(F_OL_SETLISTTYPE),		LTEXT("SetListType"),			{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	nullptr }, // アウトラインリスト種別を指定
	{EFunctionCode(F_OL_SETLABEL),			LTEXT("SetLabel"),				{VT_I4, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	nullptr }, // ラベル文字列を指定
	{EFunctionCode(F_OL_ADDFUNCINFO3),		LTEXT("AddFuncInfo3"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	&g_OutlineIfObj_FuncInfoEx_s }, //アウトライン解析に追加する（ファイル名）
	{EFunctionCode(F_OL_ADDFUNCINFO4),		LTEXT("AddFuncInfo4"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	&g_OutlineIfObj_FuncInfoEx_s }, //アウトライン解析に追加する（ファイル名、深さ指定）
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

// 関数情報
MacroFuncInfo OutlineIfObj::macroFuncInfoArr[] = {
	//ID									関数名							引数										戻り値の型	pszData
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

