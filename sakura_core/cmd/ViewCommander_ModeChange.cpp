#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

// ViewCommanderクラスのコマンド(モード切り替え系)関数群

/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void ViewCommander::Command_ChgMod_Ins(void)
{
	// 挿入モードか？
	view.SetInsMode(!view.IsInsMode());
	// キャレットの表示・更新
	GetCaret().ShowEditCaret();
	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();
}


// from ViewCommander_New.cpp
/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void ViewCommander::Command_ChgMod_EOL(EolType e)
{
	if (EolType::None < e && e < EolType::CodeMax) {
		GetDocument().docEditor.SetNewLineCode(e);
		// ステータスバーを更新するため
		// キャレットの行桁位置を表示する関数を呼び出す
		GetCaret().ShowCaretPosInfo();
	}
}


// 文字コードセット指定
void ViewCommander::Command_Chg_Charset(
	EncodingType	eCharSet,	// [in] 設定する文字コードセット
	bool		bBom		// [in] 設定するBOM(Unicode系以外は無視)
	)
{
	if (eCharSet == CODE_NONE || eCharSet ==  CODE_AUTODETECT) {
		// 文字コードが指定されていないならば
		// 文字コードの確認
		eCharSet = GetDocument().GetDocumentEncoding();	// 設定する文字コードセット
		bBom     = GetDocument().GetDocumentBomExist();	// 設定するBOM
		INT_PTR nRet = GetEditWindow().dlgSetCharSet.DoModal(G_AppInstance(), view.GetHwnd(), 
						&eCharSet, &bBom);
		if (!nRet) {
			return;
		}
	}

	// 文字コードの設定
	GetDocument().docFile.SetCodeSetChg(eCharSet, CodeTypeName(eCharSet).UseBom() & bBom);

	// ステータス表示
	GetCaret().ShowCaretPosInfo();
}


/** 各種モードの取り消し
	@param whereCursorIs 選択をキャンセルした後、キャレットをどこに置くか。0=動かさない。1=左上。2=右下。
*/
void ViewCommander::Command_Cancel_Mode(int whereCursorIs)
{
	bool bBoxSelect = false;
	auto& selInfo = view.GetSelectionInfo();
	auto& caret = GetCaret();
	if (selInfo.IsTextSelected()) {
		// 選択解除後のカーソル位置を決める。
		Point ptTo;
		Range rcMoveTo = GetSelect();
		if (selInfo.IsBoxSelecting()) { // 矩形選択ではキャレットが改行の後ろに取り残されないように、左上。
			bBoxSelect = true;
			// 2点を対角とする矩形を求める
			Range rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// 範囲選択開始
				GetSelect().GetTo()		// 範囲選択終了
			);
			// 2013.04.22 Moca 左上固定はやめる
			rcMoveTo = rcSel;
		}
		if (whereCursorIs == 1) { // 左上
			ptTo = rcMoveTo.GetFrom();
		}else if (whereCursorIs == 2) { // 右下
			ptTo = rcMoveTo.GetTo();
		}else {
			ptTo = caret.GetCaretLayoutPos();
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(true);

		// カーソルを移動
		auto& layoutMgr = GetDocument().layoutMgr;
		if (ptTo.y >= (int)layoutMgr.GetLineCount()) {
			// ファイルの最後に移動
			Command_GoFileEnd(false);
		}else {
			if (!GetDllShareData().common.general.bIsFreeCursorMode && bBoxSelect) {
				// 2013.04.22 Moca 矩形選択のとき左上固定をやめたので代わりにEOLより右だった場合にEOLに補正する
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
				if (pLayout) {
					ptTo.x = t_min((int)ptTo.x, (int)pLayout->CalcLayoutWidth(layoutMgr));
				}
			}

			caret.MoveCursor(ptTo, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
	}else {
		// 2011.12.05 Moca 選択中の未選択状態でもLockの解除と描画が必要
		if (selInfo.IsTextSelecting()
			|| selInfo.IsBoxSelecting()
		) {
			selInfo.DisableSelectArea(true);
			caret.underLine.CaretUnderLineON(true, false);
			selInfo.PrintSelectionInfoMsg();
		}
	}
}

