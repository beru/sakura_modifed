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
#include "view/EditView.h" // ColorStrategyInfo
#include "view/colors/ColorStrategy.h"
#include "Color_Comment.h"
#include "Color_Quote.h"
#include "Color_RegexKeyword.h"
#include "Color_Found.h"
#include "Color_Url.h"
#include "Color_Numeric.h"
#include "Color_KeywordSet.h"
#include "Color_Found.h"
#include "Color_Heredoc.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "types/TypeSupport.h"


bool _IsPosKeywordHead(const StringRef& str, int nPos)
{
	return (nPos == 0 || !IS_KEYWORD_CHAR(str.At(nPos - 1)));
}

/*! �F�̐؂�ւ�����
	@retval true �F�̕ύX����
	@retval false �F�̕ύX�Ȃ�
*/
bool ColorStrategyInfo::CheckChangeColor(const StringRef& lineStr)
{
	auto& pool = ColorStrategyPool::getInstance();
	pool.SetCurrentView(&view);
	Color_Found*  pFound  = pool.GetFoundStrategy();
	Color_Select* pSelect = pool.GetSelectStrategy();
	bool bChange = false;

	// �I��͈͐F�I��
	if (pStrategySelect) {
		if (pStrategySelect->EndColor(lineStr, this->GetPosInLogic())) {
			pStrategySelect = nullptr;
			bChange = true;
		}
	}
	// �I��͈͐F�J�n
	if (!pStrategySelect) {
		if (pSelect->BeginColorEx(lineStr, this->GetPosInLogic(), pDispPos->GetLayoutLineRef(), this->GetLayout())) {
			pStrategySelect = pSelect;
			bChange = true;
		}
	}

	// �����F�I��
	if (pStrategyFound) {
		if (pStrategyFound->EndColor(lineStr, this->GetPosInLogic())) {
			pStrategyFound = nullptr;
			bChange = true;
		}
	}

	// �����F�J�n
	if (!pStrategyFound) {
		if (pFound->BeginColor(lineStr, this->GetPosInLogic())) {
			pStrategyFound = pFound;
			bChange = true;
		}
	}

	// �F�I��
	if (pStrategy) {
		if (pStrategy->EndColor(lineStr, this->GetPosInLogic())) {
			pStrategy = nullptr;
			bChange = true;
		}
	}

	// �F�J�n
	if (!pStrategy) {
		int size = pool.GetStrategyCount();
		for (int i=0; i<size; ++i) {
			if (pool.GetStrategy(i)->BeginColor(lineStr, this->GetPosInLogic())) {
				pStrategy = pool.GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}

	// �J�[�\���s�w�i�F
	TypeSupport caretLineBg(view, COLORIDX_CARETLINEBG);
	if (caretLineBg.IsDisp() && !view.bMiniMap) {
		if (colorIdxBackLine == COLORIDX_CARETLINEBG) {
			if (pDispPos->GetLayoutLineRef() != view.GetCaret().GetCaretLayoutPos().y) {
				colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else {
			if (pDispPos->GetLayoutLineRef() == view.GetCaret().GetCaretLayoutPos().y) {
				colorIdxBackLine = COLORIDX_CARETLINEBG;
				bChange = true;
			}
		}
	}
	// �����s�̔w�i�F
	TypeSupport evenLineBg(view, COLORIDX_EVENLINEBG);
	if (evenLineBg.IsDisp() && !view.bMiniMap && colorIdxBackLine != COLORIDX_CARETLINEBG) {
		if (colorIdxBackLine == COLORIDX_EVENLINEBG) {
			if (pDispPos->GetLayoutLineRef() % 2 == 0) {
				colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else {
			if (pDispPos->GetLayoutLineRef() % 2 == 1) {
				colorIdxBackLine = COLORIDX_EVENLINEBG;
				bChange = true;
			}
		}
	}
	if (view.bMiniMap) {
		TypeSupport cPageViewBg(view, COLORIDX_PAGEVIEW);
		if (cPageViewBg.IsDisp()) {
			EditView& activeView = view.editWnd.GetActiveView();
			int curLine = pDispPos->GetLayoutLineRef();
			auto viewTopLine = activeView.GetTextArea().GetViewTopLine();
			auto bottomLine = activeView.GetTextArea().GetBottomLine();
			if (colorIdxBackLine == COLORIDX_PAGEVIEW) {
				if (viewTopLine <= curLine && curLine < bottomLine) {
				}else {
					colorIdxBackLine = COLORIDX_TEXT;
					bChange = true;
				}
			}else if (colorIdxBackLine == COLORIDX_TEXT) {
				if (viewTopLine <= curLine && curLine < bottomLine) {
					colorIdxBackLine = COLORIDX_PAGEVIEW;
					bChange = true;
				}
			}
		}
	}

	return bChange;
}

/*! �F�̐؂�ւ�

	@date 2013.05.11 novice ���ۂ̕ύX�͌Ăяo�����ōs��
*/
void ColorStrategyInfo::DoChangeColor(Color3Setting *pcColor)
{
	if (pStrategySelect) {
		index.eColorIndex = pStrategySelect->GetStrategyColor();
	}else if (pStrategyFound) {
		index.eColorIndex = pStrategyFound->GetStrategyColor();
	}else {
		index.eColorIndex = pStrategy->GetStrategyColorSafe();
	}

	if (pStrategyFound) {
		index.eColorIndex2 = pStrategyFound->GetStrategyColor();
	}else {
		index.eColorIndex2 = pStrategy->GetStrategyColorSafe();
	}

	index.eColorIndexBg = colorIdxBackLine;

	*pcColor = index;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �v�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

ColorStrategyPool::ColorStrategyPool()
{
	pView = &(EditWnd::getInstance().GetView(0));
	pcSelectStrategy = new Color_Select();
	pcFoundStrategy = new Color_Found();
//	vStrategies.push_back(new Color_Found);			// �}�b�`������
	vStrategies.push_back(new Color_RegexKeyword);	// ���K�\���L�[���[�h
	vStrategies.push_back(new Color_Heredoc);			// �q�A�h�L�������g
	vStrategies.push_back(new Color_BlockComment(COLORIDX_BLOCK1));	// �u���b�N�R�����g
	vStrategies.push_back(new Color_BlockComment(COLORIDX_BLOCK2));	// �u���b�N�R�����g2
	vStrategies.push_back(new Color_LineComment);		// �s�R�����g
	vStrategies.push_back(new Color_SingleQuote);		// �V���O���N�H�[�e�[�V����������
	vStrategies.push_back(new Color_DoubleQuote);		// �_�u���N�H�[�e�[�V����������
	vStrategies.push_back(new Color_Url);				// URL
	vStrategies.push_back(new Color_Numeric);			// ���p����
	vStrategies.push_back(new Color_KeywordSet);		// �L�[���[�h�Z�b�g

	// �ݒ�X�V
	OnChangeSetting();
}

ColorStrategyPool::~ColorStrategyPool()
{
	SAFE_DELETE(pcSelectStrategy);
	SAFE_DELETE(pcFoundStrategy);
	vStrategiesDisp.clear();
	int size = (int)vStrategies.size();
	for (int i=0; i<size; ++i) {
		delete vStrategies[i];
	}
	vStrategies.clear();
}

ColorStrategy* ColorStrategyPool::GetStrategyByColor(EColorIndexType eColor) const
{
	if (COLORIDX_SEARCH <= eColor && eColor <= COLORIDX_SEARCHTAIL) {
		return pcFoundStrategy;
	}
	int size = (int)vStrategiesDisp.size();
	for (int i=0; i<size; ++i) {
		if (vStrategiesDisp[i]->GetStrategyColor() == eColor) {
			return vStrategiesDisp[i];
		}
	}
	return nullptr;
}

void ColorStrategyPool::NotifyOnStartScanLogic()
{
	pcSelectStrategy->OnStartScanLogic();
	pcFoundStrategy->OnStartScanLogic();
	int size = GetStrategyCount();
	for (int i=0; i<size; ++i) {
		GetStrategy(i)->OnStartScanLogic();
	}
}

// 2005.11.20 Moca�R�����g�̐F������ON/OFF�֌W�Ȃ��s���Ă����o�O���C��
void ColorStrategyPool::CheckColorMODE(
	ColorStrategy**	ppColorStrategy,	// [in/out]
	int					nPos,
	const StringRef&	lineStr
	)
{
	// �F�I��
	if (*ppColorStrategy) {
		if ((*ppColorStrategy)->EndColor(lineStr, nPos)) {
			*ppColorStrategy = nullptr;
		}
	}

	// �F�J�n
	if (!*ppColorStrategy) {
		// CheckColorMODE �̓��C�A�E�g�����S�̂̃{�g���l�b�N�ɂȂ邭�炢�p�ɂɌĂяo�����
		// ��{�N���X����̓��I���z�֐��Ăяo�����g�p����Ɩ����ł��Ȃ��قǂ̃I�[�o�w�b�h�ɂȂ�͗l
		// �����̓G���K���g���������\�D��ŌX�̔h���N���X���� BeginColor() ���Ăяo��
		if (pcHeredoc && pcHeredoc->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcHeredoc; return; }
		if (pcBlockComment1 && pcBlockComment1->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcBlockComment1; return; }
		if (pcBlockComment2 && pcBlockComment2->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcBlockComment2; return; }
		if (pcLineComment && pcLineComment->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcLineComment; return; }
		if (pcSingleQuote && pcSingleQuote->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcSingleQuote; return; }
		if (pcDoubleQuote && pcDoubleQuote->BeginColor(lineStr, nPos)) { *ppColorStrategy = pcDoubleQuote; return; }
	}
}

/*! �ݒ�X�V
*/
void ColorStrategyPool::OnChangeSetting(void)
{
	vStrategiesDisp.clear();

	pcSelectStrategy->Update();
	pcFoundStrategy->Update();
	int size = (int)vStrategies.size();
	for (int i=0; i<size; ++i) {
		vStrategies[i]->Update();

		// �F�����\���Ώۂł���Γo�^
		if (vStrategies[i]->Disp()) {
			vStrategiesDisp.push_back(vStrategies[i]);
		}
	}

	// CheckColorMODE �p
	pcHeredoc = static_cast<Color_Heredoc*>(GetStrategyByColor(COLORIDX_HEREDOC));
	pcBlockComment1 = static_cast<Color_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK1));	// �u���b�N�R�����g
	pcBlockComment2 = static_cast<Color_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK2));	// �u���b�N�R�����g2
	pcLineComment = static_cast<Color_LineComment*>(GetStrategyByColor(COLORIDX_COMMENT));	// �s�R�����g
	pcSingleQuote = static_cast<Color_SingleQuote*>(GetStrategyByColor(COLORIDX_SSTRING));	// �V���O���N�H�[�e�[�V����������
	pcDoubleQuote = static_cast<Color_DoubleQuote*>(GetStrategyByColor(COLORIDX_WSTRING));	// �_�u���N�H�[�e�[�V����������

	// �F���������Ȃ��ꍇ�ɁA�������X�L�b�v�ł���悤�Ɋm�F����
	const TypeConfig& type = EditDoc::GetInstance(0)->docType.GetDocumentAttribute();
	EColorIndexType bSkipColorTypeTable[] = {
		COLORIDX_DIGIT,
		COLORIDX_COMMENT,
		COLORIDX_SSTRING,
		COLORIDX_WSTRING,
		COLORIDX_HEREDOC,
		COLORIDX_URL,
		COLORIDX_KEYWORD1,
		COLORIDX_KEYWORD2,
		COLORIDX_KEYWORD3,
		COLORIDX_KEYWORD4,
		COLORIDX_KEYWORD5,
		COLORIDX_KEYWORD6,
		COLORIDX_KEYWORD7,
		COLORIDX_KEYWORD8,
		COLORIDX_KEYWORD9,
		COLORIDX_KEYWORD10,
	};
	bSkipBeforeLayoutGeneral = true;
	int nKeyword1;
	int bUnuseKeyword = false;
	for (int n=0; n<_countof(bSkipColorTypeTable); ++n) {
		if (bSkipColorTypeTable[n] == COLORIDX_KEYWORD1) {
			nKeyword1 = n;
		}
		if (COLORIDX_KEYWORD1 <= bSkipColorTypeTable[n]
			&& bSkipColorTypeTable[n] <= COLORIDX_KEYWORD10
		) {
			if (type.nKeywordSetIdx[n - nKeyword1] == -1) {
				bUnuseKeyword = true; // -1�ȍ~�͖���
			}
			if (!bUnuseKeyword && type.colorInfoArr[bSkipColorTypeTable[n]].bDisp) {
				bSkipBeforeLayoutGeneral = false;
				break;
			}
		}else if (type.colorInfoArr[bSkipColorTypeTable[n]].bDisp) {
			bSkipBeforeLayoutGeneral = false;
			break;
		}
	}
	if (bSkipBeforeLayoutGeneral) {
		if (type.bUseRegexKeyword) {
			bSkipBeforeLayoutGeneral = false;
		}
	}
	bSkipBeforeLayoutFound = true;
	for (int n=COLORIDX_SEARCH; n<=COLORIDX_SEARCHTAIL; ++n) {
		if (type.colorInfoArr[n].bDisp) {
			bSkipBeforeLayoutFound = false;
			break;
		}
	}
}

bool ColorStrategyPool::IsSkipBeforeLayout()
{
	if (!bSkipBeforeLayoutGeneral) {
		return false;
	}
	if (!bSkipBeforeLayoutFound && pView->bCurSrchKeyMark) {
		return false;
	}
	return true;
}

/*!
  ini�̐F�ݒ��ԍ��łȂ�������ŏ����o���B(added by Stonee, 2001/01/12, 2001/01/15)
  �z��̏��Ԃ͋��L���������̃f�[�^�̏��Ԃƈ�v���Ă���B

  @note ���l�ɂ������I�Ή��� EColorIndexType EColorIndexType.h
  ���{�ꖼ�Ȃǂ�  ColorInfo_DEFAULT CDocTypeSetting.cpp
  CShareData����global�Ɉړ�
*/
const ColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// �L�����b�g		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IME�L�����b�g	// 2006.12.07 ryoji
	{_T("CBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | (COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD)}, // 2007.09.09 Moca �J�[�\���ʒu�c��
	{_T("NOT"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("EBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	// 2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca �w�茅�c��
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK ���p���l�̋���
	{_T("BRC"), 0},	// �Ί���	// 02/09/18 ai Add
	{_T("SEL"), 0},
	{_T("FND"), 0},
	{_T("FN2"), 0},
	{_T("FN3"), 0},
	{_T("FN4"), 0},
	{_T("FN5"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("HDC"), 0},
	{_T("URL"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK �����L�[���[�h3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	// DIFF�ǉ�	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	// DIFF�ύX	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	// DIFF�폜	//@@@ 2002.06.01 MIK
	{_T("MRK"), 0},	// �u�b�N�}�[�N	// 02/10/16 ai Add
	{_T("PGV"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LAST"), 0}	// Not Used
};


/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
int GetColorIndexByName(const TCHAR* name)
{
	for (int i=0; i<COLORIDX_LAST; ++i) {
		if (_tcscmp(name, g_ColorAttributeArr[i].szName) == 0) {
			return i;
		}
	}
	return -1;
}

/*
 * �C���f�b�N�X�ԍ�����J���[���ɕϊ�����
 */
const TCHAR* GetColorNameByIndex(int index)
{
	return g_ColorAttributeArr[index].szName;
}

