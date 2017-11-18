/*! @file
	@brief �X�V�ʒm�y�ъm�F�_�C�A���O

	�t�@�C���̍X�V�ʒm�Ɠ���̊m�F���s���_�C�A���O�{�b�N�X
*/

#include "StdAfx.h"
#include "dlg/DlgFileUpdateQuery.h"
#include "sakura_rc.h"

BOOL DlgFileUpdateQuery::OnInitDialog(
	HWND hWnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	::DlgItem_SetText(hWnd, IDC_UPDATEDFILENAME, pFilename);
	::DlgItem_SetText(hWnd, IDC_QUERYRELOADMSG, bModified ?
		LS(STR_ERR_DLGUPQRY1):LS(STR_ERR_DLGUPQRY2));

	return Dialog::OnInitDialog(hWnd, wParam, lParam);
}

/*!
	�{�^���������ꂽ�Ƃ��̓���
*/
BOOL DlgFileUpdateQuery::OnBnClicked(int id)
{
	int result;
	switch (id) {
	case IDC_BTN_RELOAD: // �ēǍ�
		result = 1;
		break;
	case IDC_BTN_CLOSE: // ����
		result = 0;
		break;
	case IDC_BTN_NOTIFYONLY: // �Ȍ�ʒm���b�Z�[�W�̂�
		result = 2;
		break;
	case IDC_BTN_NOSUPERVISION: // �Ȍ�X�V���Ď����Ȃ�
		result = 3;
		break;
	case IDC_BTN_AUTOLOAD:		// �Ȍ㖢�ҏW�ōă��[�h
		result = 4;
		break;
	default:
		result = 0;
		break;
	}
	CloseDialog(result);

	return 0;
}

