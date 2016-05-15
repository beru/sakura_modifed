/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "FuncInfo.h"

// FuncInfo�N���X�\�z
FuncInfo::FuncInfo(
	LogicInt		nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	LogicInt		nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	LayoutInt		nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	LayoutInt		nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	const TCHAR*	pszFileName,
	int				nInfo				// �t�����
	)
	:
	nDepth(0) // �[��
{
	this->nFuncLineCRLF = nFuncLineCRLF;		// �֐��̂���s(CRLF�P��)
	this->nFuncColCRLF = nFuncColCRLF;			// �֐��̂��錅(CRLF�P��)
	this->nFuncLineLAYOUT = nFuncLineLAYOUT;	// �֐��̂���s(�܂�Ԃ��P��)
	this->nFuncColLAYOUT = nFuncColLAYOUT;		// �֐��̂��錅(�܂�Ԃ��P��)
	memFuncName.SetString(pszFuncName);
	if (pszFileName) {
		memFileName.SetString( pszFileName );
	}
	this->nInfo = nInfo;
}


// FuncInfo�N���X����
FuncInfo::~FuncInfo()
{

}

