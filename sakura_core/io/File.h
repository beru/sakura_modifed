#pragma once

#include "basis/MyString.h" //FilePath
#include "util/fileUtil.h"

// ファイルの排他制御モード
enum class FileShareMode {
	NonExclusive,	// 排他制御しない
	DenyWrite,		// 他プロセスからの上書きを禁止
	DenyReadWrite,	// 他プロセスからの読み書きを禁止
};

class File {
public:
	// コンストラクタ・デストラクタ
	File(LPCTSTR pszPath = NULL);
	virtual ~File();
	// パス
	const FilePath& GetFilePathClass() const { return szFilePath; }
	LPCTSTR GetFilePath() const { return szFilePath; }
	// 設定
	void SetFilePath(LPCTSTR pszPath) { szFilePath.Assign(pszPath); }
	// 各種判定
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	// ロック
	bool FileLock(FileShareMode eShareMode, bool bMsg);	// ファイルの排他ロック
	void FileUnlock();						// ファイルの排他ロック解除
	bool IsFileLocking() const { return hLockedFile != INVALID_HANDLE_VALUE; }
	FileShareMode GetShareMode() const { return nFileShareModeOld; }
	void SetShareMode(FileShareMode eShareMode) { nFileShareModeOld = eShareMode; }
private:
	FilePath	szFilePath;					// ファイルパス
	HANDLE		hLockedFile;				// ロックしているファイルのハンドル
	FileShareMode	nFileShareModeOld;		// ファイルの排他制御モード
};


// 一時ファイル
class TmpFile {
public:
	TmpFile() { fp = tmpfile(); }
	~TmpFile() { fclose(fp); }
	FILE* GetFilePointer() const { return fp; }
private:
	FILE* fp;
};

