// 2008.XX.XX kobake ShareDataから分離
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "env/ShareData_IO.h"
#include "doc/DocTypeSetting.h" // ColorInfo !!
#include "ShareData.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "view/colors/ColorStrategy.h"
#include "plugin/Plugin.h"
#include "uiparts/MenuDrawer.h"
#include "_main/CommandLine.h"
#include "debug/RunningTimer.h"

void ShareData_IO_Sub_LogFont(DataProfile& profile, const WCHAR* pszSecName,
	const WCHAR* pszKeyLf, const WCHAR* pszKeyPointSize, const WCHAR* pszKeyFaceName, LOGFONT& lf, INT& nPointSize);

template <typename T>
void SetValueLimit(T& target, int minval, int maxval)
{
	target = t_max<T>(minval, t_min<T>(maxval, target));
}

template <typename T>
void SetValueLimit(T& target, int maxval)
{
	SetValueLimit(target, 0, maxval);
}

// 共有データのロード
bool ShareData_IO::LoadShareData()
{
	return ShareData_IO_2(true);
}

// 共有データの保存
void ShareData_IO::SaveShareData()
{
	ShareData_IO_2(false);
}

/*!
	共有データの読み込み/保存 2

	@param[in] bRead true: 読み込み / false: 書き込み

	@date 2004-01-11 D.S.Koba Profile変更によるコード簡略化
	@date 2005-04-05 D.S.Koba 各セクションの入出力を関数として分離
*/
bool ShareData_IO::ShareData_IO_2(bool bRead)
{
//	MY_RUNNINGTIMER(runningTimer, "ShareData_IO::ShareData_IO_2");
	auto& shareData = ShareData::getInstance();

	DataProfile	profile;

	// Feb. 12, 2006 D.S.Koba
	if (bRead) {
		profile.SetReadingMode();
	}else {
		profile.SetWritingMode();
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	TCHAR szIniFileName[_MAX_PATH + 1];
	FileNameManager::getInstance().GetIniFileName( szIniFileName, strProfileName.c_str(), bRead );	// 2007.05.19 ryoji iniファイル名を取得する

//	MYTRACE(_T("Iniファイル処理-1 所要時間(ミリ秒) = %d\n"), runningTimer.Read());

	if (bRead) {
		if (!profile.ReadProfile(szIniFileName)) {
			// 設定ファイルが存在しない
			return false;
		}

		// バージョンアップ時はバックアップファイルを作成する	// 2011.01.28 ryoji
		TCHAR iniVer[256];
		DWORD mH, mL, lH, lL;
		mH = mL = lH = lL = 0;	// ※ 古～い ini だと "szVersion" は無い
		if (profile.IOProfileData(LTEXT("Other"), LTEXT("szVersion"), MakeStringBufferT(iniVer)))
			_stscanf(iniVer, _T("%u.%u.%u.%u"), &mH, &mL, &lH, &lL);
		DWORD dwMS = (DWORD)MAKELONG(mL, mH);
		DWORD dwLS = (DWORD)MAKELONG(lL, lH);
		DllSharedData* pShareData = &GetDllShareData();
		if (0
			|| pShareData->version.dwProductVersionMS > dwMS
			|| (0
				&& pShareData->version.dwProductVersionMS == dwMS
				&& pShareData->version.dwProductVersionLS > dwLS
			)
		) {
			TCHAR szBkFileName[_countof(szIniFileName) + 4];
			::lstrcpy(szBkFileName, szIniFileName);
			::lstrcat(szBkFileName, _T(".bak"));
			::CopyFile(szIniFileName, szBkFileName, FALSE);
		}
	}
//	MYTRACE(_T("Iniファイル処理 0 所要時間(ミリ秒) = %d\n"), runningTimer.Read());

	auto menuDrawer = std::make_unique<MenuDrawer>(); // 2010/7/4 Uchi

	if (bRead) {
		DllSharedData* pShareData = &GetDllShareData();
		profile.IOProfileData(L"Common", L"m_szLanguageDll", MakeStringBufferT(pShareData->common.window.szLanguageDll));
		SelectLang::ChangeLang(pShareData->common.window.szLanguageDll);
		shareData.RefreshString();
	}

	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru(profile);
	ShareData_IO_Keys(profile);
	ShareData_IO_Grep(profile);
	ShareData_IO_Folders(profile);
	ShareData_IO_Cmd(profile);
	ShareData_IO_Nickname(profile);
	ShareData_IO_Common(profile);
	ShareData_IO_Plugin(profile, menuDrawer.get());		// Move here	2010/6/24 Uchi
	ShareData_IO_Toolbar(profile, menuDrawer.get());
	ShareData_IO_CustMenu(profile);
	ShareData_IO_Font(profile);
	ShareData_IO_KeyBind(profile);
	ShareData_IO_Print(profile);
	ShareData_IO_Types(profile);
	ShareData_IO_Keywords(profile);
	ShareData_IO_Macro(profile);
	ShareData_IO_Statusbar(profile);		// 2008/6/21 Uchi
	ShareData_IO_MainMenu(profile);		// 2010/5/15 Uchi
	ShareData_IO_Other(profile);

	if (!bRead) {
		profile.WriteProfile( szIniFileName, LTEXT(" sakura.ini テキストエディタ設定ファイル") );
	}

//	MYTRACE(_T("Iniファイル処理 8 所要時間(ミリ秒) = %d\n"), runningTimer.Read());

	return true;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Mru(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("MRU");
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	WCHAR		szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_MRU_Counts"), pShare->history.nMRUArrNum);
	SetValueLimit(pShare->history.nMRUArrNum, MAX_MRU);
	nSize = pShare->history.nMRUArrNum;
	for (i=0; i<nSize; ++i) {
		pfiWork = &pShare->history.fiMRUArr[i];
		if (profile.IsReadingMode()) {
			pfiWork->nTypeId = -1;
		}
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewTopLine"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->nViewTopLine);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewLeftCol"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->nViewLeftCol);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nX"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->ptCursor.x);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nY"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->ptCursor.y);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nCharCode"), i );
		profile.IOProfileData_WrapInt(pszSecName, szKeyName, pfiWork->nCharCode);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szPath"), i );
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pfiWork->szPath));
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark2"), i );
		if (!profile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pfiWork->szMarkLines) )) {
			if (profile.IsReadingMode()) {
				auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark"), i ); // 旧ver互換
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pfiWork->szMarkLines));
			}
		}
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nType"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->nTypeId);
		// お気に入り	//@@@ 2003.04.08 MIK
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].bFavorite"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.bMRUArrFavorite[i]);
	}
	//@@@ 2001.12.26 YAZAKI 残りのfiMRUArrを初期化。
	if (profile.IsReadingMode()) {
		EditInfo	fiInit;
		// 残りをfiInitで初期化しておく。
		fiInit.nCharCode = CODE_DEFAULT;
		fiInit.nViewLeftCol = 0;
		fiInit.nViewTopLine = 0;
		fiInit.ptCursor.Set(0, 0);
		_tcscpy( fiInit.szPath, _T("") );
		fiInit.szMarkLines[0] = L'\0';	// 2002.01.16 hor
		for (; i<MAX_MRU; ++i) {
			pShare->history.fiMRUArr[i] = fiInit;
			pShare->history.bMRUArrFavorite[i] = false;	// お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	profile.IOProfileData(pszSecName, LTEXT("_MRUFOLDER_Counts"), pShare->history.nOPENFOLDERArrNum);
	SetValueLimit(pShare->history.nOPENFOLDERArrNum, MAX_OPENFOLDER);
	nSize = pShare->history.nOPENFOLDERArrNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf( szKeyName, LTEXT("MRUFOLDER[%02d]"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.szOPENFOLDERArr[i]);
		// お気に入り	//@@@ 2003.04.08 MIK
		wcscat(szKeyName, LTEXT(".bFavorite"));
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.bOPENFOLDERArrFavorite[i]);
	}
	// 読み込み時は残りを初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_OPENFOLDER; ++i) {
			// 2005.04.05 D.S.Koba
			pShare->history.szOPENFOLDERArr[i][0] = L'\0';
			pShare->history.bOPENFOLDERArrFavorite[i] = false;	// お気に入り	//@@@ 2003.04.08 MIK
		}
	}
	
	profile.IOProfileData(pszSecName, LTEXT("_ExceptMRU_Counts"), pShare->history.aExceptMRU._GetSizeRef());
	pShare->history.aExceptMRU.SetSizeLimit();
	nSize = pShare->history.aExceptMRU.size();
	for (i=0; i<nSize; ++i) {
		auto_sprintf( szKeyName, LTEXT("ExceptMRU[%02d]"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.aExceptMRU[i]);
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Keys(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Keys");
	WCHAR szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_SEARCHKEY_Counts"), pShare->searchKeywords.searchKeys._GetSizeRef());
	pShare->searchKeywords.searchKeys.SetSizeLimit();
	int nSize = pShare->searchKeywords.searchKeys.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("SEARCHKEY[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->searchKeywords.searchKeys[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("_REPLACEKEY_Counts"), pShare->searchKeywords.replaceKeys._GetSizeRef());
	pShare->searchKeywords.replaceKeys.SetSizeLimit();
	nSize = pShare->searchKeywords.replaceKeys.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("REPLACEKEY[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->searchKeywords.replaceKeys[i]);
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Grep(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Grep");
	int		nSize;
	WCHAR	szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_GREPFILE_Counts"), pShare->searchKeywords.grepFiles._GetSizeRef());
	pShare->searchKeywords.grepFiles.SetSizeLimit();
	nSize = pShare->searchKeywords.grepFiles.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("GREPFILE[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->searchKeywords.grepFiles[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("_GREPFOLDER_Counts"), pShare->searchKeywords.grepFolders._GetSizeRef());
	pShare->searchKeywords.grepFolders.SetSizeLimit();
	nSize = pShare->searchKeywords.grepFolders.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("GREPFOLDER[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->searchKeywords.grepFolders[i]);
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Folders(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Folders");
	// マクロ用フォルダ
	profile.IOProfileData(pszSecName, LTEXT("szMACROFOLDER"), pShare->common.macro.szMACROFOLDER);
	// 設定インポート用フォルダ
	profile.IOProfileData(pszSecName, LTEXT("szIMPORTFOLDER"), pShare->history.szIMPORTFOLDER);
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Cmd(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Cmd");
	WCHAR szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("nCmdArrNum"), pShare->history.aCommands._GetSizeRef());
	pShare->history.aCommands.SetSizeLimit();
	int nSize = pShare->history.aCommands.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("szCmdArr[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.aCommands[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("nCurDirArrNum"), pShare->history.aCurDirs._GetSizeRef());
	pShare->history.aCurDirs.SetSizeLimit();
	nSize = pShare->history.aCurDirs.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("szCurDirArr[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->history.aCurDirs[i]);
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Nickname(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Nickname");
	int i;
	WCHAR szKeyName[64];

	profile.IOProfileData( pszSecName, LTEXT("bShortPath"), pShare->common.fileName.bTransformShortPath );
	profile.IOProfileData( pszSecName, LTEXT("nShortPathMaxWidth"), pShare->common.fileName.nTransformShortMaxWidth );
	profile.IOProfileData(pszSecName, LTEXT("ArrNum"), pShare->common.fileName.nTransformFileNameArrNum);
	SetValueLimit(pShare->common.fileName.nTransformFileNameArrNum, MAX_TRANSFORM_FILENAME);
	int nSize = pShare->common.fileName.nTransformFileNameArrNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("From%02d"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pShare->common.fileName.szTransformFileNameFrom[i]));
		auto_sprintf(szKeyName, LTEXT("To%02d"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pShare->common.fileName.szTransformFileNameTo[i]));
	}
	// 読み込み時，残りをNULLで再初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_TRANSFORM_FILENAME; ++i) {
			pShare->common.fileName.szTransformFileNameFrom[i][0] = L'\0';
			pShare->common.fileName.szTransformFileNameTo[i][0]   = L'\0';
		}
	}
}

static bool ShareData_IO_RECT(DataProfile& profile, const WCHAR* pszSecName, const WCHAR* pszKeyName, RECT& rcValue)
{
	static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
	WCHAR szKeyData[100];
	bool ret = false;
	if (profile.IsReadingMode()) {
		ret = profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
		if (ret) {
			int buf[4];
			scan_ints(szKeyData, pszForm, buf);
			rcValue.left	= buf[0];
			rcValue.top		= buf[1];
			rcValue.right	= buf[2];
			rcValue.bottom	= buf[3];
		}
	}else {
		auto_sprintf(
			szKeyData,
			pszForm,
			rcValue.left,
			rcValue.top,
			rcValue.right,
			rcValue.bottom
		);
		ret = profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
	}
	return ret;
}

/*!
	@brief 共有データのCommonセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Common(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Common");
	// 2005.04.07 D.S.Koba
	CommonSetting& common = pShare->common;

	profile.IOProfileData(pszSecName, LTEXT("nCaretType")				, common.general.nCaretType);
	// Oct. 2, 2005 genta
	// 初期値を挿入モードに固定するため，設定の読み書きをやめる
	//profile.IOProfileData(pszSecName, LTEXT("bIsINSMode")				, common.bIsINSMode);
	profile.IOProfileData(pszSecName, LTEXT("bIsFreeCursorMode")		, common.general.bIsFreeCursorMode);
	
	profile.IOProfileData(pszSecName, LTEXT("bStopsBothEndsWhenSearchWord")	, common.general.bStopsBothEndsWhenSearchWord);
	profile.IOProfileData(pszSecName, LTEXT("bStopsBothEndsWhenSearchParagraph")	, common.general.bStopsBothEndsWhenSearchParagraph);
	// Oct. 27, 2000 genta
	profile.IOProfileData(pszSecName, LTEXT("bRestoreCurPosition")	, common.file.bRestoreCurPosition);
	// 2002.01.16 hor
	profile.IOProfileData(pszSecName, LTEXT("bRestoreBookmarks")	, common.file.bRestoreBookmarks);
	profile.IOProfileData(pszSecName, LTEXT("bAddCRLFWhenCopy")		, common.edit.bAddCRLFWhenCopy);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOpenDialogDir")		, common.edit.eOpenDialogDir);
	profile.IOProfileData(pszSecName, LTEXT("szOpenDialogSelDir")		, StringBufferT(common.edit.openDialogSelDir,_countof2(common.edit.openDialogSelDir)));
	profile.IOProfileData( pszSecName, LTEXT("bBoxSelectLock")	, common.edit.bBoxSelectLock );
	profile.IOProfileData(pszSecName, LTEXT("nRepeatedScrollLineNum")	, common.general.nRepeatedScrollLineNum);
	if (profile.IsReadingMode()) {
		common.general.nRepeatedScrollLineNum = std::max(1, common.general.nRepeatedScrollLineNum);
		common.general.nRepeatedScrollLineNum = std::min(10, common.general.nRepeatedScrollLineNum);
	}
	profile.IOProfileData(pszSecName, LTEXT("nRepeatedScroll_Smooth")	, common.general.nRepeatedScroll_Smooth);
	profile.IOProfileData(pszSecName, LTEXT("nPageScrollByWheel")	, common.general.nPageScrollByWheel);					// 2009.01.17 nasukoji
	profile.IOProfileData(pszSecName, LTEXT("nHorizontalScrollByWheel")	, common.general.nHorizontalScrollByWheel);	// 2009.01.17 nasukoji
		profile.IOProfileData(pszSecName, LTEXT("bCloseAllConfirm")		, common.general.bCloseAllConfirm);	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bExitConfirm")			, common.general.bExitConfirm);
	profile.IOProfileData(pszSecName, LTEXT("bSearchRegularExp")	, common.search.searchOption.bRegularExp);
	profile.IOProfileData(pszSecName, LTEXT("bSearchLoHiCase")		, common.search.searchOption.bLoHiCase);
	profile.IOProfileData(pszSecName, LTEXT("bSearchWordOnly")		, common.search.searchOption.bWordOnly);
	profile.IOProfileData(pszSecName, LTEXT("bSearchConsecutiveAll")		, common.search.bConsecutiveAll);	// 2007.01.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bSearchNOTIFYNOTFOUND")	, common.search.bNotifyNotFound);
	// 2002.01.26 hor
	profile.IOProfileData(pszSecName, LTEXT("bSearchAll")				, common.search.bSearchAll);
	profile.IOProfileData(pszSecName, LTEXT("bSearchSelectedArea")	, common.search.bSelectedArea);
	profile.IOProfileData(pszSecName, LTEXT("bGrepSubFolder")			, common.search.bGrepSubFolder);
	profile.IOProfileData( pszSecName, LTEXT("bGrepOutputLine")		, common.search.nGrepOutputLineType );
	profile.IOProfileData(pszSecName, LTEXT("nGrepOutputStyle")		, common.search.nGrepOutputStyle);
	profile.IOProfileData(pszSecName, LTEXT("bGrepOutputFileOnly")	, common.search.bGrepOutputFileOnly);
	profile.IOProfileData(pszSecName, LTEXT("bGrepOutputBaseFolder")	, common.search.bGrepOutputBaseFolder);
	profile.IOProfileData(pszSecName, LTEXT("bGrepSeparateFolder")	, common.search.bGrepSeparateFolder);
	profile.IOProfileData(pszSecName, LTEXT("bGrepDefaultFolder")		, common.search.bGrepDefaultFolder);
	profile.IOProfileData( pszSecName, LTEXT("bGrepBackup")			, common.search.bGrepBackup );
	
	// 2002/09/21 Moca 追加
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nGrepCharSet")	, common.search.nGrepCharSet);
	profile.IOProfileData(pszSecName, LTEXT("bGrepRealTime")			, common.search.bGrepRealTimeView); // 2003.06.16 Moca
	profile.IOProfileData(pszSecName, LTEXT("bCaretTextForSearch")	, common.search.bCaretTextForSearch);	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	profile.IOProfileData(pszSecName, LTEXT("bInheritKeyOtherView")	, common.search.bInheritKeyOtherView);
	profile.IOProfileData( pszSecName, LTEXT("nTagJumpMode")			, common.search.nTagJumpMode );
	profile.IOProfileData( pszSecName, LTEXT("nTagJumpModeKeyword")	, common.search.nTagJumpModeKeyword );
	
	// 正規表現DLL 2007.08.12 genta
	profile.IOProfileData(pszSecName, LTEXT("szRegexpLib")			, MakeStringBufferT(common.search.szRegexpLib));
	profile.IOProfileData(pszSecName, LTEXT("bGTJW_RETURN")			, common.search.bGTJW_Return);
	profile.IOProfileData(pszSecName, LTEXT("bGTJW_LDBLCLK")			, common.search.bGTJW_DoubleClick);
	profile.IOProfileData(pszSecName, LTEXT("bBackUp")				, common.backup.bBackUp);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpDialog")			, common.backup.bBackUpDialog);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpFolder")			, common.backup.bBackUpFolder);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpFolderRM")		, common.backup.bBackUpFolderRM);	// 2010/5/27 Uchi
	
	if (!profile.IsReadingMode()) {
		size_t nDummy = _tcslen(common.backup.szBackUpFolder);
		// フォルダの最後が「半角かつ'\\'」でない場合は、付加する
		ptrdiff_t nCharChars = &common.backup.szBackUpFolder[nDummy]
			- NativeT::GetCharPrev(common.backup.szBackUpFolder, nDummy, &common.backup.szBackUpFolder[nDummy]);
		if (1
			&& nCharChars == 1
			&& common.backup.szBackUpFolder[nDummy - 1] == '\\'
		) {
		}else {
			_tcscat(common.backup.szBackUpFolder, _T("\\"));
		}
	}
	profile.IOProfileData(pszSecName, LTEXT("szBackUpFolder"), common.backup.szBackUpFolder);
	if (profile.IsReadingMode()) {
		size_t	nDummy;
		ptrdiff_t nCharChars;
		nDummy = _tcslen(common.backup.szBackUpFolder);
		// フォルダの最後が「半角かつ'\\'」でない場合は、付加する
		nCharChars = &common.backup.szBackUpFolder[nDummy]
			- NativeT::GetCharPrev(common.backup.szBackUpFolder, nDummy, &common.backup.szBackUpFolder[nDummy]);
		if (1
			&& nCharChars == 1
			&& common.backup.szBackUpFolder[nDummy - 1] == '\\'
		) {
		}else {
			_tcscat(common.backup.szBackUpFolder, _T("\\"));
		}
	}
	
	profile.IOProfileData(pszSecName, LTEXT("nBackUpType")				, common.backup.nBackUpType);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt1")		, common.backup.nBackUpType_Opt1);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt2")		, common.backup.nBackUpType_Opt2);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt3")		, common.backup.nBackUpType_Opt3);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt4")		, common.backup.nBackUpType_Opt4);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpDustBox")			, common.backup.bBackUpDustBox);	//@@@ 2001.12.11 add MIK
	profile.IOProfileData(pszSecName, LTEXT("bBackUpPathAdvanced")		, common.backup.bBackUpPathAdvanced);	// 20051107 aroka
	profile.IOProfileData(pszSecName, LTEXT("szBackUpPathAdvanced")	, common.backup.szBackUpPathAdvanced);	// 20051107 aroka
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nFileShareMode")			, common.file.nFileShareMode);
	profile.IOProfileData(pszSecName, LTEXT("szExtHelp"), MakeStringBufferT(common.helper.szExtHelp));
	profile.IOProfileData(pszSecName, LTEXT("szExtHtmlHelp"), MakeStringBufferT(common.helper.szExtHtmlHelp));
	
	profile.IOProfileData(pszSecName, LTEXT("szMigemoDll"), MakeStringBufferT(common.helper.szMigemoDll));
	profile.IOProfileData(pszSecName, LTEXT("szMigemoDict"), MakeStringBufferT(common.helper.szMigemoDict));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		ShareData_IO_Sub_LogFont(profile, pszSecName, L"khlf", L"khps", L"khlfFaceName",
			common.helper.lf, common.helper.nPointSize);
	}// Keword Help Font
	
	
	profile.IOProfileData(pszSecName, LTEXT("nMRUArrNum_MAX")			, common.general.nMRUArrNum_MAX);
	SetValueLimit(common.general.nMRUArrNum_MAX, MAX_MRU);
	profile.IOProfileData(pszSecName, LTEXT("nOPENFOLDERArrNum_MAX")	, common.general.nOPENFOLDERArrNum_MAX);
	SetValueLimit(common.general.nOPENFOLDERArrNum_MAX, MAX_OPENFOLDER);
	profile.IOProfileData(pszSecName, LTEXT("bDispTOOLBAR")			, common.window.bDispToolBar);
	profile.IOProfileData(pszSecName, LTEXT("bDispSTATUSBAR")			, common.window.bDispStatusBar);
	profile.IOProfileData(pszSecName, LTEXT("bDispFUNCKEYWND")			, common.window.bDispFuncKeyWnd);
	profile.IOProfileData( pszSecName, LTEXT("bDispMiniMap")			, common.window.bDispMiniMap );
	profile.IOProfileData(pszSecName, LTEXT("nFUNCKEYWND_Place")		, common.window.nFuncKeyWnd_Place);
	profile.IOProfileData(pszSecName, LTEXT("nFUNCKEYWND_GroupNum")	, common.window.nFuncKeyWnd_GroupNum);		// 2002/11/04 Moca ファンクションキーのグループボタン数
	profile.IOProfileData(pszSecName, LTEXT("m_szLanguageDll")			, MakeStringBufferT(common.window.szLanguageDll));
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapFontSize")		, common.window.nMiniMapFontSize );
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapQuality")		, common.window.nMiniMapQuality );
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapWidth")			, common.window.nMiniMapWidth );
	
	profile.IOProfileData(pszSecName, LTEXT("bDispTabWnd")			, common.tabBar.bDispTabWnd);	// タブウィンドウ	//@@@ 2003.05.31 MIK
	profile.IOProfileData(pszSecName, LTEXT("bDispTabWndMultiWin")	, common.tabBar.bDispTabWndMultiWin);	// タブウィンドウ	//@@@ 2003.05.31 MIK
	profile.IOProfileData(pszSecName, LTEXT("szTabWndCaption")		, MakeStringBufferW(common.tabBar.szTabWndCaption));	//@@@ 2003.06.13 MIK
	profile.IOProfileData(pszSecName, LTEXT("bSameTabWidth")			, common.tabBar.bSameTabWidth);	// 2006.01.28 ryoji タブを等幅にする
	profile.IOProfileData(pszSecName, LTEXT("bDispTabIcon")			, common.tabBar.bDispTabIcon);	// 2006.01.28 ryoji タブにアイコンを表示する
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bDispTabClose")	, common.tabBar.dispTabClose);	// 2012.04.14 syat
	profile.IOProfileData(pszSecName, LTEXT("bSortTabList")			, common.tabBar.bSortTabList);	// 2006.05.10 ryoji タブ一覧をソートする
	profile.IOProfileData(pszSecName, LTEXT("bTab_RetainEmptyWin")	, common.tabBar.bTab_RetainEmptyWin);	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
	profile.IOProfileData(pszSecName, LTEXT("bTab_CloseOneWin")	, common.tabBar.bTab_CloseOneWin);	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
	profile.IOProfileData(pszSecName, LTEXT("bTab_ListFull")			, common.tabBar.bTab_ListFull);	// タブ一覧をフルパス表示する	// 2007.02.28 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bChgWndByWheel")		, common.tabBar.bChgWndByWheel);	// 2006.03.26 ryoji マウスホイールでウィンドウ切り替え
	profile.IOProfileData(pszSecName, LTEXT("bNewWindow")			, common.tabBar.bNewWindow);	// 外部から起動するときは新しいウィンドウで開く
	profile.IOProfileData( pszSecName, L"bTabMultiLine"			, common.tabBar.bTabMultiLine );	// タブ多段
	profile.IOProfileData_WrapInt( pszSecName, L"eTabPosition"		, common.tabBar.eTabPosition );	// タブ位置

	ShareData_IO_Sub_LogFont(profile, pszSecName, L"lfTabFont", L"lfTabFontPs", L"lfTabFaceName",
		common.tabBar.lf, common.tabBar.nPointSize);
	
	profile.IOProfileData( pszSecName, LTEXT("nTabMaxWidth")			, common.tabBar.nTabMaxWidth );
	profile.IOProfileData( pszSecName, LTEXT("nTabMinWidth")			, common.tabBar.nTabMinWidth );
	profile.IOProfileData( pszSecName, LTEXT("nTabMinWidthOnMulti")	, common.tabBar.nTabMinWidthOnMulti );

	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	profile.IOProfileData(pszSecName, LTEXT("bSplitterWndHScroll")	, common.window.bSplitterWndHScroll);
	profile.IOProfileData(pszSecName, LTEXT("bSplitterWndVScroll")	, common.window.bSplitterWndVScroll);
	
	profile.IOProfileData(pszSecName, LTEXT("szMidashiKigou")		, MakeStringBufferW(common.format.szMidashiKigou));
	profile.IOProfileData(pszSecName, LTEXT("szInyouKigou")			, MakeStringBufferW(common.format.szInyouKigou));
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	profile.IOProfileData(pszSecName, LTEXT("bUseHokan")				, common.helper.bUseHokan);
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bSaveWindowSize")	, common.window.eSaveWindowSize);	//#####フラグ名が激しくきもい
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeType")			, common.window.nWinSizeType);
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeCX")				, common.window.nWinSizeCX);
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeCY")				, common.window.nWinSizeCY);
	// 2004.03.30 Moca *nWinPos*を追加
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nSaveWindowPos")	, common.window.eSaveWindowPos);	//#####フラグ名がきもい
	profile.IOProfileData(pszSecName, LTEXT("nWinPosX")				, common.window.nWinPosX);
	profile.IOProfileData(pszSecName, LTEXT("nWinPosY")				, common.window.nWinPosY);
	profile.IOProfileData(pszSecName, LTEXT("bTaskTrayUse")			, common.general.bUseTaskTray);
	profile.IOProfileData(pszSecName, LTEXT("bTaskTrayStay")			, common.general.bStayTaskTray);

	profile.IOProfileData(pszSecName, LTEXT("wTrayMenuHotKeyCode")		, common.general.wTrayMenuHotKeyCode);
	profile.IOProfileData(pszSecName, LTEXT("wTrayMenuHotKeyMods")		, common.general.wTrayMenuHotKeyMods);
	profile.IOProfileData(pszSecName, LTEXT("bUseOLE_DragDrop")			, common.edit.bUseOLE_DragDrop);
	profile.IOProfileData(pszSecName, LTEXT("bUseOLE_DropSource")			, common.edit.bUseOLE_DropSource);
	profile.IOProfileData(pszSecName, LTEXT("bDispExitingDialog")			, common.general.bDispExitingDialog);
	profile.IOProfileData(pszSecName, LTEXT("bEnableUnmodifiedOverwrite")	, common.file.bEnableUnmodifiedOverwrite);
	profile.IOProfileData(pszSecName, LTEXT("bSelectClickedURL")			, common.edit.bSelectClickedURL);
	profile.IOProfileData(pszSecName, LTEXT("bGrepExitConfirm")			, common.search.bGrepExitConfirm);// Grepモードで保存確認するか
//	profile.IOProfileData(pszSecName, LTEXT("bRulerDisp")					, common.bRulerDisp);					// ルーラー表示
	profile.IOProfileData(pszSecName, LTEXT("nRulerHeight")				, common.window.nRulerHeight);		// ルーラー高さ
	profile.IOProfileData(pszSecName, LTEXT("nRulerBottomSpace")			, common.window.nRulerBottomSpace);	// ルーラーとテキストの隙間
	profile.IOProfileData(pszSecName, LTEXT("nRulerType")					, common.window.nRulerType);			// ルーラーのタイプ
	// Sep. 18, 2002 genta 追加
	profile.IOProfileData(pszSecName, LTEXT("nLineNumberRightSpace")	, common.window.nLineNumRightSpace);	// 行番号の右側の隙間
	profile.IOProfileData(pszSecName, LTEXT("nVertLineOffset")			, common.window.nVertLineOffset);		// 2005.11.10 Moca
	profile.IOProfileData(pszSecName, LTEXT("bUseCompotibleBMP")		, common.window.bUseCompatibleBMP);	// 2007.09.09 Moca
	profile.IOProfileData(pszSecName, LTEXT("bCopyAndDisablSelection")	, common.edit.bCopyAndDisablSelection);	// コピーしたら選択解除
	profile.IOProfileData(pszSecName, LTEXT("bEnableNoSelectCopy")		, common.edit.bEnableNoSelectCopy);		// 選択なしでコピーを可能にする	// 2007.11.18 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bEnableLineModePaste")	, common.edit.bEnableLineModePaste);		// ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bConvertEOLPaste")		, common.edit.bConvertEOLPaste);			// 改行コードを変換して貼り付ける	// 2009.02.28 salarm
	profile.IOProfileData(pszSecName, LTEXT("bEnableExtEol")			, common.edit.bEnableExtEol);
	
	profile.IOProfileData(pszSecName, LTEXT("bHtmlHelpIsSingle")		, common.helper.bHtmlHelpIsSingle);		// HtmlHelpビューアはひとつ
	profile.IOProfileData(pszSecName, LTEXT("bCompareAndTileHorz")		, common.compare.bCompareAndTileHorz);	// 文書比較後、左右に並べて表示	// Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	profile.IOProfileData(pszSecName, LTEXT("bDropFileAndClose")		, common.file.bDropFileAndClose);			// ファイルをドロップしたときは閉じて開く
	profile.IOProfileData(pszSecName, LTEXT("nDropFileNumMax")			, common.file.nDropFileNumMax);			// 一度にドロップ可能なファイル数
	profile.IOProfileData(pszSecName, LTEXT("bCheckFileTimeStamp")		, common.file.bCheckFileTimeStamp);		// 更新の監視
	profile.IOProfileData(pszSecName, LTEXT("nAutoloadDelay")			, common.file.nAutoloadDelay);			// 自動読込時遅延
	profile.IOProfileData(pszSecName, LTEXT("bUneditableIfUnwritable")	, common.file.bUneditableIfUnwritable);	// 上書き禁止検出時は編集禁止にする
	profile.IOProfileData(pszSecName, LTEXT("bNotOverWriteCRLF")		, common.edit.bNotOverWriteCRLF);			// 改行は上書きしない
	profile.IOProfileData(pszSecName, LTEXT("bOverWriteFixMode")		, common.edit.bOverWriteFixMode);			// 文字幅に合わせてスペースを詰める
	profile.IOProfileData(pszSecName, LTEXT("bOverWriteBoxDelete")		, common.edit.bOverWriteBoxDelete);
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgFind")		, common.search.bAutoCloseDlgFind);		// 検索ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgFuncList")	, common.outline.bAutoCloseDlgFuncList);	// アウトライン ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgReplace")	, common.search.bAutoCloseDlgReplace);	// 置換 ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoColmnPaste")			, common.edit.bAutoColumnPaste);			// 矩形コピーのテキストは常に矩形貼り付け // 2013.5.23 aroka iniファイルのtypo未修正
	profile.IOProfileData(pszSecName, LTEXT("NoCaretMoveByActivation")	, common.general.bNoCaretMoveByActivation);// マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta)
	profile.IOProfileData(pszSecName, LTEXT("bScrollBarHorz")			, common.window.bScrollBarHorz);			// 水平スクロールバーを使う

	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_RETURN")		, common.helper.bHokanKey_RETURN);		// VK_RETURN 補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_TAB")			, common.helper.bHokanKey_TAB);			// VK_TAB    補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_RIGHT")			, common.helper.bHokanKey_RIGHT);			// VK_RIGHT  補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_SPACE")			, common.helper.bHokanKey_SPACE);			// VK_SPACE  補完決定キーが有効/無効
	
	profile.IOProfileData(pszSecName, LTEXT("nDateFormatType")			, common.format.nDateFormatType);			// 日付書式のタイプ
	profile.IOProfileData(pszSecName, LTEXT("szDateFormat")			, MakeStringBufferT(common.format.szDateFormat));	// 日付書式
	profile.IOProfileData(pszSecName, LTEXT("nTimeFormatType")			, common.format.nTimeFormatType);			// 時刻書式のタイプ
	profile.IOProfileData(pszSecName, LTEXT("szTimeFormat")			, MakeStringBufferT(common.format.szTimeFormat));	// 時刻書式
	
	profile.IOProfileData(pszSecName, LTEXT("bMenuIcon")				, common.window.bMenuIcon);			// メニューにアイコンを表示する
	profile.IOProfileData(pszSecName, LTEXT("bAutoMIMEdecode")			, common.file.bAutoMimeDecode);			// ファイル読み込み時にMIMEのdecodeを行うか
	profile.IOProfileData(pszSecName, LTEXT("bQueryIfCodeChange")		, common.file.bQueryIfCodeChange);		// Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	profile.IOProfileData(pszSecName, LTEXT("bAlertIfFileNotExist")	, common.file.bAlertIfFileNotExist);	// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	profile.IOProfileData(pszSecName, LTEXT("bNoFilterSaveNew")		, common.file.bNoFilterSaveNew);	// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bNoFilterSaveFile")		, common.file.bNoFilterSaveFile);	// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bAlertIfLargeFile")		, common.file.bAlertIfLargeFile);	// 開こうとしたファイルが大きい場合に警告する
	profile.IOProfileData(pszSecName, LTEXT("nAlertFileSize")			, common.file.nAlertFileSize);	// 警告を開始するファイルサイズ(MB単位)
	
	//「開く」ダイアログのサイズと位置
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcOpenDialog")		, common.others.rcOpenDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcCompareDialog")	, common.others.rcCompareDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcDiffDialog")		, common.others.rcDiffDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcFavoriteDialog")	, common.others.rcFavoriteDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcTagJumpDialog")	, common.others.rcTagJumpDialog);
	
	// 2002.02.08 aroka,hor
	profile.IOProfileData(pszSecName, LTEXT("bMarkUpBlankLineEnable")	, common.outline.bMarkUpBlankLineEnable);
	profile.IOProfileData(pszSecName, LTEXT("bFunclistSetFocusOnJump")	, common.outline.bFunclistSetFocusOnJump);
	
	// Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	profile.IOProfileData(pszSecName, LTEXT("szWinCaptionActive") , MakeStringBufferT(common.window.szWindowCaptionActive));
	profile.IOProfileData(pszSecName, LTEXT("szWinCaptionInactive"), MakeStringBufferT(common.window.szWindowCaptionInactive));
	
	// アウトライン/トピックリスト の位置とサイズを記憶  20060201 aroka
	profile.IOProfileData(pszSecName, LTEXT("bRememberOutlineWindowPos"), common.outline.bRememberOutlineWindowPos);
	if (common.outline.bRememberOutlineWindowPos) {
		profile.IOProfileData(pszSecName, LTEXT("widthOutlineWindow")	, common.outline.widthOutlineWindow);
		profile.IOProfileData(pszSecName, LTEXT("heightOutlineWindow"), common.outline.heightOutlineWindow);
		profile.IOProfileData(pszSecName, LTEXT("xOutlineWindowPos")	, common.outline.xOutlineWindowPos);
		profile.IOProfileData(pszSecName, LTEXT("yOutlineWindowPos")	, common.outline.yOutlineWindowPos);
	}
	profile.IOProfileData(pszSecName, LTEXT("nOutlineDockSet"), common.outline.nOutlineDockSet);
	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockSync"), common.outline.bOutlineDockSync);
	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockDisp"), common.outline.bOutlineDockDisp);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOutlineDockSide"), common.outline.eOutlineDockSide);
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR szKeyData[1024];
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData))) {
				int buf[4];
				scan_ints(szKeyData, pszForm, buf);
				common.outline.cxOutlineDockLeft	= buf[0];
				common.outline.cyOutlineDockTop	= buf[1];
				common.outline.cxOutlineDockRight	= buf[2];
				common.outline.cyOutlineDockBottom	= buf[3];
			}
		}else {
			auto_sprintf(
				szKeyData,
				pszForm,
				common.outline.cxOutlineDockLeft,
				common.outline.cyOutlineDockTop,
				common.outline.cxOutlineDockRight,
				common.outline.cyOutlineDockBottom
			);
			profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
		}
	}
	profile.IOProfileData(pszSecName, LTEXT("nDockOutline"), (int&)common.outline.nDockOutline);
	ShareData_IO_FileTree( profile, common.outline.fileTree, pszSecName );
	profile.IOProfileData( pszSecName, LTEXT("szFileTreeDefIniName"), common.outline.fileTreeDefIniName );
}


// プラグインコマンドを名前から機能番号へ変換
EFunctionCode GetPlugCmdInfoByName(
	const WCHAR* pszFuncName			// [in]  プラグインコマンド名
	)
{
	if (!pszFuncName) {
		return F_INVALID;
	}
	const WCHAR* psCmdName;
	if (!(psCmdName = wcschr(pszFuncName, L'/'))) {
		return F_INVALID;
	}
	size_t nLen = MAX_PLUGIN_ID < (psCmdName - pszFuncName) ? MAX_PLUGIN_ID : (psCmdName - pszFuncName);
	WCHAR sPluginName[MAX_PLUGIN_ID + 1];
	wcsncpy(sPluginName, pszFuncName, nLen);
	sPluginName[nLen] = L'\0'; 
	++psCmdName;

	CommonSetting_Plugin& plugin = GetDllShareData().common.plugin;
	int nId = -1;
	for (int i=0; i<MAX_PLUGIN; ++i) {
		PluginRec& pluginrec = plugin.pluginTable[i];
		if (auto_strcmp(pluginrec.szId, sPluginName) == 0) {
			nId = i;
			break;
		}
	}
	int nNo = _wtoi(psCmdName);

	if (nId < 0 || nNo <= 0 || nNo >= MAX_PLUG_CMD) {
		// プラグインがない/番号がおかしい
		return F_INVALID;
	}
	
	return Plug::GetPluginFunctionCode(nId, nNo);
}

// プラグインコマンドを機能番号から名前へ変換
bool GetPlugCmdInfoByFuncCode(
	EFunctionCode	eFuncCode,				// [in]  機能コード
	WCHAR*			pszFuncName				// [out] 機能名．この先にはMAX_PLUGIN_ID + 20文字のメモリが必要．
	)
{
	if (eFuncCode < F_PLUGCOMMAND_FIRST || eFuncCode > F_PLUGCOMMAND_LAST) {
		return false;
	}

	PluginId nID = Plug::GetPluginId(eFuncCode);
	PlugId nNo = Plug::GetPlugId(eFuncCode);
	if (nID < 0 || nNo < 0) {
		return false;
	}
	CommonSetting_Plugin& plugin = GetDllShareData().common.plugin;
	auto_sprintf(pszFuncName, L"%ls/%02d", plugin.pluginTable[nID].szId, nNo);

	return true;
}


/*!
	@brief 共有データのToolbarセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Toolbar(DataProfile& profile, MenuDrawer* pMenuDrawer)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Toolbar");
	int		i;
	WCHAR	szKeyName[64];
	CommonSetting_ToolBar& toolbar = pShare->common.toolBar;

	EFunctionCode	eFunc;
	WCHAR			szText[MAX_PLUGIN_ID + 20];
	int				nInvalid = -1;

	profile.IOProfileData(pszSecName, LTEXT("bToolBarIsFlat"), toolbar.bToolBarIsFlat);

	profile.IOProfileData(pszSecName, LTEXT("nToolBarButtonNum"), toolbar.nToolBarButtonNum);
	SetValueLimit(toolbar.nToolBarButtonNum, MAX_TOOLBAR_BUTTON_ITEMS);
	int	nSize = toolbar.nToolBarButtonNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("nTBB[%03d]"), i);
		// Plugin String Parametor
		if (profile.IsReadingMode()) {
			// 読み込み
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szText));
			if (!wcschr(szText, L'/')) {
				// 番号
				toolbar.nToolBarButtonIdxArr[i] = _wtoi(szText);
			}else {
				// Plugin
				eFunc = GetPlugCmdInfoByName(szText);
				if (eFunc == F_INVALID) {
					toolbar.nToolBarButtonIdxArr[i] = -1;		// 未解決
				}else {
					toolbar.nToolBarButtonIdxArr[i] = pMenuDrawer->FindToolbarNoFromCommandId(eFunc, false);
				}
			}
		}else {
			// 書き込み
			if (toolbar.nToolBarButtonIdxArr[i] <= MAX_TOOLBAR_ICON_COUNT + 1) {	// +1はセパレータ分
				profile.IOProfileData(pszSecName, szKeyName, toolbar.nToolBarButtonIdxArr[i]);	
			}else {
				// Plugin
				eFunc = (EFunctionCode)toolbar.nToolBarButtonIdxArr[i];
				if (eFunc == F_DEFAULT) {
					profile.IOProfileData(pszSecName, szKeyName, nInvalid);	
				}else if (GetPlugCmdInfoByFuncCode(eFunc, szText)) {
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szText));	
				}else {
					profile.IOProfileData(pszSecName, szKeyName, toolbar.nToolBarButtonIdxArr[i]);	
				}
			}
		}
	}
	// 読み込み時は残りを初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_TOOLBAR_BUTTON_ITEMS; ++i) {
			toolbar.nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief 共有データのCustMenuセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2010.08.21 Moca 旧ShareData_IO_CustMenuをIO_CustMenuに変更
*/
void ShareData_IO::ShareData_IO_CustMenu(DataProfile& profile)
{
	IO_CustMenu(profile, GetDllShareData().common.customMenu, false);
}

/*!
	@brief CustMenuの入出力
	@param[in,out]	profile	INIファイル入出力クラス
	@param[in,out]	menu	入出力対象
	@param	bOutCmdName	出力時にマクロ名で出力

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::IO_CustMenu(DataProfile& profile, CommonSetting_CustomMenu& menu, bool bOutCmdName)
{
	static const WCHAR* pszSecName = LTEXT("CustMenu");
	wchar_t szKeyName[64];
	wchar_t szFuncName[1024];
	EFunctionCode n;

	for (int i=0; i<MAX_CUSTOM_MENU; ++i) {
		auto_sprintf(szKeyName, LTEXT("szCMN[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(menu.szCustMenuNameArr[i]));	// Oct. 15, 2001 genta 最大長指定
		auto_sprintf(szKeyName, LTEXT("bCMPOP[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, menu.bCustMenuPopupArr[i]);
		auto_sprintf(szKeyName, LTEXT("nCMIN[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, menu.nCustMenuItemNumArr[i]);
		SetValueLimit(menu.nCustMenuItemNumArr[i], _countof(menu.nCustMenuItemFuncArr[0]));
		int nSize = menu.nCustMenuItemNumArr[i];
		for (int j=0; j<nSize; ++j) {
			// start マクロ名でも設定できるように 2008/5/24 Uchi
			auto_sprintf(szKeyName, LTEXT("nCMIF[%02d][%02d]"), i, j);
			if (profile.IsReadingMode()) {
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
				if (wcschr(szFuncName, L'/')) {
					// Plugin名
					n = GetPlugCmdInfoByName(szFuncName);
				}else if (1
					&& WCODE::Is09(*szFuncName) 
					&& (szFuncName[1] == L'\0' || WCODE::Is09(szFuncName[1]))
				) {
					n = (EFunctionCode)auto_atol(szFuncName);
				}else {
					n = SMacroMgr::GetFuncInfoByName(0, szFuncName, NULL);
				}
				if (n == F_INVALID) {
					n = F_DEFAULT;
				}
				menu.nCustMenuItemFuncArr[i][j] = n;
			}else {
				if (GetPlugCmdInfoByFuncCode(menu.nCustMenuItemFuncArr[i][j], szFuncName)) {
					// Plugin
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
				}else {
					if (bOutCmdName) {
						WCHAR* p = SMacroMgr::GetFuncInfoByID(
							G_AppInstance(),
							menu.nCustMenuItemFuncArr[i][j],
							szFuncName,
							NULL
						);
						if (!p) {
							auto_sprintf(szFuncName, L"%d", menu.nCustMenuItemFuncArr[i][j]);
						}
						profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
					}else {
						profile.IOProfileData_WrapInt(pszSecName, szKeyName, menu.nCustMenuItemFuncArr[i][j]);
					}
				}
			}
			// end

			auto_sprintf(szKeyName, LTEXT("nCMIK[%02d][%02d]"), i, j);
			profile.IOProfileData(pszSecName, szKeyName, menu.nCustMenuItemKeyArr[i][j]);
		}
	}
}

/*!
	@brief 共有データのFontセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Font(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Font");
	CommonSetting_View& view = pShare->common.view;
	ShareData_IO_Sub_LogFont(profile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
		view.lf, view.nPointSize);

	profile.IOProfileData(pszSecName, LTEXT("bFontIs_FIXED_PITCH"), view.bFontIs_FixedPitch);
}

/*!
	@brief 共有データのKeyBindセクションの入出力
*/
void ShareData_IO::ShareData_IO_KeyBind(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();
	IO_KeyBind(profile, pShare->common.keyBind, false);	// add Parameter 2008/5/24
}

/*!
	@brief KeyBindセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス
	@param[in,out]	keyBind	キー割り当て設定

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
	@date 2010.08.21 Moca ShareData_IO_KeyBindをIO_KeyBindに名称変更
	@date 2012.11.20 aroka 引数を CommonSetting_KeyBind に変更
	@date 2012.11.25 aroka マウスコードの固定と重複排除
*/
void ShareData_IO::IO_KeyBind(DataProfile& profile, CommonSetting_KeyBind& keyBind, bool bOutCmdName)
{
	static const WCHAR* szSecName = L"KeyBind";
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
//	int		nSize = pShareData->nKeyNameArrNum;
	WCHAR	szWork[MAX_PLUGIN_ID + 20 + 4];
	bool	bOldVer = false;
	const int KEYNAME_SIZE = _countof(keyBind.pKeyNameArr) - 1;// 最後の１要素はダミー用に予約 2012.11.25 aroka
	int nKeyNameArrUsed = keyBind.nKeyNameArrNum; // 使用済み領域

	if (profile.IsReadingMode()) { 
		if (!profile.IOProfileData(szSecName, L"KeyBind[000]", MakeStringBufferW(szKeyData))) {
			bOldVer = true;
		}else {
			// 新スタイルのImportは割り当て表サイズぎりぎりまで読み込む
			// 旧スタイルは初期値と一致しないKeyNameは捨てるのでデータ数に変化なし
			keyBind.nKeyNameArrNum = KEYNAME_SIZE;
		}
	}

	for (int i=0; i<keyBind.nKeyNameArrNum; ++i) {
		// 2005.04.07 D.S.Koba
		//KeyData& keydata = pShareData->pKeyNameArr[i];
		//KeyData& keydata = keyBind.ppKeyNameArr[i];
		
		if (profile.IsReadingMode()) {
			if (bOldVer) {
				KeyData& keydata = keyBind.pKeyNameArr[i];
				_tcstowcs(szKeyName, keydata.szKeyName, _countof(szKeyName));
				if (profile.IOProfileData(szSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					int buf[8];
					scan_ints(szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"), buf);
					keydata.nFuncCodeArr[0]	= (EFunctionCode)buf[0];
					keydata.nFuncCodeArr[1]	= (EFunctionCode)buf[1];
					keydata.nFuncCodeArr[2]	= (EFunctionCode)buf[2];
					keydata.nFuncCodeArr[3]	= (EFunctionCode)buf[3];
					keydata.nFuncCodeArr[4]	= (EFunctionCode)buf[4];
					keydata.nFuncCodeArr[5]	= (EFunctionCode)buf[5];
					keydata.nFuncCodeArr[6]	= (EFunctionCode)buf[6];
					keydata.nFuncCodeArr[7]	= (EFunctionCode)buf[7];
				}
			}else {		// 新バージョン(キー割り当てのImport,export の合わせた)	2008/5/25 Uchi
				KeyData tmpKeydata;
				auto_sprintf(szKeyName, L"KeyBind[%03d]", i);
				if (profile.IOProfileData(szSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					wchar_t	*p;
					wchar_t	*pn;
					int		nRes;

					p = szKeyData;
					// keycode取得
					int keycode;
					pn = auto_strchr(p, ',');
					if (!pn)	continue;
					*pn = 0;
					nRes = scan_ints(p, L"%04x", &keycode);
					if (nRes != 1)	continue;
					tmpKeydata.nKeyCode = (short)keycode;
					p = pn + 1;

					// 後に続くトークン 
					for (int j=0; j<8; ++j) {
						EFunctionCode n;
						// 機能名を数値に置き換える。(数値の機能名もあるかも)
						// @@@ 2002.2.2 YAZAKI マクロをSMacroMgrに統一
						pn = auto_strchr(p, ',');
						if (!pn)	break;
						*pn = 0;
						if (wcschr(p, L'/')) {
							// Plugin名
							n = GetPlugCmdInfoByName(p);
						}else if (WCODE::Is09(*p) && (p[1] == L'\0' || WCODE::Is09(p[1]))) {
							n = (EFunctionCode)auto_atol(p);
						}else {
							n = SMacroMgr::GetFuncInfoByName(0, p, NULL);
						}
						if (n == F_INVALID) {
							n = F_DEFAULT;
						}
						tmpKeydata.nFuncCodeArr[j] = n;
						p = pn + 1;
					}
					// KeyName
					auto_strncpy(tmpKeydata.szKeyName, to_tchar(p), _countof(tmpKeydata.szKeyName) - 1);
					tmpKeydata.szKeyName[_countof(tmpKeydata.szKeyName) - 1] = '\0';

					if (tmpKeydata.nKeyCode <= 0) { // マウスコードは先頭に固定されている KeyCodeが同じなのでKeyNameで判別
						// 2013.10.23 syat マウスのキーコードを拡張仮想キーコードに変更。以下は互換性のため残す。
						for (int im=0; im<jpVKEXNamesLen; ++im) {
							if (_tcscmp(tmpKeydata.szKeyName, jpVKEXNames[im]) == 0) {
								_tcscpy(tmpKeydata.szKeyName, keyBind.pKeyNameArr[im].szKeyName);
								keyBind.pKeyNameArr[im + 0x0100] = tmpKeydata;
							}
						}
					}else {
						// 割り当て済みキーコードは上書き
						int idx = keyBind.keyToKeyNameArr[tmpKeydata.nKeyCode];
						if (idx != KEYNAME_SIZE) {
							_tcscpy(tmpKeydata.szKeyName, keyBind.pKeyNameArr[idx].szKeyName);
							keyBind.pKeyNameArr[idx] = tmpKeydata;
						}else {// 未割り当てキーコードは末尾に追加
							if (nKeyNameArrUsed >= KEYNAME_SIZE) {
							}else {
								_tcscpy(tmpKeydata.szKeyName, keyBind.pKeyNameArr[nKeyNameArrUsed].szKeyName);
								keyBind.pKeyNameArr[nKeyNameArrUsed] = tmpKeydata;
								keyBind.keyToKeyNameArr[tmpKeydata.nKeyCode] = (BYTE)nKeyNameArrUsed++;
							}
						}
					}
				}
			}
		}else {
		//	auto_sprintf(szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
		//		keydata.nFuncCodeArr[0],
		//		keydata.nFuncCodeArr[1],
		//		keydata.nFuncCodeArr[2],
		//		keydata.nFuncCodeArr[3],
		//		keydata.nFuncCodeArr[4],
		//		keydata.nFuncCodeArr[5],
		//		keydata.nFuncCodeArr[6],
		//		keydata.nFuncCodeArr[7]
		//	);
		//	profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));

// start 新バージョン	2008/5/25 Uchi
			KeyData& keydata = keyBind.pKeyNameArr[i];
			auto_sprintf(szKeyName, L"KeyBind[%03d]", i);
			auto_sprintf(szKeyData, L"%04x", keydata.nKeyCode);
			for (int j=0; j<8; ++j) {
				WCHAR	szFuncName[256];
				if (GetPlugCmdInfoByFuncCode(keydata.nFuncCodeArr[j], szFuncName)) {
					// Plugin
					auto_sprintf(szWork, L",%ls", szFuncName);
				}else {
					if (bOutCmdName) {
						//@@@ 2002.2.2 YAZAKI マクロをSMacroMgrに統一
						// 2010.06.30 Moca 日本語名を取得しないように
						WCHAR* p = SMacroMgr::GetFuncInfoByID(
							0,
							keydata.nFuncCodeArr[j],
							szFuncName,
							NULL
						);
						if (p) {
							auto_sprintf(szWork, L",%ls", p);
						}else {
							auto_sprintf(szWork, L",%d", keydata.nFuncCodeArr[j]);
						}
					}else {
						auto_sprintf(szWork, L",%d", keydata.nFuncCodeArr[j]);
					}
				}
				wcscat(szKeyData, szWork);
			}

			if (0x0100 <= keydata.nKeyCode) {
				auto_sprintf(szWork, L",%ts", jpVKEXNames[keydata.nKeyCode - 0x0100]);
			}else {
				auto_sprintf(szWork, L",%ts", keydata.szKeyName);
			}
			wcscat(szKeyData, szWork);
			profile.IOProfileData(szSecName, szKeyName, MakeStringBufferW(szKeyData));
//
		}
	}

	if (profile.IsReadingMode()) {
		keyBind.nKeyNameArrNum = nKeyNameArrUsed;
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Print(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Print");
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	for (int i=0; i<MAX_PrintSettingARR; ++i) {
		// 2005.04.07 D.S.Koba
		PrintSetting& printsetting = pShare->printSettingArr[i];
		auto_sprintf(szKeyName, LTEXT("PS[%02d].nInts"), i);
		static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
				int buf[19];
				scan_ints(szKeyData, pszForm, buf);
				printsetting.nPrintFontWidth			= buf[0];
				printsetting.nPrintFontHeight			= buf[1];
				printsetting.nPrintDansuu				= buf[2];
				printsetting.nPrintDanSpace			= buf[3];
				printsetting.nPrintLineSpacing		= buf[4];
				printsetting.nPrintMarginTY			= buf[5];
				printsetting.nPrintMarginBY			= buf[6];
				printsetting.nPrintMarginLX			= buf[7];
				printsetting.nPrintMarginRX			= buf[8];
				printsetting.nPrintPaperOrientation	= (short)buf[9];
				printsetting.nPrintPaperSize			= (short)buf[10];
				printsetting.bPrintWordWrap			= (buf[11] != 0);
				printsetting.bPrintLineNumber			= (buf[12] != 0);
				printsetting.bHeaderUse[0]			= buf[13];
				printsetting.bHeaderUse[1]			= buf[14];
				printsetting.bHeaderUse[2]			= buf[15];
				printsetting.bFooterUse[0]			= buf[16];
				printsetting.bFooterUse[1]			= buf[17];
				printsetting.bFooterUse[2]			= buf[18];
			}
		}else {
			auto_sprintf(szKeyData, pszForm,
				printsetting.nPrintFontWidth		,
				printsetting.nPrintFontHeight		,
				printsetting.nPrintDansuu			,
				printsetting.nPrintDanSpace			,
				printsetting.nPrintLineSpacing		,
				printsetting.nPrintMarginTY			,
				printsetting.nPrintMarginBY			,
				printsetting.nPrintMarginLX			,
				printsetting.nPrintMarginRX			,
				printsetting.nPrintPaperOrientation	,
				printsetting.nPrintPaperSize		,
				printsetting.bPrintWordWrap ? 1 : 0,
				printsetting.bPrintLineNumber ? 1 : 0,
				printsetting.bHeaderUse[0] ? 1 : 0,
				printsetting.bHeaderUse[1] ? 1 : 0,
				printsetting.bHeaderUse[2] ? 1 : 0,
				printsetting.bFooterUse[0] ? 1 : 0,
				printsetting.bFooterUse[1] ? 1 : 0,
				printsetting.bFooterUse[2] ? 1 : 0
			);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
		}

		auto_sprintf(szKeyName, LTEXT("PS[%02d].szSName")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.szPrintSettingName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szFF")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.szPrintFontFaceHan));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szFFZ")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.szPrintFontFaceZen));
		// ヘッダ/フッタ
		for (int j=0; j<3; ++j) {
			auto_sprintf(szKeyName, LTEXT("PS[%02d].szHF[%d]") , i, j);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(printsetting.szHeaderForm[j]));
			auto_sprintf(szKeyName, LTEXT("PS[%02d].szFTF[%d]"), i, j);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(printsetting.szFooterForm[j]));
		}
		{ // ヘッダ/フッタ フォント設定
			WCHAR	szKeyName2[64];
			WCHAR	szKeyName3[64];
			auto_sprintf(szKeyName,  LTEXT("PS[%02d].lfHeader"),			i);
			auto_sprintf(szKeyName2, LTEXT("PS[%02d].nHeaderPointSize"),	i);
			auto_sprintf(szKeyName3, LTEXT("PS[%02d].lfHeaderFaceName"),	i);
			ShareData_IO_Sub_LogFont(profile, pszSecName, szKeyName, szKeyName2, szKeyName3,
				printsetting.lfHeader, printsetting.nHeaderPointSize);
			auto_sprintf(szKeyName,  LTEXT("PS[%02d].lfFooter"),			i);
			auto_sprintf(szKeyName2, LTEXT("PS[%02d].nFooterPointSize"),	i);
			auto_sprintf(szKeyName3, LTEXT("PS[%02d].lfFooterFaceName"),	i);
			ShareData_IO_Sub_LogFont(profile, pszSecName, szKeyName, szKeyName2, szKeyName3,
				printsetting.lfFooter, printsetting.nFooterPointSize);
		}

		auto_sprintf(szKeyName, LTEXT("PS[%02d].szDriver"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.mdmDevMode.szPrinterDriverName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szDevice"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.mdmDevMode.szPrinterDeviceName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szOutput"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.mdmDevMode.szPrinterOutputName));

		// 2002.02.16 hor とりあえず旧設定を変換しとく
		if (wcscmp(printsetting.szHeaderForm[0], _EDITL("&f")) == 0 &&
			wcscmp(printsetting.szFooterForm[0], _EDITL("&C- &P -")) == 0
		) {
			auto_strcpy(printsetting.szHeaderForm[0], _EDITL("$f"));
			auto_strcpy(printsetting.szFooterForm[0], _EDITL(""));
			auto_strcpy(printsetting.szFooterForm[1], _EDITL("- $p -"));
		}

		// 禁則	//@@@ 2002.04.09 MIK
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuHead"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.bPrintKinsokuHead);
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuTail"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.bPrintKinsokuTail);
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuRet"),  i); profile.IOProfileData(pszSecName, szKeyName, printsetting.bPrintKinsokuRet);	//@@@ 2002.04.13 MIK
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuKuto"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.bPrintKinsokuKuto);	//@@@ 2002.04.17 MIK

		// カラー印刷
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bColorPrint"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.bColorPrint);	// 2013/4/26 Uchi
	}
}

/*!
	@brief 共有データのTypeConfigセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
	@date 2010/04/17 Uchi ループ内をShareData_IO_Type_Oneに分離。
*/
void ShareData_IO::ShareData_IO_Types(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();
	WCHAR szKey[32];
	
	int nCountOld = pShare->nTypesCount;
	if (!profile.IOProfileData(L"Other", LTEXT("nTypesCount"), pShare->nTypesCount)) {
		pShare->nTypesCount = 30; // 旧バージョン読み込み用
	}
	SetValueLimit(pShare->nTypesCount, 1, MAX_TYPES);
	// 注：コントロールプロセス専用
	std::vector<TypeConfig*>& types = ShareData::getInstance().GetTypeSettings();
	for (int i=GetDllShareData().nTypesCount; i<nCountOld; ++i) {
		delete types[i];
		types[i] = nullptr;
	}
	types.resize(pShare->nTypesCount);
	for (int i=nCountOld; i<pShare->nTypesCount; ++i) {
		types[i] = new TypeConfig();
		*types[i] = *types[0]; // 基本をコピー
		auto_sprintf(types[i]->szTypeName, LS(STR_TRAY_TYPE_NAME), i);
		types[i]->nIdx = i;
		types[i]->id = i;
	}

	for (int i=0; i<pShare->nTypesCount; ++i) {
		auto_sprintf(szKey, LTEXT("Types(%d)"), i);
		TypeConfig& type = *(types[i]);
		ShareData_IO_Type_One(profile, type, szKey);
		if (profile.IsReadingMode()) {
			type.nIdx = i;
			if (i == 0) {
				pShare->typeBasis = type;
			}
			auto_strcpy(pShare->typesMini[i].szTypeExts, type.szTypeExts);
			auto_strcpy(pShare->typesMini[i].szTypeName, type.szTypeName);
			pShare->typesMini[i].id = type.id;
			pShare->typesMini[i].encoding = type.encoding;
		}
	}
	if (profile.IsReadingMode()) {
		// Id重複チェック、更新
		for (int i=0; i<pShare->nTypesCount-1; ++i) {
			TypeConfig& type = *(types[i]);
			for (int k=i+1; k<pShare->nTypesCount; ++k) {
				TypeConfig& type2 = *(types[k]);
				if (type.id == type2.id) {
					type2.id = (::GetTickCount() & 0x3fffffff) + k * 0x10000;
					pShare->typesMini[k].id = type2.id;
				}
			}
		}
	}
}

/*!
@brief 共有データのTypeConfigセクションの入出力(１個分)
	@param[in,out]	profile	INIファイル入出力クラス
	@param[in]		type		タイプ別
	@param[in]		pszSecName	セクション名

	@date 2010/04/17 Uchi ShareData_IO_TypesOneから分離。
*/
void ShareData_IO::ShareData_IO_Type_One(DataProfile& profile, TypeConfig& types, const WCHAR* pszSecName)
{
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[MAX_REGEX_KEYWORDLEN + 20];
	assert(100 < MAX_REGEX_KEYWORDLEN + 20);

	// 2005.04.07 D.S.Koba
	static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");	// MIK
	auto_strcpy(szKeyName, LTEXT("nInts"));
	if (profile.IsReadingMode()) {
		if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
			int buf[11];
			scan_ints(szKeyData, pszForm, buf);
			types.nIdx					= buf[0];
			types.nMaxLineKetas			= buf[1];
			types.nColumnSpace			= buf[2];
			types.nTabSpace				= buf[3];
			types.nKeywordSetIdx[0]		= buf[4];
			types.nKeywordSetIdx[1]		= buf[5];
			types.stringType			= (StringLiteralType)buf[6];
			types.bLineNumIsCRLF		= (buf[7] != 0);
			types.nLineTermType			= buf[8];
			types.bWordWrap				= (buf[9] != 0);
			types.nCurrentPrintSetting	= buf[10];
		}
		// 折り返し幅の最小値は10。少なくとも４ないとハングアップする。 // 20050818 aroka
		if (types.nMaxLineKetas < MINLINEKETAS) {
			types.nMaxLineKetas = MINLINEKETAS;
		}
	}else {
		auto_sprintf(szKeyData, pszForm,
			types.nIdx,
			types.nMaxLineKetas,
			types.nColumnSpace,
			types.nTabSpace,
			types.nKeywordSetIdx[0],
			types.nKeywordSetIdx[1],
			types.stringType,
			types.bLineNumIsCRLF ? 1 : 0,
			types.nLineTermType,
			types.bWordWrap ? 1 : 0,
			types.nCurrentPrintSetting
		);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
	}
	// 2005.01.13 MIK Keywordset 3-10
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect3"),  types.nKeywordSetIdx[2]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect4"),  types.nKeywordSetIdx[3]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect5"),  types.nKeywordSetIdx[4]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect6"),  types.nKeywordSetIdx[5]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect7"),  types.nKeywordSetIdx[6]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect8"),  types.nKeywordSetIdx[7]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect9"),  types.nKeywordSetIdx[8]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect10"), types.nKeywordSetIdx[9]);

	// 行間のすきま
	profile.IOProfileData(pszSecName, LTEXT("nLineSpace"), types.nLineSpace);
	if (profile.IsReadingMode()) {
		if (types.nLineSpace < /* 1 */ 0) {
			types.nLineSpace = /* 1 */ 0;
		}
		if (types.nLineSpace > LINESPACE_MAX) {
			types.nLineSpace = LINESPACE_MAX;
		}
	}

	// 行番号の最小桁数		// 加追 2014.08.02 katze
	profile.IOProfileData( pszSecName, LTEXT("nLineNumWidth"), types.nLineNumWidth );
	if (profile.IsReadingMode()) {
		if (types.nLineNumWidth < LINENUMWIDTH_MIN) {
			types.nLineNumWidth = LINENUMWIDTH_MIN;
		}
		if (types.nLineNumWidth > LINENUMWIDTH_MAX) {
			types.nLineNumWidth = LINENUMWIDTH_MAX;
		}
	}

	profile.IOProfileData(pszSecName, LTEXT("szTypeName"), MakeStringBufferT(types.szTypeName));
	profile.IOProfileData(pszSecName, LTEXT("szTypeExts"), MakeStringBufferT(types.szTypeExts));
	profile.IOProfileData(pszSecName, LTEXT("id"), types.id);
	if (types.id < 0) {
		types.id *= -1;
	}
	profile.IOProfileData(pszSecName, LTEXT("szTabViewString"), MakeStringBufferW(types.szTabViewString));
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bTabArrow")	, types.bTabArrow);	//@@@ 2003.03.26 MIK
	profile.IOProfileData(pszSecName, LTEXT("bInsSpace")			, types.bInsSpace);	// 2001.12.03 hor

	profile.IOProfileData(pszSecName, LTEXT("nTextWrapMethod"), (int&)types.nTextWrapMethod);		// 2008.05.30 nasukoji

	profile.IOProfileData(pszSecName, LTEXT("bStringLineOnly"), types.bStringLineOnly);
	profile.IOProfileData(pszSecName, LTEXT("bStringEndLine"), types.bStringEndLine);

	// From Here Sep. 28, 2002 genta / YAZAKI
	if (profile.IsReadingMode()) {
		// Block Comment
		wchar_t buffer[2][BLOCKCOMMENT_BUFFERSIZE];
		// 2004.10.02 Moca 対になるコメント設定がともに読み込まれたときだけ有効な設定と見なす．
		// ブロックコメントの始まりと終わり．行コメントの記号と桁位置
		bool bRet1, bRet2;
		buffer[0][0] = buffer[1][0] = L'\0';
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom"), MakeStringBufferW(buffer[0]));			
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo"), MakeStringBufferW(buffer[1]));
		if (bRet1 && bRet2) types.blockComments[0].SetBlockCommentRule(buffer[0], buffer[1]);

		//@@@ 2001.03.10 by MIK
		buffer[0][0] = buffer[1][0] = L'\0';
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom2"), MakeStringBufferW(buffer[0]));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo2")	, MakeStringBufferW(buffer[1]));
		if (bRet1 && bRet2) types.blockComments[1].SetBlockCommentRule(buffer[0], buffer[1]);
		
		// Line Comment
		wchar_t lbuf[COMMENT_DELIMITER_BUFFERSIZE];
		int  pos;

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment")		, MakeStringBufferW(lbuf));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn")	, pos);
		if (bRet1 && bRet2) types.lineComment.CopyTo(0, lbuf, pos);

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment2")		, MakeStringBufferW(lbuf));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn2"), pos);
		if (bRet1 && bRet2) types.lineComment.CopyTo(1, lbuf, pos);

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment3")		, MakeStringBufferW(lbuf));	// Jun. 01, 2001 JEPRO 追加
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn3"), pos);	// Jun. 01, 2001 JEPRO 追加
		if (bRet1 && bRet2) types.lineComment.CopyTo(2, lbuf, pos);
	}else { // write
		// Block Comment
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.blockComments[0].getBlockCommentFrom())));
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.blockComments[0].getBlockCommentTo())));

		//@@@ 2001.03.10 by MIK
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom2"),
			MakeStringBufferW0(const_cast<wchar_t*>(types.blockComments[1].getBlockCommentFrom())));
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo2")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.blockComments[1].getBlockCommentTo())));

		// Line Comment
		profile.IOProfileData(pszSecName, LTEXT("szLineComment")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.lineComment.getLineComment(0))));
		profile.IOProfileData(pszSecName, LTEXT("szLineComment2")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.lineComment.getLineComment(1))));
		profile.IOProfileData(pszSecName, LTEXT("szLineComment3")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.lineComment.getLineComment(2))));	// Jun. 01, 2001 JEPRO 追加

		// From here May 12, 2001 genta
		int pos;
		pos = types.lineComment.getLineCommentPos(0);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn")	, pos);
		pos = types.lineComment.getLineCommentPos(1);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn2"), pos);
		pos = types.lineComment.getLineCommentPos(2);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn3"), pos);	// Jun. 01, 2001 JEPRO 追加
		// To here May 12, 2001 genta

	}
	// To Here Sep. 28, 2002 genta / YAZAKI

	profile.IOProfileData(pszSecName, LTEXT("szIndentChars")		, MakeStringBufferW(types.szIndentChars));
	profile.IOProfileData(pszSecName, LTEXT("cLineTermChar")		, types.cLineTermChar);

	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockDisp")			, types.bOutlineDockDisp);	// アウトライン解析表示の有無
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOutlineDockSide")	, types.eOutlineDockSide);	// アウトライン解析ドッキング配置
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR		szKeyData[1024];
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData))) {
				int buf[4];
				scan_ints(szKeyData, pszForm, buf);
				types.cxOutlineDockLeft	= buf[0];
				types.cyOutlineDockTop	= buf[1];
				types.cxOutlineDockRight	= buf[2];
				types.cyOutlineDockBottom	= buf[3];
			}
		}else {
			auto_sprintf(
				szKeyData,
				pszForm,
				types.cxOutlineDockLeft,
				types.cyOutlineDockTop,
				types.cxOutlineDockRight,
				types.cyOutlineDockBottom
			);
			profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
		}
	}
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nDockOutline")		, types.nDockOutline);			// アウトライン解析方法
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nDefaultOutline")	, types.eDefaultOutline);		// アウトライン解析方法
	profile.IOProfileData(pszSecName, LTEXT("szOutlineRuleFilename")	, types.szOutlineRuleFilename);	// アウトライン解析ルールファイル
	profile.IOProfileData(pszSecName, LTEXT("nOutlineSortCol")			, types.nOutlineSortCol);		// アウトライン解析ソート列番号
	profile.IOProfileData(pszSecName, LTEXT("bOutlineSortDesc")			, types.bOutlineSortDesc);		// アウトライン解析ソート降順
	profile.IOProfileData(pszSecName, LTEXT("nOutlineSortType")			, types.nOutlineSortType);		// アウトライン解析ソート基準
	ShareData_IO_FileTree( profile, types.fileTree, pszSecName );
	profile.IOProfileData_WrapInt( pszSecName, LTEXT("nSmartIndent")	, types.eSmartIndent );			// スマートインデント種別
	// Nov. 20, 2000 genta
	profile.IOProfileData(pszSecName, LTEXT("nImeState")				, types.nImeState);	// IME制御

	// 2001/06/14 Start By asa-o: タイプ別の補完ファイル
	// Oct. 5, 2002 genta _countof()で誤ってポインタのサイズを取得していたのを修正
	profile.IOProfileData(pszSecName, LTEXT("szHokanFile")			, types.szHokanFile);		// 補完ファイル
	// 2001/06/14 End
	profile.IOProfileData(pszSecName, LTEXT("nHokanType")			, types.nHokanType);		// 補完種別

	// 2001/06/19 asa-o
	profile.IOProfileData(pszSecName, LTEXT("bHokanLoHiCase")		, types.bHokanLoHiCase);

	// 2003.06.23 Moca ファイル内からの入力補完機能
	profile.IOProfileData(pszSecName, LTEXT("bUseHokanByFile")		, types.bUseHokanByFile);
	profile.IOProfileData(pszSecName, LTEXT("bUseHokanByKeyword")	, types.bUseHokanByKeyword);

	//@@@ 2002.2.4 YAZAKI
	profile.IOProfileData(pszSecName, LTEXT("szExtHelp")			, types.szExtHelp);

	profile.IOProfileData(pszSecName, LTEXT("szExtHtmlHelp")		, types.szExtHtmlHelp);
	profile.IOProfileData(pszSecName, LTEXT("bTypeHtmlHelpIsSingle"), types.bHtmlHelpIsSingle); // 2012.06.30 Fix bHokanLoHiCase -> bHtmlHelpIsSingle

	profile.IOProfileData(pszSecName, LTEXT("bPriorCesu8")					, types.encoding.bPriorCesu8);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eDefaultCodetype")		, types.encoding.eDefaultCodetype);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eDefaultEoltype")		, types.encoding.eDefaultEoltype);
	profile.IOProfileData(pszSecName, LTEXT("bDefaultBom")					, types.encoding.bDefaultBom);

	profile.IOProfileData(pszSecName, LTEXT("bAutoIndent")				, types.bAutoIndent);
	profile.IOProfileData(pszSecName, LTEXT("bAutoIndent_ZENSPACE")		, types.bAutoIndent_ZENSPACE);
	profile.IOProfileData(pszSecName, LTEXT("bRTrimPrevLine")			, types.bRTrimPrevLine);			// 2005.10.08 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nIndentLayout")			, types.nIndentLayout);

	// 色設定 I/O
	IO_ColorSet(&profile, pszSecName, types.colorInfoArr);

	// 2010.09.17 背景画像
	profile.IOProfileData(pszSecName, L"bgImgPath", types.szBackImgPath);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPos", types.backImgPos);
	profile.IOProfileData(pszSecName, L"bgImgScrollX",   types.backImgScrollX);
	profile.IOProfileData(pszSecName, L"bgImgScrollY",   types.backImgScrollY);
	profile.IOProfileData(pszSecName, L"bgImgRepeartX",  types.backImgRepeatX);
	profile.IOProfileData(pszSecName, L"bgImgRepeartY",  types.backImgRepeatY);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPosOffsetX",  types.backImgPosOffset.x);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPosOffsetY",  types.backImgPosOffset.y);

	// 2005.11.08 Moca 指定桁縦線
	for (int j=0; j<MAX_VERTLINES; ++j) {
		auto_sprintf(szKeyName, LTEXT("nVertLineIdx%d"), j + 1);
		profile.IOProfileData(pszSecName, szKeyName, types.nVertLineIdx[j]);
		if (types.nVertLineIdx[j] == 0) {
			break;
		}
	}
	profile.IOProfileData( pszSecName, L"nNoteLineOffset", types.nNoteLineOffset );

