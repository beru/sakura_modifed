/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X
*/

#include "StdAfx.h"
#include "dlg/DlgTagJumpList.h"
#include "SortedTagJumpList.h"
#include "func/Funccode.h"
#include "env/DllSharedData.h"
#include "util/container.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


const DWORD p_helpids[] = {
	IDC_LIST_TAGJUMP,		HIDC_LIST_TAGJUMPLIST,			// �t�@�C��
	IDOK,					HIDC_TAGJUMPLIST_IDOK,			// OK
	IDCANCEL,				HIDC_TAGJUMPLIST_IDCANCEL,		// �L�����Z��
	IDC_BUTTON_HELP,		HIDC_BUTTON_TAGJUMPLIST_HELP,	// �w���v
	IDC_KEYWORD,			HDIC_TAGJUMPLIST_KEYWORD,		// �L�[���[�h
	IDC_CHECK_ICASE,		HIDC_CHECK_ICASE,
	IDC_CHECK_ANYWHERE,		HIDC_CHECK_ANYWHERE,
	IDC_BUTTON_NEXTTAG,		HIDC_BUTTON_NEXTTAG,
	IDC_BUTTON_PREVTAG,		HIDC_BUTTON_PREVTAG,
//	IDC_STATIC,				-1,
	0, 0
};

static const AnchorListItem anchorList[] = {
	{IDC_STATIC_BASEDIR,	AnchorStyle::Bottom},
	{IDC_STATIC_KEYWORD,	AnchorStyle::Bottom},
	{IDC_KEYWORD,			AnchorStyle::Bottom},
	{IDC_LIST_TAGJUMP,		AnchorStyle::All},
	{IDC_BUTTON_PREVTAG,	AnchorStyle::Bottom},
	{IDC_BUTTON_NEXTTAG,	AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,		AnchorStyle::Bottom},
	{IDOK,					AnchorStyle::Bottom},
	{IDCANCEL,				AnchorStyle::Bottom},
	{IDC_CHECK_ICASE,		AnchorStyle::Bottom},
	{IDC_CHECK_ANYWHERE,	AnchorStyle::Bottom},
};


// �^�O�t�@�C���̃t�H�[�}�b�g
#define TAG_FORMAT_2_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%s\t%s"
#define TAG_FORMAT_1_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d"
#define TAG_FILE_INFO_A      "%[^\t\r\n]\t%[^\t\r\n]\t%[^\t\r\n]"
// #define TAG_FORMAT_E_FILE_A  "%[^\t\r\n,],%d"
// #define TAG_FORMAT_E_NAME_A  "%[^\x7f\r\n]\x7f%[^\x7ff\r\n\x01]\x01%d,%d"

// �L�[���[�h����͂��ĊY���������\������܂ł̎���(�~���b)
#define TAGJUMP_TIMER_DELAY 700
#define TAGJUMP_TIMER_DELAY_SHORT 50


