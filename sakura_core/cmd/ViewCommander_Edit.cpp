/*!	@file
@brief ViewCommanderクラスのコマンド(編集系 基本形)関数群

	2012/12/16	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2007, ryoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "view/Ruler.h"
#include "uiparts/WaitCursor.h"
#include "plugin/JackManager.h"
#include "plugin/SmartIndentIfObj.h"
#include "debug/RunningTimer.h"

// wchar_t1個分の文字を入力
void ViewCommander::Command_WCHAR(
	wchar_t wcChar,
	bool bConvertEOL
	)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	LogicInt nPos;
	auto& doc = GetDocument();
	doc.m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	if (m_view.m_bHideMouse && 0 <= m_view.m_nMousePouse) {
		m_view.m_nMousePouse = -1;
		::SetCursor(NULL);
	}

	auto& caret = GetCaret();
	auto& typeData = m_view.m_pTypeData;

	// 現在位置にデータを挿入
	NativeW memDataW2;
	memDataW2 = wcChar;
	if (WCODE::IsLineDelimiter(wcChar, GetDllShareData().common.edit.bEnableExtEol)) { 
		// 現在、Enterなどで挿入する改行コードの種類を取得
		if (bConvertEOL) {
			Eol cWork = doc.m_docEditor.GetNewLineCode();
			memDataW2.SetString(cWork.GetValue2(), cWork.GetLen());
		}

		// テキストが選択されているか
		if (selInfo.IsTextSelected()) {
			m_view.DeleteData(true);
		}
		if (typeData->bAutoIndent) {	// オートインデント
			const Layout* pLayout;
			LogicInt nLineLen;
			auto& layoutMgr = doc.m_layoutMgr;
			const wchar_t* pLine = layoutMgr.GetLineStr(caret.GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
			if (pLayout) {
				const DocLine* pDocLine = doc.m_docLineMgr.GetLine(pLayout->GetLogicLineNo());
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (pLine) {
					/*
					  カーソル位置変換
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					  →
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					*/
					LogicPoint ptXY;
					layoutMgr.LayoutToLogic(
						caret.GetCaretLayoutPos(),
						&ptXY
					);

					// 指定された桁に対応する行のデータ内の位置を調べる
					for (nPos=LogicInt(0); nPos<nLineLen && nPos<ptXY.GetX2();) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						LogicInt nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nPos);

						// その他のインデント文字
						if (0 < nCharChars
						 && pLine[nPos] != L'\0'	// その他のインデント文字に L'\0' は含まれない	// 2009.02.04 ryoji L'\0'がインデントされてしまう問題修正
						 && typeData->szIndentChars[0] != L'\0'
						) {
							wchar_t szCurrent[10];
							wmemcpy(szCurrent, &pLine[nPos], nCharChars);
							szCurrent[nCharChars] = L'\0';
							// その他のインデント対象文字
							if (wcsstr(
									typeData->szIndentChars,
									szCurrent
								)
							) {
								goto end_of_for;
							}
						}
						
						{
							bool bZenSpace = typeData->bAutoIndent_ZENSPACE;
							if (nCharChars == 1 && WCODE::IsIndentChar(pLine[nPos], bZenSpace)) {
								// 下へ進む
							}
							else break;
						}

