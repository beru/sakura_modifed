/*!	@file
	@brief EditViewクラスの補完関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
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
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/CDocReader.h"
#include "charset/charcode.h"  // 2006.06.28 rastiv
#include "window/CEditWnd.h"
#include "parse/CWordParse.h"
#include "sakura_rc.h"

/*!
	@brief コマンド受信前補完処理
	
	補完ウィンドウの非表示

	@date 2005.01.10 genta 関数化
*/
void EditView::PreprocessCommand_hokan(int nCommand)
{
	// 補完ウィンドウが表示されているとき、特別な場合を除いてウィンドウを非表示にする
	if (m_bHokan) {
		if (1
			&& nCommand != F_HOKAN		//	補完開始・終了コマンド
			&& nCommand != F_WCHAR		//	文字入力
			&& nCommand != F_IME_CHAR	//	漢字入力
		) {
			m_pEditWnd->m_hokanMgr.Hide();
			m_bHokan = FALSE;
		}
	}
}

/*!
	コマンド実行後補完処理

	@author Moca
	@date 2005.01.10 genta 関数化
*/
void EditView::PostprocessCommand_hokan(void)
{
	if (GetDllShareData().m_common.m_helper.m_bUseHokan && !m_bExecutingKeyMacro) { // キーボードマクロの実行中
		NativeW memData;

		// カーソル直前の単語を取得
		if (0 < GetParser().GetLeftWord(&memData, 100)) {
			ShowHokanMgr(memData, false);
		}else {
			if (m_bHokan) {
				m_pEditWnd->m_hokanMgr.Hide();
				m_bHokan = FALSE;
			}
		}
	}
}

