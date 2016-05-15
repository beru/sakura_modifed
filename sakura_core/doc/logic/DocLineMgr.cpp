/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/03/05  �V�K�쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
	@date 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
	@date 2005/09/25 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, ao
	Copyright (C) 2001, genta, jepro, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta, frozen, Azumaiya, YAZAKI
	Copyright (C) 2003, Moca, ryoji, genta, �����
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, ryoji, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
#include <list>
#include "DocLineMgr.h"
#include "DocLine.h"// 2002/2/10 aroka �w�b�_����
#include "charset/charcode.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/CodeMediator.h"
// Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "extmodule/Bregexp.h"
#include "_main/global.h"

// May 15, 2000 genta
#include "Eol.h"
#include "mem/Memory.h"// 2002/2/10 aroka

#include "io/FileLoad.h" // 2002/08/30 Moca
#include "io/IoBridge.h"
#include "basis/SakuraBasis.h"
#include "parse/WordParse.h"
#include "util/window.h"
#include "util/fileUtil.h"
#include "debug/RunningTimer.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DocLineMgr::DocLineMgr()
{
	_Init();
}

DocLineMgr::~DocLineMgr()
{
	DeleteAllLine();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �s�f�[�^�̊Ǘ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// pPos�̒��O�ɐV�����s��}��
DocLine* DocLineMgr::InsertNewLine(DocLine* pPos)
{
	DocLine* pDocLineNew = new DocLine;
	_InsertBeforePos(pDocLineNew, pPos);
	return pDocLineNew;
}

// �ŉ����ɐV�����s��}��
DocLine* DocLineMgr::AddNewLine()
{
	DocLine* pDocLineNew = new DocLine;
	_PushBottom(pDocLineNew);
	return pDocLineNew;
}

// �S�Ă̍s���폜����
void DocLineMgr::DeleteAllLine()
{
	DocLine* pDocLine = pDocLineTop;
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		delete pDocLine;
		pDocLine = pDocLineNext;
	}
	_Init();
}


// �s�̍폜
void DocLineMgr::DeleteLine(DocLine* pDocLineDel)
{
	// Prev�؂藣��
	if (pDocLineDel->GetPrevLine()) {
		pDocLineDel->GetPrevLine()->pNext = pDocLineDel->GetNextLine();
	}else {
		pDocLineTop = pDocLineDel->GetNextLine();
	}

	// Next�؂藣��
	if (pDocLineDel->GetNextLine()) {
		pDocLineDel->pNext->pPrev = pDocLineDel->GetPrevLine();
	}else {
		pDocLineBot = pDocLineDel->GetPrevLine();
	}
	
	// �Q�Ɛ؂藣��
	if (pCodePrevRefer == pDocLineDel) {
		pCodePrevRefer = pDocLineDel->GetNextLine();
	}

	// �f�[�^�폜
	delete pDocLineDel;

	// �s�����Z
	--nLines;
	if (nLines == LogicInt(0)) {
		// �f�[�^���Ȃ��Ȃ���
		_Init();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �s�f�[�^�ւ̃A�N�Z�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�w�肳�ꂽ�ԍ��̍s�ւ̃|�C���^��Ԃ�

	@param nLine [in] �s�ԍ�
	@return �s�I�u�W�F�N�g�ւ̃|�C���^�B�Y���s���Ȃ��ꍇ��NULL�B
*/
const DocLine* DocLineMgr::GetLine(LogicInt nLine) const
{
	if (nLines == LogicInt(0)) {
		return nullptr;
	}
	// 2004.03.28 Moca nLine�����̏ꍇ�̃`�F�b�N��ǉ�
	if (nLine < LogicInt(0) || nLine >= nLines) {
		return nullptr;
	}
	LogicInt nCounter;
	DocLine* pDocLine;
	// 2004.03.28 Moca pCodePrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	LogicInt nPrevToLineNumDiff = t_abs(nPrevReferLine - nLine);
	if (!pCodePrevRefer
	  || nLine < nPrevToLineNumDiff
	  || nLines - nLine < nPrevToLineNumDiff
	) {
		if (!pCodePrevRefer) {
			MY_RUNNINGTIMER(runningTimer, "DocLineMgr::GetLine() 	pCodePrevRefer == nullptr");
		}

		if (nLine < (nLines / 2)) {
			nCounter = LogicInt(0);
			pDocLine = pDocLineTop;
			while (pDocLine) {
				if (nLine == nCounter) {
					nPrevReferLine = nLine;
					pCodePrevRefer = pDocLine;
					pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				++nCounter;
			}
		}else {
			nCounter = nLines - LogicInt(1);
			pDocLine = pDocLineBot;
			while (pDocLine) {
				if (nLine == nCounter) {
					nPrevReferLine = nLine;
					pCodePrevRefer = pDocLine;
					pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetPrevLine();
				--nCounter;
			}
		}

	}else {
		if (nLine == nPrevReferLine) {
			nPrevReferLine = nLine;
			pDocLineCurrent = pCodePrevRefer->GetNextLine();
			return pCodePrevRefer;
		}else if (nLine > nPrevReferLine) {
			nCounter = nPrevReferLine + LogicInt(1);
			pDocLine = pCodePrevRefer->GetNextLine();
			while (pDocLine) {
				if (nLine == nCounter) {
					nPrevReferLine = nLine;
					pCodePrevRefer = pDocLine;
					pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				++nCounter;
			}
		}else {
			nCounter = nPrevReferLine - LogicInt(1);
			pDocLine = pCodePrevRefer->GetPrevLine();
			while (pDocLine) {
				if (nLine == nCounter) {
					nPrevReferLine = nLine;
					pCodePrevRefer = pDocLine;
					pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetPrevLine();
				--nCounter;
			}
		}
	}
	return nullptr;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLineMgr::_Init()
{
	pDocLineTop = nullptr;
	pDocLineBot = nullptr;
	nLines = LogicInt(0);
	nPrevReferLine = LogicInt(0);
	pCodePrevRefer = nullptr;
	pDocLineCurrent = nullptr;
	DiffManager::getInstance().SetDiffUse(false);	// DIFF�g�p��	//@@@ 2002.05.25 MIK     //##���CDocListener::OnClear (OnAfterClose) ���쐬���A�����Ɉړ�
}

// -- -- �`�F�[���֐� -- -- // 2007.10.11 kobake �쐬
// �ŉ����ɑ}��
void DocLineMgr::_PushBottom(DocLine* pDocLineNew)
{
	if (!pDocLineTop) {
		pDocLineTop = pDocLineNew;
	}
	pDocLineNew->pPrev = pDocLineBot;

	if (pDocLineBot) {
		pDocLineBot->pNext = pDocLineNew;
	}
	pDocLineBot = pDocLineNew;
	pDocLineNew->pNext = nullptr;

	++nLines;
}

// pPos�̒��O�ɑ}���BpPos�� nullptr ���w�肵���ꍇ�́A�ŉ����ɒǉ��B
void DocLineMgr::_InsertBeforePos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Next��ݒ�
	pDocLineNew->pNext = pPos;

	// New.Prev, Other.Prev��ݒ�
	if (pPos) {
		pDocLineNew->pPrev = pPos->GetPrevLine();
		pPos->pPrev = pDocLineNew;
	}else {
		pDocLineNew->pPrev = pDocLineBot;
		pDocLineBot = pDocLineNew;
	}

	// Other.Next��ݒ�
	if (pDocLineNew->GetPrevLine()) {
		pDocLineNew->GetPrevLine()->pNext = pDocLineNew;
	}else {
		pDocLineTop = pDocLineNew;
	}

	// �s�������Z
	++nLines;
}

// pPos�̒���ɑ}���BpPos�� nullptr ���w�肵���ꍇ�́A�擪�ɒǉ��B
void DocLineMgr::_InsertAfterPos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Prev��ݒ�
	pDocLineNew->pPrev = pPos;

	// New.Next, Other.Next��ݒ�
	if (pPos) {
		pDocLineNew->pNext = pPos->GetNextLine();
		pPos->pNext = pDocLineNew;
	}else {
		pDocLineNew->pNext = pDocLineTop;
		pDocLineTop = pDocLineNew;
	}

	// Other.Prev��ݒ�
	if (pDocLineNew->GetNextLine()) {
		pDocLineNew->pNext->pPrev = pDocLineNew;
	}else {
		pDocLineBot = pDocLineNew;
	}

	// �s�������Z
	++nLines;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief CDocLineMgrDEBUG�p

	@date 2004.03.18 Moca
		pDocLineCurrent��pCodePrevRefer���f�[�^�`�F�[����
		�v�f���w���Ă��邩�̌��؋@�\��ǉ��D

*/
void DocLineMgr::DUMP()
{
#ifdef _DEBUG
	MYTRACE(_T("------------------------\n"));
	
	DocLine* pDocLineNext;
	DocLine* pDocLineEnd = nullptr;
	
	// �������𒲂ׂ�
	bool bIncludeCurrent = false;
	bool bIncludePrevRefer = false;
	LogicInt nNum = LogicInt(0);
	if (pDocLineTop->pPrev) {
		MYTRACE(_T("error: pDocLineTop->m_pPrev != nullptr\n"));
	}
	if (pDocLineBot->pNext) {
		MYTRACE(_T("error: pDocLineBot->pNext != nullptr\n"));
	}
	DocLine* pDocLine = pDocLineTop;
	while (pDocLine) {
		if (pDocLineCurrent == pDocLine) {
			bIncludeCurrent = true;
		}
		if (pCodePrevRefer == pDocLine) {
			bIncludePrevRefer = true;
		}
		if (pDocLine->GetNextLine()) {
			if (pDocLine->pNext == pDocLine) {
				MYTRACE(_T("error: pDocLine->pPrev Invalid value.\n"));
				break;
			}
			if (pDocLine->pNext->pPrev != pDocLine) {
				MYTRACE(_T("error: pDocLine->pNext->pPrev != pDocLine.\n"));
				break;
			}
		}else {
			pDocLineEnd = pDocLine;
		}
		pDocLine = pDocLine->GetNextLine();
		++nNum;
	}
	
	if (pDocLineEnd != pDocLineBot) {
		MYTRACE(_T("error: pDocLineEnd != pDocLineBot"));
	}
	
	if (nNum != nLines) {
		MYTRACE(_T("error: nNum(%d) != nLines(%d)\n"), nNum, nLines);
	}
	if (!bIncludeCurrent && pDocLineCurrent) {
		MYTRACE(_T("error: pDocLineCurrent=%08lxh Invalid value.\n"), pDocLineCurrent);
	}
	if (!bIncludePrevRefer && pCodePrevRefer) {
		MYTRACE(_T("error: pCodePrevRefer =%08lxh Invalid value.\n"), pCodePrevRefer);
	}

	// DUMP
	MYTRACE(_T("nLines=%d\n"), nLines);
	MYTRACE(_T("pDocLineTop=%08lxh\n"), pDocLineTop);
	MYTRACE(_T("pDocLineBot=%08lxh\n"), pDocLineBot);
	pDocLine = pDocLineTop;
	while (pDocLine) {
		pDocLineNext = pDocLine->GetNextLine();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"), pDocLine);
		MYTRACE(_T("\tpPrev; =%08lxh\n"), pDocLine->GetPrevLine());
		MYTRACE(_T("\tpNext; =%08lxh\n"), pDocLine->GetNextLine());

		MYTRACE(_T("\tenumEOLType =%ls\n"), pDocLine->GetEol().GetName());
		MYTRACE(_T("\tnEOLLen =%d\n"), pDocLine->GetEol().GetLen());

//		MYTRACE(_T("\t[%ls]\n"), *(pDocLine->pLine));
		MYTRACE(_T("\tpDocLine->cLine.GetLength()=[%d]\n"), pDocLine->GetLengthWithEOL());
		MYTRACE(_T("\t[%ls]\n"), pDocLine->GetPtr());

		pDocLine = pDocLineNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif // _DEBUG
	return;
}

