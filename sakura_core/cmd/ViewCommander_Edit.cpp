#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "view/Ruler.h"
#include "uiparts/WaitCursor.h"
#include "plugin/JackManager.h"
#include "plugin/SmartIndentIfObj.h"
#include "debug/RunningTimer.h"

// ViewCommanderクラスのコマンド(編集系 基本形)関数群

// wchar_t1個分の文字を入力
void ViewCommander::Command_WCHAR(
	wchar_t wcChar,
	bool bConvertEOL
	)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	size_t nPos;
	auto& doc = GetDocument();
	doc.docEditor.SetModified(true, true);

	if (view.bHideMouse && 0 <= view.nMousePouse) {
		view.nMousePouse = -1;
		::SetCursor(NULL);
	}

	auto& caret = GetCaret();
	auto& typeData = view.pTypeData;

	// 現在位置にデータを挿入
	NativeW memDataW2;
	memDataW2 = wcChar;
	if (WCODE::IsLineDelimiter(wcChar, GetDllShareData().common.edit.bEnableExtEol)) { 
		// 現在、Enterなどで挿入する改行コードの種類を取得
		if (bConvertEOL) {
			Eol cWork = doc.docEditor.GetNewLineCode();
			memDataW2.SetString(cWork.GetValue2(), cWork.GetLen());
		}

		// テキストが選択されているか
		if (selInfo.IsTextSelected()) {
			view.DeleteData(true);
		}
		if (typeData->bAutoIndent) {	// オートインデント
			const Layout* pLayout;
			size_t nLineLen;
			auto& layoutMgr = doc.layoutMgr;
			const wchar_t* pLine = layoutMgr.GetLineStr(caret.GetCaretLayoutPos().y, &nLineLen, &pLayout);
			if (pLayout) {
				const DocLine* pDocLine = doc.docLineMgr.GetLine(pLayout->GetLogicLineNo());
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (pLine) {
					/*
					  カーソル位置変換
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					  →
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					*/
					Point ptXY = layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos());

					// 指定された桁に対応する行のデータ内の位置を調べる
					ASSERT_GE(ptXY.x, 0);
					for (nPos=0; nPos<nLineLen && nPos<(size_t)ptXY.x;) {
						size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nPos);

						// その他のインデント文字
						if (0 < nCharChars
						 && pLine[nPos] != L'\0'	// その他のインデント文字に L'\0' は含まれない
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
				Command_Indent(wcChar);
				return;
			}else {
				view.DeleteData(true);
			}
		}else {
			if (!view.IsInsMode()) {
				DelCharForOverwrite(&wcChar, 1);	// 上書き用の一文字削除
			}
		}
	}

	// 本文に挿入する
	Point ptLayoutNew;
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memDataW2.GetStringPtr(),
		memDataW2.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// 挿入データの最後へカーソルを移動
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	// スマートインデント
	SmartIndentType nSIndentType = typeData->eSmartIndent;
	switch (nSIndentType) {	// スマートインデント種別
	case SmartIndentType::None:
		break;
	case SmartIndentType::Cpp:
		// C/C++スマートインデント処理
		view.SmartIndent_CPP(wcChar);
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
				view.SetUndoBuffer();

				// キー入力とは別の操作ブロックにする（ただしプラグイン内の操作はまとめる）
				if (!GetOpeBlk()) {
					SetOpeBlk(new OpeBlk);
				}
				GetOpeBlk()->AddRef();	// ※ReleaseはHandleCommandの最後で行う

				// プラグイン呼び出し
				(*plugs.begin())->Invoke(view, params);
				objIndent->Release();
			}
		}
		break;
	}

	// 改行時に末尾の空白を削除
	if (WCODE::IsLineDelimiter(
			wcChar,
			GetDllShareData().common.edit.bEnableExtEol
		)
		&& typeData->bRTrimPrevLine
	) {	// 改行時に末尾の空白を削除
		// 前の行にある末尾の空白を削除する
		view.RTrimPrevLine();
	}

	view.PostprocessCommand_hokan();
}


