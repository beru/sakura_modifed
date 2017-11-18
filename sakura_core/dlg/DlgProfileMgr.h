/*!	@file
	@brief �v���t�@�C���}�l�[�W��
*/
#ifndef SAKURA_CDLGPROFILEMGR_H_
#define SAKURA_CDLGPROFILEMGR_H_

#include "dlg/Dialog.h"
#include <string>
#include <vector>

struct ProfileSettings
{
	TCHAR szDllLanguage[_MAX_PATH];
	int	nDefaultIndex;
	std::vector<std::tstring> profList;
	bool bDefaultSelect;
};

class DlgProfileMgr : public Dialog
{
public:
	/*
	||  Constructors
	*/
	DlgProfileMgr();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

protected:

	BOOL	OnBnClicked(int);
	INT_PTR	DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

	void	SetData();			// �_�C�A���O�f�[�^�̐ݒ�
	void	SetData(int);		// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData();			// �_�C�A���O�f�[�^�̎擾
	int		GetData(bool);		// �_�C�A���O�f�[�^�̎擾
	LPVOID	GetHelpIdTable(void);

	void	UpdateIni();
	void	CreateProf();
	void	DeleteProf();
	void	RenameProf();
	void	SetDefaultProf(int);
	void	ClearDefaultProf();
public:
	std::tstring strProfileName;

	static bool ReadProfSettings(ProfileSettings&);
	static bool WriteProfSettings(ProfileSettings&);
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGPROFILEMGR_H_ */


