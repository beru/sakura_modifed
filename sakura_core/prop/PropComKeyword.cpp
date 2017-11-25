/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����L�[���[�h�v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/ShareData.h"
#include "env/DocTypeManager.h"
#include "typeprop/ImpExpManager.h"	// 20210/4/23 Uchi
#include "dlg/DlgInput1.h"
#include "util/shell.h"
#include <memory>
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				HIDC_BUTTON_ADDSET,			// �L�[���[�h�Z�b�g�ǉ�
	IDC_BUTTON_DELSET,				HIDC_BUTTON_DELSET,			// �L�[���[�h�Z�b�g�폜
	IDC_BUTTON_ADDKEYWORD,			HIDC_BUTTON_ADDKEYWORD,		// �L�[���[�h�ǉ�
	IDC_BUTTON_EDITKEYWORD,			HIDC_BUTTON_EDITKEYWORD,	// �L�[���[�h�ҏW
	IDC_BUTTON_DELKEYWORD,			HIDC_BUTTON_DELKEYWORD,		// �L�[���[�h�폜
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYWORD,	// �C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYWORD,	// �G�N�X�|�[�g
	IDC_CHECK_KEYWORDCASE,			HIDC_CHECK_KEYWORDCASE,		// �L�[���[�h�̉p�啶�����������
	IDC_COMBO_SET,					HIDC_COMBO_SET,				// �����L�[���[�h�Z�b�g��
	IDC_LIST_KEYWORD,				HIDC_LIST_KEYWORD,			// �L�[���[�h�ꗗ
	IDC_BUTTON_KEYCLEAN		,		HIDC_BUTTON_KEYCLEAN,		// �L�[���[�h����
	IDC_BUTTON_KEYSETRENAME,		HIDC_BUTTON_KEYSETRENAME,	// �Z�b�g�̖��̕ύX
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropKeyword::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropKeyword::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta
INT_PTR CALLBACK PropKeyword::DlgProc_dialog(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc2(reinterpret_cast<pDispatchPage>(&PropKeyword::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// Keyword ���b�Z�[�W����
INT_PTR PropKeyword::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	int					nIndex1;
	int					i;
	LV_COLUMN			lvc;
	LV_ITEM*			plvi;
	static HWND			hwndCOMBO_SET;
	static HWND			hwndLIST_KEYWORD;
	RECT				rc;
	DlgInput1			dlgInput1;
	wchar_t				szKeyword[MAX_KEYWORDLEN + 1];
	LONG_PTR			lStyle;
	LV_DISPINFO*		plvdi;
	LV_KEYDOWN*			pnkd;

	auto& csSpecialKeyword = common.specialKeyword;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Keyword
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		if (wParam == IDOK) { // �Ɨ��E�B���h�E
			hwndCtl = ::GetDlgItem(hwndDlg, IDOK);
			GetWindowRect(hwndCtl, &rc);
			i = rc.bottom; // OK,CANCEL�{�^���̉��[

			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, NULL, 0, 0, rc.right - rc.left, i - rc.top + 10, SWP_NOZORDER|SWP_NOMOVE);
			std::tstring title = LS(STR_PROPCOMMON);
			title += _T(" - ");
			title += LS(STR_PROPCOMMON_KEYWORD);
			SetWindowText(hwndDlg, title.c_str());

			hwndCOMBO_SET = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
			Combo_SetCurSel(hwndCOMBO_SET, nKeywordSet1);
		}else {
			hwndCtl = ::GetDlgItem(hwndDlg, IDOK);
			ShowWindow(hwndCtl, SW_HIDE);
			hwndCtl = ::GetDlgItem(hwndDlg, IDCANCEL);
			ShowWindow(hwndCtl, SW_HIDE);
		}

		// �R���g���[���̃n���h�����擾
		hwndCOMBO_SET = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
		hwndLIST_KEYWORD = ::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD);
		::GetWindowRect(hwndLIST_KEYWORD, &rc);
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = rc.right - rc.left;
		lvc.pszText = const_cast<TCHAR*>(_T(""));
		lvc.iSubItem = 0;
		ListView_InsertColumn(hwndLIST_KEYWORD, 0, &lvc);

		lStyle = ::GetWindowLongPtr(hwndLIST_KEYWORD, GWL_STYLE);
		::SetWindowLongPtr(hwndLIST_KEYWORD, GWL_STYLE, lStyle | LVS_SHOWSELALWAYS);

		// �R���g���[���X�V�̃^�C�~���O�p�̃^�C�}�[���N��
		::SetTimer(hwndDlg, 1, 300, NULL);

		return TRUE;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		pnkd = (LV_KEYDOWN *)lParam;
		plvdi = (LV_DISPINFO*)lParam;
		plvi = &plvdi->item;

		if (hwndLIST_KEYWORD == pNMHDR->hwndFrom) {
			switch (pNMHDR->code) {
			case NM_DBLCLK:
//				MYTRACE(_T("NM_DBLCLK     \n"));
				// ���X�g���őI������Ă���L�[���[�h��ҏW����
				Edit_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
				return TRUE;
			case LVN_BEGINLABELEDIT:
#ifdef _DEBUG
				MYTRACE(_T("LVN_BEGINLABELEDIT\n"));
												MYTRACE(_T("	plvi->mask =[%xh]\n"), plvi->mask);
												MYTRACE(_T("	plvi->iItem =[%d]\n"), plvi->iItem);
												MYTRACE(_T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem);
				if (plvi->mask & LVIF_STATE)	MYTRACE(_T("	plvi->state =[%xf]\n"), plvi->state);
												MYTRACE(_T("	plvi->stateMask =[%xh]\n"), plvi->stateMask);
				if (plvi->mask & LVIF_TEXT)		MYTRACE(_T("	plvi->pszText =[%ts]\n"), plvi->pszText);
												MYTRACE(_T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax);
				if (plvi->mask & LVIF_IMAGE)	MYTRACE(_T("	plvi->iImage=[%d]\n"), plvi->iImage);
				if (plvi->mask & LVIF_PARAM)	MYTRACE(_T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam);
#endif
				return TRUE;
			case LVN_ENDLABELEDIT:
#ifdef _DEBUG
				MYTRACE(_T("LVN_ENDLABELEDIT\n"));
												MYTRACE(_T("	plvi->mask =[%xh]\n"), plvi->mask);
												MYTRACE(_T("	plvi->iItem =[%d]\n"), plvi->iItem);
												MYTRACE(_T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem);
				if (plvi->mask & LVIF_STATE)	MYTRACE(_T("	plvi->state =[%xf]\n"), plvi->state);
												MYTRACE(_T("	plvi->stateMask =[%xh]\n"), plvi->stateMask);
				if (plvi->mask & LVIF_TEXT)		MYTRACE(_T("	plvi->pszText =[%ts]\n"), plvi->pszText );
												MYTRACE(_T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax);
				if (plvi->mask & LVIF_IMAGE)	MYTRACE(_T("	plvi->iImage=[%d]\n"), plvi->iImage);
				if (plvi->mask & LVIF_PARAM)	MYTRACE(_T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam);
#endif
				if (!plvi->pszText) {
					return TRUE;
				}
				if (plvi->pszText[0] != _T('\0')) {
					if (MAX_KEYWORDLEN < _tcslen(plvi->pszText)) {
						InfoMessage(hwndDlg, LS(STR_PROPCOMKEYWORD_ERR_LEN), MAX_KEYWORDLEN);
						return TRUE;
					}
					// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW
					csSpecialKeyword.keywordSetMgr.UpdateKeyword(
						csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx,
						plvi->lParam,
						to_wchar(plvi->pszText)
					);
				}else {
					// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜
					csSpecialKeyword.keywordSetMgr.DelKeyword(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx, plvi->lParam);
				}
				// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
				SetKeywordSet(hwndDlg, csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx);

				ListView_SetItemState(hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

				return TRUE;
			case LVN_KEYDOWN:
//				MYTRACE(_T("LVN_KEYDOWN\n"));
				switch (pnkd->wVKey) {
				case VK_DELETE:
					// ���X�g���őI������Ă���L�[���[�h���폜����
					Delete_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					break;
				case VK_SPACE:
					// ���X�g���őI������Ă���L�[���[�h��ҏW����
					Edit_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					break;
				}
				return TRUE;
			}
		}else {
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_KEYWORD);
				return TRUE;
			case PSN_KILLACTIVE:
				DEBUG_TRACE(_T("Keyword PSN_KILLACTIVE\n"));
				// �_�C�A���O�f�[�^�̎擾 Keyword
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_KEYWORD;
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID = LOWORD(wParam);			// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		hwndCtl = (HWND) lParam;		// �R���g���[���̃n���h��
		if (hwndCOMBO_SET == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIndex1 = Combo_GetCurSel(hwndCOMBO_SET);
				// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
				if (nIndex1 < 0) {
					ClearKeywordSet(hwndDlg);
				}else {
					SetKeywordSet(hwndDlg, nIndex1);
				}
				return TRUE;
			}
		}else {
			switch (wNotifyCode) {
			// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
			case BN_CLICKED:
				switch (wID) {
				case IDC_BUTTON_ADDSET:	// �Z�b�g�ǉ�
					if (MAX_SETNUM <= csSpecialKeyword.keywordSetMgr.nKeywordSetNum) {
						InfoMessage(hwndDlg, LS(STR_PROPCOMKEYWORD_SETMAX), MAX_SETNUM);
						return TRUE;
					}
					// ���[�h���X�_�C�A���O�̕\��
					szKeyword[0] = 0;
					//	Oct. 5, 2002 genta ���������̐ݒ���C���D�o�b�t�@�I�[�o�[�������Ă����D
					if (!dlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						LS(STR_PROPCOMKEYWORD_SETNAME1),
						LS(STR_PROPCOMKEYWORD_SETNAME2),
						MAX_SETNAMELEN,
						szKeyword
						)
					) {
						return TRUE;
					}
					if (szKeyword[0] != L'\0') {
						// �Z�b�g�̒ǉ�
						csSpecialKeyword.keywordSetMgr.AddKeywordSet(szKeyword, false);
						csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx = csSpecialKeyword.keywordSetMgr.nKeywordSetNum - 1;

						// �_�C�A���O�f�[�^�̐ݒ� Keyword
						SetData(hwndDlg);
					}
					return TRUE;
				case IDC_BUTTON_DELSET:	// �Z�b�g�폜
					nIndex1 = Combo_GetCurSel(hwndCOMBO_SET);
					if (nIndex1 == CB_ERR) {
						return TRUE;
					}
					// �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v���
					static TCHAR		pszLabel[1024];
					pszLabel[0] = 0;
					for (i=0; i<GetDllShareData().nTypesCount; ++i) {
						auto type = std::make_unique<TypeConfig>();
						DocTypeManager().GetTypeConfig(TypeConfigNum(i), *type);
						// 2002/04/25 YAZAKI TypeConfig�S�̂�ێ�����K�v�͂Ȃ����ApShareData�𒼐ڌ��Ă����Ȃ��B
						if (nIndex1 == types_nKeywordSetIdx[i].index[0]
						||  nIndex1 == types_nKeywordSetIdx[i].index[1]
						||  nIndex1 == types_nKeywordSetIdx[i].index[2]
						||  nIndex1 == types_nKeywordSetIdx[i].index[3]
						||  nIndex1 == types_nKeywordSetIdx[i].index[4]
						||  nIndex1 == types_nKeywordSetIdx[i].index[5]
						||  nIndex1 == types_nKeywordSetIdx[i].index[6]
						||  nIndex1 == types_nKeywordSetIdx[i].index[7]
						||  nIndex1 == types_nKeywordSetIdx[i].index[8]
						||  nIndex1 == types_nKeywordSetIdx[i].index[9]
						) {
							_tcscat(pszLabel, _T("�E"));
							_tcscat(pszLabel, type->szTypeName);
							_tcscat(pszLabel, _T("�i"));
							_tcscat(pszLabel, type->szTypeExts);
							_tcscat(pszLabel, _T("�j"));
							_tcscat(pszLabel, _T("\n"));
						}
					}
					if (::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						LS(STR_PROPCOMKEYWORD_SETDEL),
						csSpecialKeyword.keywordSetMgr.GetTypeName(nIndex1),
						pszLabel
						) == IDCANCEL
					) {
						return TRUE;
					}
					// �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v�̃Z�b�g���N���A
					for (i=0; i<GetDllShareData().nTypesCount; ++i) {
						// 2002/04/25 YAZAKI TypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
						for (int j=0; j<MAX_KEYWORDSET_PER_TYPE; ++j) {
							if (nIndex1 == types_nKeywordSetIdx[i].index[j]) {
								types_nKeywordSetIdx[i].index[j] = -1;
							}else if (nIndex1 < types_nKeywordSetIdx[i].index[j]) {
								types_nKeywordSetIdx[i].index[j]--;
							}
						}
					}
					// ���Ԗڂ̃Z�b�g���폜
					csSpecialKeyword.keywordSetMgr.DelKeywordSet(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx);
					// �_�C�A���O�f�[�^�̐ݒ� Keyword
					SetData(hwndDlg);
					return TRUE;
				case IDC_BUTTON_KEYSETRENAME: // �L�[���[�h�Z�b�g�̖��̕ύX
					// ���[�h���X�_�C�A���O�̕\��
					wcscpy_s(szKeyword, csSpecialKeyword.keywordSetMgr.GetTypeName(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx));
					{
						BOOL bDlgInputResult = dlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							LS(STR_PROPCOMKEYWORD_RENAME1),
							LS(STR_PROPCOMKEYWORD_RENAME2),
							MAX_SETNAMELEN,
							szKeyword
						);
						if (!bDlgInputResult) {
							return TRUE;
						}
					}
					if (szKeyword[0] != L'\0') {
						csSpecialKeyword.keywordSetMgr.SetTypeName(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx, szKeyword);
						// �_�C�A���O�f�[�^�̐ݒ� Keyword
						SetData(hwndDlg);
					}
					return TRUE;
				case IDC_CHECK_KEYWORDCASE:	// �L�[���[�h�̉p�啶�����������
					csSpecialKeyword.keywordSetMgr.SetKeywordCase(
						csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx,
						DlgButton_IsChecked(hwndDlg, IDC_CHECK_KEYWORDCASE)
						);
					return TRUE;
				case IDC_BUTTON_ADDKEYWORD:	// �L�[���[�h�ǉ�
					// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
					if (!csSpecialKeyword.keywordSetMgr.CanAddKeyword(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx)) {
						InfoMessage(hwndDlg, LS(STR_PROPCOMKEYWORD_KEYMAX));
						return TRUE;
					}
					// ���[�h���X�_�C�A���O�̕\��
					szKeyword[0] = 0;
					if (!dlgInput1.DoModal(G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYADD1), LS(STR_PROPCOMKEYWORD_KEYADD2), MAX_KEYWORDLEN, szKeyword)) {
						return TRUE;
					}
					if (szKeyword[0] != L'\0') {
						// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�
						if (csSpecialKeyword.keywordSetMgr.AddKeyword(csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx, szKeyword) == 0) {
							// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
							SetKeywordSet(hwndDlg, csSpecialKeyword.keywordSetMgr.nCurrentKeywordSetIdx);
						}
					}
					return TRUE;
				case IDC_BUTTON_EDITKEYWORD:	// �L�[���[�h�ҏW
					// ���X�g���őI������Ă���L�[���[�h��ҏW����
					Edit_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					return TRUE;
				case IDC_BUTTON_DELKEYWORD:	// �L�[���[�h�폜
					// ���X�g���őI������Ă���L�[���[�h���폜����
					Delete_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					return TRUE;
				case IDC_BUTTON_KEYCLEAN:
					Clean_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					return TRUE;
				case IDC_BUTTON_IMPORT:	// �C���|�[�g
					// ���X�g���̃L�[���[�h���C���|�[�g����
					Import_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					return TRUE;
				case IDC_BUTTON_EXPORT:	// �G�N�X�|�[�g
					// ���X�g���̃L�[���[�h���G�N�X�|�[�g����
					Export_List_Keyword(hwndDlg, hwndLIST_KEYWORD);
					return TRUE;
				// �Ɨ��E�B���h�E�Ŏg�p����
				case IDOK:
					EndDialog(hwndDlg, IDOK);
					break;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
				}
				break;	// BN_CLICKED
			}
		}
		break;	// WM_COMMAND

	case WM_TIMER:
		nIndex1 = ListView_GetNextItem(hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED);
		if (nIndex1 == -1) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EDITKEYWORD), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELKEYWORD), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EDITKEYWORD), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELKEYWORD), TRUE);
		}
		break;

	case WM_DESTROY:
		::KillTimer(hwndDlg, 1);
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		//break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}