/*!
	@brief 2バイト文字入力
	
	WM_IME_CHARで送られてきた文字を処理する．
	ただし，挿入モードではWM_IME_CHARではなくWM_IME_COMPOSITIONで文字列を
	取得するのでここには来ない．

	@param wChar [in] SJIS漢字コード．上位が1バイト目，下位が2バイト目．
*/
void ViewCommander::Command_IME_CHAR(WORD wChar)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	// 上下逆転
	if ((wChar & 0xff00) == 0) {
		Command_WCHAR(wChar & 0xff);
		return;
	}
	GetDocument().docEditor.SetModified(true, true);

 	if (view.bHideMouse && 0 <= view.nMousePouse) {
		view.nMousePouse = -1;
		::SetCursor(NULL);
	}

	wchar_t szWord[2] = {wChar, 0};
	size_t nWord = 1;
	// テキストが選択されているか
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			Command_Indent(szWord, nWord);
			return;
		}else {
			view.DeleteData(true);
		}
	}else {
		if (!view.IsInsMode()) {
			DelCharForOverwrite(szWord, nWord);	// 上書き用の一文字削除
		}
	}

	Point ptLayoutNew;
	auto& caret = GetCaret();
	view.InsertData_CEditView(caret.GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true);

	// 挿入データの最後へカーソルを移動
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	view.PostprocessCommand_hokan();
}

