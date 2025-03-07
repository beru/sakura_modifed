// タイプ別設定 - ウィンドウ

#include "StdAfx.h"
#include "PropTypes.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"
#include "DlgSameColor.h"
#include "DlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "charset/CodePage.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids2[] = {	//11400
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,				// 文書アイコンを使う

	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,			// IMEのON/OFF状態
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,			// IMEの入力モード

	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,	// デフォルト文字コード
	IDC_CHECK_CP,					HIDC_CHECK_TYPE_SUPPORT_CP,		// コードページ
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,		// デフォルト改行コード
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,			// デフォルトBOM
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,			// 自動判別時にCESU-8を優先する

	IDC_EDIT_BACKIMG_PATH,			HIDC_EDIT_BACKIMG_PATH,			// 背景画像
	IDC_BUTTON_BACKIMG_PATH_SEL,	HIDC_BUTTON_BACKIMG_PATH_SEL,	// 背景画像ボタン
	IDC_COMBO_BACKIMG_POS,			HIDC_COMBO_BACKIMG_POS,			// 背景画像位置
	IDC_CHECK_BACKIMG_SCR_X,		HIDC_CHECK_BACKIMG_SCR_X,		// 背景画像ScrollX
	IDC_CHECK_BACKIMG_SCR_Y,		HIDC_CHECK_BACKIMG_SCR_Y,		// 背景画像ScrollY
	IDC_CHECK_BACKIMG_REP_X,		HIDC_CHECK_BACKIMG_REP_X,		// 背景画像RepeatX
	IDC_CHECK_BACKIMG_REP_Y,		HIDC_CHECK_BACKIMG_REP_Y,		// 背景画像RepeatY
	IDC_EDIT_BACKIMG_OFFSET_X,		HIDC_EDIT_BACKIMG_OFFSET_X,		// 背景画像OffsetX
	IDC_EDIT_BACKIMG_OFFSET_Y,		HIDC_EDIT_BACKIMG_OFFSET_Y,		// 背景画像OffsetY
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		// 行番号の表示（折り返し単位）
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		// 行番号の表示（改行単位）
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		// 行番号区切り（なし）
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		// 行番号区切り（縦線）
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		// 行番号区切り（任意）
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			// 行番号区切り
	IDC_EDIT_LINENUMWIDTH,			HIDC_EDIT_LINENUMWIDTH,			// 行番号の最小桁数
//	IDC_STATIC,						-1,
	0, 0
};



TYPE_NAME_ID<int> ImeSwitchArr[] = {
	{ 0, STR_IME_SWITCH_DONTSET },
	{ 1, STR_IME_SWITCH_ON },
	{ 2, STR_IME_SWITCH_OFF },
};

TYPE_NAME_ID<int> ImeStateArr[] = {
	{ 0, STR_IME_STATE_DEF },
	{ 1, STR_IME_STATE_FULL },
	{ 2, STR_IME_STATE_FULLHIRA },
	{ 3, STR_IME_STATE_FULLKATA },
	{ 4, STR_IME_STATE_NO }
};

static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
	L"NEL",
	L"LS",
	L"PS",
};
static const EolType aeEolType[] = {
	EolType::CRLF,
	EolType::LF,
	EolType::CR,
	EolType::NEL,
	EolType::LS,
	EolType::PS,
};

// window メッセージ処理
INT_PTR PropTypesWindow::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD	wNotifyCode;
	WORD	wID;
	NMHDR*	pNMHDR;
	NM_UPDOWN* pMNUD;

	switch (uMsg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ダイアログデータの設定 color
		SetData(hwndDlg);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), 1);

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID

		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			{
				int i;
				switch (wID) {
				case IDC_COMBO_DEFAULT_CODETYPE:
					// 文字コードの変更をBOMチェックボックスに反映
					i = Combo_GetCurSel((HWND) lParam);
					if (i != CB_ERR) {
						CodeTypeName	codeTypeName(Combo_GetItemData((HWND)lParam, i));
						::CheckDlgButton(hwndDlg, IDC_CHECK_DEFAULT_BOM, (codeTypeName.IsBomDefOn() ?  BST_CHECKED : BST_UNCHECKED));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DEFAULT_BOM), codeTypeName.UseBom());
					}
					break;
				}
			}
			break;

		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_BACKIMG_PATH_SEL:
				{
					Dialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH),
						_T("*.bmp;*.jpg;*.jpeg"), true);
				}
				return TRUE;
			//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
			case IDC_RADIO_LINETERMTYPE0: // 行番号区切り 0=なし 1=縦線 2=任意
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
				EnableTypesPropInput(hwndDlg);
				return TRUE;
			}
			case IDC_CHECK_CP:
				{
					::CheckDlgButton(hwndDlg, IDC_CHECK_CP, TRUE);
					::EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_CP), FALSE);
					CodePage::AddComboCodePages(hwndDlg, ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE), -1);
				}
				return TRUE;
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_WINDOW);
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE(_T("color PSN_KILLACTIVE\n"));
			// ダイアログデータの取得 window
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_WINDOW;
			return TRUE;
		}

		pMNUD  = (NM_UPDOWN*)lParam;
		switch ((int)wParam) {
		case IDC_SPIN_LINENUMWIDTH:
			// 行番号の最小桁数
//			MYTRACE( _T("IDC_SPIN_LINENUMWIDTH\n") );
			int nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < LINENUMWIDTH_MIN) {
				nVal = LINENUMWIDTH_MIN;
			}
			if (nVal > LINENUMWIDTH_MAX) {
				nVal = LINENUMWIDTH_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, nVal, FALSE);
			return TRUE;
		}

		break;	// WM_NOTIFY

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2);
		}
		return TRUE;
		// NOTREACHED
