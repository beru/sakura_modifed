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
	WCHAR*			pData;
	MacroParam*		pNext;
	int				nDataLen;
	MacroParamType type;

	MacroParam():pData(NULL), pNext(NULL), nDataLen(0), type(MacroParamType::Null){}
	MacroParam( const MacroParam& obj ){
		if (obj.pData) {
			pData = new WCHAR[obj.nDataLen + 1];
		}else {
			pData = NULL;
		}
		pNext = NULL;
		nDataLen = obj.nDataLen;
		type = obj.type;
	}
	~MacroParam(){
		Clear();
	}
	void Clear(){
		delete[] pData;
		pData = NULL;
		nDataLen = 0;
		type = MacroParamType::Null;
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

	@li 引数のリストを、pParamTopからのリスト構造で保持。
	@li 引数を新たに追加するには、AddParam()を使用する。
	  AddParamにどんな値が渡されてもよいように準備するコト。
	  渡された値が数値なのか、文字列へのポインタなのかは、nFuncID（機能 ID）によって、このクラス内で判別し、よろしくやること。
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

	void SetNext(Macro* pNext) { pNext = pNext; }
	Macro* GetNext() { return pNext; }
	// 2007.07.20 genta : flags追加
	bool Exec(EditView& editView, int flags) const; // 2007.09.30 kobake const追加
	void Save(HINSTANCE hInstance, TextOutputStream& out) const; // 2007.09.30 kobake const追加
	
	void AddLParam(const LPARAM* lParam, const EditView& editView );	//@@@ 2002.2.2 YAZAKI pEditViewも渡す
	void AddStringParam( const WCHAR* szParam, int nLength = -1 );
	void AddStringParam(const ACHAR* lParam) { return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand(EditView& view, EFunctionCode index, const WCHAR* arguments[], const int argLengths[], const int argSize);
	static bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* argumentss, const int argSize, VARIANT& result);
	// 2009.10.29 syat HandleCommandとHandleFunctionの引数を少しそろえた
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID(HINSTANCE , int , char* , char*);	// 機能ID→関数名，機能名日本語
	static int GetFuncInfoByName(HINSTANCE , const char* , char*);	// 関数名→機能ID，機能名日本語
	static bool CanFuncIsKeyMacro(int);	// キーマクロに記録可能な機能かどうかを調べる
#endif

protected:
	static WCHAR* GetParamAt(MacroParam*, int);

	/*
	||  実装ヘルパ関数
	*/
	EFunctionCode	nFuncID;		// 機能ID
	MacroParam*		pParamTop;	// パラメータ
	MacroParam*		pParamBot;
	Macro*			pNext;		// 次のマクロへのポインタ
};

