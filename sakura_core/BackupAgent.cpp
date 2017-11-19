#include "StdAfx.h"
#include <time.h>
#include <io.h>	// access
#include "BackupAgent.h"
#include "window/EditWnd.h"
#include "util/format.h" // GetDateTimeFormat

/*! セーブ前おまけ処理
	@param pSaveInfo [in] 保存ファイル情報

	@retval CallbackResultType::Continue 続ける
	@retval CallbackResultType::Interrupt 中断
*/
CallbackResultType BackupAgent::OnPreBeforeSave(SaveInfo* pSaveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 新しくファイルを作る場合は何もしない
	if (!fexist(pSaveInfo->filePath)) {
		return CallbackResultType::Continue;
	}

	// 共通設定：保存時にバックアップを作成する
	if (GetDllShareData().common.backup.bBackUp) {
		// ファイル保存前にバックアップ処理
		int nBackupResult = 0;
		{
			pDoc->docFileOperation.DoFileUnlock();	// バックアップ作成前にロックを解除する #####スマートじゃないよ！
			nBackupResult = MakeBackUp(pSaveInfo->filePath);
			pDoc->docFileOperation.DoFileLock();	// バックアップ作成後にロックを戻す #####スマートじゃないよ！
		}
		switch (nBackupResult) {
		case 2:	//	中断指示
			return CallbackResultType::Interrupt;
		case 3: //	ファイルエラー
			if (::MYMESSAGEBOX(
					EditWnd::getInstance().GetHwnd(),
					MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
					LS(STR_BACKUP_ERR_TITLE),
					LS(STR_BACKUP_ERR_MSG)
				) != IDYES
			) {
				return CallbackResultType::Interrupt;
			}
			break;
		}
	}
	return CallbackResultType::Continue;
}


