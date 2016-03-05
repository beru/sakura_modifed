/*
	Copyright (C) 2008, kobake

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
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/TagJumpManager.h"
#include "util/fileUtil.h"
#include "util/module.h"
#include "util/window.h"
#include "_main/ControlTray.h"
#include "charset/charcode.h"
#include "recent/Recent.h"

/*
	�w��t�@�C���̎w��ʒu�Ƀ^�O�W�����v����B

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.04.21 genta bClose�ǉ�
	@date	2004.05.29 Moca 0�ȉ����w�肳�ꂽ�Ƃ��́A�P������
	@date	2007.02.17 genta ���΃p�X�̊�f�B���N�g���w����ǉ�
*/
bool EditView::TagJumpSub(
	const TCHAR*	pszFileName,
	Point			ptJumpTo,		// �W�����v�ʒu(1�J�n)
	bool			bClose,			// [in] true: ���E�B���h�E����� / false: ���E�B���h�E����Ȃ�
	bool			bRelFromIni,
	bool*			pbJumpToSelf	// [out] �I�v�V����NULL�B�����ɃW�����v������
	)
{
	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	TagJump	tagJump;

	if (pbJumpToSelf) {
		*pbJumpToSelf = false;
	}

	// �Q�ƌ��E�B���h�E�ۑ�
	tagJump.hwndReferer = EditWnd::getInstance()->GetHwnd();

	//	Feb. 17, 2007 genta ���s�t�@�C������̑��Ύw��̏ꍇ��
	//	�\�ߐ�΃p�X�ɕϊ�����D(�L�[���[�h�w���v�W�����v�ŗp����)
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	TCHAR szJumpToFile[1024];
	if (bRelFromIni && _IS_REL_PATH(pszFileName)) {
		GetInidirOrExedir(szJumpToFile, pszFileName);
	}else {
		_tcscpy_s(szJumpToFile, pszFileName);
	}

	// �����O�t�@�C�������擾����
	TCHAR szWork[1024];
	if (::GetLongFileName(szJumpToFile, szWork)) {
		_tcscpy(szJumpToFile, szWork);
	}

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// 2004/07/05 �݂��΂�
// ����t�@�C������SendMesssage�� GetCaret().GetCaretLayoutPos().GetX2(),GetCaret().GetCaretLayoutPos().GetY2()���X�V����Ă��܂��A
// �W�����v��̏ꏊ���W�����v���Ƃ��ĕۑ�����Ă��܂��Ă���̂ŁA
// ���̑O�ŕۑ�����悤�ɕύX�B

	// �J�[�\���ʒu�ϊ�
	EditDoc* pDoc = GetDocument();
	Caret& caret = GetCaret();
	pDoc->m_layoutMgr.LayoutToLogic(
		caret.GetCaretLayoutPos(),
		&tagJump.point
	);

	// �^�O�W�����v���̕ۑ�
	TagJumpManager().PushTagJump(&tagJump);

	// �w��t�@�C�����J����Ă��邩���ׂ�
	// �J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h�����Ԃ�
	// �t�@�C�����J���Ă��邩
	HWND hwndOwner;
	if (ShareData::getInstance()->IsPathOpened(szJumpToFile, &hwndOwner)) {
		// 2004.05.13 Moca �}�C�i�X�l�͖���
		if (0 < ptJumpTo.y) {
			POINT poCaret;
			// �J�[�\�����ړ�������
			poCaret.y = ptJumpTo.y - 1;
			if (0 < ptJumpTo.x) {
				poCaret.x = ptJumpTo.x - 1;
			}else {
				poCaret.x = 0;
			}
			GetDllShareData().workBuffer.logicPoint.Set(LogicInt(poCaret.x), LogicInt(poCaret.y));
			::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
		}
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(hwndOwner);
		if (tagJump.hwndReferer == hwndOwner) {
			if (pbJumpToSelf) {
				*pbJumpToSelf = true;
			}
		}
	}else {
		// �V�����J��
		EditInfo inf;
		_tcscpy(inf.szPath, szJumpToFile);
		inf.ptCursor.Set(LogicInt(ptJumpTo.x - 1), LogicInt(ptJumpTo.y - 1));
		inf.nViewLeftCol = LayoutInt(-1);
		inf.nViewTopLine = LayoutInt(-1);
		inf.nCharCode    = CODE_AUTODETECT;

		bool bSuccess = ControlTray::OpenNewEditor2(
			G_AppInstance(),
			GetHwnd(),
			&inf,
			false,	// �r���[���[�h��
			true	// �������[�h�ŊJ��
		);

		if (!bSuccess)	// �t�@�C�����J���Ȃ�����
			return false;

		// Apr. 23, 2001 genta
		// hwndOwner�ɒl������Ȃ��Ȃ��Ă��܂������߂�
		// Tag Jump Back�����삵�Ȃ��Ȃ��Ă����̂��C��
		if (!ShareData::getInstance()->IsPathOpened(szJumpToFile, &hwndOwner))
			return false;
	}

	// 2006.12.30 ryoji ���鏈���͍Ō�Ɂi�����ʒu�ړ��j
	// Apr. 2003 genta ���邩�ǂ����͈����ɂ��
	// grep���ʂ���Enter�ŃW�����v����Ƃ����Ctrl����ړ�
	if (bClose) {
		ViewCommander& commander = GetCommander();
		commander.Command_WINCLOSE();	// ���킷�邾���B
	}

	return true;
}



