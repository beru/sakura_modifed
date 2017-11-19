#pragma once

bool fexist(LPCTSTR pszPath); // ファイルまたはディレクトリが存在すればtrue

bool IsFilePath(const wchar_t*, size_t*, int*, bool = true);
bool IsFileExists(const TCHAR* path, bool bFileOnly = false);
bool IsDirectory(LPCTSTR pszPath);

// ディレクトリの深さを調べる
int CalcDirectoryDepth(const TCHAR* path);

bool IsLocalDrive(const TCHAR* pszDrive);

// ※サクラ依存
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode);
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, bool bOrExedir = true);

// パス文字列処理
void CutLastYenFromDirectoryPath(TCHAR*);						// フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない
void AddLastYenFromDirectoryPath( CHAR*);						// フォルダの最後が半角かつ'\\'でない場合は、付加する
void AddLastYenFromDirectoryPath(wchar_t*);						// フォルダの最後が半角かつ'\\'でない場合は、付加する
void SplitPath_FolderAndFile(const TCHAR*, TCHAR*, TCHAR*);	// ファイルのフルパスを、フォルダとファイル名に分割
void Concat_FolderAndFile(const TCHAR*, const TCHAR*, TCHAR*);// フォルダ、ファイル名から、結合したパスを作成
BOOL GetLongFileName(const TCHAR*, TCHAR*);					// ロングファイル名を取得する
BOOL CheckEXT(const TCHAR*, const TCHAR*);					// 拡張子を調べる
const TCHAR* GetFileTitlePointer(const TCHAR* tszPath);			// ファイルフルパス内のファイル名を指すポインタを取得
bool _IS_REL_PATH(const TCHAR* path);							// 相対パスか判定する

// ※サクラ依存
void GetExedir(LPTSTR pDir, LPCTSTR szFile = NULL);
void GetInidir(LPTSTR pDir, LPCTSTR szFile = NULL);
void GetInidirOrExedir(LPTSTR pDir, LPCTSTR szFile = NULL, bool bRetExedirIfFileEmpty = false);

LPCTSTR GetRelPath(LPCTSTR pszPath);

// ファイル時刻
class FileTime {
public:
	FileTime() { ClearFILETIME(); }
	FileTime(const FILETIME& ftime) { SetFILETIME(ftime); }
	// 設定
	void ClearFILETIME() { ftime.dwLowDateTime = ftime.dwHighDateTime = 0; bModified = true; }
	void SetFILETIME(const FILETIME& ftime) { this->ftime = ftime; bModified = true; }
	// 取得
	const FILETIME& GetFILETIME() const { return ftime; }
	const SYSTEMTIME& GetSYSTEMTIME() const {
		// キャッシュ更新 -> systime, bModified
		if (bModified) {
			bModified = false;
			FILETIME ftimeLocal;
			if (!::FileTimeToLocalFileTime(&ftime, &ftimeLocal) || !::FileTimeToSystemTime(&ftimeLocal, &systime)) {
				memset(&systime, 0, sizeof(systime)); // 失敗時ゼロクリア
			}
		}
		return systime;
	}
	const SYSTEMTIME* operator->() const { return &GetSYSTEMTIME(); }
	// 判定
	bool IsZero() const {
		return ftime.dwLowDateTime == 0 && ftime.dwHighDateTime == 0;
	}
protected:
private:
	FILETIME ftime;
	// キャッシュ
	mutable SYSTEMTIME	systime;
	mutable bool		bModified;
};

bool GetLastWriteTimestamp(const TCHAR* filename, FileTime* pFileTime);

// 文字列分割
void my_splitpath (const char* comln , char* drv, char* dir, char* fnm, char* ext);
void my_splitpath_w (const wchar_t* comln, wchar_t* drv, wchar_t* dir, wchar_t* fnm, wchar_t* ext);
void my_splitpath_t (const TCHAR* comln, TCHAR* drv, TCHAR* dir, TCHAR* fnm, TCHAR* ext);
#define my_splitpath_t my_splitpath_w

size_t FileMatchScoreSepExt(const TCHAR* file1, const TCHAR* file2);

void GetStrTrancateWidth(TCHAR* dest, size_t nSize, const TCHAR* path, HDC hDC, int nPxWidth);
void GetShortViewPath(TCHAR* dest, size_t nSize, const TCHAR* path, HDC hDC, int nPxWidth, bool bFitMode);

bool ReadFile(const wchar_t* path, std::vector<char>& buff);
bool ReadFileAndUnicodify(const wchar_t* path, std::vector<wchar_t>& buff);

