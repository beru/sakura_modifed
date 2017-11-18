/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����
*/
#pragma once

// sakura
#include "_main/global.h"

enum class CommentType {
	Zero	= 0,
	One		= 1,
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*! �u���b�N�R�����g�f���~�^���Ǘ�����

	@note CBlockCommentGroup�́A���L������STypeConfig�Ɋ܂܂��̂ŁA�����o�ϐ��͏�Ɏ��̂������Ă��Ȃ���΂Ȃ�Ȃ��B
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

// 2005.11.10 Moca �A�N�Z�X�֐��ǉ�
class BlockComment {
public:
	// �����Ɣj��
	BlockComment();

	// �ݒ�
	void SetBlockCommentRule(const wchar_t* pszFrom, const wchar_t* pszTo);	// �s�R�����g�f���~�^���R�s�[����

	// ����
	bool Match_CommentFrom(size_t nPos, const StringRef& str) const;			// �s�R�����g�ɒl���邩�m�F����
	size_t Match_CommentTo(size_t nPos, const StringRef& str) const;			// �s�R�����g�ɒl���邩�m�F����

	// �擾
	const wchar_t* getBlockCommentFrom() const { return szBlockCommentFrom; }
	const wchar_t* getBlockCommentTo() const { return szBlockCommentTo; }
	size_t getBlockFromLen() const { return nBlockFromLen; }
	size_t getBlockToLen() const { return nBlockToLen; }

private:
	wchar_t	szBlockCommentFrom[BLOCKCOMMENT_BUFFERSIZE]; // �u���b�N�R�����g�f���~�^(From)
	wchar_t	szBlockCommentTo[BLOCKCOMMENT_BUFFERSIZE];   // �u���b�N�R�����g�f���~�^(To)
	size_t	nBlockFromLen;
	size_t	nBlockToLen;
};

