#include "StdAfx.h"
#include "GrepAgent.h"
#include "GrepEnumKeys.h"
#include "GrepEnumFilterFiles.h"
#include "GrepEnumFilterFolders.h"
#include "SearchAgent.h"
#include "dlg/DlgCancel.h"
#include "_main/AppMode.h"
#include "OpeBlk.h"
#include "window/EditWnd.h"
#include "charset/CodeMediator.h"
#include "view/colors/ColorStrategy.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/CodePage.h"
#include "io/FileLoad.h"
#include "io/BinaryStream.h"
#include "util/window.h"
#include "util/module.h"
#include "debug/RunningTimer.h"
#include <deque>
#include <memory>

#include "sakura_rc.h"

GrepAgent::GrepAgent()
	:
	m_bGrepMode(false),		// Grepモードか
	m_bGrepRunning(false)	// Grep処理中
{
}

CallbackResultType GrepAgent::OnBeforeClose()
{
	// GREP処理中は終了できない
	if (m_bGrepRunning) {
		// アクティブにする
		ActivateFrameWindow(EditWnd::getInstance().GetHwnd());	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_GREP_RUNNINNG)
		);
		return CallbackResultType::Interrupt;
	}
	return CallbackResultType::Continue;
}

void GrepAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	m_bGrepMode = false;	// grepウィンドウは通常ウィンドウ化
	AppMode::getInstance().m_szGrepKey[0] = 0;
}

/*!
	@date 2014.03.09 novice 最後の\\を取り除くのをやめる(d:\\ -> d:になる)
*/
void GrepAgent::CreateFolders(const TCHAR* pszPath, std::vector<std::tstring>& vPaths)
{
	const int nPathLen = auto_strlen(pszPath);
	auto szPath = std::make_unique<TCHAR[]>(nPathLen + 1);
	auto szTmp = std::make_unique<TCHAR[]>(nPathLen + 1);
	auto_strcpy(&szPath[0], pszPath);
	TCHAR* token;
	int nPathPos = 0;
	while (token = my_strtok<TCHAR>(&szPath[0], nPathLen, &nPathPos, _T(";"))) {
		auto_strcpy(&szTmp[0], token);
		TCHAR* p;
		TCHAR* q;
		p = q = &szTmp[0];
		while (*p) {
			if (*p != _T('"')) {
				if (p != q) {
					*q = *p;
				}
				++q;
			}
			++p;
		}
		*q = _T('\0');
#if 0
		// 2011.12.25 仕様変更。最後の\\は取り除く
		int	nFolderLen = q - &szTmp[0];
		if (0 < nFolderLen) {
			int nCharChars = &szTmp[nFolderLen] - CNativeT::GetCharPrev(&szTmp[0], nFolderLen, &szTmp[nFolderLen]);
			if (nCharChars == 1 && (_T('\\') == szTmp[nFolderLen - 1] || _T('/') == szTmp[nFolderLen - 1])) {
				szTmp[nFolderLen - 1] = _T('\0');
			}
		}
#endif
		// ロングファイル名を取得する
		TCHAR szTmp2[_MAX_PATH];
		if (::GetLongFileName(&szTmp[0], szTmp2)) {
			vPaths.push_back(szTmp2);
		}else {
			vPaths.push_back(&szTmp[0]);
		}
	}
}

/*! 最後の\\を取り除く
	@date 2014.03.09 novice 新規作成
*/
std::tstring GrepAgent::ChopYen( const std::tstring& str )
{
	std::tstring dst = str;
	size_t nPathLen = dst.length();

	// 最後のフォルダ区切り記号を削除する
	// [A:\]などのルートであっても削除
	for (size_t i=0; i<nPathLen; ++i) {
		if (1
			&& dst[i] == _T('\\')
			&& i == nPathLen - 1
		) {
			dst.resize( nPathLen - 1 );
			break;
		}
	}

	return dst;
}

void GrepAgent::AddTail( EditView& editView, const NativeW& mem, bool bAddStdout )
{
	if (bAddStdout) {
		HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (out && out != INVALID_HANDLE_VALUE) {
			Memory memOut;
			std::unique_ptr<CodeBase> pCodeBase( CodeFactory::CreateCodeBase(
					editView.GetDocument().GetDocumentEncoding(), 0) );
			pCodeBase->UnicodeToCode( mem, &memOut );
			DWORD dwWrite = 0;
			::WriteFile(out, memOut.GetRawPtr(), memOut.GetRawLength(), &dwWrite, NULL);
		}
	}else {
		editView.GetCommander().Command_ADDTAIL( mem.GetStringPtr(), mem.GetStringLength() );
	}
}