// ���X�g���őI������Ă���L�[���[�h��ҏW����
void PropKeyword::Edit_List_Keyword(HWND hwndDlg, HWND hwndLIST_KEYWORD)
{
	int			nIndex1;
	LV_ITEM		lvi;
	wchar_t		szKeyword[MAX_KEYWORDLEN + 1];
	DlgInput1	dlgInput1;

	nIndex1 = ListView_GetNextItem(hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED);
	if (nIndex1 == -1) {
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem(hwndLIST_KEYWORD, &lvi);

	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;

	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�
	wcscpy_s(szKeyword, keywordSetMgr.GetKeyword(keywordSetMgr.nCurrentKeywordSetIdx, lvi.lParam));

	// ���[�h���X�_�C�A���O�̕\��
	if (!dlgInput1.DoModal(G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYEDIT1), LS(STR_PROPCOMKEYWORD_KEYEDIT2), MAX_KEYWORDLEN, szKeyword)) {
		return;
	}
	if (szKeyword[0] != L'\0') {
		// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW
		keywordSetMgr.UpdateKeyword(
			keywordSetMgr.nCurrentKeywordSetIdx,
			lvi.lParam,
			szKeyword
		);
	}else {
		// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜
		keywordSetMgr.DelKeyword(keywordSetMgr.nCurrentKeywordSetIdx, lvi.lParam);
	}
	// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
	SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);

	ListView_SetItemState(hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return;
}


