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
#pragma once

#include "doc/CDocListener.h"
class CDlgCancel;
class CEditView;
class CSearchStringPattern;
class CGrepEnumKeys;
class CGrepEnumFiles;
class CGrepEnumFolders;

struct SGrepOption {
	bool		bGrepSubFolder;			//!< サブフォルダからも検索する
	ECodeType	nGrepCharSet;			//!< 文字コードセット選択
	bool		bGrepOutputLine;		//!< true: ヒット行を出力 / false: ヒット部分を出力
	int			nGrepOutputStyle;		//!< 出力形式 1: Normal, 2: WZ風(ファイル単位) 3: 結果のみ
	bool		bGrepOutputFileOnly;	//!< ファイル毎最初のみ検索
	bool		bGrepOutputBaseFolder;	//!< ベースフォルダ表示
	bool		bGrepSeparateFolder;	//!< フォルダ毎に表示

	SGrepOption() : 
		 bGrepSubFolder(true)
		,nGrepCharSet(CODE_AUTODETECT)
		,bGrepOutputLine(true)
		,nGrepOutputStyle(true)
		,bGrepOutputFileOnly(false)
		,bGrepOutputBaseFolder(false)
		,bGrepSeparateFolder(false)
	{}
};

//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
//	Mar. 28, 2004 genta DoGrepFileから不要な引数を削除
class CGrepAgent : public CDocListenerEx {
public:
	CGrepAgent();

	// イベント
	ECallbackResult OnBeforeClose();
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	static
	void CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths );

	// Grep実行
	DWORD DoGrep(
		CEditView*				pcViewDst,
		const CNativeW*			pcmGrepKey,
		const CNativeT*			pcmGrepFile,
		const CNativeT*			pcmGrepFolder,
		bool					bGrepCurFolder,
		BOOL					bGrepSubFolder,
		const SSearchOption&	sSearchOption,
		ECodeType				nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
		BOOL					bGrepOutputLine,
		int						nGrepOutputStyle,
		bool					bGrepOutputFileOnly,	//!< [in] ファイル毎最初のみ出力
		bool					bGrepOutputBaseFolder,	//!< [in] ベースフォルダ表示
		bool					bGrepSeparateFolder	//!< [in] フォルダ毎に表示
	);

private:
	// Grep実行
	int DoGrepTree(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,		//!< [in] Cancelダイアログへのポインタ
		const wchar_t*			pszKey,				//!< [in] 検索パターン
		CGrepEnumKeys&			cGrepEnumKeys,		//!< [in] 検索対象ファイルパターン(!で除外指定)
		CGrepEnumFiles&			cGrepExceptAbsFiles,
		CGrepEnumFolders&		cGrepExceptAbsFolders,
		const TCHAR*			pszPath,			//!< [in] 検索対象パス
		const TCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベース)
		const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
		const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
		const CSearchStringPattern& pattern,		//!< [in] 検索パターン
		CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
		int						nNest,				//!< [in] ネストレベル
		bool&					bOutputBaseFolder,
		int*					pnHitCount			//!< [i/o] ヒット数の合計
	);

	// Grep実行
	int DoGrepFile(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,
		const wchar_t*			pszKey,
		const TCHAR*			pszFile,
		const SSearchOption&	sSearchOption,
		const SGrepOption&		sGrepOption,
		const CSearchStringPattern& pattern,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		const TCHAR*			pszBaseFolder,
		const TCHAR*			pszFolder,
		const TCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		CNativeW&				cmemMessage
	);

	// Grep結果をpszWorkに格納
	void SetGrepResult(
		// データ格納先
		CNativeW&		cmemMessage,
		// マッチしたファイルの情報
		const TCHAR*	pszFilePath,	//	フルパス or 相対パス
		const TCHAR*	pszCodeName,	//	文字コード情報"[SJIS]"とか
		// マッチした行の情報
		int				nLine,			//	マッチした行番号
		int				nColumn,		//	マッチした桁番号
		const wchar_t*	pCompareData,	//	行の文字列
		int				nLineLen,		//	行の文字列の長さ
		int				nEolCodeLen,	//	EOLの長さ
		// マッチした文字列の情報
		const wchar_t*	pMatchData,		//	マッチした文字列
		int				nMatchLen,		//	マッチした文字列の長さ
		// オプション
		const SGrepOption&	sGrepOption
	);

public: //$$ 仮
	bool	m_bGrepMode;		//!< Grepモードか
	bool	m_bGrepRunning;		//!< Grep処理中
};