/*! �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐�

	@date 2003.06.28 Moca �w�b�_�E�\�[�X�t�@�C���I�[�v���@�\�̃R�[�h�𓝍�
	@date 2008.04.09 ryoji �����Ώ�(file_ext)�ƊJ���Ώ�(open_ext)�̈������t�ɂȂ��Ă����̂��C��
*/
bool EditView::OPEN_ExtFromtoExt(
	bool			bCheckOnly,		// [in] true: �`�F�b�N�̂ݍs���ăt�@�C���͊J���Ȃ�
	bool			bBeepWhenMiss,	// [in] true: �t�@�C�����J���Ȃ������ꍇ�Ɍx�������o��
	const TCHAR*	file_ext[],		// [in] �����ΏۂƂ���g���q
	const TCHAR*	open_ext[],		// [in] �J���ΏۂƂ���g���q
	int				file_extno,		// [in] �����Ώۊg���q���X�g�̗v�f��
	int				open_extno,		// [in] �J���Ώۊg���q���X�g�̗v�f��
	const TCHAR*	errmes			// [in] �t�@�C�����J���Ȃ������ꍇ�ɕ\������G���[���b�Z�[�W
	)
{
	// �ҏW���t�@�C���̊g���q�𒲂ׂ�
	for (int i=0; i<file_extno; ++i) {
		if (CheckEXT(GetDocument()->m_docFile.GetFilePath(), file_ext[i])) {
			goto open_c;
		}
	}
	if (bBeepWhenMiss) {
		ErrorBeep();
	}
	return false;

open_c:;

	TCHAR	szPath[_MAX_PATH];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	HWND	hwndOwner;

	_tsplitpath(GetDocument()->m_docFile.GetFilePath(), szDrive, szDir, szFname, szExt);

	for (int i=0; i<open_extno; ++i) {
		_tmakepath(szPath, szDrive, szDir, szFname, open_ext[i]);
		if (!fexist(szPath)) {
			if (i < open_extno - 1)
				continue;
			if (bBeepWhenMiss) {
				ErrorBeep();
			}
			return false;
		}
		break;
	}
	if (bCheckOnly) {
		return true;
	}

	// �w��t�@�C�����J����Ă��邩���ׂ�
	// �J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h�����Ԃ�
	// �t�@�C�����J���Ă��邩
	if (ShareData::getInstance()->IsPathOpened(szPath, &hwndOwner)) {
	}else {
		// �����R�[�h�͂��̃t�@�C���ɍ��킹��
		LoadInfo loadInfo;
		loadInfo.filePath = szPath;
		loadInfo.eCharCode = GetDocument()->GetDocumentEncoding();
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			this->GetHwnd(),
			loadInfo,
			NULL,
			true
		);
		// �t�@�C�����J���Ă��邩
		if (ShareData::getInstance()->IsPathOpened(szPath, &hwndOwner)) {
		}else {
			// 2011.01.12 ryoji �G���[�͕\�����Ȃ��ł���
			// �t�@�C���T�C�Y���傫�����ēǂނ��ǂ����₢���킹�Ă���悤�ȏꍇ�ł��G���[�\���ɂȂ�͕̂�
			// OpenNewEditor()�܂��͋N�����ꂽ���̃��b�Z�[�W�\���ŏ\���Ǝv����

			//ErrorMessage(this->GetHwnd(), _T("%ts\n\n%ts\n\n"), errmes, szPath);
			return false;
		}
	}
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(hwndOwner);

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// 2004/07/09 genta/Moca �^�O�W�����v�o�b�N�̓o�^����菜����Ă������A
//            ������ł��]���ǂ���o�^����
	TagJump	tagJump;
	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	GetDocument()->m_layoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&tagJump.point
	);
	tagJump.hwndReferer = EditWnd::getInstance()->GetHwnd();
	// �^�O�W�����v���̕ۑ�
	TagJumpManager().PushTagJump(&tagJump);
	return true;
}