/*! バックアップの作成
	@param target_file [in] バックアップ元パス名

	@retval 0 バックアップ作成失敗．
	@retval 1 バックアップ作成成功．
	@retval 2 バックアップ作成失敗．保存中断指示．
	@retval 3 ファイル操作エラーによるバックアップ作成失敗．

	@todo Advanced modeでの世代管理
*/
int BackupAgent::MakeBackUp(
	const TCHAR* target_file
)
{
	// バックアップソースの存在チェック
	//	書き込みアクセス権がない場合も
	//	ファイルがない場合と同様に何もしない
	if ((_taccess(target_file, 2)) == -1) {
		return 0;
	}

	const CommonSetting_Backup& bup_setting = GetDllShareData().common.backup;

	TCHAR szPath[_MAX_PATH]; // バックアップ先パス名
	if (!FormatBackUpPath(szPath, _countof(szPath), target_file)) {
		int nMsgResult = ::TopConfirmMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_BACKUP_ERR_PATH_CRETE)
		);
		if (nMsgResult == IDYES) {
			return 0;	//	保存継続
		}
		return 2;	// 保存中断
	}

	// ネットワーク・リムーバブルドライブの場合はごみ箱に放り込まない
	bool dustflag = false;
	if (bup_setting.bBackUpDustBox) {
		dustflag = !IsLocalDrive(szPath);
	}

	if (bup_setting.bBackUpDialog) {	// バックアップの作成前に確認
		ConfirmBeep();
		int nRet;
		if (bup_setting.bBackUpDustBox && !dustflag) {	// 共通設定：バックアップファイルをごみ箱に放り込む
			nRet = ::MYMESSAGEBOX(
				EditWnd::getInstance().GetHwnd(),
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_BACKUP_CONFORM_TITLE1),
				LS(STR_BACKUP_CONFORM_MSG1),
				target_file,
				szPath
			);
		}else {
			nRet = ::MYMESSAGEBOX(
				EditWnd::getInstance().GetHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_BACKUP_CONFORM_TITLE2),
				LS(STR_BACKUP_CONFORM_MSG2),
				target_file,
				szPath
			);
		}
		if (nRet == IDNO) {
			return 0;	//	保存継続
		}else if (nRet == IDCANCEL) {
			return 2;	// 保存中断
		}
	}

	if (bup_setting.GetBackupType() == 3 ||
		bup_setting.GetBackupType() == 6
	) {
		//	既に存在するBackupをずらす処理
		int i;

		//	ファイル検索用
		WIN32_FIND_DATA	fData;

		TCHAR* pBase = szPath + _tcslen(szPath) - 2;	//	2: 拡張子の最後の2桁の意味

		//------------------------------------------------------------------
		//	1. 該当ディレクトリ中のbackupファイルを1つずつ探す
		for (i=0; i<=99; ++i) {	//	最大値に関わらず，99（2桁の最大値）まで探す
			//	ファイル名をセット
			auto_sprintf(pBase, _T("%02d"), i);

			HANDLE hFind = ::FindFirstFile(szPath, &fData);
			if (hFind == INVALID_HANDLE_VALUE) {
				//	検索に失敗した == ファイルは存在しない
				break;
			}
			::FindClose(hFind);
			//	見つかったファイルの属性をチェック
			//	は面倒くさいからしない．
			//	同じ名前のディレクトリがあったらどうなるのだろう...
		}
		--i;

		//------------------------------------------------------------------
		//	2. 最大値から制限数-1番までを削除
		int boundary = bup_setting.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	最小値は0

		for (; i>=boundary; --i) {
			//	ファイル名をセット
			auto_sprintf(pBase, _T("%02d"), i);
			if (::DeleteFile(szPath) == 0) {
				::MessageBox(EditWnd::getInstance().GetHwnd(), szPath, LS(STR_BACKUP_ERR_DELETE), MB_OK);
				//	失敗しても保存は継続
				return 0;
				//	失敗した場合
				//	後で考える
			}
		}

		//	この位置でiは存在するバックアップファイルの最大番号を表している．

		//	3. そこから0番まではコピーしながら移動
		TCHAR szNewPath[MAX_PATH];

		_tcscpy(szNewPath, szPath);
		TCHAR* pNewNrBase = szNewPath + _tcslen(szNewPath) - 2;

		for (; i>=0; --i) {
			//	ファイル名をセット
			auto_sprintf(pBase, _T("%02d"), i);
			auto_sprintf(pNewNrBase, _T("%02d"), i + 1);

			//	ファイルの移動
			if (::MoveFile(szPath, szNewPath) == 0) {
				//	失敗した場合
				//	後で考える
				::MessageBox(EditWnd::getInstance().GetHwnd(), szPath, LS(STR_BACKUP_ERR_MOVE), MB_OK);
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
			}
		}
	}

	// バックアップの作成
	//	現在のファイルではなくターゲットファイルをバックアップするように
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath(szPath, szDrive, szDir, szFname, szExt);
	TCHAR	szPath2[MAX_PATH];
	auto_sprintf(szPath2, _T("%ts%ts"), szDrive, szDir);

	WIN32_FIND_DATA	fData;
	HANDLE hFind = ::FindFirstFile(szPath2, &fData);
	if (hFind == INVALID_HANDLE_VALUE) {
		//	検索に失敗した == ファイルは存在しない
		::CreateDirectory(szPath2, NULL);
	}
	::FindClose(hFind);

	if (::CopyFile(target_file, szPath, FALSE)) {
		// 正常終了
		if (bup_setting.bBackUpDustBox && !dustflag) {	// ネットワーク・リムーバブルドライブでない
			TCHAR	szDustPath[_MAX_PATH + 1];
			_tcscpy(szDustPath, szPath);
			szDustPath[_tcslen(szDustPath) + 1] = _T('\0');
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = EditWnd::getInstance().GetHwnd();
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	// ダイアログなし
			fos.fAnyOperationsAborted = true; // false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"バックアップファイルをごみ箱に移動しています...";
			if (::SHFileOperation(&fos) == 0) {
				// 正常終了
			}else {
				// エラー終了
			}
		}
	}else {
		// エラー終了
		return 3;
	}
	return 1;
}


