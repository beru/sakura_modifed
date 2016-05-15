/*
	Copyright (C) 2007, kobake, ryoji
	Copyright (C) 2008, kobake
	Copyright (C) 2010, Uchi, Moca
	Copyright (C) 2012, aroka, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "window/MainToolBar.h"
#include "window/EditWnd.h"
#include "EditApp.h"
#include "util/os.h"
#include "util/tchar_receive.h"
#include "util/window.h"
#include "uiparts/ImageListMgr.h"

MainToolBar::MainToolBar(EditWnd& owner)
	:
	owner(owner),
	hwndToolBar(NULL),
	hwndReBar(NULL),
	hwndSearchBox(NULL),
	hFontSearchBox(NULL),
	pIcons(nullptr)
{
}

void MainToolBar::Create(ImageListMgr* pIcons)
{
	this->pIcons = pIcons;
}

// �����{�b�N�X�ł̏���
void MainToolBar::ProcSearchBox(MSG *msg)
{
	if (msg->message == WM_KEYDOWN /* && ::GetParent(msg->hwnd) == hwndSearchBox */) {
		if (msg->wParam == VK_RETURN) {  // ���^�[���L�[
			// �����L�[���[�h���擾
			std::wstring strText;
			if (0 < GetSearchKey(strText)) {	// �L�[�����񂪂���
				if (strText.size() < _MAX_PATH) {
					// �����L�[��o�^
					SearchKeywordManager().AddToSearchKeys(strText.c_str());
				}
				owner.GetActiveView().strCurSearchKey = strText;
				owner.GetActiveView().bCurSearchUpdate = true;
				owner.GetActiveView().ChangeCurRegexp();

				// �����{�b�N�X���X�V	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus(hWnd);	//��Ƀt�H�[�J�X���ړ����Ă����Ȃ��ƃL�����b�g��������
				owner.GetActiveView().SetFocus();

				// �����J�n���̃J�[�\���ʒu�o�^������ύX 02/07/28 ai start
				owner.GetActiveView().ptSrchStartPos_PHY = owner.GetActiveView().GetCaret().GetCaretLogicPos();
				// 02/07/28 ai end

				// ��������
				owner.OnCommand((WORD)0 /*���j���[*/, (WORD)F_SEARCH_NEXT, (HWND)0);
			}
		}else if (msg->wParam == VK_TAB) {	// �^�u�L�[
			// �t�H�[�J�X���ړ�
			// 2004.10.27 MIK IME�\���ʒu�̂���C��
			::SetFocus(owner.GetHwnd() );
		}
	}
}

/*! �T�u�N���X�������c�[���o�[�̃E�B���h�E�v���V�[�W��
	@author ryoji
	@date 2006.09.06 ryoji
*/
static WNDPROC g_pOldToolBarWndProc;	// �c�[���o�[�̖{���̃E�B���h�E�v���V�[�W��

static LRESULT CALLBACK ToolBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	// WinXP Visual Style �̂Ƃ��Ƀc�[���o�[��ł̃}�E�X���E�{�^�����������Ŗ������ɂȂ�
	//�i�}�E�X���L���v�`���[�����܂ܕ����Ȃ��j ����������邽�߂ɉE�{�^���𖳎�����
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// �E�{�^���� UP/DOWN �͖{���̃E�B���h�E�v���V�[�W���ɓn���Ȃ�

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc);
		break;
	}
	return ::CallWindowProc(g_pOldToolBarWndProc, hWnd, msg, wParam, lParam);
}


