#include "StdAfx.h"
#include "AutoReloadAgent.h"
// #include "doc/EditDoc.h"	//  in under EditWnd.h
#include "window/EditWnd.h"
#include "dlg/DlgFileUpdateQuery.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

AutoReloadAgent::AutoReloadAgent()
	:
	watchUpdateType(WatchUpdateType::Query),
	nPauseCount(0),
	nDelayCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnBeforeSave(const SaveInfo& saveInfo)
{
	// 保存が完了するまではファイル更新の通知を抑制する
	PauseWatching();
}

void AutoReloadAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	// ファイル更新の通知を元に戻す
	ResumeWatching();

	if (!saveInfo.bOverwriteMode) {
		watchUpdateType = WatchUpdateType::Query;	// 「名前を付けて保存」で対象ファイルが変更されたので更新監視方法をデフォルトに戻す
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	//pDoc->docFile.sFileInfo.cFileTime.SetFILETIME(ftime); //#####既に設定済みのはず
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool AutoReloadAgent::_ToDoChecking() const
{
	const CommonSetting_File& setting = GetDllShareData().common.file;
	HWND hwndActive = ::GetActiveWindow();
	if (0
		|| IsPausing()
		|| !setting.bCheckFileTimeStamp	// 更新の監視設定
		|| watchUpdateType == WatchUpdateType::None
		|| setting.nFileShareMode != FileShareMode::NonExclusive	 // ファイルの排他制御モード
		|| !hwndActive		// アクティブ？
		|| hwndActive != EditWnd::getInstance().GetHwnd()
		|| !GetListeningDoc()->docFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->docFile.IsFileTimeZero()	// 現在編集中のファイルのタイムスタンプ
		|| GetListeningDoc()->pEditWnd->pPrintPreview	// 印刷Preview中
	) {
		return false;
	}
	return true;
}

bool AutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	// ファイルスタンプをチェックする
	FileTime ftime;
	if (GetLastWriteTimestamp(GetListeningDoc()->docFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->docFile.GetFileTime().GetFILETIME(),
				&ftime.GetFILETIME()
			) != 0
		) {	// タイムスタンプが古く変更されている場合も検出対象とする
			*pNewFileTime = ftime.GetFILETIME();
			return true;
		}
	}
	return false;
}

// ファイルのタイムスタンプのチェック処理
void AutoReloadAgent::CheckFileTimeStamp()
{
	// 未編集で再ロード時の遅延
	if (watchUpdateType == WatchUpdateType::AutoLoad) {
		if (++nDelayCount < GetDllShareData().common.file.nAutoloadDelay) {
			return;
		}
		nDelayCount = 0;
	}

	if (!_ToDoChecking()) {
		return;
	}

	EditDoc* pDoc = GetListeningDoc();

	// タイムスタンプ監視
	FILETIME ftime;
	if (!_IsFileUpdatedByOther(&ftime)) {
		return;
	}
	pDoc->docFile.SetFileTime(ftime); // タイムスタンプ更新

	switch (watchUpdateType) {
	case WatchUpdateType::Notify:
		{
			// ファイル更新のお知らせ -> ステータスバー
			TCHAR szText[40];
			const FileTime& time = pDoc->docFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), time->wHour, time->wMinute, time->wSecond);
			pDoc->pEditWnd->SendStatusMessage(szText);
		}
		break;
	case WatchUpdateType::AutoLoad:		// 以後未編集で再ロード
		if (!pDoc->docEditor.IsModified()) {
			PauseWatching(); // 更新監視の抑制

			// 同一ファイルの再オープン
			pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
			watchUpdateType = WatchUpdateType::AutoLoad;

			ResumeWatching(); // 監視再開
			break;
		}
		// through
	default:
		{
			PauseWatching(); // 更新監視の抑制

			DlgFileUpdateQuery dlg(pDoc->docFile.GetFilePath(), pDoc->docEditor.IsModified());
			INT_PTR result = dlg.DoModal(
				G_AppInstance(),
				EditWnd::getInstance().GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// 再読込
				// 同一ファイルの再オープン
				pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
				watchUpdateType = WatchUpdateType::Query;
				break;
			case 2:	// 以後通知メッセージのみ
				watchUpdateType = WatchUpdateType::Notify;
				break;
			case 3:	// 以後更新を監視しない
				watchUpdateType = WatchUpdateType::None;
				break;
			case 4:	// 以後未編集で再ロード
				// 同一ファイルの再オープン
				pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
				watchUpdateType = WatchUpdateType::AutoLoad;
				nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				watchUpdateType = WatchUpdateType::Query;
				break;
			}

			ResumeWatching(); // 監視再開
		}
		break;
	}
}

