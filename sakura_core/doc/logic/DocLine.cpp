/*!	@file
	@brief �����f�[�^1�s

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

/* ��s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������擾����
	true�F��s���B
	false�F��s����Ȃ����B

	2002/04/26 YAZAKI
*/
bool DocLine::IsEmptyLine() const
{
	const wchar_t* pLine = GetPtr();
	int nLineLen = GetLengthWithoutEOL();
	for (int i=0; i<nLineLen; ++i) {
		if (pLine[i] != L' ' && pLine[i] != L'\t') {
			return false;	// �X�y�[�X�ł��^�u�ł��Ȃ���������������false�B
		}
	}
	return true;	// ���ׂăX�y�[�X���^�u������������true�B
}

void DocLine::SetEol()
{
	const wchar_t* pData = line.GetStringPtr();
	int nLength = line.GetStringLength();
	// ���s�R�[�h�ݒ�
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
	// ���s�R�[�h���폜
	for (size_t i=0; i<eol.GetLen(); ++i) {
		line.Chop();
	}

	// ���s�R�[�h��}��
	this->eol = eol;
	line += eol.GetValue2();
}