// ���X�g���őI������Ă���L�[���[�h���폜����
void PropKeyword::Delete_List_Keyword(HWND hwndDlg, HWND hwndLIST_KEYWORD)
{
	int			nIndex1;
	LV_ITEM		lvi;

	nIndex1 = ListView_GetNextItem(hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED);
	if (nIndex1 == -1) {
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem(hwndLIST_KEYWORD, &lvi);

	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;
	
	// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜
	keywordSetMgr.DelKeyword(keywordSetMgr.nCurrentKeywordSetIdx, lvi.lParam);
	// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
	SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);
	ListView_SetItemState(hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	// �L�[���[�h����\������
	DispKeywordCount(hwndDlg);

	return;
}


// ���X�g���̃L�[���[�h���C���|�[�g����
void PropKeyword::Import_List_Keyword(HWND hwndDlg, HWND hwndLIST_KEYWORD)
{
	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;
	
	bool bCase = false;
	size_t nIdx = keywordSetMgr.nCurrentKeywordSetIdx;
	keywordSetMgr.SetKeywordCase(nIdx, bCase);
	ImpExpKeyword impExpKeyword(common, nIdx, bCase);

	// �C���|�[�g
	if (!impExpKeyword.ImportUI(G_AppInstance(), hwndDlg)) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}

	// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
	SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);
	return;
}

