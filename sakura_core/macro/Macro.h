/*!	@file
	@brief キーボードマクロ

	Macroのインスタンスひとつが、1コマンドになる。

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, 鬼

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

#include <Windows.h>
#include <ObjIdl.h>  // VARIANT等
#include "func/Funccode.h"

class TextOutputStream;
class EditView;

enum class MacroParamType {
	Null,
	Int,
	Str,
};

struct MacroParam {
	WCHAR*			m_pData;
	MacroParam*		m_pNext;
	int				m_nDataLen;
	MacroParamType m_type;

	MacroParam():m_pData(NULL), m_pNext(NULL), m_nDataLen(0), m_type(MacroParamType::Null){}
	MacroParam( const MacroParam& obj ){
		if (obj.m_pData) {
			m_pData = new WCHAR[obj.m_nDataLen + 1];
		}else {
			m_pData = NULL;
		}
		m_pNext = NULL;
		m_nDataLen = obj.m_nDataLen;
		m_type = obj.m_type;
	}
	~MacroParam(){
		Clear();
	}
	void Clear(){
		delete[] m_pData;
		m_pData = NULL;
		m_nDataLen = 0;
		m_type = MacroParamType::Null;
	}
	void SetStringParam( const WCHAR* szParam, int nLength = -1 );
	void SetStringParam( const ACHAR* lParam ){ SetStringParam(to_wchar(lParam)); }
	void SetIntParam( const int nParam );
};
/*! @brief キーボードマクロの1コマンド

	引数をリスト構造にして、いくつでも持てるようにしてみました。
	スタックにするのが通例なのかもしれません（よくわかりません）。
	
	今後、制御構造が入っても困らないようにしようと思ったのですが、挫折しました。
	
	さて、このクラスは次のような前提で動作している。

	@li 引数のリストを、m_pParamTopからのリスト構造で保持。
	@li 引数を新たに追加するには、AddParam()を使用する。
	  AddParamにどんな値が渡されてもよいように準備するコト。
	  渡された値が数値なのか、文字列へのポインタなのかは、m_nFuncID（機能 ID）によって、このクラス内で判別し、よろしくやること。
	@li 引数は、Macro内部ではすべて文字列で保持すること（数値97は、"97"として保持）（いまのところ）
*/
class Macro {
public:
	/*
	||  Constructors
	*/
	Macro(EFunctionCode nFuncID);	// 機能IDを指定して初期化
	~Macro();
	void ClearMacroParam();

	void SetNext(Macro* pNext) { m_pNext = pNext; }
	Macro* GetNext() { return m_pNext; }
	// 2007.07.20 genta : flags追加
	bool Exec(EditView* pEditView, int flags) const; // 2007.09.30 kobake const追加
	void Save(HINSTANCE hInstance, TextOutputStream& out) const; // 2007.09.30 kobake const追加
	
	void AddLParam(const LPARAM* lParam, const EditView* pEditView );	//@@@ 2002.2.2 YAZAKI pEditViewも渡す
	void AddStringParam( const WCHAR* szParam, int nLength = -1 );
	void AddStringParam(const ACHAR* lParam) { return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand(EditView* View, EFunctionCode ID, const WCHAR* Argument[], const int ArgLengths[], const int ArgSize);
	static bool HandleFunction(EditView* View, EFunctionCode ID, const VARIANT* Arguments, const int ArgSize, VARIANT& Result);
	// 2009.10.29 syat HandleCommandとHandleFunctionの引数を少しそろえた
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID(HINSTANCE , int , char* , char*);	// 機能ID→関数名，機能名日本語
	static int GetFuncInfoByName(HINSTANCE , const char* , char*);	// 関数名→機能ID，機能名日本語
	static BOOL CanFuncIsKeyMacro(int);	// キーマクロに記録可能な機能かどうかを調べる
#endif

protected:
	static WCHAR* GetParamAt(MacroParam*, int);

	/*
	||  実装ヘルパ関数
	*/
	EFunctionCode	m_nFuncID;		// 機能ID
	MacroParam*	m_pParamTop;	// パラメータ
	MacroParam*	m_pParamBot;
	Macro*			m_pNext;		// 次のマクロへのポインタ
};

