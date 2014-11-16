/*!	@file
@brief CViewCommander�N���X�̃R�}���h(Diff)�֐��Q

	2007.10.25 kobake CEditView_Diff���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2002, YAZAKI, genta, MIK
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, genta
	Copyright (C) 2005, maru
	Copyright (C) 2007, kobake
	Copyright (C) 2008, kobake
	Copyright (C) 2008, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "dlg/CDlgCompare.h"
#include "dlg/CDlgDiff.h"
#include "util/window.h"
#include "util/os.h"


// �t�@�C�����e��r
void CViewCommander::Command_COMPARE(void)
{
	HWND		hwndCompareWnd;
	TCHAR		szPath[_MAX_PATH + 1];
	CDlgCompare	cDlgCompare;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK
	auto& commonSetting = GetDllShareData().m_Common;
	auto& csCompare = commonSetting.m_sCompare;
	// ��r��A���E�ɕ��ׂĕ\��
	cDlgCompare.m_bCompareAndTileHorz = csCompare.m_bCompareAndTileHorz;
	BOOL bDlgCompareResult = cDlgCompare.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		GetDocument()->m_cDocFile.GetFilePath(),
		GetDocument()->m_cDocEditor.IsModified(),
		szPath,
		&hwndCompareWnd
	);
	if (!bDlgCompareResult) {
		return;
	}
	// ��r��A���E�ɕ��ׂĕ\��
	csCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	// �^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if (commonSetting.m_sTabBar.m_bDispTabWnd
		&& !commonSetting.m_sTabBar.m_bDispTabWndMultiWin
	) {
		hwndMsgBox = m_pCommanderView->GetHwnd();
		csCompare.m_bCompareAndTileHorz = FALSE;
	}else {
		hwndMsgBox = hwndCompareWnd;
	}

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint	poSrc;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&poSrc
	);

	// �J�[�\���ʒu�擾 -> poDes
	CMyPoint poDes;
	{
		::SendMessageAny(hwndCompareWnd, MYWM_GETCARETPOS, 0, 0);
		CLogicPoint* ppoCaretDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	BOOL bDefferent = TRUE;
	CLogicInt nLineLenSrc;
	const wchar_t* pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
	// �s(���s�P��)�f�[�^�̗v��
	int nLineLenDes = ::SendMessageAny(hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0);
	const wchar_t* pLineDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
	for (;;) {
		if (!pLineSrc && 0 == nLineLenDes) {
			bDefferent = FALSE;
			break;
		}
		if (!pLineSrc || 0 == nLineLenDes) {
			break;
		}
		if (nLineLenDes > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>()) {
			TopErrorMessage(m_pCommanderView->GetHwnd(),
				LS(STR_ERR_CMPERR), // "��r��̃t�@�C��\n%ts\n%d�����𒴂���s������܂��B\n��r�ł��܂���B"
				szPath,
				GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>()
			);
			return;
		}
		for (; poSrc.x < nLineLenSrc;) {
			if (poDes.x >= nLineLenDes) {
				goto end_of_compare;
			}
			if (pLineSrc[poSrc.x] != pLineDes[poDes.x]) {
				goto end_of_compare;
			}
			poSrc.x++;
			poDes.x++;
		}
		if (poDes.x < nLineLenDes) {
			goto end_of_compare;
		}
		poSrc.x = 0;
		poSrc.y++;
		poDes.x = 0;
		poDes.y++;
		pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
		// �s(���s�P��)�f�[�^�̗v��
		nLineLenDes = ::SendMessageAny(hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0);
	}
end_of_compare:;
	// ��r��A���E�ɕ��ׂĕ\��
// From Here Oct. 10, 2000 JEPRO	�`�F�b�N�{�b�N�X���{�^��������Έȉ��̍s(To Here �܂�)�͕s�v�̂͂�����
// ���܂������Ȃ������̂Ō��ɖ߂��Ă���c
	if (GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz) {
		HWND hWnds[2];
		hWnds[0] = GetMainWindow();
		hWnds[1] = hwndCompareWnd;
		
		for (int i = 0; i < 2; ++i) {
			if (::IsZoomed(hWnds[i])) {
				::ShowWindow(hWnds[i], SW_RESTORE);
			}
		}
		// �f�X�N�g�b�v�T�C�Y�𓾂� 2002.1.24 YAZAKI
		RECT rcDesktop;
		// May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect(hWnds[0], &rcDesktop);
		int width = (rcDesktop.right - rcDesktop.left) / 2;
		for (int i = 1; i >= 0; i--) {
			::SetWindowPos(
				hWnds[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows(NULL, MDITILE_VERTICAL, NULL, 2, hWnds);
	}
// To Here Oct. 10, 2000

	// 2002/05/11 YAZAKI �e�E�B���h�E�����܂��ݒ肵�Ă݂�B
	if (!bDefferent) {
		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD22));
	}else {
//		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		memcpy_raw(GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poDes, sizeof(poDes));
		::SendMessageAny(hwndCompareWnd, MYWM_SETCARETPOS, 0, 0);

		// �J�[�\�����ړ�������
		memcpy_raw(GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poSrc, sizeof(poSrc));
		::PostMessageAny(GetMainWindow(), MYWM_SETCARETPOS, 0, 0);
		TopWarningMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));	// �ʒu��ύX���Ă��烁�b�Z�[�W	2008/4/27 Uchi
	}

	// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(GetMainWindow());
	return;
}


/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	maru
	@date	2005/10/28 ����܂ł�Command_Diff��m_pCommanderView->ViewDiffInfo�ɖ��̕ύX
*/
void CViewCommander::Command_Diff(const WCHAR* _szTmpFile2, int nFlgOpt)
{
	const TCHAR* szTmpFile2 = to_tchar(_szTmpFile2);

	bool bTmpFile1 = false;
	TCHAR szTmpFile1[_MAX_PATH * 2];

	if ((DWORD)-1 == ::GetFileAttributes(szTmpFile2)) {
		WarningMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF1));
		return;
	}

	// ���t�@�C��
	if (!GetDocument()->m_cDocEditor.IsModified()) {
		_tcscpy_s(szTmpFile1, GetDocument()->m_cDocFile.GetFilePath());
	}else if (m_pCommanderView->MakeDiffTmpFile(szTmpFile1, NULL)) {
		bTmpFile1 = true;
	}else return;

	// �����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt);

	// �ꎞ�t�@�C�����폜����
	if (bTmpFile1) {
		_tunlink(szTmpFile1);
	}

	return;
}


