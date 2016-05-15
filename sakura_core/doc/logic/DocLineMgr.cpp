/*!	@file
	@brief 行データの管理

	@author Norio Nakatani
	@date 1998/03/05  新規作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
	@date 2001/06/23 N.Nakatani WhereCurrentWord()変更 WhereCurrentWord_2をコールするようにした
	@date 2005/09/25 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, ao
	Copyright (C) 2001, genta, jepro, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta, frozen, Azumaiya, YAZAKI
	Copyright (C) 2003, Moca, ryoji, genta, かろと
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, ryoji, かろと

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
#include <list>
#include "DocLineMgr.h"
#include "DocLine.h"// 2002/2/10 aroka ヘッダ整理
#include "charset/charcode.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/CodeMediator.h"
// Jun. 26, 2001 genta	正規表現ライブラリの差し替え
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
//               コンストラクタ・デストラクタ                  //
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
//                      行データの管理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// pPosの直前に新しい行を挿入
DocLine* DocLineMgr::InsertNewLine(DocLine* pPos)
{
	DocLine* pDocLineNew = new DocLine;
	_InsertBeforePos(pDocLineNew, pPos);
	return pDocLineNew;
}

// 最下部に新しい行を挿入
DocLine* DocLineMgr::AddNewLine()
{
	DocLine* pDocLineNew = new DocLine;
	_PushBottom(pDocLineNew);
	return pDocLineNew;
}

// 全ての行を削除する
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


// 行の削除
void DocLineMgr::DeleteLine(DocLine* pDocLineDel)
{
	// Prev切り離し
	if (pDocLineDel->GetPrevLine()) {
		pDocLineDel->GetPrevLine()->pNext = pDocLineDel->GetNextLine();
	}else {
		pDocLineTop = pDocLineDel->GetNextLine();
	}

	// Next切り離し
	if (pDocLineDel->GetNextLine()) {
		pDocLineDel->pNext->pPrev = pDocLineDel->GetPrevLine();
	}else {
		pDocLineBot = pDocLineDel->GetPrevLine();
	}
	
	// 参照切り離し
	if (pCodePrevRefer == pDocLineDel) {
		pCodePrevRefer = pDocLineDel->GetNextLine();
	}

	// データ削除
	delete pDocLineDel;

	// 行数減算
	--nLines;
	if (nLines == LogicInt(0)) {
		// データがなくなった
		_Init();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   行データへのアクセス                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	指定された番号の行へのポインタを返す

	@param nLine [in] 行番号
	@return 行オブジェクトへのポインタ。該当行がない場合はNULL。
*/
const DocLine* DocLineMgr::GetLine(LogicInt nLine) const
{
	if (nLines == LogicInt(0)) {
		return nullptr;
	}
	// 2004.03.28 Moca nLineが負の場合のチェックを追加
	if (nLine < LogicInt(0) || nLine >= nLines) {
		return nullptr;
	}
	LogicInt nCounter;
	DocLine* pDocLine;
	// 2004.03.28 Moca pCodePrevReferより、Top,Botのほうが近い場合は、そちらを利用する
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
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLineMgr::_Init()
{
	pDocLineTop = nullptr;
	pDocLineBot = nullptr;
	nLines = LogicInt(0);
	nPrevReferLine = LogicInt(0);
	pCodePrevRefer = nullptr;
	pDocLineCurrent = nullptr;
	DiffManager::getInstance().SetDiffUse(false);	// DIFF使用中	//@@@ 2002.05.25 MIK     //##後でCDocListener::OnClear (OnAfterClose) を作成し、そこに移動
}

// -- -- チェーン関数 -- -- // 2007.10.11 kobake 作成
// 最下部に挿入
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

// pPosの直前に挿入。pPosに nullptr を指定した場合は、最下部に追加。
void DocLineMgr::_InsertBeforePos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Nextを設定
	pDocLineNew->pNext = pPos;

	// New.Prev, Other.Prevを設定
	if (pPos) {
		pDocLineNew->pPrev = pPos->GetPrevLine();
		pPos->pPrev = pDocLineNew;
	}else {
		pDocLineNew->pPrev = pDocLineBot;
		pDocLineBot = pDocLineNew;
	}

	// Other.Nextを設定
	if (pDocLineNew->GetPrevLine()) {
		pDocLineNew->GetPrevLine()->pNext = pDocLineNew;
	}else {
		pDocLineTop = pDocLineNew;
	}

	// 行数を加算
	++nLines;
}

// pPosの直後に挿入。pPosに nullptr を指定した場合は、先頭に追加。
void DocLineMgr::_InsertAfterPos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Prevを設定
	pDocLineNew->pPrev = pPos;

	// New.Next, Other.Nextを設定
	if (pPos) {
		pDocLineNew->pNext = pPos->GetNextLine();
		pPos->pNext = pDocLineNew;
	}else {
		pDocLineNew->pNext = pDocLineTop;
		pDocLineTop = pDocLineNew;
	}

	// Other.Prevを設定
	if (pDocLineNew->GetNextLine()) {
		pDocLineNew->pNext->pPrev = pDocLineNew;
	}else {
		pDocLineBot = pDocLineNew;
	}

	// 行数を加算
	++nLines;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief CDocLineMgrDEBUG用

	@date 2004.03.18 Moca
		pDocLineCurrentとpCodePrevReferがデータチェーンの
		要素を指しているかの検証機能を追加．

*/
void DocLineMgr::DUMP()
{
#ifdef _DEBUG
	MYTRACE(_T("------------------------\n"));
	
	DocLine* pDocLineNext;
	DocLine* pDocLineEnd = nullptr;
	
	// 正当性を調べる
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

