#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"

// ViewCommanderクラスのコマンド(Grep)関数群

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void ViewCommander::Command_Grep_Dialog(void)
{
	NativeW memCurText;
	auto& dlgGrep = GetEditWindow().dlgGrep;
	// 2014.07.01 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = (dlgGrep.bSetText == false);

	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	bool bSet = view.GetCurrentTextForSearchDlg(memCurText, bGetHistory);	// 2006.08.23 ryoji ダイアログ専用関数に変更

	if (bSet) {
		dlgGrep.strText = memCurText.GetStringPtr();
		dlgGrep.bSetText = true;
	}

	// Grepダイアログの表示
	INT_PTR nRet = dlgGrep.DoModal(G_AppInstance(), view.GetHwnd(), GetDocument().docFile.GetFilePath());
//	MYTRACE(_T("nRet=%d\n"), nRet);
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	// GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void ViewCommander::Command_Grep(void)
{
	NativeW mWork1;
	NativeT mWork2;
	NativeT mWork3;
	NativeW	mWork4;

	auto& dlgGrep = GetEditWindow().dlgGrep;
	mWork1.SetString(dlgGrep.strText.c_str());
	mWork2.SetString(dlgGrep.szFile);
	mWork3.SetString(dlgGrep.szFolder);

	auto& grepAgent = *EditApp::getInstance().pGrepAgent;
	auto& doc = GetDocument();
	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if (
		(grepAgent.bGrepMode && !grepAgent.bGrepRunning)
		|| (
			!doc.docEditor.IsModified()
			&& !doc.docFile.GetFilePathClass().IsValidPath()		// 現在編集中のファイルのパス
			&& !AppMode::getInstance().IsDebugMode()
		)
	) {
		// 2011.01.23 Grepタイプ別適用
		if (!doc.docEditor.IsModified() && doc.docLineMgr.GetLineCount() == 0) {
			TypeConfigNum typeGrep = DocTypeManager().GetDocumentTypeOfExt(_T("grepout"));
			const TypeConfigMini* pConfig;
			DocTypeManager().GetTypeConfigMini(typeGrep, &pConfig);
			doc.docType.SetDocumentTypeIdx(pConfig->id);
			doc.docType.LockDocumentType();
			doc.OnChangeType();
		}
		
		grepAgent.DoGrep(
			view,
			false,
			&mWork1,
			&mWork4,
			&mWork2,
			&mWork3,
			false,
			dlgGrep.bSubFolder,
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
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(GetEditWindow().GetActiveView(), params);
		}
	}else {
		// 編集ウィンドウの上限チェック
		if (GetDllShareData().nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正	//@@@ 2003.05.31 MIK
			OkMessage(view.GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
			return;
		}

		//======= Grepの実行 =============
		// Grep結果ウィンドウの表示
		ControlTray::DoGrepCreateWindow(G_AppInstance(), view.GetHwnd(), dlgGrep);
	}
	return;
}


/*! GREP置換ダイアログの表示
*/
void ViewCommander::Command_Grep_Replace_Dlg( void )
{
	NativeW memCurText;
	DlgGrepReplace& dlgGrepRep = GetEditWindow().dlgGrepReplace;

	// 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = dlgGrepRep.bSetText == false;

	view.GetCurrentTextForSearchDlg( memCurText, bGetHistory );

	if (0 < memCurText.GetStringLength()) {
		dlgGrepRep.strText = memCurText.GetStringPtr();
		dlgGrepRep.bSetText = true;
	}
	if (0 < GetDllShareData().searchKeywords.replaceKeys.size()) {
		if (dlgGrepRep.nReplaceKeySequence < GetDllShareData().common.search.nReplaceKeySequence) {
			dlgGrepRep.strText2 = GetDllShareData().searchKeywords.replaceKeys[0];
		}
	}

	INT_PTR nRet = dlgGrepRep.DoModal( G_AppInstance(), view.GetHwnd(), GetDocument().docFile.GetFilePath(), (LPARAM)&view );
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP_REPLACE, true, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP置換実行
*/
void ViewCommander::Command_Grep_Replace(void)
{
	NativeW cmWork1;
	NativeT cmWork2;
	NativeT cmWork3;
	NativeW cmWork4;

	DlgGrepReplace& dlgGrepRep = GetEditWindow().dlgGrepReplace;
	cmWork1.SetString( dlgGrepRep.strText.c_str() );
	cmWork2.SetString( dlgGrepRep.szFile );
	cmWork3.SetString( dlgGrepRep.szFolder );
	cmWork4.SetString( dlgGrepRep.strText2.c_str() );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	auto& grepAgent = *EditApp::getInstance().pGrepAgent;
	if ((grepAgent.bGrepMode &&
		  !grepAgent.bGrepRunning ) ||
		( !GetDocument().docEditor.IsModified() &&
		  !GetDocument().docFile.GetFilePathClass().IsValidPath() &&		// 現在編集中のファイルのパス
		  !AppMode::getInstance().IsDebugMode()
		)
	) {
		grepAgent.DoGrep(
			view,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			dlgGrepRep.bSubFolder,
			false, // Stdout
			true, // Header
			dlgGrepRep.searchOption,
			dlgGrepRep.nGrepCharSet,
			dlgGrepRep.nGrepOutputLineType,
			dlgGrepRep.nGrepOutputStyle,
			dlgGrepRep.bGrepOutputFileOnly,
			dlgGrepRep.bGrepOutputBaseFolder,
			dlgGrepRep.bGrepSeparateFolder,
			dlgGrepRep.bPaste,
			dlgGrepRep.bBackup
		);
	}else {
		// 編集ウィンドウの上限チェック
		if (GetDllShareData().nodes.nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( view.GetHwnd(), _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
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
		cmdLine.AppendStringLiteral(_T("-GREPMODE -GKEY=\""));
		cmdLine.AppendStringW(cmWork1.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GREPR=\""));
		cmdLine.AppendStringW(cmWork4.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GFILE=\""));
		cmdLine.AppendString(cmWork2.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GFOLDER=\""));
		cmdLine.AppendString(cmWork3.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), dlgGrepRep.nGrepCharSet );
		cmdLine.AppendString(szTemp);

		//GOPTオプション
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if (dlgGrepRep.bSubFolder				) _tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
		if (dlgGrepRep.searchOption.bWordOnly	) _tcscat( pOpt, _T("W") );	// 単語単位で探す
		if (dlgGrepRep.searchOption.bLoHiCase	) _tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
		if (dlgGrepRep.searchOption.bRegularExp	) _tcscat( pOpt, _T("R") );	// 正規表現
		if (dlgGrepRep.nGrepOutputLineType == 1	) _tcscat( pOpt, _T("P") );	// 行を出力する
		// if (dlgGrepRep.nGrepOutputLineType == 2) _tcscat( pOpt, _T("N") );	// 否ヒット行を出力する 2014.09.23
		if (dlgGrepRep.nGrepOutputStyle == 1	) _tcscat( pOpt, _T("1") );	// Grep: 出力形式
		if (dlgGrepRep.nGrepOutputStyle == 2	) _tcscat( pOpt, _T("2") );	// Grep: 出力形式
		if (dlgGrepRep.nGrepOutputStyle == 3	) _tcscat( pOpt, _T("3") );
		if (dlgGrepRep.bGrepOutputFileOnly		) _tcscat( pOpt, _T("F") );
		if (dlgGrepRep.bGrepOutputBaseFolder	) _tcscat( pOpt, _T("B") );
		if (dlgGrepRep.bGrepSeparateFolder		) _tcscat( pOpt, _T("D") );
		if (dlgGrepRep.bPaste					) _tcscat( pOpt, _T("C") );	// クリップボードから貼り付け
		if (dlgGrepRep.bBackup				) _tcscat( pOpt, _T("O") );	// バックアップ作成
		if (0 < _tcslen( pOpt )) {
			cmdLine.AppendStringLiteral( _T(" -GOPT=") );
			cmdLine.AppendString( pOpt );
		}

		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_NONE;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			view.GetHwnd(),
			loadInfo,
			cmdLine.GetStringPtr(),
			false,
			NULL,
			GetDllShareData().common.tabBar.bNewWindow
		);
	}
	return;
}

