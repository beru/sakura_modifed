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

#include "doc/DocListener.h"
#include "io/FileLoad.h"

class DlgCancel;
class EditView;
class SearchStringPattern;
class GrepEnumKeys;
class GrepEnumFiles;
class GrepEnumFolders;

struct GrepOption {
	bool		bGrepReplace;			// Grep置換
	bool		bGrepSubFolder;			// サブフォルダからも検索する
	bool		bGrepStdout;			// 標準出力モード
	bool		bGrepHeader;			// ヘッダ・フッダ表示
	EncodingType	nGrepCharSet;		// 文字コードセット選択
	int			nGrepOutputLineType;	// 0:ヒット部分を出力, 1: ヒット行を出力, 2: 否ヒット行を出力
	int			nGrepOutputStyle;		// 出力形式 1: Normal, 2: WZ風(ファイル単位) 3: 結果のみ
	bool		bGrepOutputFileOnly;	// ファイル毎最初のみ検索
	bool		bGrepOutputBaseFolder;	// ベースフォルダ表示
	bool		bGrepSeparateFolder;	// フォルダ毎に表示
	bool		bGrepPaste;				// Grep置換：クリップボードから貼り付ける
	bool		bGrepBackup;			// Grep置換：バックアップ

	GrepOption() : 
		 bGrepReplace(false)
		,bGrepSubFolder(true)
		,bGrepStdout(false)
		,bGrepHeader(true)
		,nGrepCharSet(CODE_AUTODETECT)
		,nGrepOutputLineType(1)
		,nGrepOutputStyle(1)
		,bGrepOutputFileOnly(false)
		,bGrepOutputBaseFolder(false)
		,bGrepSeparateFolder(false)
		,bGrepPaste(false)
		,bGrepBackup(false)
	{}
};

//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
//	Mar. 28, 2004 genta DoGrepFileから不要な引数を削除
class GrepAgent : public DocListenerEx {
public:
	GrepAgent();

	// イベント
	CallbackResultType OnBeforeClose();
	void OnAfterSave(const SaveInfo& saveInfo);

	static void CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths );
	static std::tstring ChopYen( const std::tstring& str );

	// Grep実行
	DWORD DoGrep(
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
		EncodingType			nGrepCharSet,			// 2002/09/21 Moca 文字コードセット選択
		int						nGrepOutputLineType,
		int						nGrepOutputStyle,
		bool					bGrepOutputFileOnly,	// [in] ファイル毎最初のみ出力
		bool					bGrepOutputBaseFolder,	// [in] ベースフォルダ表示
		bool					bGrepSeparateFolder,	// [in] フォルダ毎に表示
		bool					bGrepPaste,
		bool					bGrepBackup
	);

private:
	// Grep実行
	int DoGrepTree(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,			// [in] Cancelダイアログへのポインタ
		const wchar_t*			pszKey,				// [in] 検索パターン
		size_t					nKeyLen,
		const NativeW&			mGrepReplace,
		const GrepEnumKeys&		grepEnumKeys,		// [in] 検索対象ファイルパターン(!で除外指定)
		GrepEnumFiles&			grepExceptAbsFiles,
		GrepEnumFolders&		grepExceptAbsFolders,
		const TCHAR*			pszPath,			// [in] 検索対象パス
		const TCHAR*			pszBasePath,		// [in] 検索対象パス(ベース)
		const SearchOption&		searchOption,		// [in] 検索オプション
		const GrepOption&		grepOption,			// [in] Grepオプション
		const SearchStringPattern& pattern,			// [in] 検索パターン
		Bregexp&				regexp,				// [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
		int						nNest,				// [in] ネストレベル
		bool&					bOutputBaseFolder,
		int*					pnHitCount,			// [i/o] ヒット数の合計
		NativeW&				memMessage
	);

	// Grep実行
	int DoGrepFile(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,
		const wchar_t*			pszKey,
		size_t					nKeyLen,
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
	);

	int DoGrepReplaceFile(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,
		const wchar_t*			pszKey,
		size_t					nKeyLen,
		const NativeW&			mGrepReplace,
		const TCHAR*			pszFile,
		const SearchOption&		searchOption,
		const GrepOption&		grepOption,
		const SearchStringPattern& pattern,
		Bregexp&				regexp,
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		const TCHAR*			pszBaseFolder,
		const TCHAR*			pszFolder,
		const TCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		NativeW&				memMessage
	);

	// Grep結果をpszWorkに格納
	void SetGrepResult(
		// データ格納先
		NativeW&		memMessage,
		// マッチしたファイルの情報
		const TCHAR*	pszFilePath,	//	フルパス or 相対パス
		const TCHAR*	pszCodeName,	//	文字コード情報"[SJIS]"とか
		// マッチした行の情報
		LONGLONG		nLine,			//	マッチした行番号
		int				nColumn,		//	マッチした桁番号
		const wchar_t*	pCompareData,	//	行の文字列
		int				nLineLen,		//	行の文字列の長さ
		int				nEolCodeLen,	//	EOLの長さ
		// マッチした文字列の情報
		const wchar_t*	pMatchData,		//	マッチした文字列
		int				nMatchLen,		//	マッチした文字列の長さ
		// オプション
		const GrepOption&	grepOption
	);
	void AddTail(
		EditWnd& editWnd,
		EditView& editView,
		const NativeW& mem,
		bool bAddStdout
	);

public: //$$ 仮
	bool	m_bGrepMode;		// Grepモードか
	bool	m_bGrepRunning;		// Grep処理中
private:
	ULONGLONG m_lastStaticCurFileSetTime;
	ULONGLONG m_lastViewDstAddedTime;
	std::vector<std::pair<const wchar_t*, LogicInt>> m_searchWords;
	NativeW m_memBuf;
	NativeW m_unicodeBuffer;
	FileLoad m_fl;	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
};