/*
	ctags.exe ���o�͂���A�g���q�ƑΉ�������
*/
static const TCHAR* p_extentions[] = {
	/*asm*/			_T("asm,s"),								_T("d=define,l=label,m=macro,t=type"),
	/*asp*/			_T("asp,asa"),								_T("f=function,s=sub"),
	/*awk*/			_T("awk,gawk,mawk"),						_T("f=function"),
	/*beta*/		_T("bet"),									_T("f=fragment,p=pattern,s=slot,v=virtual"),
	/*c*/			_T("c,h"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*c++*/			_T("c++,cc,cp,cpp,cxx,h++,hh,hp,hpp,hxx"),	_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*java*/		_T("java"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*vera*/		_T("vr,vri,vrh"),							_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*cobol*/		_T("cbl,cob"),								_T("d=data,f=file,g=group,p=paragraph,P=program,s=section"),
	/*eiffel*/		_T("e"),									_T("c=class,f=feature,l=local"),
	/*fortran*/		_T("f,for,ftn,f77,f90,f95"),				_T("b=block data,c=common,e=entry,f=function,i=interface,k=component,l=label,L=local,m=module,n=namelist,p=program,s=subroutine,t=type,v=variable"),
	/*lisp*/		_T("cl,clisp,el,l,lisp,lsp,ml"),			_T("f=function"),
	/*lua*/			_T("lua"),									_T("f=function"),
	/*makefile*/	_T("mak"),									_T("m=macro"),
	/*pascal*/		_T("p,pas"),								_T("f=function,p=procedure"),
	/*perl*/		_T("pl,pm,perl"),							_T("s=subroutine,p=package"),
	/*php*/			_T("php,php3,phtml"),						_T("c=class,f=function"),
	/*python*/		_T("py,python"),							_T("c=class,f=function,m=member"),
	/*rexx*/		_T("cmd,rexx,rx"),							_T("s=subroutine"),
	/*ruby*/		_T("rb"),									_T("c=class,f=method,F=singleton method,m=mixin"),
	/*scheme*/		_T("sch,scheme,scm,sm"),					_T("f=function,s=set"),
	/*sh*/			_T("sh,bsh,bash,ksh,zsh"),					_T("f=function"),
	/*slang*/		_T("sl"),									_T("f=function,n=namespace"),
	/*sql*/			_T("sql"),									_T("c=cursor,d=prototype,f=function,F=field,l=local,P=package,p=procedure,r=record,s=subtype,t=table,T=trigger,v=variable"),
	/*tcl*/			_T("tcl,tk,wish,itcl"),						_T("p=procedure,c=class,f=method"),
	/*verilog*/		_T("v"),									_T("f=function,m=module,P=parameter,p=port,r=reg,t=task,v=variable,w=wire"),
	/*vim*/			_T("vim"),									_T("f=function,v=variable"),
	/*yacc*/		_T("y"),									_T("l=label"),
//	/*vb*/			_T("bas,cls,ctl,dob,dsr,frm,pag"),			_T("a=attribute,c=class,f=function,l=label,s=procedure,v=variable"),
					NULL,									NULL
};

inline bool DlgTagJumpList::IsDirectTagJump() {
	return bDirectTagJump;
}

inline void DlgTagJumpList::ClearPrevFindInfo() {
	psFindPrev->nMatchAll = -1;
	psFind0Match->nDepth  = -1;
	psFind0Match->nMatchAll = 0;
}


DlgTagJumpList::DlgTagJumpList(bool bDirectTagJump)
	:
	Dialog(true),
	bDirectTagJump(bDirectTagJump),
	nIndex(-1),
	pszFileName(NULL),
	pszKeyword(NULL),
	nLoop(-1),
	pList(NULL),
	nTimerId(0),
	bTagJumpICase(FALSE),
	bTagJumpAnyWhere(FALSE),
	nTop(0),
	bNextItem(false),
	psFindPrev(NULL),
	psFind0Match(NULL),
	strOldKeyword(L"")
{
	// �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[����
	assert(_countof(anchorList) == _countof(rcItems));

	pList = new SortedTagJumpList(50);
	psFindPrev = new TagFindState();
	psFind0Match = new TagFindState();
	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
	ClearPrevFindInfo();
}

DlgTagJumpList::~DlgTagJumpList()
{
	Empty();

	if (pszFileName) {
		free(pszFileName);
	}
	pszFileName = NULL;
	if (pszKeyword) {
		free(pszKeyword);
	}
	pszKeyword = NULL;

	StopTimer();
	SAFE_DELETE(pList);
	SAFE_DELETE(psFindPrev);
	SAFE_DELETE(psFind0Match);
}

/*!
	�^�C�}�[��~
*/
void DlgTagJumpList::StopTimer(void)
{
	if (nTimerId != 0) {
		::KillTimer(GetHwnd(), nTimerId);
		nTimerId = 0;
	}
}

/*!
	�^�C�}�[�J�n
	
	�L�[���[�h�w�莞�C�����ԕ������͂��Ȃ���΃��X�g���X�V���邽��
	�u�����ԁv���v��^�C�}�[���K�v
*/
void DlgTagJumpList::StartTimer(int nDelay = TAGJUMP_TIMER_DELAY)
{
	StopTimer();
	nTimerId = ::SetTimer(GetHwnd(), 12345, nDelay, NULL);
}

/*!
	���X�g�̃N���A
*/
void DlgTagJumpList::Empty(void)
{
	nIndex = -1;
	pList->Empty();
}

/*
	���[�_���_�C�A���O�̕\��

	@param[in] lParam 0=�_�C���N�g�^�O�W�����v, 1=�L�[���[�h���w�肵�ă^�O�W�����v
*/
INT_PTR DlgTagJumpList::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
	)
{
	INT_PTR ret = Dialog::DoModal(hInstance, hwndParent, IDD_TAGJUMPLIST, lParam);
	StopTimer();
	return ret;
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgTagJumpList::SetData(void)
{
	if (IsDirectTagJump()) {
		bTagJumpICase = false;
		CheckButton(IDC_CHECK_ICASE, false);
		bTagJumpAnyWhere = false;
		CheckButton(IDC_CHECK_ANYWHERE, false);
		bTagJumpExactMatch = true;

		if (pszKeyword) {
			SetItemText(IDC_KEYWORD, pszKeyword);
		}
	}else {
		// From Here 2005.04.03 MIK �ݒ�l�̓ǂݍ���
		HWND hwndKey;
		hwndKey = GetItemHwnd(IDC_KEYWORD);

		bTagJumpICase = pShareData->tagJump.bTagJumpICase;
		CheckButton(IDC_CHECK_ICASE, bTagJumpICase);
		bTagJumpAnyWhere = pShareData->tagJump.bTagJumpAnyWhere;
		CheckButton(IDC_CHECK_ANYWHERE, bTagJumpAnyWhere);
		bTagJumpExactMatch = FALSE;
		Combo_LimitText(hwndKey, _MAX_PATH-1);
		RecentTagJumpKeyword cRecentTagJump;
		for (size_t i=0; i<cRecentTagJump.GetItemCount(); ++i) {
			Combo_AddString(hwndKey, cRecentTagJump.GetItemText(i));
		}
		if (pszKeyword) {
			SetItemText(IDC_KEYWORD, pszKeyword);
		}else if (cRecentTagJump.GetItemCount() > 0) {
			Combo_SetCurSel(hwndKey, 0);
		}
		cRecentTagJump.Terminate();
	}
	
	SetTextDir();

	UpdateData(true);

	// �O�̂��ߏォ��UpdateData�̌�Ɉړ�
	if (!IsDirectTagJump()) {
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT); // �ŏ��͋K�莞�ԑ҂��Ȃ�
	}
}

/*! @brief Jump���̍X�V */
void DlgTagJumpList::UpdateData(bool bInit)
{
	HWND	hwndList;
	LV_ITEM	lvi;
	int		nIndex;
	int		count;

	hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);
	ListView_DeleteAllItems(hwndList);

	count = pList->GetCount();

	TCHAR	tmp[32];
	for (nIndex=0; nIndex<count; ++nIndex) {
		SortedTagJumpList::TagJumpInfo* item;
		item = pList->GetPtr(nIndex);
		if (!item) break;

		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = item->keyword;
		ListView_InsertItem(hwndList, &lvi);

		if (item->baseDirId) {
			auto_sprintf( tmp, _T("(%d)"), item->depth );
		}else {
			auto_sprintf( tmp, _T("%d"), item->depth );
		}
		ListView_SetItemText(hwndList, nIndex, 1, tmp);

		auto_sprintf( tmp, _T("%d"), item->no );
		ListView_SetItemText(hwndList, nIndex, 2, tmp);

		TCHAR* p;
		p = GetNameByType(item->type, item->filename);
		ListView_SetItemText(hwndList, nIndex, 3, p);
		free(p);

		ListView_SetItemText(hwndList, nIndex, 4, item->filename);
		ListView_SetItemText(hwndList, nIndex, 5, item->note);
		ListView_SetItemState(hwndList, nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	const TCHAR* pszMsgText = NULL;

	if (!bInit && pList->GetCount() == 0) {
		pszMsgText = LS(STR_DLGTAGJMP2);
	}
	if (pszMsgText) {
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP1));
		lvi.lParam   = -1;
		ListView_InsertItem(hwndList, &lvi);
//		ListView_SetItemText(hwndList, nIndex, 1, _T(""));
//		ListView_SetItemText(hwndList, nIndex, 2, _T(""));
//		ListView_SetItemText(hwndList, nIndex, 3, _T(""));
		ListView_SetItemText(hwndList, nIndex, 4, const_cast<TCHAR*>(pszMsgText));
//		ListView_SetItemText(hwndList, nIndex, 5, _T(""));
	}

	if (IsDirectTagJump() && nTop == 0 && ! bNextItem) {
		// �_�C���N�g�^�O�W�����v�ŁA�y�[�W���O�̕K�v���Ȃ��Ƃ��͔�\��
		::ShowWindow(GetItemHwnd(IDC_BUTTON_NEXTTAG), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_PREVTAG), SW_HIDE);
	}else {
		EnableItem(IDC_BUTTON_NEXTTAG, bNextItem);
		EnableItem(IDC_BUTTON_PREVTAG, (0 < nTop));
	}

	nIndex = SearchBestTag();
	if (nIndex != -1) {
		ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hwndList, nIndex, FALSE);
	}

	return;
}

