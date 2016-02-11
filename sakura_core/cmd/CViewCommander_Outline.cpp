/*!	@file
@brief ViewCommanderクラスのコマンド(検索系 アウトライン解析)関数群

	2012/12/17	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, zenryaku
	Copyright (C) 2006, aroka
	Copyright (C) 2007, genta, kobake
	Copyright (C) 2009, genta
	Copyright (C) 2011, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "outline/CFuncInfoArr.h"
#include "plugin/CJackManager.h"
#include "plugin/COutlineIfObj.h"
#include "sakura_rc.h"


/*!	アウトライン解析
	
	2002/3/13 YAZAKI nOutlineTypeとnListTypeを統合。
*/
// トグル用のフラグに変更 20060201 aroka
bool ViewCommander::Command_FUNCLIST(
	int nAction,
	int _nOutlineType = OUTLINE_DEFAULT
	)
{
	static bool bIsProcessing = false;	// アウトライン解析処理中フラグ

	// アウトラインプラグイン内でのEditor.Outline呼び出しによる再入を禁止する
	if (bIsProcessing) return false;

	bIsProcessing = true;

	// 自プロセスが前面にいるかどうか調べる
	DWORD dwPid2;
	DWORD dwPid1 = ::GetCurrentProcessId();
	::GetWindowThreadProcessId(::GetForegroundWindow(), &dwPid2);
	bool bForeground = (dwPid1 == dwPid2);

	EOutlineType nOutlineType = (EOutlineType)_nOutlineType; // 2007.11.29 kobake

//	if (bCheckOnly) {
//		return TRUE;
//	}

	static FuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring sTitleOverride;				// プラグインによるダイアログタイトル上書き

	// 2001.12.03 hor & 2002.3.13 YAZAKI
	if (nOutlineType == OUTLINE_DEFAULT) {
		// タイプ別に設定されたアウトライン解析方法
		nOutlineType = m_pCommanderView->m_pTypeData->m_eDefaultOutline;
		if (nOutlineType == OUTLINE_CPP) {
			if (CheckEXT(GetDocument()->m_docFile.GetFilePath(), _T("c"))) {
				nOutlineType = OUTLINE_C;	// これでC関数一覧リストビューになる
			}
		}
	}

	auto& dlgFuncList = GetEditWindow()->m_dlgFuncList;
	if (dlgFuncList.GetHwnd() && nAction != (int)ShowDialogType::Reload) {
		switch (nAction) {
		case ShowDialogType::Normal: // アクティブにする
			// 開いているものと種別が同じならActiveにするだけ．異なれば再解析
			dlgFuncList.SyncColor();
			if (dlgFuncList.CheckListType(nOutlineType)) {
				if (bForeground) {
					::SetFocus(dlgFuncList.GetHwnd());
				}
				bIsProcessing = false;
				return true;
			}
			break;
		case ShowDialogType::Toggle: // 閉じる
			// 開いているものと種別が同じなら閉じる．異なれば再解析
			if (dlgFuncList.CheckListType(nOutlineType)) {
				if (dlgFuncList.IsDocking())
					::DestroyWindow(dlgFuncList.GetHwnd());
				else
					::SendMessage(dlgFuncList.GetHwnd(), WM_CLOSE, 0, 0);
				bIsProcessing = false;
				return true;
			}
			break;
		default:
			break;
		}
	}

	// 解析結果データを空にする
	cFuncInfoArr.Empty();
	int nListType = nOutlineType;			// 2011.06.25 syat

	auto& cDocOutline = GetDocument()->m_docOutline;
	switch (nOutlineType) {
	case OUTLINE_C:			// C/C++ は MakeFuncList_C
	case OUTLINE_CPP:		cDocOutline.MakeFuncList_C(&cFuncInfoArr);break;
	case OUTLINE_PLSQL:		cDocOutline.MakeFuncList_PLSQL(&cFuncInfoArr);break;
	case OUTLINE_JAVA:		cDocOutline.MakeFuncList_Java(&cFuncInfoArr);break;
	case OUTLINE_COBOL:		cDocOutline.MakeTopicList_cobol(&cFuncInfoArr);break;
	case OUTLINE_ASM:		cDocOutline.MakeTopicList_asm(&cFuncInfoArr);break;
	case OUTLINE_PERL:		cDocOutline.MakeFuncList_Perl(&cFuncInfoArr);break;	// Sep. 8, 2000 genta
	case OUTLINE_VB:		cDocOutline.MakeFuncList_VisualBasic(&cFuncInfoArr);break;	// June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		cDocOutline.MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku 階層付テキスト アウトライン解析
	case OUTLINE_HTML:		cDocOutline.MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML アウトライン解析
	case OUTLINE_TEX:		cDocOutline.MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX アウトライン解析
	case OUTLINE_BOOKMARK:	cDocOutline.MakeFuncList_BookMark(&cFuncInfoArr);break;	// 2001.12.03 hor
	case OUTLINE_FILE:		cDocOutline.MakeFuncList_RuleFile(&cFuncInfoArr, sTitleOverride);break;	// 2002.04.01 YAZAKI アウトライン解析にルールファイルを導入
//	case OUTLINE_UNKNOWN:	// Jul. 08, 2001 JEPRO 使わないように変更
	case OUTLINE_PYTHON:	cDocOutline.MakeFuncList_python(&cFuncInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	cDocOutline.MakeFuncList_Erlang(&cFuncInfoArr);break;		// 2009.08.10 genta
	case OUTLINE_FILETREE:	/* 特に何もしない*/ ;break;	// 2013.12.08 Moca
	case OUTLINE_TEXT:
		// fall though
		// ここには何も入れてはいけない 2007.02.28 genta 注意書き
	default:
		// プラグインから検索する
		{
			Plug::Array plugs;
			JackManager::getInstance()->GetUsablePlug(PP_OUTLINE, nOutlineType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(1 == plugs.size());
				// インタフェースオブジェクト準備
				WSHIfObj::List params;
				OutlineIfObj* objOutline = new OutlineIfObj(cFuncInfoArr);
				objOutline->AddRef();
				params.push_back(objOutline);
				// プラグイン呼び出し
				(*plugs.begin())->Invoke(m_pCommanderView, params);

				nListType = objOutline->m_nListType;			// ダイアログの表示方法をを上書き
				sTitleOverride = objOutline->m_sOutlineTitle;	// ダイアログタイトルを上書き

				objOutline->Release();
				break;
			}
		}

		// それ以外
		cDocOutline.MakeTopicList_txt(&cFuncInfoArr);
		break;
	}

	// 解析対象ファイル名
	_tcscpy(cFuncInfoArr.m_szFilePath, GetDocument()->m_docFile.GetFilePath());

	// アウトライン ダイアログの表示
	LayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			(LPARAM)m_pCommanderView,
			&cFuncInfoArr,
			poCaret.GetY2() + LayoutInt(1),
			poCaret.GetX2() + LayoutInt(1),
			nOutlineType,
			nListType,
			m_pCommanderView->m_pTypeData->m_bLineNumIsCRLF	// 行番号の表示 false=折り返し単位／true=改行単位
		);
	}else {
		// アクティブにする
		dlgFuncList.Redraw(nOutlineType, nListType, &cFuncInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1);
		if (bForeground) {
			::SetFocus(dlgFuncList.GetHwnd());
		}
	}

	// ダイアログタイトルを上書き
	if (!sTitleOverride.empty()) {
		dlgFuncList.SetWindowText(sTitleOverride.c_str());
	}

	bIsProcessing = false;
	return true;
}