//@@@ 2001.11.17 add start MIK
	{	// 正規表現キーワード
		WCHAR* p;
		profile.IOProfileData(pszSecName, LTEXT("bUseRegexKeyword"), types.bUseRegexKeyword);	// 正規表現キーワード使用するか？
		wchar_t* pKeyword = types.regexKeywordList;
		int nPos = 0;
		int nKeywordSize = _countof(types.regexKeywordList);
		for (int j=0; j<_countof(types.regexKeywordArr); ++j) {
			auto_sprintf(szKeyName, LTEXT("RxKey[%03d]"), j);
			if (profile.IsReadingMode()) {
				types.regexKeywordArr[j].nColorIndex = COLORIDX_REGEX1;
				if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					p = wcschr(szKeyData, LTEXT(','));
					if (p) {
						*p = LTEXT('\0');
						types.regexKeywordArr[j].nColorIndex = GetColorIndexByName(to_tchar(szKeyData));	//@@@ 2002.04.30
						if (types.regexKeywordArr[j].nColorIndex == -1)	// 名前でない
							types.regexKeywordArr[j].nColorIndex = _wtoi(szKeyData);
						++p;
						if (0 < nKeywordSize - nPos - 1) {
							wcscpyn(&pKeyword[nPos], p, nKeywordSize - nPos - 1);
						}
						if (0
							|| types.regexKeywordArr[j].nColorIndex < 0
							|| types.regexKeywordArr[j].nColorIndex >= COLORIDX_LAST
						) {
							types.regexKeywordArr[j].nColorIndex = COLORIDX_REGEX1;
						}
						if (pKeyword[nPos]) {
							nPos += auto_strlen(&pKeyword[nPos]) + 1;
						}
					}
				}else {
					// 2010.06.18 Moca 値がない場合は終了
					break;
				}
			// 2002.02.08 hor 未定義値を無視
			}else if (pKeyword[nPos]) {
				auto_sprintf(szKeyData, LTEXT("%ls,%ls"),
					GetColorNameByIndex(types.regexKeywordArr[j].nColorIndex),
					&pKeyword[nPos]);
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
				nPos += auto_strlen(&pKeyword[nPos]) + 1;
			}
		}
		if (profile.IsReadingMode()) {
			pKeyword[nPos] = L'\0';
		}
	}