// ���X�g���̃L�[���[�h���G�N�X�|�[�g����
void PropKeyword::Export_List_Keyword(HWND hwndDlg, HWND hwndLIST_KEYWORD)
{
	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;

	// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
	SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);

	bool	bCase;
	ImpExpKeyword impExpKeyword(common, keywordSetMgr.nCurrentKeywordSetIdx, bCase);

	// �G�N�X�|�[�g
	if (!impExpKeyword.ExportUI(G_AppInstance(), hwndDlg)) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}


// �L�[���[�h�𐮓ڂ���
void PropKeyword::Clean_List_Keyword(HWND hwndDlg, HWND hwndLIST_KEYWORD)
{
	if (::MessageBox(hwndDlg, LS(STR_PROPCOMKEYWORD_DEL),
			GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION
		) == IDYES
	) {
		auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;

		if (keywordSetMgr.CleanKeywords(keywordSetMgr.nCurrentKeywordSetIdx)) {
		}
		SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);
	}
}

// �_�C�A���O�f�[�^�̐ݒ� Keyword
void PropKeyword::SetData(HWND hwndDlg)
{
	// �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g
	HWND hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
	Combo_ResetContent(hwndWork);  // �R���{�{�b�N�X����ɂ���
	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;
	if (0 < keywordSetMgr.nKeywordSetNum) {
		for (size_t i=0; i<keywordSetMgr.nKeywordSetNum; ++i) {
			Combo_AddString(hwndWork, keywordSetMgr.GetTypeName(i));
		}
		// �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I��
		Combo_SetCurSel(hwndWork, keywordSetMgr.nCurrentKeywordSetIdx);

		// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
		SetKeywordSet(hwndDlg, keywordSetMgr.nCurrentKeywordSetIdx);
	}else {
		// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
		ClearKeywordSet(hwndDlg);
	}
	return;
}

