#include "StdAfx.h"
#include "DocLocker.h"
#include "DocFile.h"
#include "window/EditWnd.h"



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DocLocker::DocLocker()
	:
	bIsDocWritable(true)
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLocker::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 書き込めるか検査
	CheckWritable(!loadInfo.bViewMode && !loadInfo.bWritableNoMsg);
	if (!bIsDocWritable) {
		return;
	}

	// ファイルの排他ロック
	pDoc->docFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLocker::OnBeforeSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// ファイルの排他ロック解除
	pDoc->docFileOperation.DoFileUnlock();
}

void DocLocker::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 書き込めるか検査
	bIsDocWritable = true;

	// ファイルの排他ロック
	pDoc->docFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         チェック                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 書き込めるか検査
void DocLocker::CheckWritable(bool bMsg)
{
	EditDoc* pDoc = GetListeningDoc();

	// ファイルが存在しない場合 (「開く」で新しくファイルを作成した扱い) は、以下の処理は行わない
	if (!fexist(pDoc->docFile.GetFilePath())) {
		bIsDocWritable = true;
		return;
	}

	// 読み取り専用ファイルの場合は、以下の処理は行わない
	if (!pDoc->docFile.HasWritablePermission()) {
		bIsDocWritable = false;
		return;
	}

	// 書き込めるか検査
	DocFile& docFile = pDoc->docFile;
	bIsDocWritable = docFile.IsFileWritable();
	if (!bIsDocWritable && bMsg) {
		// 排他されている場合だけメッセージを出す
		// その他の原因（ファイルシステムのセキュリティ設定など）では読み取り専用と同様にメッセージを出さない
		if (::GetLastError() == ERROR_SHARING_VIOLATION) {
			TopWarningMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGEDITDOC21),	// "%ts\nは現在他のプロセスによって書込みが禁止されています。"
				docFile.GetFilePathClass().IsValidPath() ? docFile.GetFilePath() : LS(STR_NO_TITLE1)	// "(無題)"
			);
		}
	}
}