//@@@ 2001.11.17 add end MIK

	// 禁則
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuHead")	, types.bKinsokuHead);
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuTail")	, types.bKinsokuTail);
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuRet")	, types.bKinsokuRet);	//@@@ 2002.04.13 MIK
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuKuto")	, types.bKinsokuKuto);	//@@@ 2002.04.17 MIK
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuHide")	, types.bKinsokuHide);	// 2012/11/30 Uchi
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuHead")	, MakeStringBufferW(types.szKinsokuHead));
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuTail")	, MakeStringBufferW(types.szKinsokuTail));
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuKuto")	, MakeStringBufferW(types.szKinsokuKuto));	// 2009.08.07 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bUseDocumentIcon")	, types.bUseDocumentIcon);	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
	{	// キーワード辞書
		WCHAR	*pH, *pT;	// <pH>keyword<pT>
		profile.IOProfileData(pszSecName, LTEXT("bUseKeywordHelp"), types.bUseKeywordHelp);			// キーワード辞書選択を使用するか？
//		profile.IOProfileData(pszSecName, LTEXT("nKeyHelpNum"), types.nKeyHelpNum);					// 登録辞書数
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpAllSearch"), types.bUseKeyHelpAllSearch);	// ヒットした次の辞書も検索(&A)
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpKeyDisp"), types.bUseKeyHelpKeyDisp);		// 1行目にキーワードも表示する(&W)
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpPrefix"), types.bUseKeyHelpPrefix);		// 選択範囲で前方一致検索(&P)
		for (int j=0; j<MAX_KEYHELP_FILE; ++j) {
			auto_sprintf(szKeyName, LTEXT("KDct[%02d]"), j);
			// 読み出し
			if (profile.IsReadingMode()) {
				types.keyHelpArr[j].bUse = false;
				types.keyHelpArr[j].szAbout[0] = _T('\0');
				types.keyHelpArr[j].szPath[0] = _T('\0');
				if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					pH = szKeyData;
					if (pT = wcschr(pH, L',')) {
						*pT = L'\0';
						types.keyHelpArr[j].bUse = (_wtoi(pH) != 0);
						pH = pT + 1;
						if (pT = wcschr(pH, L',')) {
							*pT = L'\0';
							_wcstotcs(types.keyHelpArr[j].szAbout, pH, _countof(types.keyHelpArr[j].szAbout));
							pH = pT + 1;
							if (L'\0' != (*pH)) {
								_wcstotcs(types.keyHelpArr[j].szPath, pH, _countof2(types.keyHelpArr[j].szPath));
								types.nKeyHelpNum = j + 1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
							}
						}
					}
				}
			// 書き込み
			}else {
				if (types.keyHelpArr[j].szPath[0] != _T('\0')) {
					auto_sprintf(szKeyData, LTEXT("%d,%ts,%ts"),
						types.keyHelpArr[j].bUse ? 1 : 0,
						types.keyHelpArr[j].szAbout,
						types.keyHelpArr[j].szPath.c_str()
					);
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
				}
			}
		}
		// 旧バージョンiniファイルの読み出しサポート
		if (profile.IsReadingMode()) {
			SFilePath tmp;
			if (profile.IOProfileData(pszSecName, LTEXT("szKeywordHelpFile"), tmp)) {
				types.keyHelpArr[0].szPath = tmp;
			}
		}
	}