/*!	補完ウィンドウを表示する
	ウィンドウを表示した後は、HokanMgrに任せるので、ShowHokanMgrの知るところではない。
	
	@param memData [in] 補完する元のテキスト 「Ab」などがくる。
	@param bAutoDecided [in] 候補が1つだったら確定する

	@date 2005.01.10 genta CEditView_Commandから移動
*/
void EditView::ShowHokanMgr(NativeW& memData, bool bAutoDecided)
{
	// 補完対象ワードリストを調べる
	NativeW	memHokanWord;
	POINT		poWin;
	/* 補完ウィンドウの表示位置を算出 */
	LayoutXInt nX = GetCaret().GetCaretLayoutPos().GetX2() - GetTextArea().GetViewLeftCol();
	if (nX < 0) {
		poWin.x = 0;
	}else if (GetTextArea().m_nViewColNum < nX) {
		poWin.x = GetTextArea().GetAreaRight();
	}else {
		poWin.x = GetTextArea().GetAreaLeft() + (Int)(nX) * GetTextMetrics().GetHankakuDx();
	}
	LayoutYInt nY = GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine();
	if (nY < 0) {
		poWin.y = 0;
	}else if (GetTextArea().m_nViewRowNum < nY) {
		poWin.y = GetTextArea().GetAreaBottom();
	}else {
		poWin.y = GetTextArea().GetAreaTop() + (Int)(nY) * GetTextMetrics().GetHankakuDy();
	}
	this->ClientToScreen(&poWin);
	poWin.x -= (
		memData.GetStringLength()
		 * GetTextMetrics().GetHankakuDx()
	);

	/*	補完ウィンドウを表示
		ただし、bAutoDecided == trueの場合は、補完候補が1つのときは、ウィンドウを表示しない。
		詳しくは、Search()の説明を参照のこと。
	*/
	NativeW* pMemHokanWord;
	if (bAutoDecided) {
		pMemHokanWord = &memHokanWord;
	}else {
		pMemHokanWord = NULL;
	}

	// 入力補完ウィンドウ作成
	// 以前はエディタ起動時に作成していたが必要になってからここで作成するようにした。
	// エディタ起動時だとエディタ可視化の途中になぜか不可視の入力補完ウィンドウが一時的にフォアグラウンドになって、
	// タブバーに新規タブが追加されるときのタブ切替でタイトルバーがちらつく（一瞬非アクティブ表示になるのがはっきり見える）ことがあった。
	// ※ Vista/7 の特定の PC でだけのちらつきか？ 該当 PC 以外の Vista/7 PC でもたまに微妙に表示が乱れた感じになる程度の症状が見られたが、それらが同一原因かどうかは不明。
	auto& hokanMgr = m_pEditWnd->m_hokanMgr;
	if (!hokanMgr.GetHwnd()) {
		hokanMgr.DoModeless(
			G_AppInstance(),
			GetHwnd(),
			(LPARAM)this
		);
		::SetFocus(GetHwnd());	// エディタにフォーカスを戻す
	}
	int nKouhoNum = hokanMgr.HokanMgr::Search(
		&poWin,
		GetTextMetrics().GetHankakuHeight(),
		GetTextMetrics().GetHankakuDx(),
		memData.GetStringPtr(),
		m_pTypeData->m_szHokanFile,
		m_pTypeData->m_bHokanLoHiCase,
		m_pTypeData->m_bUseHokanByFile, // 2003.06.22 Moca
		m_pTypeData->m_nHokanType,
		m_pTypeData->m_bUseHokanByKeyword,
		pMemHokanWord
	);
	// 補完候補の数によって動作を変える
	if (nKouhoNum <= 0) {				//	候補無し
		if (m_bHokan) {
			hokanMgr.Hide();
			m_bHokan = FALSE;
			// 2003.06.25 Moca 失敗してたら、ビープ音を出して補完終了。
			ErrorBeep();
		}
	}else if (bAutoDecided && nKouhoNum == 1) { //	候補1つのみ→確定。
		if (m_bHokan) {
			hokanMgr.Hide();
			m_bHokan = FALSE;
		}
		// 2004.05.14 Moca HokanMgr::Search側で改行を削除するようにし、直接書き換えるのをやめた

		GetCommander().Command_WordDeleteToStart();
		GetCommander().Command_INSTEXT(true, memHokanWord.GetStringPtr(), memHokanWord.GetStringLength(), TRUE);
	}else {
		m_bHokan = TRUE;
	}
	
	//	補完終了。
	if (!m_bHokan) {
		GetDllShareData().m_common.m_helper.m_bUseHokan = FALSE;	//	入力補完終了の知らせ
	}
}


