#include "StdAfx.h"
#include <io.h>
#include "fileUtil.h"
#include "charset/CharPointer.h"
#include "util/module.h"
#include "util/window.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/FileNameManager.h"
#include "_main/CommandLine.h"

bool fexist(LPCTSTR pszPath)
{
	return _taccess(pszPath, 0) != -1;
}

/**	ファイル名の切り出し

	指定文字列からファイル名と認識される文字列を取り出し、
	先頭Offset及び長さを返す。
	
	@retval true ファイル名発見
	@retval false ファイル名は見つからなかった
*/
bool IsFilePath(
	const wchar_t*	pLine,		// [in]  探査対象文字列
	size_t*			pnBgn,		// [out] 先頭offset。pLine + *pnBgnがファイル名先頭へのポインタ。
	int*			pnPathLen,	// [out] ファイル名の長さ
	bool			bFileOnly	// [in]  true: ファイルのみ対象 / false: ディレクトリも対象
	)
{
	wchar_t	szJumpToFile[_MAX_PATH];
	wmemset(szJumpToFile, 0, _countof(szJumpToFile));

	size_t	nLineLen = wcslen(pLine);

	// 先頭の空白を読み飛ばす
	size_t i;
	for (i=0; i<nLineLen; ++i) {
		wchar_t c = pLine[i];
		if (L' ' != c && L'\t' != c && L'\"' != c) {
			break;
		}
	}

	// #include <ファイル名>の考慮
	// #で始まるときは"または<まで読み飛ばす
	if (i < nLineLen && L'#' == pLine[i]) {
		for (; i<nLineLen; ++i) {
			if (L'<'  == pLine[i] || L'\"' == pLine[i]) {
				++i;
				break;
			}
		}
	}

	// この時点で既に行末に達していたらファイル名は見つからない
	if (i >= nLineLen) {
		return false;
	}

	*pnBgn = i;
	int cur_pos = 0;
	int tmp_end = 0;
	for (; i<=nLineLen && cur_pos+1<_countof(szJumpToFile); ++i) {
		// ファイル名終端を検知する
		if (WCODE::IsLineDelimiterExt(pLine[i]) || pLine[i] == L'\0') {
			break;
		}

		// ファイル名終端を検知する
		if ((i == nLineLen    ||
			  pLine[i] == L' ' ||
			  pLine[i] == L'\t'||
			  pLine[i] == L'(' ||
			  pLine[i] == L'"' ||
			  wcschr(L")'`[]{};#!@&%$", pLine[i])
			) &&
			szJumpToFile[0] != L'\0'
		) {
			// ファイル存在確認
			if (IsFileExists(to_tchar(szJumpToFile), bFileOnly)) {
				tmp_end = cur_pos;
			}
		}

		// C:\の:はファイル区切りと見なして欲しくない
		if (cur_pos > 1 && pLine[i] == L':') {
			break;
		}
		// ファイル名に使えない文字が含まれていたら、即ループ終了
		if (!WCODE::IsValidFilenameChar(pLine, i)) {
			break;
		}

		szJumpToFile[cur_pos] = pLine[i];
		++cur_pos;
	}

	// ファイル存在確認方法変更
	if (szJumpToFile[0] != L'\0' && IsFileExists(to_tchar(szJumpToFile), bFileOnly)) {
		tmp_end = cur_pos;
	}
	if (tmp_end != 0) {
		*pnPathLen = tmp_end;
		return true;
	}

	return false;

}

/*!
	ローカルドライブの判定

	@param[in] pszDrive ドライブ名を含むパス名
	
	@retval true ローカルドライブ
	@retval false リムーバブルドライブ．ネットワークドライブ．
*/
bool IsLocalDrive(const TCHAR* pszDrive)
{
	TCHAR szDriveType[_MAX_DRIVE + 1];	// "A:\ "登録用
	long lngRet;

	if (iswalpha(pszDrive[0])) {
		auto_sprintf_s(szDriveType, _T("%tc:\\"), _totupper(pszDrive[0]));
		lngRet = GetDriveType(szDriveType);
		if (lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE) {
			return false;
		}
	}else if (pszDrive[0] == _T('\\') && pszDrive[1] == _T('\\')) {
		// ネットワークパス
		return false;
	}
	return true;
}


const TCHAR* GetFileTitlePointer(const TCHAR* tszPath)
{
	const TCHAR* pszName;
	CharPointerT p;
	p = pszName = tszPath;
	while (*p) {
		if (*p == _T('\\')) {
			pszName = p + 1;
			++p;
		}else {
			++p;
		}
	}
	return pszName;
}


/*! fnameが相対パスの場合は、実行ファイルのパスからの相対パスとして開く */
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode)
{
	if (_IS_REL_PATH(fname)) {
		TCHAR path[_MAX_PATH];
		GetExedir(path, fname);
		return _tfopen(path, mode);
	}
	return _tfopen(fname, mode);
}

/*! fnameが相対パスの場合は、INIファイルのパスからの相対パスとして開く */
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, bool bOrExedir/*=true*/)
{
	if (_IS_REL_PATH(fname)) {
		TCHAR path[_MAX_PATH];
		if (bOrExedir) {
			GetInidirOrExedir(path, fname);
		}else {
			GetInidir(path, fname);
		}
		return _tfopen(path, mode);
	}
	return _tfopen(fname, mode);
}


// フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない
void CutLastYenFromDirectoryPath(TCHAR* pszFolder)
{
	if (_tcslen(pszFolder) == 3
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	) {
		// ドライブ名:'\\'
	}else {
		// フォルダの最後が半角かつ'\\'の場合は、取り除く
		size_t nFolderLen = _tcslen(pszFolder);
		if (0 < nFolderLen) {
			ptrdiff_t nCharChars = &pszFolder[nFolderLen] - NativeT::GetCharPrev(pszFolder, nFolderLen, &pszFolder[nFolderLen]);
			if (nCharChars == 1 && pszFolder[nFolderLen - 1] == _T('\\')) {
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	return;
}


// フォルダの最後が半角かつ'\\'でない場合は、付加する
void AddLastYenFromDirectoryPath(CHAR* pszFolder)
{
	if (auto_strlen(pszFolder) == 3
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	) {
		// ドライブ名:'\\'
	}else {
		// フォルダの最後が半角かつ'\\'でない場合は、付加する
		size_t	nFolderLen = auto_strlen(pszFolder);
		if (0 < nFolderLen) {
			ptrdiff_t nCharChars = &pszFolder[nFolderLen] - NativeA::GetCharPrev(pszFolder, nFolderLen, &pszFolder[nFolderLen]);
			if (nCharChars == 1 && ('\\' == pszFolder[nFolderLen - 1] || '/' == pszFolder[nFolderLen - 1])) {
			}else {
				pszFolder[nFolderLen] = '\\';
				pszFolder[nFolderLen + 1] = '\0';
			}
		}
	}
	return;
}

void AddLastYenFromDirectoryPath(wchar_t* pszFolder)
{
	if (auto_strlen(pszFolder) == 3
	 && pszFolder[1] == L':'
	 && pszFolder[2] == L'\\'
	) {
		// ドライブ名:'\\'
	}else {
		// フォルダの最後が半角かつ'\\'でない場合は、付加する
		size_t	nFolderLen = auto_strlen(pszFolder);
		if (0 < nFolderLen) {
			if (L'\\' == pszFolder[nFolderLen - 1] || L'/' == pszFolder[nFolderLen - 1]) {
			}else {
				pszFolder[nFolderLen] = L'\\';
				pszFolder[nFolderLen + 1] = L'\0';
			}
		}
	}
	return;
}


// ファイルのフルパスを、フォルダとファイル名に分割
// [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt]
void SplitPath_FolderAndFile(
	const TCHAR* pszFilePath,
	TCHAR* pszFolder,
	TCHAR* pszFile
	)
{
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath(pszFilePath, szDrive, szDir, szFname, szExt);
	if (pszFolder) {
		_tcscpy(pszFolder, szDrive);
		_tcscat(pszFolder, szDir);
		// フォルダの最後が半角かつ'\\'の場合は、取り除く
		size_t nFolderLen = _tcslen(pszFolder);
		if (0 < nFolderLen) {
			ptrdiff_t nCharChars = &pszFolder[nFolderLen] - NativeT::GetCharPrev(pszFolder, nFolderLen, &pszFolder[nFolderLen]);
			if (nCharChars == 1 && pszFolder[nFolderLen - 1] == _T('\\')) {
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	if (pszFile) {
		_tcscpy(pszFile, szFname);
		_tcscat(pszFile, szExt);
	}
	return;
}

/* フォルダ、ファイル名から、結合したパスを作成
 * [c:\work\test] + [aaa.txt] → [c:\work\test\aaa.txt]
 * フォルダ末尾に円記号があってもなくても良い。
 */
void Concat_FolderAndFile(
	const TCHAR* pszDir,
	const TCHAR* pszTitle,
	TCHAR* pszPath
	)
{
	TCHAR* out = pszPath;
	// フォルダをコピー
	for (const TCHAR* in=pszDir; *in!='\0'; ) {
		*out++ = *in++;
	}
	// 円記号を付加
#if UNICODE
	if (*(out-1) != '\\') { *out++ = '\\'; }
#else
	if (*(out-1) != '\\' ||
		(out - NativeT::GetCharPrev(pszDir, out - pszDir, out) == 1)
	) {
		*out++ = '\\';
	}
#endif
	// ファイル名をコピー
	for (const TCHAR* in=pszTitle; *in!='\0'; ) {
		*out++ = *in++;
	}
	*out = '\0';
	return;
}


/*! ロングファイル名を取得する 

	@param[in] pszFilePathSrc 変換元パス名
	@param[out] pszFilePathDes 結果書き込み先 (長さMAX_PATHの領域が必要)
*/
BOOL GetLongFileName(
	const TCHAR* pszFilePathSrc,
	TCHAR* pszFilePathDes
	)
{
	TCHAR* name;
	TCHAR szBuf[_MAX_PATH + 1];
	int len = ::GetFullPathName(pszFilePathSrc, _MAX_PATH, szBuf, &name);
	if (len <= 0 || _MAX_PATH <= len) {
		len = ::GetLongPathName(pszFilePathSrc, pszFilePathDes, _MAX_PATH);
		if (len <= 0 || _MAX_PATH < len) {
			return FALSE;
		}
		return TRUE;
	}
	len = ::GetLongPathName(szBuf, pszFilePathDes, _MAX_PATH);
	if (len <= 0 || _MAX_PATH < len) {
		_tcscpy(pszFilePathDes, szBuf);
	}
	return TRUE;
}


// 拡張子を調べる
BOOL CheckEXT(
	const TCHAR* pszPath,
	const TCHAR* pszExt
	)
{
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(pszPath, NULL, NULL, NULL, szExt);
	TCHAR* pszWork = szExt;
	if (pszWork[0] == _T('.')) {
		++pszWork;
	}
	if (_tcsicmp(pszExt, pszWork) == 0) {
		return TRUE;
	}else {
		return FALSE;
	}
}

/*! 相対パスか判定する */
bool _IS_REL_PATH(const TCHAR* path)
{
	bool ret = true;
	if ((_T('A') <= path[0] && path[0] <= _T('Z') || _T('a') <= path[0] && path[0] <= _T('z'))
		&& path[1] == _T(':') && path[2] == _T('\\')
		|| path[0] == _T('\\') && path[1] == _T('\\')
	) {
		ret = false;
	}
	return ret;
}


/*! @brief ディレクトリの深さを計算する

	与えられたパス名からディレクトリの深さを計算する．
	パスの区切りは\．ルートディレクトリが深さ0で，サブディレクトリ毎に
	深さが1ずつ上がっていく．
*/
int CalcDirectoryDepth(
	const TCHAR* path	// [in] 深さを調べたいファイル/ディレクトリのフルパス
	)
{
	int depth = 0;
 
	// とりあえず\の数を数える
	for (CharPointerT p=path; *p!=_T('\0'); ++p) {
		if (*p == _T('\\')) {
			++depth;
			// フルパスには入っていないはずだが念のため
			// .\はカレントディレクトリなので，深さに関係ない．
			while (p[1] == _T('.') && p[2] == _T('\\')) {
				p += 2;
			}
		}
	}
 
	// 補正
	// ドライブ名はパスの深さに数えない
	if (_T('A') <= path[0] && path[0] <= _T('Z') && path[1] == _T(':') && path[2] == _T('\\')) {
		// フルパス
		--depth; // C:\ の \ はルートの記号なので階層深さではない
	}else if (path[0] == _T('\\')) {
		if (path[1] == _T('\\')) {
			// ネットワークパス
			// 先頭の2つはネットワークを表し，その次はホスト名なので
			// ディレクトリ階層とは無関係
			depth -= 3;
		}else {
			// ドライブ名無しのフルパス
			// 先頭の\は対象外
			--depth;
		}
	}
	return depth;
}


/*!
	@brief exeファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
*/
void GetExedir(
	LPTSTR	pDir,	// [out] EXEファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR	szFile	// [in]  ディレクトリ名に結合するファイル名．
	)
{
	if (!pDir)
		return;
	
	TCHAR szPath[_MAX_PATH];
	// sakura.exe のパスを取得
	::GetModuleFileName(NULL, szPath, _countof(szPath));
	if (!szFile) {
		SplitPath_FolderAndFile(szPath, pDir, NULL);
	}else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile(szPath, szDir, NULL);
		auto_snprintf_s(pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile);
	}
}

/*!
	@brief INIファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
*/
void GetInidir(
	LPTSTR	pDir,				// [out] INIファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR szFile	/*=NULL*/	// [in] ディレクトリ名に結合するファイル名．
	)
{
	if (!pDir)
		return;
	
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	TCHAR szPath[_MAX_PATH];

	// sakura.ini のパスを取得
	FileNameManager::getInstance().GetIniFileName( szPath, strProfileName.c_str() );
	if (!szFile) {
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}else {
		TCHAR szDir[_MAX_PATH];
		SplitPath_FolderAndFile(szPath, szDir, NULL);
		auto_snprintf_s(pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile);
	}
}


/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）．
*/
void GetInidirOrExedir(
	LPTSTR	pDir,								// [out] INIファイルまたはEXEファイルのあるディレクトリを返す場所．
												//       予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR	szFile					/*=NULL*/,	// [in] ディレクトリ名に結合するファイル名．
	bool	bRetExedirIfFileEmpty	/*=false*/	// [in] ファイル名の指定が空の場合はEXEファイルのフルパスを返す．
	)
{
	// ファイル名の指定が空の場合はEXEファイルのフルパスを返す（オプション）
	if (bRetExedirIfFileEmpty && (!szFile || szFile[0] == _T('\0'))) {
		TCHAR szExedir[_MAX_PATH];
		GetExedir(szExedir, szFile);
		::lstrcpy(pDir, szExedir);
		return;
	}

	// INI基準のフルパスが実在すればそのパスを返す
	TCHAR szInidir[_MAX_PATH];
	GetInidir(szInidir, szFile);
	if (fexist(szInidir)) {
		::lstrcpy(pDir, szInidir);
		return;
	}

	// EXE基準のフルパスが実在すればそのパスを返す
	if (ShareData::getInstance().IsPrivateSettings()) {	// INIとEXEでパスが異なる場合
		TCHAR szExedir[_MAX_PATH];
		GetExedir(szExedir, szFile);
		if (fexist(szExedir)) {
			::lstrcpy(pDir, szExedir);
			return;
		}
	}

	// どちらにも実在しなければINI基準のフルパスを返す
	::lstrcpy(pDir, szInidir);
}

/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリの相対パスを返す（INIを優先）．
	@param pszPath [in] 対象パス
*/
LPCTSTR GetRelPath(LPCTSTR pszPath)
{
	TCHAR szPath[_MAX_PATH + 1];
	LPCTSTR pszFileName = pszPath;

	GetInidir(szPath, _T(""));
	size_t nLen = auto_strlen(szPath);
	if (auto_strnicmp(szPath, pszPath, nLen) == 0) {
		pszFileName = pszPath + nLen;
	}else {
		GetExedir(szPath, _T(""));
		nLen = auto_strlen(szPath);
		if (auto_strnicmp(szPath, pszPath, nLen) == 0) {
			pszFileName = pszPath + nLen;
		}
	}

	return pszFileName;
}


/**	ファイルの存在チェック

	指定されたパスのファイルが存在するかどうかを確認する。
	
	@param path [in] 調べるパス名
	@param bFileOnly [in] true: ファイルのみ対象 / false: ディレクトリも対象
	
	@retval true  ファイルは存在する
	@retval false ファイルは存在しない
*/
bool IsFileExists(const TCHAR* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		::FindClose(hFind);
		if (bFileOnly && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
		return true;
	}
	return false;
}

/**	ディレクトリチェック

	指定されたパスがディレクトリかどうかを確認する。

	@param pszPath [in] 調べるパス名

	@retval true  ディレクトリ
	@retval false ディレクトリではない
*/
bool IsDirectory(LPCTSTR pszPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(pszPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		::FindClose(hFind);
		return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	return false;
}


/*!	ファイルの更新日時を取得

	@return true: 成功, false: FindFirstFile失敗

	@note 書き込み後にファイルを再オープンしてタイムスタンプを得ようとすると
	ファイルがまだロックされていることがあり，上書き禁止と誤認されることがある．
	FindFirstFileを使うことでファイルのロック状態に影響されずにタイムスタンプを
	取得できる．
*/
bool GetLastWriteTimestamp(
	const TCHAR*	pszFileName,	// [in]  ファイルのパス
	FileTime*		pFileTime		// [out] 更新日時を返す場所
	)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFindFile = ::FindFirstFile(pszFileName, &ffd);
	if (hFindFile != INVALID_HANDLE_VALUE) {
		::FindClose(hFindFile);
		pFileTime->SetFILETIME(ffd.ftLastWriteTime);
		return true;
	}else {
		// ファイルが見つからなかった
		pFileTime->ClearFILETIME();
		return false;
	}
}


// -----------------------------------------------------------------------------
//
//
//                   MY_SP.c by SUI
//
//


/* ============================================================================
my_splitpath(const char* CommandLine, char* drive, char* dir, char* fname, char* ext);

★ 概要
CommandLine に与えられたコマンドライン文字列の先頭から、実在するファイル・ディ
レクトリの部分のみを抽出し、その抽出部分に対して _splitpath() と同等の処理をお
こないます。
先頭部分に実在するファイル・ディレクトリ名が無い場合は空文字列が返ります。
文字列中の日本語(Shift_JISコードのみ)に対応しています。

★ プログラム記述内容について(言い訳あれこれ)
文字列の split 処理部だけにでも _splitpath() を使えばもっと短くなったのですが、
そうやらずに全て自前で処理している理由は、
・コンパイラによっては _splitpath() が日本語に対応していない可能性もある。
・_splitpath() って、コンパイラによって、詳細動作が微妙に異なるかもしれないから
　仕様をハッキリさせるためにもコンパイラに添付されている _splitpath() にあまり
　頼りたくなかった。
・というか、主に動作確認に使用していた LSI-C試食版にはそもそも _splitpath() が
　存在しないから、やらざるをえなかった。 :-(
という事によります。

※ "LFN library" -> http://webs.to/ken/

★ 詳細動作
my_splitpath(CommandLine, drive, dir, fname, ext);
CommandLine に文字列として D:\Test.ext が与えられた場合、
├・D:\Test.ext というファイルが存在する場合
│　drive = "D:"
│　dir   = "\"
│　fname = "Test"
│　ext   = ".ext"
├・D:\Test.ext というディレクトリが存在する場合
│　drive = "D:"
│　dir   = "\Test.ext\"
│　fname = ""
│　ext   = ""
└・D:\Test.ext というファイル・ディレクトリが存在しない場合、
　　├・D:ドライブは有効
　　│　drive = "D:"
　　│　dir   = "\"
　　│　fname = ""
　　│　ext   = ""
　　└・D:ドライブは無効
　　　　drive = ""
　　　　dir   = ""
　　　　fname = ""
　　　　ext   = ""
)=========================================================================== */

/* Shift_JIS 対応で検索対象文字を２個指定できる strrchr() みたいなもの。
/ 指定された２つの文字のうち、見つかった方(より後方の方)の位置を返す。
/ # strrchr(char* s , char c) とは、文字列 s 中の最後尾の c を探し出す関数。
/ # 文字 c が見つかったら、その位置を返す。
/ # 文字 c が見つからない場合は NULL を返す。 */
char* sjis_strrchr2(const char* pt , const char ch1 , const char ch2) {
	const char* pf = NULL;
	while (*pt != '\0') {	// 文字列の終端まで調べる。
		if ((*pt == ch1) || (*pt == ch2))	pf = pt;	// pf = 検索文字の位置
		if (_IS_SJIS_1(*pt))	++pt;	// Shift_JIS の1文字目なら、次の1文字をスキップ
		if (*pt != '\0')		++pt;	// 次の文字へ
	}
	return (char*) pf;
}
wchar_t* wcsrchr2(const wchar_t* pt , const wchar_t ch1 , const wchar_t ch2) {
	const wchar_t* pf = NULL;
	while (*pt != L'\0') {	// 文字列の終端まで調べる。
		if ((*pt == ch1) || (*pt == ch2))	pf = pt;	// pf = 検索文字の位置
		if (*pt != '\0')		++pt;	// 次の文字へ
	}
	return (wchar_t*)pf;
}

#define		GetExistPath_NO_DriveLetter	0	// ドライブレターが無い
#define		GetExistPath_IV_Drive		1	// ドライブが無効
#define		GetExistPath_AV_Drive		2	// ドライブが有効

void GetExistPath(char* po , const char* pi)
{
	char*	pw;
	int		cnt;
	char	drv[4] = "_:\\";

	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 (_MAX_PATH -1) 文字まで
	/ po にコピーする。 */
	for (pw=po, cnt=0; (*pi!='\0') && (cnt<_MAX_PATH -1); ++pi) {
		// /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。
		if (*pi == '\"')	continue;		//  " なら何もしない。次の文字へ
		if (*pi == '/')		*pw++ = '\\';	//  / なら \ に変換してコピー
		else				*pw++ = *pi;	// その他の文字はそのままコピー
		++cnt;	// コピーした文字数 ++
	}
	*pw = '\0';		// 文字列終端

	// ドライブの状態
	int dl = GetExistPath_NO_DriveLetter;	//「ドライブレターが無い」にしておく
	if (
		(*(po + 1) == ':')&&
		(ACODE::IsAZ(*po))
	) {	// 先頭にドライブレターがある。そのドライブが有効かどうか判定する
		drv[0] = *po;
		if (access(drv, 0) == 0)		dl = GetExistPath_AV_Drive;		// 有効
		else						dl = GetExistPath_IV_Drive;		// 無効
	}

	if (dl == GetExistPath_IV_Drive) {	// ドライブ自体が無効
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = '\0';	// 返値文字列 = "";(空文字列)
		return;		// これ以上何もしない
	}

	// ps = 検索開始位置
	char* ps = po;	// ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す
	if ((*po == '\\') && (*(po + 1) == '\\')) ps += 2;

	if (*ps == '\0') {	// 検索対象が空文字列なら
		*po = '\0';		// 返値文字列 = "";(空文字列)
		return;			// これ以上何もしない
	}

	for (;;) {
		if (access(po, 0) == 0)	break;	// 有効なパス文字列が見つかった
		// ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。

		pw = sjis_strrchr2(ps, '\\',' ');	// 最末尾の \ か ' ' を探す。
		if (!pw) {	// 文字列中に '\\' も ' ' も無かった
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if (dl == GetExistPath_AV_Drive) {
				// 先頭に有効なドライブのドライブレターがある。
				*(po + 2) = '\0';		// ドライブレター部の文字列のみ返す
			}else {	// 有効なパス部分が全く見つからなかった
				*po = '\0';	// 返値文字列 = "";(空文字列)
			}
			break;		// ループを抜ける
		}
		// ↓ルートディレクトリを引っかけるための処理
		if ((*pw == '\\')&&(*(pw-1) == ':')) {	// C:\ とかの \ っぽい
			* (pw + 1) = '\0';		// \ の後ろの位置を文字列の終端にする。
			if (access(po, 0) == 0)	break;	// 有効なパス文字列が見つかった
		}
		*pw = '\0';		// \ か ' ' の位置を文字列の終端にする。
		// ↓末尾がスペースなら、スペースを全て削除する
		while ((pw != ps) && (*(pw-1) == ' '))	* --pw = '\0';
	}

	return;
}

void GetExistPathW(wchar_t* po , const wchar_t* pi)
{
	wchar_t* pw;
	int		cnt;
	wchar_t	drv[4] = L"_:\\";
	
	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 (_MAX_PATH-1) 文字まで
	/ po にコピーする。 */
	for (pw=po, cnt=0; (*pi!=L'\0') && (cnt<_MAX_PATH-1); ++pi) {
		// /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。
		if (*pi == L'\"')	continue;		//  " なら何もしない。次の文字へ
		if (*pi == L'/')	*pw++ = L'\\';	//  / なら \ に変換してコピー
		else				*pw++ = *pi;	// その他の文字はそのままコピー
		++cnt;	// コピーした文字数 ++
	}
	*pw = L'\0';		// 文字列終端

	// ドライブの状態
	int dl = GetExistPath_NO_DriveLetter;		//「ドライブレターが無い」にしておく
	if (*(po + 1) == L':' && WCODE::IsAZ(*po)) {	// 先頭にドライブレターがある。そのドライブが有効かどうか判定する
		drv[0] = *po;
		if (_waccess(drv, 0) == 0)	dl = GetExistPath_AV_Drive;		// 有効
		else						dl = GetExistPath_IV_Drive;		// 無効
	}

	if (dl == GetExistPath_IV_Drive) {	// ドライブ自体が無効
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = L'\0';	// 返値文字列 = "";(空文字列)
		return;			// これ以上何もしない
	}

	// ps = 検索開始位置
	wchar_t* ps = po;	// ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す
	if ((*po == L'\\')&&(*(po + 1) == L'\\'))	ps += 2;

	if (*ps == L'\0') {	// 検索対象が空文字列なら
		*po = L'\0';	// 返値文字列 = "";(空文字列)
		return;			// これ以上何もしない
	}

	for (;;) {
		if (_waccess(po, 0) == 0)	break;	// 有効なパス文字列が見つかった
		// ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。

		pw = wcsrchr2(ps, '\\',' ');	// 最末尾の \ か ' ' を探す。
		if (!pw) {	// 文字列中に '\\' も ' ' も無かった
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if (dl == GetExistPath_AV_Drive) {
				// 先頭に有効なドライブのドライブレターがある。
				*(po + 2) = L'\0';		// ドライブレター部の文字列のみ返す
			}else {	// 有効なパス部分が全く見つからなかった
				*po = L'\0';	// 返値文字列 = "";(空文字列)
			}
			break;		// ループを抜ける
		}
		// ↓ルートディレクトリを引っかけるための処理
		if ((*pw == L'\\')&&(*(pw-1) == L':')) {	// C:\ とかの \ っぽい
			* (pw + 1) = L'\0';		// \ の後ろの位置を文字列の終端にする。
			if (_waccess(po, 0) == 0)	break;	// 有効なパス文字列が見つかった
		}
		*pw = L'\0';		// \ か ' ' の位置を文字列の終端にする。
		// ↓末尾がスペースなら、スペースを全て削除する
		while ((pw != ps) && (*(pw-1) == L' '))	* --pw = L'\0';
	}

	return;
}

/* 与えられたコマンドライン文字列の先頭部分から実在するファイル・ディレクトリ
　 のパス文字列を抽出し、そのパスを分解して drv dir fnm ext に書き込む。
　 先頭部分に有効なパス名が存在しない場合、全てに空文字列が返る。 */
void my_splitpath_w(
	const wchar_t* comln,
	wchar_t* drv,
	wchar_t* dir,
	wchar_t* fnm,
	wchar_t* ext
	)
{
	wchar_t	ppp[_MAX_PATH];		// パス格納（作業用）
	wchar_t* pd;
	wchar_t* pf;
	wchar_t* pe;

	if (drv)	*drv = L'\0';
	if (dir)	*dir = L'\0';
	if (fnm)	*fnm = L'\0';
	if (ext)	*ext = L'\0';
	if (*comln == L'\0')	return;

	// コマンドライン先頭部分の実在するパス名を ppp に書き出す。
	GetExistPathW(ppp , comln);

	if (*ppp != L'\0') {	// ファイル・ディレクトリが存在する場合
		/* 先頭文字がドライブレターかどうか判定し、
		　 pd = ディレクトリ名の先頭位置に設定する。 */
		pd = ppp;
		if (*(pd + 1) == L':' && WCODE::IsAZ(*pd)) {	// 先頭にドライブレターがある。
			pd += 2;	// pd = ドライブレター部の後ろ
		}				//      (= ディレクトリ名の先頭位置)
		// ここまでで、pd = ディレクトリ名の先頭位置

		DWORD attr =  GetFileAttributesW(ppp);
		int a_dir = (attr & FILE_ATTRIBUTE_DIRECTORY) ?  1 : 0;

		if (!a_dir) {	// 見つけた物がファイルだった場合。
			pf = wcsrchr(ppp, L'\\');	// 最末尾の \ を探す。
			if (pf)	++pf;		// 見つかった→  pf=\の次の文字の位置
			else			pf = pd;	// 見つからない→pf=パス名の先頭位置
			// ここまでで pf = ファイル名の先頭位置
			pe = wcsrchr(pf, L'.');		// 最末尾の '.' を探す。
			if (pe) {					// 見つかった(pe = L'.'の位置)
				if (ext) {	// 拡張子を返値として書き込む。
					wcsncpy(ext, pe, _MAX_EXT-1);
					ext[_MAX_EXT -1] = L'\0';
				}
				*pe = L'\0';	// 区切り位置を文字列終端にする。pe = 拡張子名の先頭位置。
			}
			if (fnm) {	// ファイル名を返値として書き込む。
				wcsncpy(fnm, pf, _MAX_FNAME-1);
				fnm[_MAX_FNAME -1] = L'\0';
			}
			*pf = L'\0';	// ファイル名の先頭位置を文字列終端にする。
		}
		// ここまでで文字列 ppp はドライブレター＋ディレクトリ名のみになっている
		if (dir) {
			// ディレクトリ名の最後の文字が \ ではない場合、\ にする。

			// ↓最後の文字を ch に得る。(ディレクトリ文字列が空の場合 ch=L'\\' となる)
			wchar_t	ch;
			for (ch=L'\\', pf=pd; *pf!=L'\0'; ++pf) {
				ch = *pf;
			}
			// 文字列が空でなく、かつ、最後の文字が \ でなかったならば \ を追加。
			if ((ch != L'\\') && (wcslen(ppp) < _MAX_PATH -1)) {
				*pf++ = L'\\';	*pf = L'\0';
			}

			// ディレクトリ名を返値として書き込む。
			wcsncpy(dir, pd, _MAX_DIR -1);
			dir[_MAX_DIR -1] = L'\0';
		}
		*pd = L'\0';		// ディレクトリ名の先頭位置を文字列終端にする。
		if (drv) {	// ドライブレターを返値として書き込む。
			wcsncpy(drv, ppp, _MAX_DRIVE -1);
			drv[_MAX_DRIVE -1] = L'\0';
		}
	}
	return;
}

//
//
//
//
//
// -----------------------------------------------------------------------------
static size_t FileMatchScore(const TCHAR* file1, const TCHAR* file2);

// フルパスからファイル名の.以降を分離する
static
void FileNameSepExt(
	const TCHAR *file,
	TCHAR* pszFile,
	TCHAR* pszExt
	)
{
	const TCHAR* folderPos = file;
	const TCHAR* x = folderPos;
	while (x) {
		x = auto_strchr(folderPos, _T('\\'));
		if (x) {
			++x;
			folderPos = x;
		}
	}
	const TCHAR* p = auto_strchr(folderPos, _T('.'));
	if (p) {
		auto_memcpy(pszFile, file, p - file);
		pszFile[p - file] = _T('\0');
		auto_strcpy(pszExt, p);
	}else {
		auto_strcpy(pszFile, file);
		pszExt[0] = _T('\0');
	}
}

size_t FileMatchScoreSepExt(
	const TCHAR* file1,
	const TCHAR* file2
	)
{
	TCHAR szFile1[_MAX_PATH];
	TCHAR szFile2[_MAX_PATH];
	TCHAR szFileExt1[_MAX_PATH];
	TCHAR szFileExt2[_MAX_PATH];
	FileNameSepExt(file1, szFile1, szFileExt1);
	FileNameSepExt(file2, szFile2, szFileExt2);
	size_t score = FileMatchScore(szFile1, szFile2);
	score += FileMatchScore(szFileExt1, szFileExt2);
	return score;
}

// 2つのファイル名の最長一致部分の長さを返す
size_t FileMatchScore(
	const TCHAR* file1,
	const TCHAR* file2
	)
{
	size_t score = 0;
	size_t len1 = auto_strlen(file1);
	size_t len2 = auto_strlen(file2);
	if (len1 < len2) {
		const TCHAR* tmp = file1;
		file1 = file2;
		file2 = tmp;
		size_t tmpLen = len1;
		len1 = len2;
		len2 = tmpLen;
	}
	for (size_t i=0; i<len1; ) {
		for (size_t k=0; k<len2 && score<(len2 - k);) {
			size_t tmpScore = 0;
			for (size_t m=k; m<len2;) {
				size_t pos1 = i + (m - k);
				size_t chars1 = NativeT::GetSizeOfChar(file1, len1, pos1);
				size_t chars2 = NativeT::GetSizeOfChar(file2, len2, m);
				if (chars1 == chars2) {
					if (chars1 == 1) {
						if (_tcs_tolower(file1[pos1]) == _tcs_tolower(file2[m])) {
							tmpScore += chars1;
						}else {
							break;
						}
					}else {
						if (auto_strnicmp(&file1[pos1], &file2[m], chars1) == 0) {
							tmpScore += chars1;
						}else {
							break;
						}
					}
				}else {
					break;
				}
				m += t_max((size_t)1, chars1);
			}
			if (score < tmpScore) {
				score = tmpScore;
			}
			k += t_max(1, (int)NativeT::GetSizeOfChar(file2, len2, k));
		}
		i += t_max(1, (int)NativeT::GetSizeOfChar(file1, len1, i));
	}
	return score;
}

/*! 指定幅までに文字列を省略 */
void GetStrTrancateWidth(
	TCHAR* dest,
	size_t nSize,
	const TCHAR* path,
	HDC hDC,
	int nPxWidth
	)
{
	// できるだけ左側から表示
	// \\server\dir...
	const size_t nPathLen = auto_strlen(path);
	TextWidthCalc calc(hDC);
	if (calc.GetTextWidth(path) <= nPxWidth) {
		_tcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	std::tstring strTemp;
	std::tstring strTempOld;
	int nPos = 0;
	while (path[nPos] != _T('\0')) {
		strTemp.assign(path, nPos);
		std::tstring strTemp2 = strTemp;
		strTemp2 += _T("...");
		if (nPxWidth < calc.GetTextWidth(strTemp2.c_str())) {
			// 入りきらなかったので1文字前までをコピー
			_tcsncpy_s(dest, t_max(0, (int)nSize - 3), strTempOld.c_str(), _TRUNCATE);
			_tcscat_s(dest, nSize, _T("..."));
			return;
		}
		strTempOld = strTemp;
		nPos += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nPos));
	}
	// 全部表示(ここには来ないはず)
	_tcsncpy_s(dest, nSize, path, _TRUNCATE);
}

/*! パスの省略表示
	in  C:\sub1\sub2\sub3\file.ext
	out C:\...\sub3\file.ext
*/
void GetShortViewPath(
	TCHAR* dest,
	size_t nSize,
	const TCHAR* path,
	HDC hDC,
	int nPxWidth,
	bool bFitMode
	)
{
	int nLeft = 0; // 左側固定表示部分
	int nSkipLevel = 1;
	const size_t nPathLen = auto_strlen(path);
	TextWidthCalc calc(hDC);
	if (calc.GetTextWidth(path) <= nPxWidth) {
		// 全部表示可能
		_tcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	if (path[0] == _T('\\') && path[1] == _T('\\')) {
		if (path[2] == _T('?') && path[4] == _T('\\')) {
			// [\\?\A:\]
			nLeft = 4;
		}else {
			nSkipLevel = 2; // [\\server\dir\] の2階層飛ばす
			nLeft = 2;
		}
	}else {
		// http://server/ とか ftp://server/ とかを保持
		int nTop = 0;
		while (path[nTop] != _T('\0') && path[nTop] != _T('/')) {
			nTop += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nTop));
		}
		if (0 < nTop && path[nTop - 1] == ':') {
			// 「ほにゃらら:/」だった /が続いてる間飛ばす
			while (path[nTop] == _T('/')) {
				nTop += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nTop));
			}
			nLeft = nTop;
		}
	}
	for (int i=0; i<nSkipLevel; ++i) {
		while (path[nLeft] != _T('\0') && path[nLeft] != _T('\\') && path[nLeft] != _T('/')) {
			nLeft += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nLeft));
		}
		if (path[nLeft] != _T('\0')) {
			if (i + 1 < nSkipLevel) {
				++nLeft;
			}
		}else {
			if (bFitMode) {
				GetStrTrancateWidth(dest, nSize, path, hDC, nPxWidth);
				return;
			}
			// ここで終端なら全部表示
			_tcsncpy_s(dest, nSize, path, _TRUNCATE);
			return;
		}
	}
	int nRight = nLeft; // 右側の表示開始位置(nRightは\を指している)
	while (path[nRight] != _T('\0')) {
		int nNext = nRight;
		++nNext;
		while (path[nNext] != _T('\0') && path[nNext] != _T('\\') && path[nNext] != _T('/')) {
			nNext += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nNext));
		}
		if (path[nNext] != _T('\0')) {
			// サブフォルダ省略
			// C:\...\dir\file.ext
			std::tstring strTemp(path, nLeft + 1);
			if (nLeft + 1 < nRight) {
				strTemp += _T("...");
			}
			strTemp += &path[nRight];
			if (calc.GetTextWidth(strTemp.c_str()) <= nPxWidth) {
				_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
				return;
			}
			// C:\...\dir\   フォルダパスだった。最後のフォルダを表示
			if (path[nNext+1] == _T('\0')) {
				if (bFitMode) {
					GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
					return;
				}
				_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
				return;
			}
			nRight = nNext;
		}else {
			break;
		}
	}
	// nRightより右に\が見つからなかった=ファイル名だったのでファイル名表示
	// C:\...\file.ext
	int nLeftLen = nLeft;
	if (nLeftLen && nLeftLen != nRight) {
		++nLeftLen;
	}
	std::tstring strTemp(path, nLeftLen);
	if (nLeft != nRight) {
		strTemp += _T("...");
	}
	strTemp += &path[nRight];
	if (bFitMode) {
		if (calc.GetTextWidth(strTemp.c_str()) <= nPxWidth) {
			_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
			return;
		}
		// ファイル名(か左側固定部)が長すぎてはいらない
		int nExtPos = -1;
		{
			// 拡張子の.を探す
			int nExt = nRight;
			while (path[nExt] != _T('\0')) {
				if (path[nExt] == _T('.')) {
					nExtPos = nExt;
				}
				nExt += t_max(1, (int)NativeT::GetSizeOfChar(path, nPathLen, nExt));
			}
		}
		if (nExtPos != -1) {
			std::tstring strLeftFile(path, nLeftLen); // [C:\]  
			if (nLeft != nRight) {
				strLeftFile += _T("..."); // C:\...
			}
			int nExtWidth = calc.GetTextWidth(&path[nExtPos]);
			int nLeftWidth = calc.GetTextWidth(strLeftFile.c_str());
			int nFileNameWidth = nPxWidth - nLeftWidth - nExtWidth;
			if (0 < nFileNameWidth) {
				// 拡張子は省略しない(ファイルタイトルを省略)
				std::tstring strFile(&path[nRight], nExtPos - nRight); // \longfilename
				strLeftFile += strFile; // C:\...\longfilename
				size_t nExtLen = nPathLen - nExtPos;
				GetStrTrancateWidth(dest, t_max(0, (int)nSize - (int)nExtLen), strLeftFile.c_str(), hDC, nPxWidth - nExtWidth);
				_tcscat_s(dest, nSize, &path[nExtPos+1]); // 拡張子連結 C:\...\longf...ext
			}else {
				// ファイル名が置けないくらい拡張子か左側が長い。パスの左側を優先して残す
				GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
			}
		}else {
			// 拡張子はなかった。左側から残す
			GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
		}
		return;
	}
	_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
}

