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
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "outline/FuncInfoArr.h"
#include "plugin/JackManager.h"
#include "plugin/OutlineIfObj.h"
#include "sakura_rc.h"


/*!	アウトライン解析
	
	2002/3/13 YAZAKI nOutlineTypeとnListTypeを統合。
*/
// トグル用のフラグに変更 20060201 aroka
bool ViewCommander::Command_FUNCLIST(
	ShowDialogType nAction,
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

	static FuncInfoArr funcInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring titleOverride;				// プラグインによるダイアログタイトル上書き

	// 2001.12.03 hor & 2002.3.13 YAZAKI
	if (nOutlineType == OUTLINE_DEFAULT) {
		// タイプ別に設定されたアウトライン解析方法
		nOutlineType = m_pCommanderView->m_pTypeData->eDefaultOutline;
		if (nOutlineType == OUTLINE_CPP) {
			if (CheckEXT(GetDocument()->m_docFile.GetFilePath(), _T("c"))) {
				nOutlineType = OUTLINE_C;	// これでC関数一覧リストビューになる
			}
		}
	}

	auto& dlgFuncList = GetEditWindow()->m_dlgFuncList;
	if (dlgFuncList.GetHwnd() && nAction != ShowDialogType::Reload) {
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
	funcInfoArr.Empty();
	int nListType = nOutlineType;			// 2011.06.25 syat

	auto& docOutline = GetDocument()->m_docOutline;
	switch (nOutlineType) {
	case OUTLINE_C:			// C/C++ は MakeFuncList_C
	case OUTLINE_CPP:		docOutline.MakeFuncList_C(&funcInfoArr);break;
	case OUTLINE_PLSQL:		docOutline.MakeFuncList_PLSQL(&funcInfoArr);break;
	case OUTLINE_JAVA:		docOutline.MakeFuncList_Java(&funcInfoArr);break;
	case OUTLINE_COBOL:		docOutline.MakeTopicList_cobol(&funcInfoArr);break;
	case OUTLINE_ASM:		docOutline.MakeTopicList_asm(&funcInfoArr);break;
	case OUTLINE_PERL:		docOutline.MakeFuncList_Perl(&funcInfoArr);break;	// Sep. 8, 2000 genta
	case OUTLINE_VB:		docOutline.MakeFuncList_VisualBasic(&funcInfoArr);break;	// June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		docOutline.MakeTopicList_wztxt(&funcInfoArr);break;		// 2003.05.20 zenryaku 階層付テキスト アウトライン解析
	case OUTLINE_HTML:		docOutline.MakeTopicList_html(&funcInfoArr);break;		// 2003.05.20 zenryaku HTML アウトライン解析
	case OUTLINE_TEX:		docOutline.MakeTopicList_tex(&funcInfoArr);break;		// 2003.07.20 naoh TeX アウトライン解析
	case OUTLINE_BOOKMARK:	docOutline.MakeFuncList_BookMark(&funcInfoArr);break;	// 2001.12.03 hor
	case OUTLINE_FILE:		docOutline.MakeFuncList_RuleFile(&funcInfoArr, titleOverride);break;	// 2002.04.01 YAZAKI アウトライン解析にルールファイルを導入
//	case OUTLINE_UNKNOWN:	// Jul. 08, 2001 JEPRO 使わないように変更
	case OUTLINE_PYTHON:	docOutline.MakeFuncList_python(&funcInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	docOutline.MakeFuncList_Erlang(&funcInfoArr);break;		// 2009.08.10 genta
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
				assert_warning(plugs.size() == 1);
				// インタフェースオブジェクト準備
				WSHIfObj::List params;
				OutlineIfObj* objOutline = new OutlineIfObj(funcInfoArr);
				objOutline->AddRef();
				params.push_back(objOutline);
				// プラグイン呼び出し
				(*plugs.begin())->Invoke(m_pCommanderView, params);

				nListType = objOutline->m_nListType;			// ダイアログの表示方法をを上書き
				titleOverride = objOutline->m_sOutlineTitle;	// ダイアログタイトルを上書き

				objOutline->Release();
				break;
			}
		}

		// それ以外
		docOutline.MakeTopicList_txt(&funcInfoArr);
		break;
	}

	// 解析対象ファイル名
	_tcscpy(funcInfoArr.m_szFilePath, GetDocument()->m_docFile.GetFilePath());

	// アウトライン ダイアログの表示
	LayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			(LPARAM)m_pCommanderView,
			&funcInfoArr,
			poCaret.GetY2() + LayoutInt(1),
			poCaret.GetX2() + LayoutInt(1),
			nOutlineType,
			nListType,
			m_pCommanderView->m_pTypeData->bLineNumIsCRLF	// 行番号の表示 false=折り返し単位／true=改行単位
		);
	}else {
		// アクティブにする
		dlgFuncList.Redraw(nOutlineType, nListType, &funcInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1);
		if (bForeground) {
			::SetFocus(dlgFuncList.GetHwnd());
		}
	}

	// ダイアログタイトルを上書き
	if (!titleOverride.empty()) {
		dlgFuncList.SetWindowText(titleOverride.c_str());
	}

	bIsProcessing = false;
	return true;
}

