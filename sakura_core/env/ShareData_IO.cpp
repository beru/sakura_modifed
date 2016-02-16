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
	//MY_RUNNINGTIMER(runningTimer, "ShareData_IO::ShareData_IO_2");
	ShareData* pShare = ShareData::getInstance();

	DataProfile	profile;

	// Feb. 12, 2006 D.S.Koba
	if (bRead) {
		profile.SetReadingMode();
	}else {
		profile.SetWritingMode();
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance()->GetProfileName());
	TCHAR szIniFileName[_MAX_PATH + 1];
	FileNameManager::getInstance()->GetIniFileName( szIniFileName, strProfileName.c_str(), bRead );	// 2007.05.19 ryoji iniファイル名を取得する

//	MYTRACE(_T("Iniファイル処理-1 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read());

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
		DLLSHAREDATA* pShareData = &GetDllShareData();
		if (0
			|| pShareData->m_version.m_dwProductVersionMS > dwMS
			|| (0
				&& pShareData->m_version.m_dwProductVersionMS == dwMS
				&& pShareData->m_version.m_dwProductVersionLS > dwLS
			)
		) {
			TCHAR szBkFileName[_countof(szIniFileName) + 4];
			::lstrcpy(szBkFileName, szIniFileName);
			::lstrcat(szBkFileName, _T(".bak"));
			::CopyFile(szIniFileName, szBkFileName, FALSE);
		}
	}
//	MYTRACE(_T("Iniファイル処理 0 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read());

	MenuDrawer* pMenuDrawer = new MenuDrawer; // 2010/7/4 Uchi

	if (bRead) {
		DLLSHAREDATA* pShareData = &GetDllShareData();
		profile.IOProfileData(L"Common", L"szLanguageDll", MakeStringBufferT(pShareData->m_common.m_window.m_szLanguageDll));
		SelectLang::ChangeLang(pShareData->m_common.m_window.m_szLanguageDll);
		pShare->RefreshString();
	}

	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru(profile);
	ShareData_IO_Keys(profile);
	ShareData_IO_Grep(profile);
	ShareData_IO_Folders(profile);
	ShareData_IO_Cmd(profile);
	ShareData_IO_Nickname(profile);
	ShareData_IO_Common(profile);
	ShareData_IO_Plugin(profile, pMenuDrawer);		// Move here	2010/6/24 Uchi
	ShareData_IO_Toolbar(profile, pMenuDrawer);
	ShareData_IO_CustMenu(profile);
	ShareData_IO_Font(profile);
	ShareData_IO_KeyBind(profile);
	ShareData_IO_Print(profile);
	ShareData_IO_Types(profile);
	ShareData_IO_KeyWords(profile);
	ShareData_IO_Macro(profile);
	ShareData_IO_Statusbar(profile);		// 2008/6/21 Uchi
	ShareData_IO_MainMenu(profile);		// 2010/5/15 Uchi
	ShareData_IO_Other(profile);

	delete pMenuDrawer;					// 2010/7/4 Uchi
	pMenuDrawer = NULL;

	if (!bRead) {
		profile.WriteProfile( szIniFileName, LTEXT(" sakura.ini テキストエディタ設定ファイル") );
	}

//	MYTRACE(_T("Iniファイル処理 8 所要時間(ミリ秒) = %d\n"), cRunningTimer.Read());
//	MYTRACE(_T("Iniファイル処理 所要時間(ミリ秒) = %d\n"), cRunningTimerStart.Read());

	return true;
}

