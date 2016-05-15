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

#include "basis/MyString.h" //FilePath
#include "util/fileUtil.h"

// ファイルの排他制御モード  2007.10.11 kobake 作成
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

