/*!	@file
	@brief EditView�N���X�̕⊮�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/DocReader.h"
#include "charset/charcode.h"  // 2006.06.28 rastiv
#include "window/EditWnd.h"
#include "parse/WordParse.h"
#include "sakura_rc.h"

/*!
	@brief �R�}���h��M�O�⊮����
	
	�⊮�E�B���h�E�̔�\��

	@date 2005.01.10 genta �֐���
*/
void EditView::PreprocessCommand_hokan(int nCommand)
{
	// �⊮�E�B���h�E���\������Ă���Ƃ��A���ʂȏꍇ�������ăE�B���h�E���\���ɂ���
	if (bHokan) {
		if (1
			&& nCommand != F_HOKAN		//	�⊮�J�n�E�I���R�}���h
			&& nCommand != F_WCHAR		//	��������
			&& nCommand != F_IME_CHAR	//	��������
		) {
			editWnd.hokanMgr.Hide();
			bHokan = false;
		}
	}
}

/*!
	�R�}���h���s��⊮����

	@author Moca
	@date 2005.01.10 genta �֐���
*/
void EditView::PostprocessCommand_hokan(void)
{
	if (GetDllShareData().common.helper.bUseHokan && !bExecutingKeyMacro) { // �L�[�{�[�h�}�N���̎��s��
		NativeW memData;

		// �J�[�\�����O�̒P����擾
		if (0 < GetParser().GetLeftWord(&memData, 100)) {
			ShowHokanMgr(memData, false);
		}else {
			if (bHokan) {
				editWnd.hokanMgr.Hide();
				bHokan = false;
			}
		}
	}
}

/*!	�⊮�E�B���h�E��\������
	�E�B���h�E��\��������́AHokanMgr�ɔC����̂ŁAShowHokanMgr�̒m��Ƃ���ł͂Ȃ��B
	
	@param memData [in] �⊮���錳�̃e�L�X�g �uAb�v�Ȃǂ�����B
	@param bAutoDecided [in] ��₪1��������m�肷��

	@date 2005.01.10 genta CEditView_Command����ړ�
*/
void EditView::ShowHokanMgr(NativeW& memData, bool bAutoDecided)
{
	// �⊮�Ώۃ��[�h���X�g�𒲂ׂ�
	NativeW	memHokanWord;
	POINT		poWin;
	// �⊮�E�B���h�E�̕\���ʒu���Z�o
	auto& textArea = GetTextArea();
	int nX = GetCaret().GetCaretLayoutPos().x - textArea.GetViewLeftCol();
	if (nX < 0) {
		poWin.x = 0;
	}else if (textArea.nViewColNum < nX) {
		poWin.x = textArea.GetAreaRight();
	}else {
		poWin.x = textArea.GetAreaLeft() + nX * GetTextMetrics().GetHankakuDx();
	}
	int nY = GetCaret().GetCaretLayoutPos().y - textArea.GetViewTopLine();
	if (nY < 0) {
		poWin.y = 0;
	}else if (textArea.nViewRowNum < nY) {
		poWin.y = textArea.GetAreaBottom();
	}else {
		poWin.y = textArea.GetAreaTop() + nY * GetTextMetrics().GetHankakuDy();
	}
	this->ClientToScreen(&poWin);
	poWin.x -= memData.GetStringLength() * GetTextMetrics().GetHankakuDx();

	/*	�⊮�E�B���h�E��\��
		�������AbAutoDecided == true�̏ꍇ�́A�⊮��₪1�̂Ƃ��́A�E�B���h�E��\�����Ȃ��B
		�ڂ����́ASearch()�̐������Q�Ƃ̂��ƁB
	*/
	NativeW* pMemHokanWord;
	if (bAutoDecided) {
		pMemHokanWord = &memHokanWord;
	}else {
		pMemHokanWord = nullptr;
	}

	// ���͕⊮�E�B���h�E�쐬
	// �ȑO�̓G�f�B�^�N�����ɍ쐬���Ă������K�v�ɂȂ��Ă��炱���ō쐬����悤�ɂ����B
	// �G�f�B�^�N�������ƃG�f�B�^�����̓r���ɂȂ����s���̓��͕⊮�E�B���h�E���ꎞ�I�Ƀt�H�A�O���E���h�ɂȂ��āA
	// �^�u�o�[�ɐV�K�^�u���ǉ������Ƃ��̃^�u�ؑւŃ^�C�g���o�[��������i��u��A�N�e�B�u�\���ɂȂ�̂��͂����茩����j���Ƃ��������B
	// �� Vista/7 �̓���� PC �ł����̂�������H �Y�� PC �ȊO�� Vista/7 PC �ł����܂ɔ����ɕ\�������ꂽ�����ɂȂ���x�̏Ǐ󂪌���ꂽ���A����炪���ꌴ�����ǂ����͕s���B
	auto& hokanMgr = editWnd.hokanMgr;
	if (!hokanMgr.GetHwnd()) {
		hokanMgr.DoModeless(
			G_AppInstance(),
			GetHwnd(),
			(LPARAM)this
		);
		::SetFocus(GetHwnd());	// �G�f�B�^�Ƀt�H�[�J�X��߂�
	}
	int nKouhoNum = hokanMgr.HokanMgr::Search(
		&poWin,
		GetTextMetrics().GetHankakuHeight(),
		GetTextMetrics().GetHankakuDx(),
		memData.GetStringPtr(),
		pTypeData->szHokanFile,
		pTypeData->bHokanLoHiCase,
		pTypeData->bUseHokanByFile, // 2003.06.22 Moca
		pTypeData->nHokanType,
		pTypeData->bUseHokanByKeyword,
		pMemHokanWord
	);
	// �⊮���̐��ɂ���ē����ς���
	if (nKouhoNum <= 0) {				//	��△��
		if (bHokan) {
			hokanMgr.Hide();
			bHokan = false;
			// 2003.06.25 Moca ���s���Ă���A�r�[�v�����o���ĕ⊮�I���B
			ErrorBeep();
		}
	}else if (bAutoDecided && nKouhoNum == 1) { //	���1�̂݁��m��B
		if (bHokan) {
			hokanMgr.Hide();
			bHokan = false;
		}
		// 2004.05.14 Moca HokanMgr::Search���ŉ��s���폜����悤�ɂ��A���ڏ���������̂���߂�

		GetCommander().Command_WordDeleteToStart();
		GetCommander().Command_InsText(true, memHokanWord.GetStringPtr(), memHokanWord.GetStringLength(), true);
	}else {
		bHokan = true;
	}
	
	//	�⊮�I���B
	if (!bHokan) {
		GetDllShareData().common.helper.bUseHokan = false;	//	���͕⊮�I���̒m�点
	}
}


