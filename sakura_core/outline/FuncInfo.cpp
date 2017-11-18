/*!	@file
	@brief アウトライン解析  データ要素
*/

#include "StdAfx.h"
#include "FuncInfo.h"

// FuncInfoクラス構築
FuncInfo::FuncInfo(
	size_t	nFuncLineCRLF,		// 関数のある行(CRLF単位)
	size_t	nFuncColCRLF,		// 関数のある桁(CRLF単位)
	size_t	nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	size_t	nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	const TCHAR*	pszFileName,
	int				nInfo				// 付加情報
	)
	:
	nDepth(0) // 深さ
{
	this->nFuncLineCRLF = nFuncLineCRLF;		// 関数のある行(CRLF単位)
	this->nFuncColCRLF = nFuncColCRLF;			// 関数のある桁(CRLF単位)
	this->nFuncLineLAYOUT = nFuncLineLAYOUT;	// 関数のある行(折り返し単位)
	this->nFuncColLAYOUT = nFuncColLAYOUT;		// 関数のある桁(折り返し単位)
	memFuncName.SetString(pszFuncName);
	if (pszFileName) {
		memFileName.SetString( pszFileName );
	}
	this->nInfo = nInfo;
}


// FuncInfoクラス消滅
FuncInfo::~FuncInfo()
{

}