/*! Grep実行

  @param[in] pmGrepKey 検索パターン
  @param[in] pmGrepFile 検索対象ファイルパターン(!で除外指定))
  @param[in] pmGrepFolder 検索対象フォルダ

  @date 2008.12.07 nasukoji	ファイル名パターンのバッファオーバラン対策
  @date 2008.12.13 genta 検索パターンのバッファオーバラン対策
  @date 2012.10.13 novice 検索オプションをクラスごと代入
*/
DWORD GrepAgent::DoGrep(
	EditView&				viewDst,
	bool					bGrepReplace,
	const NativeW*			pmGrepKey,
	const NativeW*			pmGrepReplace,
	const NativeT*			pmGrepFile,
	const NativeT*			pmGrepFolder,
	bool					bGrepCurFolder,
	bool					bGrepSubFolder,
	bool					bGrepStdout,
	bool					bGrepHeader,
	const SearchOption&		searchOption,
	EncodingType			nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
	int						nGrepOutputLineType,
	int						nGrepOutputStyle,
	bool					bGrepOutputFileOnly,
	bool					bGrepOutputBaseFolder,
	bool					bGrepSeparateFolder,
	bool					bGrepPaste,
	bool					bGrepBackup
	)
{
#ifdef _DEBUG
	RunningTimer runningTimer("EditView::DoGrep");
#endif

	// 再入不可
	if (this->m_bGrepRunning) {
		assert_warning(!this->m_bGrepRunning);
		return 0xffffffff;
	}

	this->m_bGrepRunning = true;

	int			nHitCount = 0;
	DlgCancel	dlgCancel;
	HWND		hwndCancel;
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	Bregexp		regexp;
	NativeW		memMessage;
	int			nWork;
	GrepOption	grepOption;

	/*
	|| バッファサイズの調整
	*/
	memMessage.AllocStringBuffer(4000);

	viewDst.m_bDoing_UndoRedo		= true;


	// アンドゥバッファの処理
	if (viewDst.GetDocument().m_docEditor.m_pOpeBlk) {	// 操作ブロック
//@@@2002.2.2 YAZAKI NULLじゃないと進まないので、とりあえずコメント。＆NULLのときは、new OpeBlkする。
//		while (m_pOpeBlk) {}
//		delete m_pOpeBlk;
//		m_pOpeBlk = NULL;
	}else {
		viewDst.GetDocument().m_docEditor.m_pOpeBlk = new OpeBlk;
		viewDst.GetDocument().m_docEditor.m_nOpeBlkRedawCount = 0;
	}
	viewDst.GetDocument().m_docEditor.m_pOpeBlk->AddRef();

	viewDst.m_bCurSrchKeyMark = true;								// 検索文字列のマーク
	viewDst.m_strCurSearchKey = pmGrepKey->GetStringPtr();		// 検索文字列
	viewDst.m_curSearchOption = searchOption;						// 検索オプション
	viewDst.m_nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

	// 置換後文字列の準備
	NativeW memReplace;
	if (bGrepReplace) {
		if (bGrepPaste) {
			// 矩形・ラインモード貼り付けは未サポート
			bool bColmnSelect;
			bool bLineSelect;
			if (!viewDst.MyGetClipboardData(memReplace, &bColmnSelect, GetDllShareData().common.edit.bEnableLineModePaste? &bLineSelect: NULL)) {
				this->m_bGrepRunning = false;
				viewDst.m_bDoing_UndoRedo = false;
				ErrorMessage( viewDst.m_hwndParent, LS(STR_DLGREPLC_CLIPBOARD) );
				return 0;
			}
			if (bLineSelect) {
				int len = memReplace.GetStringLength();
				if (memReplace[len - 1] != WCODE::CR && memReplace[len - 1] != WCODE::LF) {
					memReplace.AppendString(viewDst.GetDocument().m_docEditor.GetNewLineCode().GetValue2());
				}
			}
			if (GetDllShareData().common.edit.bConvertEOLPaste) {
				LogicInt len = memReplace.GetStringLength();
				std::vector<wchar_t> convertedText(len * 2); // 全文字\n→\r\n変換で最大の２倍になる
				wchar_t* pszConvertedText = &convertedText[0];
				LogicInt nConvertedTextLen = viewDst.m_commander.ConvertEol(memReplace.GetStringPtr(), len, pszConvertedText);
				memReplace.SetString(pszConvertedText, nConvertedTextLen);
			}
		}else {
			memReplace = *pmGrepReplace;
		}
	}	// 正規表現

	//	From Here Jun. 27 genta
	/*
		Grepを行うに当たって検索・画面色分け用正規表現バッファも
		初期化する．これはGrep検索結果の色分けを行うため．

		Note: ここで強調するのは最後の検索文字列であって
		Grep対象パターンではないことに注意
	*/
	if (!viewDst.m_searchPattern.SetPattern(viewDst.GetHwnd(),
												viewDst.m_strCurSearchKey.c_str(),
												viewDst.m_strCurSearchKey.size(),
												viewDst.m_curSearchOption,
												&viewDst.m_curRegexp)
	) {
		this->m_bGrepRunning = false;
		viewDst.m_bDoing_UndoRedo = false;
		viewDst.SetUndoBuffer();
		return 0;
	}

	// 2014.06.13 別ウィンドウで検索したとき用にGrepダイアログの検索キーを設定
	viewDst.m_editWnd.m_dlgGrep.m_strText = pmGrepKey->GetStringPtr();
	viewDst.m_editWnd.m_dlgGrep.m_bSetText = true;
	viewDst.m_editWnd.m_dlgGrepReplace.m_strText = pmGrepKey->GetStringPtr();
	if (bGrepReplace) {
		viewDst.m_editWnd.m_dlgGrepReplace.m_strText2 = pmGrepReplace->GetStringPtr();
	}
	viewDst.m_editWnd.m_dlgGrepReplace.m_bSetText = true;
	hwndCancel = dlgCancel.DoModeless(G_AppInstance(), viewDst.m_hwndParent, IDD_GREPRUNNING);

	::SetDlgItemInt(hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE);
	::DlgItem_SetText(hwndCancel, IDC_STATIC_CURFILE, _T(" "));	// 2002/09/09 Moca add
	::CheckDlgButton(hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().common.search.bGrepRealTimeView);	// 2003.06.23 Moca

	//	2008.12.13 genta パターンが長すぎる場合は登録しない
	//	(正規表現が途中で途切れると困るので)
	//	2011.12.10 Moca 表示の際に...に切り捨てられるので登録するように
	wcsncpy_s(AppMode::getInstance().m_szGrepKey, _countof(AppMode::getInstance().m_szGrepKey), pmGrepKey->GetStringPtr(), _TRUNCATE);
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	SearchStringPattern pattern;
	{
		// 検索パターンのコンパイル
		bool bError;
		if (bGrepReplace && !bGrepPaste) {
			// Grep置換
			bError = !pattern.SetPattern(viewDst.GetHwnd(),
										pmGrepKey->GetStringPtr(),
										pmGrepKey->GetStringLength(),
										memReplace.GetStringPtr(),
										searchOption,
										&regexp);
		}else {
			bError = !pattern.SetPattern(viewDst.GetHwnd(),
										pmGrepKey->GetStringPtr(),
										pmGrepKey->GetStringLength(),
										searchOption,
										&regexp);
		}
		if (bError) {
			this->m_bGrepRunning = false;
			viewDst.m_bDoing_UndoRedo = false;
			viewDst.SetUndoBuffer();
			return 0;
		}
	}
	
	// Grepオプションまとめ
	grepOption.bGrepSubFolder = bGrepSubFolder;
	grepOption.bGrepStdout = bGrepStdout;
	grepOption.bGrepHeader = bGrepHeader;
	grepOption.nGrepCharSet = nGrepCharSet;
	grepOption.nGrepOutputLineType = nGrepOutputLineType;
	grepOption.nGrepOutputStyle = nGrepOutputStyle;
	grepOption.bGrepOutputFileOnly = bGrepOutputFileOnly;
	grepOption.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	grepOption.bGrepSeparateFolder = bGrepSeparateFolder;
	grepOption.bGrepReplace = bGrepReplace;
	grepOption.bGrepPaste = bGrepPaste;
	grepOption.bGrepBackup = bGrepBackup;
	if (grepOption.bGrepReplace) {
		// Grep否定行はGrep置換では無効
		if (grepOption.nGrepOutputLineType == 2) {
			grepOption.nGrepOutputLineType = 1; // 行単位
		}
	}

	// 2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	// Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig   = GetAppIcon(G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, false);
	hIconSmall = GetAppIcon(G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true);

	// Sep. 10, 2002 genta
	// EditWndに新設した関数を使うように
	auto& editWnd = EditWnd::getInstance();	//	Sep. 10, 2002 genta
	editWnd.SetWindowIcon(hIconSmall, ICON_SMALL);
	editWnd.SetWindowIcon(hIconBig, ICON_BIG);

	GrepEnumKeys grepEnumKeys;
	{
		int nErrorNo = grepEnumKeys.SetFileKeys(pmGrepFile->GetStringPtr());
		if (nErrorNo != 0) {
			this->m_bGrepRunning = false;
			viewDst.m_bDoing_UndoRedo = false;
			viewDst.SetUndoBuffer();

			const TCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if (nErrorNo == 1) {
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}else if (nErrorNo == 2) {
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			ErrorMessage(viewDst.m_hwndParent, _T("%ts"), pszErrorMessage);
			return 0;
		}
	}

	std::vector<std::tstring> vPaths;
	CreateFolders(pmGrepFolder->GetStringPtr(), vPaths);

	nWork = pmGrepKey->GetStringLength(); // 2003.06.10 Moca あらかじめ長さを計算しておく

	// 最後にテキストを追加
	NativeW memWork;
	memMessage.AppendString(LSW(STR_GREP_SEARCH_CONDITION));	//L"\r\n□検索条件  "
	if (0 < nWork) {
		NativeW memWork2;
		memWork2.SetNativeData(*pmGrepKey);
		const TypeConfig& type = viewDst.m_pEditDoc->m_docType.GetDocumentAttribute();
		if (!type.colorInfoArr[COLORIDX_WSTRING].bDisp) {
			// 2011.11.28 色指定が無効ならエスケープしない
		}else
		if (type.stringType == StringLiteralType::CPP
			|| type.stringType == StringLiteralType::CSharp
			|| type.stringType == StringLiteralType::Python
		) {	// 文字列区切り記号エスケープ方法
			memWork2.Replace(L"\\", L"\\\\");
			memWork2.Replace(L"\'", L"\\\'");
			memWork2.Replace(L"\"", L"\\\"");
		}else if (type.stringType == StringLiteralType::PLSQL) {
			memWork2.Replace(L"\'", L"\'\'");
			memWork2.Replace(L"\"", L"\"\"");
		}
		memWork.AppendString(L"\"");
		memWork.AppendNativeData(memWork2);
		memWork.AppendString(L"\"\r\n");
	}else {
		memWork.AppendString(LSW(STR_GREP_SEARCH_FILE));	// L"「ファイル検索」\r\n"
	}
	memMessage += memWork;

	if (bGrepReplace) {
		memMessage.AppendString( LSW(STR_GREP_REPLACE_TO) );
		if (bGrepPaste) {
			memMessage.AppendString( LSW(STR_GREP_PASTE_CLIPBOAD) );
		}else {
			NativeW memWork2;
			memWork2.SetNativeData( memReplace );
			const TypeConfig& type = viewDst.m_pEditDoc->m_docType.GetDocumentAttribute();
			if (!type.colorInfoArr[COLORIDX_WSTRING].bDisp) {
				// 2011.11.28 色指定が無効ならエスケープしない
			}else
			if (0
				|| type.stringType == StringLiteralType::CPP
				|| type.stringType == StringLiteralType::CSharp
				|| type.stringType == StringLiteralType::Python
			) {	// 文字列区切り記号エスケープ方法
				memWork2.Replace( L"\\", L"\\\\" );
				memWork2.Replace( L"\'", L"\\\'" );
				memWork2.Replace( L"\"", L"\\\"" );
			}else if (type.stringType == StringLiteralType::PLSQL) {
				memWork2.Replace( L"\'", L"\'\'" );
				memWork2.Replace( L"\"", L"\"\"" );
			}
			memMessage.AppendString( L"\"" );
			memMessage.AppendNativeData( memWork2 );
			memMessage.AppendString( L"\"\r\n" );
		}
	}


	memMessage.AppendString(LSW(STR_GREP_SEARCH_TARGET));	// L"検索対象   "
	if (viewDst.m_pEditDoc->m_docType.GetDocumentAttribute().stringType == StringLiteralType::CPP) {	// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	}else {
	}
	memWork.SetStringT(pmGrepFile->GetStringPtr());
	memMessage += memWork;

	memMessage.AppendString(L"\r\n");
	memMessage.AppendString(LSW(STR_GREP_SEARCH_FOLDER));	// L"フォルダ   "
	{
		std::tstring grepFolder;
		for (int i=0; i<(int)vPaths.size(); ++i) {
			if (i) {
				grepFolder += _T(';');
			}
			std::tstring sPath = ChopYen( vPaths[i] );
			if (auto_strchr( sPath.c_str(), _T(';') )) {
				grepFolder += _T('"');
				grepFolder += sPath;
				grepFolder += _T('"');
			}else {
				grepFolder += sPath;
			}
		}
		memWork.SetStringT(grepFolder.c_str());
	}
	if (viewDst.m_pEditDoc->m_docType.GetDocumentAttribute().stringType == StringLiteralType::CPP) {	// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	}else {
	}
	memMessage += memWork;
	memMessage.AppendString(L"\r\n");

	const wchar_t*	pszWork;
	if (grepOption.bGrepSubFolder) {
		pszWork = LSW(STR_GREP_SUBFOLDER_YES);	// L"    (サブフォルダも検索)\r\n"
	}else {
		pszWork = LSW(STR_GREP_SUBFOLDER_NO);	// L"    (サブフォルダを検索しない)\r\n"
	}
	memMessage.AppendString(pszWork);

	if (0 < nWork) { // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if (searchOption.bWordOnly) {
		// 単語単位で探す
			memMessage.AppendString(LSW(STR_GREP_COMPLETE_WORD));	// L"    (単語単位で探す)\r\n"
		}

		if (searchOption.bLoHiCase) {
			pszWork = LSW(STR_GREP_CASE_SENSITIVE);	// L"    (英大文字小文字を区別する)\r\n"
		}else {
			pszWork = LSW(STR_GREP_IGNORE_CASE);	// L"    (英大文字小文字を区別しない)\r\n"
		}
		memMessage.AppendString(pszWork);

		if (searchOption.bRegularExp) {
			//	2007.07.22 genta : 正規表現ライブラリのバージョンも出力する
			memMessage.AppendString(LSW(STR_GREP_REGEX_DLL));	// L"    (正規表現:"
			memMessage.AppendStringT(regexp.GetVersionT());
			memMessage.AppendString(L")\r\n");
		}
	}

	if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
		memMessage.AppendString(LSW(STR_GREP_CHARSET_AUTODETECT));	// L"    (文字コードセットの自動判別)\r\n"
	}else if (IsValidCodeOrCPType(grepOption.nGrepCharSet)) {
		memMessage.AppendString(LSW(STR_GREP_CHARSET));	// L"    (文字コードセット："
		TCHAR szCpName[100];
		CodePage::GetNameNormal(szCpName, grepOption.nGrepCharSet);
		memMessage.AppendStringT( szCpName );
		memMessage.AppendString(L")\r\n");
	}

	if (0 < nWork) { // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if (grepOption.nGrepOutputLineType == 1) {
			// 該当行
			pszWork = LSW(STR_GREP_SHOW_MATCH_LINE);	// L"    (一致した行を出力)\r\n"
		}else if (grepOption.nGrepOutputLineType == 2) {
			// 否該当行
			pszWork = LSW( STR_GREP_SHOW_MATCH_NOHITLINE );	//L"    (一致しなかった行を出力)\r\n"
		}else {
			if (bGrepReplace && searchOption.bRegularExp && !bGrepPaste) {
				pszWork = LSW(STR_GREP_SHOW_FIRST_LINE);
			}else {
				pszWork = LSW( STR_GREP_SHOW_MATCH_AREA );
			}
		}
		memMessage.AppendString(pszWork);

		if (grepOption.bGrepOutputFileOnly) {
			pszWork = LSW(STR_GREP_SHOW_FIRST_MATCH);	// L"    (ファイル毎最初のみ検索)\r\n"
			memMessage.AppendString(pszWork);
		}
	}

	memMessage.AppendString(L"\r\n\r\n");
	pszWork = memMessage.GetStringPtr(&nWork);
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	LayoutInt tmp_PosY_Layout = viewDst.m_pEditDoc->m_layoutMgr.GetLineCount();
	if (0 < nWork && grepOption.bGrepHeader) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
	}
	memMessage.Clear(); // もういらない
	pszWork = NULL;
	
	//	2007.07.22 genta バージョンを取得するために，
	//	正規表現の初期化を上へ移動

	// 表示処理ON/OFF
	// 2003.06.23 Moca 共通設定で変更できるように
	// 2008.06.08 ryoji 全ビューの表示ON/OFFを同期させる
