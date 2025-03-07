// タイプ別設定インポート確認ダイアログ

#include "StdAfx.h"
#include "DlgTypeAscertain.h"
#include "env/DocTypeManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura.hh"
#include "sakura_rc.h"

// タイプ別設定インポート確認 CDlgTypeAscertain.cpp
const DWORD p_helpids[] = {
	IDC_RADIO_TYPE_TO,		HIDC_RADIO_TYPE_TO,		// タイプ別名
	IDC_RADIO_TYPE_ADD,		HIDC_RADIO_TYPE_ADD,	// タイプ別追加
	IDC_COMBO_COLORS,		HIDC_COMBO_COLORS,		// 色指定
	IDOK,					HIDOK_DTA,				// OK
	IDCANCEL,				HIDCANCEL_DTA,			// キャンセル
	IDC_BUTTON_HELP,		HIDC_DTA_BUTTON_HELP,	// ヘルプ
//	IDC_STATIC,				-1,
	0, 0
};

// Constructors
DlgTypeAscertain::DlgTypeAscertain()
	:
	psi(nullptr)
{
}

// モーダルダイアログの表示
INT_PTR DlgTypeAscertain::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	AscertainInfo* psAscertainInfo
	)
{
	psi = psAscertainInfo;

	psi->nColorType = -1;

	return Dialog::DoModal(hInstance, hwndParent, IDD_TYPE_ASCERTAIN, (LPARAM)NULL);
}

// ボタンクリック
BOOL DlgTypeAscertain::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「タイプ別設定インポート」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000338);
		return TRUE;
	case IDOK:
		TCHAR	buff1[_MAX_PATH + 20];
		wchar_t	buff2[_MAX_PATH + 20];

		psi->bAddType = IsButtonChecked(IDC_RADIO_TYPE_ADD);
		psi->sColorFile = L"";
		psi->nColorType = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_COLORS)) - 1;
		if (psi->nColorType >= MAX_TYPES
			&& Combo_GetLBText(GetItemHwnd(IDC_COMBO_COLORS), psi->nColorType + 1, buff1)
		) {
			if (_stscanf(buff1, _T("File -- %ls"), buff2) > 0) {
				psi->sColorFile = buff2;
				psi->nColorType = MAX_TYPES;
			}
		}
		::EndDialog(GetHwnd(), TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


// ダイアログデータの設定
void DlgTypeAscertain::SetData(void)
{
	// タイプ名設定
	std::wstring typeNameTo = psi->sTypeNameTo + L"(&B)";
	SetItemText(IDC_RADIO_TYPE_TO, typeNameTo.c_str());
	SetItemText(IDC_STATIC_TYPE_FILE, psi->sTypeNameFile.c_str());

	CheckButton(IDC_RADIO_TYPE_ADD, true);

	TCHAR szText[130];
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_COLORS);
	// コンボボックスを空にする
	Combo_ResetContent(hwndCombo);
	// 一行目はそのまま
	Combo_AddString(hwndCombo, LSW(STR_DLGTYPEASC_IMPORT));

	// エディタ内の設定
	for (int nIdx=0; nIdx<GetDllShareData().nTypesCount; ++nIdx) {
		const TypeConfigMini* type;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(nIdx), &type);
		if (type->szTypeExts[0] != _T('\0')) {		// タイプ属性：拡張子リスト
			auto_sprintf_s(szText, _T("%ts (%ts)"),
				type->szTypeName,	// タイプ属性：名称
				type->szTypeExts	// タイプ属性：拡張子リスト
			);
		}else {
			auto_sprintf_s(szText, _T("%ts"),
				type->szTypeName	// タイプ属性：拡称
			);
		}
		::Combo_AddString(hwndCombo, szText);
	}
	// 読込色設定ファイル設定
	HANDLE	hFind;
	WIN32_FIND_DATA	wf;
	BOOL	bFind;
	TCHAR	sTrgCol[_MAX_PATH + 1];

	::SplitPath_FolderAndFile(psi->sImportFile.c_str(), sTrgCol, NULL);
	_tcscat(sTrgCol, _T("\\*.col"));
	for (bFind = ((hFind = FindFirstFile(sTrgCol, &wf)) != INVALID_HANDLE_VALUE);
		bFind;
		bFind = FindNextFile(hFind, &wf)
	) {
		if ((wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			// 読込色設定ファイル発見
			auto_sprintf_s(szText, _T("File -- %ts"), wf.cFileName);
			::Combo_AddString(hwndCombo, szText);
		}
	}
	FindClose(hFind);

	// コンボボックスのデフォルト選択
	Combo_SetCurSel(hwndCombo, 0);
	return;
}

LPVOID DlgTypeAscertain::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

