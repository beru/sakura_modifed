/*!	@file
	@brief プロファイルマネージャ
*/
#include "StdAfx.h"
#include "dlg/DlgProfileMgr.h"
#include "dlg/DlgInput1.h"
#include "DataProfile.h"
#include "util/fileUtil.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_LIST_PROFILE,				HIDC_LIST_PROFILE,				//プロファイル一覧
	IDC_CHECK_PROF_DEFSTART,		HIDC_CHECK_PROF_DEFSTART,		//デフォルト設定にして起動
	IDOK,							HIDOK_PROFILEMGR,				//起動
	IDCANCEL,						HIDCANCEL_PROFILEMGR,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_PROFILEMGR_BUTTON_HELP,	//ヘルプ
	IDC_BUTTON_PROF_CREATE,			HIDC_BUTTON_PROF_CREATE,		//新規作成
	IDC_BUTTON_PROF_RENAME,			HIDC_BUTTON_PROF_RENAME,		//名前変更
	IDC_BUTTON_PROF_DELETE,			HIDC_BUTTON_PROF_DELETE,		//削除
	IDC_BUTTON_PROF_DEFSET,			HIDC_BUTTON_PROF_DEFSET,		//デフォルト設定
	IDC_BUTTON_PROF_DEFCLEAR,		HIDC_BUTTON_PROF_DEFCLEAR,		//デフォルト解除
	0, 0
};

DlgProfileMgr::DlgProfileMgr()
	:
	Dialog(false, false)
{
	return;
}

// モーダルダイアログの表示
INT_PTR DlgProfileMgr::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	return Dialog::DoModal(hInstance, hwndParent, IDD_PROFILEMGR, lParam);
}


static std::tstring GetProfileMgrFileName(LPCTSTR profName = NULL)
{
	static TCHAR szPath[_MAX_PATH];
	static TCHAR szPath2[_MAX_PATH];
	static TCHAR* pszPath;
	static bool bSet = false;
	if (!bSet) {
		pszPath = szPath;
		FileNameManager::GetIniFileNameDirect( szPath, szPath2, _T("") );
		if (szPath[0] == _T('\0')) {
			pszPath = szPath2;
		}
		bSet = true;
	}
	
	TCHAR szDir[_MAX_PATH];
	SplitPath_FolderAndFile(pszPath, szDir, NULL);

	TCHAR szExePath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir2[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(NULL, szExePath, _countof(szExePath));
	_tsplitpath(szExePath, szDrive, szDir2, szFname, szExt);

	TCHAR szIniFile[_MAX_PATH];
	if (!profName) {
		auto_snprintf_s(szIniFile, _MAX_PATH - 1, _T("%ts%ts%ts_prof%ts"), szDrive, szDir2, szFname, _T(".ini"));
	}else {
		auto_snprintf_s(szIniFile, _MAX_PATH - 1, _T("%ts%ts%ts"), szDrive, szDir2, profName);
	}

	return std::tstring(szIniFile);
}


// ダイアログデータの設定
void DlgProfileMgr::SetData()
{
	SetData(-1);
}


void DlgProfileMgr::SetData( int nSelIndex )
{
	int nExtent = 0;
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);

	List_ResetContent( hwndList );
	ProfileSettings settings;
	ReadProfSettings(settings);
	std::tstring strdef = _T("(default)");
	if (settings.nDefaultIndex == 0) {
		strdef += _T("*");
	}
	List_AddString(hwndList, strdef.c_str());
	TextWidthCalc calc(hwndList);
	calc.SetDefaultExtend(TextWidthCalc::WIDTH_MARGIN_SCROLLBER);
	size_t count = settings.profList.size();
	for (size_t i=0; i<count; ++i) {
		std::tstring str = settings.profList[i];
		if (settings.nDefaultIndex == i + 1) {
			str += _T("*");
		}
		List_AddString(hwndList, str.c_str());
		calc.SetTextWidthIfMax(str.c_str());
	}
	List_SetHorizontalExtent(hwndList, calc.GetCx());
	if (nSelIndex == -1) {
		nSelIndex = settings.nDefaultIndex;
	}
	if (nSelIndex < 0) {
		nSelIndex = 0;
	}
	List_SetCurSel(hwndList, nSelIndex);

	EnableItem(IDC_BUTTON_PROF_DEFCLEAR, settings.nDefaultIndex != -1);
	CheckButton(IDC_CHECK_PROF_DEFSTART, settings.bDefaultSelect);
}


