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

#include "macro/CWSHIfObj.h"
#include "outline/CFuncInfo.h"	// FUNCINFO_INFOMASK

class COutlineIfObj : public CWSHIfObj {
	// 型定義
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					// ↓コマンドは以下に追加する
		F_OL_ADDFUNCINFO,						// アウトライン解析に追加する
		F_OL_ADDFUNCINFO2,						// アウトライン解析に追加する（深さ指定）
		F_OL_SETTITLE,							// アウトラインダイアログタイトルを指定
		F_OL_SETLISTTYPE,						// アウトラインリスト種別を指定
		F_OL_SETLABEL,							// ラベル文字列を指定
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST	// ↓関数は以下に追加する
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	COutlineIfObj( CFuncInfoArr& cFuncInfoArr )
		:
		CWSHIfObj( L"Outline", false ),
		m_nListType( OUTLINE_PLUGIN ),
		m_cFuncInfoArr( cFuncInfoArr )
	{
	}

	// デストラクタ
public:
	~COutlineIfObj(){}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const { return m_MacroFuncInfoCommandArr; }
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const { return m_MacroFuncInfoArr; }
	// 関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result) {
		return false;
	}
	// コマンドを処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize) {
		switch (LOWORD(ID)) {
		case F_OL_ADDFUNCINFO:			// アウトライン解析に追加する
		case F_OL_ADDFUNCINFO2:			// アウトライン解析に追加する（深さ指定）
			{
				if (!Arguments[0]) return false;
				if (!Arguments[1]) return false;
				if (!Arguments[2]) return false;
				if (!Arguments[3]) return false;
				CLogicPoint ptLogic( _wtoi(Arguments[1])-1, _wtoi(Arguments[0])-1 );
				if (ptLogic.x < 0) ptLogic.x = 0;
				if (ptLogic.y < 0) ptLogic.y = 0;
				CLayoutPoint ptLayout;
				View->GetDocument()->m_cLayoutMgr.LogicToLayout( ptLogic, &ptLayout );
				int nParam = _wtoi(Arguments[3]);
				if (LOWORD(ID) == F_OL_ADDFUNCINFO) {
					m_cFuncInfoArr.AppendData( ptLogic.GetY()+1, ptLogic.GetX()+1, ptLayout.GetY()+1, ptLayout.GetX()+1, Arguments[2], nParam );
				}else {
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					m_cFuncInfoArr.AppendData( ptLogic.GetY()+1, ptLogic.GetX()+1, ptLayout.GetY()+1, ptLayout.GetX()+1, Arguments[2], nParam, nDepth );
				}
			}
			break;
		case F_OL_SETTITLE:				// アウトラインダイアログタイトルを指定
			if (!Arguments[0]) return false;
			m_sOutlineTitle = to_tchar( Arguments[0] );
			break;
		case F_OL_SETLISTTYPE:			// アウトラインリスト種別を指定
			if (!Arguments[0]) return false;
			m_nListType = (EOutlineType)_wtol(Arguments[0]);
			break;
		case F_OL_SETLABEL:				// ラベル文字列を指定
			if (!Arguments[0] || !Arguments[1]) {
				return false;
			}
			{
				std::wstring sLabel = Arguments[1];
				m_cFuncInfoArr.SetAppendText( _wtol(Arguments[0]), sLabel, true );
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
	EOutlineType m_nListType;
private:
	CFuncInfoArr& m_cFuncInfoArr;
	static MacroFuncInfo m_MacroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo m_MacroFuncInfoArr[];	// 関数情報(戻り値あり)
};

// コマンド情報
MacroFuncInfo COutlineIfObj::m_MacroFuncInfoCommandArr[] = {
	// ID									関数名							引数										戻り値の型	m_pszData
	{EFunctionCode(F_OL_ADDFUNCINFO),		LTEXT("AddFuncInfo"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	NULL }, //アウトライン解析に追加する
	{EFunctionCode(F_OL_ADDFUNCINFO2),		LTEXT("AddFuncInfo2"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	NULL }, //アウトライン解析に追加する（深さ指定）
	{EFunctionCode(F_OL_SETTITLE),			LTEXT("SetTitle"),				{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL },	//アウトラインダイアログタイトルを指定
	{EFunctionCode(F_OL_SETLISTTYPE),		LTEXT("SetListType"),			{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	NULL }, //アウトラインリスト種別を指定
	{EFunctionCode(F_OL_SETLABEL),			LTEXT("SetLabel"),				{VT_I4, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	NULL }, //ラベル文字列を指定
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// 関数情報
MacroFuncInfo COutlineIfObj::m_MacroFuncInfoArr[] = {
	//ID									関数名							引数										戻り値の型	m_pszData
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

