/*
	2008.05.18 kobake CShareData から分離
*/
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
#include <ShlObj.h> // CSIDL_PROFILE等

#include "DLLSHAREDATA.h"
#include "CFileNameManager.h"
#include "charset/CCodePage.h"
#include "util/module.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/file.h"
#include "util/window.h"
#include "_main/CCommandLine.h"
#include "_os/COsVersionInfo.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ファイル名管理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!	共有データの設定に従ってパスを縮小表記に変換する
	@param pszSrc   [in]  ファイル名
	@param pszDest  [out] 変換後のファイル名の格納先
	@param nDestLen [in]  終端のNULLを含むpszDestのTCHAR単位の長さ _MAX_PATH まで
	@date 2003.01.27 Moca 新規作成
	@note 連続して呼び出す場合のため、展開済みメタ文字列をキャッシュして高速化している。
*/
LPTSTR CFileNameManager::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, HDC hDC, bool bFitMode, int cchMaxWidth )
{
	TCHAR szBuf[_MAX_PATH + 1];

	if (m_nTransformFileNameCount == -1) {
		TransformFileName_MakeCache();
	}

	int nPxWidth = -1;
	auto& csFileName = m_pShareData->m_common.m_sFileName;
	if (csFileName.m_bTransformShortPath && cchMaxWidth != -1) {
		if (cchMaxWidth == 0) {
			cchMaxWidth = csFileName.m_nTransformShortMaxWidth;
		}
		CTextWidthCalc calc(hDC);
		nPxWidth = calc.GetTextWidth(_T("x")) * cchMaxWidth;
	}

	if (0 < m_nTransformFileNameCount) {
		GetFilePathFormat(pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			csFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]]
		);
		for (int i=1; i<m_nTransformFileNameCount; ++i) {
			_tcscpy(szBuf, pszDest);
			GetFilePathFormat(szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				csFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]]);
		}
		if (nPxWidth != -1) {
			_tcscpy( szBuf, pszDest );
			GetShortViewPath( pszDest, nDestLen, szBuf, hDC, nPxWidth, bFitMode );
		}
	}else if (nPxWidth != -1) {
		GetShortViewPath( pszDest, nDestLen, pszSrc, hDC, nPxWidth, bFitMode );
	}else {
		// 変換する必要がない コピーだけする
		_tcsncpy(pszDest, pszSrc, nDestLen - 1);
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	展開済みメタ文字列のキャッシュを作成・更新する
	@retval 有効な展開済み置換前文字列の数
	@date 2003.01.27 Moca 新規作成
	@date 2003.06.23 Moca 関数名変更
*/
int CFileNameManager::TransformFileName_MakeCache(void) {
	int nCount = 0;
	auto& csFileName = m_pShareData->m_common.m_sFileName;
	for (int i=0; i<csFileName.m_nTransformFileNameArrNum; ++i) {
		if (L'\0' != csFileName.m_szTransformFileNameFrom[i][0]) {
			if (ExpandMetaToFolder(
				csFileName.m_szTransformFileNameFrom[i],
				m_szTransformFileNameFromExp[nCount],
				_MAX_PATH
				)
			) {
				// m_szTransformFileNameToとm_szTransformFileNameFromExpの番号がずれることがあるので記録しておく
				m_nTransformFileNameOrgId[nCount] = i;
				++nCount;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}


/*!	ファイル・フォルダ名を置換して、簡易表示名を取得する
	@date 2002.11.27 Moca 新規作成
	@note 大小文字を区別しない。nDestLenに達したときは後ろを切り捨てられる
*/
LPCTSTR CFileNameManager::GetFilePathFormat(LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo)
{
	int nSrcLen  = _tcslen(pszSrc);
	int nFromLen = _tcslen(pszFrom);
	int nToLen   = _tcslen(pszTo);

	--nDestLen;

	int j = 0;
	for (int i=0; i<nSrcLen && j<nDestLen; ++i) {
		if (_tcsncicmp(&pszSrc[i], pszFrom, nFromLen) == 0)
		{
			int nCopy = t_min(nToLen, nDestLen - j);
			memcpy(&pszDest[j], pszTo, nCopy * sizeof(TCHAR));
			j += nCopy;
			i += nFromLen - 1;
		}else {
			pszDest[j] = pszSrc[i];
			++j;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%などのパラメータ指定を実際のパス名に変換する

	@param pszSrc  [in]  変換前文字列
	@param pszDes  [out] 変換後文字列
	@param nDesLen [in]  pszDesのNULLを含むTCHAR単位の長さ
	@retval true  正常に変換できた
	@retval false バッファが足りなかった，またはエラー。pszDesは不定
	@date 2002.11.27 Moca 作成開始
*/
bool CFileNameManager::ExpandMetaToFolder(LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen)
{
#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias {
		LPCTSTR szAlias;
		int nLenth;
		LPCTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	LPCTSTR ps;
	LPTSTR pd;
	LPTSTR pd_end = pszDes + (nDesLen - 1);
	for (ps=pszSrc, pd=pszDes; _T('\0')!=*ps; ++ps) {
		if (pd_end <= pd) {
			if (pd_end == pd) {
				*pd = _T('\0');
			}
			return false;
		}

		if (_T('%') != *ps) {
			*pd = *ps;
			++pd;
			continue;
		}

		// %% は %
		if (_T('%') == ps[1]) {
			*pd = _T('%');
			++pd;
			++ps;
			continue;
		}

		if (_T('\0') != ps[1]) {
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			LPCTSTR  pStr;
			++ps;
			// %SAKURA%
			if (auto_strnicmp(_T("SAKURA%"), ps, 7) == 0) {
				// exeのあるフォルダ
				GetExedir(szPath);
				nMetaLen = 6;
			// %SAKURADATA%	// 2007.06.06 ryoji
			}else if (auto_strnicmp(_T("SAKURADATA%"), ps, 11) == 0) {
				// iniのあるフォルダ
				GetInidir(szPath);
				nMetaLen = 10;
			// メタ文字列っぽい
			}else if (pStr = _tcschr(ps, _T('%'))) {
				nMetaLen = pStr - ps;
				if (nMetaLen < _MAX_PATH) {
					auto_memcpy(szMeta, ps, nMetaLen);
					szMeta[nMetaLen] = _T('\0');
				}else {
					*pd = _T('\0');
					return false;
				}
#ifdef _USE_META_ALIAS
				// メタ文字列がエイリアス名なら書き換える
				const MetaAlias* pAlias;
				for (pAlias=&AliasList[0]; nMetaLen<pAlias->nLenth; ++pAlias)
					; // 読み飛ばす
				for (; nMetaLen==pAlias->nLenth; ++pAlias) {
					if (auto_stricmp(pAlias->szAlias, szMeta) == 0) {
						_tcscpy(szMeta, pAlias->szOrig);
						break;
					}
				}
#endif
				// 直接レジストリで調べる
				szPath[0] = _T('\0');
				bFolderPath = ReadRegistry(HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, _countof(szPath));
				if (!bFolderPath || _T('\0') == szPath[0]) {
					bFolderPath = ReadRegistry(HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, _countof(szPath));
				}
				if (!bFolderPath || _T('\0') == szPath[0]) {
					pStr = _tgetenv(szMeta);
					// 環境変数
					if (pStr) {
						nPathLen = _tcslen(pStr);
						if (nPathLen < _MAX_PATH) {
							_tcscpy(szPath, pStr);
						}else {
							*pd = _T('\0');
							return false;
						}
					// 未定義のメタ文字列は 入力された%...%を，そのまま文字として処理する
					}else if (pd + (nMetaLen + 2) < pd_end) {
						*pd = _T('%');
						auto_memcpy(&pd[1], ps, nMetaLen);
						pd[nMetaLen + 1] = _T('%');
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else {
						*pd = _T('\0');
						return false;
					}
				}
			}else {
				// %...%の終わりの%がない とりあえず，%をコピー
				*pd = _T('%');
				++pd;
				--ps; // 先に++psしてしまったので戻す
				continue;
			}

			// ロングファイル名にする
			nPathLen = _tcslen(szPath);
			LPTSTR pStr2 = szPath;
			if (nPathLen < _MAX_PATH && nPathLen != 0) {
				if (FALSE != GetLongFileName(szPath, szMeta)) {
					pStr2 = szMeta;
				}
			}

			// 最後のフォルダ区切り記号を削除する
			// [A:\]などのルートであっても削除
			for (nPathLen=0; pStr2[nPathLen]!=_T('\0'); ++nPathLen) {
				if (_T('\\') == pStr2[nPathLen] && _T('\0') == pStr2[nPathLen + 1]) {
					pStr2[nPathLen] = _T('\0');
					break;
				}
			}

			if (pd + nPathLen < pd_end && nPathLen != 0) {
				auto_memcpy(pd, pStr2, nPathLen);
				pd += nPathLen;
				ps += nMetaLen;
			}else {
				*pd = _T('\0');
				return false;
			}
		}else {
			// 最後の文字が%だった
			*pd = *ps;
			++pd;
		}
	}
	*pd = _T('\0');
	return true;
}


// static
TCHAR CFileNameManager::GetAccessKeyByIndex(int index, bool bZeroOrigin)
{
	if (index < 0) {
		return 0;
	}
	int accKeyIndex = ((bZeroOrigin? index: index + 1) % 36);
	TCHAR c = (TCHAR)((accKeyIndex < 10) ? (_T('0') + accKeyIndex) : (_T('A') + accKeyIndex - 10));
	return c;
}

static void GetAccessKeyLabelByIndex(TCHAR* pszLabel, bool bEspaceAmp, int index, bool bZeroOrigin)
{
	TCHAR c = CFileNameManager::GetAccessKeyByIndex(index, bZeroOrigin);
	if (c) {
		if (bEspaceAmp) {
			pszLabel[0] = _T('&');
			pszLabel[1] = c;
			pszLabel[2] = _T(' ');
			pszLabel[3] = _T('\0');
		}else {
			pszLabel[0] = c;
			pszLabel[1] = _T(' ');
			pszLabel[2] = _T('\0');
		}
	}else {
		pszLabel[0] = _T('\0');
	}
}

/*
	@param editInfo      ウィンドウ情報。NULで不明扱い
	@param index         いつも0originで指定。 -1で非表示
	@param bZeroOrigin   アクセスキーを0から振る
*/
bool CFileNameManager::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const EditInfo* editInfo, int nId, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
) {
	const EditInfo* pfi = editInfo;
	TCHAR szAccKey[4];
	int ret = 0;
	if (!pfi) {
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_UNKOWN), szAccKey);
		return 0 < ret;
	}else if (pfi->m_bIsGrep) {
		
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		//pfi->m_szGrepKeyShort → cmemDes
		CNativeW cmemDes;
		int nGrepKeyLen = wcslen(pfi->m_szGrepKey);
		const int GREPKEY_LIMIT_LEN = 64;
		// CSakuraEnvironment::ExpandParameter では 32文字制限
		// メニューは 64文字制限
		LimitStringLengthW(pfi->m_szGrepKey, nGrepKeyLen, GREPKEY_LIMIT_LEN, cmemDes);
		
		const TCHAR* pszKey;
		TCHAR szMenu2[GREPKEY_LIMIT_LEN * 2 * 2 + 1]; // WCHAR=>ACHARで2倍、&で2倍
		if (bEspaceAmp) {
			dupamp(cmemDes.GetStringT(), szMenu2);
			pszKey = szMenu2;
		}else {
			pszKey = cmemDes.GetStringT();
		}

		// szMenuを作る
		// Jan. 19, 2002 genta
		// &の重複処理を追加したため継続判定を若干変更
		// 20100729 ExpandParameterにあわせて、・・・を...に変更
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_GREP),
			szAccKey, pszKey,
			(nGrepKeyLen > cmemDes.GetStringLength()) ? _T("..."):_T("")
		);
	}else if (pfi->m_bIsDebug) {
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_OUTPUT), szAccKey);
	}else {
		return GetMenuFullLabel(pszOutput, nBuffSize, bEspaceAmp, pfi->m_szPath, nId, pfi->m_bIsModified, pfi->m_nCharCode, bFavorite,
			 index, bAccKeyZeroOrigin, hDC);
	}
	return 0 < ret;
}

bool CFileNameManager::GetMenuFullLabel(
	TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const TCHAR* pszFile, int nId, bool bModified, ECodeType nCharCode, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
) {
	TCHAR szAccKey[4];
	TCHAR szFileName[_MAX_PATH];
	TCHAR szMenu2[_MAX_PATH * 2];
	const TCHAR* pszName;

	GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
	if (pszFile[0]) {
		this->GetTransformFileNameFast( pszFile, szFileName, _MAX_PATH, hDC );

		// szFileName → szMenu2
		// Jan. 19, 2002 genta
		// メニュー文字列の&を考慮
		if (bEspaceAmp) {
			dupamp(szFileName, szMenu2);
			pszName = szMenu2;
		}else {
			pszName = szFileName;
		}
	}else {
		if (nId == -1) {
			wsprintf(szFileName, LS(STR_NO_TITLE1));
		}else {
			wsprintf(szFileName, _T("%s%d"), LS(STR_NO_TITLE1), nId);
		}
		pszName = szFileName;
	}
	const TCHAR* pszCharset = _T("");
	TCHAR szCodePageName[100];
	if (IsValidCodeTypeExceptSJIS(nCharCode)) {
		pszCharset = CodeTypeName(nCharCode).Bracket();
	}else if (IsValidCodeOrCPTypeExceptSJIS(nCharCode)) {
		CodePage::GetNameBracket(szCodePageName, nCharCode);
		pszCharset = szCodePageName;
	}
	
	int ret = auto_snprintf_s(pszOutput, nBuffSize, _T("%ts%ts%ts %ts%ts"),
		szAccKey, (bFavorite ? _T("★ ") : _T("")), pszName,
		(bModified ? _T("*"):_T(" ")), pszCharset
	);
	return 0 < ret;
}


/**
	構成設定ファイルからiniファイル名を取得する

	sakura.exe.iniからsakura.iniの格納フォルダを取得し、フルパス名を返す

	@param[out] pszPrivateIniFile マルチユーザ用のiniファイルパス
	@param[out] pszIniFile EXE基準のiniファイルパス

	@author ryoji
	@date 2007.09.04 ryoji 新規作成
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
void CFileNameManager::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile, LPCTSTR pszProfName )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		NULL,
		szPath, _countof(szPath)
	);
	_tsplitpath(szPath, szDrive, szDir, szFname, szExt);

	if (pszProfName[0] == '\0') {
		auto_snprintf_s(pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".ini"));
	}else {
		auto_snprintf_s( pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts\\%ts%ts"), szDrive, szDir, pszProfName, szFname, _T(".ini") );
	}

	// マルチユーザ用のiniファイルパス
	//		exeと同じフォルダに置かれたマルチユーザ構成設定ファイル（sakura.exe.ini）の内容
	//		に従ってマルチユーザ用のiniファイルパスを決める
	pszPrivateIniFile[0] = _T('\0');
	if (IsWin2000_or_later()) {
		auto_snprintf_s(szPath, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".exe.ini"));
		int nEnable = ::GetPrivateProfileInt(_T("Settings"), _T("MultiUser"), 0, szPath);
		if (nEnable) {
			int nFolder = ::GetPrivateProfileInt(_T("Settings"), _T("UserRootFolder"), 0, szPath);
			switch (nFolder) {
			case 1:
				nFolder = CSIDL_PROFILE;			// ユーザのルートフォルダ
				break;
			case 2:
				nFolder = CSIDL_PERSONAL;			// ユーザのドキュメントフォルダ
				break;
			case 3:
				nFolder = CSIDL_DESKTOPDIRECTORY;	// ユーザのデスクトップフォルダ
				break;
			default:
				nFolder = CSIDL_APPDATA;			// ユーザのアプリケーションデータフォルダ
				break;
			}
			::GetPrivateProfileString(_T("Settings"), _T("UserSubFolder"), _T("sakura"), szDir, _MAX_DIR, szPath);
			if (szDir[0] == _T('\0'))
				::lstrcpy(szDir, _T("sakura"));
			if (GetSpecialFolderPath(nFolder, szPath)) {
				if (pszProfName[0] == '\0') {
					auto_snprintf_s(pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, _T(".ini"));
				}else {
					auto_snprintf_s( pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, pszProfName, _T(".ini") );
				}
	
	
	
	
			}
		}
	}
}

/**
	iniファイル名の取得

	共有データからsakura.iniの格納フォルダを取得し、フルパス名を返す
	（共有データ未設定のときは共有データ設定を行う）

	@param[out] pszIniFileName iniファイル名（フルパス）
	@param[in] bRead true: 読み込み / false: 書き込み

	@author ryoji
	@date 2007.05.19 ryoji 新規作成
*/
void CFileNameManager::GetIniFileName( LPTSTR pszIniFileName, LPCTSTR pszProfName, BOOL bRead/*=FALSE*/ )
{
	auto& iniFolder = m_pShareData->m_fileNameManagement.m_IniFolder;
	if (!iniFolder.m_bInit) {
		iniFolder.m_bInit = true;			// 初期化済フラグ
		iniFolder.m_bReadPrivate = false;	// マルチユーザ用iniからの読み出しフラグ
		iniFolder.m_bWritePrivate = false;	// マルチユーザ用iniへの書き込みフラグ

		GetIniFileNameDirect(iniFolder.m_szPrivateIniFile, iniFolder.m_szIniFile, pszProfName);
		if (iniFolder.m_szPrivateIniFile[0] != _T('\0')) {
			iniFolder.m_bReadPrivate = true;
			iniFolder.m_bWritePrivate = true;
			if (CommandLine::getInstance()->IsNoWindow() && CommandLine::getInstance()->IsWriteQuit())
				iniFolder.m_bWritePrivate = false;

			// マルチユーザ用のiniフォルダを作成しておく
			if (iniFolder.m_bWritePrivate) {
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( iniFolder.m_szPrivateIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf_s( szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir );
				MakeSureDirectoryPathExistsT( szPath );
			}
		}else {
			if (pszProfName[0] != _T('\0')) {
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( iniFolder.m_szIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf_s(szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir);
				MakeSureDirectoryPathExistsT(szPath);
			}
		}
	}

	bool bPrivate = bRead ? iniFolder.m_bReadPrivate : iniFolder.m_bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? iniFolder.m_szPrivateIniFile: iniFolder.m_szIniFile );
}