static bool MyList_GetText(
	HWND hwndList,
	int index,
	TCHAR* szText
	)
{
	List_GetText(hwndList, index, szText);
	TCHAR* pos = auto_strchr(szText, _T('*'));
	if (pos) {
		*pos = _T('\0');
		return true;
	}
	return false;
}


// ダイアログデータの取得
int DlgProfileMgr::GetData()
{
	return GetData(true);
}

int DlgProfileMgr::GetData(bool bStart)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	int nCurIndex = List_GetCurSel(hwndList);
	TCHAR szText[_MAX_PATH];
	MyList_GetText(hwndList, nCurIndex, szText);
	strProfileName = szText;
	if (strProfileName == _T("(default)")) {
		strProfileName = _T("");
	}
	bool bDefaultSelect = IsButtonChecked(IDC_CHECK_PROF_DEFSTART);
	ProfileSettings settings;
	ReadProfSettings(settings);
	bool bWrite = false;
	if (settings.bDefaultSelect != bDefaultSelect) {
		bWrite = true;
	}
	if (bDefaultSelect && bStart) {
		bWrite = true;
		SetDefaultProf( nCurIndex );
	}
	if (bWrite) {
		UpdateIni();
	}
	return 1;
}


BOOL DlgProfileMgr::OnBnClicked( int wID )
{
	switch (wID) {
	case IDC_BUTTON_PROF_CREATE:
		CreateProf();
		break;

	case IDC_BUTTON_PROF_RENAME:
		RenameProf();
		break;

	case IDC_BUTTON_PROF_DELETE:
		DeleteProf();
		break;

	case IDC_BUTTON_PROF_DEFSET:
		{
			HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
			int nSelIndex = List_GetCurSel(hwndList);
			SetDefaultProf(nSelIndex);
			UpdateIni();
			List_SetCurSel(hwndList, nSelIndex);
			EnableItem(IDC_BUTTON_PROF_DEFCLEAR, true);
		}
		break;

	case IDC_BUTTON_PROF_DEFCLEAR:
		{
			HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
			int nSelIndex = List_GetCurSel(hwndList);
			ClearDefaultProf();
			UpdateIni();
			List_SetCurSel(hwndList, nSelIndex);
			EnableItem(IDC_BUTTON_PROF_DEFCLEAR, false);
		}
		break;

	case IDC_BUTTON_HELP:
		// 「検索」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROFILEMGR));
		break;

	case IDOK:
		GetData();
		CloseDialog(1);
		return TRUE;
	case IDCANCEL:
		GetData(false);
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}


INT_PTR DlgProfileMgr::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LIST_PROFILE) {
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				HWND hwndList = (HWND)lParam;
				int nIdx = List_GetCurSel(hwndList);
				EnableItem(IDC_BUTTON_PROF_DELETE, nIdx != 0);
				EnableItem(IDC_BUTTON_PROF_RENAME, nIdx != 0);
				return TRUE;
			}
		}
	}
	return result;
}


void DlgProfileMgr::UpdateIni()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	ProfileSettings settings;
	ReadProfSettings(settings);
	settings.profList.clear();
	settings.nDefaultIndex = -1;
	int nCount = List_GetCount(hwndList);
	for (int i=0; i<nCount; ++i) {
		TCHAR szProfileName[_MAX_PATH];
		if (MyList_GetText(hwndList, i, szProfileName)) {
			settings.nDefaultIndex = i;
		}
		if (0 < i) {
			settings.profList.emplace_back(szProfileName);
		}
	}
	settings.bDefaultSelect = IsButtonChecked(IDC_CHECK_PROF_DEFSTART);

	if (!WriteProfSettings(settings)) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_WRITE));
	}
}