/*!
	@brief 共有データのMruセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Mru(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	const WCHAR* pszSecName = LTEXT("MRU");
	int			i;
	int			nSize;
	EditInfo*	pfiWork;
	WCHAR		szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_MRU_Counts"), pShare->m_history.m_nMRUArrNum);
	SetValueLimit(pShare->m_history.m_nMRUArrNum, MAX_MRU);
	nSize = pShare->m_history.m_nMRUArrNum;
	for (i=0; i<nSize; ++i) {
		pfiWork = &pShare->m_history.m_fiMRUArr[i];
		if (profile.IsReadingMode()) {
			pfiWork->m_nTypeId = -1;
		}
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewTopLine"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->m_nViewTopLine);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nViewLeftCol"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->m_nViewLeftCol);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nX"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->m_ptCursor.x);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nY"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->m_ptCursor.y);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nCharCode"), i );
		profile.IOProfileData_WrapInt(pszSecName, szKeyName, pfiWork->m_nCharCode);
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szPath"), i );
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pfiWork->m_szPath));
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark2"), i );
		if (!profile.IOProfileData( pszSecName, szKeyName, MakeStringBufferW(pfiWork->m_szMarkLines) )) {
			if (profile.IsReadingMode()) {
				auto_sprintf( szKeyName, LTEXT("MRU[%02d].szMark"), i ); // 旧ver互換
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pfiWork->m_szMarkLines));
			}
		}
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].nType"), i );
		profile.IOProfileData(pszSecName, szKeyName, pfiWork->m_nTypeId);
		// お気に入り	//@@@ 2003.04.08 MIK
		auto_sprintf( szKeyName, LTEXT("MRU[%02d].bFavorite"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_bMRUArrFavorite[i]);
	}
	//@@@ 2001.12.26 YAZAKI 残りのm_fiMRUArrを初期化。
	if (profile.IsReadingMode()) {
		EditInfo	fiInit;
		// 残りをfiInitで初期化しておく。
		fiInit.m_nCharCode = CODE_DEFAULT;
		fiInit.m_nViewLeftCol = LayoutInt(0);
		fiInit.m_nViewTopLine = LayoutInt(0);
		fiInit.m_ptCursor.Set(LogicInt(0), LogicInt(0));
		_tcscpy( fiInit.m_szPath, _T("") );
		fiInit.m_szMarkLines[0] = L'\0';	// 2002.01.16 hor
		for (; i<MAX_MRU; ++i) {
			pShare->m_history.m_fiMRUArr[i] = fiInit;
			pShare->m_history.m_bMRUArrFavorite[i] = false;	// お気に入り	//@@@ 2003.04.08 MIK
		}
	}

	profile.IOProfileData(pszSecName, LTEXT("_MRUFOLDER_Counts"), pShare->m_history.m_nOPENFOLDERArrNum);
	SetValueLimit(pShare->m_history.m_nOPENFOLDERArrNum, MAX_OPENFOLDER);
	nSize = pShare->m_history.m_nOPENFOLDERArrNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf( szKeyName, LTEXT("MRUFOLDER[%02d]"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_szOPENFOLDERArr[i]);
		// お気に入り	//@@@ 2003.04.08 MIK
		wcscat(szKeyName, LTEXT(".bFavorite"));
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_bOPENFOLDERArrFavorite[i]);
	}
	// 読み込み時は残りを初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_OPENFOLDER; ++i) {
			// 2005.04.05 D.S.Koba
			pShare->m_history.m_szOPENFOLDERArr[i][0] = L'\0';
			pShare->m_history.m_bOPENFOLDERArrFavorite[i] = false;	// お気に入り	//@@@ 2003.04.08 MIK
		}
	}
	
	profile.IOProfileData(pszSecName, LTEXT("_ExceptMRU_Counts"), pShare->m_history.m_aExceptMRU._GetSizeRef());
	pShare->m_history.m_aExceptMRU.SetSizeLimit();
	nSize = pShare->m_history.m_aExceptMRU.size();
	for (i=0; i<nSize; ++i) {
		auto_sprintf( szKeyName, LTEXT("ExceptMRU[%02d]"), i );
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_aExceptMRU[i]);
	}
}

/*!
	@brief 共有データのKeysセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Keys(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Keys");
	WCHAR szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_SEARCHKEY_Counts"), pShare->m_searchKeywords.m_aSearchKeys._GetSizeRef());
	pShare->m_searchKeywords.m_aSearchKeys.SetSizeLimit();
	int nSize = pShare->m_searchKeywords.m_aSearchKeys.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("SEARCHKEY[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_searchKeywords.m_aSearchKeys[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("_REPLACEKEY_Counts"), pShare->m_searchKeywords.m_aReplaceKeys._GetSizeRef());
	pShare->m_searchKeywords.m_aReplaceKeys.SetSizeLimit();
	nSize = pShare->m_searchKeywords.m_aReplaceKeys.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("REPLACEKEY[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_searchKeywords.m_aReplaceKeys[i]);
	}
}

/*!
	@brief 共有データのGrepセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Grep(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Grep");
	int		nSize;
	WCHAR	szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("_GREPFILE_Counts"), pShare->m_searchKeywords.m_aGrepFiles._GetSizeRef());
	pShare->m_searchKeywords.m_aGrepFiles.SetSizeLimit();
	nSize = pShare->m_searchKeywords.m_aGrepFiles.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("GREPFILE[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_searchKeywords.m_aGrepFiles[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("_GREPFOLDER_Counts"), pShare->m_searchKeywords.m_aGrepFolders._GetSizeRef());
	pShare->m_searchKeywords.m_aGrepFolders.SetSizeLimit();
	nSize = pShare->m_searchKeywords.m_aGrepFolders.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("GREPFOLDER[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_searchKeywords.m_aGrepFolders[i]);
	}
}

/*!
	@brief 共有データのFoldersセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Folders(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Folders");
	// マクロ用フォルダ
	profile.IOProfileData(pszSecName, LTEXT("szMACROFOLDER"), pShare->m_common.m_macro.m_szMACROFOLDER);
	// 設定インポート用フォルダ
	profile.IOProfileData(pszSecName, LTEXT("szIMPORTFOLDER"), pShare->m_history.m_szIMPORTFOLDER);
}

/*!
	@brief 共有データのCmdセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Cmd(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Cmd");
	WCHAR szKeyName[64];

	profile.IOProfileData(pszSecName, LTEXT("nCmdArrNum"), pShare->m_history.m_aCommands._GetSizeRef());
	pShare->m_history.m_aCommands.SetSizeLimit();
	int nSize = pShare->m_history.m_aCommands.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("szCmdArr[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_aCommands[i]);
	}

	profile.IOProfileData(pszSecName, LTEXT("nCurDirArrNum"), pShare->m_history.m_aCurDirs._GetSizeRef());
	pShare->m_history.m_aCurDirs.SetSizeLimit();
	nSize = pShare->m_history.m_aCurDirs.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("szCurDirArr[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_history.m_aCurDirs[i]);
	}
}

/*!
	@brief 共有データのNicknameセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。読み込み時の初期化を修正
*/
void ShareData_IO::ShareData_IO_Nickname(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Nickname");
	int i;
	WCHAR szKeyName[64];

	profile.IOProfileData( pszSecName, LTEXT("bShortPath"), pShare->m_common.m_fileName.m_bTransformShortPath );
	profile.IOProfileData( pszSecName, LTEXT("nShortPathMaxWidth"), pShare->m_common.m_fileName.m_nTransformShortMaxWidth );
	profile.IOProfileData(pszSecName, LTEXT("ArrNum"), pShare->m_common.m_fileName.m_nTransformFileNameArrNum);
	SetValueLimit(pShare->m_common.m_fileName.m_nTransformFileNameArrNum, MAX_TRANSFORM_FILENAME);
	int nSize = pShare->m_common.m_fileName.m_nTransformFileNameArrNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("From%02d"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pShare->m_common.m_fileName.m_szTransformFileNameFrom[i]));
		auto_sprintf(szKeyName, LTEXT("To%02d"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(pShare->m_common.m_fileName.m_szTransformFileNameTo[i]));
	}
	// 読み込み時，残りをNULLで再初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_TRANSFORM_FILENAME; ++i) {
			pShare->m_common.m_fileName.m_szTransformFileNameFrom[i][0] = L'\0';
			pShare->m_common.m_fileName.m_szTransformFileNameTo[i][0]   = L'\0';
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
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Common");
	// 2005.04.07 D.S.Koba
	CommonSetting& common = pShare->m_common;

	profile.IOProfileData(pszSecName, LTEXT("nCaretType")				, common.m_general.m_nCaretType);
	// Oct. 2, 2005 genta
	// 初期値を挿入モードに固定するため，設定の読み書きをやめる
	//profile.IOProfileData(pszSecName, LTEXT("bIsINSMode")				, common.m_bIsINSMode);
	profile.IOProfileData(pszSecName, LTEXT("bIsFreeCursorMode")		, common.m_general.m_bIsFreeCursorMode);
	
	profile.IOProfileData(pszSecName, LTEXT("bStopsBothEndsWhenSearchWord")	, common.m_general.m_bStopsBothEndsWhenSearchWord);
	profile.IOProfileData(pszSecName, LTEXT("bStopsBothEndsWhenSearchParagraph")	, common.m_general.m_bStopsBothEndsWhenSearchParagraph);
	// Oct. 27, 2000 genta
	profile.IOProfileData(pszSecName, LTEXT("m_bRestoreCurPosition")	, common.m_file.m_bRestoreCurPosition);
	// 2002.01.16 hor
	profile.IOProfileData(pszSecName, LTEXT("m_bRestoreBookmarks")	, common.m_file.m_bRestoreBookmarks);
	profile.IOProfileData(pszSecName, LTEXT("bAddCRLFWhenCopy")		, common.m_edit.m_bAddCRLFWhenCopy);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOpenDialogDir")		, common.m_edit.m_eOpenDialogDir);
	profile.IOProfileData(pszSecName, LTEXT("szOpenDialogSelDir")		, StringBufferT(common.m_edit.m_OpenDialogSelDir,_countof2(common.m_edit.m_OpenDialogSelDir)));
	profile.IOProfileData( pszSecName, LTEXT("bBoxSelectLock")	, common.m_edit.m_bBoxSelectLock );
	profile.IOProfileData(pszSecName, LTEXT("nRepeatedScrollLineNum")	, common.m_general.m_nRepeatedScrollLineNum);
	profile.IOProfileData(pszSecName, LTEXT("nRepeatedScroll_Smooth")	, common.m_general.m_nRepeatedScroll_Smooth);
	profile.IOProfileData(pszSecName, LTEXT("nPageScrollByWheel")	, common.m_general.m_nPageScrollByWheel);					// 2009.01.17 nasukoji
	profile.IOProfileData(pszSecName, LTEXT("nHorizontalScrollByWheel")	, common.m_general.m_nHorizontalScrollByWheel);	// 2009.01.17 nasukoji
	profile.IOProfileData(pszSecName, LTEXT("bCloseAllConfirm")		, common.m_general.m_bCloseAllConfirm);	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bExitConfirm")			, common.m_general.m_bExitConfirm);
	profile.IOProfileData(pszSecName, LTEXT("bSearchRegularExp")	, common.m_search.m_searchOption.bRegularExp);
	profile.IOProfileData(pszSecName, LTEXT("bSearchLoHiCase")		, common.m_search.m_searchOption.bLoHiCase);
	profile.IOProfileData(pszSecName, LTEXT("bSearchWordOnly")		, common.m_search.m_searchOption.bWordOnly);
	profile.IOProfileData(pszSecName, LTEXT("bSearchConsecutiveAll")		, common.m_search.m_bConsecutiveAll);	// 2007.01.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bSearchNOTIFYNOTFOUND")	, common.m_search.m_bNOTIFYNOTFOUND);
	// 2002.01.26 hor
	profile.IOProfileData(pszSecName, LTEXT("bSearchAll")				, common.m_search.m_bSearchAll);
	profile.IOProfileData(pszSecName, LTEXT("bSearchSelectedArea")	, common.m_search.m_bSelectedArea);
	profile.IOProfileData(pszSecName, LTEXT("bGrepSubFolder")			, common.m_search.m_bGrepSubFolder);
	profile.IOProfileData( pszSecName, LTEXT("bGrepOutputLine")		, common.m_search.m_nGrepOutputLineType );
	profile.IOProfileData(pszSecName, LTEXT("nGrepOutputStyle")		, common.m_search.m_nGrepOutputStyle);
	profile.IOProfileData(pszSecName, LTEXT("bGrepOutputFileOnly")	, common.m_search.m_bGrepOutputFileOnly);
	profile.IOProfileData(pszSecName, LTEXT("bGrepOutputBaseFolder")	, common.m_search.m_bGrepOutputBaseFolder);
	profile.IOProfileData(pszSecName, LTEXT("bGrepSeparateFolder")	, common.m_search.m_bGrepSeparateFolder);
	profile.IOProfileData(pszSecName, LTEXT("bGrepDefaultFolder")		, common.m_search.m_bGrepDefaultFolder);
	profile.IOProfileData( pszSecName, LTEXT("bGrepBackup")			, common.m_search.m_bGrepBackup );
	
	// 2002/09/21 Moca 追加
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nGrepCharSet")	, common.m_search.m_nGrepCharSet);
	profile.IOProfileData(pszSecName, LTEXT("bGrepRealTime")			, common.m_search.m_bGrepRealTimeView); // 2003.06.16 Moca
	profile.IOProfileData(pszSecName, LTEXT("bCaretTextForSearch")	, common.m_search.m_bCaretTextForSearch);	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	profile.IOProfileData(pszSecName, LTEXT("m_bInheritKeyOtherView")	, common.m_search.m_bInheritKeyOtherView);
	profile.IOProfileData( pszSecName, LTEXT("nTagJumpMode")			, common.m_search.m_nTagJumpMode );
	profile.IOProfileData( pszSecName, LTEXT("nTagJumpModeKeyword")	, common.m_search.m_nTagJumpModeKeyword );
	
	// 正規表現DLL 2007.08.12 genta
	profile.IOProfileData(pszSecName, LTEXT("szRegexpLib")			, MakeStringBufferT(common.m_search.m_szRegexpLib));
	profile.IOProfileData(pszSecName, LTEXT("bGTJW_RETURN")			, common.m_search.m_bGTJW_RETURN);
	profile.IOProfileData(pszSecName, LTEXT("bGTJW_LDBLCLK")			, common.m_search.m_bGTJW_LDBLCLK);
	profile.IOProfileData(pszSecName, LTEXT("bBackUp")				, common.m_backup.m_bBackUp);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpDialog")			, common.m_backup.m_bBackUpDialog);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpFolder")			, common.m_backup.m_bBackUpFolder);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpFolderRM")		, common.m_backup.m_bBackUpFolderRM);	// 2010/5/27 Uchi
	
	if (!profile.IsReadingMode()) {
		int nDummy = _tcslen(common.m_backup.m_szBackUpFolder);
		// フォルダの最後が「半角かつ'\\'」でない場合は、付加する
		int	nCharChars = &common.m_backup.m_szBackUpFolder[nDummy]
			- NativeT::GetCharPrev(common.m_backup.m_szBackUpFolder, nDummy, &common.m_backup.m_szBackUpFolder[nDummy]);
		if (1
			&& nCharChars == 1
			&& common.m_backup.m_szBackUpFolder[nDummy - 1] == '\\'
		) {
		}else {
			_tcscat(common.m_backup.m_szBackUpFolder, _T("\\"));
		}
	}
	profile.IOProfileData(pszSecName, LTEXT("szBackUpFolder"), common.m_backup.m_szBackUpFolder);
	if (profile.IsReadingMode()) {
		int	nDummy;
		int	nCharChars;
		nDummy = _tcslen(common.m_backup.m_szBackUpFolder);
		// フォルダの最後が「半角かつ'\\'」でない場合は、付加する
		nCharChars = &common.m_backup.m_szBackUpFolder[nDummy]
			- NativeT::GetCharPrev(common.m_backup.m_szBackUpFolder, nDummy, &common.m_backup.m_szBackUpFolder[nDummy]);
		if (1
			&& nCharChars == 1
			&& common.m_backup.m_szBackUpFolder[nDummy - 1] == '\\'
		) {
		}else {
			_tcscat(common.m_backup.m_szBackUpFolder, _T("\\"));
		}
	}
	
	profile.IOProfileData(pszSecName, LTEXT("nBackUpType")				, common.m_backup.m_nBackUpType);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt1")		, common.m_backup.m_nBackUpType_Opt1);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt2")		, common.m_backup.m_nBackUpType_Opt2);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt3")		, common.m_backup.m_nBackUpType_Opt3);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpType2_Opt4")		, common.m_backup.m_nBackUpType_Opt4);
	profile.IOProfileData(pszSecName, LTEXT("bBackUpDustBox")			, common.m_backup.m_bBackUpDustBox);	//@@@ 2001.12.11 add MIK
	profile.IOProfileData(pszSecName, LTEXT("bBackUpPathAdvanced")		, common.m_backup.m_bBackUpPathAdvanced);	// 20051107 aroka
	profile.IOProfileData(pszSecName, LTEXT("szBackUpPathAdvanced")	, common.m_backup.m_szBackUpPathAdvanced);	// 20051107 aroka
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nFileShareMode")			, common.m_file.m_nFileShareMode);
	profile.IOProfileData(pszSecName, LTEXT("szExtHelp"), MakeStringBufferT(common.m_helper.m_szExtHelp));
	profile.IOProfileData(pszSecName, LTEXT("szExtHtmlHelp"), MakeStringBufferT(common.m_helper.m_szExtHtmlHelp));
	
	profile.IOProfileData(pszSecName, LTEXT("szMigemoDll"), MakeStringBufferT(common.m_helper.m_szMigemoDll));
	profile.IOProfileData(pszSecName, LTEXT("szMigemoDict"), MakeStringBufferT(common.m_helper.m_szMigemoDict));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		ShareData_IO_Sub_LogFont(profile, pszSecName, L"khlf", L"khps", L"khlfFaceName",
			common.m_helper.m_lf, common.m_helper.m_nPointSize);
	}// Keword Help Font
	
	
	profile.IOProfileData(pszSecName, LTEXT("nMRUArrNum_MAX")			, common.m_general.m_nMRUArrNum_MAX);
	SetValueLimit(common.m_general.m_nMRUArrNum_MAX, MAX_MRU);
	profile.IOProfileData(pszSecName, LTEXT("nOPENFOLDERArrNum_MAX")	, common.m_general.m_nOPENFOLDERArrNum_MAX);
	SetValueLimit(common.m_general.m_nOPENFOLDERArrNum_MAX, MAX_OPENFOLDER);
	profile.IOProfileData(pszSecName, LTEXT("bDispTOOLBAR")			, common.m_window.m_bDispTOOLBAR);
	profile.IOProfileData(pszSecName, LTEXT("bDispSTATUSBAR")			, common.m_window.m_bDispSTATUSBAR);
	profile.IOProfileData(pszSecName, LTEXT("bDispFUNCKEYWND")			, common.m_window.m_bDispFUNCKEYWND);
	profile.IOProfileData( pszSecName, LTEXT("bDispMiniMap")			, common.m_window.m_bDispMiniMap );
	profile.IOProfileData(pszSecName, LTEXT("nFUNCKEYWND_Place")		, common.m_window.m_nFUNCKEYWND_Place);
	profile.IOProfileData(pszSecName, LTEXT("nFUNCKEYWND_GroupNum")	, common.m_window.m_nFUNCKEYWND_GroupNum);		// 2002/11/04 Moca ファンクションキーのグループボタン数
	profile.IOProfileData(pszSecName, LTEXT("szLanguageDll")			, MakeStringBufferT(common.m_window.m_szLanguageDll));
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapFontSize")		, common.m_window.m_nMiniMapFontSize );
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapQuality")		, common.m_window.m_nMiniMapQuality );
	profile.IOProfileData( pszSecName, LTEXT("nMiniMapWidth")			, common.m_window.m_nMiniMapWidth );
	
	profile.IOProfileData(pszSecName, LTEXT("bDispTabWnd")			, common.m_tabBar.m_bDispTabWnd);	// タブウィンドウ	//@@@ 2003.05.31 MIK
	profile.IOProfileData(pszSecName, LTEXT("bDispTabWndMultiWin")	, common.m_tabBar.m_bDispTabWndMultiWin);	// タブウィンドウ	//@@@ 2003.05.31 MIK
	profile.IOProfileData(pszSecName, LTEXT("szTabWndCaption")		, MakeStringBufferW(common.m_tabBar.m_szTabWndCaption));	//@@@ 2003.06.13 MIK
	profile.IOProfileData(pszSecName, LTEXT("bSameTabWidth")			, common.m_tabBar.m_bSameTabWidth);	// 2006.01.28 ryoji タブを等幅にする
	profile.IOProfileData(pszSecName, LTEXT("bDispTabIcon")			, common.m_tabBar.m_bDispTabIcon);	// 2006.01.28 ryoji タブにアイコンを表示する
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bDispTabClose")	, common.m_tabBar.m_dispTabClose);	// 2012.04.14 syat
	profile.IOProfileData(pszSecName, LTEXT("bSortTabList")			, common.m_tabBar.m_bSortTabList);	// 2006.05.10 ryoji タブ一覧をソートする
	profile.IOProfileData(pszSecName, LTEXT("bTab_RetainEmptyWin")	, common.m_tabBar.m_bTab_RetainEmptyWin);	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
	profile.IOProfileData(pszSecName, LTEXT("bTab_CloseOneWin")	, common.m_tabBar.m_bTab_CloseOneWin);	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
	profile.IOProfileData(pszSecName, LTEXT("bTab_ListFull")			, common.m_tabBar.m_bTab_ListFull);	// タブ一覧をフルパス表示する	// 2007.02.28 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bChgWndByWheel")		, common.m_tabBar.m_bChgWndByWheel);	// 2006.03.26 ryoji マウスホイールでウィンドウ切り替え
	profile.IOProfileData(pszSecName, LTEXT("bNewWindow")			, common.m_tabBar.m_bNewWindow);	// 外部から起動するときは新しいウィンドウで開く
	profile.IOProfileData( pszSecName, L"bTabMultiLine"			, common.m_tabBar.m_bTabMultiLine );	// タブ多段
	profile.IOProfileData_WrapInt( pszSecName, L"eTabPosition"		, common.m_tabBar.m_eTabPosition );	// タブ位置

	ShareData_IO_Sub_LogFont(profile, pszSecName, L"lfTabFont", L"lfTabFontPs", L"lfTabFaceName",
		common.m_tabBar.m_lf, common.m_tabBar.m_nPointSize);
	
	profile.IOProfileData( pszSecName, LTEXT("nTabMaxWidth")			, common.m_tabBar.m_nTabMaxWidth );
	profile.IOProfileData( pszSecName, LTEXT("nTabMinWidth")			, common.m_tabBar.m_nTabMinWidth );
	profile.IOProfileData( pszSecName, LTEXT("nTabMinWidthOnMulti")	, common.m_tabBar.m_nTabMinWidthOnMulti );

	// 2001/06/20 asa-o 分割ウィンドウのスクロールの同期をとる
	profile.IOProfileData(pszSecName, LTEXT("bSplitterWndHScroll")	, common.m_window.m_bSplitterWndHScroll);
	profile.IOProfileData(pszSecName, LTEXT("bSplitterWndVScroll")	, common.m_window.m_bSplitterWndVScroll);
	
	profile.IOProfileData(pszSecName, LTEXT("szMidashiKigou")		, MakeStringBufferW(common.m_format.m_szMidashiKigou));
	profile.IOProfileData(pszSecName, LTEXT("szInyouKigou")			, MakeStringBufferW(common.m_format.m_szInyouKigou));
	
	// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除：３行
	profile.IOProfileData(pszSecName, LTEXT("bUseHokan")				, common.m_helper.m_bUseHokan);
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect は bGrepCharSetに統合したので削除
	// 2001/06/19 asa-o タイプ別に移動したので削除：1行
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bSaveWindowSize")	, common.m_window.m_eSaveWindowSize);	//#####フラグ名が激しくきもい
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeType")			, common.m_window.m_nWinSizeType);
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeCX")				, common.m_window.m_nWinSizeCX);
	profile.IOProfileData(pszSecName, LTEXT("nWinSizeCY")				, common.m_window.m_nWinSizeCY);
	// 2004.03.30 Moca *nWinPos*を追加
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nSaveWindowPos")	, common.m_window.m_eSaveWindowPos);	//#####フラグ名がきもい
	profile.IOProfileData(pszSecName, LTEXT("nWinPosX")				, common.m_window.m_nWinPosX);
	profile.IOProfileData(pszSecName, LTEXT("nWinPosY")				, common.m_window.m_nWinPosY);
	profile.IOProfileData(pszSecName, LTEXT("bTaskTrayUse")			, common.m_general.m_bUseTaskTray);
	profile.IOProfileData(pszSecName, LTEXT("bTaskTrayStay")			, common.m_general.m_bStayTaskTray);

	profile.IOProfileData(pszSecName, LTEXT("wTrayMenuHotKeyCode")		, common.m_general.m_wTrayMenuHotKeyCode);
	profile.IOProfileData(pszSecName, LTEXT("wTrayMenuHotKeyMods")		, common.m_general.m_wTrayMenuHotKeyMods);
	profile.IOProfileData(pszSecName, LTEXT("bUseOLE_DragDrop")			, common.m_edit.m_bUseOLE_DragDrop);
	profile.IOProfileData(pszSecName, LTEXT("bUseOLE_DropSource")			, common.m_edit.m_bUseOLE_DropSource);
	profile.IOProfileData(pszSecName, LTEXT("bDispExitingDialog")			, common.m_general.m_bDispExitingDialog);
	profile.IOProfileData(pszSecName, LTEXT("bEnableUnmodifiedOverwrite")	, common.m_file.m_bEnableUnmodifiedOverwrite);
	profile.IOProfileData(pszSecName, LTEXT("bSelectClickedURL")			, common.m_edit.m_bSelectClickedURL);
	profile.IOProfileData(pszSecName, LTEXT("bGrepExitConfirm")			, common.m_search.m_bGrepExitConfirm);// Grepモードで保存確認するか
