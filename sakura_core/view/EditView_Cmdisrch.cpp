/*!	@file
	@brief EditView�N���X�̃C���N�������^���T�[�`�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "extmodule/Migemo.h"
#include "sakura_rc.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(ISearch��)�y��
	�C���N�������^���T�[�`���[�h�𔲂��锻��

	@return true: �R�}���h�����ς� / false: �R�}���h�����p��

	@date 2004.09.14 isearch �V�K�쐬
	@date 2005.01.10 genta �֐���, UNINDENT�ǉ�

	@note UNINDENT��ʏ핶���Ƃ��Ĉ����̂́C
		SHIFT+�����̌��SPACE����͂���悤�ȃP�[�X��
		SHIFT�̉�����x��Ă�����������Ȃ��Ȃ邱�Ƃ�h�����߁D
*/
void EditView::TranslateCommand_isearch(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
)
{
	if (m_nISearchMode <= 0)
		return;

	switch (nCommand) {
	// �����̋@�\�̂Ƃ��A�C���N�������^���T�[�`�ɓ���
	case F_ISEARCH_NEXT:
	case F_ISEARCH_PREV:
	case F_ISEARCH_REGEXP_NEXT:
	case F_ISEARCH_REGEXP_PREV:
	case F_ISEARCH_MIGEMO_NEXT:
	case F_ISEARCH_MIGEMO_PREV:
		break;

	// �ȉ��̋@�\�̂Ƃ��A�C���N�������^���T�[�`���͌����������͂Ƃ��ď���
	case F_WCHAR:
	case F_IME_CHAR:
		nCommand = F_ISEARCH_ADD_CHAR;
		break;
	case F_INSTEXT_W:
		nCommand = F_ISEARCH_ADD_STR;
		break;

	case F_INDENT_TAB:	// TAB�̓C���f���g�ł͂Ȃ��P�Ȃ�TAB�����ƌ��Ȃ�
	case F_UNINDENT_TAB:	// genta�ǉ�
		nCommand = F_ISEARCH_ADD_CHAR;
		lparam1 = '\t';
		break;
	case F_INDENT_SPACE:	// �X�y�[�X�̓C���f���g�ł͂Ȃ��P�Ȃ�TAB�����ƌ��Ȃ�
	case F_UNINDENT_SPACE:	// genta�ǉ�
		nCommand = F_ISEARCH_ADD_CHAR;
		lparam1 = ' ';
		break;
	case F_DELETE_BACK:
		nCommand = F_ISEARCH_DEL_BACK;
		break;

	default:
		// ��L�ȊO�̃R�}���h�̏ꍇ�̓C���N�������^���T�[�`�𔲂���
		ISearchExit();
	}
}

/*!
	ISearch �R�}���h����

	@date 2005.01.10 genta �e�R�}���h�ɓ����Ă���������1�J���Ɉړ�
*/
bool EditView::ProcessCommand_isearch(
	int	nCommand,
	bool	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	switch (nCommand) {
	// ����������̕ύX����
	case F_ISEARCH_ADD_CHAR:
		ISearchExec((DWORD)lparam1);
		return true;
	
	case F_ISEARCH_DEL_BACK:
		ISearchBack();
		return true;

	case F_ISEARCH_ADD_STR:
		ISearchExec((LPCWSTR)lparam1);
		return true;

	// �������[�h�ւ̈ڍs
	case F_ISEARCH_NEXT:
		ISearchEnter(1, SearchDirection::Forward);		// �O���C���N�������^���T�[�` // 2004.10.13 isearch
		return true;
	case F_ISEARCH_PREV:
		ISearchEnter(1, SearchDirection::Backward);	// ����C���N�������^���T�[�` // 2004.10.13 isearch
		return true;
	case F_ISEARCH_REGEXP_NEXT:
		ISearchEnter(2, SearchDirection::Forward);		// �O�����K�\���C���N�������^���T�[�`  // 2004.10.13 isearch
		return true;
	case F_ISEARCH_REGEXP_PREV:
		ISearchEnter(2, SearchDirection::Backward);	// ������K�\���C���N�������^���T�[�`  // 2004.10.13 isearch
		return true;
	case F_ISEARCH_MIGEMO_NEXT:
		ISearchEnter(3, SearchDirection::Forward);		// �O��MIGEMO�C���N�������^���T�[�`    // 2004.10.13 isearch
		return true;
	case F_ISEARCH_MIGEMO_PREV:
		ISearchEnter(3, SearchDirection::Backward);	// ���MIGEMO�C���N�������^���T�[�`    // 2004.10.13 isearch
		return true;
	}
	return false;
}

