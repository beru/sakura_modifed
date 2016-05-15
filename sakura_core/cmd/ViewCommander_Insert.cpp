/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�}���n)�֐��Q

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCtrlCode.h"	// �R���g���[���R�[�h�̓���(�_�C�A���O)
#include "env/FormatManager.h"

// ���t�}��
void ViewCommander::Command_INS_DATE(void)
{
	// ���t���t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT(true, to_wchar(szText), LogicInt(-1), true);
}


// �����}��
void ViewCommander::Command_INS_TIME(void)
{
	// �������t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT(true, to_wchar(szText), LogicInt(-1), true);
}


// from ViewCommander_New.cpp
/*!	�R���g���[���R�[�h�̓���(�_�C�A���O)
	@author	MIK
	@date	2002/06/02
*/
void ViewCommander::Command_CtrlCode_Dialog(void)
{
	DlgCtrlCode dlgCtrlCode;

	// �R���g���[���R�[�h���̓_�C�A���O��\������
	if (dlgCtrlCode.DoModal(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetDocument())) {
		// �R���g���[���R�[�h����͂���
		// 2013.06.11 Command_WCHAR -> HandleCommand �}�N���L�^�Ή�
		// 2013.12.12 F_WCHAR -> F_CTRL_CODE
		HandleCommand(F_CTRL_CODE, true, dlgCtrlCode.GetCharCode(), 0, 0, 0);
	}
}

