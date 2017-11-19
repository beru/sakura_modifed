// �s�f�[�^�̊Ǘ�

#include "StdAfx.h"
#include <stdio.h>
#include <io.h>
#include <list>
#include "DocLineMgr.h"
#include "DocLine.h"
#include "charset/charcode.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/CodeMediator.h"
// ���K�\�����C�u�����̍����ւ�
#include "extmodule/Bregexp.h"
#include "_main/global.h"

#include "Eol.h"
#include "mem/Memory.h"

#include "io/FileLoad.h"
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
	if (nLines == 0) {
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
const DocLine* DocLineMgr::GetLine(size_t nLine) const
{
	if (nLines == 0) {
		return nullptr;
	}
	// nLine�����̏ꍇ�̃`�F�b�N��ǉ�
	if (nLine < 0 || nLine >= nLines) {
		return nullptr;
	}
	int nCounter;
	DocLine* pDocLine;
	// pCodePrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	int nPrevToLineNumDiff = t_abs(nPrevReferLine - (int)nLine);
	ASSERT_GE(nLines, nLine);
	if (!pCodePrevRefer
	  || (int)nLine < nPrevToLineNumDiff
	  || (int)(nLines - nLine) < nPrevToLineNumDiff
	) {
		if (!pCodePrevRefer) {
			MY_RUNNINGTIMER(runningTimer, "DocLineMgr::GetLine() 	pCodePrevRefer == nullptr");
		}

		if (nLine < (nLines / 2)) {
			nCounter = 0;
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
			nCounter = nLines - 1;
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
		}else if ((int)nLine > nPrevReferLine) {
			nCounter = nPrevReferLine + 1;
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
			nCounter = nPrevReferLine - 1;
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
	nLines = 0;
	nPrevReferLine = 0;
	pCodePrevRefer = nullptr;
	pDocLineCurrent = nullptr;
	DiffManager::getInstance().SetDiffUse(false);	// DIFF�g�p��
}

// -- -- �`�F�[���֐� -- --
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

/*!	@brief CDocLineMgrDEBUG�p */
void DocLineMgr::DUMP()
{
#ifdef _DEBUG
	MYTRACE(_T("------------------------\n"));
	
	DocLine* pDocLineNext;
	DocLine* pDocLineEnd = nullptr;
	
	// �������𒲂ׂ�
	bool bIncludeCurrent = false;
	bool bIncludePrevRefer = false;
	int nNum = 0;
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

