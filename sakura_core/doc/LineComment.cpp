/*!	@file
	@brief �s�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Yazaki, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "LineComment.h"

LineComment::LineComment()
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		pszLineComment[i][0] = '\0';
		lineCommentPos[i] = -1;
	}
}

/*!
	�s�R�����g�f���~�^���R�s�[����
	@param n [in]           �R�s�[�Ώۂ̃R�����g�ԍ�
	@param buffer [in]      �R�����g������
	@param commentPos [in] �R�����g�ʒu�D-1�̂Ƃ��͎w�薳���D
*/
void LineComment::CopyTo(const int n, const wchar_t* buffer, int commentPos)
{
	int nStrLen = wcslen(buffer);
	if (0 < nStrLen && nStrLen < COMMENT_DELIMITER_BUFFERSIZE) {
		wcscpy(pszLineComment[n], buffer);
		lineCommentPos[n] = commentPos;
		lineCommentLen[n] = nStrLen;
	}else {
		pszLineComment[n][0] = L'\0';
		lineCommentPos[n] = -1;
		lineCommentLen[n] = 0;
	}
}

bool LineComment::Match(int pos, const StringRef& str) const
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		if (1
			&& pszLineComment[i][0] != L'\0'	// �s�R�����g�f���~�^
			&& (lineCommentPos[i] < 0 || lineCommentPos[i] == pos)	// �ʒu�w��ON.
			&& pos <= str.GetLength() - lineCommentLen[i]	// �s�R�����g�f���~�^
			//&& auto_memicmp(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// ��ASCII���啶������������ʂ��Ȃ�	//###locale �ˑ�
			&& wmemicmp_ascii(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// ASCII�̂ݑ啶������������ʂ��Ȃ��i�����j
		) {
			return true;
		}
	}
	return false;
}