//@@@ 2006.04.10 fon ADD-end

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	profile.IOProfileData(pszSecName, LTEXT("bChkEnterAtEnd")	, types.bChkEnterAtEnd);

	{ // フォント設定
		profile.IOProfileData(pszSecName, LTEXT("bUseTypeFont"), types.bUseTypeFont);
		ShareData_IO_Sub_LogFont(profile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
			types.lf, types.nPointSize);
	}
}

/*!
	@brief 共有データのKeywordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Keywords(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Keywords");
	WCHAR			szKeyName[64];
	WCHAR			szKeyData[1024];
	KeywordSetMgr*	pKeywordSetMgr = &pShare->common.specialKeyword.keywordSetMgr;
	int				nKeywordSetNum = pKeywordSetMgr->nKeywordSetNum;

	profile.IOProfileData(pszSecName, LTEXT("nCurrentKeywordSetIdx")	, pKeywordSetMgr->nCurrentKeywordSetIdx);
	bool bIOSuccess = profile.IOProfileData(pszSecName, LTEXT("nKeywordSetNum"), nKeywordSetNum);
	if (profile.IsReadingMode()) {
		// nKeywordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
		if (bIOSuccess) {
			// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
			// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
			pKeywordSetMgr->ResetAllKeywordSet();
			for (int i=0; i<nKeywordSetNum; ++i) {
				bool bKeywordCase = false;
				int nKeywordNum = 0;
				// 値の取得
				auto_sprintf(szKeyName, LTEXT("szSN[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
				auto_sprintf(szKeyName, LTEXT("nCASE[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, bKeywordCase);
				auto_sprintf(szKeyName, LTEXT("nKWN[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, nKeywordNum);

				// 追加
				pKeywordSetMgr->AddKeywordSet(szKeyData, bKeywordCase, nKeywordNum);
				auto_sprintf(szKeyName, LTEXT("szKW[%02d]"), i);
				std::wstring sValue;	// wstring のまま受ける（古い ini ファイルのキーワードは中身が NULL 文字区切りなので StringBufferW では NG だった）
				if (profile.IOProfileData(pszSecName, szKeyName, sValue)) {
					pKeywordSetMgr->SetKeywordArr(i, nKeywordNum, sValue.c_str());
				}
			}
		}
	}else {
		int nSize = pKeywordSetMgr->nKeywordSetNum;
		for (int i=0; i<nSize; ++i) {
			auto_sprintf(szKeyName, LTEXT("szSN[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pKeywordSetMgr->szSetNameArr[i]));
			auto_sprintf(szKeyName, LTEXT("nCASE[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, pKeywordSetMgr->bKeywordCaseArr[i]);
			auto_sprintf(szKeyName, LTEXT("nKWN[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, pKeywordSetMgr->nKeywordNumArr[i]);
			
			int nMemLen = 0;
			for (int j=0; j<pKeywordSetMgr->nKeywordNumArr[i]; ++j) {
				nMemLen += wcslen(pKeywordSetMgr->GetKeyword(i, j));
				nMemLen ++;
			}
			nMemLen ++;
			auto_sprintf(szKeyName, LTEXT("szKW[%02d].Size"), i);
			profile.IOProfileData(pszSecName, szKeyName, nMemLen);
			std::vector<wchar_t> szMem(nMemLen + 1); // May 25, 2003 genta 区切りをTABに変更したので，最後の\0の分を追加
			wchar_t* pszMem = &szMem[0];
			wchar_t* pMem = pszMem;
			for (int j=0; j<pKeywordSetMgr->nKeywordNumArr[i]; ++j) {
				// May 25, 2003 genta 区切りをTABに変更
				size_t kwlen = wcslen(pKeywordSetMgr->GetKeyword(i, j));
				auto_memcpy(pMem, pKeywordSetMgr->GetKeyword(i, j), kwlen);
				pMem += kwlen;
				*pMem++ = L'\t';
			}
			*pMem = L'\0';
			auto_sprintf(szKeyName, LTEXT("szKW[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, StringBufferW(pszMem, nMemLen));
		}
	}
}

/*!
	@brief 共有データのMacroセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Macro(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Macro");
	WCHAR szKeyName[64];
	for (int i=0; i<MAX_CUSTMACRO; ++i) {
		MacroRec& macrorec = pShare->common.macro.macroTable[i];
		// Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if (!profile.IsReadingMode() && macrorec.szName[0] == _T('\0') && macrorec.szFile[0] == _T('\0')) continue;
		auto_sprintf(szKeyName, LTEXT("Name[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(macrorec.szName));
		auto_sprintf(szKeyName, LTEXT("File[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(macrorec.szFile));
		auto_sprintf(szKeyName, LTEXT("ReloadWhenExecute[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, macrorec.bReloadWhenExecute);
	}
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnOpened"), pShare->common.macro.nMacroOnOpened);			// オープン後自動実行マクロ番号			//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnTypeChanged"), pShare->common.macro.nMacroOnTypeChanged);// タイプ変更後自動実行マクロ番号		//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnSave"), pShare->common.macro.nMacroOnSave);				// 保存前自動実行マクロ番号				//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroCancelTimer"), pShare->common.macro.nMacroCancelTimer);	// マクロ停止ダイアログ表示待ち時間		// 2011.08.04 syat
}

/*!
	@brief 共有データのStatusbarセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2008/6/21 Uchi
*/
void ShareData_IO::ShareData_IO_Statusbar(DataProfile& profile)
{
	static const WCHAR* pszSecName = LTEXT("Statusbar");
	CommonSetting_StatusBar& statusbar = GetDllShareData().common.statusBar;

	// 表示文字コードの指定
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInSjis")			, statusbar.bDispUniInSjis);		// SJISで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInJis")				, statusbar.bDispUniInJis);		// JISで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInEuc")				, statusbar.bDispUniInEuc);		// EUCで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUtf8Codepoint")			, statusbar.bDispUtf8Codepoint);	// UTF-8をコードポイントで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispSurrogatePairCodepoint")	, statusbar.bDispSPCodepoint);	// サロゲートペアをコードポイントで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispSelectCountByByte")		, statusbar.bDispSelCountByByte);	// 選択文字数を文字単位ではなくバイト単位で表示する
}

