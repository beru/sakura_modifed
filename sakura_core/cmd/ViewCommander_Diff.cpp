/*!	@file
@brief ViewCommander�N���X�̃R�}���h(Diff)�֐��Q

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
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCompare.h"
#include "dlg/DlgDiff.h"
#include "charset/CodeMediator.h"
#include "charset/CodePage.h"
#include "env/ShareData.h"
#include "util/window.h"
#include "util/os.h"
#include "_main/Mutex.h"


/*!
	@return true:����I�� / false:�G���[�I��
*/
static bool Commander_COMPARE_core(
	ViewCommander& commander,
	bool& bDifferent,
	HWND hwnd,
	LogicPoint& poSrc,
	LogicPoint& poDes
	)
{
	const wchar_t*	pLineSrc;
	size_t			nLineLenSrc;
	const wchar_t*	pLineDes;
	int nLineLenDes;
	int max_size = (int)GetDllShareData().workBuffer.GetWorkBufferCount<EDIT_CHAR>();
	const DocLineMgr& docMgr = commander.GetDocument().docLineMgr;

	bDifferent = true;
	{
		pLineDes = GetDllShareData().workBuffer.GetWorkBuffer<const EDIT_CHAR>();
		int nLineOffset = 0;
		for (;;) {
			pLineSrc = docMgr.GetLine(poSrc.y)->GetDocLineStrWithEOL(&nLineLenSrc);
			do {
				// workBuffer#Work�̔r������B�O���R�}���h�o��/TraceOut/Diff���Ώ�
				LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
				// �s(���s�P��)�f�[�^�̗v��
				nLineLenDes = ::SendMessage( hwnd, MYWM_GETLINEDATA, poDes.y, nLineOffset );
				if (nLineLenDes < 0) {
					return false;
				}
				// �ǂ������ŏI�s(EOF)�ɓ��B�B����Ɣ���
				if (!pLineSrc && nLineLenDes == 0) {
					bDifferent = false;
					return true;
				}
				// �ǂ��炩�������A�ŏI�s�ɓ��B
				if (!pLineSrc || nLineLenDes == 0) {
					return true;
				}
				int nDstEndPos = std::min( nLineLenDes, max_size ) + nLineOffset;
				if (poDes.x < nLineOffset) {
					// 1�s�ڍs���f�[�^�ǂݔ�΂�
					if (nLineLenDes < poDes.x) {
						poDes.x = nLineLenDes - 1;
						return true;
					}
					nLineOffset = poDes.x;
				}else {
					// Note: �T���Q�[�g/���s�̓r���ɃJ�[�\�������邱�Ƃ�����
					while (poDes.x < nDstEndPos) {
						if (nLineLenSrc <= poSrc.x) {
							return true;
						}
						if (pLineSrc[poSrc.x] != pLineDes[poDes.x - nLineOffset]) {
							return true;
						}
						poSrc.x++;
						poDes.x++;
					}
				}
				nLineOffset += max_size;
			}while (max_size < nLineLenDes);

			if (poSrc.x < nLineLenSrc) {
				return true;
			}
			poSrc.x = 0;
			poSrc.y++;
			poDes.x = 0;
			poDes.y++;
			nLineOffset = 0;
		}
	}
	assert_warning(0);
	return false;
}

