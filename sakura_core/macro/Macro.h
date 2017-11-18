/*!	@file
	@brief キーボードマクロ

	Macroのインスタンスひとつが、1コマンドになる。
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
	wchar_t*		pData;
	MacroParam*		pNext;
	size_t			nDataLen;
	MacroParamType type;

	MacroParam():pData(NULL), pNext(NULL), nDataLen(0), type(MacroParamType::Null){}
	MacroParam( const MacroParam& obj ){
		if (obj.pData) {
			pData = new wchar_t[obj.nDataLen + 1];
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
	void SetStringParam( const wchar_t* szParam, int nLength = -1 );
	void SetStringParam( const char* lParam ){ SetStringParam(to_wchar(lParam)); }
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

	void SetNext(Macro* pNext) { this->pNext = pNext; }
	Macro* GetNext() { return pNext; }
	bool Exec(EditView& editView, int flags) const;
	void Save(HINSTANCE hInstance, TextOutputStream& out) const;
	
	void AddLParam(const LPARAM* lParam, const EditView& editView );
	void AddStringParam( const wchar_t* szParam, int nLength = -1 );
	void AddStringParam(const char* lParam) { return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize);
	static bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* argumentss, const int argSize, VARIANT& result);
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID(HINSTANCE , int , char* , char*);	// 機能ID→関数名，機能名日本語
	static int GetFuncInfoByName(HINSTANCE , const char* , char*);	// 関数名→機能ID，機能名日本語
	static bool CanFuncIsKeyMacro(int);	// キーマクロに記録可能な機能かどうかを調べる
#endif

protected:
	static wchar_t* GetParamAt(MacroParam*, int);

	/*
	||  実装ヘルパ関数
	*/
	EFunctionCode	nFuncID;		// 機能ID
	MacroParam*		pParamTop;	// パラメータ
	MacroParam*		pParamBot;
	Macro*			pNext;		// 次のマクロへのポインタ
};

