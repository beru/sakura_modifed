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
	m_watchUpdateType(WatchUpdateType::Query),
	m_nPauseCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnBeforeSave(const SaveInfo& saveInfo)
{
	//	Sep. 7, 2003 genta
	//	保存が完了するまではファイル更新の通知を抑制する
	PauseWatching();
}

void AutoReloadAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	//	Sep. 7, 2003 genta
	//	ファイル更新の通知を元に戻す
	ResumeWatching();

	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	if (!saveInfo.bOverwriteMode) {
		m_watchUpdateType = WatchUpdateType::Query;	// 「名前を付けて保存」で対象ファイルが変更されたので更新監視方法をデフォルトに戻す
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	//pDoc->m_docFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //#####既に設定済みのはず
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
		|| m_watchUpdateType == WatchUpdateType::None
		|| setting.nFileShareMode != FileShareMode::NonExclusive	 // ファイルの排他制御モード
		|| !hwndActive		// アクティブ？
		|| hwndActive != EditWnd::getInstance()->GetHwnd()
		|| !GetListeningDoc()->m_docFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->m_docFile.IsFileTimeZero()	// 現在編集中のファイルのタイムスタンプ
		|| GetListeningDoc()->m_pEditWnd->m_pPrintPreview	// 印刷Preview中	2013/5/8 Uchi
	) {
		return false;
	}
	return true;
}

bool AutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	// ファイルスタンプをチェックする
	// 2005.10.20 ryoji FindFirstFileを使うように変更（ファイルがロックされていてもタイムスタンプ取得可能）
	FileTime ftime;
	if (GetLastWriteTimestamp(GetListeningDoc()->m_docFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->m_docFile.GetFileTime().GetFILETIME(),
				&ftime.GetFILETIME()
			) != 0
		) {	//	Aug. 13, 2003 wmlhq タイムスタンプが古く変更されている場合も検出対象とする
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
	if (m_watchUpdateType == WatchUpdateType::AutoLoad) {
		if (++m_nDelayCount < GetDllShareData().common.file.nAutoloadDelay) {
			return;
		}
		m_nDelayCount = 0;
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
	pDoc->m_docFile.SetFileTime(ftime); // タイムスタンプ更新

	//	From Here Dec. 4, 2002 genta
	switch (m_watchUpdateType) {
	case WatchUpdateType::Notify:
		{
			// ファイル更新のお知らせ -> ステータスバー
			TCHAR szText[40];
			const FileTime& time = pDoc->m_docFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), time->wHour, time->wMinute, time->wSecond);
			pDoc->m_pEditWnd->SendStatusMessage(szText);
		}
		break;
	case WatchUpdateType::AutoLoad:		// 以後未編集で再ロード
		if (!pDoc->m_docEditor.IsModified()) {
			PauseWatching(); // 更新監視の抑制

			// 同一ファイルの再オープン
			pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
			m_watchUpdateType = WatchUpdateType::AutoLoad;

			ResumeWatching(); // 監視再開
			break;
		}
		// through
	default:
		{
			PauseWatching(); // 更新監視の抑制

			DlgFileUpdateQuery dlg(pDoc->m_docFile.GetFilePath(), pDoc->m_docEditor.IsModified());
			int result = dlg.DoModal(
				G_AppInstance(),
				EditWnd::getInstance()->GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// 再読込
				// 同一ファイルの再オープン
				pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
				m_watchUpdateType = WatchUpdateType::Query;
				break;
			case 2:	// 以後通知メッセージのみ
				m_watchUpdateType = WatchUpdateType::Notify;
				break;
			case 3:	// 以後更新を監視しない
				m_watchUpdateType = WatchUpdateType::None;
				break;
			case 4:	// 以後未編集で再ロード
				// 同一ファイルの再オープン
				pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
				m_watchUpdateType = WatchUpdateType::AutoLoad;
				m_nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				m_watchUpdateType = WatchUpdateType::Query;
				break;
			}

			ResumeWatching(); // 監視再開
		}
		break;
	}
	//	To Here Dec. 4, 2002 genta
}