static inline
size_t getFileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	return length;
}

bool ReadFile(const wchar_t* path, std::vector<char>& buff)
{
	FILE* f = _wfopen(path, L"rb");
	if (!f) {
		return false;
	}
	size_t sz = getFileSize(f);
	buff.resize(sz);
	fread(&buff[0], 1, sz, f);
	fclose(f);
	// TODO: to check read failure
	return true;
}

bool ReadFileAndUnicodify(const wchar_t* path, std::vector<wchar_t>& buffW)
{
	std::vector<char> buffM;
	if (!ReadFile(path, buffM) || buffM.size() < 3) {
		return false;
	}

	const char* pBuffM = &buffM[0];
	size_t buffSize = buffM.size();
	static const uint8_t utf8_bom[] = { 0xEF, 0xBB, 0xBF };
	bool isUtf8 = (memcmp(utf8_bom, pBuffM, 3) == 0);
	if (isUtf8) {
		pBuffM += 3;
		buffSize -= 3;
	}
	static const UINT cp_sjis = 932;
	UINT codePage = isUtf8 ? CP_UTF8 : cp_sjis;

	buffW.resize(buffSize);
	wchar_t* pBuffW = &buffW[0];
	int ret = MultiByteToWideChar(codePage, 0, pBuffM, (int)buffSize, pBuffW, (int)buffSize);
	if (ret <= 0) {
		return false;
	}
	buffW.resize(ret);
	return true;
}