end_of_for:;
						nPos += nCharChars;
					}

					// インデント取得
					//NativeW memIndent;
					//memIndent.SetString(pLine, nPos);

					// インデント付加
					memDataW2.AppendString(pLine, nPos);
				}
			}
		}
	}else {
		// テキストが選択されているか
		if (selInfo.IsTextSelected()) {
			// 矩形範囲選択中か
			if (selInfo.IsBoxSelecting()) {
				Command_INDENT(wcChar);
				return;
			}else {
				m_view.DeleteData(true);
			}
		}else {
			if (!m_view.IsInsMode() /* Oct. 2, 2005 genta */) {
				DelCharForOverwrite(&wcChar, 1);	// 上書き用の一文字削除	// 2009.04.11 ryoji
			}
		}
	}

	// 本文に挿入する
	LayoutPoint ptLayoutNew;
	m_view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memDataW2.GetStringPtr(),
		memDataW2.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// 挿入データの最後へカーソルを移動
	caret.MoveCursor(ptLayoutNew, true);
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	// スマートインデント
	SmartIndentType nSIndentType = typeData->eSmartIndent;
	switch (nSIndentType) {	// スマートインデント種別
	case SmartIndentType::None:
		break;
	case SmartIndentType::Cpp:
		// C/C++スマートインデント処理
		m_view.SmartIndent_CPP(wcChar);
		break;
	default:
		// プラグインから検索する
		{
			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_SMARTINDENT, (PlugId)nSIndentType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// インタフェースオブジェクト準備
				WSHIfObj::List params;
				SmartIndentIfObj* objIndent = new SmartIndentIfObj(wcChar);	// スマートインデントオブジェクト
				objIndent->AddRef();
				params.push_back(objIndent);

				// キー入力をアンドゥバッファに反映
				m_view.SetUndoBuffer();

				// キー入力とは別の操作ブロックにする（ただしプラグイン内の操作はまとめる）
				if (!GetOpeBlk()) {
					SetOpeBlk(new OpeBlk);
				}
				GetOpeBlk()->AddRef();	// ※ReleaseはHandleCommandの最後で行う

				// プラグイン呼び出し
				(*plugs.begin())->Invoke(&m_view, params);
				objIndent->Release();
			}
		}
		break;
	}

	// 2005.10.11 ryoji 改行時に末尾の空白を削除
	if (WCODE::IsLineDelimiter(
			wcChar,
			GetDllShareData().common.edit.bEnableExtEol
		)
		&& typeData->bRTrimPrevLine
	) {	// 改行時に末尾の空白を削除
		// 前の行にある末尾の空白を削除する
		m_view.RTrimPrevLine();
	}

	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta 関数化
}


/*!
	@brief 2バイト文字入力
	
	WM_IME_CHARで送られてきた文字を処理する．
	ただし，挿入モードではWM_IME_CHARではなくWM_IME_COMPOSITIONで文字列を
	取得するのでここには来ない．

	@param wChar [in] SJIS漢字コード．上位が1バイト目，下位が2バイト目．
	
	@date 2002.10.06 genta 引数の上下バイトの意味を逆転．
		WM_IME_CHARのwParamに合わせた．
*/
void ViewCommander::Command_IME_CHAR(WORD wChar)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	// Oct. 6 ,2002 genta 上下逆転
	if ((wChar & 0xff00) == 0) {
		Command_WCHAR(wChar & 0xff);
		return;
	}
	GetDocument().m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

 	if (m_view.m_bHideMouse && 0 <= m_view.m_nMousePouse) {
		m_view.m_nMousePouse = -1;
		::SetCursor(NULL);
	}

	// Oct. 6 ,2002 genta バッファに格納する
	// Aug. 15, 2007 kobake WCHARバッファに変換する
#ifdef _UNICODE
	wchar_t szWord[2] = {wChar, 0};
#else
	ACHAR szAnsiWord[3] = {(wChar >> 8) & 0xff, wChar & 0xff, 0};
	const wchar_t* pUniData = to_wchar(szAnsiWord);
	wchar_t szWord[2] = {pUniData[0], 0};
#endif
	LogicInt nWord = LogicInt(1);

	// テキストが選択されているか
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			Command_INDENT(szWord, nWord);	// Oct. 6 ,2002 genta 
			return;
		}else {
			m_view.DeleteData(true);
		}
	}else {
		if (!m_view.IsInsMode()) {	// Oct. 2, 2005 genta
			DelCharForOverwrite(szWord, nWord);	// 上書き用の一文字削除	// 2009.04.11 ryoji
		}
	}

	// Oct. 6 ,2002 genta 
	LayoutPoint ptLayoutNew;
	auto& caret = GetCaret();
	m_view.InsertData_CEditView(caret.GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true);

	// 挿入データの最後へカーソルを移動
	caret.MoveCursor(ptLayoutNew, true);
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta 関数化
}


