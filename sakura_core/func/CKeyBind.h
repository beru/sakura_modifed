/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "Funccode_enum.h"

class FuncLookup;

// �L�[����ێ�����
struct KEYDATA {
	// �L�[�R�[�h
	short			m_nKeyCode;
	
	//	�L�[�̖��O
	TCHAR			m_szKeyName[30];
	
	/*!	�Ή�����@�\�ԍ�

		SHIFT, CTRL, ALT�̂R�̃V�t�g��Ԃ̂��ꂼ��ɑ΂���
		�@�\�����蓖�Ă邽�߁A�z��ɂȂ��Ă���B
	*/
	EFunctionCode	m_nFuncCodeArr[8];
};

// ���z�L�[�R�[�h�Ǝ��g��
#define VKEX_DBL_CLICK		0x0100	// �_�u���N���b�N
#define VKEX_R_CLICK		0x0101	// �E�N���b�N
#define VKEX_MDL_CLICK		0x0102	// ���N���b�N
#define VKEX_LSD_CLICK		0x0103	// ���T�C�h�N���b�N
#define VKEX_RSD_CLICK		0x0104	// �E�T�C�h�N���b�N

#define VKEX_TRI_CLICK		0x0105	// �g���v���N���b�N
#define VKEX_QUA_CLICK		0x0106	// �N�A�h���v���N���b�N

#define VKEX_WHEEL_UP		0x0107	// �z�C�[���A�b�v
#define VKEX_WHEEL_DOWN		0x0108	// �z�C�[���_�E��
#define VKEX_WHEEL_LEFT		0x0109	// �z�C�[����
#define VKEX_WHEEL_RIGHT	0x010A	// �z�C�[���E

extern const TCHAR* jpVKEXNames[];
extern const int jpVKEXNamesLen;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �L�[���蓖�Ċ֘A���[�`��
	
	���ׂĂ̊֐���static�ŁA�ێ�����f�[�^�͂Ȃ��B
*/
class KeyBind {
public:
	/*
	||  Constructors
	*/
	KeyBind();
	~KeyBind();

	/*
	||  �Q�ƌn�����o�֐�
	*/
	static HACCEL CreateAccerelator(int, KEYDATA*);
	static EFunctionCode GetFuncCode(WORD nAccelCmd, int nKeyNameArrNum, KEYDATA* pKeyNameArr, bool bGetDefFuncCode = true);
	static EFunctionCode GetFuncCodeAt(KEYDATA& KeyData, int nState, bool bGetDefFuncCode = true);	// ����̃L�[��񂩂�@�\�R�[�h���擾����	// 2007.02.24 ryoji
	static EFunctionCode GetDefFuncCode(int nKeyCode, int nState);	// �L�[�̃f�t�H���g�@�\���擾����	// 2007.02.22 ryoji

	// �L�[���蓖�Ĉꗗ���쐬����
	static int CreateKeyBindList(HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, NativeW& cMemList, FuncLookup* pFuncLookup, bool bGetDefFuncCode = true);
	static int GetKeyStr(HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CNativeT& cMemList, int nFuncId, bool bGetDefFuncCode = true);	// �@�\�ɑΉ�����L�[���̎擾
	static int GetKeyStrList(HINSTANCE	hInstance, int nKeyNameArrNum,KEYDATA* pKeyNameArr, CNativeT*** pppcMemList, int nFuncId, bool bGetDefFuncCode = true);	// �@�\�ɑΉ�����L�[���̎擾(����)
	static TCHAR* GetMenuLabel(HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, int nFuncId, TCHAR* pszLabel, const TCHAR* pszKey, BOOL bKeyStr, int nLabelSize, bool bGetDefFuncCode = true);	// ���j���[���x���̍쐬	// add pszKey	2010/5/17 Uchi

	static TCHAR* MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey);

protected:
	/*
	||  �����w���p�֐�
	*/
	static bool GetKeyStrSub(int& nKeyNameArrBegin, int nKeyNameArrEnd, KEYDATA* pKeyNameArr,
			int nShiftState, CNativeT& cMemList, int nFuncId, bool bGetDefFuncCode);
};

