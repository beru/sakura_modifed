/*!	@file
	@brief キーボードマクロ

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CPPAMacroMgr.h"
#include "mem/CMemory.h"
#include "CMacroFactory.h"
#include <string.h>
#include "io/CTextStream.h"
using namespace std;

PPA PPAMacroMgr::m_cPPA;

PPAMacroMgr::PPAMacroMgr()
{
}

PPAMacroMgr::~PPAMacroMgr()
{
}

/** PPAマクロの実行

	PPA.DLLに、バッファ内容を渡して実行。

	@date 2007.07.20 genta flags追加
*/
bool PPAMacroMgr::ExecKeyMacro(EditView* pEditView, int flags) const
{
	m_cPPA.SetSource(to_achar(m_cBuffer.GetStringPtr()));
	return m_cPPA.Execute(pEditView, flags);
}

/*! キーボードマクロの読み込み（ファイルから）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool PPAMacroMgr::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	TextInputStream in(pszPath);
	if (!in) {
		m_nReady = false;
		return false;
	}

	NativeW memWork;

	// バッファ（memWork）にファイル内容を読み込み、m_cPPAに渡す。
	while (in) {
		wstring szLine = in.ReadLineW();
		szLine += L"\n";
		memWork.AppendString(szLine.c_str());
	}
	in.Close();

	m_cBuffer.SetNativeData(memWork);	// m_cBufferにコピー

	m_nReady = true;
	return true;
}

/*! キーボードマクロの読み込み（文字列から）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool PPAMacroMgr::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	m_cBuffer.SetNativeData(to_wchar(pszCode));	// m_cBufferにコピー

	m_nReady = true;
	return true;
}

// From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
		そのため，過ったオブジェクト生成を行わないために拡張子チェックは必須．

*/
MacroManagerBase* PPAMacroMgr::Creator(const TCHAR* ext)
{
	if (_tcscmp(ext, _T("ppa")) == 0) {
		return new PPAMacroMgr;
	}
	return NULL;
}

/*!	CPPAMacroManagerの登録

	PPAが利用できないときは何もしない。

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
*/
void PPAMacroMgr::declare (void)
{
	if (DLL_SUCCESS == m_cPPA.InitDll()) {
		MacroFactory::getInstance()->RegisterCreator(Creator);
	}
}
// To Here Apr. 29, 2002 genta


