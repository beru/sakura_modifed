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
#include "CAutoReloadAgent.h"
// #include "doc/CEditDoc.h"	//  in under CEditWnd.h
#include "window/CEditWnd.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

AutoReloadAgent::AutoReloadAgent()
	:
	m_eWatchUpdate(WU_QUERY),
	m_nPauseCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnBeforeSave(const SaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	保存が完了するまではファイル更新の通知を抑制する
	PauseWatching();
}

void AutoReloadAgent::OnAfterSave(const SaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	ファイル更新の通知を元に戻す
	ResumeWatching();

	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	if (!sSaveInfo.bOverwriteMode) {
		m_eWatchUpdate = WU_QUERY;	// 「名前を付けて保存」で対象ファイルが変更されたので更新監視方法をデフォルトに戻す
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnAfterLoad(const LoadInfo& sLoadInfo)
{
	//pcDoc->m_cDocFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //#####既に設定済みのはず
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool AutoReloadAgent::_ToDoChecking() const
{
	const CommonSetting_File& setting = GetDllShareData().m_common.m_sFile;
	HWND hwndActive = ::GetActiveWindow();
	if (0
		|| IsPausing()
		|| !setting.m_bCheckFileTimeStamp	// 更新の監視設定
		|| m_eWatchUpdate == WU_NONE
		|| setting.m_nFileShareMode != SHAREMODE_NOT_EXCLUSIVE	 // ファイルの排他制御モード
		|| !hwndActive		// アクティブ？
		|| hwndActive != EditWnd::getInstance()->GetHwnd()
		|| !GetListeningDoc()->m_cDocFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->m_cDocFile.IsFileTimeZero()	// 現在編集中のファイルのタイムスタンプ
		|| GetListeningDoc()->m_pEditWnd->m_pPrintPreview	// 印刷プレビュー中	2013/5/8 Uchi
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
	if (GetLastWriteTimestamp(GetListeningDoc()->m_cDocFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->m_cDocFile.GetFileTime().GetFILETIME(),
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
	if (m_eWatchUpdate == WU_AUTOLOAD) {
		if (++m_nDelayCount < GetDllShareData().m_common.m_sFile.m_nAutoloadDelay) {
			return;
		}
		m_nDelayCount = 0;
	}

	if (!_ToDoChecking()) {
		return;
	}

	EditDoc* pcDoc = GetListeningDoc();

	// タイムスタンプ監視
	FILETIME ftime;
	if (!_IsFileUpdatedByOther(&ftime)) {
		return;
	}
	pcDoc->m_cDocFile.SetFileTime(ftime); // タイムスタンプ更新

	//	From Here Dec. 4, 2002 genta
	switch (m_eWatchUpdate) {
	case WU_NOTIFY:
		{
			// ファイル更新のお知らせ -> ステータスバー
			TCHAR szText[40];
			const FileTime& ctime = pcDoc->m_cDocFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), ctime->wHour, ctime->wMinute, ctime->wSecond);
			pcDoc->m_pEditWnd->SendStatusMessage(szText);
		}
		break;
	case WU_AUTOLOAD:		// 以後未編集で再ロード
		if (!pcDoc->m_cDocEditor.IsModified()) {
			PauseWatching(); // 更新監視の抑制

			// 同一ファイルの再オープン
			pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
			m_eWatchUpdate = WU_AUTOLOAD;

			ResumeWatching(); // 監視再開
			break;
		}
		// through
	default:
		{
			PauseWatching(); // 更新監視の抑制

			DlgFileUpdateQuery dlg(pcDoc->m_cDocFile.GetFilePath(), pcDoc->m_cDocEditor.IsModified());
			int result = dlg.DoModal(
				G_AppInstance(),
				EditWnd::getInstance()->GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// 再読込
				// 同一ファイルの再オープン
				pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
				m_eWatchUpdate = WU_QUERY;
				break;
			case 2:	// 以後通知メッセージのみ
				m_eWatchUpdate = WU_NOTIFY;
				break;
			case 3:	// 以後更新を監視しない
				m_eWatchUpdate = WU_NONE;
				break;
			case 4:	// 以後未編集で再ロード
				// 同一ファイルの再オープン
				pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
				m_eWatchUpdate = WU_AUTOLOAD;
				m_nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				m_eWatchUpdate = WU_QUERY;
				break;
			}

			ResumeWatching(); // 監視再開
		}
		break;
	}
	//	To Here Dec. 4, 2002 genta
}

