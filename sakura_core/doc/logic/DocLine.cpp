// 文書データ1行

#include "StdAfx.h"
#include "DocLine.h"
#include "mem/Memory.h"

DocLine::DocLine()
	:
	pPrev(nullptr),
	pNext(nullptr)
{
}

DocLine::~DocLine()
{
}

/* 空行（スペース、タブ、改行記号のみの行）かどうかを取得する
	true：空行だ。
	false：空行じゃないぞ。
*/
bool DocLine::IsEmptyLine() const
{
	const wchar_t* pLine = GetPtr();
	size_t nLineLen = GetLengthWithoutEOL();
	for (size_t i=0; i<nLineLen; ++i) {
		if (pLine[i] != L' ' && pLine[i] != L'\t') {
			return false;	// スペースでもタブでもない文字があったらfalse。
		}
	}
	return true;	// すべてスペースかタブだけだったらtrue。
}

void DocLine::SetEol()
{
	const wchar_t* pData = line.GetStringPtr();
	size_t nLength = line.GetStringLength();
	// 改行コード設定
	const wchar_t* p = &pData[nLength] - 1;
	while (p >= pData && WCODE::IsLineDelimiter(*p, GetDllShareData().common.edit.bEnableExtEol)) --p;
	++p;
	if (p >= pData) {
		eol.SetTypeByString(p, &pData[nLength] - p);
	}else {
		eol = EolType::None;
	}
}


void DocLine::SetDocLineString(const wchar_t* pData, int nLength)
{
	line.SetString(pData, nLength);
	SetEol();
}

void DocLine::SetDocLineString(const NativeW& data)
{
	SetDocLineString(data.GetStringPtr(), data.GetStringLength());
}

void DocLine::SetDocLineStringMove(NativeW* pDataFrom)
{
	line.swap(*pDataFrom);
	SetEol();
}

void DocLine::SetEol(const Eol& eol, OpeBlk* pOpeBlk)
{
	// 改行コードを削除
	for (size_t i=0; i<eol.GetLen(); ++i) {
		line.Chop();
	}

	// 改行コードを挿入
	this->eol = eol;
	line += eol.GetValue2();
}

