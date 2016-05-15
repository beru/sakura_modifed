#include "StdAfx.h"
#include "io/File.h"
#include "window/EditWnd.h" // 変更予定
#include <io.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

File::File(LPCTSTR pszPath)
	:
	hLockedFile(INVALID_HANDLE_VALUE),
	nFileShareModeOld(FileShareMode::NonExclusive)
{
	if (pszPath) {
		SetFilePath(pszPath);
	}
}

File::~File()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool File::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool File::HasWritablePermission() const
{
	return _taccess(GetFilePath(), 2) != -1;
}

bool File::IsFileWritable() const
{
	// 書き込めるか検査
	// Note. 他のプロセスが明示的に書き込み禁止しているかどうか
	//       ⇒ GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE でチェックする
	//          実際のファイル保存もこれと等価な _tfopen の _T("wb") を使用している
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			// ファイル名
		GENERIC_WRITE,					// 書きモード
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// 読み書き共有
		NULL,							// 既定のセキュリティ記述子
		OPEN_EXISTING,					// ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			// 特に属性は指定しない
		NULL							// テンプレート無し
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool File::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if (hTest == INVALID_HANDLE_VALUE) {
		// 読み込みアクセス権がない
		return false;
	}
	CloseHandle(hTest);
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ファイルの排他ロック解除
void File::FileUnlock()
{
	// クローズ
	if (hLockedFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(hLockedFile);
		hLockedFile = INVALID_HANDLE_VALUE;
	}
}

// ファイルの排他ロック
bool File::FileLock(FileShareMode eShareMode, bool bMsg)
{
	// ロック解除
	FileUnlock();

	// ファイルの存在チェック
	if (!this->IsFileExist()) {
		return false;
	}

	// モード設定
	if (eShareMode == FileShareMode::NonExclusive) {
		return true;
	}
	// フラグ
	DWORD dwShareMode = 0;
	switch (eShareMode) {
	case FileShareMode::NonExclusive:	return true;										break; // 排他制御無し
	case FileShareMode::DenyReadWrite:	dwShareMode = 0;									break; // 読み書き禁止→共有無し
	case FileShareMode::DenyWrite:		dwShareMode = FILE_SHARE_READ;						break; // 書き込み禁止→読み込みのみ認める
	default:							dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; // 禁止事項なし→読み書き共に認める
	}

	// オープン
	hLockedFile = CreateFile(
		this->GetFilePath(),			// ファイル名
		GENERIC_READ,					// 読み書きタイプ
		dwShareMode,					// 共有モード
		NULL,							// 既定のセキュリティ記述子
		OPEN_EXISTING,					// ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			// 特に属性は指定しない
		NULL							// テンプレート無し
	);

	// 結果
	if (hLockedFile == INVALID_HANDLE_VALUE && bMsg) {
		const TCHAR* pszMode;
		switch (eShareMode) {
		case FileShareMode::DenyReadWrite:	pszMode = LS(STR_EXCLU_DENY_READWRITE); break;
		case FileShareMode::DenyWrite:		pszMode = LS(STR_EXCLU_DENY_WRITE); break;
		default:							pszMode = LS(STR_EXCLU_UNDEFINED); break;
		}
		TopWarningMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_FILE_LOCK_ERR),
			GetFilePathClass().IsValidPath() ? GetFilePath() : LS(STR_NO_TITLE1),
			pszMode
		);
		return false;
	}

	return true;
}