/*!
	編集中データから入力補完キーワードの検索
	HokanMgrから呼ばれる

	@return 候補数

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta  CEditView_Commandから移動
	@date 2007/10/17 kobake 読みやすいようにネストを浅くしました。
	@date 2008.07.25 nasukoji 大文字小文字を同一視の場合でも候補の振るい落としは完全一致で見る
	@date 2008.10.11 syat 日本語の補完
	@date 2010.06.16 Moca ひらがなで続行する場合、直前を漢字に制限
*/
int EditView::HokanSearchByFile(
	const wchar_t*	pszKey,				// [in]
	bool			bHokanLoHiCase,		// [in] 英大文字小文字を同一視する
	vector_ex<std::wstring>& 	vKouho,	// [in,out] 候補
	int				nMaxKouho			// [in] Max候補数(0 == 無制限)
) {
	const int nKeyLen = wcslen(pszKey);
	int nLines = m_pEditDoc->m_docLineMgr.GetLineCount();
	int j, nWordLen, nLineLen, nRet, nCharSize, nWordBegin, nWordLenStop;

	LogicPoint ptCur = GetCaret().GetCaretLogicPos(); // 物理カーソル位置

	// キーが記号で始まるか
	// キーの先頭が記号(#$@\)かどうか判定
	bool bKeyStartWithMark = wcschr(L"$@#\\", pszKey[0]) != NULL;

	for (LogicInt i=LogicInt(0); i<nLines; ++i) {
		const wchar_t* pszLine = DocReader(m_pEditDoc->m_docLineMgr).GetLineStrWithoutEOL(i, &nLineLen);

		for (j=0; j<nLineLen; j+=nCharSize) {
			nCharSize = NativeW::GetSizeOfChar(pszLine, nLineLen, j);

			// 半角記号は候補に含めない
			if (pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR(pszLine[j])) continue;

			// キーの先頭が記号以外の場合、記号で始まる単語は候補からはずす
			if (!bKeyStartWithMark && wcschr(L"$@#\\", pszLine[j])) continue;

			// 文字種類取得
			ECharKind kindPre = WordParse::WhatKindOfChar(pszLine, nLineLen, j);	// 文字種類取得

			// 全角記号は候補に含めない
			if (0
				|| kindPre == CK_ZEN_SPACE
				|| kindPre == CK_ZEN_NOBASU
				|| kindPre == CK_ZEN_DAKU
				|| kindPre == CK_ZEN_KIGO
				|| kindPre == CK_ZEN_SKIGO
			)
				continue;

			// 候補が記号で始まるか
			bool bWordStartWithMark = wcschr(L"$@#\\", pszLine[j]) != NULL;

			nWordBegin = j;
			// 候補単語の終了位置を求める
			nWordLen = nCharSize;
			nWordLenStop = -1; // 送り仮名無視用単語の終わり。-1は無効
			for (j+=nCharSize; j<nLineLen; j+=nCharSize) {
				nCharSize = NativeW::GetSizeOfChar(pszLine, nLineLen, j);

				// 半角記号は含めない
				if (pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR(pszLine[j])) break;

				// 文字種類取得
				ECharKind kindCur = WordParse::WhatKindOfChar(pszLine, nLineLen, j);
				// 全角記号は候補に含めない
				if (kindCur == CK_ZEN_SPACE || kindCur == CK_ZEN_KIGO || kindCur == CK_ZEN_SKIGO) {
					break;
				}

				// 文字種類が変わったら単語の切れ目とする
				ECharKind kindMerge = WordParse::WhatKindOfTwoChars(kindPre, kindCur);
				if (kindMerge == CK_NULL) {	// kindPreとkindCurが別種
					if (kindCur == CK_HIRA) {
						kindMerge = kindCur;		// ひらがななら続行
						// 2010.06.16 Moca 漢字のみ送り仮名を候補に含める
						if (kindPre != CK_ZEN_ETC) {
							nWordLenStop = nWordLen;
						}
					}else if (bKeyStartWithMark && bWordStartWithMark && kindPre == CK_UDEF) {
						kindMerge = kindCur;		// 記号で始まる単語は制限を緩める
					}else {
						j -= nCharSize;
						break;						// それ以外は単語の切れ目
					}
				}

				kindPre = kindMerge;
				nWordLen += nCharSize;				// 次の文字へ
			}

			if (0 < nWordLenStop) {
				nWordLen  = nWordLenStop;
			}


			// CDicMgr等の制限により長すぎる単語は無視する
			if (nWordLen > 1020) {
				continue;
			}
			if (nKeyLen > nWordLen) continue;

			// 候補単語の開始位置を求める
			const wchar_t* word = pszLine + nWordBegin;

			// キーと比較する
			if (bHokanLoHiCase) {
				nRet = auto_memicmp(pszKey, word, nKeyLen);
			}else {
				nRet = auto_memcmp(pszKey, word, nKeyLen);
			}
			if (nRet != 0) continue;

			// カーソル位置の単語は候補からはずす
			if (ptCur.y == i && nWordBegin <= ptCur.x && ptCur.x <= nWordBegin + nWordLen) {	// 2010.02.20 syat 修正// 2008.11.09 syat 修正
				continue;
			}

			// 候補を追加(重複は除く)
			{
				std::wstring strWord = std::wstring(word, nWordLen);
				HokanMgr::AddKouhoUnique(vKouho, strWord);
			}
			if (nMaxKouho != 0 && nMaxKouho <= (int)vKouho.size()) {
				return vKouho.size();
			}
		}
	}
	return vKouho.size();
}

