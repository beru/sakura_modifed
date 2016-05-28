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
	if (nISearchMode <= 0)
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
	@date 2011.12.15 Moca sCurSearchOption/sSearchOption�Ɠ������Ƃ�
	@date 2012.10.11 novice sCurSearchOption/sSearchOption�̓�����switch�̑O�ɕύX
	@date 2012.10.11 novice MIGEMO�̏�����case���Ɉړ�
*/
void EditView::ISearchEnter(int mode, SearchDirection direction)
{

	if (nISearchMode == mode) {
		// �Ď��s
		nISearchDirection =  direction;
		
		if (bISearchFirst) {
			bISearchFirst = false;
		}
		// ������ƏC��
		ISearchExec(true);

	}else {
		auto& selInfo = GetSelectionInfo();
		// �C���N�������^���T�[�`���[�h�ɓ��邾��.		
		// �I��͈͂̉���
		if (selInfo.IsTextSelected())	
			selInfo.DisableSelectArea(true);

		curSearchOption = GetDllShareData().common.search.searchOption;
		switch (mode) {
		case 1: // �ʏ�C���N�������^���T�[�`
			curSearchOption.bRegularExp = false;
			curSearchOption.bLoHiCase = false;
			curSearchOption.bWordOnly = false;
			//SendStatusMessage(_T("I-Search: "));
			break;
		case 2: // ���K�\���C���N�������^���T�[�`
			if (!curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			curSearchOption.bRegularExp = true;
			curSearchOption.bLoHiCase = false;
			//SendStatusMessage(_T("[RegExp] I-Search: "));
			break;
		case 3: // MIGEMO�C���N�������^���T�[�`
			if (!curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			if (!pMigemo) {
				pMigemo = &Migemo::getInstance();
				pMigemo->InitDll();
			}
			// migemo dll �`�F�b�N
			//	Jan. 10, 2005 genta �ݒ�ύX�Ŏg����悤�ɂȂ��Ă���
			//	�\��������̂ŁC�g�p�\�łȂ���Έꉞ�����������݂�
			if (!pMigemo->IsAvailable() && pMigemo->InitDll() != InitDllResultType::Success) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO1));
				return;
			}
			pMigemo->migemo_load_all();
			if (pMigemo->migemo_is_enable()) {
				curSearchOption.bRegularExp = true;
				curSearchOption.bLoHiCase = false;
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
		if (nISearchMode == 0) {
			ptSrchStartPos_PHY = GetCaret().GetCaretLogicPos();
		}
		
		bCurSrchKeyMark = false;
		nISearchDirection = direction;
		nISearchMode = mode;
		
		nISearchHistoryCount = 0;
		searchHistory[nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());

		Redraw();
		
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		
		bISearchWrap = false;
		bISearchFirst = true;
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
	if (strCurSearchKey.size() < _MAX_PATH) {
		SearchKeywordManager().AddToSearchKeys(strCurSearchKey.c_str());
	}
	nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
	GetDllShareData().common.search.searchOption = curSearchOption;
	editWnd.toolbar.AcceptSharedSearchKey();
	nISearchDirection = SearchDirection::Backward;
	nISearchMode = 0;
	
	if (nISearchHistoryCount == 0) {
		strCurSearchKey.clear();
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
	
	if (bISearchFirst) {
		bISearchFirst = false;
		strCurSearchKey.clear();
	}

	if (wChar <= 0xffff) {
		strCurSearchKey.append(1, (WCHAR)wChar);
	}else {
		strCurSearchKey.append(1, (WCHAR)(wChar>>16));
		strCurSearchKey.append(1, (WCHAR)wChar);
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

	const WCHAR* p = pszText;
	DWORD c;
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

	if ((strCurSearchKey.size() == 0) || (nISearchMode == 0)) {
		// �X�e�[�^�X�̕\��
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		return ;
	}
	
	ISearchWordMake();
	
	int nLine(0);
	int nIdx1(0);
	
	if (bNext && bISearchWrap) {
		switch (nISearchDirection) {
		case 1:
			nLine = 0;
			nIdx1 = 0;
			break;
		case 0:
			// �Ōォ�猟��
			int nLineP;
			int nIdxP;
			auto& docLineMgr = pEditDoc->docLineMgr;
			nLineP =  docLineMgr.GetLineCount() - 1;
			DocLine* pDocLine = docLineMgr.GetLine(nLineP);
			nIdxP = pDocLine->GetLengthWithEOL() -1;
			Point ptTmp = pEditDoc->layoutMgr.LogicToLayout(Point(nIdxP, nLineP));
			nIdx1 = ptTmp.x;
			nLine = ptTmp.y;
		}
	}else if (GetSelectionInfo().IsTextSelected()) {
		auto& select = GetSelectionInfo().select;
		switch ((int)nISearchDirection * 2 + (bNext ? 1: 0)) {
		case 2 : // �O�������Ō��݈ʒu���猟���̂Ƃ�
		case 1 : // ��������Ŏ��������̂Ƃ�
			// �I��͈͂̐擪�������J�n�ʒu��
			nLine = select.GetFrom().y;
			nIdx1 = select.GetFrom().x;
			break;
		case 0 : // �O�������Ŏ�������
		case 3 : // ��������Ō��݈ʒu���猟��
			// �I��͈͂̌�납��
			nLine = select.GetTo().y;
			nIdx1 = select.GetTo().x;
			break;
		}
	}else {
		auto& pos = GetCaret().GetCaretLayoutPos();
		nLine = pos.y;
		nIdx1  = pos.x;
	}

	// ���ʒu����index�ɕϊ�
	Layout* pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(nLine);
	int nIdx = LineColumnToIndex(pLayout, nIdx1);

	nISearchHistoryCount ++ ;

	NativeT msg;
	ISearchSetStatusMsg(&msg);

	if (nISearchHistoryCount >= 256) {
		nISearchHistoryCount = 156;
		for (int i=100; i<=255; ++i) {
			bISearchFlagHistory[i-100] = bISearchFlagHistory[i];
			searchHistory[i-100] = searchHistory[i];
		}
	}
	bISearchFlagHistory[nISearchHistoryCount] = bNext;

	Range matchRange;

	int nSearchResult = pEditDoc->layoutMgr.SearchWord(
		nLine,						// �����J�n���C�A�E�g�s
		nIdx,						// �����J�n�f�[�^�ʒu
		nISearchDirection,		// 0==�O������ 1==�������
		&matchRange,				// �}�b�`���C�A�E�g�͈�
		searchPattern
	);
	if (nSearchResult == 0) {
		// �������ʂ��Ȃ�
		msg.AppendString(LS(STR_EDITVWISRCH_NOMATCH));
		SendStatusMessage(msg.GetStringPtr());
		
		if (bNext) {
			bISearchWrap = true;
		}
		if (GetSelectionInfo().IsTextSelected()) {
			searchHistory[nISearchHistoryCount] = GetSelectionInfo().select;
		}else {
			searchHistory[nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());
		}
	}else {
		// �������ʂ���
		// �L�����b�g�ړ�
		GetCaret().MoveCursor(matchRange.GetFrom(), true, _CARETMARGINRATE / 3);
		
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(matchRange);

		bISearchWrap = false;
		searchHistory[nISearchHistoryCount] = matchRange;
	}

	bCurSrchKeyMark = true;

	Redraw();	
	SendStatusMessage(msg.GetStringPtr());
	return ;
}

// �o�b�N�X�y�[�X�������ꂽ�Ƃ��̏���
void EditView::ISearchBack(void)
{
	if (nISearchHistoryCount == 0) {
		return;
	}
	
	if (nISearchHistoryCount == 1) {
		bCurSrchKeyMark = false;
		bISearchFirst = true;
	}else if (!bISearchFlagHistory[nISearchHistoryCount]) {
		// �����������ւ炷
		size_t l = strCurSearchKey.size();
		if (l > 0) {
			// �Ō�̕����̈�O
			wchar_t* p = (wchar_t*)NativeW::GetCharPrev(strCurSearchKey.c_str(), l, &strCurSearchKey.c_str()[l]);
			size_t new_len = p - strCurSearchKey.c_str();
			strCurSearchKey.resize(new_len);
			//szCurSrchKey[l-1] = '\0';

			if (new_len > 0) 
				ISearchWordMake();
			else
				bCurSrchKeyMark = false;

		}else {
			WarningBeep();
		}
	}
	nISearchHistoryCount --;

	Range range = searchHistory[nISearchHistoryCount];

	if (nISearchHistoryCount == 0) {
		GetSelectionInfo().DisableSelectArea(true);
		range.SetToX(range.GetFrom().x);
	}

	GetCaret().MoveCursor(range.GetFrom(), true, _CARETMARGINRATE / 3);
	if (nISearchHistoryCount != 0) {
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(range);
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
	switch (nISearchMode) {
	case 1: // �ʏ�C���N�������^���T�[�`
	case 2: // ���K�\���C���N�������^���T�[�`
		searchPattern.SetPattern(this->GetHwnd(), strCurSearchKey.c_str(), strCurSearchKey.size(), curSearchOption, &curRegexp);
		break;
	case 3: // MIGEMO�C���N�������^���T�[�`
		{
			// migemo�ő{��
			std::wstring strMigemoWord = pMigemo->migemo_query_w(strCurSearchKey.c_str());
			
			// �����p�^�[���̃R���p�C��
			const wchar_t* p = strMigemoWord.c_str();
			searchPattern.SetPattern(this->GetHwnd(), p, strMigemoWord.size(), curSearchOption, &curRegexp);

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

	switch (nISearchMode) {
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
	if ((int)nISearchDirection == 0) {
		msg->AppendStringLiteral(_T(" Backward: "));
	}else {
		msg->AppendStringLiteral(_T(": "));
	}

	if ((int)nISearchHistoryCount > 0)
		msg->AppendString(to_tchar(strCurSearchKey.c_str()));
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
		return nISearchMode == 1 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_PREV:
		return nISearchMode == 1 && nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_REGEXP_NEXT:
		return nISearchMode == 2 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_REGEXP_PREV:
		return nISearchMode == 2 && nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_MIGEMO_NEXT:
		return nISearchMode == 3 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_MIGEMO_PREV:
		return nISearchMode == 3 && nISearchDirection == SearchDirection::Backward;
	}
	return false;
}

