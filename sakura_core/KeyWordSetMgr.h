/*!	@file
	@brief �����L�[���[�h�Ǘ�

	@author Norio Nakatani

	@date 2000.12.01 MIK binary search
	@date 2005.01.26 Moca �L�[���[�h���ω�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, jepro
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#pragma once

#include <Windows.h>
#include "_main/global.h"// 2002/2/10 aroka

#define		MAX_SETNUM		100	// 2007.12.01 genta �ő�l����
#define		MAX_SETNAMELEN	32


// �L�[���[�h���� (2005.01.27 1�Z�b�g������̐����Z�b�g�S�̂̑����ֈӖ��ύX)
#define		MAX_KEYWORDNUM	15000
#define		MAX_KEYWORDLEN	63

/*! @brief �����L�[���[�h�Ǘ�

	@date 2005.01.27 Moca �L�[���[�h�����ςɁD
	
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
	bool DelKeywordSet(int);			// ���Ԗڂ̃Z�b�g���폜
	const wchar_t* GetTypeName(int);	// ���Ԗڂ̃Z�b�g����Ԃ�
	const wchar_t* SetTypeName(int, const wchar_t*);	// ���Ԗڂ̃Z�b�g����ݒ肷�� // 2005.01.26 Moca
	void SetKeywordCase(int, bool);				// ���Ԗڂ̃Z�b�g�̑啶�����������f���Z�b�g����		//MIK
	bool GetKeywordCase(int);					// ���Ԗڂ̃Z�b�g�̑啶�����������f���擾����		//MIK
	void SortKeyword(int);						// ���Ԗڂ̃Z�b�g�̃L�[���[�h���\�[�g����			//MIK

	// From Here 2004.07.29 Moca �ǉ� �ϒ��L��
	int SetKeywordArr(int, int, const wchar_t*);			// ini����L�[���[�h��ݒ肷��
	int SetKeywordArr(						// �L�[���[�h�̔z�񂩂�ݒ肷��
		int				nIdx,				// [in] �L�[���[�h�Z�b�g�ԍ�
		int				nSize,				// [in] ppszKeywordArr�̗v�f��
		const wchar_t*	ppszKeywordArr[]	// [in] �L�[���[�h�̔z��(�d���E�����������A�l���ς݂ł��邱��)
	);
	// To Here 2004.07.29 Moca
	//@}

	//@{
	///	@name �L�[���[�h����
	int GetKeywordNum(int);				// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
	const wchar_t* GetKeyword(int , int);	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�
	const wchar_t* UpdateKeyword(int , int , const WCHAR*);	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW
	int AddKeyword(int, const wchar_t*);	// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�
	int DelKeyword(int , int);			// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜
	bool CanAddKeyword(int);	// �L�[���[�h���ǉ��\��
	//@}
	
	//@{
	///	@name ����
	//int SearchKeyword(int , const char*, int);				// ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ�
//	BOOL IsModify(KeywordSetMgr&, BOOL* pnModifyFlagArr);	// �ύX�󋵂𒲍�	// Uchi 2010/4/14 ���̂������̂ō폜
	int SearchKeyword2(int nIdx , const wchar_t* pszKeyword, int nKeywordLen);	// ���Ԗڂ̃Z�b�g����w��L�[���[�h���o�C�i���T�[�`�B������� 0�ȏ��Ԃ�	//MIK
	int SearchKeywordSet(const wchar_t* pszKeyword);		// �L�[���[�h�Z�b�g������Z�b�g�ԍ����擾�B������Ȃ���� -1��Ԃ�	// Uchi 2010/4/14
	//@}

	// From Here 2004.07.29 Moca �ǉ� �ϒ��L��
	int CleanKeywords(int);				// �L�[���[�h�̐��ځE���p�ł��Ȃ��L�[���[�h�̍폜
	int GetAllocSize(int) const;		// �m�ۂ��Ă��鐔��Ԃ�
	int GetFreeSize() const;			// �����蓖�ău���b�N�̃L�[���[�h����Ԃ�
	void ResetAllKeywordSet(void);		// �S�L�[���[�h�Z�b�g�̍폜�Ə�����
	// To Here 2004.07.29 Moca

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
	int		m_nCurrentKeywordSetIdx;
	int		m_nKeywordSetNum;				// �L�[���[�h�Z�b�g��
	wchar_t	m_szSetNameArr[MAX_SETNUM][MAX_SETNAMELEN + 1];	// �L�[���[�h�Z�b�g��
	bool	m_bKeywordCaseArr[MAX_SETNUM];	// �L�[���[�h�̉p�啶�����������
	int		m_nKeywordNumArr[MAX_SETNUM];	// �L�[���[�h�Z�b�g�ɓo�^����Ă���L�[���[�h��
private:
	// �L�[���[�h�i�[�̈�
	wchar_t	m_szKeywordArr[MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];	
	char	m_isSorted[MAX_SETNUM];	// �\�[�g�������ǂ����̃t���O(INI���ۑ�)	 //MIK

protected:
	// 2004.07.29 Moca �ϒ��L��
	/*! �L�[���[�h�Z�b�g�̊J�n�ʒu(INI���ۑ�)
		���̊J�n�ʒu�܂ł��m�ۍς݂̗̈�D
		+1���Ă���͍̂Ōオ0�ŏI���悤�ɂ��邽�߁D
	*/
	int		m_nStartIdx[MAX_SETNUM + 1];
	int		m_nKeywordMaxLenArr[MAX_SETNUM]; // ��Ԓ����L�[���[�h�̒���(�\�[�g��̂ݗL��)(INI���ۑ�)

protected:
	/*
	||  �����w���p�֐�
	*/
	//bool KeywordAlloc(int);
	bool KeywordReAlloc(int, int);
	void KeywordMaxLen( int );
};