// from ViewCommander_New.cpp
// Undo 元に戻す
void ViewCommander::Command_Undo(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = view.commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			view.SetUndoBuffer();
			if (!view.commander.GetOpeBlk() && 0 < nCount) {
				view.commander.SetOpeBlk(new OpeBlk());
				view.commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
	}

	if (!GetDocument().docEditor.IsEnableUndo()) {	// Undo(元に戻す)可能な状態か？
		return;
	}

	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_Undo()");

	Ope* pOpe = nullptr;

	OpeBlk*	pOpeBlk;
	size_t nOpeBlkNum;
	bool bIsModified;
//	int nNewLine;	// 挿入された部分の次の位置の行
//	int nNewPos;	// 挿入された部分の次の位置のデータ位置

	Point ptCaretPos_Before;
	Point ptCaretPos_After;

	// 各種モードの取り消し
	Command_Cancel_Mode();

	view.bDoing_UndoRedo = true;	// Undo, Redoの実行中か

	// 現在のUndo対象の操作ブロックを返す
	auto& caret = GetCaret();
	auto& docEditor = GetDocument().docEditor;
	if ((pOpeBlk = docEditor.opeBuf.DoUndo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = view.SetDrawSwitch(bDraw);	// hor


		WaitCursor waitCursor(view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().layoutMgr;
		for (int i=(int)nOpeBlkNum-1; i>=0; --i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
			}else {
				ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				// カーソルを移動
				caret.MoveCursor(ptCaretPos_After, false);
			}

			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// 選択範囲の変更
					Range selectLogic;
					selectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					selectLogic.SetTo(pOpe->ptCaretPos_PHY_After);
					if (bFastMode) {
					}else {
						selInfo.selectBgn.SetFrom(ptCaretPos_Before);
						selInfo.selectBgn.SetTo(selInfo.selectBgn.GetFrom());
						selInfo.select.SetFrom(ptCaretPos_Before);
						selInfo.select.SetTo(ptCaretPos_After);
					}

					// データ置換 削除&挿入にも使える
					bDrawAll |= view.ReplaceData_CEditView3(
						selInfo.select,				// 削除範囲
						&pInsertOpe->opeLineData,	// 削除されたデータのコピー(NULL可能)
						nullptr,
						bDraw,						// 再描画するか否か
						nullptr,
						pInsertOpe->nOrgSeq,
						nullptr,
						bFastMode,
						&selectLogic
					);

					// 選択範囲の変更
					selInfo.selectBgn.Clear(-1); // 範囲選択(原点)
					selInfo.select.Clear(-1);
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (0 < pDeleteOpe->opeLineData.size()) {
						// データ置換 削除&挿入にも使える
						Range range;
						range.Set(ptCaretPos_Before);
						Range cSelectLogic;
						cSelectLogic.Set(pOpe->ptCaretPos_PHY_Before);
						bDrawAll |= view.ReplaceData_CEditView3(
							range,
							nullptr,									// 削除されたデータのコピー(NULL可能)
							&pDeleteOpe->opeLineData,
							bDraw,										// 再描画するか否か
							nullptr,
							0,
							&pDeleteOpe->nOrgSeq,
							bFastMode,
							&cSelectLogic
						);
					}
					pDeleteOpe->opeLineData.clear();
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					Range range;
					range.SetFrom(ptCaretPos_Before);
					range.SetTo(ptCaretPos_After);
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pOpe->ptCaretPos_PHY_After);

					// データ置換 削除&挿入にも使える
					bDrawAll |= view.ReplaceData_CEditView3(
						range,				// 削除範囲
						&pReplaceOpe->pMemDataIns,	// 削除されたデータのコピー(NULL可能)
						&pReplaceOpe->pMemDataDel,	// 挿入するデータ
						bDraw,							// 再描画するか否か
						nullptr,
						pReplaceOpe->nOrgInsSeq,
						&pReplaceOpe->nOrgDelSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->pMemDataDel.clear();
				}
				break;
			case OpeCode::MoveCaret:
				// カーソルを移動
				if (bFastMode) {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
				}else {
					caret.MoveCursor(ptCaretPos_After, false);
				}
				break;
			}

			if (bFastMode) {
				if (i == 0) {
					layoutMgr._DoLayout();
					GetEditWindow().ClearViewCaretPosInfo();
					if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
					caret.MoveCursor(ptCaretPos_Before, true);
					// 通常モードではReplaceData_CEditViewの中で設定される
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_Before);
				}
			}else {
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				// カーソルを移動
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv((int)nOpeBlkNum - i, 100, (int)nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		view.SetDrawSwitch(bDrawSwitchOld);
		view.AdjustScrollBars();

		// Undo後の変更フラグ
		docEditor.SetModified(bIsModified, true);

		view.bDoing_UndoRedo = false;	// Undo, Redoの実行中か

		view.SetBracketPairPos(true);

		// 再描画
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平Scrollもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = view.GetRuler().GetRedrawFlag();
		view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0), false);
		if (!bRedrawRuler) {
			// ルーラーのキャレットのみを再描画
			HDC hdc = view.GetDC();
			view.GetRuler().DispRuler(hdc);
			view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// キャレットの行桁位置を表示する

		if (!GetEditWindow().UpdateTextWrap() && bDrawAll) {	// 折り返し方法関連の更新
			GetEditWindow().RedrawAllViews(&view);	// 他のペインの表示を更新
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
	view.bDoing_UndoRedo = false;	// Undo, Redoの実行中か

	return;
}


// from ViewCommander_New.cpp
// Redo やり直し
void ViewCommander::Command_Redo(void)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = view.commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			view.SetUndoBuffer();
			if (!view.commander.GetOpeBlk() && 0 < nCount) {
				view.commander.SetOpeBlk(new OpeBlk());
				view.commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
		// 注意：Opeを追加するとRedoはできなくなる
	}

	auto& docEditor = GetDocument().docEditor;
	if (!docEditor.IsEnableRedo()) {	// Redo(やり直し)可能な状態か？
		return;
	}
	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_Redo()");

	Ope*		pOpe = nullptr;
	OpeBlk*	pOpeBlk;
	size_t		nOpeBlkNum;
//	int			nNewLine;	// 挿入された部分の次の位置の行
//	int			nNewPos;	// 挿入された部分の次の位置のデータ位置
	bool		bIsModified;

	Point ptCaretPos_Before;
	Point ptCaretPos_To;
	Point ptCaretPos_After;

	// 各種モードの取り消し
	Command_Cancel_Mode();

	view.bDoing_UndoRedo = true;	// Undo, Redoの実行中か

	// 現在のRedo対象の操作ブロックを返す
	auto& caret = GetCaret();
	if ((pOpeBlk = docEditor.opeBuf.DoRedo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = view.SetDrawSwitch(bDraw);

		WaitCursor waitCursor(view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().layoutMgr;
		for (size_t i=0; i<nOpeBlkNum; ++i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				if (i == 0) {
					ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
					caret.MoveCursor(ptCaretPos_Before, true);
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_Before);
				}
			}else {
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					if (0 < pInsertOpe->opeLineData.size()) {
						// データ置換 削除&挿入にも使える
						Range range;
						range.Set(ptCaretPos_Before);
						Range cSelectLogic;
						cSelectLogic.Set(pOpe->ptCaretPos_PHY_Before);
						bDrawAll |= view.ReplaceData_CEditView3(
							range,
							nullptr,								// 削除されたデータのコピー(NULL可能)
							&pInsertOpe->opeLineData,				// 挿入するデータ
							bDraw,									// 再描画するか否か
							nullptr,
							0,
							&pInsertOpe->nOrgSeq,
							bFastMode,
							&cSelectLogic
						);

					}
					pInsertOpe->opeLineData.clear();
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (bFastMode) {
					}else {
						ptCaretPos_To = layoutMgr.LogicToLayout(pDeleteOpe->ptCaretPos_PHY_To);
					}
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pDeleteOpe->ptCaretPos_PHY_To);

					// データ置換 削除&挿入にも使える
					bDrawAll |= view.ReplaceData_CEditView3(
						Range(ptCaretPos_Before, ptCaretPos_To),
						&pDeleteOpe->opeLineData,	// 削除されたデータのコピー(NULL可能)
						nullptr,
						bDraw,
						nullptr,
						pDeleteOpe->nOrgSeq,
						nullptr,
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
						ptCaretPos_To = layoutMgr.LogicToLayout(pReplaceOpe->ptCaretPos_PHY_To);
					}
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pReplaceOpe->ptCaretPos_PHY_To);

					// データ置換 削除&挿入にも使える
					bDrawAll |= view.ReplaceData_CEditView3(
						Range(ptCaretPos_Before, ptCaretPos_To),
						&pReplaceOpe->pMemDataDel,	// 削除されたデータのコピー(NULL可能)
						&pReplaceOpe->pMemDataIns,	// 挿入するデータ
						bDraw,
						nullptr,
						pReplaceOpe->nOrgDelSeq,
						&pReplaceOpe->nOrgInsSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->pMemDataIns.clear();
				}
				break;
			case OpeCode::MoveCaret:
				break;
			}
			if (bFastMode) {
				if (i == nOpeBlkNum - 1) {
					layoutMgr._DoLayout();
					GetEditWindow().ClearViewCaretPosInfo();
					if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
					caret.MoveCursor(ptCaretPos_After, true);
					// 通常モードではReplaceData_CEditViewの中で設定される
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
				}
			}else {
				ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
				caret.MoveCursor(ptCaretPos_After, (i == nOpeBlkNum - 1));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv((int)i + 1, 100, (int)nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		view.SetDrawSwitch(bDrawSwitchOld);
		view.AdjustScrollBars();

		// Redo後の変更フラグ
		docEditor.SetModified(bIsModified, true);

		view.bDoing_UndoRedo = false;	// Undo, Redoの実行中か

		view.SetBracketPairPos(true);

		// 再描画
		// ルーラー再描画の必要があるときは DispRuler() ではなく他の部分と同時に Call_OnPaint() で描画する
		// ・DispRuler() はルーラーとテキストの隙間（左側は行番号の幅に合わせた帯）を描画してくれない
		// ・行番号表示に必要な幅は OPE_INSERT/OPE_DELETE 処理内で更新されており変更があればルーラー再描画フラグに反映されている
		// ・水平Scrollもルーラー再描画フラグに反映されている
		const bool bRedrawRuler = view.GetRuler().GetRedrawFlag();
		view.Call_OnPaint(
			(int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0),
			false
		);
		if (!bRedrawRuler) {
			// ルーラーのキャレットのみを再描画
			HDC hdc = view.GetDC();
			view.GetRuler().DispRuler(hdc);
			view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// キャレットの行桁位置を表示する

		if (!GetEditWindow().UpdateTextWrap())	// 折り返し方法関連の更新
			GetEditWindow().RedrawAllViews(&view);	// 他のペインの表示を更新

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	view.bDoing_UndoRedo = false;	// Undo, Redoの実行中か

	return;
}


// カーソル位置または選択エリアを削除
void ViewCommander::Command_Delete(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {		// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!selInfo.IsTextSelected()) {	// テキストが選択されているか
		auto& layoutMgr = GetDocument().layoutMgr;
		// 選択範囲なしでDELETEを実行した場合、カーソル位置まで半角スペースを挿入した後改行を削除して次行と連結する
		auto& caret = GetCaret();
		if ((int)layoutMgr.GetLineCount() > caret.GetCaretLayoutPos().y) {
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
			if (pLayout) {
				size_t nLineLen;
				size_t nIndex = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
				if (nLineLen != 0) {	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					if (pLayout->GetLayoutEol().GetType() != EolType::None) {	// 行終端は改行コードか?
						Command_InsText(true, L"", 0, false);	// カーソル位置まで半角スペース挿入
					}else {	// 行終端が折り返し
						// 折り返し行末ではスペース挿入後、次の文字を削除する

						// フリーカーソル時の折り返し越え位置での削除はどうするのが妥当かよくわからないが
						// 非フリーカーソル時（ちょうどカーソルが折り返し位置にある）には次の行の先頭文字を削除したい

						if ((int)nLineLen < caret.GetCaretLayoutPos().x) {	// 折り返し行末とカーソルの間に隙間がある
							Command_InsText(true, L"", 0, false);	// カーソル位置まで半角スペース挿入
							pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
							nIndex = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
						}
						if (nLineLen != 0) {	// （スペース挿入後も）折り返し行末なら次文字を削除するために次行の先頭に移動する必要がある
							if (pLayout->GetNextLayout()) {	// 最終行末ではない
								Point ptLog(pLayout->GetLogicOffset() + (int)nIndex, pLayout->GetLogicLineNo());
								Point ptLay = layoutMgr.LogicToLayout(ptLog);
								caret.MoveCursor(ptLay, true);
								caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
							}
						}
					}
				}
			}
		}
	}
	view.DeleteData(true);
	return;
}


// カーソル前を削除
void ViewCommander::Command_Delete_Back(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {				// テキストが選択されているか
		view.DeleteData(true);
	}else {
		auto& caret = GetCaret();
		Point	ptLayoutPos_Old = caret.GetCaretLayoutPos();
		Point ptLogicPos_Old = caret.GetCaretLogicPos();
		BOOL bBool = Command_Left(false, false);
		if (bBool) {
			const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
			if (pLayout) {
				size_t nLineLen;
				size_t nIdx = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
				if (nLineLen == 0) {	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					// 右からの移動では折り返し末尾文字は削除するが改行は削除しない
					// 下から（下の行の行頭から）の移動では改行も削除する
					if (nIdx < pLayout->GetLengthWithoutEOL() || caret.GetCaretLayoutPos().y < ptLayoutPos_Old.y) {
						if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
							// 操作の追加
							GetOpeBlk()->AppendOpe(
								new MoveCaretOpe(
									ptLogicPos_Old,
									caret.GetCaretLogicPos()
								)
							);
						}
						view.DeleteData(true);
					}
				}
			}
		}
	}
	view.PostprocessCommand_hokan();
}

// 	上書き用の一文字削除
void ViewCommander::DelCharForOverwrite(
	const wchar_t* pszInput,
	size_t nLen
	)
{
	bool bEol = false;
	bool bDelete = true;
	auto& caret = GetCaret();
	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	int nDelLen = 0;
	int nKetaDiff = 0;
	int nKetaAfterIns = 0;
	if (pLayout) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		size_t nIdxTo = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
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
				size_t nPos = caret.GetCaretLogicPos().GetX();
				if (line.At(nPos) != WCODE::TAB) {
					size_t nKetaBefore = NativeW::GetKetaOfChar(line, nPos);
					size_t nKetaAfter = NativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = (int)nKetaBefore - (int)nKetaAfter;
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
							size_t nKetaBefore2 = NativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = (int)nKetaBefore + (int)nKetaBefore2 - (int)nKetaAfter;
						}
					}
				}
			}
		}
	}
	if (bDelete) {
		// 上書きモードなので、現在位置の文字を１文字消去
		Point posBefore;
		if (bEol) {
			Command_Delete();	// 行数減では再描画が必要＆行末以後の削除を処理統一
			posBefore = caret.GetCaretLayoutPos();
		}else {
			// 1文字削除
			view.DeleteData(false);
			posBefore = caret.GetCaretLayoutPos();
			for (int i=1; i<nDelLen; ++i) {
				view.DeleteData(false);
			}
		}
		NativeW tmp;
		for (int i=0; i<nKetaDiff; ++i) {
			tmp.AppendStringLiteral(L" ");
		}
		for (int i=0; i<nKetaAfterIns; ++i) {
			tmp.AppendStringLiteral(L" ");
		}
		if (0 < tmp.GetStringLength()) {
			Command_InsText(false, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			caret.MoveCursor(posBefore, false);
		}
	}
}

