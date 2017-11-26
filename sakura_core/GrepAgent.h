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
		EncodingType			nGrepCharSet,			// 文字コードセット選択
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
		size_t					pathLen,
		const TCHAR*			pszBasePath,		// [in] 検索対象パス(ベース)
		size_t					basePathLen,
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
		size_t			nColumn,		//	マッチした桁番号
		const wchar_t*	pCompareData,	//	行の文字列
		size_t			nLineLen,		//	行の文字列の長さ
		size_t			nEolCodeLen,	//	EOLの長さ
		// マッチした文字列の情報
		const wchar_t*	pMatchData,		//	マッチした文字列
		size_t			nMatchLen,		//	マッチした文字列の長さ
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
	bool	bGrepMode;		// Grepモードか
	bool	bGrepRunning;		// Grep処理中
private:
	ULONGLONG lastViewDstAddedTime;
	std::vector<std::pair<const wchar_t*, size_t>> searchWords;
	NativeW memBuf;
	NativeW unicodeBuffer;
	FileLoad fl;
	int oldSetHitCnt;
	LONGLONG oldCheckTime = 0;
};

