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
	const wchar_t* pData = m_cLine.GetStringPtr();
	int nLength = m_cLine.GetStringLength();
	// ���s�R�[�h�ݒ�
	const wchar_t* p = &pData[nLength] - 1;
	while (p >= pData && WCODE::IsLineDelimiter(*p, GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)) --p;
	++p;
	if (p >= pData) {
		m_cEol.SetTypeByString(p, &pData[nLength] - p);
	}else {
		m_cEol = EOL_NONE;
	}
}


void DocLine::SetDocLineString(const wchar_t* pData, int nLength)
{
	m_cLine.SetString(pData, nLength);
	SetEol();
}

void DocLine::SetDocLineString(const NativeW& cData)
{
	SetDocLineString(cData.GetStringPtr(), cData.GetStringLength());
}

void DocLine::SetDocLineStringMove(NativeW* pcDataFrom)
{
	m_cLine.swap(*pcDataFrom);
	SetEol();
}

void DocLine::SetEol(const Eol& cEol, OpeBlk* pcOpeBlk)
{
	// ���s�R�[�h���폜
	for (int i=0; i<(Int)m_cEol.GetLen(); ++i) {
		m_cLine.Chop();
	}

	// ���s�R�[�h��}��
	m_cEol = cEol;
	m_cLine += cEol.GetValue2();
}

