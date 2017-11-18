/*! @file
	@brief �X�V�ʒm�y�ъm�F�_�C�A���O

	�t�@�C���̍X�V�ʒm�Ɠ���̊m�F���s���_�C�A���O�{�b�N�X
*/
#pragma once

#include "dlg/Dialog.h"

class DlgFileUpdateQuery : public Dialog {
public:
	DlgFileUpdateQuery(const TCHAR* filename, bool IsModified)
		:
		pFilename(filename),
		bModified(IsModified)
	{
	}
	virtual BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int);

private:
	const TCHAR* pFilename;
	bool bModified;
};