static bool IsProfileDuplicate(
	HWND hwndList,
	LPCTSTR szProfName,
	int skipIndex
	)
{
	int nCount = List_GetCount(hwndList);
	for (int i=0; i<nCount; ++i) {
		if (i == skipIndex) {
			continue;
		}
		TCHAR szProfileName[_MAX_PATH];
		MyList_GetText(hwndList, i, szProfileName);
		if (auto_stricmp(szProfName, szProfileName) == 0) {
			return true;
		}
	}
	return false;
}

void DlgProfileMgr::CreateProf()
{
	DlgInput1 dlgInput1;
	int max_size = _MAX_PATH;
	TCHAR szText[_MAX_PATH];
	std::tstring strTitle = LS(STR_DLGPROFILE_NEW_PROF_TITLE);
	std::tstring strMessage = LS(STR_DLGPROFILE_NEW_PROF_MSG);
	szText[0] = _T('\0');
	if (!dlgInput1.DoModal(G_AppInstance(), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size, szText)) {
		return;
	}
	if (szText[0] == _T('\0')) {
		return;
	}
	std::wstring strText = to_wchar(szText);
	static const wchar_t szReservedChars[] = L"/\\*?<>&|:\"'\t";
	for (size_t x=0; x<_countof(szReservedChars); ++x) {
		if (strText.npos != strText.find(szReservedChars[x])) {
			ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR));
			return;
		}
	}
	if (auto_strcmp( szText, _T("..") ) == 0) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR));
		return;
	}
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	if (IsProfileDuplicate(hwndList, szText, -1)) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY));
		return;
	}
	std::tstring strProfDir = GetProfileMgrFileName(szText);
	if (IsFileExists(strProfDir.c_str(), true)) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_FILE));
		return;
	}

	List_AddString(hwndList, szText);
	int sel = List_GetCurSel(hwndList);
	UpdateIni();
	SetData(sel);
}


void DlgProfileMgr::DeleteProf()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	int nCurIndex = List_GetCurSel(hwndList);
	List_DeleteString(hwndList, nCurIndex);
	UpdateIni();
	if (List_GetCount(hwndList) <= nCurIndex) {
		--nCurIndex;
	}
	SetData(nCurIndex);
}


void DlgProfileMgr::RenameProf()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	DlgInput1 dlgInput1;
	int nCurIndex = List_GetCurSel(hwndList);
	TCHAR szText[_MAX_PATH];
	bool bDefault = MyList_GetText(hwndList, nCurIndex, szText);
	TCHAR szTextOld[_MAX_PATH];
	auto_strcpy(szTextOld, szText);
	std::tstring strTitle = LS(STR_DLGPROFILE_RENAME_TITLE);
	std::tstring strMessage = LS(STR_DLGPROFILE_RENAME_MSG);
	int max_size = _MAX_PATH;
	if (!dlgInput1.DoModal(G_AppInstance(), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size, szText)) {
		return;
	}
	if (szText[0] == _T('\0')) {
		return;
	}
	if (auto_strcmp(szTextOld, szText) == 0) {
		return; // 未変更
	}
	std::wstring strText = to_wchar(szText);
	static const wchar_t szReservedChars[] = L"/\\*?<>&|:\"'\t";
	for (size_t x=0; x<_countof(szReservedChars); ++x) {
		if (strText.npos != strText.find(szReservedChars[x])) {
			ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR));
			return;
		}
	}
	if (auto_strcmp(szText, _T("..")) == 0) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR));
		return;
	}
	if (IsProfileDuplicate(hwndList, szText, nCurIndex)) {
		ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY));
		return;
	}
	std::tstring strProfDirOld = GetProfileMgrFileName(szTextOld);
	std::tstring strProfDir = GetProfileMgrFileName(szText);
	if (IsFileExists(strProfDirOld.c_str(), false)) {
		if (!IsFileExists(strProfDirOld.c_str(), true)) {
			// プロファイル名はディレクトリ
			if (!::MoveFile(strProfDirOld.c_str(), strProfDir.c_str())) {
				ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_RENAME));
				return;
			}
		}else {
			// 旧プロファイル名はファイルだったので新規プロファイルとして作成確認
			if (IsFileExists(strProfDir.c_str(), true)) {
				ErrorMessage(GetHwnd(), LS(STR_DLGPROFILE_ERR_FILE));
				return;
			}
		}
	}
	if (bDefault) {
		auto_strcat(szText, _T("*"));
	}
	List_DeleteString(hwndList, nCurIndex);
	List_InsertString(hwndList, nCurIndex, szText);
	UpdateIni();
	SetData(nCurIndex);
}