//	SetDrawSwitch(false);
	if (!EditWnd::getInstance().UpdateTextWrap()) {		// 折り返し方法関連の更新
		EditWnd::getInstance().RedrawAllViews(&viewDst);	// 他のペインの表示を更新
	}
	const bool bDrawSwitchOld = viewDst.SetDrawSwitch(GetDllShareData().common.search.bGrepRealTimeView != 0);

	GrepEnumOptions grepEnumOptions;
	GrepEnumFiles grepExceptAbsFiles;
	grepExceptAbsFiles.Enumerates(_T(""), grepEnumKeys.m_vecExceptAbsFileKeys, grepEnumOptions);
	GrepEnumFolders grepExceptAbsFolders;
	grepExceptAbsFolders.Enumerates(_T(""), grepEnumKeys.m_vecExceptAbsFolderKeys, grepEnumOptions);

	int nGrepTreeResult = 0;

	for (int nPath=0; nPath<(int)vPaths.size(); ++nPath) {
		bool bOutputBaseFolder = false;
		std::tstring sPath = ChopYen( vPaths[nPath] );
		int nTreeRet = DoGrepTree(
			viewDst,
			dlgCancel,
			pmGrepKey->GetStringPtr(),
			memReplace,
			grepEnumKeys,
			grepExceptAbsFiles,
			grepExceptAbsFolders,
			sPath.c_str(),
			sPath.c_str(),
			searchOption,
			grepOption,
			pattern,
			regexp,
			0,
			bOutputBaseFolder,
			&nHitCount
		);
		if (nTreeRet == -1) {
			nGrepTreeResult = -1;
			break;
		}
		nGrepTreeResult += nTreeRet;
	}
	if (nGrepTreeResult == -1 && grepOption.bGrepHeader) {
		const wchar_t* p = LSW(STR_GREP_SUSPENDED);	// L"中断しました。\r\n"
		NativeW memSuspend;
		memSuspend.SetString( p );
		AddTail( viewDst, memSuspend, grepOption.bGrepStdout );
	}
	if (grepOption.bGrepHeader) {
		WCHAR szBuffer[128];
		if (bGrepReplace) {
			auto_sprintf( szBuffer, LSW(STR_GREP_REPLACE_COUNT), nHitCount );
		}else {
			auto_sprintf( szBuffer, LSW( STR_GREP_MATCH_COUNT ), nHitCount );
		}
		NativeW memOutput;
		memOutput.SetString( szBuffer );
		AddTail( viewDst, memOutput, grepOption.bGrepStdout );
#ifdef _DEBUG
		auto_sprintf( szBuffer, LSW(STR_GREP_TIMER), runningTimer.Read() );
		memOutput.SetString( szBuffer );
		AddTail( viewDst, memOutput, grepOption.bGrepStdout );
#endif
	}
	viewDst.GetCaret().MoveCursor(LayoutPoint(LayoutInt(0), tmp_PosY_Layout), true);	// カーソルをGrep直前の位置に戻す。

	dlgCancel.CloseDialog(0);

	// アクティブにする
	ActivateFrameWindow(EditWnd::getInstance().GetHwnd());

	// アンドゥバッファの処理
	viewDst.SetUndoBuffer();

	// Apr. 13, 2001 genta
	// Grep実行後はファイルを変更無しの状態にする．
	viewDst.m_pEditDoc->m_docEditor.SetModified(false, false);

	this->m_bGrepRunning = false;
	viewDst.m_bDoing_UndoRedo = false;

	// 表示処理ON/OFF
	editWnd.SetDrawSwitchOfAllViews(bDrawSwitchOld);

	// 再描画
	if (!editWnd.UpdateTextWrap()) {	// 折り返し方法関連の更新	// 2008.06.10 ryoji
		editWnd.RedrawAllViews(NULL);
	}

	if (!bGrepCurFolder) {
		// 現行フォルダを検索したフォルダに変更
		if (0 < vPaths.size()) {
			::SetCurrentDirectory(vPaths[0].c_str());
		}
	}

	return nHitCount;
}