/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�����)
	@author	MIK
	@date	2002/05/25
	@date	2002/11/09 �ҏW���t�@�C��������
	@date	2005/10/29 maru �ꎞ�t�@�C���쐬������m_pCommanderView->MakeDiffTmpFile�ֈړ�
*/
void CViewCommander::Command_Diff_Dialog(void)
{
	CDlgDiff cDlgDiff;
	bool bTmpFile1 = false, bTmpFile2 = false;

	auto& docFile = GetDocument()->m_cDocFile;
	auto& docEditor = GetDocument()->m_cDocEditor;
	// DIFF�����\���_�C�A���O��\������
	int nDiffDlgResult = cDlgDiff.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		docFile.GetFilePath(),
		docEditor.IsModified()
	);
	if (!nDiffDlgResult) {
		return;
	}
	
	// ���t�@�C��
	TCHAR szTmpFile1[_MAX_PATH * 2];
	if (!docEditor.IsModified()) _tcscpy_s(szTmpFile1, docFile.GetFilePath());
	else if (m_pCommanderView->MakeDiffTmpFile(szTmpFile1, NULL)) bTmpFile1 = true;
	else return;
		
	// ����t�@�C��
	TCHAR szTmpFile2[_MAX_PATH * 2];
	if (!cDlgDiff.m_bIsModifiedDst) _tcscpy_s(szTmpFile2, cDlgDiff.m_szFile2);
	else if (m_pCommanderView->MakeDiffTmpFile (szTmpFile2, cDlgDiff.m_hWnd_Dst)) bTmpFile2 = true;
	else {
		if (bTmpFile1) _tunlink(szTmpFile1);
		return;
	}
	
	// �����\��
	m_pCommanderView->ViewDiffInfo(szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt);
	
	// �ꎞ�t�@�C�����폜����
	if (bTmpFile1) _tunlink(szTmpFile1);
	if (bTmpFile2) _tunlink(szTmpFile2);

	return;
}



//	���̍�����T���C����������ړ�����
void CViewCommander::Command_Diff_Next(void)
{
	BOOL bFound = FALSE;
	BOOL bRedo = TRUE;

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld_Logic = ptXY.y;
	CLogicInt tmp_y;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();

re_do:;	
	if (CDiffLineMgr(&GetDocument()->m_cDocLineMgr).SearchDiffMark(ptXY.GetY2(), SEARCH_FORWARD, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptXY_Layout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptXY_Layout);
		if (selInfo.m_bSelectingLock) {
			if (!selInfo.IsTextSelected()) selInfo.BeginSelectArea();
		}else {
			if (selInfo.IsTextSelected()) selInfo.DisableSelectArea(true);
		}

		if (selInfo.m_bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().m_Common.m_sSearch.m_bSearchAll) {
		// ������Ȃ������B���A�ŏ��̌���
		if (!bFound	&& bRedo) {
			ptXY.y = 0 - 1;	// 1��O���w��
			bRedo = FALSE;
			goto re_do;		// �擪����Č���
		}
	}

	if (bFound) {
		if (nYOld_Logic >= ptXY.y) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_DIFF_NEXT_NOT_FOUND));
	}

	return;
}



// �O�̍�����T���C����������ړ�����
void CViewCommander::Command_Diff_Prev(void)
{
	BOOL bFound = FALSE;
	BOOL bRedo = TRUE;

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int			nYOld_Logic = ptXY.y;
	CLogicInt tmp_y;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();

re_do:;
	if (CDiffLineMgr(&GetDocument()->m_cDocLineMgr).SearchDiffMark(ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptXY_Layout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptXY_Layout);
		if (selInfo.m_bSelectingLock) {
			if (!selInfo.IsTextSelected()) selInfo.BeginSelectArea();
		}else {
			if (selInfo.IsTextSelected()) selInfo.DisableSelectArea(true);
		}

		if (selInfo.m_bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().m_Common.m_sSearch.m_bSearchAll) {
		// ������Ȃ������A���A�ŏ��̌���
		if (!bFound	&& bRedo) {
			// 2011.02.02 m_cLayoutMgr��m_cDocLineMgr
			ptXY.y = GetDocument()->m_cDocLineMgr.GetLineCount();	// 1��O���w��
			bRedo = FALSE;
			goto re_do;	// ��������Č���
		}
	}

	if (bFound) {
		if (nYOld_Logic <= ptXY.y) m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_DIFF_PREV_NOT_FOUND));
	}

	return;
}


/*!	�����\���̑S����
	@author	MIK
	@date	2002/05/26
*/
void CViewCommander::Command_Diff_Reset(void)
{
	CDiffLineMgr(&GetDocument()->m_cDocLineMgr).ResetAllDiffMark();

	// ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
	return;
}