/*!
	�C���N�������^���T�[�`���[�h�ɓ���

	@param mode [in] �������@ 1:�ʏ�, 2:���K�\��, 3:MIGEMO
	@param direction [in] �������� 0:���(���), 1:�O��(����)

	@author isearch
	@date 2011.12.15 Moca m_sCurSearchOption/m_sSearchOption�Ɠ������Ƃ�
	@date 2012.10.11 novice m_sCurSearchOption/m_sSearchOption�̓�����switch�̑O�ɕύX
	@date 2012.10.11 novice MIGEMO�̏�����case���Ɉړ�
*/
void EditView::ISearchEnter(int mode, SearchDirection direction)
{

	if (m_nISearchMode == mode) {
		// �Ď��s
		m_nISearchDirection =  direction;
		
		if (m_bISearchFirst) {
			m_bISearchFirst = false;
		}
		// ������ƏC��
		ISearchExec(true);

	}else {
		auto& selInfo = GetSelectionInfo();
		// �C���N�������^���T�[�`���[�h�ɓ��邾��.		
		// �I��͈͂̉���
		if (selInfo.IsTextSelected())	
			selInfo.DisableSelectArea(true);

		m_curSearchOption = GetDllShareData().m_common.m_search.m_searchOption;
		switch (mode) {
		case 1: // �ʏ�C���N�������^���T�[�`
			m_curSearchOption.bRegularExp = false;
			m_curSearchOption.bLoHiCase = false;
			m_curSearchOption.bWordOnly = false;
			//SendStatusMessage(_T("I-Search: "));
			break;
		case 2: // ���K�\���C���N�������^���T�[�`
			if (!m_curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			m_curSearchOption.bRegularExp = true;
			m_curSearchOption.bLoHiCase = false;
			//SendStatusMessage(_T("[RegExp] I-Search: "));
			break;
		case 3: // MIGEMO�C���N�������^���T�[�`
			if (!m_curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			if (!m_pMigemo) {
				m_pMigemo = Migemo::getInstance();
				m_pMigemo->InitDll();
			}
			// migemo dll �`�F�b�N
			//	Jan. 10, 2005 genta �ݒ�ύX�Ŏg����悤�ɂȂ��Ă���
			//	�\��������̂ŁC�g�p�\�łȂ���Έꉞ�����������݂�
			if (!m_pMigemo->IsAvailable() && DLL_SUCCESS != m_pMigemo->InitDll()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO1));
				return;
			}
			m_pMigemo->migemo_load_all();
			if (m_pMigemo->migemo_is_enable()) {
				m_curSearchOption.bRegularExp = true;
				m_curSearchOption.bLoHiCase = false;
				//SendStatusMessage(_T("[MIGEMO] I-Search: "));
			}else {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO2));
				return;
			}
			break;
		}
		
		//	Feb. 04, 2005 genta	�����J�n�ʒu���L�^
		//	�C���N�������^���T�[�`�ԂŃ��[�h��؂�ւ���ꍇ�ɂ͊J�n�ƌ��Ȃ��Ȃ�
		if (m_nISearchMode == 0) {
			m_ptSrchStartPos_PHY = GetCaret().GetCaretLogicPos();
		}
		
		m_bCurSrchKeyMark = false;
		m_nISearchDirection = direction;
		m_nISearchMode = mode;
		
		m_nISearchHistoryCount = 0;
		m_searchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());

		Redraw();
		
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		
		m_bISearchWrap = false;
		m_bISearchFirst = true;
	}

	// �}�E�X�J�[�\���ύX
	if (direction == SearchDirection::Forward) {
		::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else {
		::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

// �C���N�������^���T�[�`���[�h���甲����
void EditView::ISearchExit()
{
	// �V�[�P���X���㏑�����Č��݂̌����L�[���ێ�����
	if (m_strCurSearchKey.size() < _MAX_PATH) {
		SearchKeywordManager().AddToSearchKeyArr(m_strCurSearchKey.c_str());
	}
	m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;
	GetDllShareData().m_common.m_search.m_searchOption = m_curSearchOption;
	m_pEditWnd->m_toolbar.AcceptSharedSearchKey();
	m_nISearchDirection = SearchDirection::Backward;
	m_nISearchMode = 0;
	
	if (m_nISearchHistoryCount == 0) {
		m_strCurSearchKey.clear();
	}

	// �}�E�X�J�[�\�������ɖ߂�
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0, point1.x, point1.y);

	// �X�e�[�^�X�\���G���A���N���A
	SendStatusMessage(_T(""));

}

/*!
	@brief �C���N�������^���T�[�`�̎��s(1�����ǉ�)
	
	@param wChar [in] �ǉ����镶�� (1byte or 2byte)
*/
void EditView::ISearchExec(DWORD wChar)
{
	// ���ꕶ���͏������Ȃ�
	switch (wChar) {
	case L'\r':
	case L'\n':
		ISearchExit();
		return;
	//case '\t':
	//	break;
	}
	
	if (m_bISearchFirst) {
		m_bISearchFirst = false;
		m_strCurSearchKey.clear();
	}

	if (wChar <= 0xffff) {
		m_strCurSearchKey.append(1, (WCHAR)wChar);
	}else {
		m_strCurSearchKey.append(1, (WCHAR)(wChar>>16));
		m_strCurSearchKey.append(1, (WCHAR)wChar);
	}
	
	ISearchExec(false);
	return ;
}

/*!
	@brief �C���N�������^���T�[�`�̎��s(������ǉ�)
	
	@param pszText [in] �ǉ����镶����
*/
void EditView::ISearchExec(LPCWSTR pszText)
{
	// �ꕶ�����������Ď��s

	const WCHAR* p;
	DWORD c;
	p = pszText;
	
	while (*p != L'\0') {
		if (IsUtf16SurrogHi(*p) && IsUtf16SurrogLow(*(p + 1))) {
			c = (((WORD)*p) << 16) | ((WORD)*(p + 1));
			++p;
		}else {
			c = *p;
		}
		ISearchExec(c);
		++p;
	}
	return ;
}

/*!
	@brief �C���N�������^���T�[�`�̎��s

	@param bNext [in] true:���̌�������, false:���݂̃J�[�\���ʒu�̂܂܌���
*/
void EditView::ISearchExec(bool bNext) 
{
	// ���������s����.

	if ((m_strCurSearchKey.size() == 0) || (m_nISearchMode == 0)) {
		// �X�e�[�^�X�̕\��
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		return ;
	}
	
	ISearchWordMake();
	
	LayoutInt nLine(0);
	LayoutInt nIdx1(0);
	
	if (bNext && m_bISearchWrap) {
		switch (m_nISearchDirection) {
		case 1:
			nLine = LayoutInt(0);
			nIdx1 = LayoutInt(0);
			break;
		case 0:
			// �Ōォ�猟��
			LogicInt nLineP;
			int nIdxP;
			auto& docLineMgr = m_pEditDoc->m_docLineMgr;
			nLineP =  docLineMgr.GetLineCount() - LogicInt(1);
			DocLine* pDocLine = docLineMgr.GetLine(nLineP);
			nIdxP = pDocLine->GetLengthWithEOL() -1;
			LayoutPoint ptTmp;
			m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(nIdxP, nLineP), &ptTmp);
			nIdx1 = ptTmp.GetX2();
			nLine = ptTmp.GetY2();
		}
	}else if (GetSelectionInfo().IsTextSelected()) {
		auto& select = GetSelectionInfo().m_select;
		switch ((int)m_nISearchDirection * 2 + (bNext ? 1: 0)) {
		case 2 : // �O�������Ō��݈ʒu���猟���̂Ƃ�
		case 1 : // ��������Ŏ��������̂Ƃ�
			// �I��͈͂̐擪�������J�n�ʒu��
			nLine = select.GetFrom().GetY2();
			nIdx1 = select.GetFrom().GetX2();
			break;
		case 0 : // �O�������Ŏ�������
		case 3 : // ��������Ō��݈ʒu���猟��
			// �I��͈͂̌�납��
			nLine = select.GetTo().GetY2();
			nIdx1 = select.GetTo().GetX2();
			break;
		}
	}else {
		auto& pos = GetCaret().GetCaretLayoutPos();
		nLine = pos.GetY2();
		nIdx1  = pos.GetX2();
	}

	// ���ʒu����index�ɕϊ�
	Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(nLine);
	LogicInt nIdx = LineColumnToIndex(pLayout, nIdx1);

	m_nISearchHistoryCount ++ ;

	NativeT msg;
	ISearchSetStatusMsg(&msg);

	if (m_nISearchHistoryCount >= 256) {
		m_nISearchHistoryCount = 156;
		for (int i=100; i<=255; ++i) {
			m_bISearchFlagHistory[i-100] = m_bISearchFlagHistory[i];
			m_searchHistory[i-100] = m_searchHistory[i];
		}
	}
	m_bISearchFlagHistory[m_nISearchHistoryCount] = bNext;

	LayoutRange matchRange;

	int nSearchResult = m_pEditDoc->m_layoutMgr.SearchWord(
		nLine,						// �����J�n���C�A�E�g�s
		nIdx,						// �����J�n�f�[�^�ʒu
		m_nISearchDirection,		// 0==�O������ 1==�������
		&matchRange,				// �}�b�`���C�A�E�g�͈�
		m_searchPattern
	);
	if (nSearchResult == 0) {
		// �������ʂ��Ȃ�
		msg.AppendString(LS(STR_EDITVWISRCH_NOMATCH));
		SendStatusMessage(msg.GetStringPtr());
		
		if (bNext) 	m_bISearchWrap = true;
		if (GetSelectionInfo().IsTextSelected()) {
			m_searchHistory[m_nISearchHistoryCount] = GetSelectionInfo().m_select;
		}else {
			m_searchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());
		}
	}else {
		// �������ʂ���
		// �L�����b�g�ړ�
		GetCaret().MoveCursor(matchRange.GetFrom(), true, _CARETMARGINRATE / 3);
		
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(matchRange);

		m_bISearchWrap = false;
		m_searchHistory[m_nISearchHistoryCount] = matchRange;
	}

	m_bCurSrchKeyMark = true;

	Redraw();	
	SendStatusMessage(msg.GetStringPtr());
	return ;
}

