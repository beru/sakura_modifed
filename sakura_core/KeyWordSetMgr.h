#pragma once

// �����L�[���[�h�Ǘ�

#include <Windows.h>
#include "_main/global.h"

#define		MAX_SETNUM		100
#define		MAX_SETNAMELEN	32


// �L�[���[�h����
#define		MAX_KEYWORDNUM	15000
#define		MAX_KEYWORDLEN	63

/*! @brief �����L�[���[�h�Ǘ�

	@par �L�[���[�h���ςɂ���
	
	�]���͊e�L�[���[�h�Z�b�g���ɌŒ�T�C�Y�����蓖�ĂĂ�����
	PHP�L�[���[�h�ȂǑ����̃L�[���[�h��o�^�ł��Ȃ������
	�����̃L�[���[�h���蓖�Ăł͖��ʂ����������D
	
	�L�[���[�h��S�̂�1�̔z��ɓ���C�J�n�ʒu��ʓr�Ǘ����邱�Ƃ�
	�L�[���[�h������S�̂ŊǗ�����悤�ɕύX�����D
	
	�Z�b�g����������ꍇ�ɑO�̃Z�b�g�ɃL�[���[�h��o�^���Ă����ꍇ��
	�ۊǏꏊ���s������Ƃ���ȍ~�����ɂ��炷�K�v������D
	�p�ɂɂ��炷���삪�������Ȃ��悤�CnKeywordSetBlockSize(50��)����
	�u���b�N�P�ʂŏꏊ���m�ۂ���悤�ɂ��Ă���D
*/
class KeywordSetMgr {
public:
	/*
	||  Constructors
	*/
	KeywordSetMgr();
	~KeywordSetMgr();
	
	///	@name �L�[���[�h�Z�b�g����
	bool AddKeywordSet(							// �Z�b�g�̒ǉ�
		const wchar_t*	pszSetName,				// [in] �Z�b�g��
		bool			bKeywordCase,			// [in] �啶���������̋�ʁDtrue:����, false:����
		int				nSize			= -1	// [in] �ŏ��ɗ̈���m�ۂ���T�C�Y�D
	);
	bool DelKeywordSet(size_t);			// ���Ԗڂ̃Z�b�g���폜
	const wchar_t* GetTypeName(size_t);	// ���Ԗڂ̃Z�b�g����Ԃ�
	const wchar_t* SetTypeName(size_t, const wchar_t*);	// ���Ԗڂ̃Z�b�g����ݒ肷��
	void SetKeywordCase(size_t, bool);				// ���Ԗڂ̃Z�b�g�̑啶�����������f���Z�b�g����
	bool GetKeywordCase(size_t);					// ���Ԗڂ̃Z�b�g�̑啶�����������f���擾����
	void SortKeyword(size_t);						// ���Ԗڂ̃Z�b�g�̃L�[���[�h���\�[�g����

	size_t SetKeywordArr(size_t, size_t, const wchar_t*);			// ini����L�[���[�h��ݒ肷��
	size_t SetKeywordArr(						// �L�[���[�h�̔z�񂩂�ݒ肷��
		size_t			nIdx,				// [in] �L�[���[�h�Z�b�g�ԍ�
		size_t			nSize,				// [in] ppszKeywordArr�̗v�f��
		const wchar_t*	ppszKeywordArr[]	// [in] �L�[���[�h�̔z��(�d���E�����������A�l���ς݂ł��邱��)
	);
	//@}

	//@{
	///	@name �L�[���[�h����
	size_t GetKeywordNum(size_t);				// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
	const wchar_t* GetKeyword(size_t, size_t);	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�
	const wchar_t* UpdateKeyword(size_t, size_t, const wchar_t*);	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW
	size_t AddKeyword(size_t, const wchar_t*);	// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�
	size_t DelKeyword(size_t, size_t);			// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜
	bool CanAddKeyword(size_t);	// �L�[���[�h���ǉ��\��
	//@}
	
	//@{
	///	@name ����
	//int SearchKeyword(int , const char*, int);				// ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ�
	int SearchKeyword2(size_t nIdx , const wchar_t* pszKeyword, size_t nKeywordLen);	// ���Ԗڂ̃Z�b�g����w��L�[���[�h���o�C�i���T�[�`�B������� 0�ȏ��Ԃ�
	int SearchKeywordSet(const wchar_t* pszKeyword);		// �L�[���[�h�Z�b�g������Z�b�g�ԍ����擾�B������Ȃ���� -1��Ԃ�
	//@}

	size_t CleanKeywords(size_t);			// �L�[���[�h�̐��ځE���p�ł��Ȃ��L�[���[�h�̍폜
	size_t GetAllocSize(size_t) const;		// �m�ۂ��Ă��鐔��Ԃ�
	size_t GetFreeSize() const;			// �����蓖�ău���b�N�̃L�[���[�h����Ԃ�
	void ResetAllKeywordSet(void);		// �S�L�[���[�h�Z�b�g�̍폜�Ə�����

	/*
	|| ���Z�q
	*/
	const KeywordSetMgr& operator = (KeywordSetMgr&);
	/*
	||  Attributes & Operations
	*/
	/*!
		@brief ���݂̃L�[���[�h�Z�b�g�ԍ�(GUI�p)

		�{���̏����Ƃ͖��֌W�����C����E�B���h�E�őI�������Z�b�g��
		�ʂ̃E�B���h�E�̐ݒ��ʂɂ������p�����悤�ɂ��邽�߁D
	*/
	size_t	nCurrentKeywordSetIdx;
	size_t	nKeywordSetNum;				// �L�[���[�h�Z�b�g��
	wchar_t	szSetNameArr[MAX_SETNUM][MAX_SETNAMELEN + 1];	// �L�[���[�h�Z�b�g��
	bool	bKeywordCaseArr[MAX_SETNUM];	// �L�[���[�h�̉p�啶�����������
	size_t	nKeywordNumArr[MAX_SETNUM];	// �L�[���[�h�Z�b�g�ɓo�^����Ă���L�[���[�h��
private:
	// �L�[���[�h�i�[�̈�
	wchar_t	szKeywordArr[MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];	
	char	isSorted[MAX_SETNUM];	// �\�[�g�������ǂ����̃t���O(INI���ۑ�)

protected:
	/*! �L�[���[�h�Z�b�g�̊J�n�ʒu(INI���ۑ�)
		���̊J�n�ʒu�܂ł��m�ۍς݂̗̈�D
		+1���Ă���͍̂Ōオ0�ŏI���悤�ɂ��邽�߁D
	*/
	size_t	nStartIdx[MAX_SETNUM + 1];
	size_t	nKeywordMaxLenArr[MAX_SETNUM]; // ��Ԓ����L�[���[�h�̒���(�\�[�g��̂ݗL��)(INI���ۑ�)

protected:
	/*
	||  �����w���p�֐�
	*/
	//bool KeywordAlloc(int);
	bool KeywordReAlloc(size_t, size_t);
	void KeywordMaxLen( size_t );
};