/*!	@brief �܂�Ԃ��̓��������

	�g�O���R�}���h�u���݂̃E�B���h�E���Ő܂�Ԃ��v���s�����ꍇ�̓�������肷��
	
	@retval TGWRAP_NONE No action
	@retval TGWRAP_FULL �ő�l
	@retval TGWRAP_WINDOW �E�B���h�E��
	@retval TGWRAP_PROP �ݒ�l

	@date 2006.01.08 genta ���j���[�\���œ���̔�����g�����߁CCommand_WRAPWINDOWWIDTH()��蕪���D
	@date 2006.01.08 genta ���������������
	@date 2008.06.08 ryoji �E�B���h�E���ݒ�ɂԂ牺���]����ǉ�
*/
EditView::TOGGLE_WRAP_ACTION EditView::GetWrapMode(LayoutInt* _newKetas)
{
	LayoutInt& newKetas = *_newKetas;
	//@@@ 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���R�}���h�B
	// 2002/04/08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	// 20051022 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���R�}���h
	// �E�B���h�E��==�����^�C�v||�ő�l==�����^�C�v �̏ꍇ�����邽�ߔ��菇���ɒ��ӂ���B
	/*	Jan.  8, 2006 genta
		���イ������̗v�]�ɂ�蔻����@���čl�D���݂̕��ɍ��킹��̂��ŗD��ɁD
	
		��{����F �ݒ�l���E�B���h�E��
			��(�E�B���h�E���ƍ����Ă��Ȃ����)���E�B���h�E������֖߂�
			��(�E�B���h�E���ƍ����Ă�����)���ő�l���ݒ�l
			�������C�ő�l==�ݒ�l�̏ꍇ�ɂ͍ő�l���ݒ�l�̑J�ڂ��ȗ�����ď�ɖ߂�
			
			�E�B���h�E�����ɒ[�ɋ����ꍇ�ɂ̓E�B���h�E���ɍ��킹�邱�Ƃ͏o���Ȃ����C
			�ݒ�l�ƍő�l�̃g�O���͉\�D

		0)���݂̃e�L�X�g�̐܂�Ԃ����@ != �w�茅�Ő܂�Ԃ��F�ύX�s�\
		1)���݂̐܂�Ԃ��� == �E�B���h�E�� : �ő�l
		2)���݂̐܂�Ԃ��� != �E�B���h�E��
		3)���E�B���h�E�����ɒ[�ɋ����ꍇ
		4)�@�����܂�Ԃ���!=�ő�l : �ő�l
		5)�@�����܂�Ԃ���==�ő�l
		6)�@�@�@�����ő�l==�ݒ�l : �ύX�s�\
		7)�@�@�@�����ő�l!=�ݒ�l : �ݒ�l
		8)���E�B���h�E�����\���ɂ���
		9)�@�����܂�Ԃ���==�ő�l
		a)�@�@�@�����ő�l!=�ݒ�l : �ݒ�l
	 	b)�@�@�@�����ő�l==�ݒ�l : �E�B���h�E��
		c)�@�����E�B���h�E��
	*/
	
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	if (layoutMgr.GetMaxLineKetas() == ViewColNumToWrapColNum(GetTextArea().m_nViewColNum)) {
		// a)
		newKetas = LayoutInt(MAXLINEKETAS);
		return TGWRAP_FULL;
	}else if (MINLINEKETAS > GetTextArea().m_nViewColNum - GetWrapOverhang()) { // 2)
		// 3)
		if (layoutMgr.GetMaxLineKetas() != MAXLINEKETAS) {
			// 4)
			newKetas = LayoutInt(MAXLINEKETAS);
			return TGWRAP_FULL;
		}else if (m_pTypeData->nMaxLineKetas == MAXLINEKETAS) { // 5)
			// 6)
			return TGWRAP_NONE;
		}else { // 7)
			newKetas = LayoutInt(m_pTypeData->nMaxLineKetas);
			return TGWRAP_PROP;
		}
	}else { // 8)
		if (1
			&& layoutMgr.GetMaxLineKetas() == MAXLINEKETAS // 9)
			&& m_pTypeData->nMaxLineKetas != MAXLINEKETAS
		) {
			// a)
			newKetas = LayoutInt(m_pTypeData->nMaxLineKetas);
			return TGWRAP_PROP;
			
		}else {	// b) c)
			//	���݂̃E�B���h�E��
			newKetas = ViewColNumToWrapColNum(GetTextArea().m_nViewColNum);
			return TGWRAP_WINDOW;
		}
	}
}