/*! バックアップパスの作成

	@param szNewPath [out] バックアップ先パス名
	@param newPathCount [in] szNewPathのサイズ
	@param target_file [in] バックアップ元パス名

	@retval true  成功
	@retval false バッファ不足

	@todo Advanced modeでの世代管理
*/
bool BackupAgent::FormatBackUpPath(
	TCHAR*			szNewPath,
	size_t 			newPathCount,
	const TCHAR*	target_file
	)
{
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	TCHAR*	psNext;

	const CommonSetting_Backup& bup_setting = GetDllShareData().common.backup;

	// パスの分解
	_tsplitpath(target_file, szDrive, szDir, szFname, szExt);

	if (bup_setting.bBackUpFolder
	  && (!bup_setting.bBackUpFolderRM || !IsLocalDrive(target_file))
	) {	// 指定フォルダにバックアップを作成する
		TCHAR selDir[_MAX_PATH];
		FileNameManager::ExpandMetaToFolder(bup_setting.szBackUpFolder, selDir, _countof(selDir));
		if (GetFullPathName(selDir, _MAX_PATH, szNewPath, &psNext) == 0) {
			// うまく取れなかった
			_tcscpy(szNewPath, selDir);
		}
		// フォルダの最後が半角かつ'\\'でない場合は、付加する
		AddLastYenFromDirectoryPath(szNewPath);
	}else {
		auto_sprintf(szNewPath, _T("%ts%ts"), szDrive, szDir);
	}

	// 相対フォルダを挿入
	if (!bup_setting.bBackUpPathAdvanced) {
		time_t	ltime;
		struct	tm* today;
		wchar_t	szTime[64];
		wchar_t	szForm[64];

		TCHAR* pBase = szNewPath + _tcslen(szNewPath);
		ASSERT_GE(newPathCount, _tcslen(szNewPath));
		size_t nBaseCount = newPathCount - _tcslen(szNewPath);

		// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
		switch (bup_setting.GetBackupType()) {
		case 1:
			if (auto_snprintf_s(pBase, nBaseCount, _T("%ts.bak"), szFname) == -1) {
				return false;
			}
			break;
		case 5:
			if (auto_snprintf_s(pBase, nBaseCount, _T("%ts%ts.bak"), szFname, szExt) == -1) {
				return false;
			}
			break;
		case 2:	//	日付，時刻
			_tzset();
			_wstrdate(szTime);
			time(&ltime);				// システム時刻を得ます
			today = localtime(&ltime);// 現地時間に変換する

			szForm[0] = 0;
			if (bup_setting.GetBackupOpt(BKUP_YEAR)) {	// バックアップファイル名：日付の年
				wcscat(szForm, L"%Y");
			}
			if (bup_setting.GetBackupOpt(BKUP_MONTH)) {	// バックアップファイル名：日付の月
				wcscat(szForm, L"%m");
			}
			if (bup_setting.GetBackupOpt(BKUP_DAY)) {	// バックアップファイル名：日付の日
				wcscat(szForm, L"%d");
			}
			if (bup_setting.GetBackupOpt(BKUP_HOUR)) {	// バックアップファイル名：日付の時
				wcscat(szForm, L"%H");
			}
			if (bup_setting.GetBackupOpt(BKUP_MIN)) {	// バックアップファイル名：日付の分
				wcscat(szForm, L"%M");
			}
			if (bup_setting.GetBackupOpt(BKUP_SEC)) {	// バックアップファイル名：日付の秒
				wcscat(szForm, L"%S");
			}
			// YYYYMMDD時分秒 形式に変換
			wcsftime(szTime, _countof(szTime) - 1, szForm, today);
			if (auto_snprintf_s(pBase, nBaseCount, _T("%ts_%ls%ts"), szFname, szTime, szExt) == -1) {
				return false;
			}
			break;
		// ファイルに付ける日付を前回の保存時(更新日時)にする
		case 4:	//	日付，時刻
			{
				FileTime ctimeLastWrite;
				GetLastWriteTimestamp(target_file, &ctimeLastWrite);

				szTime[0] = 0;
				if (bup_setting.GetBackupOpt(BKUP_YEAR)) {	// バックアップファイル名：日付の年
					auto_sprintf(szTime, L"%d", ctimeLastWrite->wYear);
				}
				if (bup_setting.GetBackupOpt(BKUP_MONTH)) {	// バックアップファイル名：日付の月
					auto_sprintf(szTime, L"%ls%02d", szTime, ctimeLastWrite->wMonth);
				}
				if (bup_setting.GetBackupOpt(BKUP_DAY)) {	// バックアップファイル名：日付の日
					auto_sprintf(szTime, L"%ls%02d", szTime, ctimeLastWrite->wDay);
				}
				if (bup_setting.GetBackupOpt(BKUP_HOUR)) {	// バックアップファイル名：日付の時
					auto_sprintf(szTime, L"%ls%02d", szTime, ctimeLastWrite->wHour);
				}
				if (bup_setting.GetBackupOpt(BKUP_MIN)) {	// バックアップファイル名：日付の分
					auto_sprintf(szTime, L"%ls%02d", szTime, ctimeLastWrite->wMinute);
				}
				if (bup_setting.GetBackupOpt(BKUP_SEC)) {	// バックアップファイル名：日付の秒
					auto_sprintf(szTime, L"%ls%02d", szTime, ctimeLastWrite->wSecond);
				}
				if (auto_sprintf_s(pBase, nBaseCount, _T("%ts_%ls%ts"), szFname, szTime, szExt) == -1) {
					return false;
				}
			}
			break;

		case 3: //	?xx : xx = 00~99, ?は任意の文字
		case 6: //	Jun.  5, 2005 genta 3の拡張子を残す版
			//	ここでは作成するバックアップファイル名のみ生成する．
			//	ファイル名のRotationは確認ダイアログの後で行う．
			{
				// 拡張子を残せるように処理起点を操作する
				TCHAR* ptr;
				if (bup_setting.GetBackupType() == 3) {
					ptr = szExt;
				}else {
					ptr = szExt + _tcslen(szExt);
				}
				*ptr   = _T('.');
				*++ptr = bup_setting.GetBackupExtChar();
				*++ptr = _T('0');
				*++ptr = _T('0');
				*++ptr = _T('\0');
			}
			if (auto_snprintf_s(pBase, nBaseCount, _T("%ts%ts"), szFname, szExt) == -1) {
				return false;
			}
			break;
		}
	}else { // 詳細設定使用する
		TCHAR szFormat[1024];

		switch (bup_setting.GetBackupTypeAdv()) {
		case 4:	//	ファイルの日付，時刻
			{
				FileTime ctimeLastWrite;
				GetLastWriteTimestamp(target_file, &ctimeLastWrite);
				if (!GetDateTimeFormat(szFormat, _countof(szFormat), bup_setting.szBackUpPathAdvanced , ctimeLastWrite.GetSYSTEMTIME())) {
					return false;
				}
			}
			break;
		case 2:	//	現在の日付，時刻
		default:
			{
				// 詳細設定のファイル保存日時と現在時刻で書式を合わせる
				SYSTEMTIME	SystemTime;
				::GetSystemTime(&SystemTime);			// 現在時刻を取得

				if (!GetDateTimeFormat(szFormat, _countof(szFormat), bup_setting.szBackUpPathAdvanced , SystemTime)) {
					return false;
				}
			}
			break;
		}

		{
			// make keys
			// $0-$9に対応するフォルダ名を切り出し
			TCHAR keybuff[1024];
			_tcscpy(keybuff, szDir);
			CutLastYenFromDirectoryPath(keybuff);

			TCHAR* folders[10];
			{
				int idx;
				for (idx=0; idx<10; ++idx) {
					folders[idx] = 0;
				}
				folders[0] = szFname;

				for (idx=1; idx<10; ++idx) {
					TCHAR* cp = _tcsrchr(keybuff, _T('\\'));
					if (cp) {
						folders[idx] = cp + 1;
						*cp = _T('\0');
					}else {
						break;
					}
				}
			}
			{
				// $0-$9を置換
				//wcscpy(szNewPath, L"");
				TCHAR* q = szFormat;
				TCHAR* q2 = szFormat;
				while (*q) {
					if (*q == _T('$')) {
						++q;
						if (isdigit(*q)) {
							q[-1] = _T('\0');
							_tcscat(szNewPath, q2);
//							if (newPathCount <  auto_strlcat(szNewPath, q2, newPathCount)) {
//								return false;
//							}
							if (folders[*q-_T('0')] != 0) {
								_tcscat(szNewPath, folders[*q-_T('0')]);
//								if (newPathCount < auto_strlcat(szNewPath, folders[*q-_T('0')], newPathCount)) {
//									return false;
//								}
							}
							q2 = q + 1;
						}
					}
					++q;
				}
				_tcscat(szNewPath, q2);
//				if (newPathCount < auto_strlcat(szNewPath, q2, newPathCount)) {
//					return false;
//				}
			}
		}
		{
			TCHAR temp[1024];
			TCHAR* cp;
			TCHAR* ep = (szExt[0] != 0) ? &szExt[1] : &szExt[0];
			assert(newPathCount <= _countof(temp));

			// * を拡張子にする
			while (_tcschr(szNewPath, _T('*'))) {
				_tcscpy(temp, szNewPath);
				cp = _tcschr(temp, _T('*'));
				*cp = 0;
				if (auto_snprintf_s(szNewPath, newPathCount, _T("%ts%ts%ts"), temp, ep, cp + 1) == -1) {
					return false;
				}
			}
			//	??はバックアップ連番にしたいところではあるが，
			//	連番処理は末尾の2桁にしか対応していないので
			//	使用できない文字?を_に変換してお茶を濁す
			while ((cp = _tcschr(szNewPath, _T('?')))) {
				*cp = _T('_');
			}
		}
	}
	return true;
}