/*!
	@brief 共有データのPluginセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2009/11/30 syat
*/
void ShareData_IO::ShareData_IO_Plugin(DataProfile& profile, MenuDrawer* pMenuDrawer)
{
	static const WCHAR* pszSecName = LTEXT("Plugin");
	CommonSetting& common = GetDllShareData().common;
	CommonSetting_Plugin& plugin = GetDllShareData().common.plugin;

	profile.IOProfileData(pszSecName, LTEXT("EnablePlugin"), plugin.bEnablePlugin);		// プラグインを使用する

	// プラグインテーブル
	WCHAR	szKeyName[64];
	for (int i=0; i<MAX_PLUGIN; ++i) {
		PluginRec& pluginrec = common.plugin.pluginTable[i];

		// 2010.08.04 Moca 書き込み直前に削除フラグで削除扱いにする
		if (pluginrec.state == PLS_DELETED) {
			pluginrec.szName[0] = L'\0';
			pluginrec.szId[0] = L'\0';
		}
		auto_sprintf(szKeyName, LTEXT("P[%02d].Name"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pluginrec.szName));
		auto_sprintf(szKeyName, LTEXT("P[%02d].Id"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pluginrec.szId));
		auto_sprintf(szKeyName, LTEXT("P[%02d].CmdNum"), i);
		profile.IOProfileData(pszSecName, szKeyName, pluginrec.nCmdNum);	// 2010/7/4 Uchi
		pluginrec.state = (pluginrec.szId[0] == '\0' ? PLS_NONE : PLS_STOPPED);
		// Command 仮設定	// 2010/7/4 Uchi
		if (pluginrec.szId[0] != '\0' && pluginrec.nCmdNum >0) {
			for (int j=1; j<=pluginrec.nCmdNum; ++j) {
				pMenuDrawer->AddToolButton(MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT, Plug::GetPluginFunctionCode(i, j));
			}
		}
	}
}