// �t�@�C�����e��r
void ViewCommander::Command_Compare(void)
{
	HWND		hwndCompareWnd = NULL;
	TCHAR		szPath[_MAX_PATH + 1];
	DlgCompare	dlgCompare;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK
	auto& commonSetting = GetDllShareData().common;
	auto& csCompare = commonSetting.compare;
	// ��r��A���E�ɕ��ׂĕ\��
	dlgCompare.bCompareAndTileHorz = csCompare.bCompareAndTileHorz;
	BOOL bDlgCompareResult = dlgCompare.DoModal(
		G_AppInstance(),
		view.GetHwnd(),
		(LPARAM)&GetDocument(),
		GetDocument().docFile.GetFilePath(),
		szPath,
		&hwndCompareWnd
	);
	if (!bDlgCompareResult) {
		return;
	}
	// ��r��A���E�ɕ��ׂĕ\��
	csCompare.bCompareAndTileHorz = dlgCompare.bCompareAndTileHorz;

	// �^�u�E�B���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if (commonSetting.tabBar.bDispTabWnd
		&& !commonSetting.tabBar.bDispTabWndMultiWin
	) {
		hwndMsgBox = view.GetHwnd();
		csCompare.bCompareAndTileHorz = false;
	}else {
		hwndMsgBox = hwndCompareWnd;
	}

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	LogicPoint	poSrc;
	GetDocument().layoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&poSrc
	);

	// �J�[�\���ʒu�擾 -> poDes
	LogicPoint	poDes;
	{
		::SendMessage(hwndCompareWnd, MYWM_GETCARETPOS, 0, 0);
		LogicPoint* ppoCaretDes = &(GetDllShareData().workBuffer.logicPoint);
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bool bDifferent = false;
	// �{����
	Commander_COMPARE_core(*this, bDifferent, hwndCompareWnd, poSrc, poDes);

	// ��r��A���E�ɕ��ׂĕ\��
// From Here Oct. 10, 2000 JEPRO	�`�F�b�N�{�b�N�X���{�^��������Έȉ��̍s(To Here �܂�)�͕s�v�̂͂�����
// ���܂������Ȃ������̂Ō��ɖ߂��Ă���c
	if (GetDllShareData().common.compare.bCompareAndTileHorz) {
		HWND hWndArr[2];
		hWndArr[0] = GetMainWindow();
		hWndArr[1] = hwndCompareWnd;
		for (int i=0; i<2; ++i) {
			if (::IsZoomed( hWndArr[i] )) {
				::ShowWindow( hWndArr[i], SW_RESTORE );
			}
		}
		// �f�X�N�g�b�v�T�C�Y�𓾂� 2002.1.24 YAZAKI
		RECT rcDesktop;
		// May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( hWndArr[0], &rcDesktop );
		int width = (rcDesktop.right - rcDesktop.left) / 2;
		for (int i=1; i>=0; --i) {
			::SetWindowPos(
				hWndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
	}
// To Here Oct. 10, 2000

	// 2002/05/11 YAZAKI �e�E�B���h�E�����܂��ݒ肵�Ă݂�B
	if (!bDifferent) {
		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD22));
	}else {
//		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		GetDllShareData().workBuffer.logicPoint = poDes;
		::SendMessage(hwndCompareWnd, MYWM_SETCARETPOS, 0, 0);

		// �J�[�\�����ړ�������
		GetDllShareData().workBuffer.logicPoint = poSrc;
		::PostMessage(GetMainWindow(), MYWM_SETCARETPOS, 0, 0);
		TopWarningMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));	// �ʒu��ύX���Ă��烁�b�Z�[�W	2008/4/27 Uchi
	}

	// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(GetMainWindow());
	return;
}


static
EncodingType GetFileCharCode( LPCTSTR pszFile )
{
	const TypeConfigMini* typeMini;
	DocTypeManager().GetTypeConfigMini( DocTypeManager().GetDocumentTypeOfPath( pszFile ), &typeMini );
	return CodeMediator(typeMini->encoding).CheckKanjiCodeOfFile( pszFile );
}


static
EncodingType GetDiffCreateTempFileCode(EncodingType code)
{
	EEncodingTrait e = CodePage::GetEncodingTrait(code);
	if (e != ENCODING_TRAIT_ASCII) {
		return CODE_UTF8;
	}
	return code;
}


/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�Ȃ���)
	@author	maru
	@date	2005/10/28 ����܂ł�Command_Diff��view.ViewDiffInfo�ɖ��̕ύX
