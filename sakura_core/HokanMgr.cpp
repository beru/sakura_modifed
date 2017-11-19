#include "StdAfx.h"

#include <memory>

#include "HokanMgr.h"
#include "env/ShareData.h"
#include "view/EditView.h"
#include "plugin/JackManager.h"
#include "plugin/ComplementIfObj.h"
#include "util/input.h"
#include "util/os.h"
#include "sakura_rc.h"

WNDPROC g_wpHokanListProc;


LRESULT APIENTRY HokanList_SubclassProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	Dialog* pDialog = (Dialog*)::GetWindowLongPtr(::GetParent(hwnd), DWLP_USER);
	HokanMgr* pHokanMgr = (HokanMgr*)::GetWindowLongPtr(::GetParent(hwnd), DWLP_USER);
	MSG* pMsg;
	int nVKey;

	switch (uMsg) {
	case WM_KEYDOWN:
		nVKey = (int) wParam;	// virtual-key code
		// �L�[������U�����悤
		if (nVKey == VK_SPACE) {	//	Space
			// Shift,Ctrl,Alt�L�[��������Ă�����
			int nIdx = GetCtrlKeyState();
			if (nIdx == _SHIFT) {
				//	Shift + Space�Ł����U��
				wParam = VK_UP;
			}else if (nIdx == 0) {
				//	Space�݂̂Ł����U��
				wParam = VK_DOWN;
			}
		}
		// �⊮���s�L�[�Ȃ�⊮����
		if (pHokanMgr->KeyProc(wParam, lParam) != -1) {
			// �L�[�X�g���[�N��e�ɓ]��
			return ::PostMessage(::GetParent(::GetParent(pDialog->hwndParent)), uMsg, wParam, lParam);
		}
		break;
	case WM_GETDLGCODE:
		pMsg = (MSG*) lParam; // pointer to an MSG structure
		if (!pMsg) {
//			MYTRACE(_T("WM_GETDLGCODE  pMsg==NULL\n"));
			return 0;
		}
//		MYTRACE(_T("WM_GETDLGCODE  pMsg->message = %xh\n"), pMsg->message);
		return DLGC_WANTALLKEYS; // ���ׂẴL�[�X�g���[�N�����ɉ�����
	}
	return CallWindowProc(g_wpHokanListProc, hwnd, uMsg, wParam, lParam);
}


HokanMgr::HokanMgr()
{
	memCurWord.SetString(L"");

	nCurKouhoIdx = -1;
	bTimerFlag = TRUE;
}

HokanMgr::~HokanMgr()
{
}

// ���[�h���X�_�C�A���O�̕\��
HWND HokanMgr::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	HWND hwndWork = Dialog::DoModeless(hInstance, hwndParent, IDD_HOKAN, lParam, SW_HIDE);
	OnSize(0, 0);
	// ���X�g���t�b�N
	::g_wpHokanListProc = (WNDPROC) ::SetWindowLongPtr(GetItemHwnd(IDC_LIST_WORDS), GWLP_WNDPROC, (LONG_PTR)HokanList_SubclassProc);

	::ShowWindow(GetHwnd(), SW_HIDE);
	return hwndWork;
}

// ���[�h���X���F�ΏۂƂȂ�r���[�̕ύX
void HokanMgr::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}

void HokanMgr::Hide(void)
{
	::ShowWindow(GetHwnd(), SW_HIDE);
	nCurKouhoIdx = -1;
	// ���̓t�H�[�J�X���󂯎�����Ƃ��̏���
	EditView* pEditView = reinterpret_cast<EditView*>(lParam);
	pEditView->OnSetFocus();
}

