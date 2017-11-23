#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCtrlCode.h"	// �R���g���[���R�[�h�̓���(�_�C�A���O)
#include "env/FormatManager.h"

// ViewCommander�N���X�̃R�}���h(�}���n)�֐��Q

// ���t�}��
void ViewCommander::Command_Ins_Date(void)
{
	// ���t���t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);

	// �e�L�X�g��\��t�� ver1
	Command_InsText(true, to_wchar(szText), wcslen(szText), true);
}


// �����}��
void ViewCommander::Command_Ins_Time(void)
{
	// �������t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);

	// �e�L�X�g��\��t�� ver1
	Command_InsText(true, to_wchar(szText), wcslen(szText), true);
}


//	�R���g���[���R�[�h�̓���(�_�C�A���O)
void ViewCommander::Command_CtrlCode_Dialog(void)
{
	DlgCtrlCode dlgCtrlCode;

	// �R���g���[���R�[�h���̓_�C�A���O��\������
	if (dlgCtrlCode.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument())) {
		// �R���g���[���R�[�h����͂���
		HandleCommand(F_CTRL_CODE, true, dlgCtrlCode.GetCharCode(), 0, 0, 0);
	}
}

