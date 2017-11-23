/*!	@file
	@brief �t�@�C���I�[�v���_�C�A���O�{�b�N�X
*/
#pragma once

#include <CommDlg.h>
#include <vector>
#include "util/design_template.h"
#include "Eol.h"
#include "basis/MyString.h"
#include "dlg/Dialog.h"

struct LoadInfo;	// doc/DocListener.h
struct SaveInfo;	// doc/DocListener.h
struct OPENFILENAMEZ;
class DlgOpenFileMem;


/*!	�t�@�C���I�[�v���_�C�A���O�{�b�N�X */
class DlgOpenFile
{
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	DlgOpenFile();
	~DlgOpenFile();
	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU			= std::vector<LPCTSTR>(),
		const std::vector<LPCTSTR>& vOPENFOLDER		= std::vector<LPCTSTR>()
	);

	// ����
	bool DoModal_GetOpenFileName(TCHAR*, bool bSetCurDir = false);	// �J���_�C�A���O ���[�_���_�C�A���O�̕\��
	bool DoModal_GetSaveFileName(TCHAR*, bool bSetCurDir = false);	// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	bool DoModalOpenDlg(LoadInfo* pLoadInfo, std::vector<std::tstring>*, bool bOptions = true);	// �J���_�C�A�O ���[�_���_�C�A���O�̕\��
	bool DoModalSaveDlg(SaveInfo* pSaveInfo, bool bSimpleMode);		// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��

protected:
	DlgOpenFileMem*	mem;

	/*
	||  �����w���p�֐�
	*/

	void DlgOpenFail(void);

	// OS �o�[�W�����Ή��� OPENFILENAME �������p�֐�
	void InitOfn(OPENFILENAMEZ*);

	// �������C�A�E�g�ݒ菈��
	static void InitLayout(HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl);

	// ���g���C�@�\�t�� GetOpenFileName
	bool _GetOpenFileNameRecover(OPENFILENAMEZ* ofn);
	// ���g���C�@�\�t�� GetOpenFileName
	bool GetSaveFileNameRecover(OPENFILENAMEZ* ofn);

	friend UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	DISALLOW_COPY_AND_ASSIGN(DlgOpenFile);
};