/*! @brief Grep実行

	@date 2001.06.27 genta	正規表現ライブラリの差し替え
	@date 2003.06.23 Moca   サブフォルダ→ファイルだったのをファイル→サブフォルダの順に変更
	@date 2003.06.23 Moca   ファイル名から""を取り除くように
	@date 2003.03.27 みく   除外ファイル指定の導入と重複検索防止の追加．
		大部分が変更されたため，個別の変更点記入は無し．
*/
int GrepAgent::DoGrepTree(
	EditView&				viewDst,
	DlgCancel&				dlgCancel,				// [in] Cancelダイアログへのポインタ
	const wchar_t*			pszKey,					// [in] 検索キー
	const NativeW&			mGrepReplace,
	const GrepEnumKeys&		grepEnumKeys,			// [in] 検索対象ファイルパターン
	GrepEnumFiles&			grepExceptAbsFiles,		// [in] 除外ファイル絶対パス
	GrepEnumFolders&		grepExceptAbsFolders,	// [in] 除外フォルダ絶対パス
	const TCHAR*			pszPath,				// [in] 検索対象パス
	const TCHAR*			pszBasePath,			// [in] 検索対象パス(ベースフォルダ)
	const SearchOption&		searchOption,			// [in] 検索オプション
	const GrepOption&		grepOption,				// [in] Grepオプション
	const SearchStringPattern& pattern,				// [in] 検索パターン
	Bregexp&				regexp,					// [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int						nNest,					// [in] ネストレベル
	bool&					bOutputBaseFolder,		// [i/o] ベースフォルダ名出力
	int*					pnHitCount				// [i/o] ヒット数の合計
	)
{
	dlgCancel.SetItemText(IDC_STATIC_CURPATH, pszPath);

	NativeW	memMessage;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	int			nBasePathLen = auto_strlen(pszBasePath);
	GrepEnumOptions grepEnumOptions;
	GrepEnumFilterFiles grepEnumFilterFiles;
	grepEnumFilterFiles.Enumerates( pszPath, grepEnumKeys, grepEnumOptions, grepExceptAbsFiles );

	/*
	 * カレントフォルダのファイルを探索する。
	 */
	int count = grepEnumFilterFiles.GetCount();
	for (int i=0; i<count; ++i) {
		LPCTSTR lpFileName = grepEnumFilterFiles.GetFileName(i);

		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(dlgCancel.GetHwnd())) {
			goto cancel_return;
		}
		// 中断ボタン押下チェック
		if (dlgCancel.IsCanceled()) {
			goto cancel_return;
		}

		// 表示設定をチェック
		EditWnd::getInstance().SetDrawSwitchOfAllViews(
			dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
		);

		// GREP実行！
		dlgCancel.SetItemText(IDC_STATIC_CURFILE, lpFileName);

		std::tstring currentFile = pszPath;
		currentFile += _T("\\");
		currentFile += lpFileName;
		int nBasePathLen2 = nBasePathLen + 1;
		if ((int)auto_strlen(pszPath) < nBasePathLen2) {
			nBasePathLen2 = nBasePathLen;
		}

		// ファイル内の検索
		int nRet;
		if (grepOption.bGrepReplace) {
			nRet = DoGrepReplaceFile(
				viewDst,
				dlgCancel,
				pszKey,
				mGrepReplace,
				lpFileName,
				searchOption,
				grepOption,
				pattern,
				regexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(grepOption.bGrepSeparateFolder && grepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(grepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				memMessage
			);
		}else {
			nRet = DoGrepFile(
				viewDst,
				dlgCancel,
				pszKey,
				lpFileName,
				searchOption,
				grepOption,
				pattern,
				regexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(grepOption.bGrepSeparateFolder && grepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(grepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				memMessage
			);
		}

		// 2003.06.23 Moca リアルタイム表示のときは早めに表示
		if (viewDst.GetDrawSwitch()) {
			if (pszKey[0] != LTEXT('\0')) {
				// データ検索のときファイルの合計が最大10MBを超えたら表示
				nWork += (grepEnumFilterFiles.GetFileSizeLow(i) + 1023) / 1024;
			}
			if (*pnHitCount - nHitCountOld && 
				(*pnHitCount < 20 || 10000 < nWork)
			) {
				nHitCountOld = -100; // 即表示
			}
		}
		if (*pnHitCount - nHitCountOld  >= 10) {
			// 結果出力
			if (0 < memMessage.GetStringLength()) {
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND(false);
				if (!EditWnd::getInstance().UpdateTextWrap()) {		// 折り返し方法関連の更新	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews(&viewDst);	//	他のペインの表示を更新
				}
				memMessage.Clear();
			}
			nWork = 0;
			nHitCountOld = *pnHitCount;
		}
		if (nRet == -1) {
			goto cancel_return;
		}
	}

	// 2010.08.25 フォルダ移動前に残りを先に出力
	if (0 < memMessage.GetStringLength()) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
		viewDst.GetCommander().Command_GOFILEEND(false);
		if (!EditWnd::getInstance().UpdateTextWrap()) {		// 折り返し方法関連の更新
			EditWnd::getInstance().RedrawAllViews(&viewDst);	//	他のペインの表示を更新
		}
		memMessage.Clear();
	}

	/*
	 * サブフォルダを検索する。
	 */
	if (grepOption.bGrepSubFolder) {
		GrepEnumOptions grepEnumOptionsDir;
		GrepEnumFilterFolders grepEnumFilterFolders;
		grepEnumFilterFolders.Enumerates( pszPath, grepEnumKeys, grepEnumOptionsDir, grepExceptAbsFolders );

		int count = grepEnumFilterFolders.GetCount();
		for (int i=0; i<count; ++i) {
			LPCTSTR lpFileName = grepEnumFilterFolders.GetFileName(i);

			// サブフォルダの探索を再帰呼び出し。
			// 処理中のユーザー操作を可能にする
			if (!::BlockingHook(dlgCancel.GetHwnd())) {
				goto cancel_return;
			}
			// 中断ボタン押下チェック
			if (dlgCancel.IsCanceled()) {
				goto cancel_return;
			}
			// 表示設定をチェック
			EditWnd::getInstance().SetDrawSwitchOfAllViews(
				dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
			);

			// フォルダ名を作成する。
			// 2010.08.01 キャンセルでメモリーリークしてました
			std::tstring currentPath  = pszPath;
			currentPath += _T("\\");
			currentPath += lpFileName;

			int nGrepTreeResult = DoGrepTree(
				viewDst,
				dlgCancel,
				pszKey,
				mGrepReplace,
				grepEnumKeys,
				grepExceptAbsFiles,
				grepExceptAbsFolders,
				currentPath.c_str(),
				pszBasePath,
				searchOption,
				grepOption,
				pattern,
				regexp,
				nNest + 1,
				bOutputBaseFolder,
				pnHitCount
			);
			if (nGrepTreeResult == -1) {
				goto cancel_return;
			}
			dlgCancel.SetItemText(IDC_STATIC_CURPATH, pszPath);	//@@@ 2002.01.10 add サブフォルダから戻ってきたら...
		}
	}

	dlgCancel.SetItemText(IDC_STATIC_CURFILE, LTEXT(" "));	// 2002/09/09 Moca add
	return 0;

cancel_return:;
	// 結果出力
	if (0 < memMessage.GetStringLength()) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
		viewDst.GetCommander().Command_GOFILEEND(false);
		if (!EditWnd::getInstance().UpdateTextWrap()) {		// 折り返し方法関連の更新
			EditWnd::getInstance().RedrawAllViews(&viewDst);	//	他のペインの表示を更新
		}
		memMessage.Clear();
	}

	return -1;
}


/*!	@brief Grep結果を構築する


	pWorkは充分なメモリ領域を持っているコト
	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
	@date 2013.11.05 Moca memMessageに直接追加するように
*/
void GrepAgent::SetGrepResult(
	// データ格納先
	NativeW& memMessage,
	// マッチしたファイルの情報
	const TCHAR*	pszFilePath,		// [in] フルパス or 相対パス
	const TCHAR*	pszCodeName,		// [in] 文字コード情報．" [SJIS]"とか
	// マッチした行の情報
	LONGLONG	nLine,					// [in] マッチした行番号(1～)
	int			nColumn,				// [in] マッチした桁番号(1～)
	const wchar_t*	pCompareData,		// [in] 行の文字列
	int			nLineLen,				// [in] 行の文字列の長さ
	int			nEolCodeLen,			// [in] EOLの長さ
	// マッチした文字列の情報
	const wchar_t*	pMatchData,			// [in] マッチした文字列
	int			nMatchLen,				// [in] マッチした文字列の長さ
	// オプション
	const GrepOption&	grepOption
	)
{

	NativeW memBuf(L"");
	wchar_t strWork[64];
	const wchar_t* pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr = 0;

	switch (grepOption.nGrepOutputStyle) {
	// ノーマル
	case 1:
		if (grepOption.bGrepOutputBaseFolder || grepOption.bGrepSeparateFolder) {
			memBuf.AppendString(L"・");
		}
		memBuf.AppendStringT(pszFilePath);
		::auto_sprintf( strWork, L"(%I64d,%d)", nLine, nColumn );
		memBuf.AppendString(strWork);
		memBuf.AppendStringT(pszCodeName);
		memBuf.AppendString(L": ");
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
		break;
	// WZ風
	case 2:
		::auto_sprintf( strWork, L"・(%6I64d,%-5d): ", nLine, nColumn );
		memBuf.AppendString(strWork);
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
		break;
	// 結果のみ
	case 3:
		nMaxOutStr = 2500;
		break;
	}

	// 該当行
	if (grepOption.nGrepOutputLineType != 0) {
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if (nMaxOutStr < k) {
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
	// 該当部分
	}else {
		pDispData = pMatchData;
		k = nMatchLen;
		if (nMaxOutStr < k) {
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
		// 該当部分に改行を含む場合はその改行コードをそのまま利用する(次の行に空行を作らない)
		// 2003.06.10 Moca k==0のときにバッファアンダーランしないように
		if (0 < k && WCODE::IsLineDelimiter(pMatchData[ k - 1 ], GetDllShareData().common.edit.bEnableExtEol)) {
			bEOL = false;
		}
	}

	memMessage.AllocStringBuffer(memMessage.GetStringLength() + memBuf.GetStringLength() + 2);
	memMessage.AppendNativeData(memBuf);
	memMessage.AppendString(pDispData, k);
	if (bEOL) {
		memMessage.AppendString(L"\r\n", 2);
	}
}

static void OutputPathInfo(
	NativeW&		memMessage,
	GrepOption		grepOption,
	const TCHAR*	pszFullPath,
	const TCHAR*	pszBaseFolder,
	const TCHAR*	pszFolder,
	const TCHAR*	pszRelPath,
	const TCHAR*	pszCodeName,
	bool&			bOutputBaseFolder,
	bool&			bOutputFolderName,
	BOOL&			bOutFileName
	)
{
	{
		// バッファを2^n 分確保する
		int n = 1024;
		int size = memMessage.GetStringLength() + 300;
		while (n < size) {
			n *= 2;
		}
		memMessage.AllocStringBuffer(n);
	}
	if (grepOption.nGrepOutputStyle == 3) {
		return;
	}

	if (!bOutputBaseFolder && grepOption.bGrepOutputBaseFolder) {
		if (!grepOption.bGrepSeparateFolder && grepOption.nGrepOutputStyle == 1) {
			memMessage.AppendString(L"■\"");
		}else {
			memMessage.AppendString(L"◎\"");
		}
		memMessage.AppendStringT(pszBaseFolder);
		memMessage.AppendString(L"\"\r\n");
		bOutputBaseFolder = true;
	}
	if (!bOutputFolderName && grepOption.bGrepSeparateFolder) {
		if (pszFolder[0]) {
			memMessage.AppendString(L"■\"");
			memMessage.AppendStringT(pszFolder);
			memMessage.AppendString(L"\"\r\n");
		}else {
			memMessage.AppendString(L"■\r\n");
		}
		bOutputFolderName = true;
	}
	if (grepOption.nGrepOutputStyle == 2) {
		if (!bOutFileName) {
			const TCHAR* pszDispFilePath = (grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder) ? pszRelPath : pszFullPath;
			if (grepOption.bGrepSeparateFolder) {
				memMessage.AppendString(L"◆\"");
			}else {
				memMessage.AppendString(L"■\"");
			}
			memMessage.AppendStringT(pszDispFilePath);
			memMessage.AppendString(L"\"");
			memMessage.AppendStringT(pszCodeName);
			memMessage.AppendString(L"\r\n");
			bOutFileName = TRUE;
		}
	}
}

/*!
	Grep実行 (FileLoadを使ったテスト版)

	@retval -1 GREPのキャンセル
	@retval それ以外 ヒット数(ファイル検索時はファイル数)

	@date 2001/06/27 genta	正規表現ライブラリの差し替え
	@date 2002/08/30 Moca FileLoadを使ったテスト版
	@date 2004/03/28 genta 不要な引数nNest, bGrepSubFolder, pszPathを削除
*/
int GrepAgent::DoGrepFile(
	EditView&				viewDst,			// 
	DlgCancel&				dlgCancel,			// [in] Cancelダイアログへのポインタ
	const wchar_t*			pszKey,				// [in] 検索パターン
	const TCHAR*			pszFile,			// [in] 処理対象ファイル名(表示用)
	const SearchOption&		searchOption,		// [in] 検索オプション
	const GrepOption&		grepOption,			// [in] Grepオプション
	const SearchStringPattern& pattern,			// [in] 検索パターン
	Bregexp&				regexp,			// [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int*					pnHitCount,			// [i/o] ヒット数の合計．元々の値に見つかった数を加算して返す．
	const TCHAR*			pszFullPath,		// [in] 処理対象ファイルパス C:\Folder\SubFolder\File.ext
	const TCHAR*			pszBaseFolder,		// [in] 検索フォルダ C:\Folder
	const TCHAR*			pszFolder,			// [in] サブフォルダ SubFolder (!bGrepSeparateFolder) または C:\Folder\SubFolder (!bGrepSeparateFolder)
	const TCHAR*			pszRelPath,			// [in] 相対パス File.ext(bGrepSeparateFolder) または  SubFolder\File.ext(!bGrepSeparateFolder)
	bool&					bOutputBaseFolder,	// 
	bool&					bOutputFolderName,	// 
	NativeW&				memMessage			// 
	)
{
	int		nHitCount;
	LONGLONG	nLine;
	const wchar_t*	pszRes; // 2002/08/29 const付加
	EncodingType	nCharCode;
	const wchar_t*	pCompareData; // 2002/08/29 const付加
	int		nColumn;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	Eol	eol;
	int		nEolCodeLen;
	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini(DocTypeManager().GetDocumentTypeOfPath(pszFile), &type);
	FileLoad	fl(type->encoding);	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	int		nOldPercent = 0;

	int	nKeyLen = wcslen(pszKey);
	// ファイル名表示
	const TCHAR* pszDispFilePath = (grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder) ? pszRelPath : pszFullPath;

	//	ここでは正規表現コンパイルデータの初期化は不要
	const TCHAR*	pszCodeName; // 2002/08/29 const付加
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	// 検索条件が長さゼロの場合はファイル名だけ返す
	// 2002/08/29 行ループの前からここに移動
	if (nKeyLen == 0) {
		TCHAR szCpName[100];
		if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
			// 2003.06.10 Moca コード判別処理をここに移動．
			// 判別エラーでもファイル数にカウントするため
			// ファイルの日本語コードセット判別
			// 2014.06.19 Moca ファイル名のタイプ別のm_encodingに変更
			CodeMediator mediator( type->encoding );
			nCharCode = mediator.CheckKanjiCodeOfFile(pszFullPath);
			if (!IsValidCodeOrCPType(nCharCode)) {
				pszCodeName = _T("  [(DetectError)]");
			}else if (IsValidCodeType(nCharCode)) {
				pszCodeName = CodeTypeName(nCharCode).Bracket();
			}else {
				CodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}
		{
			const wchar_t* pszFormatFullPath = L"";
			const wchar_t* pszFormatBasePath2 = L"";
			const wchar_t* pszFormatFilePath = L"";
			const wchar_t* pszFormatFilePath2 = L"";
			if (grepOption.nGrepOutputStyle == 1) {
				// ノーマル
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"■\"%ts\"\r\n";
				pszFormatFilePath   = L"・\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"・\"%ts\"%ts\r\n";
			}else if (grepOption.nGrepOutputStyle == 2) {
				// WZ風
				pszFormatFullPath   = L"■\"%ts\"%ts\r\n";
				pszFormatBasePath2  = L"◎\"%ts\"\r\n";
				pszFormatFilePath   = L"◆\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"■\"%ts\"%ts\r\n";
			}else if (grepOption.nGrepOutputStyle == 3) {
				// 結果のみ
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"■\"%ts\"\r\n";
				pszFormatFilePath   = L"%ts\r\n";
				pszFormatFilePath2  = L"%ts\r\n";
			}
/*
			Base/Sep
			O / O  : (A)BaseFolder -> (C)Folder(Rel) -> (E)RelPath(File)
			O / X  : (B)BaseFolder ->                   (F)RelPath(RelFolder/File)
			X / O  :                  (D)Folder(Abs) -> (G)RelPath(File)
			X / X  : (H)FullPath
*/
			auto pszWork = std::make_unique<wchar_t[]>(auto_strlen(pszFullPath) + auto_strlen(pszCodeName) + 10);
			wchar_t* szWork0 = &pszWork[0];
			if (grepOption.bGrepOutputBaseFolder || grepOption.bGrepSeparateFolder) {
				if (!bOutputBaseFolder && grepOption.bGrepOutputBaseFolder) {
					const wchar_t* pszFormatBasePath = L"";
					if (grepOption.bGrepSeparateFolder) {
						pszFormatBasePath = L"◎\"%ts\"\r\n";	// (A)
					}else {
						pszFormatBasePath = pszFormatBasePath2;	// (B)
					}
					auto_sprintf(szWork0, pszFormatBasePath, pszBaseFolder);
					memMessage.AppendString(szWork0);
					bOutputBaseFolder = true;
				}
				if (!bOutputFolderName && grepOption.bGrepSeparateFolder) {
					if (pszFolder[0]) {
						auto_sprintf(szWork0, L"■\"%ts\"\r\n", pszFolder);	// (C), (D)
					}else {
						auto_strcpy(szWork0, L"■\r\n");
					}
					memMessage.AppendString(szWork0);
					bOutputFolderName = true;
				}
				auto_sprintf(szWork0,
					(grepOption.bGrepSeparateFolder ? pszFormatFilePath // (E)
						: pszFormatFilePath2),	// (F), (G)
					pszDispFilePath, pszCodeName);
				memMessage.AppendString(szWork0);
			}else {
				auto_sprintf(szWork0, pszFormatFullPath, pszFullPath, pszCodeName);	// (H)
				memMessage.AppendString(szWork0);
			}
		}
		++(*pnHitCount);
		dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
		return 1;
	}

	try {
		// ファイルを開く
		// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
		// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
		nCharCode = fl.FileOpen( pszFullPath, true, grepOption.nGrepCharSet, GetDllShareData().common.file.GetAutoMIMEdecode() );
		TCHAR szCpName[100];
		{
			if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
				if (IsValidCodeType(nCharCode)) {
					auto_strcpy( szCpName, CodeTypeName(nCharCode).Bracket() );
					pszCodeName = szCpName;
				}else {
					CodePage::GetNameBracket(szCpName, nCharCode);
					pszCodeName = szCpName;
				}
			}
		}

	//	// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(dlgCancel.GetHwnd())) {
			return -1;
		}
		// 中断ボタン押下チェック
		if (dlgCancel.IsCanceled()) {
			return -1;
		}
		int nOutputHitCount = 0;

		// 検索条件が長さゼロの場合はファイル名だけ返す
		// 2002/08/29 ファイルオープンの手前へ移動
		
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords;
		if (searchOption.bWordOnly) {
			SearchAgent::CreateWordList(searchWords, pszKey, nKeyLen);
		}

		// 注意 : fl.ReadLine が throw する可能性がある
		NativeW unicodeBuffer;
		while (CodeConvertResult::Failure != fl.ReadLine(&unicodeBuffer, &eol)) {
			const wchar_t*	pLine = unicodeBuffer.GetStringPtr();
			int		nLineLen = unicodeBuffer.GetStringLength();

			nEolCodeLen = eol.GetLen();
			++nLine;
			pCompareData = pLine;

			// 処理中のユーザー操作を可能にする
			// 2010.08.31 間隔を1/32にする
			if (((nLine%32 == 0) || nLineLen > 10000) && !::BlockingHook(dlgCancel.GetHwnd())) {
				return -1;
			}
			if (nLine%64 == 0) {
				// 中断ボタン押下チェック
				if (dlgCancel.IsCanceled()) {
					return -1;
				}
				//	2003.06.23 Moca 表示設定をチェック
				EditWnd::getInstance().SetDrawSwitchOfAllViews(
					dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
				);
				// 2002/08/30 Moca 進行状態を表示する(5MB以上)
				if (5000000 < fl.GetFileSize()) {
					int nPercent = fl.GetPercent();
					if (5 <= nPercent - nOldPercent) {
						nOldPercent = nPercent;
						TCHAR szWork[10];
						::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
						std::tstring str;
						str = str + pszFile + szWork;
						dlgCancel.SetItemText(IDC_STATIC_CURFILE, str.c_str());
					}
				}
			}
			int nHitOldLine = nHitCount;
			int nHitCountOldLine = *pnHitCount;

			// 正規表現検索
			if (searchOption.bRegularExp) {
				int nIndex = 0;
	#ifdef _DEBUG
				int nIndexPrev = -1;
	#endif

				//	Jun. 21, 2003 genta ループ条件見直し
				//	マッチ箇所を1行から複数検出するケースを標準に，
				//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
				//	ループ継続・打ち切り条件(nGrepOutputLineType)を逆にした．
				//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
				// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
				// 2010.08.25 行頭以外で^にマッチする不具合の修正
				while (nIndex <= nLineLen && regexp.Match(pLine, nLineLen, nIndex)) {

					// パターン発見
					nIndex = regexp.GetIndex();
					int matchlen = regexp.GetMatchLen();
#ifdef _DEBUG
					if (nIndex <= nIndexPrev) {
						MYTRACE(_T("ERROR: EditView::DoGrepFile() nIndex <= nIndexPrev break \n"));
						break;
					}
					nIndexPrev = nIndex;
#endif
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {

						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							nLine,
							nIndex + 1,
							pLine,
							nLineLen,
							nEolCodeLen,
							pLine + nIndex,
							matchlen,
							grepOption
						);
						if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
					// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if ( grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly ) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					if (matchlen <= 0) {
						matchlen = NativeW::GetSizeOfChar(pLine, nLineLen, nIndex);
						if (matchlen <= 0) {
							matchlen = 1;
						}
					}
					nIndex += matchlen;
				}
			// 単語のみ検索
			}else if (searchOption.bWordOnly) {
				/*
					2002/02/23 Norio Nakatani
					単語単位のGrepを試験的に実装。単語はWhereCurrentWord()で判別してますので、
					英単語やC/C++識別子などの検索条件ならヒットします。

					2002/03/06 YAZAKI
					Grepにも試験導入。
					WhereCurrentWordで単語を抽出して、その単語が検索語とあっているか比較する。
				*/
				int nMatchLen;
				int nIdx = 0;
				// Jun. 26, 2003 genta 無駄なwhileは削除
				while ((pszRes = SearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, searchOption.bLoHiCase, &nMatchLen))) {
					nIdx = pszRes - pLine + nMatchLen;
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							//	Jun. 25, 2002 genta
							//	桁位置は1始まりなので1を足す必要がある
							nLine,
							pszRes - pLine + 1,
							pLine,
							nLineLen,
							nEolCodeLen,
							pszRes,
							nMatchLen,
							grepOption
						);
						//	May 22, 2000 genta
						if (((*pnHitCount)%128)==0 || *pnHitCount<128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
	
					// 2010.10.31 ryoji 行単位で出力する場合は1つ見つかれば十分
					if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
						break;
					}
				}
			}else {
				// 文字列検索
				int nColumnPrev = 0;
				//	Jun. 21, 2003 genta ループ条件見直し
				//	マッチ箇所を1行から複数検出するケースを標準に，
				//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
				//	ループ継続・打ち切り条件(nGrepOutputLineType)を逆にした．
				for (;;) {
					pszRes = SearchAgent::SearchString(
						pCompareData,
						nLineLen,
						0,
						pattern
					);
					if (!pszRes) {
						break;
					}

					nColumn = pszRes - pCompareData + 1;
	
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							nLine,
							nColumn + nColumnPrev,
							pCompareData,
							nLineLen,
							nEolCodeLen,
							pszRes,
							nKeyLen,
							grepOption
						);
						//	May 22, 2000 genta
						if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
						
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	nClom : マッチ位置
					//	matchlen : マッチした文字列の長さ
					int nPosDiff = nColumn += nKeyLen - 1;
					pCompareData += nPosDiff;
					nLineLen -= nPosDiff;
					nColumnPrev += nPosDiff;
				}
			}
			// 2014.09.23 否ヒット行を出力
			if (grepOption.nGrepOutputLineType == 2) {
				bool bNoHit = nHitOldLine == nHitCount;
				// ヒット数を戻す
				nHitCount = nHitOldLine;
				*pnHitCount = nHitCountOldLine;
				// 否ヒット行だった
				if (bNoHit) {
					++nHitCount;
					(*pnHitCount)++;
					OutputPathInfo(
						memMessage, grepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					SetGrepResult(
						memMessage, pszDispFilePath, pszCodeName,
						nLine, 1, pLine, nLineLen, nEolCodeLen,
						pLine, nLineLen, grepOption
					);
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
				}
			}
			// 2014.09.23 データが多い時はバッファ出力
			if (0 < memMessage.GetStringLength() && 2800 < nHitCount - nOutputHitCount) {
				nOutputHitCount = nHitCount;
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND( FALSE );
				if (!EditWnd::getInstance().UpdateTextWrap()) {	// 折り返し方法関連の更新	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews( &viewDst );	//	他のペインの表示を更新
				}
				memMessage._SetStringLength(0);
			}

			// ファイル検索の場合は、1つ見つかったら終了
			if (grepOption.bGrepOutputFileOnly && 1 <= nHitCount) {
				break;
			}
		}

		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		fl.FileClose();
	} // try
	catch (Error_FileOpen) {
		NativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData( str );
		return 0;
	}
	catch (Error_FileRead) {
		NativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}

