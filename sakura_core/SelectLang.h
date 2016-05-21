/*!	@file
	@brief �e���ꃁ�b�Z�[�W���\�[�X�Ή�

	@author nasukoji
	@date 2011.04.10	�V�K�쐬
*/
/*
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <windows.h>
#include <vector>

#define MAX_SELLANG_NAME_STR	128		// ���b�Z�[�W���\�[�X�̌��ꖼ�̍ő啶���񒷁i�T�C�Y�͓K���j

class SelectLang {
public:
	// ���b�Z�[�W���\�[�X�p�\����
	struct SelLangInfo {
		TCHAR szDllName[MAX_PATH];		// ���b�Z�[�W���\�[�XDLL�̃t�@�C����
		TCHAR szLangName[MAX_SELLANG_NAME_STR];		// ���ꖼ
		HINSTANCE hInstance;			// �ǂݍ��񂾃��\�[�X�̃C���X�^���X�n���h��
		WORD wLangId;					// ����ID
		BOOL bValid;					// ���b�Z�[�W���\�[�XDLL�Ƃ��ėL��
	};

protected:
	//static LPTSTR szDefaultLang;					// ���b�Z�[�W���\�[�XDLL���ǂݍ��ݎ��̃f�t�H���g����
	static SelLangInfo* psLangInfo;				// ���b�Z�[�W���\�[�X�̏��
public:
	typedef std::vector<SelLangInfo*> PSSelLangInfoList;
	static PSSelLangInfoList psLangInfoList;

public:
	/*
	||  Constructors
	*/
	SelectLang() {}
	~SelectLang();

	/*
	||  Attributes & Operations
	*/
	static HINSTANCE getLangRsrcInstance( void );			// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h����Ԃ�
	static LPCTSTR getDefaultLangString( void );			// ���b�Z�[�W���\�[�XDLL���ǂݍ��ݎ��̃f�t�H���g����i"(Japanese)" or "(English(United States))"�j
	static WORD getDefaultLangId(void);

	static HINSTANCE InitializeLanguageEnvironment(void);		// �����������������
	static HINSTANCE LoadLangRsrcLibrary( SelLangInfo& lang );	// ���b�Z�[�W�p���\�[�XDLL�����[�h����
	static void ChangeLang( TCHAR* pszDllName );	// �����ύX����

protected:
	/*
	||  �����w���p�֐�
	*/
	static HINSTANCE ChangeLang( UINT nSelIndex );	// �����ύX����

private:
};


/*!
	@brief �����񃊃\�[�X�ǂݍ��݃N���X

	@date 2011.06.01 nasukoji	�V�K�쐬
*/

#define LOADSTR_ADD_SIZE		256			// �����񃊃\�[�X�p�o�b�t�@�̏����܂��͒ǉ��T�C�Y�iTCHAR�P�ʁj

class ResourceString {
protected:
	// �����񃊃\�[�X�ǂݍ��ݗp�o�b�t�@�N���X
	class LoadStrBuffer {
	public:
		LoadStrBuffer() {
			pszString   = szString;				// �ϐ����ɏ��������o�b�t�@��ڑ�
			nBufferSize = _countof(szString);	// �z���
			nLength     = 0;
			szString[0] = 0;
		}

		// virtual
		~LoadStrBuffer() {
			// �o�b�t�@���擾���Ă����ꍇ�͉������B
			if (pszString && pszString != szString) {
				delete[] pszString;
			}
		}

		/*virtual*/ LPCTSTR GetStringPtr() const { return pszString; }	// �ǂݍ��񂾕�����̃|�C���^��Ԃ�
		/*virtual*/ size_t GetBufferSize() const { return nBufferSize; }	// �ǂݍ��݃o�b�t�@�̃T�C�Y�iTCHAR�P�ʁj��Ԃ�
		/*virtual*/ size_t GetStringLength() const { return nLength; }		// �ǂݍ��񂾕������iTCHAR�P�ʁj��Ԃ�

		/*virtual*/ size_t Load( UINT uid );								// �����񃊃\�[�X��ǂݍ��ށi�ǂݍ��ݎ��s���j

	protected:
		LPTSTR pszString;						// ������ǂݍ��݃o�b�t�@�̃|�C���^
		size_t nBufferSize;						// �擾�z����iTCHAR�P�ʁj
		size_t nLength;							// �擾�������iTCHAR�P�ʁj
		TCHAR szString[LOADSTR_ADD_SIZE];		// ������ǂݍ��݃o�b�t�@�i�o�b�t�@�g����͎g�p����Ȃ��j

	private:
		LoadStrBuffer( const LoadStrBuffer& );					// �R�s�[�֎~�Ƃ���
		LoadStrBuffer operator = ( const LoadStrBuffer& );		// ����֎~�Ƃ���
	};

	static LoadStrBuffer aLoadStrBufferTemp[4];		// ������ǂݍ��݃o�b�t�@�̔z��iResourceString::LoadStringSt() ���g�p����j
	static int nDataTempArrayIndex;					// �Ō�Ɏg�p�����o�b�t�@�̃C���f�b�N�X�iResourceString::LoadStringSt() ���g�p����j
	LoadStrBuffer loadStrBuffer;					// ������ǂݍ��݃o�b�t�@�iResourceString::LoadString() ���g�p����j

public:
	/*
	||  Constructors
	*/
	ResourceString() {}
	ResourceString( UINT uid ) { Load( uid ); }		// ������ǂݍ��ݕt���R���X�g���N�^
	/*virtual*/ ~ResourceString() {}


	/*
	||  Attributes & Operations
	*/
	/*virtual*/ LPCTSTR GetStringPtr() const { return loadStrBuffer.GetStringPtr(); }	// �ǂݍ��񂾕�����̃|�C���^��Ԃ�
//	/*virtual*/ int GetBufferSize() const { return loadStrBuffer.GetBufferSize(); }		// �ǂݍ��݃o�b�t�@�̃T�C�Y�iTCHAR�P�ʁj��Ԃ�
	/*virtual*/ size_t GetStringLength() const { return loadStrBuffer.GetStringLength(); }	// �ǂݍ��񂾕������iTCHAR�P�ʁj��Ԃ�

	static LPCTSTR LoadStringSt( UINT uid );			// �ÓI�o�b�t�@�ɕ����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j
	/*virtual*/ LPCTSTR Load( UINT uid );			// �����񃊃\�[�X��ǂݍ��ށi�e���ꃁ�b�Z�[�W���\�[�X�Ή��j

protected:

private:
	ResourceString( const ResourceString& );					// �R�s�[�֎~�Ƃ���
	ResourceString operator = ( const ResourceString& );		// ����֎~�Ƃ���
};

// �����񃍁[�h�ȈՉ��}�N��
#define LS( id ) ( ResourceString::LoadStringSt( id ) )
#define LSW( id ) to_wchar( ResourceString::LoadStringSt( id ) )