// �o�b�N�X�y�[�X�������ꂽ�Ƃ��̏���
void EditView::ISearchBack(void) {
	if (m_nISearchHistoryCount == 0) return;
	
	if (m_nISearchHistoryCount == 1) {
		m_bCurSrchKeyMark = false;
		m_bISearchFirst = true;
	}else if (!m_bISearchFlagHistory[m_nISearchHistoryCount]) {
		// �����������ւ炷
		size_t l = m_strCurSearchKey.size();
		if (l > 0) {
			// �Ō�̕����̈�O
			wchar_t* p = (wchar_t*)NativeW::GetCharPrev(m_strCurSearchKey.c_str(), l, &m_strCurSearchKey.c_str()[l]);
			size_t new_len = p - m_strCurSearchKey.c_str();
			m_strCurSearchKey.resize(new_len);
			//m_szCurSrchKey[l-1] = '\0';

			if (new_len > 0) 
				ISearchWordMake();
			else
				m_bCurSrchKeyMark = false;

		}else {
			WarningBeep();
		}
	}
	m_nISearchHistoryCount --;

	LayoutRange sRange = m_searchHistory[m_nISearchHistoryCount];

	if (m_nISearchHistoryCount == 0) {
		GetSelectionInfo().DisableSelectArea(true);
		sRange.SetToX(sRange.GetFrom().x);
	}

	GetCaret().MoveCursor(sRange.GetFrom(), true, _CARETMARGINRATE / 3);
	if (m_nISearchHistoryCount != 0) {
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(sRange);
	}

	Redraw();

	// �X�e�[�^�X�\��
	NativeT msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetStringPtr());
	
}

