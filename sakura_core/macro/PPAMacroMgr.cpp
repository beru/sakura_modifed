/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author YAZAKI
	@date 2002�N1��26��
*/
/*
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "PPAMacroMgr.h"
#include "mem/Memory.h"
#include "MacroFactory.h"
#include <string.h>
#include "io/TextStream.h"
using namespace std;

PPA PPAMacroMgr::cPPA;

PPAMacroMgr::PPAMacroMgr()
{
}

PPAMacroMgr::~PPAMacroMgr()
{
}

/** PPA�}�N���̎��s

	PPA.DLL�ɁA�o�b�t�@���e��n���Ď��s�B

	@date 2007.07.20 genta flags�ǉ�
*/
bool PPAMacroMgr::ExecKeyMacro(EditView& editView, int flags) const
{
	cPPA.SetSource(to_achar(buffer.GetStringPtr()));
	return cPPA.Execute(editView, flags);
}

/*! �L�[�{�[�h�}�N���̓ǂݍ��݁i�t�@�C������j
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
bool PPAMacroMgr::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	TextInputStream in(pszPath);
	if (!in) {
		nReady = false;
		return false;
	}

	NativeW memWork;

	// �o�b�t�@�imemWork�j�Ƀt�@�C�����e��ǂݍ��݁AcPPA�ɓn���B
	while (in) {
		wstring szLine = in.ReadLineW();
		szLine += L"\n";
		memWork.AppendString(szLine.c_str());
	}
	in.Close();

	buffer.SetNativeData(memWork);	// buffer�ɃR�s�[

	nReady = true;
	return true;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ��݁i�����񂩂�j
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
bool PPAMacroMgr::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	buffer.SetNativeData(to_wchar(pszCode));	// buffer�ɃR�s�[

	nReady = true;
	return true;
}

// From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
		���̂��߁C�߂����I�u�W�F�N�g�������s��Ȃ����߂Ɋg���q�`�F�b�N�͕K�{�D

*/
MacroManagerBase* PPAMacroMgr::Creator(class EditView& view, const TCHAR* ext)
{
	if (_tcscmp(ext, _T("ppa")) == 0) {
		return new PPAMacroMgr;
	}
	return NULL;
}

/*!	CPPAMacroManager�̓o�^

	PPA�����p�ł��Ȃ��Ƃ��͉������Ȃ��B

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
*/
void PPAMacroMgr::Declare (void)
{
	if (cPPA.InitDll() == InitDllResultType::Success) {
		MacroFactory::getInstance().RegisterCreator(Creator);
	}
}
// To Here Apr. 29, 2002 genta