/*!	������
	pMemHokanWord == NULL�̂Ƃ��A�⊮��₪�ЂƂ�������A�⊮�E�B���h�E��\�����Ȃ��ŏI�����܂��B
	Search()�Ăяo�����Ŋm�菈����i�߂Ă��������B
*/
size_t HokanMgr::Search(
	POINT*			pPt,
	int				nWinHeight,
	int				nColumnWidth,
	const wchar_t*	pszCurWord,
	const TCHAR*	pszHokanFile,
	bool			bHokanLoHiCase,	// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����
	bool			bHokanByFile,	// �ҏW���f�[�^�������T��
	int				nHokanType,
	bool			bHokanByKeyword,
	NativeW*		pMemHokanWord
	)
{
	EditView* pEditView = reinterpret_cast<EditView*>(lParam);

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	/*
	||  �⊮�L�[���[�h�̌���
	||
	||  �E���������������ׂĕԂ�(���s�ŋ�؂��ĕԂ�)
	||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
	||  �E������������Ԃ�
	||
	*/
	vKouho.clear();
	DicMgr::HokanSearch(
		pszCurWord,
		bHokanLoHiCase,
		vKouho,
		0, // Max��␔
		pszHokanFile
	);

	// �ҏW���f�[�^���������T��
	if (bHokanByFile) {
		pEditView->HokanSearchByFile(
			pszCurWord,
			bHokanLoHiCase,
			vKouho,
			1024 // �ҏW���f�[�^����Ȃ̂Ő��𐧌����Ă���
		);
	}
	// �����L�[���[�h�������T��
	if (bHokanByKeyword) {
		HokanSearchByKeyword(
			pszCurWord,
			bHokanLoHiCase,
			vKouho
		);
	}

	{
		int nOption = (
			  (bHokanLoHiCase ? 0x01 : 0)
			  | (bHokanByFile ? 0x02 : 0)
			);
		
		Plug::Array plugs;
		Plug::Array plugType;
		JackManager::getInstance().GetUsablePlug(PP_COMPLEMENTGLOBAL, 0, &plugs);
		if (nHokanType != 0) {
			JackManager::getInstance().GetUsablePlug(PP_COMPLEMENT, nHokanType, &plugType);
			if (0 < plugType.size()) {
				plugs.push_back(plugType[0]);
			}
		}

		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			// �C���^�t�F�[�X�I�u�W�F�N�g����
			WSHIfObj::List params;
			std::wstring curWord = pszCurWord;
			ComplementIfObj* objComp = new ComplementIfObj(curWord , *this, nOption);
			objComp->AddRef();
			params.push_back(objComp);
			// �v���O�C���Ăяo��
			(*it)->Invoke(*pEditView, params);

			objComp->Release();
		}
	}

	if (vKouho.size() == 0) {
		nCurKouhoIdx = -1;
		return 0;
	}

  // ��₪�P�̏ꍇ�⊮�E�B���h�E�͕\�����Ȃ�(�����⊮�̏ꍇ�͏���)
	if (vKouho.size() == 1) {
		if (pMemHokanWord) {
			nCurKouhoIdx = -1;
			pMemHokanWord->SetString(vKouho[0].c_str());
			return 1;
		}
	}


//	hFont = hFont;
	point.x = pPt->x;
	point.y = pPt->y;
	this->nWinHeight = nWinHeight;
	this->nColumnWidth = nColumnWidth;
//	memCurWord.SetData(pszCurWord, lstrlen(pszCurWord));
	memCurWord.SetString(pszCurWord);
	nCurKouhoIdx = 0;
//	SetCurKouhoStr();

//	::ShowWindow(GetHwnd(), SW_SHOWNA);

	HWND hwndList;
	hwndList = GetItemHwnd(IDC_LIST_WORDS);
	List_ResetContent(hwndList);
	{
		size_t kouhoNum = vKouho.size();
		for (size_t i=0; i<kouhoNum; ++i) {
			::List_AddString(hwndList, vKouho[i].c_str());
		}
	}
	List_SetCurSel(hwndList, 0);

//@@	::EnableWindow(::GetParent(::GetParent(hwndParent)), FALSE);

	RECT rcDesktop;
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(GetHwnd(), &rcDesktop );

	int nX = point.x - nColumnWidth;
	int nY = point.y + nWinHeight + 4;
	int nCX = nWidth;
	int nCY = nHeight;

	// ���ɓ���Ȃ�
	if (nY + nCY < rcDesktop.bottom) {
		// �������Ȃ�
	}else
	// ��ɓ���Ȃ�
	if (rcDesktop.top < point.y - nHeight - 4) {
		// ��ɏo��
		nY = point.y - nHeight - 4;
	}else
	// ��ɏo�������ɏo����(�L���ق��ɏo��)
	if (rcDesktop.bottom - nY > point.y) {
		// ���ɏo��
//		nHeight = rcDesktop.bottom - nY;
		nCY = rcDesktop.bottom - nY;
	}else {
		// ��ɏo��
		nY = rcDesktop.top;
		nCY = point.y - 4 - rcDesktop.top;
	}

  // �\���ʒu�␳

	// �E�ɓ���
	if (nX + nCX < rcDesktop.right) {
		// ���̂܂�
	}else
	// ���ɓ���
	if (rcDesktop.left < nX - nCX + 8) {
		// ���ɕ\��
		nX -= nCX - 8;
	}else {
		// �T�C�Y�𒲐����ĉE�ɕ\��
		nCX = t_max((int)(rcDesktop.right - nX) , 100);	// �Œ�T�C�Y��100���炢��
	}

  // �␳��̈ʒu�E�T�C�Y��ۑ�
	point.x = nX;
	point.y = nY;
	nHeight = nCY;
	nWidth = nCX;

	// �͂ݏo���Ȃ珬��������
//	if (rcDesktop.bottom < nY + nCY) {
//		// ���ɂ͂ݏo��
//		if (point.y - 4 - nCY < 0) {
//			// ��ɂ͂ݏo��
//			// ��������������
//			nCY = rcDesktop.bottom - nY - 4;
//		}else {
//
//		}
//
//	}
	::MoveWindow(GetHwnd(), nX, nY, nCX, nCY, TRUE);
	::ShowWindow(GetHwnd(), SW_SHOW);
//	::ShowWindow(GetHwnd(), SW_SHOWNA);
	::SetFocus(GetHwnd());
//	::SetFocus(GetItemHwnd(IDC_LIST_WORDS));
//	::SetFocus(::GetParent(::GetParent(hwndParent)));

	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��

  // ���̃��\�b�h�Ŏg���Ă��Ȃ��̂ŁA�Ƃ肠�����폜���Ă���
	size_t kouhoNum = vKouho.size();
	vKouho.clear();
	return kouhoNum;
}

void HokanMgr::HokanSearchByKeyword(
	const wchar_t*	pszCurWord,
	bool 			bHokanLoHiCase,
	vector_ex<std::wstring>& 	vKouho
) {
	const EditView* pEditView = reinterpret_cast<const EditView*>(lParam);
	const TypeConfig& type = pEditView->GetDocument().docType.GetDocumentAttribute();
	KeywordSetMgr& keywordMgr = pShareData->common.specialKeyword.keywordSetMgr;
	const size_t nKeyLen = wcslen(pszCurWord);
	for (int n=0; n<MAX_KEYWORDSET_PER_TYPE; ++n) {
		int kwdset = type.nKeywordSetIdx[n];
		if (kwdset == -1) {
			continue;
		}
		const size_t keyCount = keywordMgr.GetKeywordNum(kwdset);
		for (size_t i=0; i<keyCount; ++i) {
			const wchar_t* word = keywordMgr.GetKeyword(kwdset, i);
			int nRet;
			if (bHokanLoHiCase) {
				nRet = auto_memicmp(pszCurWord, word, nKeyLen );
			}else {
				nRet = auto_memcmp(pszCurWord, word, nKeyLen );
			}
			if (nRet != 0) {
				continue;
			}
			std::wstring strWord = std::wstring(word);
			AddKouhoUnique(vKouho, strWord);
		}
	}
}


BOOL HokanMgr::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);
	// ���N���X�����o
//-	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);

}

BOOL HokanMgr::OnDestroy(void)
{
	// ���N���X�����o
	CreateSizeBox();
	return Dialog::OnDestroy();
}