/*!	�_�C�A���O�f�[�^�̎擾

	@return TRUE: ����, FALSE: ���̓G���[
*/
int DlgTagJumpList::GetData(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);
	nIndex = ListView_GetNextItem(hwndList, -1, LVIS_SELECTED);

	if (!IsDirectTagJump()) {
		pShareData->tagJump.bTagJumpICase = bTagJumpICase;
		pShareData->tagJump.bTagJumpAnyWhere = bTagJumpAnyWhere;
		// ��₪��ł��W�����v�ŕ����Ƃ��́A�I�v�V������ۑ�����
		if (nIndex == -1 || nIndex >= pList->GetCapacity()) {
			return FALSE;
		}
		wchar_t	tmp[MAX_TAG_STRING_LENGTH];
		tmp[0] = 0;
		GetItemText(IDC_KEYWORD, tmp, _countof(tmp));
		SetKeyword(tmp);

		// �ݒ��ۑ�
		RecentTagJumpKeyword cRecentTagJumpKeyword;
		cRecentTagJumpKeyword.AppendItem(pszKeyword);
		cRecentTagJumpKeyword.Terminate();
	}
	// To Here 2005.04.03 MIK
	if (nIndex == -1 || nIndex >= pList->GetCapacity()) return FALSE;

	return TRUE;
}

/*!
	�K�w�J�����̒ǉ��D�L�[���[�h�w�藓�̒ǉ�
*/
BOOL DlgTagJumpList::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;
	BOOL		bRet;

	_SetHwnd(hwndDlg);
	::SetWindowLongPtr(GetHwnd(), DWLP_USER, lParam);

	CreateSizeBox();
	Dialog::OnSize();
	
	::GetWindowRect(hwndDlg, &rc);
	ptDefaultSize.x = rc.right - rc.left;
	ptDefaultSize.y = rc.bottom - rc.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().common.others.rcTagJumpDialog;
	if (0
		|| rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		xPos = rcDialog.left;
		yPos = rcDialog.top;
		nWidth = rcDialog.right - rcDialog.left;
		nHeight = rcDialog.bottom - rcDialog.top;
	}

	// �E�B���h�E�̃��T�C�Y
	SetDialogPosSize();

	// ���X�g�r���[�̕\���ʒu���擾����B
	hwndList = ::GetDlgItem(hwndDlg, IDC_LIST_TAGJUMP);
	//ListView_DeleteAllItems(hwndList);
	rc.left = rc.top = rc.right = rc.bottom = 0;
	::GetWindowRect(hwndList, &rc);
	
	int nWidth = (rc.right - rc.left) - ::GetSystemMetrics(SM_CXHSCROLL) - TextWidthCalc::WIDTH_MARGIN_SCROLLBER;

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 20 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST1));
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_CENTER;
	col.cx       = nWidth * 7 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST2));
	col.iSubItem = 1;
	ListView_InsertColumn(hwndList, 1, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_RIGHT;
	col.cx       = nWidth * 8 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST3));
	col.iSubItem = 2;
	ListView_InsertColumn(hwndList, 2, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 9 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST4));
	col.iSubItem = 3;
	ListView_InsertColumn(hwndList, 3, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 35 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST5));
	col.iSubItem = 4;
	ListView_InsertColumn(hwndList, 4, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 21 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGTAGJMP_LIST6));
	col.iSubItem = 5;
	ListView_InsertColumn(hwndList, 5, &col);

	// �s�I��
	lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	// �_�C���N�g�^�u�W�����v�̎��́A�L�[���[�h���\��
	HWND hwndKey = GetItemHwnd(IDC_KEYWORD);
	int nShowFlag = (IsDirectTagJump() ? SW_HIDE : SW_SHOW);
	::ShowWindow(GetItemHwnd(IDC_STATIC_KEYWORD), nShowFlag);
	::ShowWindow(hwndKey, nShowFlag);
	::ShowWindow(GetItemHwnd(IDC_CHECK_ICASE), nShowFlag);
	::ShowWindow(GetItemHwnd(IDC_CHECK_ANYWHERE), nShowFlag);
	if (IsDirectTagJump()) {
		// �_�C���N�g�^�O�W�����v
		bRet = TRUE;
	}else {
		// �L�[���[�h�w��
		::SetFocus(hwndKey);
		bRet = FALSE;	// for set focus
	}

	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentKeyword;
	SetComboBoxDeleter(hwndKey, &comboDel);

	// ���N���X�����o
	Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
	
	return bRet;
}

BOOL DlgTagJumpList::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// �w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGJUMP_LIST));
		return TRUE;

	case IDOK:			// ���E�ɕ\��
		StopTimer();
		// �_�C�A���O�f�[�^�̎擾
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	case IDCANCEL:
		StopTimer();
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;

	// ���������ݒ�
	case IDC_CHECK_ICASE:
		bTagJumpICase = IsButtonChecked(IDC_CHECK_ICASE);
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT);
		return TRUE;

	case IDC_CHECK_ANYWHERE:
		bTagJumpAnyWhere = IsButtonChecked(IDC_CHECK_ANYWHERE);
		StartTimer(TAGJUMP_TIMER_DELAY_SHORT);
		return TRUE;

	case IDC_BUTTON_NEXTTAG:
		nTop += pList->GetCapacity();
		StopTimer();
		FindNext(false);
		return TRUE;
	case IDC_BUTTON_PREVTAG:
		nTop = t_max(0, nTop - pList->GetCapacity());
		StopTimer();
		FindNext(false);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}


INT_PTR DlgTagJumpList::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);

	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}


BOOL DlgTagJumpList::OnSize(WPARAM wParam, LPARAM lParam)
{
	// ���N���X�����o
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().common.others.rcTagJumpDialog);

	RECT  rc;
	POINT ptNew;
	GetWindowRect(&rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (size_t i=0 ; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), ptDefaultSize, ptNew, rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}


