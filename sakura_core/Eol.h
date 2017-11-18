#pragma once

// End of Line��ʂ̊Ǘ�

#include "_main/global.h"

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

#define EOL_TYPE_NUM	(size_t)EolType::CodeMax // 8

// �s�I�[�q�̔z��
extern const EolType g_pnEolTypeArr[EOL_TYPE_NUM];

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
	Eol() { eEolType = EolType::None; }
	Eol(EolType t) { SetType(t); }

	// ��r
	bool operator == (EolType t) const { return GetType() == t; }
	bool operator != (EolType t) const { return GetType() != t; }

	// ���
	const Eol& operator = (const Eol& t) { eEolType = t.eEolType; return *this; }

	// �^�ϊ�
	operator EolType() const { return GetType(); }

	// �ݒ�
	bool SetType(EolType t);	//	Type�̐ݒ�
	void SetTypeByString(const wchar_t* pszData, size_t nDataLen);
	void SetTypeByString(const char* pszData, size_t nDataLen);

	// �ݒ�i�t�@�C���ǂݍ��ݎ��Ɏg�p�j
	void SetTypeByStringForFile(const char* pszData, size_t nDataLen) { SetTypeByString(pszData, nDataLen); }
	void SetTypeByStringForFile_uni(const char* pszData, size_t nDataLen);
	void SetTypeByStringForFile_unibe(const char* pszData, size_t nDataLen);

	// �擾
	EolType			GetType()	const { return eEolType; }		// ���݂�Type���擾
	size_t			GetLen()	const;	// ���݂�EOL�����擾�B�����P�ʁB
	const TCHAR*	GetName()	const;	// ���݂�EOL�̖��̎擾
	const wchar_t*	GetValue2()	const;	// ���݂�EOL������擪�ւ̃|�C���^���擾
	//#####

	bool IsValid() const {
		return eEolType >= EolType::CRLF && eEolType < EolType::CodeMax;
	}

private:
	EolType	eEolType;	// ���s�R�[�h�̎��
};

