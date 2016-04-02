/*!	@file
	@brief Outlineオブジェクト

*/
/*
	Copyright (C) 2009, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
		m_nListType(OutlineType::PlugIn),
		m_funcInfoArr(funcInfoArr)
	{
	}

	// デストラクタ
public:
	~OutlineIfObj() {}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const { return m_macroFuncInfoCommandArr; }
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const { return m_macroFuncInfoArr; }
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
		const WCHAR* arguments[],
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
				LogicPoint ptLogic( _wtoi(arguments[1])-1, _wtoi(arguments[0])-1 );
				LayoutPoint ptLayout;
				if (ptLogic.x < 0 || ptLogic.y < 0) {
					ptLayout.x = (Int)ptLogic.x;
					ptLayout.y = (Int)ptLogic.y;
				}else {
					view.GetDocument().m_layoutMgr.LogicToLayout( ptLogic, &ptLayout );
				}
				int nParam = _wtoi(arguments[3]);
				if (index == F_OL_ADDFUNCINFO) {
					m_funcInfoArr.AppendData(
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
					m_funcInfoArr.AppendData(
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
					m_funcInfoArr.AppendData(
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
					m_funcInfoArr.AppendData(
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
			m_sOutlineTitle = to_tchar(arguments[0]);
			break;
		case F_OL_SETLISTTYPE:			// アウトラインリスト種別を指定
			if (!arguments[0]) return false;
			m_nListType = (OutlineType)_wtol(arguments[0]);
			break;
		case F_OL_SETLABEL:				// ラベル文字列を指定
			if (!arguments[0] || !arguments[1]) {
				return false;
			}
			{
				std::wstring sLabel = arguments[1];
				m_funcInfoArr.SetAppendText(_wtol(arguments[0]), sLabel, true);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	// メンバ変数
public:
	tstring m_sOutlineTitle;
	OutlineType m_nListType;
private:
	FuncInfoArr& m_funcInfoArr;
	static MacroFuncInfo m_macroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo m_macroFuncInfoArr[];	// 関数情報(戻り値あり)
};

VARTYPE g_OutlineIfObj_MacroArgEx_s[] = {VT_BSTR};
MacroFuncInfoEx g_OutlineIfObj_FuncInfoEx_s = {5, 5, g_OutlineIfObj_MacroArgEx_s};

// コマンド情報
MacroFuncInfo OutlineIfObj::m_macroFuncInfoCommandArr[] = {
	// ID									関数名							引数										戻り値の型	m_pszData
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
MacroFuncInfo OutlineIfObj::m_macroFuncInfoArr[] = {
	//ID									関数名							引数										戻り値の型	m_pszData
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