//	profile.IOProfileData(pszSecName, LTEXT("bRulerDisp")					, common.m_bRulerDisp);					// ルーラー表示
	profile.IOProfileData(pszSecName, LTEXT("nRulerHeight")				, common.m_window.m_nRulerHeight);		// ルーラー高さ
	profile.IOProfileData(pszSecName, LTEXT("nRulerBottomSpace")			, common.m_window.m_nRulerBottomSpace);	// ルーラーとテキストの隙間
	profile.IOProfileData(pszSecName, LTEXT("nRulerType")					, common.m_window.m_nRulerType);			// ルーラーのタイプ
	// Sep. 18, 2002 genta 追加
	profile.IOProfileData(pszSecName, LTEXT("nLineNumberRightSpace")	, common.m_window.m_nLineNumRightSpace);	// 行番号の右側の隙間
	profile.IOProfileData(pszSecName, LTEXT("nVertLineOffset")			, common.m_window.m_nVertLineOffset);		// 2005.11.10 Moca
	profile.IOProfileData(pszSecName, LTEXT("bUseCompotibleBMP")		, common.m_window.m_bUseCompatibleBMP);	// 2007.09.09 Moca
	profile.IOProfileData(pszSecName, LTEXT("bCopyAndDisablSelection")	, common.m_edit.m_bCopyAndDisablSelection);	// コピーしたら選択解除
	profile.IOProfileData(pszSecName, LTEXT("bEnableNoSelectCopy")		, common.m_edit.m_bEnableNoSelectCopy);		// 選択なしでコピーを可能にする	// 2007.11.18 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bEnableLineModePaste")	, common.m_edit.m_bEnableLineModePaste);		// ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bConvertEOLPaste")		, common.m_edit.m_bConvertEOLPaste);			// 改行コードを変換して貼り付ける	// 2009.02.28 salarm
	profile.IOProfileData(pszSecName, LTEXT("bEnableExtEol")			, common.m_edit.m_bEnableExtEol);
	
	profile.IOProfileData(pszSecName, LTEXT("bHtmlHelpIsSingle")		, common.m_helper.m_bHtmlHelpIsSingle);		// HtmlHelpビューアはひとつ
	profile.IOProfileData(pszSecName, LTEXT("bCompareAndTileHorz")		, common.m_compare.m_bCompareAndTileHorz);	// 文書比較後、左右に並べて表示	// Oct. 10, 2000 JEPRO チェックボックスをボタン化すればこの行は不要のはず
	profile.IOProfileData(pszSecName, LTEXT("bDropFileAndClose")		, common.m_file.m_bDropFileAndClose);			// ファイルをドロップしたときは閉じて開く
	profile.IOProfileData(pszSecName, LTEXT("nDropFileNumMax")			, common.m_file.m_nDropFileNumMax);			// 一度にドロップ可能なファイル数
	profile.IOProfileData(pszSecName, LTEXT("bCheckFileTimeStamp")		, common.m_file.m_bCheckFileTimeStamp);		// 更新の監視
	profile.IOProfileData(pszSecName, LTEXT("nAutoloadDelay")			, common.m_file.m_nAutoloadDelay);			// 自動読込時遅延
	profile.IOProfileData(pszSecName, LTEXT("bUneditableIfUnwritable")	, common.m_file.m_bUneditableIfUnwritable);	// 上書き禁止検出時は編集禁止にする
	profile.IOProfileData(pszSecName, LTEXT("bNotOverWriteCRLF")		, common.m_edit.m_bNotOverWriteCRLF);			// 改行は上書きしない
	profile.IOProfileData(pszSecName, LTEXT("bOverWriteFixMode")		, common.m_edit.m_bOverWriteFixMode);			// 文字幅に合わせてスペースを詰める
	profile.IOProfileData(pszSecName, LTEXT("bOverWriteBoxDelete")		, common.m_edit.m_bOverWriteBoxDelete);
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgFind")		, common.m_search.m_bAutoCloseDlgFind);		// 検索ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgFuncList")	, common.m_outline.m_bAutoCloseDlgFuncList);	// アウトライン ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoCloseDlgReplace")	, common.m_search.m_bAutoCloseDlgReplace);	// 置換 ダイアログを自動的に閉じる
	profile.IOProfileData(pszSecName, LTEXT("bAutoColmnPaste")			, common.m_edit.m_bAutoColumnPaste);			// 矩形コピーのテキストは常に矩形貼り付け // 2013.5.23 aroka iniファイルのtypo未修正
	profile.IOProfileData(pszSecName, LTEXT("NoCaretMoveByActivation")	, common.m_general.m_bNoCaretMoveByActivation);// マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta)
	profile.IOProfileData(pszSecName, LTEXT("bScrollBarHorz")			, common.m_window.m_bScrollBarHorz);			// 水平スクロールバーを使う

	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_RETURN")		, common.m_helper.m_bHokanKey_RETURN);		// VK_RETURN 補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_TAB")			, common.m_helper.m_bHokanKey_TAB);			// VK_TAB    補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_RIGHT")			, common.m_helper.m_bHokanKey_RIGHT);			// VK_RIGHT  補完決定キーが有効/無効
	profile.IOProfileData(pszSecName, LTEXT("bHokanKey_SPACE")			, common.m_helper.m_bHokanKey_SPACE);			// VK_SPACE  補完決定キーが有効/無効
	
	profile.IOProfileData(pszSecName, LTEXT("nDateFormatType")			, common.m_format.m_nDateFormatType);			// 日付書式のタイプ
	profile.IOProfileData(pszSecName, LTEXT("szDateFormat")			, MakeStringBufferT(common.m_format.m_szDateFormat));	// 日付書式
	profile.IOProfileData(pszSecName, LTEXT("nTimeFormatType")			, common.m_format.m_nTimeFormatType);			// 時刻書式のタイプ
	profile.IOProfileData(pszSecName, LTEXT("szTimeFormat")			, MakeStringBufferT(common.m_format.m_szTimeFormat));	// 時刻書式
	
	profile.IOProfileData(pszSecName, LTEXT("bMenuIcon")				, common.m_window.m_bMenuIcon);			// メニューにアイコンを表示する
	profile.IOProfileData(pszSecName, LTEXT("bAutoMIMEdecode")			, common.m_file.m_bAutoMIMEdecode);			// ファイル読み込み時にMIMEのdecodeを行うか
	profile.IOProfileData(pszSecName, LTEXT("bQueryIfCodeChange")		, common.m_file.m_bQueryIfCodeChange);		// Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うか
	profile.IOProfileData(pszSecName, LTEXT("bAlertIfFileNotExist")	, common.m_file.m_bAlertIfFileNotExist);	// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	
	profile.IOProfileData(pszSecName, LTEXT("bNoFilterSaveNew")		, common.m_file.m_bNoFilterSaveNew);	// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bNoFilterSaveFile")		, common.m_file.m_bNoFilterSaveFile);	// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bAlertIfLargeFile")		, common.m_file.m_bAlertIfLargeFile);	// 開こうとしたファイルが大きい場合に警告する
	profile.IOProfileData(pszSecName, LTEXT("nAlertFileSize")			, common.m_file.m_nAlertFileSize);	// 警告を開始するファイルサイズ(MB単位)
	
	//「開く」ダイアログのサイズと位置
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcOpenDialog")		, common.m_others.m_rcOpenDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcCompareDialog")	, common.m_others.m_rcCompareDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcDiffDialog")		, common.m_others.m_rcDiffDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcFavoriteDialog")	, common.m_others.m_rcFavoriteDialog);
	ShareData_IO_RECT(profile,  pszSecName, LTEXT("rcTagJumpDialog")	, common.m_others.m_rcTagJumpDialog);
	
	// 2002.02.08 aroka,hor
	profile.IOProfileData(pszSecName, LTEXT("bMarkUpBlankLineEnable")	, common.m_outline.m_bMarkUpBlankLineEnable);
	profile.IOProfileData(pszSecName, LTEXT("bFunclistSetFocusOnJump")	, common.m_outline.m_bFunclistSetFocusOnJump);
	
	// Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	profile.IOProfileData(pszSecName, LTEXT("szWinCaptionActive") , MakeStringBufferT(common.m_window.m_szWindowCaptionActive));
	profile.IOProfileData(pszSecName, LTEXT("szWinCaptionInactive"), MakeStringBufferT(common.m_window.m_szWindowCaptionInactive));
	
	// アウトライン/トピックリスト の位置とサイズを記憶  20060201 aroka
	profile.IOProfileData(pszSecName, LTEXT("bRememberOutlineWindowPos"), common.m_outline.m_bRememberOutlineWindowPos);
	if (common.m_outline.m_bRememberOutlineWindowPos) {
		profile.IOProfileData(pszSecName, LTEXT("widthOutlineWindow")	, common.m_outline.m_widthOutlineWindow);
		profile.IOProfileData(pszSecName, LTEXT("heightOutlineWindow"), common.m_outline.m_heightOutlineWindow);
		profile.IOProfileData(pszSecName, LTEXT("xOutlineWindowPos")	, common.m_outline.m_xOutlineWindowPos);
		profile.IOProfileData(pszSecName, LTEXT("yOutlineWindowPos")	, common.m_outline.m_yOutlineWindowPos);
	}
	profile.IOProfileData(pszSecName, LTEXT("nOutlineDockSet"), common.m_outline.m_nOutlineDockSet);
	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockSync"), common.m_outline.m_bOutlineDockSync);
	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockDisp"), common.m_outline.m_bOutlineDockDisp);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOutlineDockSide"), common.m_outline.m_eOutlineDockSide);
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR szKeyData[1024];
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData))) {
				int buf[4];
				scan_ints(szKeyData, pszForm, buf);
				common.m_outline.m_cxOutlineDockLeft	= buf[0];
				common.m_outline.m_cyOutlineDockTop	= buf[1];
				common.m_outline.m_cxOutlineDockRight	= buf[2];
				common.m_outline.m_cyOutlineDockBottom	= buf[3];
			}
		}else {
			auto_sprintf(
				szKeyData,
				pszForm,
				common.m_outline.m_cxOutlineDockLeft,
				common.m_outline.m_cyOutlineDockTop,
				common.m_outline.m_cxOutlineDockRight,
				common.m_outline.m_cyOutlineDockBottom
			);
			profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
		}
	}
	profile.IOProfileData(pszSecName, LTEXT("nDockOutline"), common.m_outline.m_nDockOutline);
	ShareData_IO_FileTree( profile, common.m_outline.m_fileTree, pszSecName );
	profile.IOProfileData( pszSecName, LTEXT("szFileTreeDefIniName"), common.m_outline.m_fileTreeDefIniName );
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

	CommonSetting_Plugin& plugin = GetDllShareData().m_common.m_plugin;
	int nId = -1;
	for (int i=0; i<MAX_PLUGIN; ++i) {
		PluginRec& pluginrec = plugin.m_pluginTable[i];
		if (auto_strcmp(pluginrec.m_szId, sPluginName) == 0) {
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
	CommonSetting_Plugin& plugin = GetDllShareData().m_common.m_plugin;
	auto_sprintf(pszFuncName, L"%ls/%02d", plugin.m_pluginTable[nID].m_szId, nNo);

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
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Toolbar");
	int		i;
	WCHAR	szKeyName[64];
	CommonSetting_ToolBar& toolbar = pShare->m_common.m_toolBar;

	EFunctionCode	eFunc;
	WCHAR			szText[MAX_PLUGIN_ID + 20];
	int				nInvalid = -1;

	profile.IOProfileData(pszSecName, LTEXT("bToolBarIsFlat"), toolbar.m_bToolBarIsFlat);

	profile.IOProfileData(pszSecName, LTEXT("nToolBarButtonNum"), toolbar.m_nToolBarButtonNum);
	SetValueLimit(toolbar.m_nToolBarButtonNum, MAX_TOOLBAR_BUTTON_ITEMS);
	int	nSize = toolbar.m_nToolBarButtonNum;
	for (i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("nTBB[%03d]"), i);
		// Plugin String Parametor
		if (profile.IsReadingMode()) {
			// 読み込み
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szText));
			if (!wcschr(szText, L'/')) {
				// 番号
				toolbar.m_nToolBarButtonIdxArr[i] = _wtoi(szText);
			}else {
				// Plugin
				eFunc = GetPlugCmdInfoByName(szText);
				if (eFunc == F_INVALID) {
					toolbar.m_nToolBarButtonIdxArr[i] = -1;		// 未解決
				}else {
					toolbar.m_nToolBarButtonIdxArr[i] = pMenuDrawer->FindToolbarNoFromCommandId(eFunc, false);
				}
			}
		}else {
			// 書き込み
			if (toolbar.m_nToolBarButtonIdxArr[i] <= MAX_TOOLBAR_ICON_COUNT + 1) {	// +1はセパレータ分
				profile.IOProfileData(pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i]);	
			}else {
				// Plugin
				eFunc = (EFunctionCode)toolbar.m_nToolBarButtonIdxArr[i];
				if (eFunc == F_DEFAULT) {
					profile.IOProfileData(pszSecName, szKeyName, nInvalid);	
				}else if (GetPlugCmdInfoByFuncCode(eFunc, szText)) {
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szText));	
				}else {
					profile.IOProfileData(pszSecName, szKeyName, toolbar.m_nToolBarButtonIdxArr[i]);	
				}
			}
		}
	}
	// 読み込み時は残りを初期化
	if (profile.IsReadingMode()) {
		for (; i<MAX_TOOLBAR_BUTTON_ITEMS; ++i) {
			toolbar.m_nToolBarButtonIdxArr[i] = 0;
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
	IO_CustMenu(profile, GetDllShareData().m_common.m_customMenu, false);
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
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(menu.m_szCustMenuNameArr[i]));	// Oct. 15, 2001 genta 最大長指定
		auto_sprintf(szKeyName, LTEXT("bCMPOP[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, menu.m_bCustMenuPopupArr[i]);
		auto_sprintf(szKeyName, LTEXT("nCMIN[%02d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, menu.m_nCustMenuItemNumArr[i]);
		SetValueLimit(menu.m_nCustMenuItemNumArr[i], _countof(menu.m_nCustMenuItemFuncArr[0]));
		int nSize = menu.m_nCustMenuItemNumArr[i];
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
				menu.m_nCustMenuItemFuncArr[i][j] = n;
			}else {
				if (GetPlugCmdInfoByFuncCode(menu.m_nCustMenuItemFuncArr[i][j], szFuncName)) {
					// Plugin
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
				}else {
					if (bOutCmdName) {
						WCHAR* p = SMacroMgr::GetFuncInfoByID(
							G_AppInstance(),
							menu.m_nCustMenuItemFuncArr[i][j],
							szFuncName,
							NULL
						);
						if (!p) {
							auto_sprintf(szFuncName, L"%d", menu.m_nCustMenuItemFuncArr[i][j]);
						}
						profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szFuncName));
					}else {
						profile.IOProfileData_WrapInt(pszSecName, szKeyName, menu.m_nCustMenuItemFuncArr[i][j]);
					}
				}
			}
			// end

			auto_sprintf(szKeyName, LTEXT("nCMIK[%02d][%02d]"), i, j);
			profile.IOProfileData(pszSecName, szKeyName, menu.m_nCustMenuItemKeyArr[i][j]);
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
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Font");
	CommonSetting_View& view = pShare->m_common.m_view;
	ShareData_IO_Sub_LogFont(profile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
		view.m_lf, view.m_nPointSize);

	profile.IOProfileData(pszSecName, LTEXT("bFontIs_FIXED_PITCH"), view.m_bFontIs_FIXED_PITCH);
}

/*!
	@brief 共有データのKeyBindセクションの入出力
*/
void ShareData_IO::ShareData_IO_KeyBind(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	IO_KeyBind(profile, pShare->m_common.m_keyBind, false);	// add Parameter 2008/5/24
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
//	int		nSize = m_pShareData->m_nKeyNameArrNum;
	WCHAR	szWork[MAX_PLUGIN_ID + 20 + 4];
	bool	bOldVer = false;
	const int KEYNAME_SIZE = _countof(keyBind.m_pKeyNameArr) - 1;// 最後の１要素はダミー用に予約 2012.11.25 aroka
	int nKeyNameArrUsed = keyBind.m_nKeyNameArrNum; // 使用済み領域

	if (profile.IsReadingMode()) { 
		if (!profile.IOProfileData(szSecName, L"KeyBind[000]", MakeStringBufferW(szKeyData))) {
			bOldVer = true;
		}else {
			// 新スタイルのImportは割り当て表サイズぎりぎりまで読み込む
			// 旧スタイルは初期値と一致しないKeyNameは捨てるのでデータ数に変化なし
			keyBind.m_nKeyNameArrNum = KEYNAME_SIZE;
		}
	}

	for (int i=0; i<keyBind.m_nKeyNameArrNum; ++i) {
		// 2005.04.07 D.S.Koba
		//KEYDATA& keydata = m_pShareData->m_pKeyNameArr[i];
		//KEYDATA& keydata = keyBind.ppKeyNameArr[i];
		
		if (profile.IsReadingMode()) {
			if (bOldVer) {
				KEYDATA& keydata = keyBind.m_pKeyNameArr[i];
				_tcstowcs(szKeyName, keydata.m_szKeyName, _countof(szKeyName));
				if (profile.IOProfileData(szSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					int buf[8];
					scan_ints(szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"), buf);
					keydata.m_nFuncCodeArr[0]	= (EFunctionCode)buf[0];
					keydata.m_nFuncCodeArr[1]	= (EFunctionCode)buf[1];
					keydata.m_nFuncCodeArr[2]	= (EFunctionCode)buf[2];
					keydata.m_nFuncCodeArr[3]	= (EFunctionCode)buf[3];
					keydata.m_nFuncCodeArr[4]	= (EFunctionCode)buf[4];
					keydata.m_nFuncCodeArr[5]	= (EFunctionCode)buf[5];
					keydata.m_nFuncCodeArr[6]	= (EFunctionCode)buf[6];
					keydata.m_nFuncCodeArr[7]	= (EFunctionCode)buf[7];
				}
			}else {		// 新バージョン(キー割り当てのImport,export の合わせた)	2008/5/25 Uchi
				KEYDATA tmpKeydata;
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
					tmpKeydata.m_nKeyCode = (short)keycode;
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
						tmpKeydata.m_nFuncCodeArr[j] = n;
						p = pn + 1;
					}
					// KeyName
					auto_strncpy(tmpKeydata.m_szKeyName, to_tchar(p), _countof(tmpKeydata.m_szKeyName) - 1);
					tmpKeydata.m_szKeyName[_countof(tmpKeydata.m_szKeyName) - 1] = '\0';

					if (tmpKeydata.m_nKeyCode <= 0) { // マウスコードは先頭に固定されている KeyCodeが同じなのでKeyNameで判別
						// 2013.10.23 syat マウスのキーコードを拡張仮想キーコードに変更。以下は互換性のため残す。
						for (int im=0; im<jpVKEXNamesLen; ++im) {
							if (_tcscmp(tmpKeydata.m_szKeyName, jpVKEXNames[im]) == 0) {
								_tcscpy(tmpKeydata.m_szKeyName, keyBind.m_pKeyNameArr[im].m_szKeyName);
								keyBind.m_pKeyNameArr[im + 0x0100] = tmpKeydata;
							}
						}
					}else {
						// 割り当て済みキーコードは上書き
						int idx = keyBind.m_VKeyToKeyNameArr[tmpKeydata.m_nKeyCode];
						if (idx != KEYNAME_SIZE) {
							_tcscpy(tmpKeydata.m_szKeyName, keyBind.m_pKeyNameArr[idx].m_szKeyName);
							keyBind.m_pKeyNameArr[idx] = tmpKeydata;
						}else {// 未割り当てキーコードは末尾に追加
							if (nKeyNameArrUsed >= KEYNAME_SIZE) {
							}else {
								_tcscpy(tmpKeydata.m_szKeyName, keyBind.m_pKeyNameArr[nKeyNameArrUsed].m_szKeyName);
								keyBind.m_pKeyNameArr[nKeyNameArrUsed] = tmpKeydata;
								keyBind.m_VKeyToKeyNameArr[tmpKeydata.m_nKeyCode] = (BYTE)nKeyNameArrUsed++;
							}
						}
					}
				}
			}
		}else {
		//	auto_sprintf(szKeyData, LTEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
		//		keydata.m_nFuncCodeArr[0],
		//		keydata.m_nFuncCodeArr[1],
		//		keydata.m_nFuncCodeArr[2],
		//		keydata.m_nFuncCodeArr[3],
		//		keydata.m_nFuncCodeArr[4],
		//		keydata.m_nFuncCodeArr[5],
		//		keydata.m_nFuncCodeArr[6],
		//		keydata.m_nFuncCodeArr[7]
		//	);
		//	profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));

// start 新バージョン	2008/5/25 Uchi
			KEYDATA& keydata = keyBind.m_pKeyNameArr[i];
			auto_sprintf(szKeyName, L"KeyBind[%03d]", i);
			auto_sprintf(szKeyData, L"%04x", keydata.m_nKeyCode);
			for (int j=0; j<8; ++j) {
				WCHAR	szFuncName[256];
				if (GetPlugCmdInfoByFuncCode(keydata.m_nFuncCodeArr[j], szFuncName)) {
					// Plugin
					auto_sprintf(szWork, L",%ls", szFuncName);
				}else {
					if (bOutCmdName) {
						//@@@ 2002.2.2 YAZAKI マクロをSMacroMgrに統一
						// 2010.06.30 Moca 日本語名を取得しないように
						WCHAR* p = SMacroMgr::GetFuncInfoByID(
							0,
							keydata.m_nFuncCodeArr[j],
							szFuncName,
							NULL
						);
						if (p) {
							auto_sprintf(szWork, L",%ls", p);
						}else {
							auto_sprintf(szWork, L",%d", keydata.m_nFuncCodeArr[j]);
						}
					}else {
						auto_sprintf(szWork, L",%d", keydata.m_nFuncCodeArr[j]);
					}
				}
				wcscat(szKeyData, szWork);
			}

			if (0x0100 <= keydata.m_nKeyCode) {
				auto_sprintf(szWork, L",%ts", jpVKEXNames[keydata.m_nKeyCode - 0x0100]);
			}else {
				auto_sprintf(szWork, L",%ts", keydata.m_szKeyName);
			}
			wcscat(szKeyData, szWork);
			profile.IOProfileData(szSecName, szKeyName, MakeStringBufferW(szKeyData));
//
		}
	}

	if (profile.IsReadingMode()) {
		keyBind.m_nKeyNameArrNum = nKeyNameArrUsed;
	}
}

/*!
	@brief 共有データのPrintセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_Print(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Print");
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	for (int i=0; i<MAX_PRINTSETTINGARR; ++i) {
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = pShare->m_printSettingArr[i];
		auto_sprintf(szKeyName, LTEXT("PS[%02d].nInts"), i);
		static const WCHAR* pszForm = LTEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d");
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
				int buf[19];
				scan_ints(szKeyData, pszForm, buf);
				printsetting.m_nPrintFontWidth			= buf[0];
				printsetting.m_nPrintFontHeight			= buf[1];
				printsetting.m_nPrintDansuu				= buf[2];
				printsetting.m_nPrintDanSpace			= buf[3];
				printsetting.m_nPrintLineSpacing		= buf[4];
				printsetting.m_nPrintMarginTY			= buf[5];
				printsetting.m_nPrintMarginBY			= buf[6];
				printsetting.m_nPrintMarginLX			= buf[7];
				printsetting.m_nPrintMarginRX			= buf[8];
				printsetting.m_nPrintPaperOrientation	= (short)buf[9];
				printsetting.m_nPrintPaperSize			= (short)buf[10];
				printsetting.m_bPrintWordWrap			= (buf[11] != 0);
				printsetting.m_bPrintLineNumber			= (buf[12] != 0);
				printsetting.m_bHeaderUse[0]			= buf[13];
				printsetting.m_bHeaderUse[1]			= buf[14];
				printsetting.m_bHeaderUse[2]			= buf[15];
				printsetting.m_bFooterUse[0]			= buf[16];
				printsetting.m_bFooterUse[1]			= buf[17];
				printsetting.m_bFooterUse[2]			= buf[18];
			}
		}else {
			auto_sprintf(szKeyData, pszForm,
				printsetting.m_nPrintFontWidth		,
				printsetting.m_nPrintFontHeight		,
				printsetting.m_nPrintDansuu			,
				printsetting.m_nPrintDanSpace			,
				printsetting.m_nPrintLineSpacing		,
				printsetting.m_nPrintMarginTY			,
				printsetting.m_nPrintMarginBY			,
				printsetting.m_nPrintMarginLX			,
				printsetting.m_nPrintMarginRX			,
				printsetting.m_nPrintPaperOrientation	,
				printsetting.m_nPrintPaperSize		,
				printsetting.m_bPrintWordWrap ? 1 : 0,
				printsetting.m_bPrintLineNumber ? 1 : 0,
				printsetting.m_bHeaderUse[0] ? 1 : 0,
				printsetting.m_bHeaderUse[1] ? 1 : 0,
				printsetting.m_bHeaderUse[2] ? 1 : 0,
				printsetting.m_bFooterUse[0] ? 1 : 0,
				printsetting.m_bFooterUse[1] ? 1 : 0,
				printsetting.m_bFooterUse[2] ? 1 : 0
			);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
		}

		auto_sprintf(szKeyName, LTEXT("PS[%02d].szSName")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintSettingName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szFF")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintFontFaceHan));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szFFZ")	, i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_szPrintFontFaceZen));
		// ヘッダ/フッタ
		for (int j=0; j<3; ++j) {
			auto_sprintf(szKeyName, LTEXT("PS[%02d].szHF[%d]") , i, j);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szHeaderForm[j]));
			auto_sprintf(szKeyName, LTEXT("PS[%02d].szFTF[%d]"), i, j);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(printsetting.m_szFooterForm[j]));
		}
		{ // ヘッダ/フッタ フォント設定
			WCHAR	szKeyName2[64];
			WCHAR	szKeyName3[64];
			auto_sprintf(szKeyName,  LTEXT("PS[%02d].lfHeader"),			i);
			auto_sprintf(szKeyName2, LTEXT("PS[%02d].nHeaderPointSize"),	i);
			auto_sprintf(szKeyName3, LTEXT("PS[%02d].lfHeaderFaceName"),	i);
			ShareData_IO_Sub_LogFont(profile, pszSecName, szKeyName, szKeyName2, szKeyName3,
				printsetting.m_lfHeader, printsetting.m_nHeaderPointSize);
			auto_sprintf(szKeyName,  LTEXT("PS[%02d].lfFooter"),			i);
			auto_sprintf(szKeyName2, LTEXT("PS[%02d].nFooterPointSize"),	i);
			auto_sprintf(szKeyName3, LTEXT("PS[%02d].lfFooterFaceName"),	i);
			ShareData_IO_Sub_LogFont(profile, pszSecName, szKeyName, szKeyName2, szKeyName3,
				printsetting.m_lfFooter, printsetting.m_nFooterPointSize);
		}

		auto_sprintf(szKeyName, LTEXT("PS[%02d].szDriver"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterDriverName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szDevice"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterDeviceName));
		auto_sprintf(szKeyName, LTEXT("PS[%02d].szOutput"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(printsetting.m_mdmDevMode.m_szPrinterOutputName));

		// 2002.02.16 hor とりあえず旧設定を変換しとく
		if (wcscmp(printsetting.m_szHeaderForm[0], _EDITL("&f")) == 0 &&
			wcscmp(printsetting.m_szFooterForm[0], _EDITL("&C- &P -")) == 0
		) {
			auto_strcpy(printsetting.m_szHeaderForm[0], _EDITL("$f"));
			auto_strcpy(printsetting.m_szFooterForm[0], _EDITL(""));
			auto_strcpy(printsetting.m_szFooterForm[1], _EDITL("- $p -"));
		}

		// 禁則	//@@@ 2002.04.09 MIK
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuHead"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead);
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuTail"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail);
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuRet"),  i); profile.IOProfileData(pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet);	//@@@ 2002.04.13 MIK
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bKinsokuKuto"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto);	//@@@ 2002.04.17 MIK

		// カラー印刷
		auto_sprintf(szKeyName, LTEXT("PS[%02d].bColorPrint"), i); profile.IOProfileData(pszSecName, szKeyName, printsetting.m_bColorPrint);	// 2013/4/26 Uchi
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
	DLLSHAREDATA* pShare = &GetDllShareData();
	WCHAR szKey[32];
	
	int nCountOld = pShare->m_nTypesCount;
	if (!profile.IOProfileData(L"Other", LTEXT("nTypesCount"), pShare->m_nTypesCount)) {
		pShare->m_nTypesCount = 30; // 旧バージョン読み込み用
	}
	SetValueLimit(pShare->m_nTypesCount, 1, MAX_TYPES);
	// 注：コントロールプロセス専用
	std::vector<TypeConfig*>& types = ShareData::getInstance()->GetTypeSettings();
	for (int i=GetDllShareData().m_nTypesCount; i<nCountOld; ++i) {
		delete types[i];
		types[i] = NULL;
	}
	types.resize(pShare->m_nTypesCount);
	for (int i=nCountOld; i<pShare->m_nTypesCount; ++i) {
		types[i] = new TypeConfig();
		*types[i] = *types[0]; // 基本をコピー
		auto_sprintf(types[i]->m_szTypeName, LS(STR_TRAY_TYPE_NAME), i);
		types[i]->m_nIdx = i;
		types[i]->m_id = i;
	}

	for (int i=0; i<pShare->m_nTypesCount; ++i) {
		auto_sprintf(szKey, LTEXT("Types(%d)"), i);
		TypeConfig& type = *(types[i]);
		ShareData_IO_Type_One(profile, type, szKey);
		if (profile.IsReadingMode()) {
			type.m_nIdx = i;
			if (i == 0) {
				pShare->m_TypeBasis = type;
			}
			auto_strcpy(pShare->m_TypeMini[i].m_szTypeExts, type.m_szTypeExts);
			auto_strcpy(pShare->m_TypeMini[i].m_szTypeName, type.m_szTypeName);
			pShare->m_TypeMini[i].m_id = type.m_id;
			pShare->m_TypeMini[i].m_encoding = type.m_encoding;
		}
	}
	if (profile.IsReadingMode()) {
		// Id重複チェック、更新
		for (int i=0; i<pShare->m_nTypesCount-1; ++i) {
			TypeConfig& type = *(types[i]);
			for (int k=i+1; k<pShare->m_nTypesCount; ++k) {
				TypeConfig& type2 = *(types[k]);
				if (type.m_id == type2.m_id) {
					type2.m_id = (::GetTickCount() & 0x3fffffff) + k * 0x10000;
					pShare->m_TypeMini[k].m_id = type2.m_id;
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
			types.m_nIdx					= buf[0];
			types.m_nMaxLineKetas			= buf[1];
			types.m_nColumnSpace			= buf[2];
			types.m_nTabSpace				= buf[3];
			types.m_nKeyWordSetIdx[0]		= buf[4];
			types.m_nKeyWordSetIdx[1]		= buf[5];
			types.m_nStringType				= (StringLiteralType)buf[6];
			types.m_bLineNumIsCRLF			= (buf[7] != 0);
			types.m_nLineTermType			= buf[8];
			types.m_bWordWrap				= (buf[9] != 0);
			types.m_nCurrentPrintSetting	= buf[10];
		}
		// 折り返し幅の最小値は10。少なくとも４ないとハングアップする。 // 20050818 aroka
		if (types.m_nMaxLineKetas < LayoutInt(MINLINEKETAS)) {
			types.m_nMaxLineKetas = LayoutInt(MINLINEKETAS);
		}
	}else {
		auto_sprintf(szKeyData, pszForm,
			types.m_nIdx,
			types.m_nMaxLineKetas,
			types.m_nColumnSpace,
			types.m_nTabSpace,
			types.m_nKeyWordSetIdx[0],
			types.m_nKeyWordSetIdx[1],
			types.m_nStringType,
			types.m_bLineNumIsCRLF ? 1 : 0,
			types.m_nLineTermType,
			types.m_bWordWrap ? 1 : 0,
			types.m_nCurrentPrintSetting
		);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
	}
	// 2005.01.13 MIK Keywordset 3-10
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect3"),  types.m_nKeyWordSetIdx[2]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect4"),  types.m_nKeyWordSetIdx[3]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect5"),  types.m_nKeyWordSetIdx[4]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect6"),  types.m_nKeyWordSetIdx[5]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect7"),  types.m_nKeyWordSetIdx[6]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect8"),  types.m_nKeyWordSetIdx[7]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect9"),  types.m_nKeyWordSetIdx[8]);
	profile.IOProfileData(pszSecName, LTEXT("nKeywordSelect10"), types.m_nKeyWordSetIdx[9]);

	// 行間のすきま
	profile.IOProfileData(pszSecName, LTEXT("nLineSpace"), types.m_nLineSpace);
	if (profile.IsReadingMode()) {
		if (types.m_nLineSpace < /* 1 */ 0) {
			types.m_nLineSpace = /* 1 */ 0;
		}
		if (types.m_nLineSpace > LINESPACE_MAX) {
			types.m_nLineSpace = LINESPACE_MAX;
		}
	}

	// 行番号の最小桁数		// 加追 2014.08.02 katze
	profile.IOProfileData( pszSecName, LTEXT("nLineNumWidth"), types.m_nLineNumWidth );
	if (profile.IsReadingMode()) {
		if (types.m_nLineNumWidth < LINENUMWIDTH_MIN) {
			types.m_nLineNumWidth = LINENUMWIDTH_MIN;
		}
		if (types.m_nLineNumWidth > LINENUMWIDTH_MAX) {
			types.m_nLineNumWidth = LINENUMWIDTH_MAX;
		}
	}

	profile.IOProfileData(pszSecName, LTEXT("szTypeName"), MakeStringBufferT(types.m_szTypeName));
	profile.IOProfileData(pszSecName, LTEXT("szTypeExts"), MakeStringBufferT(types.m_szTypeExts));
	profile.IOProfileData(pszSecName, LTEXT("id"), types.m_id);
	if (types.m_id < 0) {
		types.m_id *= -1;
	}
	profile.IOProfileData(pszSecName, LTEXT("szTabViewString"), MakeStringBufferW(types.m_szTabViewString));
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("bTabArrow")	, types.m_bTabArrow);	//@@@ 2003.03.26 MIK
	profile.IOProfileData(pszSecName, LTEXT("bInsSpace")			, types.m_bInsSpace);	// 2001.12.03 hor

	profile.IOProfileData(pszSecName, LTEXT("nTextWrapMethod"), (int&)types.m_nTextWrapMethod);		// 2008.05.30 nasukoji

	profile.IOProfileData(pszSecName, LTEXT("bStringLineOnly"), types.m_bStringLineOnly);
	profile.IOProfileData(pszSecName, LTEXT("bStringEndLine"), types.m_bStringEndLine);

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
		if (bRet1 && bRet2) types.m_blockComments[0].SetBlockCommentRule(buffer[0], buffer[1]);

		//@@@ 2001.03.10 by MIK
		buffer[0][0] = buffer[1][0] = L'\0';
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom2"), MakeStringBufferW(buffer[0]));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo2")	, MakeStringBufferW(buffer[1]));
		if (bRet1 && bRet2) types.m_blockComments[1].SetBlockCommentRule(buffer[0], buffer[1]);
		
		// Line Comment
		wchar_t lbuf[COMMENT_DELIMITER_BUFFERSIZE];
		int  pos;

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment")		, MakeStringBufferW(lbuf));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn")	, pos);
		if (bRet1 && bRet2) types.m_lineComment.CopyTo(0, lbuf, pos);

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment2")		, MakeStringBufferW(lbuf));
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn2"), pos);
		if (bRet1 && bRet2) types.m_lineComment.CopyTo(1, lbuf, pos);

		lbuf[0] = L'\0'; pos = -1;
		bRet1 = profile.IOProfileData(pszSecName, LTEXT("szLineComment3")		, MakeStringBufferW(lbuf));	// Jun. 01, 2001 JEPRO 追加
		bRet2 = profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn3"), pos);	// Jun. 01, 2001 JEPRO 追加
		if (bRet1 && bRet2) types.m_lineComment.CopyTo(2, lbuf, pos);
	}else { // write
		// Block Comment
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_blockComments[0].getBlockCommentFrom())));
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_blockComments[0].getBlockCommentTo())));

		//@@@ 2001.03.10 by MIK
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentFrom2"),
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_blockComments[1].getBlockCommentFrom())));
		profile.IOProfileData(pszSecName, LTEXT("szBlockCommentTo2")	,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_blockComments[1].getBlockCommentTo())));

		// Line Comment
		profile.IOProfileData(pszSecName, LTEXT("szLineComment")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_lineComment.getLineComment(0))));
		profile.IOProfileData(pszSecName, LTEXT("szLineComment2")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_lineComment.getLineComment(1))));
		profile.IOProfileData(pszSecName, LTEXT("szLineComment3")		,
			MakeStringBufferW0(const_cast<wchar_t*>(types.m_lineComment.getLineComment(2))));	// Jun. 01, 2001 JEPRO 追加

		// From here May 12, 2001 genta
		int pos;
		pos = types.m_lineComment.getLineCommentPos(0);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn")	, pos);
		pos = types.m_lineComment.getLineCommentPos(1);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn2"), pos);
		pos = types.m_lineComment.getLineCommentPos(2);
		profile.IOProfileData(pszSecName, LTEXT("nLineCommentColumn3"), pos);	// Jun. 01, 2001 JEPRO 追加
		// To here May 12, 2001 genta

	}
	// To Here Sep. 28, 2002 genta / YAZAKI

	profile.IOProfileData(pszSecName, LTEXT("szIndentChars")		, MakeStringBufferW(types.m_szIndentChars));
	profile.IOProfileData(pszSecName, LTEXT("cLineTermChar")		, types.m_cLineTermChar);

	profile.IOProfileData(pszSecName, LTEXT("bOutlineDockDisp")			, types.m_bOutlineDockDisp);	// アウトライン解析表示の有無
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eOutlineDockSide")	, types.m_eOutlineDockSide);	// アウトライン解析ドッキング配置
	{
		const WCHAR* pszKeyName = LTEXT("xyOutlineDock");
		const WCHAR* pszForm = LTEXT("%d,%d,%d,%d");
		WCHAR		szKeyData[1024];
		if (profile.IsReadingMode()) {
			if (profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData))) {
				int buf[4];
				scan_ints(szKeyData, pszForm, buf);
				types.m_cxOutlineDockLeft	= buf[0];
				types.m_cyOutlineDockTop	= buf[1];
				types.m_cxOutlineDockRight	= buf[2];
				types.m_cyOutlineDockBottom	= buf[3];
			}
		}else {
			auto_sprintf(
				szKeyData,
				pszForm,
				types.m_cxOutlineDockLeft,
				types.m_cyOutlineDockTop,
				types.m_cxOutlineDockRight,
				types.m_cyOutlineDockBottom
			);
			profile.IOProfileData(pszSecName, pszKeyName, MakeStringBufferW(szKeyData));
		}
	}
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nDockOutline")	, types.m_nDockOutline);			// アウトライン解析方法
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("nDefaultOutline")	, types.m_eDefaultOutline);			// アウトライン解析方法
	profile.IOProfileData(pszSecName, LTEXT("szOutlineRuleFilename")	, types.m_szOutlineRuleFilename);	// アウトライン解析ルールファイル
	profile.IOProfileData(pszSecName, LTEXT("nOutlineSortCol")			, types.m_nOutlineSortCol);			// アウトライン解析ソート列番号
	profile.IOProfileData(pszSecName, LTEXT("bOutlineSortDesc")		, types.m_bOutlineSortDesc);		// アウトライン解析ソート降順
	profile.IOProfileData(pszSecName, LTEXT("nOutlineSortType")		, types.m_nOutlineSortType);		// アウトライン解析ソート基準
	ShareData_IO_FileTree( profile, types.m_fileTree, pszSecName );
	profile.IOProfileData_WrapInt( pszSecName, LTEXT("nSmartIndent")	, types.m_eSmartIndent );			// スマートインデント種別
	// Nov. 20, 2000 genta
	profile.IOProfileData(pszSecName, LTEXT("nImeState")				, types.m_nImeState);	// IME制御

	// 2001/06/14 Start By asa-o: タイプ別の補完ファイル
	// Oct. 5, 2002 genta _countof()で誤ってポインタのサイズを取得していたのを修正
	profile.IOProfileData(pszSecName, LTEXT("szHokanFile")			, types.m_szHokanFile);		// 補完ファイル
	// 2001/06/14 End
	profile.IOProfileData(pszSecName, LTEXT("nHokanType")			, types.m_nHokanType);		// 補完種別

	// 2001/06/19 asa-o
	profile.IOProfileData(pszSecName, LTEXT("bHokanLoHiCase")		, types.m_bHokanLoHiCase);

	// 2003.06.23 Moca ファイル内からの入力補完機能
	profile.IOProfileData(pszSecName, LTEXT("bUseHokanByFile")		, types.m_bUseHokanByFile);
	profile.IOProfileData(pszSecName, LTEXT("bUseHokanByKeyword")	, types.m_bUseHokanByKeyword);

	//@@@ 2002.2.4 YAZAKI
	profile.IOProfileData(pszSecName, LTEXT("szExtHelp")			, types.m_szExtHelp);

	profile.IOProfileData(pszSecName, LTEXT("szExtHtmlHelp")		, types.m_szExtHtmlHelp);
	profile.IOProfileData(pszSecName, LTEXT("bTypeHtmlHelpIsSingle"), types.m_bHtmlHelpIsSingle); // 2012.06.30 Fix m_bHokanLoHiCase -> m_bHtmlHelpIsSingle

	profile.IOProfileData(pszSecName, LTEXT("bPriorCesu8")					, types.m_encoding.m_bPriorCesu8);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eDefaultCodetype")	, types.m_encoding.m_eDefaultCodetype);
	profile.IOProfileData_WrapInt(pszSecName, LTEXT("eDefaultEoltype")		, types.m_encoding.m_eDefaultEoltype);
	profile.IOProfileData(pszSecName, LTEXT("bDefaultBom")					, types.m_encoding.m_bDefaultBom);

	profile.IOProfileData(pszSecName, LTEXT("bAutoIndent")				, types.m_bAutoIndent);
	profile.IOProfileData(pszSecName, LTEXT("bAutoIndent_ZENSPACE")	, types.m_bAutoIndent_ZENSPACE);
	profile.IOProfileData(pszSecName, LTEXT("bRTrimPrevLine")			, types.m_bRTrimPrevLine);			// 2005.10.08 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nIndentLayout")			, types.m_nIndentLayout);

	// 色設定 I/O
	IO_ColorSet(&profile, pszSecName, types.m_colorInfoArr);

	// 2010.09.17 背景画像
	profile.IOProfileData(pszSecName, L"bgImgPath", types.m_szBackImgPath);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPos", types.m_backImgPos);
	profile.IOProfileData(pszSecName, L"bgImgScrollX",   types.m_backImgScrollX);
	profile.IOProfileData(pszSecName, L"bgImgScrollY",   types.m_backImgScrollY);
	profile.IOProfileData(pszSecName, L"bgImgRepeartX",  types.m_backImgRepeatX);
	profile.IOProfileData(pszSecName, L"bgImgRepeartY",  types.m_backImgRepeatY);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPosOffsetX",  types.m_backImgPosOffset.x);
	profile.IOProfileData_WrapInt(pszSecName, L"bgImgPosOffsetY",  types.m_backImgPosOffset.y);

	// 2005.11.08 Moca 指定桁縦線
	for (int j=0; j<MAX_VERTLINES; ++j) {
		auto_sprintf(szKeyName, LTEXT("nVertLineIdx%d"), j + 1);
		profile.IOProfileData(pszSecName, szKeyName, types.m_nVertLineIdx[j]);
		if (types.m_nVertLineIdx[j] == 0) {
			break;
		}
	}
	profile.IOProfileData( pszSecName, L"nNoteLineOffset", types.m_nNoteLineOffset );

