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
	DocLine* pDocLine = m_pDocLineTop;
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
		pDocLineDel->GetPrevLine()->m_pNext = pDocLineDel->GetNextLine();
	}else {
		m_pDocLineTop = pDocLineDel->GetNextLine();
	}

	// Next切り離し
	if (pDocLineDel->GetNextLine()) {
		pDocLineDel->m_pNext->m_pPrev = pDocLineDel->GetPrevLine();
	}else {
		m_pDocLineBot = pDocLineDel->GetPrevLine();
	}
	
	// 参照切り離し
	if (m_pCodePrevRefer == pDocLineDel) {
		m_pCodePrevRefer = pDocLineDel->GetNextLine();
	}

	// データ削除
	delete pDocLineDel;

	// 行数減算
	--m_nLines;
	if (m_nLines == LogicInt(0)) {
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
	if (m_nLines == LogicInt(0)) {
		return nullptr;
	}
	// 2004.03.28 Moca nLineが負の場合のチェックを追加
	if (nLine < LogicInt(0) || nLine >= m_nLines) {
		return nullptr;
	}
	LogicInt nCounter;
	DocLine* pDocLine;
	// 2004.03.28 Moca m_pCodePrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	LogicInt nPrevToLineNumDiff = t_abs(m_nPrevReferLine - nLine);
	if (!m_pCodePrevRefer
	  || nLine < nPrevToLineNumDiff
	  || m_nLines - nLine < nPrevToLineNumDiff
	) {
		if (!m_pCodePrevRefer) {
			MY_RUNNINGTIMER(runningTimer, "DocLineMgr::GetLine() 	m_pCodePrevRefer == nullptr");
		}

		if (nLine < (m_nLines / 2)) {
			nCounter = LogicInt(0);
			pDocLine = m_pDocLineTop;
			while (pDocLine) {
				if (nLine == nCounter) {
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				++nCounter;
			}
		}else {
			nCounter = m_nLines - LogicInt(1);
			pDocLine = m_pDocLineBot;
			while (pDocLine) {
				if (nLine == nCounter) {
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetPrevLine();
				--nCounter;
			}
		}

	}else {
		if (nLine == m_nPrevReferLine) {
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->GetNextLine();
			return m_pCodePrevRefer;
		}else if (nLine > m_nPrevReferLine) {
			nCounter = m_nPrevReferLine + LogicInt(1);
			pDocLine = m_pCodePrevRefer->GetNextLine();
			while (pDocLine) {
				if (nLine == nCounter) {
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				++nCounter;
			}
		}else {
			nCounter = m_nPrevReferLine - LogicInt(1);
			pDocLine = m_pCodePrevRefer->GetPrevLine();
			while (pDocLine) {
				if (nLine == nCounter) {
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
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
	m_pDocLineTop = nullptr;
	m_pDocLineBot = nullptr;
	m_nLines = LogicInt(0);
	m_nPrevReferLine = LogicInt(0);
	m_pCodePrevRefer = nullptr;
	m_pDocLineCurrent = nullptr;
	DiffManager::getInstance().SetDiffUse(false);	// DIFF使用中	//@@@ 2002.05.25 MIK     //##後でCDocListener::OnClear (OnAfterClose) を作成し、そこに移動
}

// -- -- チェーン関数 -- -- // 2007.10.11 kobake 作成
// 最下部に挿入
void DocLineMgr::_PushBottom(DocLine* pDocLineNew)
{
	if (!m_pDocLineTop) {
		m_pDocLineTop = pDocLineNew;
	}
	pDocLineNew->m_pPrev = m_pDocLineBot;

	if (m_pDocLineBot) {
		m_pDocLineBot->m_pNext = pDocLineNew;
	}
	m_pDocLineBot = pDocLineNew;
	pDocLineNew->m_pNext = nullptr;

	++m_nLines;
}

// pPosの直前に挿入。pPosに nullptr を指定した場合は、最下部に追加。
void DocLineMgr::_InsertBeforePos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Nextを設定
	pDocLineNew->m_pNext = pPos;

	// New.Prev, Other.Prevを設定
	if (pPos) {
		pDocLineNew->m_pPrev = pPos->GetPrevLine();
		pPos->m_pPrev = pDocLineNew;
	}else {
		pDocLineNew->m_pPrev = m_pDocLineBot;
		m_pDocLineBot = pDocLineNew;
	}

	// Other.Nextを設定
	if (pDocLineNew->GetPrevLine()) {
		pDocLineNew->GetPrevLine()->m_pNext = pDocLineNew;
	}else {
		m_pDocLineTop = pDocLineNew;
	}

	// 行数を加算
	++m_nLines;
}

// pPosの直後に挿入。pPosに nullptr を指定した場合は、先頭に追加。
void DocLineMgr::_InsertAfterPos(DocLine* pDocLineNew, DocLine* pPos)
{
	// New.Prevを設定
	pDocLineNew->m_pPrev = pPos;

	// New.Next, Other.Nextを設定
	if (pPos) {
		pDocLineNew->m_pNext = pPos->GetNextLine();
		pPos->m_pNext = pDocLineNew;
	}else {
		pDocLineNew->m_pNext = m_pDocLineTop;
		m_pDocLineTop = pDocLineNew;
	}

	// Other.Prevを設定
	if (pDocLineNew->GetNextLine()) {
		pDocLineNew->m_pNext->m_pPrev = pDocLineNew;
	}else {
		m_pDocLineBot = pDocLineNew;
	}

	// 行数を加算
	++m_nLines;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief CDocLineMgrDEBUG用

	@date 2004.03.18 Moca
		m_pDocLineCurrentとm_pCodePrevReferがデータチェーンの
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
	if (m_pDocLineTop->m_pPrev) {
		MYTRACE(_T("error: m_pDocLineTop->m_pPrev != nullptr\n"));
	}
	if (m_pDocLineBot->m_pNext) {
		MYTRACE(_T("error: m_pDocLineBot->pNext != nullptr\n"));
	}
	DocLine* pDocLine = m_pDocLineTop;
	while (pDocLine) {
		if (m_pDocLineCurrent == pDocLine) {
			bIncludeCurrent = true;
		}
		if (m_pCodePrevRefer == pDocLine) {
			bIncludePrevRefer = true;
		}
		if (pDocLine->GetNextLine()) {
			if (pDocLine->m_pNext == pDocLine) {
				MYTRACE(_T("error: pDocLine->m_pPrev Invalid value.\n"));
				break;
			}
			if (pDocLine->m_pNext->m_pPrev != pDocLine) {
				MYTRACE(_T("error: pDocLine->pNext->m_pPrev != pDocLine.\n"));
				break;
			}
		}else {
			pDocLineEnd = pDocLine;
		}
		pDocLine = pDocLine->GetNextLine();
		++nNum;
	}
	
	if (pDocLineEnd != m_pDocLineBot) {
		MYTRACE(_T("error: pDocLineEnd != m_pDocLineBot"));
	}
	
	if (nNum != m_nLines) {
		MYTRACE(_T("error: nNum(%d) != m_nLines(%d)\n"), nNum, m_nLines);
	}
	if (!bIncludeCurrent && m_pDocLineCurrent) {
		MYTRACE(_T("error: m_pDocLineCurrent=%08lxh Invalid value.\n"), m_pDocLineCurrent);
	}
	if (!bIncludePrevRefer && m_pCodePrevRefer) {
		MYTRACE(_T("error: m_pCodePrevRefer =%08lxh Invalid value.\n"), m_pCodePrevRefer);
	}

	// DUMP
	MYTRACE(_T("m_nLines=%d\n"), m_nLines);
	MYTRACE(_T("m_pDocLineTop=%08lxh\n"), m_pDocLineTop);
	MYTRACE(_T("m_pDocLineBot=%08lxh\n"), m_pDocLineBot);
	pDocLine = m_pDocLineTop;
	while (pDocLine) {
		pDocLineNext = pDocLine->GetNextLine();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"), pDocLine);
		MYTRACE(_T("\tpPrev; =%08lxh\n"), pDocLine->GetPrevLine());
		MYTRACE(_T("\tpNext; =%08lxh\n"), pDocLine->GetNextLine());

		MYTRACE(_T("\tm_enumEOLType =%ls\n"), pDocLine->GetEol().GetName());
		MYTRACE(_T("\tm_nEOLLen =%d\n"), pDocLine->GetEol().GetLen());

//		MYTRACE(_T("\t[%ls]\n"), *(pDocLine->m_pLine));
		MYTRACE(_T("\tpDocLine->m_cLine.GetLength()=[%d]\n"), pDocLine->GetLengthWithEOL());
		MYTRACE(_T("\t[%ls]\n"), pDocLine->GetPtr());

		pDocLine = pDocLineNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif // _DEBUG
	return;
}