class Error_WriteFileOpen
{
public:
	virtual ~Error_WriteFileOpen(){}
};

class WriteData {
public:
	WriteData(int& hit,
				LPCTSTR name,
				EncodingType code_,
				bool bBom_,
				bool bOldSave_,
				NativeW& message)
		:
		nHitCount(hit),
		fileName(name),
		code(code_),
		bBom(bBom_),
		bOldSave(bOldSave_),
		bufferSize(0),
		out(NULL),
		pCodeBase(CodeFactory::CreateCodeBase(code_,0)),
		memMessage(message)
	{
	}
	void AppendBuffer(const NativeW& strLine)
	{
		if (!out) {
			bufferSize += strLine.GetStringLength();
			buffer.push_back(strLine);
			// 10MB 以上だったら出力してしまう
			if (0xa00000 <= bufferSize) {
				OutputHead();
			}
		}else {
			Output(strLine);
		}
	}
	void OutputHead()
	{
		if (!out) {
			std::tstring name = fileName;
			name += _T(".skrnew");
			try {
				out = new BinaryOutputStream(name.c_str(), true);
			}catch (Error_FileOpen) {
				throw Error_WriteFileOpen();
			}
			if (bBom) {
				Memory bom;
				pCodeBase->GetBom(&bom);
				out->Write(bom.GetRawPtr(), bom.GetRawLength());
			}
			for (size_t i=0; i<buffer.size(); ++i) {
				Output(buffer[i]);
			}
			buffer.clear();
			std::deque<NativeW>().swap(buffer);
		}
	}
	void Output(const NativeW& strLine)
	{
		Memory dest;
		pCodeBase->UnicodeToCode(strLine, &dest);
		// 場合によっては改行ごとではないので、JIS/UTF-7での出力が一定でない可能性あり
		out->Write(dest.GetRawPtr(), dest.GetRawLength());
	}
	void Close()
	{
		if (nHitCount && out) {
			out->Close();
			delete out;
			out = NULL;
			if (bOldSave) {
				std::tstring oldFile = fileName;
				oldFile += _T(".skrold");
				if (fexist(oldFile.c_str())) {
					if (::DeleteFile(oldFile.c_str()) == FALSE) {
						std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
						msg += L"[";
						msg += to_wchar(oldFile.c_str());
						msg += L"]\r\n";
						memMessage.AppendString( msg.c_str() );
						return;
					}
				}
				if (::MoveFile(fileName, oldFile.c_str()) == FALSE) {
					std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
					msg += L"[";
					msg += to_wchar(oldFile.c_str());
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}else {
				if (::DeleteFile(fileName) == FALSE) {
					std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
					msg += L"[";
					msg += to_wchar(fileName);
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			if (::MoveFile(name.c_str(), fileName) == FALSE) {
				std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
				msg += L"[";
				msg += to_wchar(fileName);
				msg += L"]\r\n";
				memMessage.AppendString( msg.c_str()  );
				return;
			}
		}
		return;
	}
	~WriteData()
	{
		if (out) {
			out->Close();
			delete out;
			out = NULL;
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			::DeleteFile( name.c_str() );
		}
	}
private:
	int& nHitCount;
	LPCTSTR fileName;
	EncodingType code;
	bool bBom;
	bool bOldSave;
	size_t bufferSize;
	std::deque<NativeW> buffer;
	BinaryOutputStream* out;
	std::unique_ptr<CodeBase> pCodeBase;
	NativeW&	memMessage;
};

/*!
	Grep置換実行
	@date 2013.06.12 Moca 新規作成
*/
int GrepAgent::DoGrepReplaceFile(
	EditView&				viewDst,
	DlgCancel&				dlgCancel,
	const wchar_t*			pszKey,
	const NativeW&			mGrepReplace,
	const TCHAR*			pszFile,
	const SearchOption&		searchOption,
	const GrepOption&		grepOption,
	const SearchStringPattern& pattern,
	Bregexp&				regexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	int*					pnHitCount,
	const TCHAR*			pszFullPath,
	const TCHAR*			pszBaseFolder,
	const TCHAR*			pszFolder,
	const TCHAR*			pszRelPath,
	bool&					bOutputBaseFolder,
	bool&					bOutputFolderName,
	NativeW&				memMessage
	)
{
	LONGLONG	nLine = 0;
	int			nHitCount = 0;
	EncodingType	nCharCode;
	BOOL		bOutFileName = FALSE;
	Eol		cEol;
	int		nEolCodeLen;
	int		nOldPercent = 0;
	int		nKeyLen = wcslen( pszKey );
	const TCHAR*	pszCodeName = _T("");

	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini( DocTypeManager().GetDocumentTypeOfPath( pszFile ), &type );
	FileLoad fl( type->encoding );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	bool bBom;
	// ファイル名表示
	const TCHAR* pszDispFilePath = ( grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;


	try {
		// ファイルを開く
		// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
		// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
		nCharCode = fl.FileOpen( pszFullPath, true, grepOption.nGrepCharSet, GetDllShareData().common.file.GetAutoMIMEdecode(), &bBom );
		WriteData output(nHitCount, pszFullPath, nCharCode, bBom, grepOption.bGrepBackup, memMessage );
		TCHAR szCpName[100];
		{
			if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
				if (IsValidCodeType(nCharCode)) {
					auto_strcpy( szCpName, CodeTypeName(nCharCode).Bracket() );
					pszCodeName = szCpName;
				}else {
					CodePage::GetNameBracket(szCpName, nCharCode);
					pszCodeName = szCpName;
				}
			}
		}
		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook( dlgCancel.GetHwnd() )) {
			return -1;
		}
		// 中断ボタン押下チェック
		if (dlgCancel.IsCanceled()) {
			return -1;
		}
		int nOutputHitCount = 0;
	
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords;
		if (searchOption.bWordOnly) {
			SearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
		}
	
		NativeW outBuffer;
		// 注意 : fl.ReadLine が throw する可能性がある
		NativeW unicodeBuffer;
		while (fl.ReadLine(&unicodeBuffer, &cEol) != CodeConvertResult::Failure) {
			const wchar_t*	pLine = unicodeBuffer.GetStringPtr();
			int nLineLen = unicodeBuffer.GetStringLength();
	
			nEolCodeLen = cEol.GetLen();
			++nLine;
	
			// 処理中のユーザー操作を可能にする
			// 2010.08.31 間隔を1/32にする
			if (((nLine%32 == 0)|| 10000 < nLineLen ) && !::BlockingHook( dlgCancel.GetHwnd() )) {
				return -1;
			}
			if (nLine%64 == 0) {
				// 中断ボタン押下チェック
				if (dlgCancel.IsCanceled()) {
					return -1;
				}
				//	2003.06.23 Moca 表示設定をチェック
				EditWnd::getInstance().SetDrawSwitchOfAllViews(
					dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
				);
				// 2002/08/30 Moca 進行状態を表示する(5MB以上)
				if (5000000 < fl.GetFileSize()) {
					int nPercent = fl.GetPercent();
					if (5 <= nPercent - nOldPercent) {
						nOldPercent = nPercent;
						TCHAR szWork[10];
						::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
						std::tstring str;
						str = str + pszFile + szWork;
						dlgCancel.SetItemText(IDC_STATIC_CURFILE, str.c_str() );
					}
				}
			}
			outBuffer.SetString( L"", 0 );
			bool bOutput = true;
			if (grepOption.bGrepOutputFileOnly && 1 <= nHitCount) {
				bOutput = false;
			}
	
			// 正規表現検索
			if (searchOption.bRegularExp) {
				int nIndex = 0;
				int nIndexOld = nIndex;
				int nMatchNum = 0;
				//	Jun. 21, 2003 genta ループ条件見直し
				//	マッチ箇所を1行から複数検出するケースを標準に，
				//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
				//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
				//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
				// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
				// 2010.08.25 行頭以外で^にマッチする不具合の修正
				while (
					nIndex <= nLineLen
					&& (  (!grepOption.bGrepPaste && (nMatchNum = regexp.Replace(pLine, nLineLen, nIndex)))
						|| (grepOption.bGrepPaste && regexp.Match(pLine, nLineLen, nIndex))
					)
				) {
					//	パターン発見
					nIndex = regexp.GetIndex();
					int matchlen = regexp.GetMatchLen();
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep結果を、memMessageに格納する
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							nLine, nIndex + 1,
							pLine, nLineLen, nEolCodeLen,
							pLine + nIndex, matchlen,
							grepOption
						);
						// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (!grepOption.bGrepPaste) {
						// gオプションでは行末まで一度に置換済み
						nHitCount += nMatchNum - 1;
						*pnHitCount += nMatchNum - 1;
						outBuffer.AppendString( regexp.GetString(), regexp.GetStringLen() );
						nIndexOld = nLineLen;
						break;
					}
					if (0 < nIndex - nIndexOld) {
						outBuffer.AppendString( &pLine[nIndexOld], nIndex - nIndexOld );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					if (matchlen <= 0) {
						matchlen = NativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
						if (matchlen <= 0) {
							matchlen = 1;
						}
					}
					nIndex += matchlen;
					nIndexOld = nIndex;
				}
				if (0 < nLineLen - nIndexOld) {
					outBuffer.AppendString( &pLine[nIndexOld], nLineLen - nIndexOld );
				}
			}
			// 単語のみ検索
			else if (searchOption.bWordOnly) {
				/*
					2002/02/23 Norio Nakatani
					単語単位のGrepを試験的に実装。単語はWhereCurrentWord()で判別してますので、
					英単語やC/C++識別子などの検索条件ならヒットします。
	
					2002/03/06 YAZAKI
					Grepにも試験導入。
					WhereCurrentWordで単語を抽出して、その単語が検索語とあっているか比較する。
				*/
				const wchar_t* pszRes;
				int nMatchLen;
				int nIdx = 0;
				int nOutputPos = 0;
				// Jun. 26, 2003 genta 無駄なwhileは削除
				while (pszRes = SearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, searchOption.bLoHiCase, &nMatchLen)) {
					nIdx = pszRes - pLine + nMatchLen;
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep結果を、memMessageに格納する
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							//	Jun. 25, 2002 genta
							//	桁位置は1始まりなので1を足す必要がある
							nLine, pszRes - pLine + 1, pLine, nLineLen, nEolCodeLen,
							pszRes, nMatchLen,
							grepOption
						);
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (0 < pszRes - pLine - nOutputPos) {
						outBuffer.AppendString( &pLine[nOutputPos], pszRes - pLine - nOutputPos );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					nOutputPos = pszRes - pLine + nMatchLen;
				}
				outBuffer.AppendString( &pLine[nOutputPos], nLineLen - nOutputPos );
			}else {
				// 文字列検索
				int nColumnPrev = 0;
				const wchar_t*	pCompareData = pLine;
				int nCompareLen = nLineLen;
				//	Jun. 21, 2003 genta ループ条件見直し
				//	マッチ箇所を1行から複数検出するケースを標準に，
				//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
				//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
				for (;;) {
					const wchar_t* pszRes = SearchAgent::SearchString( pCompareData, nCompareLen, 0, pattern );
					if (!pszRes) {
						break;
					}
	
					int	nColumn = pszRes - pCompareData;
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep結果を、memMessageに格納する
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							nLine, nColumn + nColumnPrev + 1, pLine, nLineLen, nEolCodeLen,
							pszRes, nKeyLen,
							grepOption
						);
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (nColumn) {
						outBuffer.AppendString( pCompareData, nColumn );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	nClom : マッチ位置
					//	matchlen : マッチした文字列の長さ
					int nPosDiff = nColumn + nKeyLen;
					pCompareData += nPosDiff;
					nCompareLen -= nPosDiff;
					nColumnPrev += nPosDiff;
				}
				outBuffer.AppendString( &pLine[nColumnPrev], nLineLen - nColumnPrev );
			}
			output.AppendBuffer(outBuffer);
	
			// 2014.09.23 データが多い時はバッファ出力
			if (0 < memMessage.GetStringLength() && 2800 < nHitCount - nOutputHitCount) {
				nOutputHitCount = nHitCount;
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND( false );
				if (!EditWnd::getInstance().UpdateTextWrap())	// 折り返し方法関連の更新	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews( &viewDst );	//	他のペインの表示を更新
				memMessage._SetStringLength(0);
			}
		}
	
		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		fl.FileClose();
		output.Close();
	} // try
	catch( Error_FileOpen ){
		NativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData(str);
		return 0;
	}catch (Error_FileRead) {
		NativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData(str);
	}catch (Error_WriteFileOpen) {
		std::tstring file = pszFullPath;
		file += _T(".skrnew");
		NativeW str(LSW(STR_GREP_ERR_FILEWRITE));
		str.Replace(L"%ts", to_wchar(file.c_str()));
		memMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}