BOOL HokanMgr::OnSize(WPARAM wParam, LPARAM lParam)
{
	// ���N���X�����o
	Dialog::OnSize(wParam, lParam);

	static const int controls[] = {
		IDC_LIST_WORDS
	};
	RECT	rcDlg;
	::GetClientRect(GetHwnd(), &rcDlg);
	int nWidth = rcDlg.right - rcDlg.left;  // width of client area
	int nHeight = rcDlg.bottom - rcDlg.top; // height of client area

  // �T�C�Y�ύX��̈ʒu��ۑ�
	point.x = rcDlg.left - 4;
	point.y = rcDlg.top - 3;
	::ClientToScreen(GetHwnd(), &point);

	int nControls = _countof(controls);
	for (int i=0; i<nControls; ++i) {
		HWND hwndCtrl = GetItemHwnd(controls[i]);
		RECT rc;
		::GetWindowRect(hwndCtrl, &rc);
		POINT po;
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient(GetHwnd(), &po);
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient(GetHwnd(), &po);
		rc.right = po.x;
		rc.bottom  = po.y;
		if (controls[i] == IDC_LIST_WORDS) {
			::SetWindowPos(
				hwndCtrl,
				NULL,
				rc.left,
				rc.top,
				nWidth - rc.left * 2,
				nHeight - rc.top * 2/* - 20*/,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
	}

	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��

	return TRUE;

}

BOOL HokanMgr::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:
//		CloseDialog(0);
		Hide();
		return TRUE;
	case IDOK:
//		CloseDialog(0);
		// �⊮���s
		DoHokan(VK_RETURN);
		return TRUE;
	}
	return FALSE;
}


BOOL HokanMgr::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	int nVKey = (int) wParam;	// virtual-key code
//	lKeyData = lParam;			// key data
	switch (nVKey) {
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		return 1;
	}
 	return 0;
}


BOOL HokanMgr::OnLbnSelChange(HWND hwndCtl, int wID)
{
	ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��
	return TRUE;
}

BOOL HokanMgr::OnLbnDblclk(int wID)
{
	// �⊮���s
	DoHokan(0);
	return TRUE;

}


BOOL HokanMgr::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
//	Hide();
	return TRUE;
}


// �⊮���s
BOOL HokanMgr::DoHokan(int nVKey)
{
	DEBUG_TRACE(_T("HokanMgr::DoHokan(nVKey==%xh)\n"), nVKey);

	// �⊮��⌈��L�[
	auto& csHelper = pShareData->common.helper;
	if (nVKey == VK_RETURN	&& !csHelper.bHokanKey_RETURN)	return FALSE; // VK_RETURN �⊮����L�[���L��/����
	if (nVKey == VK_TAB		&& !csHelper.bHokanKey_TAB)		return FALSE; // VK_TAB    �⊮����L�[���L��/����
	if (nVKey == VK_RIGHT	&& !csHelper.bHokanKey_RIGHT)	return FALSE; // VK_RIGHT  �⊮����L�[���L��/����

	HWND hwndList = GetItemHwnd(IDC_LIST_WORDS);
	int nItem = List_GetCurSel(hwndList);
	if (nItem == LB_ERR) {
		return FALSE;
	}
	int nLabelLen = List_GetTextLen( hwndList, nItem );
	auto wszLabel = std::make_unique<wchar_t[]>(nLabelLen + 1);
	List_GetText( hwndList, nItem, &wszLabel[0] );

 	// �e�L�X�g��\��t��
	EditView* pEditView = reinterpret_cast<EditView*>(lParam);
	pEditView->GetCommander().HandleCommand(F_WordDeleteToStart, false, 0, 0, 0, 0);
	pEditView->GetCommander().HandleCommand( F_INSTEXT_W, true, (LPARAM)&wszLabel[0], wcslen(&wszLabel[0]), TRUE, 0 );

//	pEditView->GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)(wszLabel + memCurWord.GetLength()), TRUE, 0, 0);
	Hide();

	pShareData->common.helper.bUseHokan = false;	//	�⊮������
	return TRUE;
}

/*
�߂�l�� -2 �̏ꍇ�́A�A�v���P�[�V�����͍��ڂ̑I�����������A
���X�g �{�b�N�X�ł���ȏ�̓��삪�K�v�łȂ����Ƃ������܂��B

�߂�l�� -1 �̏ꍇ�́A���X�g �{�b�N�X���L�[�X�g���[�N�ɉ�����
�f�t�H���g�̓�������s���邱�Ƃ������܂��B

 �߂�l�� 0 �ȏ�̏ꍇ�́A���̒l�̓��X�g �{�b�N�X�̍��ڂ� 0 ��
��Ƃ����C���f�b�N�X���Ӗ����A���X�g �{�b�N�X�����̍��ڂł�
�L�[�X�g���[�N�ɉ����ăf�t�H���g�̓�������s���邱�Ƃ������܂��B

*/
//	int HokanMgr::OnVKeyToItem(WPARAM wParam, LPARAM lParam)
//	{
//		return KeyProc(wParam, lParam);
//	}

