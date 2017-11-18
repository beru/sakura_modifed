/*!	@file
	@brief キーボードマクロ
*/
#include "StdAfx.h"
#include "PPAMacroMgr.h"
#include "mem/Memory.h"
#include "MacroFactory.h"
#include <string.h>
#include "io/TextStream.h"
using namespace std;

PPA PPAMacroMgr::cPPA;

PPAMacroMgr::PPAMacroMgr()
{
}

PPAMacroMgr::~PPAMacroMgr()
{
}

/** PPAマクロの実行

	PPA.DLLに、バッファ内容を渡して実行。
*/
bool PPAMacroMgr::ExecKeyMacro(EditView& editView, int flags) const
{
	cPPA.SetSource(to_achar(buffer.GetStringPtr()));
	return cPPA.Execute(editView, flags);
}

/*! キーボードマクロの読み込み（ファイルから）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool PPAMacroMgr::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	TextInputStream in(pszPath);
	if (!in) {
		nReady = false;
		return false;
	}

	NativeW memWork;

	// バッファ（memWork）にファイル内容を読み込み、cPPAに渡す。
	while (in) {
		wstring szLine = in.ReadLineW();
		szLine += L"\n";
		memWork.AppendString(szLine.c_str());
	}
	in.Close();

	buffer.SetNativeData(memWork);	// bufferにコピー

	nReady = true;
	return true;
}

/*! キーボードマクロの読み込み（文字列から）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool PPAMacroMgr::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	buffer.SetNativeData(to_wchar(pszCode));	// bufferにコピー

	nReady = true;
	return true;
}

/*!
	@brief Factory

	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)
*/
MacroManagerBase* PPAMacroMgr::Creator(class EditView& view, const TCHAR* ext)
{
	if (_tcscmp(ext, _T("ppa")) == 0) {
		return new PPAMacroMgr;
	}
	return NULL;
}

/*!	CPPAMacroManagerの登録

	PPAが利用できないときは何もしない。
*/
void PPAMacroMgr::Declare (void)
{
	if (cPPA.InitDll() == InitDllResultType::Success) {
		MacroFactory::getInstance().RegisterCreator(Creator);
	}
}


