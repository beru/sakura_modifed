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
#include "CLineComment.h"

CLineComment::CLineComment()
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		m_pszLineComment[i][0] = '\0';
		m_nLineCommentPos[i] = -1;
	}
}

/*!
	�s�R�����g�f���~�^���R�s�[����
	@param n [in]           �R�s�[�Ώۂ̃R�����g�ԍ�
	@param buffer [in]      �R�����g������
	@param nCommentPos [in] �R�����g�ʒu�D-1�̂Ƃ��͎w�薳���D
*/
void CLineComment::CopyTo(const int n, const wchar_t* buffer, int nCommentPos)
{
	int nStrLen = wcslen(buffer);
	if (0 < nStrLen && nStrLen < COMMENT_DELIMITER_BUFFERSIZE) {
		wcscpy(m_pszLineComment[n], buffer);
		m_nLineCommentPos[n] = nCommentPos;
		m_nLineCommentLen[n] = nStrLen;
	}else {
		m_pszLineComment[n][0] = L'\0';
		m_nLineCommentPos[n] = -1;
		m_nLineCommentLen[n] = 0;
	}
}

bool CLineComment::Match(int nPos, const CStringRef& cStr) const
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		if (1
			&& m_pszLineComment[i][0] != L'\0'	// �s�R�����g�f���~�^
			&& (m_nLineCommentPos[i] < 0 || m_nLineCommentPos[i] == nPos)	// �ʒu�w��ON.
			&& nPos <= cStr.GetLength() - m_nLineCommentLen[i]	// �s�R�����g�f���~�^
			//&& auto_memicmp(&cStr.GetPtr()[nPos], m_pszLineComment[i], m_nLineCommentLen[i]) == 0	// ��ASCII���啶������������ʂ��Ȃ�	//###locale �ˑ�
			&& wmemicmp_ascii(&cStr.GetPtr()[nPos], m_pszLineComment[i], m_nLineCommentLen[i]) == 0	// ASCII�̂ݑ啶������������ʂ��Ȃ��i�����j
		) {
			return true;
		}
	}
	return false;
}