struct MainMenuAddItemInfo
{
	int nVer;
	EFunctionCode nAddFuncCode;
	EFunctionCode nPrevFuncCode;
	wchar_t cAccKey;
	bool bAddPrevSeparete;
	bool bAddNextSeparete;
};

void ShareData_IO::ShareData_IO_MainMenu(DataProfile& profile)
{
	IO_MainMenu(profile, GetDllShareData().common.mainMenu, false);		// 2010/5/15 Uchi

	// 2015.02.26 Moca メインメニュー自動更新
	const WCHAR*	pszSecName = LTEXT("MainMenu");
	int& nVersion = GetDllShareData().common.mainMenu.nVersion;
	// ※メニュー定義を追加したらnCurrentVerを修正
	const int nCurrentVer = 1;
	nVersion = nCurrentVer;
	if (profile.IOProfileData(pszSecName, LTEXT("nMainMenuVer"), nVersion)) {
	}else {
		if (profile.IsReadingMode()) {
			int menuNum;
			if (profile.IOProfileData(pszSecName, LTEXT("nMainMenuNum"), menuNum)) {
				// メインメニューが定義されていた
				nVersion = 0; // 旧定義はVer0
			}else {
				// メインメニューすらない古いバージョンからのアップデートでは、最新メニューになるのでパス
			}
		}
	}
	if (profile.IsReadingMode() && nVersion < nCurrentVer) {
		CommonSetting_MainMenu& mainmenu = GetDllShareData().common.mainMenu;
		MainMenuAddItemInfo addInfos[] = {
			{1, F_FILENEW_NEWWINDOW, F_FILENEW, L'M', false, false},	// 新しいウィンドウを開く
			{1, F_CHG_CHARSET, F_TOGGLE_KEY_SEARCH, L'A', false, false},	// 文字コード変更
			{1, F_CHG_CHARSET, F_VIEWMODE, L'A', false, false}, 	// 文字コード変更(Sub)
			{1, F_FILE_REOPEN_LATIN1, F_FILE_REOPEN_EUC, L'L', false, false}, 	// Latin1で開き直す
			{1, F_FILE_REOPEN_LATIN1, F_FILE_REOPEN, L'L', false, false}, 	// Latin1で開き直す(Sub)
			{1, F_COPY_COLOR_HTML, F_COPYLINESWITHLINENUMBER, L'C', false, false}, 	// 選択範囲内色付きHTMLコピー
			{1, F_COPY_COLOR_HTML_LINENUMBER, F_COPY_COLOR_HTML, L'F', false, false}, 	// 選択範囲内行番号色付きHTMLコピー
			// 矩形選択類は省略...
			{1, F_GREP_REPLACE_DLG, F_GREP_DIALOG, L'\0', false, false}, 	// Grep置換
			{1, F_FILETREE, F_OUTLINE, L'E', false, false}, 	// ファイルツリー表示
			{1, F_FILETREE, F_OUTLINE_TOGGLE, L'E', false, false}, 	// ファイルツリー表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWSTATUSBAR, L'N', false, false}, 	// ミニマップ表示
			{1, F_SHOWMINIMAP, F_SHOWTAB, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWFUNCKEY, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_SHOWMINIMAP, F_SHOWTOOLBAR, L'N', false, false}, 	// ミニマップ表示(Sub)
			{1, F_FUNCLIST_NEXT, F_JUMPHIST_SET, L'\0', true, false}, 	// 次の関数リストマーク(セパレータ追加)
			{1, F_FUNCLIST_PREV, F_FUNCLIST_NEXT, L'\0', false, false}, 	// 前の関数リストマーク
			{1, F_MODIFYLINE_NEXT, F_FUNCLIST_PREV, L'\0', false, false}, 	// 次の変更行へ
			{1, F_MODIFYLINE_PREV, F_MODIFYLINE_NEXT, L'\0', false, false}, 	// 前の変更行へ
			{1, F_MODIFYLINE_NEXT_SEL, F_GOFILEEND_SEL, L'\0', true, false}, 	// (選択)次の変更行へ
			{1, F_MODIFYLINE_PREV_SEL, F_MODIFYLINE_NEXT_SEL, L'\0', false, false}, 	// (選択)前の変更行へ
		};
		for (int i=0; i<_countof(addInfos); ++i) {
			MainMenuAddItemInfo& item = addInfos[i];
			if (item.nVer <= nVersion) {
				continue;
			}
			MainMenu* pMenuTbl = mainmenu.mainMenuTbl;
			int k = 0;
			for (; k<mainmenu.nMainMenuNum; ++k) {
				if (pMenuTbl[k].nFunc == item.nAddFuncCode) {
					break;
				}
			}
			int nAddSep = 0;
			if (item.bAddPrevSeparete) {
				++nAddSep;
			}
			if (item.bAddNextSeparete) {
				++nAddSep;
			}
			if (k == mainmenu.nMainMenuNum && mainmenu.nMainMenuNum + nAddSep < _countof(mainmenu.mainMenuTbl)) {
				// メニュー内にまだ追加されていないので追加する
				for (int r=0; r<mainmenu.nMainMenuNum; ++r) {
					if (pMenuTbl[r].nFunc == item.nPrevFuncCode && 0 < pMenuTbl[r].nLevel) {
						// 追加分後ろにずらす
						for (int n=mainmenu.nMainMenuNum-1; r<n; --n) {
							pMenuTbl[n + 1 + nAddSep] = pMenuTbl[n];
						}
						for (int n=0; n<MAX_MAINMENU_TOP; ++n) {
							if (r < mainmenu.nMenuTopIdx[n]) {
								mainmenu.nMenuTopIdx[n] += 1 + nAddSep;
							}
						}
						MainMenu* pMenu = &pMenuTbl[r+1];
						const int nLevel = pMenuTbl[r].nLevel;
						if (item.bAddPrevSeparete) {
							pMenu->type    = MainMenuType::Separator;
							pMenu->nFunc    = F_SEPARATOR;
							pMenu->nLevel   = nLevel;
							pMenu->sName[0] = L'\0';
							pMenu->sKey[0]  = L'\0';
							pMenu->sKey[1]  = L'\0';
							++pMenu;
							mainmenu.nMainMenuNum++;
						}
						pMenu->type    = MainMenuType::Leaf;
						pMenu->nFunc    = item.nAddFuncCode;
						pMenu->nLevel   = nLevel;
						pMenu->sName[0] = L'\0';
						pMenu->sKey[0]  = L'\0';
						pMenu->sKey[1]  = L'\0';
						mainmenu.nMainMenuNum++;
						if (item.bAddNextSeparete) {
							++pMenu;
							pMenu->type    = MainMenuType::Separator;
							pMenu->nFunc    = F_SEPARATOR;
							pMenu->nLevel   = nLevel;
							pMenu->sName[0] = L'\0';
							pMenu->sKey[0]  = L'\0';
							pMenu->sKey[1]  = L'\0';
							mainmenu.nMainMenuNum++;
						}
						break;
					}
				}
			}
		}
	}
}