*/
void ViewCommander::Command_Diff(const WCHAR* _szDiffFile2, int nFlgOpt)
{
	const std::tstring strDiffFile2 = to_tchar(_szDiffFile2);
	const TCHAR* szDiffFile2 = strDiffFile2.c_str();

	bool bTmpFile1 = false;
	TCHAR szTmpFile1[_MAX_PATH * 2];

	if (!IsFileExists( szDiffFile2, true )) {
		WarningMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF1));
		return;
	}

	// ���t�@�C��
	// 2013.06.21 Unicode�̂Ƃ��́A�����t�@�C���o��
	EncodingType code = GetDocument().GetDocumentEncoding();
	EncodingType saveCode = GetDiffCreateTempFileCode(code);
	EncodingType code2 = GetFileCharCode(szDiffFile2);
	EncodingType saveCode2 = GetDiffCreateTempFileCode(code2);
	// 2014.10.24 �R�[�h���Ⴄ�Ƃ��͕K��UTF-8�t�@�C���o��
	if (saveCode != saveCode2) {
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}

	if (GetDocument().docEditor.IsModified()
		|| saveCode != code
		|| !GetDocument().docFile.GetFilePathClass().IsValidPath() // 2014.06.25 Grep/�A�E�g�v�b�g���Ώۂɂ���
	) {
		if (!view.MakeDiffTmpFile(szTmpFile1, NULL, saveCode, GetDocument().GetDocumentBomExist())) {
			return;
		}
		bTmpFile1 = true;
	}else {
		_tcscpy( szTmpFile1, GetDocument().docFile.GetFilePath() );
	}

	bool bTmpFile2 = false;
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	bool bTmpFileMode = code2 != saveCode2;
	if (!bTmpFileMode) {
		_tcscpy(szTmpFile2, szDiffFile2);
	}else if (view.MakeDiffTmpFile2( szTmpFile2, szDiffFile2, code2, saveCode2 )) {
		bTmpFile2 = true;
	}else {
		if (bTmpFile1) _tunlink( szTmpFile1 );
		return;
	}

	bool bUTF8io = true;
	if (saveCode == CODE_SJIS) {
		bUTF8io = false;
	}

	// �����\��
	view.ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt, bUTF8io);

	// �ꎞ�t�@�C�����폜����
	if (bTmpFile1) _tunlink( szTmpFile1 );
	if (bTmpFile2) _tunlink( szTmpFile2 );

	return;
}


/*!	�����\��
	@note	HandleCommand����̌Ăяo���Ή�(�_�C�A���O�����)
	@author	MIK
	@date	2002/05/25
	@date	2002/11/09 �ҏW���t�@�C��������
	@date	2005/10/29 maru �ꎞ�t�@�C���쐬������view.MakeDiffTmpFile�ֈړ�
*/
void ViewCommander::Command_Diff_Dialog(void)
{
	DlgDiff dlgDiff;
	bool bTmpFile1 = false, bTmpFile2 = false;

	auto& docFile = GetDocument().docFile;
	auto& docEditor = GetDocument().docEditor;
	// DIFF�����\���_�C�A���O��\������
	int nDiffDlgResult = dlgDiff.DoModal(
		G_AppInstance(),
		view.GetHwnd(),
		(LPARAM)&GetDocument(),
		docFile.GetFilePath()
	);
	if (!nDiffDlgResult) {
		return;
	}
	
	// ���t�@�C��
	TCHAR szTmpFile1[_MAX_PATH * 2];
	EncodingType code = GetDocument().GetDocumentEncoding();
	EncodingType saveCode = GetDiffCreateTempFileCode(code);
	EncodingType code2 = dlgDiff.nCodeTypeDst;
	if (code2 == CODE_ERROR) {
		if (dlgDiff.szFile2[0] != _T('\0')) {
			// �t�@�C�����w��
			code2 = GetFileCharCode(dlgDiff.szFile2);
		}
	}
	EncodingType saveCode2 = GetDiffCreateTempFileCode(code2);
	// 2014.10.24 �R�[�h���Ⴄ�Ƃ��͕K��UTF-8�t�@�C���o��
	if (saveCode != saveCode2) {
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}
	if (GetDocument().docEditor.IsModified()
		|| code != saveCode
		|| !GetDocument().docFile.GetFilePathClass().IsValidPath() // 2014.06.25 Grep/�A�E�g�v�b�g���Ώۂɂ���
	) {
		if (!view.MakeDiffTmpFile( szTmpFile1, NULL, saveCode, GetDocument().GetDocumentBomExist() )) { return; }
		bTmpFile1 = true;
	}else {
		_tcscpy( szTmpFile1, GetDocument().docFile.GetFilePath() );
	}
		
	// ����t�@�C��
	// UNICODE,UNICODEBE�̏ꍇ�͏�Ɉꎞ�t�@�C����UTF-8�ɂ���
	TCHAR szTmpFile2[_MAX_PATH * 2];
	// 2014.06.25 �t�@�C�������Ȃ�(=����,Grep,�A�E�g�v�b�g)��TmpFileMode�ɂ���
	bool bTmpFileMode = dlgDiff.bIsModifiedDst || code2 != saveCode2 || dlgDiff.szFile2[0] == _T('\0');
	if (!bTmpFileMode) {
		// ���ύX�Ńt�@�C�������ASCII�n�R�[�h�̏ꍇ�̂�,���̂܂܃t�@�C���𗘗p����
		_tcscpy( szTmpFile2, dlgDiff.szFile2 );
	}else if (dlgDiff.hWnd_Dst) {
		// �t�@�C���ꗗ����I��
		if (view.MakeDiffTmpFile( szTmpFile2, dlgDiff.hWnd_Dst, saveCode2, dlgDiff.bBomDst )) {
			bTmpFile2 = true;
		}else {
			if (bTmpFile1) _tunlink( szTmpFile1 );
			return;
		}
	}else {
		// �t�@�C�����w��Ŕ�ASCII�n�������ꍇ
		if (view.MakeDiffTmpFile2( szTmpFile2, dlgDiff.szFile2, code2, saveCode2 )) {
			bTmpFile2 = true;
		}else {
			// Error
			if (bTmpFile1) _tunlink( szTmpFile1 );
			return;
		}
	}
	
	bool bUTF8io = true;
	if (saveCode == CODE_SJIS) {
		bUTF8io = false;
	}

	// �����\��
	view.ViewDiffInfo(szTmpFile1, szTmpFile2, dlgDiff.nDiffFlgOpt, bUTF8io);
	
	// �ꎞ�t�@�C�����폜����
	if (bTmpFile1) {
		_tunlink(szTmpFile1);
	}
	if (bTmpFile2) {
		_tunlink(szTmpFile2);
	}

	return;
}