// from ViewCommander_New.cpp
// Undo 元に戻す
void ViewCommander::Command_UNDO(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = m_view.m_commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			m_view.SetUndoBuffer();
			if (!m_view.m_commander.GetOpeBlk() && 0 < nCount) {
				m_view.m_commander.SetOpeBlk(new OpeBlk());
				m_view.m_commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
	}

	if (!GetDocument().m_docEditor.IsEnableUndo()) {	// Undo(元に戻す)可能な状態か？
		return;
	}

	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_UNDO()");

	Ope*		pOpe = NULL;

	OpeBlk*	pOpeBlk;
	int			nOpeBlkNum;
	bool		bIsModified;
//	int			nNewLine;	// 挿入された部分の次の位置の行
//	int			nNewPos;	// 挿入された部分の次の位置のデータ位置

	LayoutPoint ptCaretPos_Before;
	LayoutPoint ptCaretPos_After;

	// 各種モードの取り消し
	Command_CANCEL_MODE();

	m_view.m_bDoing_UndoRedo = true;	// Undo, Redoの実行中か

	// 現在のUndo対象の操作ブロックを返す
	auto& caret = GetCaret();
	auto& docEditor = GetDocument().m_docEditor;
	if ((pOpeBlk = docEditor.m_opeBuf.DoUndo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDraw);	// hor


		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().m_layoutMgr;
		for (int i=nOpeBlkNum-1; i>=0; --i) {
			Ope* pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_After,
					&ptCaretPos_After
				);
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);

				// カーソルを移動
				caret.MoveCursor(ptCaretPos_After, false);
			}

			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// 選択範囲の変更
					LogicRange selectLogic;
					selectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					selectLogic.SetTo(pOpe->m_ptCaretPos_PHY_After);
					if (bFastMode) {
					}else {
						selInfo.m_selectBgn.SetFrom(ptCaretPos_Before);
						selInfo.m_selectBgn.SetTo(selInfo.m_selectBgn.GetFrom());
						selInfo.m_select.SetFrom(ptCaretPos_Before);
						selInfo.m_select.SetTo(ptCaretPos_After);
					}

					// データ置換 削除&挿入にも使える
					bDrawAll |= m_view.ReplaceData_CEditView3(
						selInfo.m_select,				// 削除範囲
						&pInsertOpe->m_opeLineData,	// 削除されたデータのコピー(NULL可能)
						NULL,
						bDraw,						// 再描画するか否か
						NULL,
						pInsertOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&selectLogic
					);

					// 選択範囲の変更
					selInfo.m_selectBgn.Clear(-1); // 範囲選択(原点)
					selInfo.m_select.Clear(-1);
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					// 2007.10.17 kobake メモリリークしてました。修正。
					if (0 < pDeleteOpe->m_opeLineData.size()) {
						// データ置換 削除&挿入にも使える
						LayoutRange range;
						range.Set(ptCaretPos_Before);
						LogicRange cSelectLogic;
						cSelectLogic.Set(pOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_view.ReplaceData_CEditView3(
							range,
							NULL,										// 削除されたデータのコピー(NULL可能)
							&pDeleteOpe->m_opeLineData,
							bDraw,										// 再描画するか否か
							NULL,
							0,
							&pDeleteOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);
					}
					pDeleteOpe->m_opeLineData.clear();
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					LayoutRange range;
					range.SetFrom(ptCaretPos_Before);
					range.SetTo(ptCaretPos_After);
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pOpe->m_ptCaretPos_PHY_After);

					// データ置換 削除&挿入にも使える
					bDrawAll |= m_view.ReplaceData_CEditView3(
						range,				// 削除範囲
						&pReplaceOpe->m_pMemDataIns,	// 削除されたデータのコピー(NULL可能)
						&pReplaceOpe->m_pMemDataDel,	// 挿入するデータ
						bDraw,						// 再描画するか否か
						NULL,
						pReplaceOpe->m_nOrgInsSeq,
						&pReplaceOpe->m_nOrgDelSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->m_pMemDataDel.clear();
				}
				break;
			case OpeCode::MoveCaret:
				// カーソルを移動
				if (bFastMode) {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
				}else {
					caret.MoveCursor(ptCaretPos_After, false);
				}
				break;
			}

			if (bFastMode) {
				if (i == 0) {
					layoutMgr._DoLayout();
					GetEditWindow()->ClearViewCaretPosInfo();
					if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					caret.MoveCursor(ptCaretPos_Before, true);
					// 通常モードではReplaceData_CEditViewの中で設定される
					caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_Before);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);
				// カーソルを移動
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv(nOpeBlkNum - i, 100, nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		m_view.SetDrawSwitch(bDrawSwitchOld);	// hor
		m_view.AdjustScrollBars(); // 2007.07.22 ryoji

		// Undo後の変更フラグ
		docEditor.SetModified(bIsModified, true);	// Jan. 22, 2002 genta

		m_view.m_bDoing_UndoRedo = false;	// Undo, Redoの実行中か

		m_view.SetBracketPairPos(true);	// 03/03/07 ai

		// 再描画
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する	// 2010.08.20 ryoji
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平Scrollもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = m_view.GetRuler().GetRedrawFlag();
		m_view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0), false);
		if (!bRedrawRuler) {
			// ルーラーのキャレットのみを再描画
			HDC hdc = m_view.GetDC();
			m_view.GetRuler().DispRuler(hdc);
			m_view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if (!GetEditWindow()->UpdateTextWrap() && bDrawAll) {	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews(&m_view);	// 他のペインの表示を更新
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_view.m_bDoing_UndoRedo = false;	// Undo, Redoの実行中か

	return;
}


// from ViewCommander_New.cpp
// Redo やり直し
void ViewCommander::Command_REDO(void)
{
	if (m_view.GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = m_view.m_commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			m_view.SetUndoBuffer();
			if (!m_view.m_commander.GetOpeBlk() && 0 < nCount) {
				m_view.m_commander.SetOpeBlk(new OpeBlk());
				m_view.m_commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
		// 注意：Opeを追加するとRedoはできなくなる
	}

	auto& docEditor = GetDocument().m_docEditor;
	if (!docEditor.IsEnableRedo()) {	// Redo(やり直し)可能な状態か？
		return;
	}
	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_REDO()");

	Ope*		pOpe = NULL;
	OpeBlk*	pOpeBlk;
	int			nOpeBlkNum;
//	int			nNewLine;	// 挿入された部分の次の位置の行
//	int			nNewPos;	// 挿入された部分の次の位置のデータ位置
	bool		bIsModified;

	LayoutPoint ptCaretPos_Before;
	LayoutPoint ptCaretPos_To;
	LayoutPoint ptCaretPos_After;

	// 各種モードの取り消し
	Command_CANCEL_MODE();

	m_view.m_bDoing_UndoRedo = true;	// Undo, Redoの実行中か

	// 現在のRedo対象の操作ブロックを返す
	auto& caret = GetCaret();
	if ((pOpeBlk = docEditor.m_opeBuf.DoRedo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDraw);	// 2007.07.22 ryoji

		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().m_layoutMgr;
		for (int i=0; i<nOpeBlkNum; ++i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				if (i == 0) {
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					caret.MoveCursor(ptCaretPos_Before, true);
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_Before);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// 2007.10.17 kobake メモリリークしてました。修正。
					if (0 < pInsertOpe->m_opeLineData.size()) {
						// データ置換 削除&挿入にも使える
						LayoutRange range;
						range.Set(ptCaretPos_Before);
						LogicRange cSelectLogic;
						cSelectLogic.Set(pOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_view.ReplaceData_CEditView3(
							range,
							NULL,										// 削除されたデータのコピー(NULL可能)
							&pInsertOpe->m_opeLineData,				// 挿入するデータ
							bDraw,										// 再描画するか否か
							NULL,
							0,
							&pInsertOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);

					}
					pInsertOpe->m_opeLineData.clear();
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (bFastMode) {
					}else {
						layoutMgr.LogicToLayout(
							pDeleteOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pDeleteOpe->m_ptCaretPos_PHY_To);

					// データ置換 削除&挿入にも使える
					bDrawAll |= m_view.ReplaceData_CEditView3(
						LayoutRange(ptCaretPos_Before, ptCaretPos_To),
						&pDeleteOpe->m_opeLineData,	// 削除されたデータのコピー(NULL可能)
						NULL,
						bDraw,
						NULL,
						pDeleteOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&cSelectLogic
					);
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					if (bFastMode) {
					}else {
						layoutMgr.LogicToLayout(
							pReplaceOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pReplaceOpe->m_ptCaretPos_PHY_To);

					// データ置換 削除&挿入にも使える
					bDrawAll |= m_view.ReplaceData_CEditView3(
						LayoutRange(ptCaretPos_Before, ptCaretPos_To),
						&pReplaceOpe->m_pMemDataDel,	// 削除されたデータのコピー(NULL可能)
						&pReplaceOpe->m_pMemDataIns,	// 挿入するデータ
						bDraw,
						NULL,
						pReplaceOpe->m_nOrgDelSeq,
						&pReplaceOpe->m_nOrgInsSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->m_pMemDataIns.clear();
				}
				break;
			case OpeCode::MoveCaret:
				break;
			}
			if (bFastMode) {
				if (i == nOpeBlkNum - 1) {
					layoutMgr._DoLayout();
					GetEditWindow()->ClearViewCaretPosInfo();
					if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After);
					caret.MoveCursor(ptCaretPos_After, true);
					// 通常モードではReplaceData_CEditViewの中で設定される
					caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After);
				caret.MoveCursor(ptCaretPos_After, (i == nOpeBlkNum - 1));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv(i + 1, 100, nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		m_view.SetDrawSwitch(bDrawSwitchOld); // 2007.07.22 ryoji
		m_view.AdjustScrollBars(); // 2007.07.22 ryoji

		// Redo後の変更フラグ
		docEditor.SetModified(bIsModified, true);	// Jan. 22, 2002 genta

		m_view.m_bDoing_UndoRedo = false;	// Undo, Redoの実行中か

		m_view.SetBracketPairPos(true);	// 03/03/07 ai

		// 再描画
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する	// 2010.08.20 ryoji
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平Scrollもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = m_view.GetRuler().GetRedrawFlag();
		m_view.Call_OnPaint(
			(int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0),
			false
		);
		if (!bRedrawRuler) {
			// ルーラーのキャレットのみを再描画
			HDC hdc = m_view.GetDC();
			m_view.GetRuler().DispRuler(hdc);
			m_view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// キャレットの行桁位置を表示する	// 2007.10.19 ryoji

		if (!GetEditWindow()->UpdateTextWrap())	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews(&m_view);	// 他のペインの表示を更新

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji 追加
	m_view.m_bDoing_UndoRedo = false;	// Undo, Redoの実行中か

	return;
}


// カーソル位置または選択エリアを削除
void ViewCommander::Command_DELETE(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {		// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!selInfo.IsTextSelected()) {	// テキストが選択されているか
		auto& layoutMgr = GetDocument().m_layoutMgr;
		// 2008.08.03 nasukoji	選択範囲なしでDELETEを実行した場合、カーソル位置まで半角スペースを挿入した後改行を削除して次行と連結する
		auto& caret = GetCaret();
		if (layoutMgr.GetLineCount() > caret.GetCaretLayoutPos().GetY2()) {
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
			if (pLayout) {
				LayoutInt nLineLen;
				LogicInt nIndex;
				nIndex = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
				if (nLineLen != 0) {	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					if (pLayout->GetLayoutEol().GetType() != EolType::None) {	// 行終端は改行コードか?
						Command_INSTEXT(true, L"", LogicInt(0), FALSE);	// カーソル位置まで半角スペース挿入
					}else {	// 行終端が折り返し
						// 折り返し行末ではスペース挿入後、次の文字を削除する	// 2009.02.19 ryoji

						// フリーカーソル時の折り返し越え位置での削除はどうするのが妥当かよくわからないが
						// 非フリーカーソル時（ちょうどカーソルが折り返し位置にある）には次の行の先頭文字を削除したい

						if (nLineLen < caret.GetCaretLayoutPos().GetX2()) {	// 折り返し行末とカーソルの間に隙間がある
							Command_INSTEXT(true, L"", LogicInt(0), FALSE);	// カーソル位置まで半角スペース挿入
							pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
							nIndex = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
						}
						if (nLineLen != 0) {	// （スペース挿入後も）折り返し行末なら次文字を削除するために次行の先頭に移動する必要がある
							if (pLayout->GetNextLayout()) {	// 最終行末ではない
								LayoutPoint ptLay;
								LogicPoint ptLog(pLayout->GetLogicOffset() + nIndex, pLayout->GetLogicLineNo());
								layoutMgr.LogicToLayout(ptLog, &ptLay);
								caret.MoveCursor(ptLay, true);
								caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_view.DeleteData(true);
	return;
}


// カーソル前を削除
void ViewCommander::Command_DELETE_BACK(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	// May 29, 2004 genta 実際に削除された文字がないときはフラグをたてないように
	//GetDocument().m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta
	if (selInfo.IsTextSelected()) {				// テキストが選択されているか
		m_view.DeleteData(true);
	}else {
		auto& caret = GetCaret();
		LayoutPoint	ptLayoutPos_Old = caret.GetCaretLayoutPos();
		LogicPoint		ptLogicPos_Old = caret.GetCaretLogicPos();
		BOOL bBool = Command_LEFT(false, false);
		if (bBool) {
			const Layout* pLayout = GetDocument().m_layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
			if (pLayout) {
				LayoutInt nLineLen;
				LogicInt nIdx = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
				if (nLineLen == 0) {	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					// 右からの移動では折り返し末尾文字は削除するが改行は削除しない
					// 下から（下の行の行頭から）の移動では改行も削除する
					if (nIdx < pLayout->GetLengthWithoutEOL() || caret.GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2()) {
						if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
							// 操作の追加
							GetOpeBlk()->AppendOpe(
								new MoveCaretOpe(
									ptLogicPos_Old,
									caret.GetCaretLogicPos()
								)
							);
						}
						m_view.DeleteData(true);
					}
				}
			}
		}
	}
	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta 関数化
}


// 	上書き用の一文字削除	2009.04.11 ryoji
void ViewCommander::DelCharForOverwrite(
	const wchar_t* pszInput,
	int nLen
	)
{
	bool bEol = false;
	bool bDelete = true;
	const Layout* pLayout = GetDocument().m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	int nDelLen = LogicInt(0);
	LayoutInt nKetaDiff = LayoutInt(0);
	LayoutInt nKetaAfterIns = LayoutInt(0);
	if (pLayout) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		LogicInt nIdxTo = m_view.LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());
		if (nIdxTo >= pLayout->GetLengthWithoutEOL()) {
			bEol = true;	// 現在位置は改行または折り返し以後
			if (pLayout->GetLayoutEol() != EolType::None) {
				if (GetDllShareData().common.edit.bNotOverWriteCRLF) {	// 改行は上書きしない
					// 現在位置が改行ならば削除しない
					bDelete = false;
				}
			}
		}else {
			// 文字幅に合わせてスペースを詰める
			if (GetDllShareData().common.edit.bOverWriteFixMode) {
				const StringRef line = pLayout->GetDocLineRef()->GetStringRefWithEOL();
				LogicInt nPos = GetCaret().GetCaretLogicPos().GetX();
				if (line.At(nPos) != WCODE::TAB) {
					LayoutInt nKetaBefore = NativeW::GetKetaOfChar(line, nPos);
					LayoutInt nKetaAfter = NativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = nKetaBefore - nKetaAfter;
					nPos += NativeW::GetSizeOfChar(line.GetPtr(), line.GetLength(), nPos);
					nDelLen = 1;
					if (nKetaDiff < 0 && nPos < line.GetLength()) {
						wchar_t c = line.At(nPos);
						if (c != WCODE::TAB
							&& !WCODE::IsLineDelimiter(
								c,
								GetDllShareData().common.edit.bEnableExtEol
							)
						) {
							nDelLen = 2;
							LayoutInt nKetaBefore2 = NativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = nKetaBefore + nKetaBefore2 - nKetaAfter;
						}
					}
				}
			}
		}
	}
	if (bDelete) {
		// 上書きモードなので、現在位置の文字を１文字消去
		LayoutPoint posBefore;
		if (bEol) {
			Command_DELETE();	// 行数減では再描画が必要＆行末以後の削除を処理統一
			posBefore = GetCaret().GetCaretLayoutPos();
		}else {
			// 1文字削除
			m_view.DeleteData(false);
			posBefore = GetCaret().GetCaretLayoutPos();
			for (int i=1; i<nDelLen; ++i) {
				m_view.DeleteData(false);
			}
		}
		NativeW tmp;
		for (LayoutInt i=LayoutInt(0); i<nKetaDiff; ++i) {
			tmp.AppendString(L" ");
		}
		for (LayoutInt i=LayoutInt(0); i<nKetaAfterIns; ++i) {
			tmp.AppendString(L" ");
		}
		if (0 < tmp.GetStringLength()) {
			Command_INSTEXT(false, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			GetCaret().MoveCursor(posBefore, false);
		}
	}
}