void EditView::AddToCmdArr(const TCHAR* szCmd)
{
	RecentCmd recentCmd;
	recentCmd.AppendItem(szCmd);
	recentCmd.Terminate();
}

/*! ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�)
	@date 2002.01.16 hor ���ʃ��W�b�N���֐��ɂ��������E�E�E
	@date 2011.12.18 Moca �V�[�P���X�����Bview�̌��������񒷂̓P�p�B���̃r���[�̌��������������p���t���O��ǉ�
*/
BOOL EditView::ChangeCurRegexp(bool bRedrawIfChanged)
{
	bool bChangeState = false;

	if (GetDllShareData().common.search.bInheritKeyOtherView
			&& m_nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
		|| m_strCurSearchKey.size() == 0
	) {
		// �����̌����L�[�ɍX�V
		m_strCurSearchKey = GetDllShareData().searchKeywords.searchKeys[0];		// ����������
		m_curSearchOption = GetDllShareData().common.search.searchOption;// �����^�u��  �I�v�V����
		m_nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
		bChangeState = true;
	}else if (m_bCurSearchUpdate) {
		bChangeState = true;
	}
	m_bCurSearchUpdate = false;
	if (bChangeState) {
		if (!m_searchPattern.SetPattern(this->GetHwnd(), m_strCurSearchKey.c_str(), m_strCurSearchKey.size(),
			m_curSearchOption, &m_curRegexp)
		) {
				m_bCurSrchKeyMark = false;
				return FALSE;
		}
		m_bCurSrchKeyMark = true;
		if (bRedrawIfChanged) {
			Redraw();
		}
		m_pEditWnd->m_toolbar.AcceptSharedSearchKey();
		return TRUE;
	}
	if (!m_bCurSrchKeyMark) {
		m_bCurSrchKeyMark = true;
		// ����������̃}�[�N�����ݒ�
		if (bRedrawIfChanged) {
			Redraw(); // ��View�ĕ`��
		}
	}

	return TRUE;
}


/*!
	�J�[�\���s���N���b�v�{�[�h�ɃR�s�[����

	@date 2007.10.08 ryoji �V�K�iCommand_COPY()���珈�������o���j
*/
void EditView::CopyCurLine(
	bool	bAddCRLFWhenCopy,		// [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
	EolType	neweol,					// [in] �R�s�[����Ƃ���EOL�B
	bool	bEnableLineModePaste	// [in] ���C�����[�h�\��t�����\�ɂ���
	)
{
	if (GetSelectionInfo().IsTextSelected()) {
		return;
	}

	const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		return;
	}

	// �N���b�v�{�[�h�ɓ����ׂ��e�L�X�g�f�[�^���AmemBuf�Ɋi�[����
	NativeW memBuf;
	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL());
	if (pLayout->GetLayoutEol().GetLen() != 0) {
		memBuf.AppendString(
			(neweol == EolType::Unknown) ?
				pLayout->GetLayoutEol().GetValue2() : Eol(neweol).GetValue2()
		);
	}else if (bAddCRLFWhenCopy) {	// 2007.10.08 ryoji bAddCRLFWhenCopy�Ή������ǉ�
		memBuf.AppendString(
			(neweol == EolType::Unknown) ?
				WCODE::CRLF : Eol(neweol).GetValue2()
		);
	}

	// �N���b�v�{�[�h�Ƀf�[�^memBuf�̓��e��ݒ�
	BOOL bSetResult = MySetClipboardData(
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		false,
		bEnableLineModePaste
	);
	if (!bSetResult) {
		ErrorBeep();
	}
}

void EditView::DrawBracketCursorLine(bool bDraw)
{
	if (bDraw) {
		GetCaret().m_underLine.CaretUnderLineON(true, true);
		DrawBracketPair(false);
		SetBracketPairPos(true);
		DrawBracketPair(true);
	}
}

HWND EditView::StartProgress()
{
	HWND hwndProgress = m_pEditWnd->m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
		Progress_SetRange(hwndProgress, 0, 101);
		Progress_SetPos(hwndProgress, 0);
	}
	return hwndProgress;
}