void PropKeyword::ClearKeywordSet(HWND hwndDlg)
{
	::CheckDlgButton(hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE);

	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELSET), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KEYWORDCASE), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADDKEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EDITKEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELKEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_IMPORT), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EXPORT), FALSE);
}

// �_�C�A���O�f�[�^�̐ݒ� Keyword �w��L�[���[�h�Z�b�g�̐ݒ�
void PropKeyword::SetKeywordSet(HWND hwndDlg, size_t nIdx)
{
	ListView_DeleteAllItems(::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD));
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELSET), TRUE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KEYWORDCASE), TRUE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD), TRUE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADDKEYWORD), TRUE);
	//	Jan. 29, 2005 genta �L�[���[�h�Z�b�g�؂�ւ�����̓L�[���[�h�͖��I��
	//	���̂��ߗL���ɂ��Ă����Ƀ^�C�}�[�Ŗ����ɂȂ�D
	//	�Ȃ̂ł����Ŗ����ɂ��Ă����D
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EDITKEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELKEYWORD), FALSE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_IMPORT), TRUE);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_EXPORT), TRUE);

	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;
	// �L�[���[�h�̉p�啶�����������
	::CheckDlgButton(hwndDlg, IDC_CHECK_KEYWORDCASE, keywordSetMgr.GetKeywordCase(nIdx));

	// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
	size_t nNum = keywordSetMgr.GetKeywordNum(nIdx);
	HWND hwndList = ::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD);
	LV_ITEM	lvi;

	// ���X�g�ǉ����͍ĕ`���}�����Ă��΂₭�\��
	::SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

	for (size_t i=0; i<nNum; ++i) {
		// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�
		const TCHAR* pszKeyword = to_tchar(keywordSetMgr.GetKeyword(nIdx, i));
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = const_cast<TCHAR*>(pszKeyword);
		lvi.iItem = (int)i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem(hwndList, &lvi);
	}
	keywordSetMgr.nCurrentKeywordSetIdx = nIdx;

	// ���X�g�ǉ������̂��ߍĕ`�拖��
	::SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

	// �L�[���[�h����\������B
	DispKeywordCount(hwndDlg);

	return;
}


// �_�C�A���O�f�[�^�̎擾 Keyword
int PropKeyword::GetData(HWND hwndDlg)
{
	return TRUE;
}

// �L�[���[�h����\������B
void PropKeyword::DispKeywordCount(HWND hwndDlg)
{
	HWND hwndList = ::GetDlgItem(hwndDlg, IDC_LIST_KEYWORD);
	int n = ListView_GetItemCount(hwndList);
	if (n < 0) {
		n = 0;
	}

	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;

	size_t nAlloc
		= keywordSetMgr.GetAllocSize(keywordSetMgr.nCurrentKeywordSetIdx)
		- keywordSetMgr.GetKeywordNum(keywordSetMgr.nCurrentKeywordSetIdx)
		+ keywordSetMgr.GetFreeSize()
	;
	
	TCHAR szCount[256];
	auto_sprintf(szCount, LS(STR_PROPCOMKEYWORD_INFO), MAX_KEYWORDLEN, n, nAlloc);
	::SetWindowText(::GetDlgItem(hwndDlg, IDC_STATIC_KEYWORD_COUNT), szCount);
}

