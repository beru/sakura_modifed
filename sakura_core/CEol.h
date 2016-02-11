/*! @file
	@brief End of Line��ʂ̊Ǘ�

	@author genta
	@date 2000/5/15 �V�K�쐬
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca

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

#include "_main/global.h"

// 2002/09/22 Moca EOL_CRLF_UNICODE��p�~
// �s�I�[�q�̎��
enum class EolType {
	None,			// 
	CRLF,			// 0d0a
	LF,				// 0a
	CR,				// 0d
	NEL,			// 85
	LS,				// 2028
	PS,				// 2029
	CodeMax,		//
	Unknown = -1	//
};

#define EOL_TYPE_NUM	(int)EolType::CodeMax // 8

// �s�I�[�q�̔z��
extern const EolType gm_pnEolTypeArr[EOL_TYPE_NUM];

#include "basis/SakuraBasis.h"

/*!
	@brief �s���̉��s�R�[�h���Ǘ�

	�Ǘ��Ƃ͌����Ă��I�u�W�F�N�g�����邱�Ƃň��S�ɐݒ���s������֘A���̎擾��
	�I�u�W�F�N�g�ɑ΂��郁�\�b�h�ōs���邾�������A�O���[�o���ϐ��ւ̎Q�Ƃ�
	�N���X�����ɕ����߂邱�Ƃ��ł���̂ł���Ȃ�ɈӖ��͂���Ǝv���B
*/
class Eol {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Eol() { m_eEolType = EolType::None; }
	Eol(EolType t) { SetType(t); }

	// ��r
	bool operator == (EolType t) const { return GetType() == t; }
	bool operator != (EolType t) const { return GetType() != t; }

	// ���
	const Eol& operator = (const Eol& t) { m_eEolType = t.m_eEolType; return *this; }

	// �^�ϊ�
	operator EolType() const { return GetType(); }

	// �ݒ�
	bool SetType(EolType t);	//	Type�̐ݒ�
	void SetTypeByString(const wchar_t* pszData, int nDataLen);
	void SetTypeByString(const char* pszData, int nDataLen);

	// �ݒ�i�t�@�C���ǂݍ��ݎ��Ɏg�p�j
	void SetTypeByStringForFile(const char* pszData, int nDataLen) { SetTypeByString(pszData, nDataLen); }
	void SetTypeByStringForFile_uni(const char* pszData, int nDataLen);
	void SetTypeByStringForFile_unibe(const char* pszData, int nDataLen);

	// �擾
	EolType		GetType()	const { return m_eEolType; }		// ���݂�Type���擾
	LogicInt		GetLen()	const;	// ���݂�EOL�����擾�B�����P�ʁB
	const TCHAR*	GetName()	const;	// ���݂�EOL�̖��̎擾
	const wchar_t*	GetValue2()	const;	// ���݂�EOL������擪�ւ̃|�C���^���擾
	//#####

	bool IsValid() const {
		return m_eEolType >= EolType::CRLF && m_eEolType < EolType::CodeMax;
	}

private:
	EolType	m_eEolType;	// ���s�R�[�h�̎��
};

