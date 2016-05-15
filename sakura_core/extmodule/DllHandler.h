/*!	@file
	@brief DLL�̃��[�h�A�A�����[�h

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, genta

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
#include <string>
#include "_main/global.h"

/*! DllImp �����b�v
	DllImp::DeinitDll ���ĂіY��Ȃ����߂̃w���p�I�N���X�B
	���̂Ƃ���DeinitDll���g���Ă���ӏ��������̂ŁA���̃N���X�̏o�Ԃ͂���܂��񂪁B
	2008.05.10 kobake �쐬
*/
template <class DLLIMP>
class DllHandler {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	DllHandler() {
		pDllImp = new DLLIMP();
		pDllImp->InitDll();
	}
	~DllHandler() {
		pDllImp->DeinitDll(true); // ���I�������Ɏ��s���Ă������I��DLL���
		delete pDllImp;
	}

	// �A�N�Z�T
	DLLIMP* operator -> () { return pDllImp; }

	// ���p��Ԃ̃`�F�b�N�ioperator�Łj
	bool operator!() const { return pDllImp->IsAvailable(); }

private:
	DLLIMP*	pDllImp;
};


// ���ʒ萔
enum class InitDllResultType {
	Success,		// ����
	LoadFailure,	// DLL���[�h���s
	InitFailure,	// ���������Ɏ��s
};

// DLL�̓��I��Load/Unload���s�����߂̃N���X
/*!
	@author genta
	@date Jun. 10, 2001 genta
	@date 2001.07.05 genta InitDll: �����ǉ��B�p�X�̎w��ȂǂɎg����
	@date Apr. 15, 2002 genta RegisterEntries�̒ǉ��B
	@date 2007.06.25 genta InitDll: GetDllNameImp���g���悤�Ɏ�����ύX�D
	@date 2001.07.05 genta GetDllName: �����ǉ��B�p�X�̎w��ȂǂɎg����
	@date 2007.06.25 genta GetDllName: GetDllNameImp���g�p����ꍇ�͕K�{�ł͂Ȃ��̂ŁC
										�������z�֐��͂�߂ăv���[�X�z���_�[��p�ӂ���D
	@date 2008.05.10 kobake �����B�h���N���X�́A�`Imp���I�[�o�[���[�h����Ηǂ��Ƃ��������ł��B
*/
class DllImp {
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                            �^                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
		�A�h���X�ƃG���g�����̑Ή��\�BRegisterEntries�Ŏg����B
		@author YAZAKI
		@date 2002.01.26
	*/
	struct ImportTable {
		void*		proc;
		const char*	name;
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����Ɣj��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	DllImp();
	virtual ~DllImp();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         DLL���[�h                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// DLL�̊֐����Ăяo���邩��Ԃǂ���
	virtual bool IsAvailable() const { return hInstance != NULL; }

	// DLL���[�h�Ə�������
	InitDllResultType InitDll(
		LPCTSTR pszSpecifiedDllName = NULL	// [in] �N���X����`���Ă���DLL���ȊO��DLL��ǂݍ��݂����Ƃ��ɁA����DLL�����w��B
	);

	// �I��������DLL�A�����[�h
	bool DeinitDll(
		bool force = false	// [in] �I�������Ɏ��s���Ă�DLL��������邩�ǂ���
	);

	// �C���X�^���X�n���h���̎擾
	HINSTANCE GetInstance() const { return hInstance; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ���[�h�ς�DLL�t�@�C�����̎擾�B���[�h����Ă��Ȃ� (�܂��̓��[�h�Ɏ��s����) �ꍇ�� NULL ��Ԃ��B
	LPCTSTR GetLoadedDllName() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  �I�[�o�[���[�h�\����                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// DLL�̏�����
	/*!
		DLL�̃��[�h�ɐ�����������ɌĂяo�����D�G���g���|�C���g��
		�m�F�Ȃǂ��s���D

		@retval true ����I��
		@retval false �ُ�I��

		@note false��Ԃ����ꍇ�́A�ǂݍ���DLL���������D
	*/
	virtual bool InitDllImp() = 0;

	// �֐��̏�����
	/*!
		DLL�̃A�����[�h���s�����O�ɌĂяo�����D�������̉���Ȃǂ�
		�s���D

		@retval true ����I��
		@retval false �ُ�I��

		@note false��Ԃ����Ƃ���DLL��Unload�͍s���Ȃ��D
		@par ����
		�f�X�g���N�^����DeinitDll�y��DeinitDllImp���Ăяo���ꂽ�Ƃ���
		�|�����[�t�B�Y�����s���Ȃ����߂ɃT�u�N���X��DeinitDllImp���Ăяo����Ȃ��B
		���̂��߁A�T�u�N���X�̃f�X�g���N�^�ł�DeinitDllImp�𖾎��I�ɌĂяo���K�v������B
		
		DeinitDll���f�X�g���N�^�ȊO����Ăяo�����ꍇ��DeinitDllImp�͉��z�֐��Ƃ���
		�T�u�N���X�̂��̂��Ăяo����A�f�X�g���N�^�͓��R�Ăяo����Ȃ��̂�
		DeinitDllImp���̂��͕̂K�v�ł���B
		
		�f�X�g���N�^����DeinitDllImp���ĂԂƂ��́A����������Ă���Ƃ����ۏ؂��Ȃ��̂�
		�Ăяo���O��IsAvailable�ɂ��m�F��K���s�����ƁB
		
		@date 2002.04.15 genta ���ӏ����ǉ�
	*/
	virtual bool DeinitDllImp();

	// DLL�t�@�C�����̎擾(����������)
	/*!
		DLL�t�@�C�����Ƃ��ĕ����̉\��������C���̂����̈�ł�
		�����������̂��g�p����ꍇ�ɑΉ�����D
		
		�ԍ��ɉ����Ă��ꂼ��قȂ�t�@�C������Ԃ����Ƃ��ł���D
		LoadLibrary()�����counter��0����1�����������ď��ɌĂт������D
		�����DLL�̃��[�h�ɐ�������(����)���C�߂�l�Ƃ���NULL��Ԃ�(���s)
		�܂ő�������D

		@param[in] nIndex �C���f�b�N�X�D(0�`)
		
		@return �����ɉ�����DLL��(LoadLibrary�ɓn��������)�C�܂���NULL�D
	*/
	virtual LPCTSTR GetDllNameImp(int nIndex) = 0;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	bool RegisterEntries(const ImportTable table[]);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	HINSTANCE		hInstance;
	std::tstring	strLoadedDllName;
};