//		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2);
		return TRUE;

	}
	return FALSE;
}


void PropTypesWindow::SetCombobox(
	HWND hwndWork,
	const int* nIds,
	int nCount,
	int select
	)
{
	Combo_ResetContent(hwndWork);
	for (int i=0; i<nCount; ++i) {
		Combo_AddString(hwndWork, LS(nIds[i]));
	}
	Combo_SetCurSel(hwndWork, select);
}


// ダイアログデータの設定 window
void PropTypesWindow::SetData(HWND hwndDlg)
{
	{
		// 文書アイコンを使う
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_DOCICON, types.bUseDocumentIcon);
	}

	// 起動時のIME(日本語入力変換)
	{
		int ime;
		// ON/OFF状態
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESWITCH);
		Combo_ResetContent(hwndCombo);
		ime = types.nImeState & 3;
		int nSelPos = 0;
		for (size_t i=0; i<_countof(ImeSwitchArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(ImeSwitchArr[i].nNameId));
			if (ImeSwitchArr[i].nMethod == ime) {	// IME状態
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// 入力モード
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESTATE);
		Combo_ResetContent(hwndCombo);
		ime = types.nImeState >> 2;
		nSelPos = 0;
		for (size_t i=0; i<_countof(ImeStateArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(ImeStateArr[i].nNameId));
			if (ImeStateArr[i].nMethod == ime) {	// IME状態
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);
	}

	// 「文字コード」グループの設定
	{
		HWND hCombo;

		// 「自動認識時にCESU-8を優先」types.encoding.bPriorCesu8 をチェック
		::CheckDlgButton(hwndDlg, IDC_CHECK_PRIOR_CESU8, types.encoding.bPriorCesu8);

		// デフォルトコードタイプのコンボボックス設定
		size_t nSel= 0;
		size_t j = 0;
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE);
		CodeTypesForCombobox codeTypes;
		for (size_t i=0; i<codeTypes.GetCount(); ++i) {
			if (CodeTypeName(codeTypes.GetCode(i)).CanDefault()) {
				int idx = Combo_AddString(hCombo, codeTypes.GetName(i));
				Combo_SetItemData(hCombo, idx, codeTypes.GetCode(i));
				if (types.encoding.eDefaultCodetype == codeTypes.GetCode(i)) {
					nSel = j;
				}
				++j;
			}
		}
		if (nSel == -1) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_CP, TRUE);
			::EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_CP), FALSE);
			int nIdx = CodePage::AddComboCodePages(hwndDlg, hCombo, types.encoding.eDefaultCodetype);
			if (nIdx == -1) {
				nSel = 0;
			}else {
				nSel = nIdx;
			}
		}
		Combo_SetCurSel(hCombo, nSel);

		// BOM チェックボックス設定
		CodeTypeName	cCodeTypeName(types.encoding.eDefaultCodetype);
		if (!cCodeTypeName.UseBom()) {
			types.encoding.bDefaultBom = false;
		}
		::CheckDlgButton(hwndDlg, IDC_CHECK_DEFAULT_BOM, (types.encoding.bDefaultBom ? BST_CHECKED : BST_UNCHECKED));
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DEFAULT_BOM), (int)cCodeTypeName.UseBom());

		// デフォルト改行タイプのコンボボックス設定
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE);
		for (size_t i=0; i<_countof(aszEolStr); ++i) {
			ApiWrap::Combo_AddString(hCombo, aszEolStr[i]);
		}
		size_t i;
		for (i=0; i<_countof(aeEolType); ++i) {
			if (types.encoding.eDefaultEoltype == aeEolType[i]) {
				break;
			}
		}
		if (i == _countof(aeEolType)) {
			i = 0;
		}
		Combo_SetCurSel(hCombo, i);
	}

	// 行番号の表示 false=折り返し単位／true=改行単位
	if (!types.bLineNumIsCRLF) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE);
	}

	{
		// 行番号の最小桁数	
		::SetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, types.nLineNumWidth, FALSE);
	}

	// 背景画像
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH), _countof2(types.szBackImgPath));
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X), 5);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y), 5);

	DlgItem_SetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, types.szBackImgPath);
	{
		static const int posNameId[] ={
			STR_IMAGE_POS1,
			STR_IMAGE_POS2,
			STR_IMAGE_POS3,
			STR_IMAGE_POS4,
			STR_IMAGE_POS5,
			STR_IMAGE_POS6,
			STR_IMAGE_POS7,
			STR_IMAGE_POS8,
			STR_IMAGE_POS9,
		};
		// BGIMAGE_TOP_LEFT ..
		int nCount = _countof(posNameId);
		SetCombobox(::GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS), posNameId, nCount, (int)types.backImgPos);
	}
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X, types.backImgRepeatX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y, types.backImgRepeatY);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X, types.backImgScrollX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y, types.backImgScrollY);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, types.backImgPosOffset.x, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, types.backImgPosOffset.y, TRUE);

	// 行番号区切り  0=なし 1=縦線 2=任意
	if (types.nLineTermType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE);
	}else
	if (types.nLineTermType == 1) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE);
	}else
	if (types.nLineTermType == 2) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE);
	}

	// 行番号区切り文字
	wchar_t	szLineTermChar[2];
	auto_sprintf_s(szLineTermChar, L"%lc", types.cLineTermChar);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar);

	//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
	EnableTypesPropInput(hwndDlg);

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ダイアログデータの取得 color
int PropTypesWindow::GetData(HWND hwndDlg)
{
	{
		// 文書アイコンを使う
		types.bUseDocumentIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DOCICON);
	}

	// 起動時のIME(日本語入力変換)
	{
		// 入力モード
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESTATE);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		types.nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME入力モード

		// ON/OFF状態
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESWITCH);
		nSelPos = Combo_GetCurSel(hwndCombo);
		types.nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	}

	// 「文字コード」グループの設定
	{
		// types.bPriorCesu8 を設定
		types.encoding.bPriorCesu8 = DlgButton_IsChecked(hwndDlg, IDC_CHECK_PRIOR_CESU8);

		// types.eDefaultCodetype を設定
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE);
		int i = Combo_GetCurSel(hCombo);
		if (i != CB_ERR) {
			types.encoding.eDefaultCodetype = EncodingType(Combo_GetItemData(hCombo, i));
		}

		// types.bDefaultBom を設定
		types.encoding.bDefaultBom = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DEFAULT_BOM);

		// types.eDefaultEoltype を設定
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE);
		i = Combo_GetCurSel(hCombo);
		if (i != CB_ERR) {
			types.encoding.eDefaultEoltype = aeEolType[i];
		}
	}

	// 行番号の表示 false=折り返し単位／true=改行単位
	types.bLineNumIsCRLF = !DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINENUM_LAYOUT);

	DlgItem_GetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, types.szBackImgPath, _countof2(types.szBackImgPath));
	types.backImgPos = static_cast<BackgroundImagePosType>(Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS)));
	types.backImgRepeatX = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_REP_X);
	types.backImgRepeatY = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_REP_Y);
	types.backImgScrollX = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_SCR_X);
	types.backImgScrollY = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y);
	types.backImgPosOffset.x = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, NULL, TRUE);
	types.backImgPosOffset.y = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, NULL, TRUE);

	// 行番号区切り  0=なし 1=縦線 2=任意
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE0)) {
		types.nLineTermType = 0;
	}else
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE1)) {
		types.nLineTermType = 1;
	}else
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE2)) {
		types.nLineTermType = 2;
	}
	
	// 行番号区切り文字
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText(hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2);
	types.cLineTermChar = szLineTermChar[0];

	// 行番号の最小桁数
	types.nLineNumWidth = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE);
	if (types.nLineNumWidth < LINENUMWIDTH_MIN) {
		types.nLineNumWidth = LINENUMWIDTH_MIN;
	}
	if (types.nLineNumWidth > LINENUMWIDTH_MAX) {
		types.nLineNumWidth = LINENUMWIDTH_MAX;
	}

	return TRUE;
}


//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void PropTypesWindow::EnableTypesPropInput(HWND hwndDlg)
{
	//	行番号区切りを任意の半角文字にするかどうか
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE2)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LINETERMCHAR), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LINETERMCHAR), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), FALSE);
	}
}

