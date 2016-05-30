/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "BlockComment.h"
#include "mem/Memory.h"

BlockComment::BlockComment()
{
	szBlockCommentFrom[0] = '\0';
	szBlockCommentTo[0] = '\0';
	nBlockFromLen = 0;
	nBlockToLen = 0;
}

/*!
	�u���b�N�R�����g�f���~�^���R�s�[����
*/
void BlockComment::SetBlockCommentRule(
	const wchar_t* pszFrom,	// [in] �R�����g�J�n������
	const wchar_t* pszTo	// [in] �R�����g�I��������
	)
{
	size_t nStrLen = wcslen(pszFrom);
	if (0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE) {
		wcscpy(szBlockCommentFrom, pszFrom);
		nBlockFromLen = nStrLen;
	}else {
		szBlockCommentFrom[0] = L'\0';
		nBlockFromLen = 0;
	}
	nStrLen = wcslen(pszTo);
	if (0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE) {
		wcscpy(szBlockCommentTo, pszTo);
		nBlockToLen = nStrLen;
	}else {
		szBlockCommentTo[0] = L'\0';
		nBlockToLen = 0;
	}
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́AnPos����̕����񂪊J�n������(From)�ɓ��Ă͂܂邩�m�F����B

	@retval true  ��v����
	@retval false ��v���Ȃ�����
*/
bool BlockComment::Match_CommentFrom(
	size_t				nPos,	// [in] �T���J�n�ʒu
	const StringRef&	str		// [in] �T���Ώە����� ���T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
	) const
{
	return (1
		&& szBlockCommentFrom[0] != L'\0'
		&& szBlockCommentTo[0] != L'\0'
		&& (int)nPos <= (int)str.GetLength() - (int)nBlockFromLen 	// �u���b�N�R�����g�f���~�^(From)
		//&& auto_memicmp(&str.GetPtr()[nPos], szBlockCommentFrom, nBlockFromLen) == 0	// ��ASCII���啶������������ʂ��Ȃ�	//###locale �ˑ�
		&& wmemicmp_ascii(&str.GetPtr()[nPos], szBlockCommentFrom, nBlockFromLen) == 0	// ASCII�̂ݑ啶������������ʂ��Ȃ��i�����j
	);
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́A���(To)�ɓ��Ă͂܂镶�����nPos�ȍ~����T��

	@return ���Ă͂܂����ʒu��Ԃ����A���Ă͂܂�Ȃ������Ƃ��́AnLineLen�����̂܂ܕԂ��B
*/
size_t BlockComment::Match_CommentTo(
	size_t				nPos,	// [in] �T���J�n�ʒu
	const StringRef&	str		// [in] �T���Ώە����� ���T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
	) const
{
	for (int i=(int)nPos; i<=(int)str.GetLength()-(int)nBlockToLen; ++i) {
		//if (auto_memicmp(&str.GetPtr()[i], szBlockCommentTo, nBlockToLen) == 0) {	// ��ASCII���啶������������ʂ��Ȃ�	//###locale �ˑ�
		if (wmemicmp_ascii(&str.GetPtr()[i], szBlockCommentTo, nBlockToLen) == 0) {	// ASCII�̂ݑ啶������������ʂ��Ȃ��i�����j
			return i + nBlockToLen;
		}
	}
	return str.GetLength();
}

