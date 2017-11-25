#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "outline/FuncInfoArr.h"
#include "plugin/JackManager.h"
#include "plugin/OutlineIfObj.h"
#include "sakura_rc.h"

// ViewCommanderクラスのコマンド(検索系 アウトライン解析)関数群

/*!	アウトライン解析 */
bool ViewCommander::Command_FuncList(
	ShowDialogType nAction,
	OutlineType _nOutlineType = OutlineType::Default
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

	OutlineType nOutlineType = _nOutlineType;

//	if (bCheckOnly) {
//		return TRUE;
//	}

	static FuncInfoArr funcInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring titleOverride;				// プラグインによるダイアログタイトル上書き

	if (nOutlineType == OutlineType::Default) {
		// タイプ別に設定されたアウトライン解析方法
		nOutlineType = view.pTypeData->eDefaultOutline;
		if (nOutlineType == OutlineType::CPP) {
			if (CheckEXT(GetDocument().docFile.GetFilePath(), _T("c"))) {
				nOutlineType = OutlineType::C;	// これでC関数一覧リストビューになる
			}
		}
	}

	auto& dlgFuncList = GetEditWindow().dlgFuncList;
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
	OutlineType nListType = nOutlineType;

	auto& docOutline = GetDocument().docOutline;
	switch (nOutlineType) {
	case OutlineType::C:			// C/C++ は MakeFuncList_C
	case OutlineType::CPP:			docOutline.MakeFuncList_C(&funcInfoArr);break;
	case OutlineType::PLSQL:		docOutline.MakeFuncList_PLSQL(&funcInfoArr);break;
	case OutlineType::Java:			docOutline.MakeFuncList_Java(&funcInfoArr);break;
	case OutlineType::Cobol:		docOutline.MakeTopicList_cobol(&funcInfoArr);break;
	case OutlineType::Asm:			docOutline.MakeTopicList_asm(&funcInfoArr);break;
	case OutlineType::Perl:			docOutline.MakeFuncList_Perl(&funcInfoArr);break;
	case OutlineType::VisualBasic:	docOutline.MakeFuncList_VisualBasic(&funcInfoArr);break;
	case OutlineType::WZText:		docOutline.MakeTopicList_wztxt(&funcInfoArr);break;		// 階層付テキスト アウトライン解析
	case OutlineType::HTML:			docOutline.MakeTopicList_html(&funcInfoArr);break;		// HTML アウトライン解析
	case OutlineType::TeX:			docOutline.MakeTopicList_tex(&funcInfoArr);break;		// TeX アウトライン解析
	case OutlineType::BookMark:		docOutline.MakeFuncList_BookMark(&funcInfoArr);break;	// 
	case OutlineType::RuleFile:		docOutline.MakeFuncList_RuleFile(&funcInfoArr, titleOverride);break;	// アウトライン解析にルールファイルを導入
	case OutlineType::Python:		docOutline.MakeFuncList_python(&funcInfoArr);break;		// 
	case OutlineType::Erlang:		docOutline.MakeFuncList_Erlang(&funcInfoArr);break;		// 
	case OutlineType::FileTree:	/* 特に何もしない*/ ;break;	// 
	case OutlineType::Text:
		// fall though
		// ここには何も入れてはいけない
	default:
		// プラグインから検索する
		{
			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_OUTLINE, (PlugId)nOutlineType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// インタフェースオブジェクト準備
				WSHIfObj::List params;
				OutlineIfObj* objOutline = new OutlineIfObj(funcInfoArr);
				objOutline->AddRef();
				params.push_back(objOutline);
				// プラグイン呼び出し
				(*plugs.begin())->Invoke(view, params);

				nListType = objOutline->nListType;			// ダイアログの表示方法をを上書き
				titleOverride = objOutline->sOutlineTitle;	// ダイアログタイトルを上書き

				objOutline->Release();
				break;
			}
		}

		// それ以外
		docOutline.MakeTopicList_txt(&funcInfoArr);
		break;
	}

	// 解析対象ファイル名
	_tcscpy(funcInfoArr.szFilePath, GetDocument().docFile.GetFilePath());

	// アウトライン ダイアログの表示
	Point poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			view.GetHwnd(),
			(LPARAM)&view,
			&funcInfoArr,
			poCaret.y + 1,
			poCaret.x + 1,
			nOutlineType,
			nListType,
			view.pTypeData->bLineNumIsCRLF	// 行番号の表示 false=折り返し単位／true=改行単位
		);
	}else {
		// アクティブにする
		dlgFuncList.Redraw(nOutlineType, nListType, &funcInfoArr, poCaret.y + 1, poCaret.x + 1);
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