BOOL DlgTagJumpList::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().common.others.rcTagJumpDialog);

	return Dialog::OnMove(wParam, lParam);
}


BOOL DlgTagJumpList::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = ptDefaultSize.y*3;
	return 0;
}


BOOL DlgTagJumpList::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hwndList = GetItemHwnd(IDC_LIST_TAGJUMP);

	// ���ꗗ���X�g�{�b�N�X
	if (hwndList == pNMHDR->hwndFrom) {
		switch (pNMHDR->code) {
		case NM_DBLCLK:
			StopTimer();
			::EndDialog(GetHwnd(), GetData());
			return TRUE;
		}
	}

	// ���N���X�����o
	return Dialog::OnNotify(wParam, lParam);
}

/*!
	�^�C�}�[�o��

	�^�C�}�[���~���C��⃊�X�g���X�V����
*/
BOOL DlgTagJumpList::OnTimer(WPARAM wParam)
{
	StopTimer();

	FindNext(true);

	return TRUE;
}

/*!
	�^�C�}�[�o��

	�^�C�}�[���J�n���C��⃊�X�g���X�V���鏀��������
*/
BOOL DlgTagJumpList::OnCbnEditChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// ���N���X�����o
	return Dialog::OnCbnEditChange(hwndCtl, wID);
}

BOOL DlgTagJumpList::OnCbnSelChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// ���N���X�����o
	return Dialog::OnCbnSelChange(hwndCtl, wID);
}

#if 0
BOOL DlgTagJumpList::OnEnChange(HWND hwndCtl, int wID)
{
	StartTimer();

	// ���N���X�����o
	return Dialog::OnEnChange(hwndCtl, wID);
}
#endif

LPVOID DlgTagJumpList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

#if 0
bool DlgTagJumpList::AddParamA(
	const char* s0,
	const char* s1,
	int n2,
	const char* s3,
	const char* s4,
	int depth,
	int fileBase
	)
{
	if (nIndex == -1) nIndex = 0;	// �K��l

	ClearPrevFindInfo();
	bNextItem = false;
	pList->AddParamA(s0, s1, n2, s3[0], s4, depth, fileBase);

	return true;
}
#endif

bool DlgTagJumpList::GetSelectedFullPathAndLine(
	TCHAR *fullPath,
	int count,
	int* lineNum,
	int* depth
	)
{
	if (pList->GetCount() != 1) {
		if (nIndex == -1 || nIndex >= pList->GetCount()) {
			return false;
		}
	}else {
		nIndex = 0;
	}

	return GetFullPathAndLine( nIndex, fullPath, count, lineNum, depth );
}

/*
	@param lineNum [out] �I�v�V����
	@param depth [out] �I�v�V����
*/
bool DlgTagJumpList::GetFullPathAndLine(
	int index,
	TCHAR* fullPath,
	int count,
	int* lineNum,
	int* depth
	)
{
	TCHAR path[1024];
	TCHAR fileName[1024];
	TCHAR dirFileName[1024];
	int tempDepth = 0;
	SplitPath_FolderAndFile(GetFilePath(), path, NULL);
	AddLastYenFromDirectoryPath(path);
	
	pList->GetParam(index, NULL, fileName, lineNum, NULL, NULL, &tempDepth, dirFileName);
	if (depth) {
		*depth = tempDepth;
	}
	const TCHAR* fileNamePath;
	// �t�@�C�����A�f�B���N�g���w��A��t�@�C���p�X�A�̏��ɓK�p�B�r���Ńt���p�X�Ȃ炻�̂܂܁B
	if (dirFileName[0]) {
		AddLastYenFromDirectoryPath(dirFileName);
		const TCHAR	*p = fileName;
		if (p[0] == _T('\\')) {
			if (p[1] == _T('\\')) {
				auto_strcpy(dirFileName, p);
			}else {
				auto_strcpy(dirFileName, p);
			}
		}else if (_istalpha(p[0]) && p[1] == _T(':')) {
			auto_strcpy(dirFileName, p);
		}else {
			// ���΃p�X�F�A������
			auto_strcat(dirFileName, p);
		}
		fileNamePath = dirFileName;
	}else {
		fileNamePath = fileName;
	}
	bool ret = GetFullPathFromDepth(fullPath, count, path, fileNamePath, tempDepth) != NULL;
	if (ret) {
		DEBUG_TRACE(_T("jump to: %ts\n"), static_cast<const TCHAR*>(fullPath));
	}else {
		DEBUG_TRACE(_T("jump to: error\n"));
	}
	return ret;
}

/*!
	@return �u.ext�v�`���̃^�C�v���B free���邱��
*/
TCHAR* DlgTagJumpList::GetNameByType(const TCHAR type, const TCHAR* name)
{
	const TCHAR* p;
	TCHAR*	token;
	TCHAR	tmp[MAX_TAG_STRING_LENGTH];

	p = _tcsrchr(name, _T('.'));
	if (!p) p = _T(".c");	// ������Ȃ��Ƃ��� ".c" �Ƒz�肷��B
	++p;

	for (int i=0; p_extentions[i]; i+=2) {
		_tcscpy(tmp, p_extentions[i]);
		token = _tcstok(tmp, _T(","));
		while (token) {
			if (_tcsicmp(p, token) == 0) {
				_tcscpy(tmp, p_extentions[i + 1]);
				token = _tcstok(tmp, _T(","));
				while (token) {
					if (token[0] == type) {
						return _tcsdup(&token[2]);
					}
					token = _tcstok(NULL, _T(","));
				}
				return _tcsdup(_T(""));
			}
			token = _tcstok(NULL, _T(","));
		}
	}
	return _tcsdup(_T(""));
}

/*!
	��t�@�C������ݒ�
*/
void DlgTagJumpList::SetFileName(const TCHAR* pszFileName)
{
	assert_warning(pszFileName);
	if (!pszFileName) {
		return;
	}

	if (this->pszFileName) {
		free(this->pszFileName);
	}

	this->pszFileName = _tcsdup(pszFileName);
	
	nLoop = CalcMaxUpDirectory(pszFileName);
}

/*!
	�����L�[���[�h�̐ݒ�

*/
void DlgTagJumpList::SetKeyword(const wchar_t* pszKeyword)
{
	if (!pszKeyword) {
		return;
	}

	if (this->pszKeyword) {
		free(this->pszKeyword);
	}

	this->pszKeyword = wcsdup(pszKeyword);
}