/*!
	@brief 共有データのMainMenuセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス
	@param[in,out]	mainmenu	共通設定MainMenuクラス
	@param[in]		bOutCmdName	出力時、名前で出力

	@date 2010/5/15 Uchi
	@date 2014.11.21 Moca pData追加。データのみのタイプを追加
*/
void ShareData_IO::IO_MainMenu(
	DataProfile& profile,
	std::vector<std::wstring>* pData,
	CommonSetting_MainMenu& mainmenu,
	bool bOutCmdName
	)
{
	static const WCHAR* pszSecName = LTEXT("MainMenu");
	WCHAR	szKeyName[64];
	WCHAR	szFuncName[MAX_PLUGIN_ID + 20];
	EFunctionCode n;
	WCHAR	szLine[1024];
	WCHAR*	p = NULL;
	WCHAR*	pn;
	std::vector<std::wstring>& data = *pData;
	int dataNum = 0;

	if (profile.IsReadingMode()) {
		int menuNum = 0;
		if (pData) {
			menuNum = (int)data.size() - 1;
		}else {
			profile.IOProfileData(pszSecName, LTEXT("nMainMenuNum"), menuNum);
		}
		if (menuNum == 0) {
			return;
		}
		mainmenu.nMainMenuNum = menuNum;
		SetValueLimit(mainmenu.nMainMenuNum, MAX_MAINMENU);
	}else {
		profile.IOProfileData(pszSecName, LTEXT("nMainMenuNum"), mainmenu.nMainMenuNum);
	}
	
	if (pData) {
		mainmenu.bMainMenuKeyParentheses = (_wtoi(data[dataNum++].c_str()) != 0);
	}else {
		profile.IOProfileData(pszSecName, LTEXT("bKeyParentheses"), mainmenu.bMainMenuKeyParentheses);
	}

	if (profile.IsReadingMode()) {
		// Top Level 初期化
		memset(mainmenu.nMenuTopIdx, -1, sizeof(mainmenu.nMenuTopIdx));
	}

	int nIdx = 0;
	for (int i=0; i<mainmenu.nMainMenuNum; ++i) {
		// メインメニューテーブル
		MainMenu* pMenu = &mainmenu.mainMenuTbl[i];

		auto_sprintf(szKeyName, LTEXT("MM[%03d]"), i);
		if (profile.IsReadingMode()) {
			// 読み込み時初期化
			pMenu->type    = MainMenuType::Node;
			pMenu->nFunc    = F_INVALID;
			pMenu->nLevel   = 0;
			pMenu->sName[0] = L'\0';
			pMenu->sKey[0]  = L'\0';
			pMenu->sKey[1]  = L'\0';

			// 読み出し
			if (pData) {
				wcscpy(szLine, data[dataNum++].c_str());
			}else {
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szLine));
			}

			// レベル
			p = szLine;
			pn = wcschr(p, L',');
			if (pn) *pn++ = L'\0';
			pMenu->nLevel = auto_atol(p);
			if (!pn) {
				continue;
			}

			// 種類
			p = pn;
			pn = wcschr(p, L',');
			if (pn) *pn++ = L'\0';
			pMenu->type = (MainMenuType)auto_atol(p);
			if (!pn) {
				continue;
			}
			
			// 機能(マクロ名対応)
			p = pn;
			pn = wcschr(p, L',');
			if (pn)	*pn++ = L'\0';
			if (wcschr(p, L'/')) {
				// Plugin名
				n = GetPlugCmdInfoByName(p);
			}else if (WCODE::Is09(*p)
			  && (WCODE::Is09(p[1]) == L'\0' ||  WCODE::Is09(p[1]))) {
				n = (EFunctionCode)auto_atol(p);
			}else {
				n = SMacroMgr::GetFuncInfoByName(0, p, NULL);
			}
			if (n == F_INVALID) {
				n = F_DEFAULT;
			}
			pMenu->nFunc = n;
			if (!pn) {
				continue;
			}

			// アクセスキー
			p = pn;
			if (*p == L',') {
				// Key なし or ,
				if (p[1] == L',') {
					// Key = ,
					pMenu->sKey[0]  = *p++;
				}
			}else {
				pMenu->sKey[0]  = *p++;
			}
			if (*p == L'\0') {
				continue;
			}

			// 表示名
			++p;
			auto_strcpy_s(pMenu->sName, MAX_MAIN_MENU_NAME_LEN + 1, p);
		}else {
			if (GetPlugCmdInfoByFuncCode(pMenu->nFunc, szFuncName)) {
				// Plugin
			}else {
				if (bOutCmdName) {
					// マクロ名対応
					p = SMacroMgr::GetFuncInfoByID(
						G_AppInstance(),
						pMenu->nFunc,
						szFuncName,
						NULL
					);
				}
				if (!bOutCmdName || !p) {
					auto_sprintf(szFuncName, L"%d", pMenu->nFunc);
				}
			}
			// 書き込み
			// ラベル編集後のノードはノード名を出力する 2012.10.14 syat 各国語対応
			auto_sprintf(szLine, L"%d,%d,%ls,%ls,%ls", 
				pMenu->nLevel, 
				pMenu->type, 
				szFuncName, 
				pMenu->sKey, 
				pMenu->nFunc == F_NODE ? pMenu->sName : L"");
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szLine));
		}

		if (profile.IsReadingMode() && pMenu->nLevel == 0) {
			// Top Level設定
			if (nIdx < MAX_MAINMENU_TOP) {
				mainmenu.nMenuTopIdx[nIdx++] = i;
			}
		}
	}
}

