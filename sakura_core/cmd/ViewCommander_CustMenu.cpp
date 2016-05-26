/*!	@file
@brief ViewCommanderクラスのコマンド(カスタムメニュー)関数群

	2012/12/20	ViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include <vector>

// 右クリックメニュー
void ViewCommander::Command_Menu_RButton(void)
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	// ポップアップメニュー(右クリック)
	int nId = view.CreatePopUpMenu_R();
	if (nId == 0) {
		return;
	}
	switch (nId) {
	case IDM_COPYDICINFO:
		{
			size_t nLength;
			const TCHAR* pszStr = view.tipWnd.info.GetStringPtr(&nLength);
			std::vector<TCHAR> szWork(nLength + 1);
			TCHAR* pszWork = &szWork[0];
			auto_memcpy(pszWork, pszStr, nLength);
			pszWork[nLength] = _T('\0');

			// 見た目と同じように、\n を CR+LFへ変換する
			for (size_t i=0; i<nLength; ++i) {
				if (pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')) {
					pszWork[i] = WCODE::CR;
					pszWork[i+1] = WCODE::LF;
				}
			}
			// クリップボードにデータを設定
			view.MySetClipboardData(pszWork, nLength, false);
		}
		break;
	case IDM_JUMPDICT:
		// キーワード辞書ファイルを開く
		if (view.pTypeData->bUseKeywordHelp) {		// キーワード辞書セレクトを使用する	// 2006.04.10 fon
			// Feb. 17, 2007 genta 相対パスを実行ファイル基準で開くように
			view.TagJumpSub(
				view.pTypeData->keyHelpArr[view.tipWnd.nSearchDict].szPath,
				Point(1, view.tipWnd.nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		// コマンドコードによる処理振り分け
//		HandleCommand(nId, true, 0, 0, 0, 0);
		::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nId, 0),  (LPARAM)NULL);
		break;
	}
	return;
}


// カスタムメニュー表示
int ViewCommander::Command_CustMenu(int nMenuIdx)
{
	GetEditWindow().GetMenuDrawer().ResetContents();

	// Oct. 3, 2001 genta
	FuncLookup& FuncLookup = GetDocument().funcLookup;

	if (nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx) {
		return 0;
	}
	if (GetDllShareData().common.customMenu.nCustMenuItemNumArr[nMenuIdx] == 0) {
		return 0;
	}
	HMENU hMenu = ::CreatePopupMenu();
	return view.CreatePopUpMenuSub(hMenu, nMenuIdx, nullptr);
}

