/*!	@file
@brief ViewCommanderクラスのコマンド(Grep)関数群

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void ViewCommander::Command_GREP_DIALOG(void)
{
	NativeW memCurText;
	// 2014.07.01 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = GetEditWindow()->m_dlgGrep.m_bSetText == false;

	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	bool bSet = m_pCommanderView->GetCurrentTextForSearchDlg(memCurText, bGetHistory);	// 2006.08.23 ryoji ダイアログ専用関数に変更

	if (bSet) {
		GetEditWindow()->m_dlgGrep.m_strText = memCurText.GetStringPtr();
		GetEditWindow()->m_dlgGrep.m_bSetText = true;
	}

	// Grepダイアログの表示
	int nRet = GetEditWindow()->m_dlgGrep.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_docFile.GetFilePath());
//	MYTRACE(_T("nRet=%d\n"), nRet);
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	// GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void ViewCommander::Command_GREP(void)
{
	NativeW mWork1;
	NativeT mWork2;
	NativeT mWork3;
	NativeW	mWork4;

	auto& dlgGrep = GetEditWindow()->m_dlgGrep;
	mWork1.SetString(dlgGrep.m_strText.c_str());
	mWork2.SetString(dlgGrep.m_szFile);
	mWork3.SetString(dlgGrep.m_szFolder);

	auto& grepAgent = *EditApp::getInstance()->m_pGrepAgent;
	auto& doc = *GetDocument();
	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if (
		(
			grepAgent.m_bGrepMode
			&& !grepAgent.m_bGrepRunning
		)
		|| (
			!doc.m_docEditor.IsModified()
			&& !doc.m_docFile.GetFilePathClass().IsValidPath()		// 現在編集中のファイルのパス
			&& !AppMode::getInstance()->IsDebugMode()
		)
	) {
		// 2011.01.23 Grepタイプ別適用
		if (!doc.m_docEditor.IsModified() && doc.m_docLineMgr.GetLineCount() == 0) {
			TypeConfigNum cTypeGrep = DocTypeManager().GetDocumentTypeOfExt(_T("grepout"));
			const TypeConfigMini* pConfig;
			DocTypeManager().GetTypeConfigMini(cTypeGrep, &pConfig);
			doc.m_docType.SetDocumentTypeIdx(pConfig->id);
			doc.m_docType.LockDocumentType();
			doc.OnChangeType();
		}
		
		grepAgent.DoGrep(
			m_pCommanderView,
			false,
			&mWork1,
			&mWork4,
			&mWork2,
			&mWork3,
			false,
			dlgGrep.m_bSubFolder,
			false,
			true, // Header
			dlgGrep.searchOption,
			dlgGrep.nGrepCharSet,
			dlgGrep.nGrepOutputLineType,
			dlgGrep.nGrepOutputStyle,
			dlgGrep.bGrepOutputFileOnly,
			dlgGrep.bGrepOutputBaseFolder,
			dlgGrep.bGrepSeparateFolder,
			false,
			false
		);

		// プラグイン：DocumentOpenイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
		}
	}else {
		// 編集ウィンドウの上限チェック
		if (GetDllShareData().m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正	//@@@ 2003.05.31 MIK
			OkMessage(m_pCommanderView->GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
			return;
		}

		//======= Grepの実行 =============
		// Grep結果ウィンドウの表示
		ControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), dlgGrep);
	}
	return;
}


/*! GREP置換ダイアログの表示
*/
void ViewCommander::Command_GREP_REPLACE_DLG( void )
{
	NativeW memCurText;
	DlgGrepReplace& dlgGrepRep = GetEditWindow()->m_dlgGrepReplace;

	// 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = dlgGrepRep.m_bSetText == false;

	m_pCommanderView->GetCurrentTextForSearchDlg( memCurText, bGetHistory );

	if (0 < memCurText.GetStringLength()) {
		dlgGrepRep.m_strText = memCurText.GetStringPtr();
		dlgGrepRep.m_bSetText = true;
	}
	if (0 < GetDllShareData().m_searchKeywords.replaceKeys.size()) {
		if (dlgGrepRep.nReplaceKeySequence < GetDllShareData().m_common.search.nReplaceKeySequence) {
			dlgGrepRep.m_strText2 = GetDllShareData().m_searchKeywords.replaceKeys[0];
		}
	}

	int nRet = dlgGrepRep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_docFile.GetFilePath(), (LPARAM)m_pCommanderView );
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP置換実行
*/
void ViewCommander::Command_GREP_REPLACE(void)
{
	NativeW cmWork1;
	NativeT cmWork2;
	NativeT cmWork3;
	NativeW cmWork4;

	DlgGrepReplace& dlgGrepRep = GetEditWindow()->m_dlgGrepReplace;
	cmWork1.SetString( dlgGrepRep.m_strText.c_str() );
	cmWork2.SetString( dlgGrepRep.m_szFile );
	cmWork3.SetString( dlgGrepRep.m_szFolder );
	cmWork4.SetString( dlgGrepRep.m_strText2.c_str() );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if (( EditApp::getInstance()->m_pGrepAgent->m_bGrepMode &&
		  !EditApp::getInstance()->m_pGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_docEditor.IsModified() &&
		  !GetDocument()->m_docFile.GetFilePathClass().IsValidPath() &&		// 現在編集中のファイルのパス
		  !AppMode::getInstance()->IsDebugMode()
		)
	) {
		EditApp::getInstance()->m_pGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			dlgGrepRep.m_bSubFolder,
			false, // Stdout
			true, // Header
			dlgGrepRep.searchOption,
			dlgGrepRep.nGrepCharSet,
			dlgGrepRep.nGrepOutputLineType,
			dlgGrepRep.nGrepOutputStyle,
			dlgGrepRep.bGrepOutputFileOnly,
			dlgGrepRep.bGrepOutputBaseFolder,
			dlgGrepRep.bGrepSeparateFolder,
			dlgGrepRep.m_bPaste,
			dlgGrepRep.m_bBackup
		);
	}else {
		// 編集ウィンドウの上限チェック
		if (GetDllShareData().m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
			return;
		}
		// ======= Grepの実行 =============
		// Grep結果ウィンドウの表示
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		NativeT cmdLine;
		TCHAR szTemp[20];
		cmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
		cmdLine.AppendStringW(cmWork1.GetStringPtr());
		cmdLine.AppendString(_T("\" -GREPR=\""));
		cmdLine.AppendStringW(cmWork4.GetStringPtr());
		cmdLine.AppendString(_T("\" -GFILE=\""));
		cmdLine.AppendString(cmWork2.GetStringPtr());
		cmdLine.AppendString(_T("\" -GFOLDER=\""));
		cmdLine.AppendString(cmWork3.GetStringPtr());
		cmdLine.AppendString(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), dlgGrepRep.nGrepCharSet );
		cmdLine.AppendString(szTemp);

		//GOPTオプション
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if (dlgGrepRep.m_bSubFolder				) _tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
		if (dlgGrepRep.searchOption.bWordOnly	) _tcscat( pOpt, _T("W") );	// 単語単位で探す
		if (dlgGrepRep.searchOption.bLoHiCase	) _tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
		if (dlgGrepRep.searchOption.bRegularExp	) _tcscat( pOpt, _T("R") );	// 正規表現
		if (dlgGrepRep.nGrepOutputLineType == 1	) _tcscat( pOpt, _T("P") );	// 行を出力する
		// if (dlgGrepRep.nGrepOutputLineType == 2) _tcscat( pOpt, _T("N") );	// 否ヒット行を出力する 2014.09.23
		if (dlgGrepRep.nGrepOutputStyle == 1		) _tcscat( pOpt, _T("1") );	// Grep: 出力形式
		if (dlgGrepRep.nGrepOutputStyle == 2		) _tcscat( pOpt, _T("2") );	// Grep: 出力形式
		if (dlgGrepRep.nGrepOutputStyle == 3		) _tcscat( pOpt, _T("3") );
		if (dlgGrepRep.bGrepOutputFileOnly		) _tcscat( pOpt, _T("F") );
		if (dlgGrepRep.bGrepOutputBaseFolder		) _tcscat( pOpt, _T("B") );
		if (dlgGrepRep.bGrepSeparateFolder		) _tcscat( pOpt, _T("D") );
		if (dlgGrepRep.m_bPaste					) _tcscat( pOpt, _T("C") );	// クリップボードから貼り付け
		if (dlgGrepRep.m_bBackup					) _tcscat( pOpt, _T("O") );	// バックアップ作成
		if (0 < _tcslen( pOpt )) {
			cmdLine.AppendString( _T(" -GOPT=") );
			cmdLine.AppendString( pOpt );
		}

		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_NONE;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			loadInfo,
			cmdLine.GetStringPtr(),
			false,
			NULL,
			GetDllShareData().m_common.tabBar.bNewWindow
		);
	}
	return;
}

