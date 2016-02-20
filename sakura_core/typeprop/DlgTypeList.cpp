/*!	@file
	@brief ファイルタイプ一覧ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Uchi, Beta.Ito, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "types/Type.h" // use DlgTypeList定義
#include "window/EditWnd.h"
#include "typeprop/DlgTypeList.h"
#include "typeprop/ImpExpManager.h"	// 2010/4/24 Uchi
#include "env/ShareData.h"
#include "env/DocTypeManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/RegKey.h"
#include "util/string_ex2.h"
#include <memory>
#include "sakura_rc.h"
#include "sakura.hh"

typedef std::basic_string<TCHAR> tstring;

#define BUFFER_SIZE 1024
#define ACTION_NAME	(_T("SakuraEditor"))
#define PROGID_BACKUP_NAME	(_T("SakuraEditorBackup"))
#define ACTION_BACKUP_PATH	(_T("\\ShellBackup"))

// 関数プロトタイプ
int CopyRegistry(HKEY srcRoot, const tstring& srcPath, HKEY destRoot, const tstring& destPath);
int DeleteRegistry(HKEY root, const tstring& path);
int RegistExt(LPCTSTR sExt, bool bDefProg);
int UnregistExt(LPCTSTR sExt);
int CheckExt(LPCTSTR sExt, bool *pbRMenu, bool *pbDblClick);

// 内部使用定数
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// タイプ別設定一覧 CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	// 一時適用
	IDOK,					HIDOK_TL,					// 設定
	IDCANCEL,				HIDCANCEL_TL,				// キャンセル
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		// ヘルプ
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			// リスト
	IDC_BUTTON_IMPORT,		HIDC_TL_BUTTON_IMPORT,		// インポート
	IDC_BUTTON_EXPORT,		HIDC_TL_BUTTON_EXPORT,		// エクスポート
	IDC_BUTTON_INITIALIZE,	HIDC_TL_BUTTON_INIT,		// 初期化
	IDC_BUTTON_COPY_TYPE,	HIDC_BUTTON_COPY_TYPE,		// 複製
	IDC_BUTTON_UP_TYPE,		HIDC_BUTTON_UP_TYPE,		// ↑
	IDC_BUTTON_DOWN_TYPE,	HIDC_BUTTON_DOWN_TYPE,		// ↓
	IDC_BUTTON_ADD_TYPE,	HIDC_BUTTON_ADD_TYPE,		// 追加
	IDC_BUTTON_DEL_TYPE,	HIDC_BUTTON_DEL_TYPE,		// 削除
	IDC_CHECK_EXT_RMENU,	HIDC_TL_CHECK_RMENU,		// 右クリックメニューに追加
	IDC_CHECK_EXT_DBLCLICK,	HIDC_TL_CHECK_DBLCLICK,		// ダブルクリックで開く
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

// モーダルダイアログの表示
int DlgTypeList::DoModal(HINSTANCE hInstance, HWND hwndParent, Result* psResult)
{
	int	nRet;
	m_nSettingType = psResult->documentType;
	m_bAlertFileAssociation = true;
	m_bEnableTempChange = psResult->bTempChange;
	nRet = (int)Dialog::DoModal(hInstance, hwndParent, IDD_TYPELIST, (LPARAM)NULL);
	if (nRet == -1) {
		return FALSE;
	}else {
		// 結果
		psResult->documentType = TypeConfigNum(nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
}


BOOL DlgTypeList::OnLbnDblclk(int wID)
{
	switch (wID) {
	case IDC_LIST_TYPES:
		// Nov. 29, 2000	genta
		// 動作変更: 指定タイプの設定ダイアログ→一時的に別の設定を適用
		::EndDialog(
			GetHwnd(),
			List_GetCurSel(GetItemHwnd(IDC_LIST_TYPES))
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	}
	return FALSE;
}

BOOL DlgTypeList::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「タイプ別設定一覧」のヘルプ
		// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	// Nov. 29, 2000	From Here	genta
	// 適用する型の一時的変更
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			GetHwnd(),
 			List_GetCurSel(GetItemHwnd(IDC_LIST_TYPES))
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	// Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog(GetHwnd(), List_GetCurSel(GetItemHwnd(IDC_LIST_TYPES)));
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), -1);
		return TRUE;
	case IDC_BUTTON_IMPORT:
		Import();
		return TRUE;
	case IDC_BUTTON_EXPORT:
		Export();
		return TRUE;
	case IDC_BUTTON_INITIALIZE:
		InitializeType();
		return TRUE;
	case IDC_BUTTON_COPY_TYPE:
		CopyType();
		return TRUE;
	case IDC_BUTTON_UP_TYPE:
		UpType();
		return TRUE;
	case IDC_BUTTON_DOWN_TYPE:
		DownType();
		return TRUE;
	case IDC_BUTTON_ADD_TYPE:
		AddType();
		return TRUE;
	case IDC_BUTTON_DEL_TYPE:
		DelType();
		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);

}


BOOL DlgTypeList::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		SetData(-1);
		return TRUE;

	case WA_INACTIVE:
	default:
		break;
	}

	// 基底クラスメンバ
	return Dialog::OnActivate(wParam, lParam);
}


INT_PTR DlgTypeList::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndRMenu = GetItemHwnd(IDC_CHECK_EXT_RMENU);
	HWND hwndDblClick = GetItemHwnd(IDC_CHECK_EXT_DBLCLICK);

	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	if (wMsg == WM_COMMAND) {
		HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
		int nIdx = List_GetCurSel(hwndList);
		const TypeConfigMini* type = NULL;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(nIdx), &type);
		if (LOWORD(wParam) == IDC_LIST_TYPES) {
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				EnableItem(IDC_BUTTON_INITIALIZE, nIdx != 0);
				EnableItem(IDC_BUTTON_UP_TYPE, 1 < nIdx);
				EnableItem(IDC_BUTTON_DOWN_TYPE, nIdx != 0 && nIdx < GetDllShareData().nTypesCount - 1);
				EnableItem(IDC_BUTTON_DEL_TYPE, nIdx != 0);
				if (type->szTypeExts[0] == '\0') {
					::EnableWindow(hwndRMenu, FALSE);
					::EnableWindow(hwndDblClick, FALSE);
				}else {
					EnableItem(IDC_CHECK_EXT_RMENU, true);
					if (!m_bRegistryChecked[nIdx]) {
						TCHAR exts[_countof(type->szTypeExts)] = {0};
						_tcscpy(exts, type->szTypeExts);
						TCHAR *ext = _tcstok( exts, DocTypeManager::m_typeExtSeps );

						m_bExtRMenu[nIdx] = true;
						m_bExtDblClick[nIdx] = true;
						while (ext) {
							if (!_tcspbrk(ext, DocTypeManager::m_typeExtWildcards)) {
								bool bRMenu;
								bool bDblClick;
								CheckExt(ext, &bRMenu, &bDblClick);
								m_bExtRMenu[nIdx] &= bRMenu;
								m_bExtDblClick[nIdx] &= bDblClick;
							}
							ext = _tcstok( NULL, DocTypeManager::m_typeExtSeps );
						}
						m_bRegistryChecked[nIdx] = true;
					}
					BtnCtl_SetCheck(hwndRMenu, m_bExtRMenu[nIdx]);
					::EnableWindow(hwndDblClick, m_bExtRMenu[nIdx]);
					BtnCtl_SetCheck(hwndDblClick, m_bExtDblClick[nIdx]);
				}
				return TRUE;
			}
		}else if (LOWORD(wParam) == IDC_CHECK_EXT_RMENU && HIWORD(wParam) == BN_CLICKED) {
			bool checked = (BtnCtl_GetCheck(hwndRMenu) == TRUE);
			if (!AlertFileAssociation()) {		// レジストリ変更確認
				BtnCtl_SetCheck(hwndRMenu, !checked);
				return result;
			}
			TCHAR exts[_countof(type->szTypeExts)] = {0};
			_tcscpy(exts, type->szTypeExts);
			TCHAR *ext = _tcstok( exts, DocTypeManager::m_typeExtSeps );
			int nRet;
			while (ext) {
				if (_tcspbrk(ext, DocTypeManager::m_typeExtWildcards) == NULL) {
					if (checked) {	//「右クリック」チェックON
						if ((nRet = RegistExt( ext, true )) != 0 ) {
	
							TCHAR buf[BUFFER_SIZE] = {0};
							::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL); 
							::MessageBox(GetHwnd(), (tstring(LS(STR_DLGTYPELIST_ERR1)) + buf).c_str(), GSTR_APPNAME, MB_OK);
							break;
						}
					}else {			//「右クリック」チェックOFF
						if ((nRet = UnregistExt(ext)) != 0) {
	
							TCHAR buf[BUFFER_SIZE] = {0};
							::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL); 
							::MessageBox(GetHwnd(), (tstring(LS(STR_DLGTYPELIST_ERR2)) + buf).c_str(), GSTR_APPNAME, MB_OK);
							break;
						}
					}
				}
				ext = _tcstok( NULL, DocTypeManager::m_typeExtSeps );
			}
			m_bExtRMenu[nIdx] = checked;
			::EnableWindow(hwndDblClick, checked);
			m_bExtDblClick[nIdx] = checked;
			BtnCtl_SetCheck(hwndDblClick, checked);
			return TRUE;
		}else if (LOWORD(wParam) == IDC_CHECK_EXT_DBLCLICK && HIWORD(wParam) == BN_CLICKED) {
			bool checked = (BtnCtl_GetCheck(hwndDblClick) == TRUE);


			if (!AlertFileAssociation()) {		// レジストリ変更確認
				BtnCtl_SetCheck(hwndDblClick, !checked);
				return result;
			}
			TCHAR exts[_countof(type->szTypeExts)] = {0};
			_tcscpy(exts, type->szTypeExts);
			TCHAR *ext = _tcstok( exts, DocTypeManager::m_typeExtSeps );
			int nRet;
			while (ext) {
				if (_tcspbrk(ext, DocTypeManager::m_typeExtWildcards) == NULL) {
					if ((nRet = RegistExt( ext, checked )) != 0) {

						TCHAR buf[BUFFER_SIZE] = {0};
						::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL); 
						::MessageBox(GetHwnd(), (tstring(LS(STR_DLGTYPELIST_ERR1)) + buf).c_str(), GSTR_APPNAME, MB_OK);
						break;
					}
				}
				ext = _tcstok( NULL, DocTypeManager::m_typeExtSeps );
			}
			m_bExtDblClick[nIdx] = checked;
			return TRUE;
		}
	}
	return result;
}


// ダイアログデータの設定
void DlgTypeList::SetData(void)
{
	SetData(m_nSettingType.GetIndex());
}

void DlgTypeList::SetData(int selIdx)
{
	int		nIdx;
	TCHAR	szText[64 + MAX_TYPES_EXTS + 10];
	int		nExtent = 0;
	HWND	hwndList = GetItemHwnd(IDC_LIST_TYPES);
	HDC		hDC = ::GetDC(hwndList);
	HFONT	hFont = (HFONT)::SendMessage(hwndList, WM_GETFONT, 0, 0);
	HFONT	hFontOld = (HFONT)::SelectObject(hDC, hFont);

	if (selIdx == -1) {
		selIdx = List_GetCurSel(hwndList);
		if (selIdx == -1) {
			selIdx = 0;
		}
	}
	if (GetDllShareData().nTypesCount <= selIdx) {
		selIdx = GetDllShareData().nTypesCount - 1;
	}
	List_ResetContent(hwndList);	// リストを空にする
	for (nIdx=0; nIdx<GetDllShareData().nTypesCount; ++nIdx) {
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
		::List_AddString(hwndList, szText);
		m_bRegistryChecked[nIdx] = FALSE;
		m_bExtRMenu[nIdx] = FALSE;
		m_bExtDblClick[nIdx] = FALSE;

		SIZE sizeExtent;
		if (::GetTextExtentPoint32(hDC, szText, _tcslen(szText), &sizeExtent) && sizeExtent.cx > nExtent) {
			nExtent = sizeExtent.cx;
		}
	}
	for (nIdx; nIdx<MAX_TYPES; ++nIdx) {
		m_bRegistryChecked[nIdx] = FALSE;
		m_bExtRMenu[nIdx] = FALSE;
		m_bExtDblClick[nIdx] = FALSE;
	}
	::SelectObject(hDC, hFontOld);
	::ReleaseDC(hwndList, hDC);
	List_SetHorizontalExtent(hwndList, nExtent + 8);
	if (GetDllShareData().nTypesCount <= selIdx) {
		selIdx = GetDllShareData().nTypesCount - 1;
	}
	List_SetCurSel(hwndList, selIdx);

	::SendMessage(GetHwnd(), WM_COMMAND, MAKEWPARAM(IDC_LIST_TYPES, LBN_SELCHANGE), 0);
	EnableItem(IDC_BUTTON_TEMPCHANGE, m_bEnableTempChange);
	EnableItem(IDC_BUTTON_COPY_TYPE, GetDllShareData().nTypesCount < MAX_TYPES);
	EnableItem(IDC_BUTTON_ADD_TYPE, GetDllShareData().nTypesCount < MAX_TYPES);
	return;
}

//@@@ 2002.01.18 add start
LPVOID DlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

static void SendChangeSetting()
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		MYWM_CHANGESETTING,
		(WPARAM)0,
		(LPARAM)PM_CHANGESETTING_ALL,
		EditWnd::getInstance()->GetHwnd()
	);
}

static void SendChangeSettingType(int nType)
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		MYWM_CHANGESETTING,
		(WPARAM)nType,
		(LPARAM)PM_CHANGESETTING_TYPE,
		EditWnd::getInstance()->GetHwnd()
	);
}

static void SendChangeSettingType2(int nType)
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		MYWM_CHANGESETTING,
		(WPARAM)nType,
		(LPARAM)PM_CHANGESETTING_TYPE2,
		EditWnd::getInstance()->GetHwnd()
	);
}

// タイプ別設定インポート
// 2010/4/12 Uchi
bool DlgTypeList::Import()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int nIdx = List_GetCurSel(hwndList);
	TypeConfig type;
	// ベースのデータは基本
	DocTypeManager().GetTypeConfig(TypeConfigNum(0), type);

	ImpExpType	cImpExpType(nIdx, type, hwndList);
	const TypeConfigMini* typeMini;
	DocTypeManager().GetTypeConfigMini(TypeConfigNum(nIdx), &typeMini);
	int id = typeMini->id;

	// インポート
	cImpExpType.SetBaseName(to_wchar(type.szTypeName));
	if (!cImpExpType.ImportUI(G_AppInstance(), GetHwnd())) {
		// インポートをしていない
		return false;
	}
	bool bAdd = cImpExpType.IsAddType();
	if (bAdd) {
		AddType();
		nIdx = GetDllShareData().nTypesCount - 1;
		type.nIdx = nIdx;
	}else {
		// UIを表示している間にずれているかもしれないのでindex再取得
		nIdx = DocTypeManager().GetDocumentTypeOfId(id).GetIndex();
		if (nIdx == -1) {
			return false;
		}
		type.nIdx = nIdx;
	}
	type.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
	// 適用
	DocTypeManager().SetTypeConfig(TypeConfigNum(nIdx), type);
	if (!bAdd) {
		SendChangeSettingType(nIdx);
	}

	// リスト再初期化
	SetData(nIdx);

	return true;
}

// タイプ別設定エクスポート
// 2010/4/12 Uchi
bool DlgTypeList::Export()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int nIdx = List_GetCurSel(hwndList);
	TypeConfig types;
	DocTypeManager().GetTypeConfig(TypeConfigNum(nIdx), types);

	ImpExpType	cImpExpType(nIdx, types, hwndList);

	// エクスポート
	cImpExpType.SetBaseName(to_wchar(types.szTypeName));
	if (!cImpExpType.ExportUI(G_AppInstance(), GetHwnd())) {
		// エクスポートをしていない
		return false;
	}

	return true;
}

// タイプ別設定初期化
// 2010/4/12 Uchi
bool DlgTypeList::InitializeType(void)
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int iDocType = List_GetCurSel(hwndList);
	if (iDocType == 0) {
		// 基本の場合には何もしない
		return true;
	}
	const TypeConfigMini* typeMini;
	DocTypeManager().GetTypeConfigMini(TypeConfigNum(iDocType), &typeMini);
	int nRet;
	if (typeMini->szTypeExts[0] != _T('\0')) { 
		nRet = ::MYMESSAGEBOX(
			GetHwnd(),
			MB_YESNO | MB_ICONQUESTION,
			GSTR_APPNAME,
			LS(STR_DLGTYPELIST_INIT1),
			typeMini->szTypeName);
		if (nRet != IDYES) {
			return false;
		}
	}

	iDocType = DocTypeManager().GetDocumentTypeOfId(typeMini->id).GetIndex();
	if (iDocType == -1) {
		return false;
	}
//	_DefaultConfig(&types);		// 規定値をコピー
	TypeConfig type;
	DocTypeManager().GetTypeConfig(TypeConfigNum(0), type); 	// 基本をコピー

	// 同じ名前にならないように数字をつける
	int nNameNum = iDocType + 1;
	bool bUpdate = true;
	for (int i=1; i<GetDllShareData().nTypesCount; ++i) {
		if (bUpdate) {
			auto_sprintf_s(type.szTypeName, LS(STR_DLGTYPELIST_SETNAME), nNameNum);
			++nNameNum;
			bUpdate = false;
		}
		if (i == iDocType) {
			continue;
		}
		const TypeConfigMini* typeMini2;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(i), &typeMini2);
		if (auto_strcmp(typeMini2->szTypeName, type.szTypeName) == 0) {
			i = 0;
			bUpdate = true;
		}
	}
	type.szTypeExts[0] = 0;
	type.nIdx = iDocType;
	type.id = (::GetTickCount() & 0x3fffffff) + iDocType * 0x10000;
	type.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();

	DocTypeManager().SetTypeConfig(TypeConfigNum(iDocType), type);

	SendChangeSettingType(iDocType);

	// リスト再初期化
	SetData(iDocType);

	InfoMessage(hwndDlg, LS(STR_DLGTYPELIST_INIT2), type.szTypeName);

	return true;
}

bool DlgTypeList::CopyType()
{
	int nNewTypeIndex = GetDllShareData().nTypesCount;
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_TYPES);
	int iDocType = List_GetCurSel(hwndList);
	TypeConfig type;
	DocTypeManager().GetTypeConfig(TypeConfigNum(iDocType), type);
	// 名前に2等を付ける
	int n = 1;
	bool bUpdate = true;
	for (int i=0; i<nNewTypeIndex; ++i) {
		if (bUpdate) {
			TCHAR* p = NULL;
			for (int k=(int)auto_strlen(type.szTypeName)-1; 0<=k; --k) {
				if (WCODE::Is09(type.szTypeName[k])) {
					p = &type.szTypeName[k];
				}else {
					break;
				}
			}
			if (p) {
				n = _ttoi(p) + 1;
				*p = _T('\0');
			}else {
				++n;
			}
			TCHAR szNum[12];
			auto_sprintf( szNum, _T("%d"), n );
			int nLen = auto_strlen( szNum );
			TCHAR szTemp[_countof(type.szTypeName) + 12];
			auto_strcpy( szTemp, type.szTypeName );
			int nTempLen = auto_strlen( szTemp );
			NativeT mem;
			// バッファをはみ出さないように
			LimitStringLengthT( szTemp, nTempLen, _countof(type.szTypeName) - nLen - 1, mem );
			auto_strcpy( type.szTypeName, mem.GetStringPtr() );
			auto_strcat(type.szTypeName, szNum);
			bUpdate = false;
		}
		const TypeConfigMini* typeMini;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(i), &typeMini);
		if (auto_strcmp(typeMini->szTypeName, type.szTypeName) == 0) {
			i = -1;
			bUpdate = true;
		}
	}
	if (!DocTypeManager().AddTypeConfig(TypeConfigNum(nNewTypeIndex))) {
		return false;
	}
	type.id = (::GetTickCount() & 0x3fffffff) + nNewTypeIndex * 0x10000;
	type.nIdx = nNewTypeIndex;
	type.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
	DocTypeManager().SetTypeConfig(TypeConfigNum(nNewTypeIndex), type);
	SetData(nNewTypeIndex);
	return true;
}

bool DlgTypeList::UpType()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int iDocType = List_GetCurSel(hwndList);
	if (iDocType == 0) {
		// 基本の場合には何もしない
		return true;
	}
	auto type1 = std::make_unique<TypeConfig>();
	auto type2 = std::make_unique<TypeConfig>();
	DocTypeManager().GetTypeConfig(TypeConfigNum(iDocType), *type1);
	DocTypeManager().GetTypeConfig(TypeConfigNum(iDocType - 1), *type2);
	--(type1->nIdx);
	++(type2->nIdx);
	DocTypeManager().SetTypeConfig(TypeConfigNum(iDocType), *type2);
	DocTypeManager().SetTypeConfig(TypeConfigNum(iDocType - 1), *type1);
	SendChangeSettingType2(iDocType);
	SendChangeSettingType2(iDocType - 1);
	SetData(iDocType - 1);
	return true;
}

bool DlgTypeList::DownType()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int iDocType = List_GetCurSel(hwndList);
	if (iDocType == 0 || GetDllShareData().nTypesCount <= iDocType + 1) {
		// 基本、最後の場合には何もしない
		return true;
	}
	auto type1 = std::make_unique<TypeConfig>();
	auto type2 = std::make_unique<TypeConfig>();
	DocTypeManager().GetTypeConfig(TypeConfigNum(iDocType), *type1);
	DocTypeManager().GetTypeConfig(TypeConfigNum(iDocType + 1), *type2);
	++(type1->nIdx);
	--(type2->nIdx);
	DocTypeManager().SetTypeConfig(TypeConfigNum(iDocType), *type2);
	DocTypeManager().SetTypeConfig(TypeConfigNum(iDocType + 1), *type1);
	SendChangeSettingType2(iDocType);
	SendChangeSettingType2(iDocType + 1);
	SetData(iDocType + 1);
	return true;
}

bool DlgTypeList::AddType()
{
	int nNewTypeIndex = GetDllShareData().nTypesCount;
	if (!DocTypeManager().AddTypeConfig(TypeConfigNum(nNewTypeIndex))) {
		return false;
	}
	SetData(nNewTypeIndex);
	return true;
}

bool DlgTypeList::DelType()
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetItemHwnd(IDC_LIST_TYPES);
	int iDocType = List_GetCurSel(hwndList);
	if (iDocType == 0) {
		// 基本の場合には何もしない
		return true;
	}
	const TypeConfigMini* typeMini;
	if (!DocTypeManager().GetTypeConfigMini(TypeConfigNum(iDocType), &typeMini)) {
		// 謎のエラー
		return false;
	}
	const TypeConfigMini type = *typeMini; // ダイアログを出している間に変更されるかもしれないのでコピーする
	int nRet = ConfirmMessage(hwndDlg,
		LS(STR_DLGTYPELIST_DEL), type.szTypeName);
	if (nRet != IDYES) {
		return false;
	}
	// ダイアログを出している間にタイプ別リストが更新されたかもしれないのでidから再検索
	TypeConfigNum config = DocTypeManager().GetDocumentTypeOfId(type.id);
	if (!config.IsValidType()) {
		return false;
	}
	iDocType = config.GetIndex();
	DocTypeManager().DelTypeConfig(config);
	if (GetDllShareData().nTypesCount <= iDocType) {
		iDocType = GetDllShareData().nTypesCount - 1;
	}
	SetData(iDocType);
	SendChangeSetting();
	return true;
}


// 再帰的レジストリコピー
int CopyRegistry(
	HKEY srcRoot,
	const tstring& srcPath,
	HKEY destRoot,
	const tstring& destPath
	)
{
	int errorCode;
	RegKey keySrc;
	if ((errorCode = keySrc.Open(srcRoot, srcPath.c_str(), KEY_READ)) != 0) {
		return errorCode;
	}

	RegKey keyDest;
	if ((errorCode = keyDest.Open(destRoot, destPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		if ((errorCode = keyDest.Create(destRoot, destPath.c_str())) != 0) {
			return errorCode;
		}
	}

	int index = 0;
	for (;;) {
		TCHAR szValue[BUFFER_SIZE] = {0};
		BYTE data[BUFFER_SIZE] = {0};
		DWORD dwDataLen;
		DWORD dwType;

		errorCode = keySrc.EnumValue(index, szValue, _countof(szValue), &dwType, data, _countof(data), &dwDataLen);
		if (errorCode == ERROR_NO_MORE_ITEMS) {
			errorCode = 0;
			break;
		}else if (errorCode) {
			return errorCode;
		}else {
			// 手抜き：データのサイズがBUFFER_SIZE(=1024)を超える場合を考慮していない
			if ((errorCode = keyDest.SetValue(szValue, data, dwDataLen, dwType)) != 0) {
				return errorCode;
			}
			++index;
		}
	}

	index = 0;
	TCHAR szSubKey[BUFFER_SIZE] = {0};
	for (;;) {
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if (errorCode == ERROR_NO_MORE_ITEMS) {
			errorCode = 0;
			break;
		}else if (errorCode) {
			return errorCode;
		}else {
			if ((errorCode = CopyRegistry(srcRoot, srcPath + _T("\\") + szSubKey, destRoot, destPath + _T("\\") + szSubKey))) {
				return errorCode;
			}
			++index;
		}
	}
	
	return errorCode;
}

// 再帰的レジストリ削除
int DeleteRegistry(HKEY root, const tstring& path)
{
	int errorCode;
	RegKey keySrc;
	if ((errorCode = keySrc.Open(root, path.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		return ERROR_SUCCESS;
	}

	int index = 0;
	index = 0;
	TCHAR szSubKey[BUFFER_SIZE] = {0};
	for (;;) {
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if (errorCode == ERROR_NO_MORE_ITEMS) {
			errorCode = 0;
			break;
		}else if (errorCode) {
			return errorCode;
		}else {
			if ((errorCode = DeleteRegistry(root, path + _T("\\") + szSubKey)) != 0) {
				return errorCode;
			}
		}
	}
	keySrc.Close();
	if ((errorCode = RegKey::DeleteKey(root, path.c_str())) != 0) {
		return errorCode;
	}
	return errorCode;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を行う
	@param sExt	拡張子
	@param bDefProg [in]既定フラグ（ダブルクリックで起動させるか）
	レジストリアクセス方針
	・管理者権限なしで実施したいため、HKLMは読み込みのみとし、書き込みはHKCUに行う。
	処理の流れ
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ
		・[HKCU\Software\Classes\.(拡張子)]を作成。値は「SakuraEditor_(拡張子)」
	・[HKCU\Software\Classes\.(拡張子)]の値が「SakuraEditor_(拡張子)」以外の場合、
		[HKCU\Software\Classes\.(拡張子)\SakuraEditorBackup]に値をコピーする
		値に「SakuraEditor_(拡張子)」を設定する
	・ProgID <- [HKCR\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)]の存在チェック
		存在しなければ
		・[HKLM\Software\Classes\(HKLMのProgID)]の存在チェック
		存在すれば
			・[HKLM\Software\Classes\(ProgID)]の構造を[HKCU\Software\Classes\(ProgID)]にコピーする
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\command]を作成。値は「"(サクラEXEパス)" "%1"」
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の値を「Sakura &Editor」とする
	・既定フラグ判定
		trueなら
			・[HKCU\Software\Classes\(ProgID)\shell]の値が空でなければ[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]に退避する
			・[HKCU\Software\Classes\(ProgID)\shell]の値を「SakuraEditor」とする
		falseなら
			・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]の存在チェック
				存在すれば、退避した値を[HKCU\Software\Classes\(ProgID)\shell]に設定
				存在しなければ、[HKCU\Software\Classes\(ProgID)\shell]の値を削除
*/
int RegistExt(LPCTSTR sExt, bool bDefProg)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring(_T("Software\\Classes\\"));

	// 小文字化
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, _countof(szLowerExt), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + _T("SakuraEditor_") + szLowerExt;

	RegKey keyExt_HKLM;
	TCHAR szProgID_HKLM[BUFFER_SIZE] = {0};
	if ((errorCode = keyExt_HKLM.Open(HKEY_LOCAL_MACHINE, sDotExt.c_str(), KEY_READ)) == 0) {
		keyExt_HKLM.GetValue(NULL, szProgID_HKLM, _countof(szProgID_HKLM));
	}

	RegKey keyExt;
	if ((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		if ((errorCode = keyExt.Create(HKEY_CURRENT_USER, sDotExt.c_str())) != 0) {
			return errorCode;
		}
	}

	TCHAR szProgID[BUFFER_SIZE] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if (_tcscmp(sGenProgID.c_str(), szProgID) != 0) {
		if (szProgID[0] != _T('\0')) {
			if ((errorCode = keyExt.SetValue(PROGID_BACKUP_NAME, szProgID)) != 0) {
				return errorCode;
			}
		} 
		if ((errorCode = keyExt.SetValue(NULL, sGenProgID.c_str())) != 0) {
			return errorCode;
		}
	}

	tstring sProgIDPath = sBasePath + sGenProgID;
	if (!RegKey::ExistsKey(HKEY_CURRENT_USER, sProgIDPath.c_str())) {
		if (szProgID_HKLM[0] != _T('\0')) {
			if ((errorCode = CopyRegistry(HKEY_LOCAL_MACHINE, (sBasePath + szProgID_HKLM).c_str(), HKEY_CURRENT_USER, sProgIDPath.c_str())) != 0) {
				return errorCode;
			}
		}
	}

	tstring sShellPath = sProgIDPath + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	tstring sShellActionCommandPath = sShellActionPath + _T("\\command");
	tstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	RegKey keyShellActionCommand;
	if ((errorCode = keyShellActionCommand.Open(HKEY_CURRENT_USER, sShellActionCommandPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		if ((errorCode = keyShellActionCommand.Create(HKEY_CURRENT_USER, sShellActionCommandPath.c_str())) != 0) {
			return errorCode;
		}
	}

	TCHAR sExePath[_MAX_PATH] = {0};
	::GetModuleFileName(NULL, sExePath, _countof(sExePath));
	tstring sCommandPathArg = tstring() + _T("\"") + sExePath + _T("\" \"%1\"");
	if ((errorCode = keyShellActionCommand.SetValue(NULL, sCommandPathArg.c_str())) != 0) {
		return errorCode;
	}

	RegKey keyShellAction;
	if ((errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) !=0) {
		return errorCode;
	}
	if ((errorCode = keyShellAction.SetValue(NULL, _T("Sakura &Editor"))) != 0) {
		return errorCode;
	}

	RegKey keyShell;
	if ((errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		return errorCode;
	}
	TCHAR szShellValue[BUFFER_SIZE] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if (bDefProg) {
		if (_tcscmp(szShellValue, ACTION_NAME) != 0) {
			if (szShellValue[0] != '\0') {
				RegKey keyBackup;
				if ((errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
					if ((errorCode = keyBackup.Create(HKEY_CURRENT_USER, sBackupPath.c_str())) != 0) {
						return errorCode;
					}
				}
				keyBackup.SetValue(NULL, szShellValue);
			}
			keyShell.SetValue(NULL, ACTION_NAME);
		}
	}else {
		RegKey keyBackup;
		if ((errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
			keyShell.DeleteValue(_T(""));
		}else {
			TCHAR sBackupValue[BUFFER_SIZE] = {0};
			keyBackup.GetValue(NULL, sBackupValue, _countof(sBackupValue));
			keyShell.SetValue(NULL, sBackupValue);
		}
	}

	return ERROR_SUCCESS;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を削除する
	@param sExt	[in]拡張子
	処理の流れ
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ終了
	・ProgID <- [HKCU\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の存在チェック
		存在しなければ終了
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]の存在チェック
		存在すれば、退避した値を[HKCU\Software\Classes\(ProgID)\shell]に設定
		存在しなければ、[HKCU\Software\Classes\(ProgID)\shell]の値を削除
	・ProgIDの先頭が"SakuraEditor_"か？
		そうなら[HKCU\Software\Classes\(ProgID)]と[HKCU\Software\Classes\.(拡張子)]を削除
	　　そうでなければ[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]を削除
*/
int UnregistExt(LPCTSTR sExt)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring(_T("Software\\Classes\\"));

	// 小文字化
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, _countof(szLowerExt), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + szLowerExt + _T("file");

	RegKey keyExt;
	if ((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		return errorCode;
	}

	TCHAR szProgID[BUFFER_SIZE] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if (szProgID[0] == _T('\0')) {
		return ERROR_SUCCESS;
	}

	tstring sProgIDPath = sBasePath + szProgID;
	tstring sShellPath = sProgIDPath + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	tstring sShellActionCommandPath = sShellActionPath + _T("\\command");
	tstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	RegKey keyShellAction;
	if ((errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		return ERROR_SUCCESS;
	}

	RegKey keyShell;
	if ((errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		return errorCode;
	}
	RegKey keyBackup;
	if ((errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0) {
		keyShell.DeleteValue(_T(""));
	}else {
		TCHAR szBackupValue[BUFFER_SIZE] = {0};
		keyBackup.GetValue(NULL, szBackupValue, _countof(szBackupValue));
		keyShell.SetValue(NULL, szBackupValue);
	}

	keyBackup.Close();
	keyShellAction.Close();
	if (_tcsncmp(szProgID, _T("SakuraEditor_"), 13) == 0) {
		if ((errorCode = DeleteRegistry(HKEY_CURRENT_USER, sProgIDPath)) != 0) {
			return errorCode;
		}

		TCHAR szBackupValue[BUFFER_SIZE] = {0};
		keyExt.GetValue(PROGID_BACKUP_NAME, szBackupValue, _countof(szBackupValue));
		if (szBackupValue[0] != _T('\0')) {
			keyExt.SetValue(NULL, szBackupValue);
		}else {
			if ((errorCode = DeleteRegistry(HKEY_CURRENT_USER, sDotExt)) != 0) {
				return errorCode;
			}
		}
	}else {
		if ((errorCode = DeleteRegistry(HKEY_CURRENT_USER, sShellActionPath)) != 0) {
			return errorCode;
		}
	}

	return ERROR_SUCCESS;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を確認する
	@param sExt			[in]拡張子
	@param pbRMenu		[out]関連付け設定
	@param pbDblClick	[out]既定設定
	処理の流れ
	・pbRMenu <- false, pbDblClick <- false
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ終了
	・ProgID <- [HKCU\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の存在チェック
		存在しなければ終了
	・pbRMenu <- true
	・[HKCU\Software\Classes\(ProgID)\shell]の値をチェック
		「SakuraEditor」なら、pbDblClick <- true
*/
int CheckExt(LPCTSTR sExt, bool* pbRMenu, bool* pbDblClick)
{
	int errorCode = ERROR_SUCCESS;
	tstring sBasePath = tstring(_T("Software\\Classes\\"));

	*pbRMenu = false;
	*pbDblClick = false;

	// 小文字化
	TCHAR szLowerExt[MAX_PATH] = {0};
	_tcsncpy_s(szLowerExt, _countof(szLowerExt), sExt, _tcslen(sExt));
	CharLower(szLowerExt);

	tstring sDotExt = sBasePath + _T(".") + szLowerExt;
	tstring sGenProgID = tstring() + szLowerExt + _T("file");

	RegKey keyExt;
	if ((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ)) != 0) {
		return ERROR_SUCCESS;
	}

	TCHAR szProgID[BUFFER_SIZE] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if (szProgID[0] == _T('\0')) {
		return ERROR_SUCCESS;
	}

	tstring sShellPath = tstring() + _T("Software\\Classes\\") + szProgID + _T("\\shell");
	tstring sShellActionPath = sShellPath + _T("\\") + ACTION_NAME;
	if (!RegKey::ExistsKey(HKEY_CURRENT_USER, sShellActionPath.c_str())) {
		return ERROR_SUCCESS;
	}
	*pbRMenu = true;

	RegKey keyShell;
	if ((errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ)) != 0) {
		return errorCode;
	}
	TCHAR szShellValue[BUFFER_SIZE] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if (_tcscmp(szShellValue, ACTION_NAME) == 0) {
		*pbDblClick = true;
	}

	return ERROR_SUCCESS;
}

/*!
	@brief レジストリ変更の警告メッセージを表示する
*/
bool DlgTypeList::AlertFileAssociation()
{
	if (m_bAlertFileAssociation) {
		if (::MYMESSAGEBOX(
				NULL, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_DLGTYPELIST_ACC)
			) == IDYES
		) {
			m_bAlertFileAssociation = false;	//「はい」なら最初の一度だけ確認する
			return true;
		}else {
			return false;
		}
	}
	return true;
}