void DlgProfileMgr::SetDefaultProf(int index)
{
	ClearDefaultProf();
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	TCHAR szProfileName[_MAX_PATH];
	MyList_GetText(hwndList, index, szProfileName);
	List_DeleteString(hwndList, index);
	auto_strcat(szProfileName, _T("*"));
	List_InsertString(hwndList, index, szProfileName);
}


void DlgProfileMgr::ClearDefaultProf()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);
	int nCount = List_GetCount(hwndList);
	for (int i=0; i<nCount; ++i) {
		TCHAR szProfileName[_MAX_PATH];
		if (MyList_GetText(hwndList, i, szProfileName)) {
			List_DeleteString(hwndList, i);
			List_InsertString(hwndList, i, szProfileName);
		}
	}
}


static bool IOProfSettings(
	ProfileSettings& settings,
	bool bWrite
	)
{
	DataProfile profile;
	if (bWrite) {
		profile.SetWritingMode();
	}else {
		profile.SetReadingMode();
	}
	std::tstring strIniName = GetProfileMgrFileName();
	if (!bWrite) {
		if (!profile.ReadProfile(strIniName.c_str())) {
			return false;
		}
	}
	int nCount = (int)settings.profList.size();
	const wchar_t* const pSection = L"Profile";
	profile.IOProfileData(pSection , L"nCount", nCount );
	for (int i=0; i<nCount; ++i) {
		wchar_t szKey[64];
		std::tstring strProfName;
		swprintf(szKey, L"P[%d]", i + 1); // 1開始
		if (bWrite) {
			strProfName = settings.profList[i];
			std::wstring wstrProfName = to_wchar(strProfName.c_str());
			profile.IOProfileData(pSection, szKey, wstrProfName);
		}else {
			std::wstring wstrProfName;
			profile.IOProfileData(pSection, szKey, wstrProfName);
			strProfName = to_tchar(wstrProfName.c_str());
			settings.profList.push_back(strProfName);
		}
	}
	profile.IOProfileData(pSection, L"nDefaultIndex", settings.nDefaultIndex);
	if (nCount < settings.nDefaultIndex) {
		settings.nDefaultIndex = -1;
	}
	if (settings.nDefaultIndex < -1) {
		settings.nDefaultIndex = -1;
	}
	profile.IOProfileData(pSection, L"szDllLanguage", StringBufferT(settings.szDllLanguage, _countof(settings.szDllLanguage)));
	profile.IOProfileData(pSection, L"bDefaultSelect", settings.bDefaultSelect);

	if (bWrite) {
		if (!profile.WriteProfile(strIniName.c_str(), L"Sakura Profile ini")) {
			return false;
		}
	}
	return true;
}


bool DlgProfileMgr::ReadProfSettings(ProfileSettings& settings)
{
	auto_strcpy(settings.szDllLanguage, _T(""));
	settings.nDefaultIndex = 0;
	settings.profList.clear();
	settings.bDefaultSelect = false;

	return IOProfSettings( settings, false );
}


bool DlgProfileMgr::WriteProfSettings(ProfileSettings& settings)
{
	return IOProfSettings( settings, true );
}


LPVOID DlgProfileMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

