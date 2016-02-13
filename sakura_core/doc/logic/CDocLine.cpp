/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, MIK, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDocLine.h"
#include "mem/CMemory.h"

DocLine::DocLine()
	:
	m_pPrev(NULL),
	m_pNext(NULL)
{
}

DocLine::~DocLine()
{
}

/* 空行（スペース、タブ、改行記号のみの行）かどうかを取得する
	true：空行だ。
	false：空行じゃないぞ。

	2002/04/26 YAZAKI
*/
bool DocLine::IsEmptyLine() const
{
	const wchar_t* pLine = GetPtr();
	int nLineLen = GetLengthWithoutEOL();
	for (int i=0; i<nLineLen; ++i) {
		if (pLine[i] != L' ' && pLine[i] != L'\t') {
			return false;	// スペースでもタブでもない文字があったらfalse。
		}
	}
	return true;	// すべてスペースかタブだけだったらtrue。
}

void DocLine::SetEol()
{
	const wchar_t* pData = m_line.GetStringPtr();
	int nLength = m_line.GetStringLength();
	// 改行コード設定
	const wchar_t* p = &pData[nLength] - 1;
	while (p >= pData && WCODE::IsLineDelimiter(*p, GetDllShareData().m_common.m_edit.m_bEnableExtEol)) --p;
	++p;
	if (p >= pData) {
		m_eol.SetTypeByString(p, &pData[nLength] - p);
	}else {
		m_eol = EolType::None;
	}
}


void DocLine::SetDocLineString(const wchar_t* pData, int nLength)
{
	m_line.SetString(pData, nLength);
	SetEol();
}

void DocLine::SetDocLineString(const NativeW& cData)
{
	SetDocLineString(cData.GetStringPtr(), cData.GetStringLength());
}

void DocLine::SetDocLineStringMove(NativeW* pDataFrom)
{
	m_line.swap(*pDataFrom);
	SetEol();
}

void DocLine::SetEol(const Eol& eol, OpeBlk* pOpeBlk)
{
	// 改行コードを削除
	for (int i=0; i<(Int)m_eol.GetLen(); ++i) {
		m_line.Chop();
	}

	// 改行コードを挿入
	m_eol = eol;
	m_line += eol.GetValue2();
}