// ���͕�������A���������𐶐�����B
void EditView::ISearchWordMake(void)
{
	switch (m_nISearchMode) {
	case 1: // �ʏ�C���N�������^���T�[�`
	case 2: // ���K�\���C���N�������^���T�[�`
		m_searchPattern.SetPattern(this->GetHwnd(), m_strCurSearchKey.c_str(), m_strCurSearchKey.size(), m_curSearchOption, &m_curRegexp);
		break;
	case 3: // MIGEMO�C���N�������^���T�[�`
		{
			// migemo�ő{��
			std::wstring strMigemoWord = m_pMigemo->migemo_query_w(m_strCurSearchKey.c_str());
			
			// �����p�^�[���̃R���p�C��
			const wchar_t* p = strMigemoWord.c_str();
			m_searchPattern.SetPattern(this->GetHwnd(), p, (int)strMigemoWord.size(), m_curSearchOption, &m_curRegexp);

		}
		break;
	}
}

/*!	@brief ISearch���b�Z�[�W�\�z

	���݂̃T�[�`���[�h�y�ь����������񂩂�
	���b�Z�[�W�G���A�ɕ\�����镶������\�z����
	
	@param msg [out] ���b�Z�[�W�o�b�t�@
	
	@author isearch
	@date 2004/10/13
	@date 2005.01.13 genta ������C��
*/
void EditView::ISearchSetStatusMsg(NativeT* msg) const
{

	switch (m_nISearchMode) {
	case 1 :
		msg->SetString(_T("I-Search"));
		break;
	case 2 :
		msg->SetString(_T("[RegExp] I-Search"));
		break;
	case 3 :
		msg->SetString(_T("[Migemo] I-Search"));
		break;
	default:
		msg->SetString(_T(""));
		return;
	}
	if ((int)m_nISearchDirection == 0) {
		msg->AppendString(_T(" Backward: "));
	}else {
		msg->AppendString(_T(": "));
	}

	if ((int)m_nISearchHistoryCount > 0)
		msg->AppendString(to_tchar(m_strCurSearchKey.c_str()));
}

/*!
	ISearch��Ԃ��c�[���o�[�ɔ��f������D
	
	@sa CEditWnd::IsFuncChecked()

	@param nCommand [in] ���ׂ����R�}���h��ID
	@return true:�`�F�b�N�L�� / false: �`�F�b�N����
	
	@date 2005.01.10 genta �V�K�쐬
*/
bool EditView::IsISearchEnabled(int nCommand) const
{
	switch (nCommand) {
	case F_ISEARCH_NEXT:
		return m_nISearchMode == 1 && m_nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_PREV:
		return m_nISearchMode == 1 && m_nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_REGEXP_NEXT:
		return m_nISearchMode == 2 && m_nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_REGEXP_PREV:
		return m_nISearchMode == 2 && m_nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_MIGEMO_NEXT:
		return m_nISearchMode == 3 && m_nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_MIGEMO_PREV:
		return m_nISearchMode == 3 && m_nISearchDirection == SearchDirection::Backward;
	}
	return false;
}