/*!
	@brief 共有データのOtherセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Other(DataProfile& profile)
{
	DllSharedData* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Other");	// セクションを1個作成した。2003.05.12 MIK
	WCHAR szKeyName[64];

	// **** その他のダイアログ ****
	// 外部コマンド実行の「標準出力を得る」
	if (!profile.IOProfileData(pszSecName, LTEXT("nExecFlgOpt")	, pShare->nExecFlgOpt)) { // 2006.12.03 maru オプション拡張
		profile.IOProfileData(pszSecName, LTEXT("bGetStdout")		, pShare->nExecFlgOpt);
	}

	// 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か
	profile.IOProfileData(pszSecName, LTEXT("bLineNumIsCRLF")	, pShare->bLineNumIsCRLF_ForJump);
	
	// DIFF差分表示	//@@@ 2002.05.27 MIK
	profile.IOProfileData(pszSecName, LTEXT("nDiffFlgOpt")	, pShare->nDiffFlgOpt);
	
	// CTAGS	//@@@ 2003.05.12 MIK
	profile.IOProfileData(pszSecName, LTEXT("nTagsOpt")		, pShare->nTagsOpt);
	profile.IOProfileData(pszSecName, LTEXT("szTagsCmdLine")	, MakeStringBufferT(pShare->szTagsCmdLine));
	
	// From Here 2005.04.03 MIK キーワード指定タグジャンプ
	profile.IOProfileData(pszSecName, LTEXT("_TagJumpKeyword_Counts"), pShare->tagJump.aTagJumpKeywords._GetSizeRef());
	pShare->history.aCommands.SetSizeLimit();
	int nSize = pShare->tagJump.aTagJumpKeywords.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("TagJumpKeyword[%02d]"), i);
		if (i >= nSize) {
			pShare->tagJump.aTagJumpKeywords[i][0] = 0;
		}
		profile.IOProfileData(pszSecName, szKeyName, pShare->tagJump.aTagJumpKeywords[i]);
	}
	profile.IOProfileData(pszSecName, LTEXT("bTagJumpICase")		, pShare->tagJump.bTagJumpICase);
	profile.IOProfileData(pszSecName, LTEXT("bTagJumpAnyWhere")		, pShare->tagJump.bTagJumpAnyWhere);
	// From Here 2005.04.03 MIK キーワード指定タグジャンプの

	// MIK バージョン情報（書き込みのみ）
	if (!profile.IsReadingMode()) {
		TCHAR	iniVer[256];
		auto_sprintf(iniVer, _T("%d.%d.%d.%d"), 
					HIWORD(pShare->version.dwProductVersionMS),
					LOWORD(pShare->version.dwProductVersionMS),
					HIWORD(pShare->version.dwProductVersionLS),
					LOWORD(pShare->version.dwProductVersionLS));
		profile.IOProfileData(pszSecName, LTEXT("szVersion"), MakeStringBufferT(iniVer));

		// 共有メモリバージョン	2010/5/20 Uchi
		int		nStructureVersion;
		nStructureVersion = int(pShare->vStructureVersion);
		profile.IOProfileData(pszSecName, LTEXT("vStructureVersion"), nStructureVersion);
	}
}

/*!
	@brief 色設定 I/O

	指定された色設定を指定されたセクションに書き込む。または
	指定されたセクションからいろ設定を読み込む。

	@param[in,out]	pProfile		書き出し、読み込み先Profile object (入出力方向はbReadに依存)
	@param[in]		pszSecName		セクション名
	@param[in,out]	pColorInfoArr	書き出し、読み込み対象の色設定へのポインタ (入出力方向はbReadに依存)
*/
void ShareData_IO::IO_ColorSet(DataProfile* pProfile, const WCHAR* pszSecName, ColorInfo* pColorInfoArr)
{
	WCHAR	szKeyName[256];
	WCHAR	szKeyData[1024];
	for (int j=0; j<COLORIDX_LAST; ++j) {
		static const WCHAR* pszForm = LTEXT("%d,%d,%06x,%06x,%d");
		auto_sprintf(szKeyName, LTEXT("C[%ts]"), g_ColorAttributeArr[j].szName);	// Stonee, 2001/01/12, 2001/01/15
		if (pProfile->IsReadingMode()) {
			if (pProfile->IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
				int buf[5];
				scan_ints(szKeyData, pszForm, buf);
				pColorInfoArr[j].bDisp                  = (buf[0] != 0);
				pColorInfoArr[j].fontAttr.bBoldFont  = (buf[1] != 0);
				pColorInfoArr[j].colorAttr.cTEXT     = buf[2];
				pColorInfoArr[j].colorAttr.cBACK     = buf[3];
				pColorInfoArr[j].fontAttr.bUnderLine = (buf[4] != 0);
			}else {
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 以前のiniファイルを読んだときにキャレットがテキスト背景色と同じになると
				// ちょっと困るのでキャレット色が読めないときはキャレット色をテキスト色と同じにする
				if (j == COLORIDX_CARET)
					pColorInfoArr[j].colorAttr.cTEXT = pColorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
			}
			// 2006.12.18 ryoji
			// 矛盾設定があれば修復する
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if ((fAttribute & COLOR_ATTRIB_FORCE_DISP) != 0)
				pColorInfoArr[j].bDisp = true;
			if ((fAttribute & COLOR_ATTRIB_NO_BOLD) != 0)
				pColorInfoArr[j].fontAttr.bBoldFont = false;
			if ((fAttribute & COLOR_ATTRIB_NO_UNDERLINE) != 0)
				pColorInfoArr[j].fontAttr.bUnderLine = false;
		}else {
			auto_sprintf(szKeyData, pszForm,
				pColorInfoArr[j].bDisp ? 1 : 0,
				pColorInfoArr[j].fontAttr.bBoldFont ? 1 : 0,
				pColorInfoArr[j].colorAttr.cTEXT,
				pColorInfoArr[j].colorAttr.cBACK,
				pColorInfoArr[j].fontAttr.bUnderLine ? 1 : 0
			);
			pProfile->IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void ShareData_IO_Sub_LogFont(DataProfile& profile, const WCHAR* pszSecName,
	const WCHAR* pszKeyLf, const WCHAR* pszKeyPointSize, const WCHAR* pszKeyFaceName, LOGFONT& lf, INT& nPointSize)
{
	const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
	WCHAR		szKeyData[1024];

	profile.IOProfileData(pszSecName, pszKeyPointSize, nPointSize);	// 2009.10.01 ryoji
	if (profile.IsReadingMode()) {
		if (profile.IOProfileData(pszSecName, pszKeyLf, MakeStringBufferW(szKeyData))) {
			int buf[13];
			scan_ints(szKeyData, pszForm, buf);
			lf.lfHeight			= buf[0];
			lf.lfWidth			= buf[1];
			lf.lfEscapement		= buf[2];
			lf.lfOrientation	= buf[3];
			lf.lfWeight			= buf[4];
			lf.lfItalic			= (BYTE)buf[5];
			lf.lfUnderline		= (BYTE)buf[6];
			lf.lfStrikeOut		= (BYTE)buf[7];
			lf.lfCharSet		= (BYTE)buf[8];
			lf.lfOutPrecision	= (BYTE)buf[9];
			lf.lfClipPrecision	= (BYTE)buf[10];
			lf.lfQuality		= (BYTE)buf[11];
			lf.lfPitchAndFamily	= (BYTE)buf[12];
			if (nPointSize != 0) {
				// DPI変更してもフォントのポイントサイズが変わらないように
				// ポイント数からピクセル数に変換する
				lf.lfHeight = -DpiPointsToPixels(abs(nPointSize), 10);	// pointSize: 1/10ポイント単位のサイズ
			}else {
				// 初回または古いバージョンからの更新時はポイント数をピクセル数から逆算して仮設定
				nPointSize = DpiPixelsToPoints(abs(lf.lfHeight), 10);		// （従来フォントダイアログで小数点は指定不可）
			}
		}
	}else {
		auto_sprintf(szKeyData, pszForm,
			lf.lfHeight,
			lf.lfWidth,
			lf.lfEscapement,
			lf.lfOrientation,
			lf.lfWeight,
			lf.lfItalic,
			lf.lfUnderline,
			lf.lfStrikeOut,
			lf.lfCharSet,
			lf.lfOutPrecision,
			lf.lfClipPrecision,
			lf.lfQuality,
			lf.lfPitchAndFamily
		);
		profile.IOProfileData(pszSecName, pszKeyLf, MakeStringBufferW(szKeyData));
	}
	
	profile.IOProfileData(pszSecName, pszKeyFaceName, MakeStringBufferT(lf.lfFaceName));
}


void ShareData_IO::ShareData_IO_FileTree( DataProfile& profile, FileTree& fileTree, const WCHAR* pszSecName )
{
	profile.IOProfileData( pszSecName, L"bFileTreeProject", fileTree.bProject );
	profile.IOProfileData( pszSecName, L"szFileTreeProjectIni", fileTree.szProjectIni );
	profile.IOProfileData( pszSecName, L"nFileTreeItemCount", fileTree.nItemCount );
	SetValueLimit( fileTree.nItemCount, _countof(fileTree.items) );
	for (int i=0; i<fileTree.nItemCount; ++i) {
		ShareData_IO_FileTreeItem( profile, fileTree.items[i], pszSecName, i );
	}
}

void ShareData_IO::ShareData_IO_FileTreeItem(
	DataProfile& profile, FileTreeItem& item, const WCHAR* pszSecName, int i )
{
	WCHAR szKey[64];
	auto_sprintf( szKey, L"FileTree(%d).eItemType", i );
	profile.IOProfileData_WrapInt( pszSecName, szKey, item.eFileTreeItemType );
	if (profile.IsReadingMode()
		|| item.eFileTreeItemType == FileTreeItemType::Grep
		|| item.eFileTreeItemType == FileTreeItemType::File
	) {
		auto_sprintf( szKey, L"FileTree(%d).szTargetPath", i );
		profile.IOProfileData( pszSecName, szKey, item.szTargetPath );
	}
	if (profile.IsReadingMode()
		|| ((item.eFileTreeItemType == FileTreeItemType::Grep || item.eFileTreeItemType == FileTreeItemType::File)
			&& item.szLabelName[0] != _T('\0') )
		|| item.eFileTreeItemType == FileTreeItemType::Folder
	) {
		auto_sprintf( szKey, L"FileTree(%d).szLabelName", i );
		profile.IOProfileData( pszSecName, szKey, item.szLabelName );
	}
	auto_sprintf( szKey, L"FileTree(%d).nDepth", i );
	profile.IOProfileData( pszSecName, szKey, item.nDepth );
	if (profile.IsReadingMode()
		|| item.eFileTreeItemType == FileTreeItemType::Grep
	) {
		auto_sprintf( szKey, L"FileTree(%d).szTargetFile", i );
		profile.IOProfileData( pszSecName, szKey, item.szTargetFile );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreHidden", i );
		profile.IOProfileData( pszSecName, szKey, item.bIgnoreHidden );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreReadOny", i );
		profile.IOProfileData( pszSecName, szKey, item.bIgnoreReadOnly );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreSystem", i );
		profile.IOProfileData( pszSecName, szKey, item.bIgnoreSystem );
	}
}