/* �c�[���o�[�쐬
	@date @@@ 2002.01.03 YAZAKI tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2005.08.29 aroka �c�[���o�[�̐܂�Ԃ�
	@date 2006.06.17 ryoji �r�W���A���X�^�C�����L���̏ꍇ�̓c�[���o�[�� Rebar �ɓ���ăT�C�Y�ύX���̂�����𖳂���
*/
void MainToolBar::CreateToolBar(void)
{
	if (hwndToolBar)
		return;
	
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	LONG_PTR		lToolType;
	nFlag = 0;

	auto& csToolBar = GetDllShareData().common.toolBar;
	// 2006.06.17 ryoji
	// Rebar �E�B���h�E�̍쐬
	if (IsVisualStyle()) {	// �r�W���A���X�^�C���L��
		hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, // ���o�[�R���g���[��
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |	// 2007.03.08 ryoji WS_VISIBLE ����
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			owner.GetHwnd(),
			NULL,
			EditApp::getInstance().GetAppInstance(),
			NULL
		);

		if (!hwndReBar) {
			TopWarningMessage(owner.GetHwnd(), LS(STR_ERR_DLGEDITWND04));
			return;
		}

		if (csToolBar.bToolBarIsFlat) {	// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
			PreventVisualStyle(hwndReBar);	// �r�W���A���X�^�C����K�p�̃t���b�g�� Rebar �ɂ���
		}

		REBARINFO rbi = {0};
		rbi.cbSize = sizeof(rbi);
		Rebar_SetbarInfo(hwndReBar, &rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// �c�[���o�[�ւ̒ǉ��X�^�C��
	}

	// �c�[���o�[�E�B���h�E�̍쐬
	hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */	// 2006.06.17 ryoji WS_CLIPCHILDREN �ǉ�	// 2007.03.08 ryoji WS_VISIBLE ����
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		owner.GetHwnd(),
		(HMENU)ID_TOOLBAR,
		EditApp::getInstance().GetAppInstance(),
		NULL
	);
	if (!hwndToolBar) {
		if (csToolBar.bToolBarIsFlat) {	// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
			csToolBar.bToolBarIsFlat = false;
		}
		TopWarningMessage(owner.GetHwnd(), LS(STR_ERR_DLGEDITWND05));
		DestroyToolBar();	// 2006.06.17 ryoji
	}else {
		// 2006.09.06 ryoji �c�[���o�[���T�u�N���X������
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		Toolbar_SetButtonSize(hwndToolBar, DpiScaleX(22), DpiScaleY(22));	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
		Toolbar_ButtonStructSize(hwndToolBar, sizeof(TBBUTTON));
		//	Oct. 12, 2000 genta
		//	���ɗp�ӂ���Ă���Image List���A�C�R���Ƃ��ēo�^
		pIcons->SetToolBarImages(hwndToolBar);
		// �c�[���o�[�Ƀ{�^����ǉ�
		int count = 0;	//@@@ 2002.06.15 MIK
		int nToolBarButtonNum = 0;// 2005/8/29 aroka
		//	From Here 2005.08.29 aroka
		// �͂��߂Ƀc�[���o�[�\���̂̔z�������Ă���
		std::vector<TBBUTTON> tbButtons(csToolBar.nToolBarButtonNum);
		TBBUTTON* pTbbArr = &tbButtons[0];
		for (i=0; i<csToolBar.nToolBarButtonNum; ++i) {
			nIdx = csToolBar.nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = owner.GetMenuDrawer().getButton(nIdx);
			// �Z�p���[�^�������Ƃ��͂ЂƂɂ܂Ƃ߂�
			// �܂�Ԃ��{�^����TBSTYLE_SEP�����������Ă���̂�
			// �܂�Ԃ��̑O�̃Z�p���[�^�͑S�č폜�����D
			if ((pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum != 0)) {
				if ((pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP)) {
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					--nToolBarButtonNum;
				}
			}
			// ���z�ܕԂ��{�^���������璼�O�̃{�^���ɐܕԂ�������t����
			if (pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP) {
				if (nToolBarButtonNum != 0) {
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			++nToolBarButtonNum;
		}
		//	To Here 2005.08.29 aroka

		for (i=0; i<nToolBarButtonNum; ++i) {
			tbb = pTbbArr[i];

			//@@@ 2002.06.15 MIK start
			switch (tbb.fsStyle) {
			case TBSTYLE_DROPDOWN:	// �h���b�v�_�E��
				// �g���X�^�C���ɐݒ�
				Toolbar_SetExtendedStyle(hwndToolBar, TBSTYLE_EX_DRAWDDARROWS);
				Toolbar_AddButtons(hwndToolBar, 1, &tbb);
				++count;
				break;

			case TBSTYLE_COMBOBOX:	// �R���{�{�b�N�X
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT			lf;

					switch (tbb.idCommand) {
					case F_SEARCH_BOX:
						if (hwndSearchBox) {
							break;
						}
						
						// �Z�p���[�^���
						memset_raw(&my_tbb, 0, sizeof(my_tbb));
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  // �{�^���ɂ��Ȃ��ƕ`�悪����� 2005/8/29 aroka
						my_tbb.idCommand = tbb.idCommand;	// ����ID�ɂ��Ă���
						if (tbb.fsState & TBSTATE_WRAP) {   // �܂�Ԃ� 2005/8/29 aroka
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						Toolbar_AddButtons(hwndToolBar, 1, &my_tbb);
						++count;

						// �T�C�Y��ݒ肷��
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = (WORD)DpiScaleX(160);	// �{�b�N�X�̕�	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
						Toolbar_SetButtonInfo(hwndToolBar, tbb.idCommand, &tbi);

						// �ʒu�ƃT�C�Y���擾����
						rc.right = rc.left = rc.top = rc.bottom = 0;
						Toolbar_GetItemRect(hwndToolBar, count-1, &rc);

						// �R���{�{�b�N�X�����
						// Mar. 8, 2003 genta �����{�b�N�X��1�h�b�g���ɂ��炵��
						hwndSearchBox = CreateWindow(_T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, EditApp::getInstance().GetAppInstance(), NULL);
						if (hwndSearchBox) {
							owner.SetCurrentFocus(0);

							lf = owner.GetLogfont();
							//memset_raw(&lf, 0, sizeof(lf));
							lf.lfHeight			= DpiPointsToPixels(-9); // Jan. 14, 2003 genta �_�C�A���O�ɂ��킹�Ă�����Ə�����	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							//lf.lfCharSet		= GetDllShareData().common.sView.lf.lfCharSet;
							lf.lfOutPrecision	= OUT_TT_ONLY_PRECIS;		// Raster Font ���g��Ȃ��悤��
							//lf.lfClipPrecision	= GetDllShareData().common.sView.lf.lfClipPrecision;
							//lf.lfQuality		= GetDllShareData().common.sView.lf.lfQuality;
							//lf.lfPitchAndFamily	= GetDllShareData().common.sView.lf.lfPitchAndFamily;
							//_tcsncpy(lf.lfFaceName, GetDllShareData().common.sView.lf.lfFaceName, _countof(lf.lfFaceName));	// ��ʂ̃t�H���g�ɐݒ�	2012/11/27 Uchi
							hFontSearchBox = ::CreateFontIndirect(&lf);
							if (hFontSearchBox) {
								::SendMessage(hwndSearchBox, WM_SETFONT, (WPARAM)hFontSearchBox, MAKELONG (TRUE, 0));
							}

							// ���͒�����
							// Combo_LimitText(hwndSearchBox, (WPARAM)_MAX_PATH - 1);

							// �����{�b�N�X���X�V	// �֐��� 2010/6/6 Uchi
							AcceptSharedSearchKey();

							comboDel = ComboBoxItemDeleter(); // �ĕ\���p�̏�����
							comboDel.pRecent = &recentSearch;
							Dialog::SetComboBoxDeleter(hwndSearchBox, &comboDel);
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	// �{�^��
			case TBSTYLE_SEP:		// �Z�p���[�^
			default:
				Toolbar_AddButtons(hwndToolBar, 1, &tbb);
				++count;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if (csToolBar.bToolBarIsFlat) {	// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
			lToolType = ::GetWindowLongPtr(hwndToolBar, GWL_STYLE);
			lToolType |= (TBSTYLE_FLAT);
			::SetWindowLongPtr(hwndToolBar, GWL_STYLE, lToolType);
			::InvalidateRect(hwndToolBar, NULL, TRUE);
		}
	}

	// 2006.06.17 ryoji
	// �c�[���o�[�� Rebar �ɓ����
	if (hwndReBar && hwndToolBar) {
		// �c�[���o�[�̍������擾����
		DWORD dwBtnSize = Toolbar_GetButtonSize(hwndToolBar);
		DWORD dwRows = Toolbar_GetRows(hwndToolBar);

		// �o���h����ݒ肷��
		// �ȑO�̃v���b�g�t�H�[���� _WIN32_WINNT >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����	// 2007.12.21 ryoji
		rbBand.cbSize = CCSIZEOF_STRUCT(REBARBANDINFO, wID);
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = hwndToolBar;	// �c�[���o�[
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// �o���h��ǉ�����
		Rebar_InsertBand(hwndReBar, -1, &rbBand);
		::ShowWindow(hwndToolBar, SW_SHOW);
	}

	return;
}

void MainToolBar::DestroyToolBar(void)
{
	if (hwndToolBar) {
		if (hwndSearchBox) {
			if (hFontSearchBox) {
				::DeleteObject(hFontSearchBox);
				hFontSearchBox = NULL;
			}

			::DestroyWindow(hwndSearchBox);
			hwndSearchBox = NULL;

			owner.SetCurrentFocus(0);
		}

		::DestroyWindow(hwndToolBar);
		hwndToolBar = NULL;

		//if (cTabWnd.owner->GetHwnd()) ::UpdateWindow(cTabWnd.owner->GetHwnd());
	}

	// 2006.06.17 ryoji Rebar ��j������
	if (hwndReBar) {
		::DestroyWindow(hwndReBar);
		hwndReBar = NULL;
	}

	return;
}

// ���b�Z�[�W�����B�Ȃ񂩏��������Ȃ� true ��Ԃ��B
bool MainToolBar::EatMessage(MSG* msg)
{
	if (hwndSearchBox && ::IsDialogMessage(hwndSearchBox, msg)) {	// �����R���{�{�b�N�X
		ProcSearchBox(msg);
		return true;
	}
	return false;
}


/*!	@brief ToolBar��OwnerDraw

	@param pnmh [in] Owner Draw���

	@note Common Control V4.71�ȍ~��NMTBCUSTOMDRAW�𑗂��Ă��邪�C
	Common Control V4.70��LPNMCUSTOMDRAW���������Ă��Ȃ��̂�
	���S�̂��ߏ��������ɍ��킹�ď������s���D
	
	@author genta
	@date 2003.07.21 �쐬

*/
LPARAM MainToolBar::ToolBarOwnerDraw(LPNMCUSTOMDRAW pnmh)
{
	switch (pnmh->dwDrawStage) {
	case CDDS_PREPAINT:
		// �`��J�n�O
		// �A�C�e�������O�ŕ`�悷��|��ʒm����
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		// �ʓ|�������̂ŁC�g��Toolbar�ɕ`���Ă��炤
		// �A�C�R�����o�^����Ă��Ȃ��̂Œ��g�͉����`����Ȃ�
		// 2010.07.15 Moca ����(�{�b�N�X)�Ȃ�g��`���Ȃ�
		if (pnmh->dwItemSpec == F_SEARCH_BOX) {
			return CDRF_SKIPDEFAULT;
		}
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			// �`��
			// �R�}���h�ԍ��ipnmh->dwItemSpec�j����A�C�R���ԍ����擾����	// 2007.11.02 ryoji
			int nIconId = Toolbar_GetBitmap(pnmh->hdr.hwndFrom, (WPARAM)pnmh->dwItemSpec);

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - pIcons->GetCy()) / 2;		// �A�C�e����`����̉摜�̃I�t�Z�b�g	// 2007.03.25 ryoji
			int shift = (pnmh->uItemState & (CDIS_SELECTED | CDIS_CHECKED)) ? 1 : 0;	//	Aug. 30, 2003 genta �{�^���������ꂽ�炿����Ɖ摜�����炷

			//	Sep. 6, 2003 genta �������͉E�����łȂ����ɂ����炷
			pIcons->Draw(nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}


/*! �c�[���o�[�X�V�p�^�C�}�[�̏���
	@date 2002.01.03 YAZAKI tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka OnTimer���番��
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	@date 2008.10.05 nasukoji �c�[���o�[�X�V�������O�ɏo����
	@date 2012.11.29 aroka OnTimer���番�������Ƃ��̃o�O�C��
*/
void MainToolBar::OnToolbarTimer(void)
{
	// 2012.11.29 aroka �����ł̓J�E���g�A�b�v�s�v
	// owner->IncrementTimerCount(10);
	UpdateToolbar();	// 2008.09.23 nasukoji	�c�[���o�[�̕\�����X�V����
}

/*!
	@brief �c�[���o�[�̕\�����X�V����
	
	@note ������Ăׂ�悤��OnToolbarTimer()���؂�o����
	
	@date 2008.10.05 nasukoji
*/
void MainToolBar::UpdateToolbar(void)
{
	// ���Preview���Ȃ�A�������Ȃ��B
	if (owner.IsInPreviewMode())
		return;
	
	// �c�[���o�[�̏�ԍX�V
	if (hwndToolBar) {
		auto& csToolBar = GetDllShareData().common.toolBar;
		for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
			TBBUTTON tbb = owner.GetMenuDrawer().getButton(
				csToolBar.nToolBarButtonIdxArr[i]
			);

			// �@�\�����p�\�����ׂ�
			Toolbar_EnableButton(
				hwndToolBar,
				tbb.idCommand,
				IsFuncEnable(owner.GetDocument(), GetDllShareData(), (EFunctionCode)tbb.idCommand)
			);

			// �@�\���`�F�b�N��Ԃ����ׂ�
			Toolbar_CheckButton(
				hwndToolBar,
				tbb.idCommand,
				IsFuncChecked(owner.GetDocument(), GetDllShareData(), (EFunctionCode)tbb.idCommand)
			);
		}
	}
}

// �����{�b�N�X���X�V
void MainToolBar::AcceptSharedSearchKey()
{
	if (hwndSearchBox) {
		int	i;
		// 2013.05.28 Combo_ResetContent���Ƃ�����̂�DeleteString�Ń��X�g�����폜
		while (Combo_GetCount(hwndSearchBox) > 0) {
			Combo_DeleteString(hwndSearchBox, 0);
		}
		int nSize = GetDllShareData().searchKeywords.searchKeys.size();
		for (i=0; i<nSize; ++i) {
			Combo_AddString(hwndSearchBox, GetDllShareData().searchKeywords.searchKeys[i]);
		}
		const wchar_t* pszText;
		if (GetDllShareData().common.search.bInheritKeyOtherView
			&& owner.GetActiveView().nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
			|| owner.GetActiveView().strCurSearchKey.size() == 0
		) {
			if (0 < nSize) {
				pszText = GetDllShareData().searchKeywords.searchKeys[0];
			}else {
				pszText = L"";
			}
		}else {
			pszText = owner.GetActiveView().strCurSearchKey.c_str();
		}
		std::wstring strText;
		GetSearchKey(strText);
		if (0 < nSize && wcscmp(strText.c_str(), pszText) != 0) {
			::SetWindowText(hwndSearchBox, to_tchar(pszText));
		}
	}
}

int MainToolBar::GetSearchKey(std::wstring& strText)
{
	if (hwndSearchBox) {
		int nBufferSize = ::GetWindowTextLength(hwndSearchBox) + 1;
		std::vector<TCHAR> vText(nBufferSize);

		::GetWindowText(hwndSearchBox, &vText[0], vText.size());
		strText = to_wchar(&vText[0]);
	}else {
		strText = L"";
	}
	return strText.length();
}


/*!
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void MainToolBar::SetFocusSearchBox(void) const
{
	if (hwndSearchBox) {
		::SetFocus(hwndSearchBox);
	}
}