//	���̍�����T���C����������ړ�����
void ViewCommander::Command_Diff_Next(void)
{
	bool bFound = false;
	bool bRedo = true;

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld_Logic = ptXY.y;
	size_t tmp_y;
	auto& selInfo = view.GetSelectionInfo();

re_do:;	
	if (DiffLineMgr(GetDocument().docLineMgr).SearchDiffMark(ptXY.GetY2(), SearchDirection::Forward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;
		LayoutPoint ptXY_Layout;
		GetDocument().layoutMgr.LogicToLayout(ptXY, &ptXY_Layout);
		if (selInfo.bSelectingLock) {
			if (!selInfo.IsTextSelected()) {
				selInfo.BeginSelectArea();
			}
		}else {
			if (selInfo.IsTextSelected()) {
				selInfo.DisableSelectArea(true);
			}
		}

		if (selInfo.bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().common.search.bSearchAll) {
		// ������Ȃ������B���A�ŏ��̌���
		if (!bFound	&& bRedo) {
			ptXY.y = 0 - 1;	// 1��O���w��
			bRedo = false;
			goto re_do;		// �擪����Č���
		}
	}

	if (bFound) {
		if (nYOld_Logic >= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_DIFF_NEXT_NOT_FOUND));
	}

	return;
}


// �O�̍�����T���C����������ړ�����
void ViewCommander::Command_Diff_Prev(void)
{
	bool bFound = false;
	bool bRedo = true;

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int			nYOld_Logic = ptXY.y;
	size_t		tmp_y;
	auto& selInfo = view.GetSelectionInfo();

re_do:;
	if (DiffLineMgr(GetDocument().docLineMgr).SearchDiffMark(ptXY.GetY2(), SearchDirection::Backward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;
		LayoutPoint ptXY_Layout;
		GetDocument().layoutMgr.LogicToLayout(ptXY, &ptXY_Layout);
		if (selInfo.bSelectingLock) {
			if (!selInfo.IsTextSelected()) {
				selInfo.BeginSelectArea();
			}
		}else {
			if (selInfo.IsTextSelected()) {
				selInfo.DisableSelectArea(true);
			}
		}

		if (selInfo.bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().common.search.bSearchAll) {
		// ������Ȃ������A���A�ŏ��̌���
		if (!bFound	&& bRedo) {
			// 2011.02.02 layoutMgr��docLineMgr
			ptXY.y = GetDocument().docLineMgr.GetLineCount();	// 1��O���w��
			bRedo = false;
			goto re_do;	// ��������Č���
		}
	}

	if (bFound) {
		if (nYOld_Logic <= ptXY.y) view.SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_DIFF_PREV_NOT_FOUND));
	}

	return;
}


/*!	�����\���̑S����
	@author	MIK
	@date	2002/05/26
*/
void ViewCommander::Command_Diff_Reset(void)
{
	DiffLineMgr(GetDocument().docLineMgr).ResetAllDiffMark();

	// ���������r���[���X�V
	GetEditWindow().Views_Redraw();
	return;
}