/*
�߂�l�� -2 �̏ꍇ�́A�A�v���P�[�V�����͍��ڂ̑I�����������A
���X�g �{�b�N�X�ł���ȏ�̓��삪�K�v�łȂ����Ƃ������܂��B

�߂�l�� -1 �̏ꍇ�́A���X�g �{�b�N�X���L�[�X�g���[�N�ɉ�����
�f�t�H���g�̓�������s���邱�Ƃ������܂��B

 �߂�l�� 0 �ȏ�̏ꍇ�́A���̒l�̓��X�g �{�b�N�X�̍��ڂ� 0 ��
��Ƃ����C���f�b�N�X���Ӗ����A���X�g �{�b�N�X�����̍��ڂł�
�L�[�X�g���[�N�ɉ����ăf�t�H���g�̓�������s���邱�Ƃ������܂��B

*/
//	int HokanMgr::OnCharToItem(WPARAM wParam, LPARAM lParam)
//	{
//		WORD vkey;
//		WORD nCaretPos;
//		LPARAM hwndLB;
//		vkey = LOWORD(wParam);		// virtual-key code
//		nCaretPos = HIWORD(wParam);	// caret position
//		hwndLB = lParam;			// handle to list box
//	//	switch (vkey) {
//	//	}
//
//		MYTRACE(_T("HokanMgr::OnCharToItem vkey=%xh\n"), vkey);
//		return -1;
//	}

int HokanMgr::KeyProc(WPARAM wParam, LPARAM lParam)
{
	WORD vkey = LOWORD(wParam);		// virtual-key code
//	MYTRACE(_T("HokanMgr::OnVKeyToItem vkey=%xh\n"), vkey);
	switch (vkey) {
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		// ���X�g�{�b�N�X�̃f�t�H���g�̓����������
		return -1;
	case VK_RETURN:
	case VK_TAB:
	case VK_RIGHT:
#if 0
	case VK_SPACE:
#endif
		// �⊮���s
		if (DoHokan(vkey)) {
			return -1;
		}else {
			return -2;
		}
	case VK_ESCAPE:
	case VK_LEFT:
		pShareData->common.helper.bUseHokan = false;
		return -2;
	}
	return -2;
}

// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v��\��
void HokanMgr::ShowTip()
{
	HWND hwndCtrl = GetItemHwnd(IDC_LIST_WORDS);
	int nItem = List_GetCurSel(hwndCtrl);
	if (nItem == LB_ERR) {
		return ;
	}

	int nLabelLen = List_GetTextLen( hwndCtrl, nItem );
	auto szLabel = std::make_unique<wchar_t[]>(nLabelLen + 1);
	List_GetText( hwndCtrl, nItem, &szLabel[0] );	// �I�𒆂̒P����擾

	EditView* pEditView = reinterpret_cast<EditView*>(lParam);
	// ���łɎ���Tip���\������Ă�����
	if (pEditView->dwTipTimer == 0) {
		// ����Tip������
		pEditView -> tipWnd.Hide();
		pEditView -> dwTipTimer = ::GetTickCount();
	}

	// �\������ʒu������
	int nTopItem = List_GetTopIndex(hwndCtrl);
	int nItemHeight = List_GetItemHeight(hwndCtrl, 0);
	POINT pt;
	pt.x = point.x + nWidth;
	pt.y = point.y + 4 + (nItem - nTopItem) * nItemHeight;
	// �I�𒆂̒P�ꂪ�⊮�E�B���h�E�ɕ\������Ă���Ȃ玫��Tip��\��
	if (pt.y > point.y && pt.y < point.y + nHeight) {
		RECT rcHokanWin;
		::SetRect(&rcHokanWin, point.x, point.y, point.x + nWidth, point.y + nHeight);
		if (!pEditView -> ShowKeywordHelp( point, &szLabel[0], &rcHokanWin )) {
			pEditView -> dwTipTimer = ::GetTickCount();	// �\������ׂ��L�[���[�h�w���v������
		}
	}
}

bool HokanMgr::AddKouhoUnique(
	vector_ex<std::wstring>& kouhoList,
	const std::wstring& strWord
	)
{
	return kouhoList.push_back_unique(strWord);
}

const DWORD p_helpids[] = {
	0, 0
};

LPVOID HokanMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

