/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "FuncInfo.h"

//! FuncInfoクラス構築
FuncInfo::FuncInfo(
	LogicInt		nFuncLineCRLF,		// 関数のある行(CRLF単位)
	LogicInt		nFuncColCRLF,		// 関数のある桁(CRLF単位)
	LayoutInt		nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	LayoutInt		nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	const TCHAR*	pszFileName,
	int				nInfo				// 付加情報
	)
	:
	m_nDepth(0) // 深さ
{
	m_nFuncLineCRLF = nFuncLineCRLF;		// 関数のある行(CRLF単位)
	m_nFuncColCRLF = nFuncColCRLF;			// 関数のある桁(CRLF単位)
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	// 関数のある行(折り返し単位)
	m_nFuncColLAYOUT = nFuncColLAYOUT;		// 関数のある桁(折り返し単位)
	m_memFuncName.SetString(pszFuncName);
	if (pszFileName) {
		m_memFileName.SetString( pszFileName );
	}

	m_nInfo = nInfo;
	return;
}


// FuncInfoクラス消滅
FuncInfo::~FuncInfo()
{

}