//@@@ 2001.11.17 add start MIK
	{	// 正規表現キーワード
		WCHAR* p;
		profile.IOProfileData(pszSecName, LTEXT("bUseRegexKeyword"), types.m_bUseRegexKeyword);	// 正規表現キーワード使用するか？
		wchar_t* pKeyword = types.m_RegexKeywordList;
		int nPos = 0;
		int nKeywordSize = _countof(types.m_RegexKeywordList);
		for (int j=0; j<_countof(types.m_RegexKeywordArr); ++j) {
			auto_sprintf(szKeyName, LTEXT("RxKey[%03d]"), j);
			if (profile.IsReadingMode()) {
				types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
				if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					p = wcschr(szKeyData, LTEXT(','));
					if (p) {
						*p = LTEXT('\0');
						types.m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(to_tchar(szKeyData));	//@@@ 2002.04.30
						if (types.m_RegexKeywordArr[j].m_nColorIndex == -1)	// 名前でない
							types.m_RegexKeywordArr[j].m_nColorIndex = _wtoi(szKeyData);
						++p;
						if (0 < nKeywordSize - nPos - 1) {
							wcscpyn(&pKeyword[nPos], p, nKeywordSize - nPos - 1);
						}
						if (0
							|| types.m_RegexKeywordArr[j].m_nColorIndex < 0
							|| types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST
						) {
							types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
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
					GetColorNameByIndex(types.m_RegexKeywordArr[j].m_nColorIndex),
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
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuHead")	, types.m_bKinsokuHead);
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuTail")	, types.m_bKinsokuTail);
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuRet")	, types.m_bKinsokuRet);	//@@@ 2002.04.13 MIK
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuKuto")	, types.m_bKinsokuKuto);	//@@@ 2002.04.17 MIK
	profile.IOProfileData(pszSecName, LTEXT("bKinsokuHide")	, types.m_bKinsokuHide);	// 2012/11/30 Uchi
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuHead")	, MakeStringBufferW(types.m_szKinsokuHead));
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuTail")	, MakeStringBufferW(types.m_szKinsokuTail));
	profile.IOProfileData(pszSecName, LTEXT("szKinsokuKuto")	, MakeStringBufferW(types.m_szKinsokuKuto));	// 2009.08.07 ryoji
	profile.IOProfileData(pszSecName, LTEXT("bUseDocumentIcon")	, types.m_bUseDocumentIcon);	// Sep. 19 ,2002 genta 変数名誤り修正

//@@@ 2006.04.10 fon ADD-start
	{	// キーワード辞書
		WCHAR	*pH, *pT;	// <pH>keyword<pT>
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyWordHelp"), types.m_bUseKeyWordHelp);			// キーワード辞書選択を使用するか？
//		profile.IOProfileData(pszSecName, LTEXT("nKeyHelpNum"), types.m_nKeyHelpNum);					// 登録辞書数
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpAllSearch"), types.m_bUseKeyHelpAllSearch);	// ヒットした次の辞書も検索(&A)
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpKeyDisp"), types.m_bUseKeyHelpKeyDisp);		// 1行目にキーワードも表示する(&W)
		profile.IOProfileData(pszSecName, LTEXT("bUseKeyHelpPrefix"), types.m_bUseKeyHelpPrefix);		// 選択範囲で前方一致検索(&P)
		for (int j=0; j<MAX_KEYHELP_FILE; ++j) {
			auto_sprintf(szKeyName, LTEXT("KDct[%02d]"), j);
			// 読み出し
			if (profile.IsReadingMode()) {
				types.m_KeyHelpArr[j].m_bUse = false;
				types.m_KeyHelpArr[j].m_szAbout[0] = _T('\0');
				types.m_KeyHelpArr[j].m_szPath[0] = _T('\0');
				if (profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData))) {
					pH = szKeyData;
					if (pT = wcschr(pH, L',')) {
						*pT = L'\0';
						types.m_KeyHelpArr[j].m_bUse = (_wtoi(pH) != 0);
						pH = pT + 1;
						if (pT = wcschr(pH, L',')) {
							*pT = L'\0';
							_wcstotcs(types.m_KeyHelpArr[j].m_szAbout, pH, _countof(types.m_KeyHelpArr[j].m_szAbout));
							pH = pT + 1;
							if (L'\0' != (*pH)) {
								_wcstotcs(types.m_KeyHelpArr[j].m_szPath, pH, _countof2(types.m_KeyHelpArr[j].m_szPath));
								types.m_nKeyHelpNum = j + 1;	// iniに保存せずに、読み出せたファイル分を辞書数とする
							}
						}
					}
				}
			// 書き込み
			}else {
				if (types.m_KeyHelpArr[j].m_szPath[0] != _T('\0')) {
					auto_sprintf(szKeyData, LTEXT("%d,%ts,%ts"),
						types.m_KeyHelpArr[j].m_bUse ? 1 : 0,
						types.m_KeyHelpArr[j].m_szAbout,
						types.m_KeyHelpArr[j].m_szPath.c_str()
					);
					profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
				}
			}
		}
		// 旧バージョンiniファイルの読み出しサポート
		if (profile.IsReadingMode()) {
			SFilePath tmp;
			if (profile.IOProfileData(pszSecName, LTEXT("szKeyWordHelpFile"), tmp)) {
				types.m_KeyHelpArr[0].m_szPath = tmp;
			}
		}
	}
//@@@ 2006.04.10 fon ADD-end

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	profile.IOProfileData(pszSecName, LTEXT("bChkEnterAtEnd")	, types.m_bChkEnterAtEnd);

	{ // フォント設定
		profile.IOProfileData(pszSecName, LTEXT("bUseTypeFont"), types.m_bUseTypeFont);
		ShareData_IO_Sub_LogFont(profile, pszSecName, L"lf", L"nPointSize", L"lfFaceName",
			types.m_lf, types.m_nPointSize);
	}
}

/*!
	@brief 共有データのKeyWordsセクションの入出力
	@param[in]		bRead		true: 読み込み / false: 書き込み
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2005-04-07 D.S.Koba ShareData_IO_2から分離。
*/
void ShareData_IO::ShareData_IO_KeyWords(DataProfile& profile)
{
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("KeyWords");
	WCHAR			szKeyName[64];
	WCHAR			szKeyData[1024];
	KeyWordSetMgr*	pKeyWordSetMgr = &pShare->m_common.m_specialKeyword.m_keyWordSetMgr;
	int				nKeyWordSetNum = pKeyWordSetMgr->m_nKeyWordSetNum;

	profile.IOProfileData(pszSecName, LTEXT("nCurrentKeyWordSetIdx")	, pKeyWordSetMgr->m_nCurrentKeyWordSetIdx);
	bool bIOSuccess = profile.IOProfileData(pszSecName, LTEXT("nKeyWordSetNum"), nKeyWordSetNum);
	if (profile.IsReadingMode()) {
		// nKeyWordSetNum が読み込めていれば、すべての情報がそろっていると仮定して処理を進める
		if (bIOSuccess) {
			// 2004.11.25 Moca キーワードセットの情報は、直接書き換えないで関数を利用する
			// 初期設定されているため、先に削除しないと固定メモリの確保に失敗する可能性がある
			pKeyWordSetMgr->ResetAllKeyWordSet();
			for (int i=0; i<nKeyWordSetNum; ++i) {
				bool bKEYWORDCASE = false;
				int nKeyWordNum = 0;
				// 値の取得
				auto_sprintf(szKeyName, LTEXT("szSN[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szKeyData));
				auto_sprintf(szKeyName, LTEXT("nCASE[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, bKEYWORDCASE);
				auto_sprintf(szKeyName, LTEXT("nKWN[%02d]"), i);
				profile.IOProfileData(pszSecName, szKeyName, nKeyWordNum);

				// 追加
				pKeyWordSetMgr->AddKeyWordSet(szKeyData, bKEYWORDCASE, nKeyWordNum);
				auto_sprintf(szKeyName, LTEXT("szKW[%02d]"), i);
				std::wstring sValue;	// wstring のまま受ける（古い ini ファイルのキーワードは中身が NULL 文字区切りなので StringBufferW では NG だった）
				if (profile.IOProfileData(pszSecName, szKeyName, sValue)) {
					pKeyWordSetMgr->SetKeyWordArr(i, nKeyWordNum, sValue.c_str());
				}
			}
		}
	}else {
		int nSize = pKeyWordSetMgr->m_nKeyWordSetNum;
		for (int i=0; i<nSize; ++i) {
			auto_sprintf(szKeyName, LTEXT("szSN[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pKeyWordSetMgr->m_szSetNameArr[i]));
			auto_sprintf(szKeyName, LTEXT("nCASE[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, pKeyWordSetMgr->m_bKEYWORDCASEArr[i]);
			auto_sprintf(szKeyName, LTEXT("nKWN[%02d]"), i);
			profile.IOProfileData(pszSecName, szKeyName, pKeyWordSetMgr->m_nKeyWordNumArr[i]);
			
			int nMemLen = 0;
			for (int j=0; j<pKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j) {
				nMemLen += wcslen(pKeyWordSetMgr->GetKeyWord(i, j));
				nMemLen ++;
			}
			nMemLen ++;
			auto_sprintf(szKeyName, LTEXT("szKW[%02d].Size"), i);
			profile.IOProfileData(pszSecName, szKeyName, nMemLen);
			std::vector<wchar_t> szMem(nMemLen + 1); // May 25, 2003 genta 区切りをTABに変更したので，最後の\0の分を追加
			wchar_t* pszMem = &szMem[0];
			wchar_t* pMem = pszMem;
			for (int j=0; j<pKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j) {
				// May 25, 2003 genta 区切りをTABに変更
				int kwlen = wcslen(pKeyWordSetMgr->GetKeyWord(i, j));
				auto_memcpy(pMem, pKeyWordSetMgr->GetKeyWord(i, j), kwlen);
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
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Macro");
	WCHAR szKeyName[64];
	for (int i=0; i<MAX_CUSTMACRO; ++i) {
		MacroRec& macrorec = pShare->m_common.m_macro.m_macroTable[i];
		// Oct. 4, 2001 genta あまり意味がなさそうなので削除：3行
		// 2002.02.08 hor 未定義値を無視
		if (!profile.IsReadingMode() && macrorec.m_szName[0] == _T('\0') && macrorec.m_szFile[0] == _T('\0')) continue;
		auto_sprintf(szKeyName, LTEXT("Name[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szName));
		auto_sprintf(szKeyName, LTEXT("File[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferT(macrorec.m_szFile));
		auto_sprintf(szKeyName, LTEXT("ReloadWhenExecute[%03d]"), i);
		profile.IOProfileData(pszSecName, szKeyName, macrorec.m_bReloadWhenExecute);
	}
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnOpened"), pShare->m_common.m_macro.m_nMacroOnOpened);			// オープン後自動実行マクロ番号			//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnTypeChanged"), pShare->m_common.m_macro.m_nMacroOnTypeChanged);// タイプ変更後自動実行マクロ番号		//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroOnSave"), pShare->m_common.m_macro.m_nMacroOnSave);				// 保存前自動実行マクロ番号				//@@@ 2006.09.01 ryoji
	profile.IOProfileData(pszSecName, LTEXT("nMacroCancelTimer"), pShare->m_common.m_macro.m_nMacroCancelTimer);	// マクロ停止ダイアログ表示待ち時間		// 2011.08.04 syat
}

/*!
	@brief 共有データのStatusbarセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2008/6/21 Uchi
*/
void ShareData_IO::ShareData_IO_Statusbar(DataProfile& profile)
{
	static const WCHAR* pszSecName = LTEXT("Statusbar");
	CommonSetting_StatusBar& statusbar = GetDllShareData().m_common.m_statusBar;

	// 表示文字コードの指定
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInSjis")			, statusbar.m_bDispUniInSjis);		// SJISで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInJis")				, statusbar.m_bDispUniInJis);		// JISで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUnicodeInEuc")				, statusbar.m_bDispUniInEuc);		// EUCで文字コード値をUnicodeで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispUtf8Codepoint")			, statusbar.m_bDispUtf8Codepoint);	// UTF-8をコードポイントで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispSurrogatePairCodepoint")	, statusbar.m_bDispSPCodepoint);	// サロゲートペアをコードポイントで表示する
	profile.IOProfileData(pszSecName, LTEXT("DispSelectCountByByte")		, statusbar.m_bDispSelCountByByte);	// 選択文字数を文字単位ではなくバイト単位で表示する
}

/*!
	@brief 共有データのPluginセクションの入出力
	@param[in,out]	profile	INIファイル入出力クラス

	@date 2009/11/30 syat
*/
void ShareData_IO::ShareData_IO_Plugin(DataProfile& profile, MenuDrawer* pMenuDrawer)
{
	static const WCHAR* pszSecName = LTEXT("Plugin");
	CommonSetting& common = GetDllShareData().m_common;
	CommonSetting_Plugin& plugin = GetDllShareData().m_common.m_plugin;

	profile.IOProfileData(pszSecName, LTEXT("EnablePlugin"), plugin.m_bEnablePlugin);		// プラグインを使用する

	// プラグインテーブル
	WCHAR	szKeyName[64];
	for (int i=0; i<MAX_PLUGIN; ++i) {
		PluginRec& pluginrec = common.m_plugin.m_pluginTable[i];

		// 2010.08.04 Moca 書き込み直前に削除フラグで削除扱いにする
		if (pluginrec.m_state == PLS_DELETED) {
			pluginrec.m_szName[0] = L'\0';
			pluginrec.m_szId[0] = L'\0';
		}
		auto_sprintf(szKeyName, LTEXT("P[%02d].Name"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pluginrec.m_szName));
		auto_sprintf(szKeyName, LTEXT("P[%02d].Id"), i);
		profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(pluginrec.m_szId));
		auto_sprintf(szKeyName, LTEXT("P[%02d].CmdNum"), i);
		profile.IOProfileData(pszSecName, szKeyName, pluginrec.m_nCmdNum);	// 2010/7/4 Uchi
		pluginrec.m_state = (pluginrec.m_szId[0] == '\0' ? PLS_NONE : PLS_STOPPED);
		// Command 仮設定	// 2010/7/4 Uchi
		if (pluginrec.m_szId[0] != '\0' && pluginrec.m_nCmdNum >0) {
			for (int j=1; j<=pluginrec.m_nCmdNum; ++j) {
				pMenuDrawer->AddToolButton(MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT, Plug::GetPluginFunctionCode(i, j));
			}
		}
	}
}

struct MainMenuAddItemInfo
{
	int m_nVer;
	EFunctionCode m_nAddFuncCode;
	EFunctionCode m_nPrevFuncCode;
	wchar_t m_cAccKey;
	bool m_bAddPrevSeparete;
	bool m_bAddNextSeparete;
};

void ShareData_IO::ShareData_IO_MainMenu(DataProfile& profile)
{
	IO_MainMenu(profile, GetDllShareData().m_common.m_mainMenu, false);		// 2010/5/15 Uchi

	// 2015.02.26 Moca メインメニュー自動更新
	const WCHAR*	pszSecName = LTEXT("MainMenu");
	int& nVersion = GetDllShareData().m_common.m_mainMenu.m_nVersion;
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
		CommonSetting_MainMenu& mainmenu = GetDllShareData().m_common.m_mainMenu;
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
			if (item.m_nVer <= nVersion) {
				continue;
			}
			MainMenu* pMenuTbl = mainmenu.m_mainMenuTbl;
			int k = 0;
			for (; k<mainmenu.m_nMainMenuNum; ++k) {
				if (pMenuTbl[k].nFunc == item.m_nAddFuncCode) {
					break;
				}
			}
			int nAddSep = 0;
			if (item.m_bAddPrevSeparete) {
				++nAddSep;
			}
			if (item.m_bAddNextSeparete) {
				++nAddSep;
			}
			if (k == mainmenu.m_nMainMenuNum && mainmenu.m_nMainMenuNum + nAddSep < _countof(mainmenu.m_mainMenuTbl)) {
				// メニュー内にまだ追加されていないので追加する
				for (int r=0; r<mainmenu.m_nMainMenuNum; ++r) {
					if (pMenuTbl[r].nFunc == item.m_nPrevFuncCode && 0 < pMenuTbl[r].m_nLevel) {
						// 追加分後ろにずらす
						for (int n=mainmenu.m_nMainMenuNum-1; r<n; --n) {
							pMenuTbl[n + 1 + nAddSep] = pMenuTbl[n];
						}
						for (int n=0; n<MAX_MAINMENU_TOP; ++n) {
							if (r < mainmenu.m_nMenuTopIdx[n]) {
								mainmenu.m_nMenuTopIdx[n] += 1 + nAddSep;
							}
						}
						MainMenu* pMenu = &pMenuTbl[r+1];
						const int nLevel = pMenuTbl[r].m_nLevel;
						if (item.m_bAddPrevSeparete) {
							pMenu->m_nType    = MainMenuType::Separator;
							pMenu->nFunc    = F_SEPARATOR;
							pMenu->m_nLevel   = nLevel;
							pMenu->m_sName[0] = L'\0';
							pMenu->m_sKey[0]  = L'\0';
							pMenu->m_sKey[1]  = L'\0';
							++pMenu;
							mainmenu.m_nMainMenuNum++;
						}
						pMenu->m_nType    = MainMenuType::Leaf;
						pMenu->nFunc    = item.m_nAddFuncCode;
						pMenu->m_nLevel   = nLevel;
						pMenu->m_sName[0] = L'\0';
						pMenu->m_sKey[0]  = L'\0';
						pMenu->m_sKey[1]  = L'\0';
						mainmenu.m_nMainMenuNum++;
						if (item.m_bAddNextSeparete) {
							++pMenu;
							pMenu->m_nType    = MainMenuType::Separator;
							pMenu->nFunc    = F_SEPARATOR;
							pMenu->m_nLevel   = nLevel;
							pMenu->m_sName[0] = L'\0';
							pMenu->m_sKey[0]  = L'\0';
							pMenu->m_sKey[1]  = L'\0';
							mainmenu.m_nMainMenuNum++;
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
		mainmenu.m_nMainMenuNum = menuNum;
		SetValueLimit(mainmenu.m_nMainMenuNum, MAX_MAINMENU);
	}else {
		profile.IOProfileData(pszSecName, LTEXT("nMainMenuNum"), mainmenu.m_nMainMenuNum);
	}
	
	if (pData) {
		mainmenu.m_bMainMenuKeyParentheses = (_wtoi(data[dataNum++].c_str()) != 0);
	}else {
		profile.IOProfileData(pszSecName, LTEXT("bKeyParentheses"), mainmenu.m_bMainMenuKeyParentheses);
	}

	if (profile.IsReadingMode()) {
		// Top Level 初期化
		memset(mainmenu.m_nMenuTopIdx, -1, sizeof(mainmenu.m_nMenuTopIdx));
	}

	int nIdx = 0;
	for (int i=0; i<mainmenu.m_nMainMenuNum; ++i) {
		// メインメニューテーブル
		MainMenu* pMenu = &mainmenu.m_mainMenuTbl[i];

		auto_sprintf(szKeyName, LTEXT("MM[%03d]"), i);
		if (profile.IsReadingMode()) {
			// 読み込み時初期化
			pMenu->m_nType    = MainMenuType::Node;
			pMenu->nFunc    = F_INVALID;
			pMenu->m_nLevel   = 0;
			pMenu->m_sName[0] = L'\0';
			pMenu->m_sKey[0]  = L'\0';
			pMenu->m_sKey[1]  = L'\0';

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
			pMenu->m_nLevel = auto_atol(p);
			if (!pn) {
				continue;
			}

			// 種類
			p = pn;
			pn = wcschr(p, L',');
			if (pn) *pn++ = L'\0';
			pMenu->m_nType = (MainMenuType)auto_atol(p);
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
					pMenu->m_sKey[0]  = *p++;
				}
			}else {
				pMenu->m_sKey[0]  = *p++;
			}
			if (*p == L'\0') {
				continue;
			}

			// 表示名
			++p;
			auto_strcpy_s(pMenu->m_sName, MAX_MAIN_MENU_NAME_LEN + 1, p);
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
				pMenu->m_nLevel, 
				pMenu->m_nType, 
				szFuncName, 
				pMenu->m_sKey, 
				pMenu->nFunc == F_NODE ? pMenu->m_sName : L"");
			profile.IOProfileData(pszSecName, szKeyName, MakeStringBufferW(szLine));
		}

		if (profile.IsReadingMode() && pMenu->m_nLevel == 0) {
			// Top Level設定
			if (nIdx < MAX_MAINMENU_TOP) {
				mainmenu.m_nMenuTopIdx[nIdx++] = i;
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
	DLLSHAREDATA* pShare = &GetDllShareData();

	static const WCHAR* pszSecName = LTEXT("Other");	// セクションを1個作成した。2003.05.12 MIK
	WCHAR szKeyName[64];

	// **** その他のダイアログ ****
	// 外部コマンド実行の「標準出力を得る」
	if (!profile.IOProfileData(pszSecName, LTEXT("nExecFlgOpt")	, pShare->m_nExecFlgOpt)) { // 2006.12.03 maru オプション拡張
		profile.IOProfileData(pszSecName, LTEXT("bGetStdout")		, pShare->m_nExecFlgOpt);
	}

	// 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か
	profile.IOProfileData(pszSecName, LTEXT("bLineNumIsCRLF")	, pShare->m_bLineNumIsCRLF_ForJump);
	
	// DIFF差分表示	//@@@ 2002.05.27 MIK
	profile.IOProfileData(pszSecName, LTEXT("nDiffFlgOpt")	, pShare->m_nDiffFlgOpt);
	
	// CTAGS	//@@@ 2003.05.12 MIK
	profile.IOProfileData(pszSecName, LTEXT("nTagsOpt")		, pShare->m_nTagsOpt);
	profile.IOProfileData(pszSecName, LTEXT("szTagsCmdLine")	, MakeStringBufferT(pShare->m_szTagsCmdLine));
	
	// From Here 2005.04.03 MIK キーワード指定タグジャンプ
	profile.IOProfileData(pszSecName, LTEXT("_TagJumpKeyword_Counts"), pShare->m_tagJump.m_aTagJumpKeywords._GetSizeRef());
	pShare->m_history.m_aCommands.SetSizeLimit();
	int nSize = pShare->m_tagJump.m_aTagJumpKeywords.size();
	for (int i=0; i<nSize; ++i) {
		auto_sprintf(szKeyName, LTEXT("TagJumpKeyword[%02d]"), i);
		if (i >= nSize) {
			pShare->m_tagJump.m_aTagJumpKeywords[i][0] = 0;
		}
		profile.IOProfileData(pszSecName, szKeyName, pShare->m_tagJump.m_aTagJumpKeywords[i]);
	}
	profile.IOProfileData(pszSecName, LTEXT("m_bTagJumpICase")		, pShare->m_tagJump.m_bTagJumpICase);
	profile.IOProfileData(pszSecName, LTEXT("m_bTagJumpAnyWhere")		, pShare->m_tagJump.m_bTagJumpAnyWhere);
	// From Here 2005.04.03 MIK キーワード指定タグジャンプの

	// MIK バージョン情報（書き込みのみ）
	if (!profile.IsReadingMode()) {
		TCHAR	iniVer[256];
		auto_sprintf(iniVer, _T("%d.%d.%d.%d"), 
					HIWORD(pShare->m_version.m_dwProductVersionMS),
					LOWORD(pShare->m_version.m_dwProductVersionMS),
					HIWORD(pShare->m_version.m_dwProductVersionLS),
					LOWORD(pShare->m_version.m_dwProductVersionLS));
		profile.IOProfileData(pszSecName, LTEXT("szVersion"), MakeStringBufferT(iniVer));

		// 共有メモリバージョン	2010/5/20 Uchi
		int		nStructureVersion;
		nStructureVersion = int(pShare->m_vStructureVersion);
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
				pColorInfoArr[j].m_bDisp                  = (buf[0] != 0);
				pColorInfoArr[j].m_fontAttr.m_bBoldFont  = (buf[1] != 0);
				pColorInfoArr[j].m_colorAttr.m_cTEXT     = buf[2];
				pColorInfoArr[j].m_colorAttr.m_cBACK     = buf[3];
				pColorInfoArr[j].m_fontAttr.m_bUnderLine = (buf[4] != 0);
			}else {
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 以前のiniファイルを読んだときにキャレットがテキスト背景色と同じになると
				// ちょっと困るのでキャレット色が読めないときはキャレット色をテキスト色と同じにする
				if (COLORIDX_CARET == j)
					pColorInfoArr[j].m_colorAttr.m_cTEXT = pColorInfoArr[COLORIDX_TEXT].m_colorAttr.m_cTEXT;
			}
			// 2006.12.18 ryoji
			// 矛盾設定があれば修復する
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if ((fAttribute & COLOR_ATTRIB_FORCE_DISP) != 0)
				pColorInfoArr[j].m_bDisp = true;
			if ((fAttribute & COLOR_ATTRIB_NO_BOLD) != 0)
				pColorInfoArr[j].m_fontAttr.m_bBoldFont = false;
			if ((fAttribute & COLOR_ATTRIB_NO_UNDERLINE) != 0)
				pColorInfoArr[j].m_fontAttr.m_bUnderLine = false;
		}else {
			auto_sprintf(szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp ? 1 : 0,
				pColorInfoArr[j].m_fontAttr.m_bBoldFont ? 1 : 0,
				pColorInfoArr[j].m_colorAttr.m_cTEXT,
				pColorInfoArr[j].m_colorAttr.m_cBACK,
				pColorInfoArr[j].m_fontAttr.m_bUnderLine ? 1 : 0
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
	profile.IOProfileData( pszSecName, L"bFileTreeProject", fileTree.m_bProject );
	profile.IOProfileData( pszSecName, L"szFileTreeProjectIni", fileTree.m_szProjectIni );
	profile.IOProfileData( pszSecName, L"nFileTreeItemCount", fileTree.m_nItemCount );
	SetValueLimit( fileTree.m_nItemCount, _countof(fileTree.m_aItems) );
	for (int i=0; i<fileTree.m_nItemCount; ++i) {
		ShareData_IO_FileTreeItem( profile, fileTree.m_aItems[i], pszSecName, i );
	}
}

void ShareData_IO::ShareData_IO_FileTreeItem(
	DataProfile& profile, FileTreeItem& item, const WCHAR* pszSecName, int i )
{
	WCHAR szKey[64];
	auto_sprintf( szKey, L"FileTree(%d).eItemType", i );
	profile.IOProfileData_WrapInt( pszSecName, szKey, item.m_eFileTreeItemType );
	if (profile.IsReadingMode()
		|| item.m_eFileTreeItemType == FileTreeItemType::Grep
		|| item.m_eFileTreeItemType == FileTreeItemType::File
	) {
		auto_sprintf( szKey, L"FileTree(%d).szTargetPath", i );
		profile.IOProfileData( pszSecName, szKey, item.m_szTargetPath );
	}
	if (profile.IsReadingMode()
		|| ((item.m_eFileTreeItemType == FileTreeItemType::Grep || item.m_eFileTreeItemType == FileTreeItemType::File)
			&& item.m_szLabelName[0] != _T('\0') )
		|| item.m_eFileTreeItemType == FileTreeItemType::Folder
	) {
		auto_sprintf( szKey, L"FileTree(%d).szLabelName", i );
		profile.IOProfileData( pszSecName, szKey, item.m_szLabelName );
	}
	auto_sprintf( szKey, L"FileTree(%d).nDepth", i );
	profile.IOProfileData( pszSecName, szKey, item.m_nDepth );
	if (profile.IsReadingMode()
		|| item.m_eFileTreeItemType == FileTreeItemType::Grep
	) {
		auto_sprintf( szKey, L"FileTree(%d).szTargetFile", i );
		profile.IOProfileData( pszSecName, szKey, item.m_szTargetFile );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreHidden", i );
		profile.IOProfileData( pszSecName, szKey, item.m_bIgnoreHidden );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreReadOny", i );
		profile.IOProfileData( pszSecName, szKey, item.m_bIgnoreReadOnly );
		auto_sprintf( szKey, L"FileTree(%d).bIgnoreSystem", i );
		profile.IOProfileData( pszSecName, szKey, item.m_bIgnoreSystem );
	}
}