struct TagPathInfo {
	TCHAR	szFileNameDst[_MAX_PATH*4];
	TCHAR	szDriveSrc[_MAX_DRIVE*2];
	TCHAR	szDriveDst[_MAX_DRIVE*2];
	TCHAR	szPathSrc[_MAX_PATH*4];
	TCHAR	szPathDst[_MAX_PATH*4];
	TCHAR	szFileSrc[_MAX_PATH*4];
	TCHAR	szFileDst[_MAX_PATH*4];
	TCHAR	szExtSrc[_MAX_EXT*2];
	TCHAR	szExtDst[_MAX_EXT*2];
	size_t	nDriveSrc;
	size_t	nDriveDst;
	size_t	nPathSrc;
	size_t	nPathDst;
	size_t	nFileSrc;
	size_t	nFileDst;
	size_t	nExtSrc;
	size_t	nExtDst;
};

/*!
	����ꂽ��₩��ł����҂ɋ߂��Ǝv������̂�
	�I�яo���D(�����I���ʒu����̂���)

	@return �I�����ꂽ�A�C�e����index

*/
int DlgTagJumpList::SearchBestTag(void)
{
	if (pList->GetCount() <= 0) {
		return -1;	// �I�ׂ܂���B
	}
	if (!pszFileName) {
		return 0;
	}

	auto lpPathInfo = std::make_unique<TagPathInfo>();
	int nMatch1 = -1;
	int nMatch2 = -1;
	int nMatch3 = -1;
	int nMatch4 = -1;
	int nMatch5 = -1;
	int nMatch6 = -1;
	int nMatch7 = -1;
	int		i;
	int		count;

	lpPathInfo->szDriveSrc[0] = _T('\0');
	lpPathInfo->szPathSrc[0] = _T('\0');
	lpPathInfo->szFileSrc[0] = _T('\0');
	lpPathInfo->szExtSrc[0] = _T('\0');
	_tsplitpath( pszFileName, lpPathInfo->szDriveSrc, lpPathInfo->szPathSrc, lpPathInfo->szFileSrc, lpPathInfo->szExtSrc );
	lpPathInfo->nDriveSrc = _tcslen(lpPathInfo->szDriveSrc);
	lpPathInfo->nPathSrc = _tcslen(lpPathInfo->szPathSrc);
	lpPathInfo->nFileSrc = _tcslen(lpPathInfo->szFileSrc);
	lpPathInfo->nExtSrc = _tcslen(lpPathInfo->szExtSrc);

	count = pList->GetCount();

	for (i=0; i<count; ++i) {
		// �^�O�̃t�@�C�����������t���p�X�ɂ���
		lpPathInfo->szFileNameDst[0] = _T('\0');
		{
			TCHAR szPath[_MAX_PATH];
			GetFullPathAndLine( i, szPath, _countof(szPath), NULL, NULL );
			if (!GetLongFileName(szPath, lpPathInfo->szFileNameDst)) {
				_tcscpy( lpPathInfo->szFileNameDst, szPath );
			}
		}

		lpPathInfo->szDriveDst[0] = _T('\0');
		lpPathInfo->szPathDst[0] = _T('\0');
		lpPathInfo->szFileDst[0] = _T('\0');
		lpPathInfo->szExtDst[0] = _T('\0');
		_tsplitpath( lpPathInfo->szFileNameDst, lpPathInfo->szDriveDst, lpPathInfo->szPathDst, lpPathInfo->szFileDst, lpPathInfo->szExtDst );
		lpPathInfo->nDriveDst = _tcslen(lpPathInfo->szDriveDst);
		lpPathInfo->nPathDst = _tcslen(lpPathInfo->szPathDst);
		lpPathInfo->nFileDst = _tcslen(lpPathInfo->szFileDst);
		lpPathInfo->nExtDst = _tcslen(lpPathInfo->szExtDst);
		
		if (_tcsicmp(pszFileName, lpPathInfo->szFileNameDst) == 0) {
			return i;	//����t�@�C����������
		}

		if ((nMatch1 == -1)
		&& (_tcsicmp(lpPathInfo->szDriveSrc, lpPathInfo->szDriveDst) == 0)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			//�t�@�C�����܂ň�v
			nMatch1 = i;
		}

		if ((nMatch2 == -1)
		&& (_tcsicmp(lpPathInfo->szDriveSrc, lpPathInfo->szDriveDst) == 0)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		) {
			//�p�X���܂ň�v
			nMatch2 = i;
		}

		if ((nMatch5 == -1)
		&& (_tcsicmp(lpPathInfo->szPathSrc, lpPathInfo->szPathDst) == 0)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			nMatch5 = i;
		}

		if ((nMatch6 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			if ((lpPathInfo->nPathSrc >= lpPathInfo->nPathDst)
			&& (_tcsicmp(&lpPathInfo->szPathSrc[lpPathInfo->nPathSrc - lpPathInfo->nPathDst], lpPathInfo->szPathDst) == 0)
			) {
				nMatch6 = i;
			}
		}

		if ((nMatch7 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			if ((lpPathInfo->nPathSrc >= lpPathInfo->nPathDst)
			&& (_tcsicmp(&lpPathInfo->szPathSrc[lpPathInfo->nPathSrc - lpPathInfo->nPathDst], lpPathInfo->szPathDst) == 0)
			) {
				nMatch7 = i;
			}
		}

		if ((nMatch3 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		&& (_tcsicmp(lpPathInfo->szExtSrc, lpPathInfo->szExtDst) == 0)
		) {
			//�t�@�C�����E�g���q����v
			nMatch3 = i;
		}

		if ((nMatch4 == -1)
		&& (_tcsicmp(lpPathInfo->szFileSrc, lpPathInfo->szFileDst) == 0)
		) {
			//�t�@�C��������v
			nMatch4 = i;
		}
	}

	if (nMatch1 != -1) return nMatch1;
	if (nMatch5 != -1) return nMatch5;
	if (nMatch6 != -1) return nMatch6;
	if (nMatch7 != -1) return nMatch7;
	if (nMatch3 != -1) return nMatch3;
	if (nMatch4 != -1) return nMatch4;
	if (nMatch2 != -1) return nMatch2;

	return 0;
}

/*!
	@param bNewFind �V������������(���E�O�̂Ƃ�false)
*/
void DlgTagJumpList::FindNext(bool bNewFind)
{
	wchar_t	szKey[MAX_TAG_STRING_LENGTH];
	szKey[0] = 0;
	GetItemText(IDC_KEYWORD, szKey, _countof(szKey));
	if (bNewFind) {
		// �O��̃L�[���[�h����̍i�������̂Ƃ��ŁAtags���X�L�b�v�ł���Ƃ��̓X�L�b�v
		if (-1 < psFind0Match->nDepth
			&& (bTagJumpAnyWhere == bOldTagJumpAnyWhere || !bTagJumpAnyWhere)
			&& (bTagJumpICase    == bOldTagJumpICase    || !bTagJumpICase)
			&& wcsncmp(strOldKeyword.GetStringPtr(), szKey, strOldKeyword.GetStringLength() == 0)
		) {
			// ���̃L�[���[�h�łP�����q�b�g���Ȃ�tags������̂Ŕ�΂�
			// �����͓������A�������Ȃ�Ȃ���Ȃ�
		}else {
			ClearPrevFindInfo();
		}
		nTop = 0;
	}
	find_key(szKey);
	UpdateData(false);
}

/*!
	�_�C���N�g�^�O�W�����v����(DoModal�O�Ɏ��s)
*/
int DlgTagJumpList::FindDirectTagJump()
{
	return find_key_core(
		0,	// 0�J�n
		pszKeyword,
		false, // ������v
		true,  // ���S��v
		false, // �召�����
		true,  // �������[�h
		pShareData->common.search.nTagJumpMode
	);
}

void DlgTagJumpList::find_key(const wchar_t* keyword)
{
	SetItemText(IDC_STATIC_KEYWORD, LS(STR_DLGTAGJMP3));
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));

	find_key_core(
		nTop,
		keyword,
		bTagJumpAnyWhere,
		bTagJumpExactMatch,
		bTagJumpICase,
		IsDirectTagJump(),
		IsDirectTagJump() ? (pShareData->common.search.nTagJumpMode) : pShareData->common.search.nTagJumpModeKeyword
	);
	SetItemText(IDC_STATIC_KEYWORD, LS(STR_DLGTAGJMP_LIST1));
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));
}

