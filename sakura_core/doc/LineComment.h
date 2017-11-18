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

	void CopyTo(size_t n, const wchar_t* buffer, int commentPos);	// �s�R�����g�f���~�^���R�s�[����
	bool Match(int pos, const StringRef& str) const;				// �s�R�����g�ɒl���邩�m�F����

	const wchar_t* getLineComment(const int n) {
		return pszLineComment[n];
	}
	int getLineCommentPos(const int n) const {
		return lineCommentPos[n];
	}

private:
	wchar_t	pszLineComment[COMMENT_DELIMITER_NUM][COMMENT_DELIMITER_BUFFERSIZE];	// �s�R�����g�f���~�^
	int		lineCommentPos[COMMENT_DELIMITER_NUM];	// �s�R�����g�̊J�n�ʒu(�����͎w�薳��)
	int		lineCommentLen[COMMENT_DELIMITER_NUM];	// �s�R�����g������̒���
};

