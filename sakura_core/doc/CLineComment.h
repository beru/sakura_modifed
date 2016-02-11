/*!	@file
	@brief �s�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 2002, Yazaki

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

// sakura
#include "_main/global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
#define COMMENT_DELIMITER_NUM	3
#define COMMENT_DELIMITER_BUFFERSIZE	16

/*! �s�R�����g�f���~�^���Ǘ�����

	@note LineComment�́A���L������STypeConfig�Ɋ܂܂��̂ŁA�����o�ϐ��͏�Ɏ��̂������Ă��Ȃ���΂Ȃ�Ȃ��B
*/
class LineComment {
public:
	/*
	||  Constructors�F�R���p�C���W�����g�p�B
	*/
	LineComment();

	void CopyTo(const int n, const wchar_t* buffer, int nCommentPos);	// �s�R�����g�f���~�^���R�s�[����
	bool Match(int nPos, const CStringRef& cStr) const;				// �s�R�����g�ɒl���邩�m�F����

	const wchar_t* getLineComment(const int n) {
		return m_pszLineComment[n];
	}
	int getLineCommentPos(const int n) const {
		return m_nLineCommentPos[n];
	}

private:
	wchar_t	m_pszLineComment[COMMENT_DELIMITER_NUM][COMMENT_DELIMITER_BUFFERSIZE];	// �s�R�����g�f���~�^
	int		m_nLineCommentPos[COMMENT_DELIMITER_NUM];	// �s�R�����g�̊J�n�ʒu(�����͎w�薳��)
	int		m_nLineCommentLen[COMMENT_DELIMITER_NUM];	// �s�R�����g������̒���
};