/*!
	�ҏW���f�[�^������͕⊮�L�[���[�h�̌���
	HokanMgr����Ă΂��

	@return ��␔

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta  CEditView_Command����ړ�
	@date 2007/10/17 kobake �ǂ݂₷���悤�Ƀl�X�g��󂭂��܂����B
	@date 2008.07.25 nasukoji �啶���������𓯈ꎋ�̏ꍇ�ł����̐U�邢���Ƃ��͊��S��v�Ō���
	@date 2008.10.11 syat ���{��̕⊮
	@date 2010.06.16 Moca �Ђ炪�Ȃő��s����ꍇ�A���O�������ɐ���
*/
size_t EditView::HokanSearchByFile(
	const wchar_t*	pszKey,					// [in]
	bool			bHokanLoHiCase,			// [in] �p�啶���������𓯈ꎋ����
	vector_ex<std::wstring>& candidates,	// [in,out] ���
	int				nMaxKouho				// [in] Max��␔(0 == ������)
) {
	const size_t nKeyLen = wcslen(pszKey);
	size_t nLines = pEditDoc->docLineMgr.GetLineCount();
	int nRet;
	int nWordLenStop;
	size_t nWordBegin;
	size_t nWordLen;
	size_t nCharSize;
	size_t nLineLen;

	Point ptCur = GetCaret().GetCaretLogicPos(); // �����J�[�\���ʒu

	// �L�[���L���Ŏn�܂邩
	// �L�[�̐擪���L��(#$@\)���ǂ�������
	bool bKeyStartWithMark = wcschr(L"$@#\\", pszKey[0]) != NULL;

	for (size_t i=0; i<nLines; ++i) {
		const wchar_t* pszLine = DocReader(pEditDoc->docLineMgr).GetLineStrWithoutEOL(i, &nLineLen);

		for (size_t j=0; j<nLineLen; j+=nCharSize) {
			nCharSize = NativeW::GetSizeOfChar(pszLine, nLineLen, j);

			// ���p�L���͌��Ɋ܂߂Ȃ�
			if (pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR(pszLine[j])) continue;

			// �L�[�̐擪���L���ȊO�̏ꍇ�A�L���Ŏn�܂�P��͌�₩��͂���
			if (!bKeyStartWithMark && wcschr(L"$@#\\", pszLine[j])) continue;

			// ������ގ擾
			ECharKind kindPre = WordParse::WhatKindOfChar(pszLine, nLineLen, j);	// ������ގ擾

			// �S�p�L���͌��Ɋ܂߂Ȃ�
			if (0
				|| kindPre == CK_ZEN_SPACE
				|| kindPre == CK_ZEN_NOBASU
				|| kindPre == CK_ZEN_DAKU
				|| kindPre == CK_ZEN_KIGO
				|| kindPre == CK_ZEN_SKIGO
			)
				continue;

			// ��₪�L���Ŏn�܂邩
			bool bWordStartWithMark = wcschr(L"$@#\\", pszLine[j]) != NULL;

			nWordBegin = j;
			// ���P��̏I���ʒu�����߂�
			nWordLen = nCharSize;
			nWordLenStop = -1; // ���艼�������p�P��̏I���B-1�͖���
			for (j+=nCharSize; j<nLineLen; j+=nCharSize) {
				nCharSize = NativeW::GetSizeOfChar(pszLine, nLineLen, j);

				// ���p�L���͊܂߂Ȃ�
				if (pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR(pszLine[j])) break;

				// ������ގ擾
				ECharKind kindCur = WordParse::WhatKindOfChar(pszLine, nLineLen, j);
				// �S�p�L���͌��Ɋ܂߂Ȃ�
				if (kindCur == CK_ZEN_SPACE || kindCur == CK_ZEN_KIGO || kindCur == CK_ZEN_SKIGO) {
					break;
				}

				// ������ނ��ς������P��̐؂�ڂƂ���
				ECharKind kindMerge = WordParse::WhatKindOfTwoChars(kindPre, kindCur);
				if (kindMerge == CK_NULL) {	// kindPre��kindCur���ʎ�
					if (kindCur == CK_HIRA) {
						kindMerge = kindCur;		// �Ђ炪�ȂȂ瑱�s
						// 2010.06.16 Moca �����̂ݑ��艼�������Ɋ܂߂�
						if (kindPre != CK_ZEN_ETC) {
							nWordLenStop = (int)nWordLen;
						}
					}else if (bKeyStartWithMark && bWordStartWithMark && kindPre == CK_UDEF) {
						kindMerge = kindCur;		// �L���Ŏn�܂�P��͐������ɂ߂�
					}else {
						j -= nCharSize;
						break;						// ����ȊO�͒P��̐؂��
					}
				}

				kindPre = kindMerge;
				nWordLen += nCharSize;				// ���̕�����
			}

			if (0 < nWordLenStop) {
				nWordLen = nWordLenStop;
			}

			// CDicMgr���̐����ɂ�蒷������P��͖�������
			if (nWordLen > 1020) {
				continue;
			}
			if (nKeyLen > nWordLen) continue;

			// ���P��̊J�n�ʒu�����߂�
			const wchar_t* word = pszLine + nWordBegin;

			// �L�[�Ɣ�r����
			if (bHokanLoHiCase) {
				nRet = auto_memicmp(pszKey, word, nKeyLen);
			}else {
				nRet = auto_memcmp(pszKey, word, nKeyLen);
			}
			if (nRet != 0) continue;

			// �J�[�\���ʒu�̒P��͌�₩��͂���
			if (ptCur.y == i && nWordBegin <= ptCur.x && ptCur.x <= nWordBegin + (int)nWordLen) {	// 2010.02.20 syat �C��// 2008.11.09 syat �C��
				continue;
			}

			// ����ǉ�(�d���͏���)
			{
				std::wstring strWord = std::wstring(word, nWordLen);
				HokanMgr::AddKouhoUnique(candidates, strWord);
			}
			if (nMaxKouho != 0 && nMaxKouho <= (int)candidates.size()) {
				return candidates.size();
			}
		}
	}
	return candidates.size();
}