/*!
	�^�O�t�@�C������L�[���[�h�Ƀ}�b�`����f�[�^�𒊏o���Cm_cList�ɐݒ肷��
*/
int DlgTagJumpList::find_key_core(
	int nTop,
	const wchar_t* keyword,
	bool bTagJumpAnyWhere,		// ������v
	bool bTagJumpExactMatch,	// ���S��v
	bool bTagJumpICase,
	bool bTagJumpICaseByTags,	// Tag�t�@�C�����̃\�[�g�ɏ]��
	int  nDefaultNextMode
	)
{
	assert_warning(!(bTagJumpAnyWhere && bTagJumpExactMatch));

	// to_achar�͈ꎞ�o�b�t�@�Ŕj�󂳂��\��������̂ŃR�s�[
	NativeA memKeyA = NativeA(to_achar(keyword));
	const char* paszKeyword = memKeyA.GetStringPtr();
	size_t	length = memKeyA.GetStringLength();

	Empty();

	strOldKeyword.SetString(keyword);
	bOldTagJumpAnyWhere = bTagJumpAnyWhere;
	bOldTagJumpICase    = bTagJumpICase;
	bNextItem = false;

	if (length == 0) {
		ClearPrevFindInfo();
		return -1;
	}
	// �����ϊ����Ă݂Ĉ�v���Ȃ�������A�����L�[�ɂ͈�v���Ȃ��Ƃ������Ƃɂ���
	if (wcscmp(to_wchar(paszKeyword), keyword) != 0) {
		ClearPrevFindInfo();
		return -1;
	}
	SortedTagJumpList& list = *pList;
	const int nCap = list.GetCapacity();
	TagFindState state;
	state.nDepth    = 0;
	state.nMatchAll = 0;
	state.nNextMode = nDefaultNextMode;
	state.nLoop     = -1;
	state.bJumpPath = false;	// �e�ȊO�̃p�X�̈ړ���w��
	state.szCurPath[0] = 0;
	
	// �O��̌��ʂ��猟���Ώ�tags���i��
	if (psFindPrev->nMatchAll <= nTop && -1 < psFindPrev->nMatchAll) {
		// �w��y�[�W�̌������X�L�b�v
		state = *psFindPrev;
		DEBUG_TRACE(_T("skip count  d:%d m:%d n:%d\n"), state.nDepth, state.nMatchAll, state.nNextMode);
	}else if (0 <= psFind0Match->nDepth) {
		// depth���󂢏��Ƀq�b�g���Ȃ����������X�L�b�v
		state = *psFind0Match;
		DEBUG_TRACE(_T("skip 0match d:%d m:%d n:%d\n"), state.nDepth, state.nMatchAll, state.nNextMode);
	}else {
		// ����or�g���Ȃ��Ƃ��̓N���A
		ClearPrevFindInfo();
		// �t�@�C�������R�s�[�������ƁA�f�B���N�g��(�Ō�\)�݂̂ɂ���
		_tcscpy( state.szCurPath, GetFilePath() );
		state.szCurPath[GetFileName() - GetFilePath()] = _T('\0');
		state.nLoop = nLoop;
	}
	
	TCHAR	szTagFile[1024];		// �^�O�t�@�C��
	TCHAR	szNextPath[1024];		// �������t�H���_
	char	szLineData[1024];		// �s�o�b�t�@
	char	s[4][1024];
	int		n2;
	szNextPath[0] = _T('\0');
	vector_ex<std::tstring> seachDirs;

	// �p�X��Jump�ŏz���Ă���ꍇ�ɍő�l���K������
	for (; state.nDepth<=state.nLoop && state.nDepth<(_MAX_PATH/2); state.nDepth++) {
		// 0 ���̃t�@�C���͌������Ȃ�
		// 1 1�ł��q�b�g�����玟�͌������Ȃ�
		// 2 ���S��v�̂Ƃ���1�ɓ����B ����ȊO��3�ɓ���
		// 3 �K����������
		if (state.nNextMode == 0) break;
		if (state.nNextMode == 1 && 0 < state.nMatchAll) break;
		if (state.nNextMode == 2 && bTagJumpExactMatch && 0 < state.nMatchAll) break; 
		{
			std::tstring curPath = state.szCurPath;
			if (seachDirs.exist(curPath)) {
				// �����ς� =>�I��
				break;
			}
			seachDirs.push_back(curPath);
		}

		// �^�O�t�@�C�������쐬����B
		auto_sprintf_s(szTagFile, _T("%ts%ts"), state.szCurPath, TAG_FILENAME_T);
		DEBUG_TRACE(_T("tag: %ts\n"), szTagFile);
		
		// �^�O�t�@�C�����J���B
		FILE* fp = _tfopen(szTagFile, _T("rb"));
		if (fp) {
			DEBUG_TRACE(_T("open tags\n"));
			bool bSorted = true;
			bool bFoldcase = false;
			int  nTagFormat = 2; // 2��1���ǂ߂�̂Ńf�t�H���g��2
			int  nLines = 0;
			int  baseDirId = 0;
			if (state.bJumpPath) {
				baseDirId = list.AddBaseDir(state.szCurPath);
			}
			state.nNextMode = nDefaultNextMode;

			// �o�b�t�@�̌�납��2�����ڂ�\0���ǂ����ŁA�s���܂œǂݍ��񂾂��m�F����
			const int nLINEDATA_LAST_CHAR = _countof(szLineData) - 2;
			szLineData[nLINEDATA_LAST_CHAR] = '\0';
			while (fgets(szLineData, _countof(szLineData), fp)) {
				nLines++;
				int  nRet;
				// fgets���s���ׂĂ�ǂݍ��߂Ă��Ȃ��ꍇ�̍l��
				if (1
					&& szLineData[nLINEDATA_LAST_CHAR] != '\0'
				    && szLineData[nLINEDATA_LAST_CHAR] != '\n'
				) {
					// ���s�R�[�h�܂ł��̂Ă�
					int ch = fgetc(fp);
					while (ch != '\n' && ch != EOF) {
						ch = fgetc(fp);
					}
				}
				if (nLines == 1 && szLineData[0] == '\x0c') {
					// etags�Ȃ̂Ŏ��̃t�@�C��
					break;
				}
				if (szLineData[0] == '!') {
					if (strncmp_literal(szLineData + 1, "_TAG_") == 0) {
						s[0][0] = s[1][0] = s[2][0] = 0;
						nRet = sscanf(
							szLineData, 
							TAG_FILE_INFO_A,	// tags�t�@�C�����
							s[0], s[1], s[2]
						);
						if (nRet < 2) goto next_line;
						const char* pTag = s[0] + 6;
						if (strncmp_literal(pTag , "FILE_FORMAT") == 0) {
							n2 = atoi(s[1]);
							if (1 <= n2 && n2 <= 2) {
								nTagFormat = n2;
							}
						}else if (strncmp_literal(pTag, "FILE_SORTED") == 0) {
							n2 = atoi(s[1]);
							bSorted = (n2 == 1);
							bFoldcase = (n2 == 2);
							if (bTagJumpICaseByTags) {
								bTagJumpICase = bFoldcase;
							}
						}else if (strncmp_literal(pTag, "S_SEARCH_NEXT") == 0) {
							// �Ǝ��g��:���Ɍ�������tag�t�@�C���̎w��
							if ('0' <= s[1][0] && s[1][0] <= '3') {
								n2 = atoi(s[1]);
								if (0 <= n2 && n2 <= 3) {
									state.nNextMode = n2;
								}
								if (1 <= n2 && s[2][0]) {
									// s[2] == ��΃p�X(�f�B���N�g��)
									TCHAR baseWork[1024];
									CopyDirDir(baseWork, to_tchar(s[2]), state.szCurPath);
									szNextPath[0] = 0;
									if (!GetLongFileName(baseWork, szNextPath)) {
										// �G���[�Ȃ�ϊ��O��K�p
										auto_strcpy(szNextPath, baseWork);
									}
								}
							}
						}else if (strncmp_literal(pTag, "S_FILE_BASEDIR") == 0) {
							TCHAR baseWork[1024];
							// �Ǝ��g��:�t�@�C�����̊�f�B���N�g��
							if (state.bJumpPath) {
								// �p�X�e�ǂݑւ����́A���΃p�X�������ꍇ�ɘA�����K�v
								CopyDirDir(baseWork, to_tchar(s[1]), state.szCurPath);
								baseDirId = list.AddBaseDir(baseWork);
							}else {
								auto_strcpy(baseWork, to_tchar(s[1]));
								AddLastYenFromDirectoryPath(baseWork);
								baseDirId = list.AddBaseDir(baseWork);
							}
						}
					}
					goto next_line;	// �R�����g�Ȃ�X�L�b�v
				}
				if (szLineData[0] < '!') {
					goto next_line;
				}
				//chop(szLineData);

				s[0][0] = s[1][0] = s[2][0] = s[3][0] = '\0';
				n2 = 0;
				// TAG_FORMAT�萔��
				if (nTagFormat == 2) {
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_2_A,	// �g��tags�t�H�[�}�b�g
						s[0], s[1], &n2, s[2], s[3]
						);
					if (nRet < 3) goto next_line;
					if (n2 <= 0) goto next_line;	// �s�ԍ��s��(-excmd=n���w�肳��ĂȂ�����)
				}else {
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_1_A,	// tags�t�H�[�}�b�g
						s[0], s[1], &n2
						);
					if (nRet < 2) goto next_line;
					if (n2 <= 0) goto next_line;
				}

				int cmp;
				if (bTagJumpAnyWhere) {
					if (bTagJumpICase) {
						cmp = stristr_j(s[0], paszKeyword) != NULL ? 0 : -1;
					}else {
						cmp = strstr_j(s[0], paszKeyword) != NULL ? 0 : -1;
					}
				}else {
					if (bTagJumpExactMatch) {
						// ���S��v
						if (bTagJumpICase) {
							cmp = auto_stricmp(s[0], paszKeyword);
						}else {
							cmp = auto_strcmp(s[0], paszKeyword);
						}
					}else {
						// �O����v
						if (bTagJumpICase) {
							cmp = my_strnicmp(s[0], paszKeyword, length);
						}else {
							cmp = strncmp(s[0], paszKeyword, length);
						}
					}
				}
				if (cmp == 0) {
					state.nMatchAll++;
					if (nTop < state.nMatchAll) {
						if (list.GetCount() < nCap) {
							list.AddParamA(s[0], s[1], n2, s[2][0], s[3], state.nDepth, baseDirId);
						}else {
							// �T���ł��؂�(���y�[�W�ł�蒼��)
							bNextItem = true;
							break;
						}
					}
				}else if (0 < cmp) {
					// tags�̓\�[�g����Ă���̂ŁC�擪�����case sensitive��
					// ��r���ʂɂ���Č����̎��͏����̑ł��؂肪�\
					if ((!bTagJumpICase) && bSorted && (!bTagJumpAnyWhere)) break;
					// Foldcase�����ł��؂�B������tags�ƃT�N�����̃\�[�g���������łȂ���΂Ȃ�Ȃ�
					if (bTagJumpICase  && bFoldcase && (!bTagJumpAnyWhere)) break;
				}
next_line:
				;
				szLineData[nLINEDATA_LAST_CHAR] = '\0';
			}

			// �t�@�C�������B
			fclose(fp);
			DEBUG_TRACE(_T("close m:%d\n "), state.nMatchAll);
		}
		
		if (szNextPath[0]) {
			state.bJumpPath = true;
			auto_strcpy(state.szCurPath, szNextPath);
			std::tstring path = state.szCurPath;
			path += _T("\\dummy");
			state.nLoop = CalcMaxUpDirectory( path.c_str() );
			state.nDepth = 0;
			szNextPath[0] = 0;
		}else {
//			_tcscat(state.szCurPath, _T("..\\"));
			// �J�����g�p�X��1�K�w��ցB
			DirUp(state.szCurPath);
		}
		
		if (state.nMatchAll != 0 && !bNextItem) {
			// 0 �y�[�W�߂���p: �ł��؂��Ă��Ȃ��̂Ŏ��̃y�[�W�ł́A����tags�̎����猟���ł���
			// (�Ō�ɒʉ߂������̂�ێ�)
			*psFindPrev = state;
			++(psFindPrev->nDepth);
			DEBUG_TRACE(_T("FindPrev udpate: d:%d m:%d n:%d L:%d j:%d\n") , psFindPrev->nDepth, psFindPrev->nMatchAll, psFindPrev->nNextMode, psFindPrev->nLoop, (int)psFindPrev->bJumpPath);
		}
		if (state.nMatchAll == 0) {
			// �L�[���[�h�i���ݗp: ���̍i�荞�݌����ł́A����tags�̎����猟���ł���
			// (�Ō�ɒʉ߂������̂�ێ�)
			*psFind0Match = state;
			++(psFind0Match->nDepth);
			DEBUG_TRACE(_T("Find0Match udpate: d:%d m:%d n:%d L:%d j:%d\n") , psFind0Match->nDepth, psFind0Match->nMatchAll, psFind0Match->nNextMode, psFind0Match->nLoop, (int)psFind0Match->bJumpPath);
		}
		if (bNextItem) {
			break;
		}
	}
	return state.nMatchAll;
}

