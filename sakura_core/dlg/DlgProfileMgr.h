/*!	@file
	@brief プロファイルマネージャ
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
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示

protected:

	BOOL	OnBnClicked(int);
	INT_PTR	DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

	void	SetData();			// ダイアログデータの設定
	void	SetData(int);		// ダイアログデータの設定
	int		GetData();			// ダイアログデータの取得
	int		GetData(bool);		// ダイアログデータの取得
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