/*!
	�p�X����t�@�C���������݂̂����o���D(2�o�C�g�Ή�)
*/
const TCHAR* DlgTagJumpList::GetFileName(void)
{
	return GetFileTitlePointer(GetFilePath());
}


void DlgTagJumpList::SetTextDir()
{
	if (GetHwnd()) {
		SetItemText(IDC_STATIC_BASEDIR, _T(""));
		if (GetFileName()) {
			std::tstring strPath = GetFilePath();
			strPath[GetFileName() - GetFilePath()] = _T('\0');
			SetItemText(IDC_STATIC_BASEDIR, strPath.c_str());
		}
	}
}

int DlgTagJumpList::CalcMaxUpDirectory(const TCHAR* p)
{
	int loop = CalcDirectoryDepth(p);
	if (loop <  0) loop =  0;
	if (loop > (_MAX_PATH/2)) loop = (_MAX_PATH/2);	// \A\B\C...�̂悤�ȂƂ�1�t�H���_��2���������̂�...
	return loop;
}

/*!
	@param basePath [in,out] \�t�f�B���N�g���p�X��΃p�X�����B���������̂ɒ���
	@param fileName [in] ���΁E��΃t�@�C�����p�X
	@param depth    [in] fineName����΃p�X�̎������B1==1��̃f�B���N�g��
	@retval pszOutput ���� �uC:\dir1\filename.txt�v�̌`��(..\�t���͔p�~)
	@retval NULL   ���s
*/
TCHAR* DlgTagJumpList::GetFullPathFromDepth(TCHAR* pszOutput, int count,
	TCHAR* basePath, const TCHAR* fileName, int depth)
{
	DEBUG_TRACE(_T("base  %ts\n"), basePath);
	DEBUG_TRACE(_T("file  %ts\n"), fileName);
	DEBUG_TRACE(_T("depth %d\n"),  depth);
	// ���S�p�X�����쐬����B
	const TCHAR	*p = fileName;
	if (p[0] == _T('\\')) {		// �h���C�u�Ȃ���΃p�X���H
		if (p[1] == _T('\\')) {	// �l�b�g���[�N�p�X���H
			_tcscpy(pszOutput, p);	// �������H���Ȃ��B
		}else {
			// �h���C�u���H�����ق����悢�H
			_tcscpy(pszOutput, p);	// �������H���Ȃ��B
		}
	}else if (_istalpha(p[0]) && p[1] == _T(':')) {	// ��΃p�X���H
		_tcscpy(pszOutput, p);	// �������H���Ȃ��B
	}else {
		for (int i=0; i<depth; ++i) {
			//_tcscat(basePath, _T("..\\"));
			DirUp(basePath);
		}
		if (auto_snprintf_s(pszOutput, count, _T("%ts%ts"), basePath, p) == -1) {
			return NULL;
		}
	}
	return pszOutput;
}

/*!
	�f�B���N�g���ƃf�B���N�g����A������
*/
TCHAR* DlgTagJumpList::CopyDirDir(TCHAR* dest, const TCHAR* target, const TCHAR* base)
{
	if (_IS_REL_PATH(target)) {
		auto_strcpy(dest, base);
		AddLastYenFromDirectoryPath(dest);
		auto_strcat(dest, target);
	}else {
		auto_strcpy(dest, target);
	}
	AddLastYenFromDirectoryPath(dest);
	return dest;
}

/*
	@param dir [in,out] �t�H���_�̃p�X 
	in == C:\dir\subdir\
	out == C:\dir\
*/
TCHAR* DlgTagJumpList::DirUp(TCHAR* dir)
{
	CutLastYenFromDirectoryPath(dir);
	const TCHAR* p = GetFileTitlePointer(dir); // �Ō��\�̎��̕������擾 last_index_of('\\') + 1;
	if (0 < p - dir) {
		dir[p - dir] = '\0';
	}
	return dir;
}

