#include "StdAfx.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/ShareData_IO.h"
#include "env/SakuraEnvironment.h"
#include "doc/DocListener.h" // LoadInfo
#include "_main/ControlTray.h"
#include "_main/CommandLine.h"
#include "_main/Mutex.h"
#include "charset/CodePage.h"
#include "debug/RunningTimer.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/os.h"
#include "DataProfile.h"
#include "sakura_rc.h"

struct ARRHEAD {
	int nLength;
	int nItemNum;
};

const unsigned int uShareDataVersion = N_SHAREDATA_VERSION;

ShareData::ShareData()
{
	hFileMap   = NULL;
	pShareData = nullptr;
	pvTypeSettings = nullptr;
}

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
ShareData::~ShareData()
{
	if (pShareData) {
		// プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします
		SetDllShareData(nullptr);
		::UnmapViewOfFile(pShareData);
		pShareData = nullptr;
	}
	if (hFileMap) {
		CloseHandle(hFileMap);
	}
	if (pvTypeSettings) {
		for (size_t i=0; i<pvTypeSettings->size(); ++i) {
			delete (*pvTypeSettings)[i];
			(*pvTypeSettings)[i] = nullptr;
		}
		delete pvTypeSettings;
		pvTypeSettings = nullptr;
	}
}


static Mutex g_cMutexShareWork( FALSE, GSTR_MUTEX_SAKURA_SHAREWORK );
 
Mutex& ShareData::GetMutexShareWork(){
	return g_cMutexShareWork;
}

// ShareDataクラスの初期化処理
/*!
	ShareDataクラスを利用する前に必ず呼び出すこと。

	@retval true 初期化成功
	@retval false 初期化失敗

	@note 既に存在する共有メモリのバージョンがこのエディタが使うものと
	異なる場合は致命的エラーを防ぐためにfalseを返します。Process::Initialize()
	でInit()に失敗するとメッセージを出してエディタの起動を中止します。
*/
bool ShareData::InitShareData()
{
	MY_RUNNINGTIMER(runningTimer, "ShareData::InitShareData");

	hwndTraceOutSource = NULL;

	// ファイルマッピングオブジェクト
	{
		std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
		std::tstring strShareDataName = GSTR_SHAREDATA;
		strShareDataName += strProfileName;
		hFileMap = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE | SEC_COMMIT,
			0,
			sizeof(DllSharedData),
			strShareDataName.c_str()
		);
	}
	if (!hFileMap) {
		::MessageBox(
			NULL,
			_T("CreateFileMapping()に失敗しました"),
			_T("予期せぬエラー"),
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		// オブジェクトが存在していなかった場合
		// ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします
		pShareData = (DllSharedData*)::MapViewOfFile(
			hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		CreateTypeSettings();
		SetDllShareData(pShareData);

		TCHAR szIniFolder[_MAX_PATH];
		pShareData->fileNameManagement.iniFolder.bInit = false;
		GetInidir(szIniFolder);
		AddLastChar(szIniFolder, _MAX_PATH, _T('\\'));

		pShareData->vStructureVersion = uShareDataVersion;
		pShareData->nSize = sizeof(*pShareData);

		// リソースから製品バージョンの取得
		GetAppVersionInfo(NULL, VS_VERSION_INFO,
			&pShareData->version.dwProductVersionMS, &pShareData->version.dwProductVersionLS);

		pShareData->flags.bEditWndChanging = false;		// 編集ウィンドウ切替中
		pShareData->flags.bRecordingKeyMacro = false;	// キーボードマクロの記録中
		pShareData->flags.hwndRecordingKeyMacro = NULL;	// キーボードマクロを記録中のウィンドウ

		pShareData->nodes.nSequences = 0;				// ウィンドウ連番
		pShareData->nodes.nNonameSequences = 0;
		pShareData->nodes.nGroupSequences = 0;			// タブグループ連番
		pShareData->nodes.nEditArrNum = 0;

		pShareData->handles.hwndTray = NULL;
		pShareData->handles.hAccel = NULL;
		pShareData->handles.hwndDebug = NULL;

		for (size_t i=0; i<_countof(pShareData->dwCustColors); ++i) {
			pShareData->dwCustColors[i] = RGB( 255, 255, 255 );
		}

		MruFile mru;
		mru.ClearAll();
		MruFolder mruFolder;
		mruFolder.ClearAll();

// MS ゴシック標準スタイル10ptに設定
//		// LOGFONTの初期化
		LOGFONT lf = {0};
		lf.lfHeight			= DpiPointsToPixels(-10);	// 高DPI対応（ポイント数から算出）
		lf.lfWidth			= 0;
		lf.lfEscapement		= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0x0;
		lf.lfUnderline		= 0x0;
		lf.lfStrikeOut		= 0x0;
		lf.lfCharSet		= 0x80;
		lf.lfOutPrecision	= 0x3;
		lf.lfClipPrecision	= 0x2;
		lf.lfQuality		= 0x1;
		lf.lfPitchAndFamily	= 0x31;
		_tcscpy(lf.lfFaceName, _T("ＭＳ ゴシック"));

		// キーワードヘルプのフォント
		LOGFONT lfIconTitle;	// エクスプローラのファイル名表示に使用されるフォント
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&lfIconTitle,					// depends on action to be taken
			0										// user profile update flag
		);
		INT nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);	// フォントサイズ（1/10ポイント単位）

		// [全般]タブ
		{
			CommonSetting_General& general = pShareData->common.general;

			general.nMRUArrNum_MAX = 15;			// ファイルの履歴MAX
			general.nOPENFOLDERArrNum_MAX = 15;		// フォルダの履歴MAX

			general.nCaretType = 0;					// カーソルのタイプ 0=win 1=dos
			general.bIsINSMode = true;				// 挿入／上書きモード
			general.bIsFreeCursorMode = false;		// フリーカーソルモードか

			general.bStopsBothEndsWhenSearchWord = false;	// 単語単位で移動するときに、単語の両端で止まるか
			general.bStopsBothEndsWhenSearchParagraph = false;	// 単語単位で移動するときに、単語の両端で止まるか

			general.bCloseAllConfirm = false;		// [すべて閉じる]で他に編集用のウィンドウがあれば確認する
			general.bExitConfirm = false;			// 終了時の確認をする
			general.nRepeatedScrollLineNum = 3;		// キーリピート時のスクロール行数
			general.nRepeatedScroll_Smooth = false;	// キーリピート時のスクロールを滑らかにするか
			general.nPageScrollByWheel = 0;			// キー/マウスボタン + ホイールスクロールでページスクロールする
			general.nHorizontalScrollByWheel = 0;	// キー/マウスボタン + ホイールスクロールで横スクロールする

			general.bUseTaskTray = true;			// タスクトレイのアイコンを使う
#ifdef _DEBUG
			general.bStayTaskTray = false;			// タスクトレイのアイコンを常駐
#else
			general.bStayTaskTray = true;			// タスクトレイのアイコンを常駐
#endif
			general.wTrayMenuHotKeyCode = L'Z';		// タスクトレイ左クリックメニュー キー
			general.wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	// タスクトレイ左クリックメニュー キー

			general.bDispExitingDialog = false;		// 終了ダイアログを表示する

			general.bNoCaretMoveByActivation = false;	// マウスクリックにてアクティベートされた時はカーソル位置を移動しない
		}

		// [ウィンドウ]タブ
		{
			CommonSetting_Window& window = pShareData->common.window;

			window.bDispToolBar = true;				// 次回ウィンドウを開いたときツールバーを表示する
			window.bDispStatusBar = true;			// 次回ウィンドウを開いたときステータスバーを表示する
			window.bDispFuncKeyWnd = false;			// 次回ウィンドウを開いたときファンクションキーを表示する
			window.bDispMiniMap = false;			// ミニマップを表示する
			window.nFuncKeyWnd_Place = 1;			// ファンクションキー表示位置／0:上 1:下
			window.nFuncKeyWnd_GroupNum = 4;		// ファンクションキーのグループボタン数
			window.nMiniMapFontSize = -1;
			window.nMiniMapQuality = NONANTIALIASED_QUALITY;
			window.nMiniMapWidth = 150;

			window.bSplitterWndHScroll = true;	// 分割ウィンドウの水平スクロールの同期をとる
			window.bSplitterWndVScroll = true;	// 分割ウィンドウの垂直スクロールの同期をとる

			// 補完とキーワードヘルプはタイプ別に移動したので削除
			// ウィンドウサイズ固定指定追加に伴う指定方法変更
			window.eSaveWindowSize = WinSizeMode::Save;	// ウィンドウサイズ継承
			window.nWinSizeType = SIZE_RESTORED;
			window.nWinSizeCX = CW_USEDEFAULT;
			window.nWinSizeCY = 0;

			window.bScrollBarHorz = true;					// 水平スクロールバーを使う
			// ウィンドウ位置
			window.eSaveWindowPos = WinSizeMode::Default;	// ウィンドウ位置固定・継承
			window.nWinPosX = CW_USEDEFAULT;
			window.nWinPosY = 0;

			window.nRulerHeight = 13;						// ルーラーの高さ
			window.nRulerBottomSpace = 0;					// ルーラーとテキストの隙間
			window.nRulerType = 0;							// ルーラーのタイプ
			window.nLineNumRightSpace = 0;					// 行番号の右の隙間
			window.nVertLineOffset = -1;					// 指定桁縦線
			window.bUseCompatibleBMP = true;				// 画面キャッシュを使う

			window.bMenuIcon = true;						// メニューにアイコンを表示する

			//	ウィンドウキャプションの初期値
			//	$N(ファイル名省略表示)をデフォルトに変更
			_tcscpy( window.szWindowCaptionActive, 
				_T("${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} -")
				_T(" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>") );
			_tcscpy( window.szWindowCaptionInactive, 
				_T("${w?$h$:アウトプット$:$f$n$}${U?(更新)$} -")
				_T(" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>") );
		}
		
		// [タブバー]タブ
		{
			CommonSetting_TabBar& tabBar = pShareData->common.tabBar;

			tabBar.bDispTabWnd = false;				// タブウィンドウ表示
			tabBar.bDispTabWndMultiWin = false;		// タブウィンドウ表示
			wcscpy(
				tabBar.szTabWndCaption,
				L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
			);
			tabBar.bSameTabWidth = false;			// タブを等幅にする
			tabBar.bDispTabIcon = false;			// タブにアイコンを表示する
			tabBar.dispTabClose = DispTabCloseType::No;	// タブに閉じるボタンを表示する
			tabBar.bSortTabList = true;				// タブ一覧をソートする
			tabBar.bTab_RetainEmptyWin = true;		// 最後のファイルが閉じられたとき(無題)を残す
			tabBar.bTab_CloseOneWin = false;		// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる
			tabBar.bTab_ListFull = false;			// タブ一覧をフルパス表示する
			tabBar.bChgWndByWheel = false;			// マウスホイールでウィンドウ切替
			tabBar.bNewWindow = false;				// 外部から起動するときは新しいウィンドウで開く
			tabBar.bTabMultiLine = false;			// タブ多段
			tabBar.eTabPosition = TabPosition::Top;	// タブ位置

			tabBar.lf = lfIconTitle;
			tabBar.nPointSize = nIconPointSize;
			tabBar.nTabMaxWidth = 200;
			tabBar.nTabMinWidth = 60;
			tabBar.nTabMinWidthOnMulti = 100;
		}

		// [編集]タブ
		{
			CommonSetting_Edit& edit = pShareData->common.edit;

			edit.bAddCRLFWhenCopy = false;			// 折り返し行に改行を付けてコピー

			edit.bUseOLE_DragDrop = true;			// OLEによるドラッグ & ドロップを使う
			edit.bUseOLE_DropSource = true;			// OLEによるドラッグ元にするか
			edit.bSelectClickedURL = true;			// URLがクリックされたら選択するか
			edit.bCopyAndDisablSelection = false;	// コピーしたら選択解除
			edit.bEnableNoSelectCopy = true;		// 選択なしでコピーを可能にする
			edit.bEnableLineModePaste = true;		// ラインモード貼り付けを可能にする
			edit.bConvertEOLPaste = false;			// 改行コードを変換して貼り付ける
			edit.bEnableExtEol = false;
			edit.bBoxSelectLock = true;

			edit.bNotOverWriteCRLF = true;			// 改行は上書きしない
			edit.bOverWriteFixMode = false;			// 文字幅に合わせてスペースを詰める

			edit.bOverWriteBoxDelete = false;
			edit.eOpenDialogDir = OPENDIALOGDIR_CUR;
			auto_strcpy(edit.openDialogSelDir, _T("%Personal%\\"));
			edit.bAutoColumnPaste = true;			// 矩形コピーのテキストは常に矩形貼り付け
		}

		// [ファイル]タブ
		{
			CommonSetting_File& file = pShareData->common.file;

			// ファイルの排他制御
			file.nFileShareMode = FileShareMode::DenyWrite;	// ファイルの排他制御モード
			file.bCheckFileTimeStamp = true;			// 更新の監視
			file.nAutoloadDelay = 0;					// 自動読込時遅延
			file.bUneditableIfUnwritable = true;		// 上書き禁止検出時は編集禁止にする

			// ファイルの保存
			file.bEnableUnmodifiedOverwrite = false;	// 無変更でも上書きするか

			//「名前を付けて保存」でファイルの種類が[ユーザ指定]のときのファイル一覧表示
			file.bNoFilterSaveNew = true;		// 新規から保存時は全ファイル表示
			file.bNoFilterSaveFile = true;		// 新規以外から保存時は全ファイル表示

			// ファイルオープン
			file.bDropFileAndClose = false;		// ファイルをドロップしたときは閉じて開く
			file.nDropFileNumMax = 8;			// 一度にドロップ可能なファイル数
			file.bRestoreCurPosition = true;	// カーソル位置復元
			file.bRestoreBookmarks = true;		// ブックマーク復元
			file.bAutoMimeDecode = false;		// ファイル読み込み時にMIMEのデコードを行うか
			file.bQueryIfCodeChange = true;		// 前回と異なる文字コードの時に問い合わせを行うか
			file.bAlertIfFileNotExist = false;	// 開こうとしたファイルが存在しないとき警告する
			file.bAlertIfLargeFile = false;		// 開こうとしたファイルが大きい場合に警告する
			file.nAlertFileSize = 10;			// 警告を始めるファイルサイズ（MB単位）
		}

		// [バックアップ]タブ
		{
			CommonSetting_Backup& backup = pShareData->common.backup;

			backup.bBackUp = false;										// バックアップの作成
			backup.bBackUpDialog = true;									// バックアップの作成前に確認
			backup.bBackUpFolder = false;								// 指定フォルダにバックアップを作成する
			backup.szBackUpFolder[0] = L'\0';							// バックアップを作成するフォルダ
			backup.nBackUpType = 2;										// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
			backup.nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;	// バックアップファイル名：日付
			backup.nBackUpType_Opt2 = ('b' << 16 ) + 10;					// バックアップファイル名：連番の数と先頭文字
			backup.nBackUpType_Opt3 = 5;									// バックアップファイル名：Option3
			backup.nBackUpType_Opt4 = 0;									// バックアップファイル名：Option4
			backup.nBackUpType_Opt5 = 0;									// バックアップファイル名：Option5
			backup.nBackUpType_Opt6 = 0;									// バックアップファイル名：Option6
			backup.bBackUpDustBox = false;								// バックアップファイルをごみ箱に放り込む
			backup.bBackUpPathAdvanced = false;							// バックアップ先フォルダを詳細設定する
			backup.szBackUpPathAdvanced[0] = _T('\0');					// バックアップを作成するフォルダの詳細設定
		}

		// [書式]タブ
		{
			CommonSetting_Format& format = pShareData->common.format;

			// 見出し記号
			wcscpy( format.szMidashiKigou, L"１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]一二三四五六七八九十壱弐参伍" );
			// 引用符
			wcscpy( format.szInyouKigou, L"> " );		// 引用符

			/*
				書式指定子の意味はWindows SDKのGetDateFormat(), GetTimeFormat()を参照のこと
			*/

			format.nDateFormatType = 0;	//日付書式のタイプ
			_tcscpy( format.szDateFormat, _T("yyyy\'年\'M\'月\'d\'日(\'dddd\')\'") );	//日付書式
			format.nTimeFormatType = 0;	//時刻書式のタイプ
			_tcscpy( format.szTimeFormat, _T("tthh\'時\'mm\'分\'ss\'秒\'")  );			//時刻書式
		}

		// [検索]タブ
		{
			CommonSetting_Search& search = pShareData->common.search;

			search.searchOption.Reset();			// 検索オプション
			search.bConsecutiveAll = 0;				// 「すべて置換」は置換の繰返し
			search.bSelectedArea = false;			// 選択範囲内置換
			search.bNotifyNotFound = true;			// 検索／置換  見つからないときメッセージを表示

			search.bGrepSubFolder = true;			// Grep: サブフォルダも検索
			search.nGrepOutputLineType = 1;			// Grep: 行を出力/該当部分/否マッチ行 を出力
			search.nGrepOutputStyle = 1;			// Grep: 出力形式
			search.bGrepOutputFileOnly = false;
			search.bGrepOutputBaseFolder = false;
			search.bGrepSeparateFolder = false;
			search.bGrepBackup = true;

			search.bGrepDefaultFolder = false;		// Grep: フォルダの初期値をカレントフォルダにする
			search.nGrepCharSet = CODE_AUTODETECT;	// Grep: 文字コードセット
			search.bGrepRealTimeView = false;		// Grep結果のリアルタイム表示
			search.bCaretTextForSearch = true;		// カーソル位置の文字列をデフォルトの検索文字列にする
			search.bInheritKeyOtherView = true;
			search.szRegexpLib[0] = _T('\0');		// 正規表現DLL
			search.bGTJW_Return = true;				// エンターキーでタグジャンプ
			search.bGTJW_DoubleClick = true;		// ダブルクリックでタグジャンプ

			search.bGrepExitConfirm = false;		// Grepモードで保存確認するか

			search.bAutoCloseDlgFind = true;		// 検索ダイアログを自動的に閉じる
			search.bSearchAll		= false;		// 検索／置換／ブックマーク  先頭（末尾）から再検索
			search.bAutoCloseDlgReplace = true;		// 置換 ダイアログを自動的に閉じる

			search.nTagJumpMode = 1;				//タグジャンプモード
			search.nTagJumpModeKeyword = 3;			//タグジャンプモード
		}

		// [キー割り当て]タブ
		{
			if (!InitKeyAssign(*pShareData)) {
				return false;
			}
		}

		// [カスタムメニュー]タブ
		{
			CommonSetting_CustomMenu& customMenu = pShareData->common.customMenu;

			for (int i=0; i<MAX_CUSTOM_MENU; ++i) {
				customMenu.szCustMenuNameArr[i][0] = '\0';
				customMenu.nCustMenuItemNumArr[i] = 0;
				for (int j=0; j<MAX_CUSTOM_MENU_ITEMS; ++j) {
					customMenu.nCustMenuItemFuncArr[i][j] = F_0;
					customMenu.nCustMenuItemKeyArr [i][j] = '\0';
				}
				customMenu.bCustMenuPopupArr[i] = true;
			}
			customMenu.szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND][0] = '\0';

			InitPopupMenu(*pShareData);
		}

		// [ツールバー]タブ
		{
			InitToolButtons(*pShareData);
		}

		// [強調キーワード]タブ
		{
			InitKeyword(*pShareData);
		}

		// [支援]タブ
		{
			CommonSetting_Helper& helper = pShareData->common.helper;

			helper.lf = lfIconTitle;
			helper.nPointSize = nIconPointSize;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化

			helper.szExtHelp[0] = L'\0';		// 外部ヘルプ１
			helper.szExtHtmlHelp[0] = L'\0';	// 外部HTMLヘルプ
		
			helper.szMigemoDll[0] = L'\0';		// migemo dll
			helper.szMigemoDict[0] = L'\0';		// migemo dict

			helper.bHtmlHelpIsSingle = true;	// HtmlHelpビューアはひとつ

			helper.bHokanKey_RETURN	= true;		// VK_RETURN 補完決定キーが有効/無効
			helper.bHokanKey_TAB	= false;	// VK_TAB   補完決定キーが有効/無効
			helper.bHokanKey_RIGHT	= true;		// VK_RIGHT 補完決定キーが有効/無効
			helper.bHokanKey_SPACE	= false;	// VK_SPACE 補完決定キーが有効/無効

			helper.bUseHokan = false;			// 入力補完機能を使用する
		}

		// [アウトライン]タブ
		{
			CommonSetting_OutLine& outline = pShareData->common.outline;

			outline.nOutlineDockSet = 0;				// アウトライン解析のドッキング位置継承方法
			outline.bOutlineDockSync = true;			// アウトライン解析のドッキング位置を同期する
			outline.bOutlineDockDisp = false;			// アウトライン解析表示の有無
			outline.eOutlineDockSide = DockSideType::Float;	// アウトライン解析ドッキング配置
			outline.cxOutlineDockLeft		=	0;		// アウトラインの左ドッキング幅
			outline.cyOutlineDockTop		=	0;		// アウトラインの上ドッキング高
			outline.cxOutlineDockRight		=	0;		// アウトラインの右ドッキング幅
			outline.cyOutlineDockBottom		=	0;		// アウトラインの下ドッキング高
			outline.nDockOutline = OutlineType::Text;
			outline.bAutoCloseDlgFuncList = false;		// アウトライン ダイアログを自動的に閉じる
			outline.bMarkUpBlankLineEnable	=	false;	// アウトラインダイアログでブックマークの空行を無視
			outline.bFunclistSetFocusOnJump	=	false;	// アウトラインダイアログでジャンプしたらフォーカスを移す

			InitFileTree( &outline.fileTree );
			outline.fileTreeDefIniName = _T("_sakurafiletree.ini");
		}

		// [ファイル内容比較]タブ
		{
			CommonSetting_Compare& compare = pShareData->common.compare;

			compare.bCompareAndTileHorz = true;		// 文書比較後、左右に並べて表示
		}

		// [ビュー]タブ
		{
			CommonSetting_View& view = pShareData->common.view;

			view.lf = lf;
			view.nPointSize = 0;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化

			view.bFontIs_FixedPitch = true;				// 現在のフォントは固定幅フォントである
		}

		// [マクロ]タブ
		{
			CommonSetting_Macro& macro = pShareData->common.macro;

			macro.szKeyMacroFileName[0] = _T('\0');	// キーワードマクロのファイル名

			// Macro登録の初期化
			MacroRec *mptr = macro.macroTable;
			for (int i=0; i<MAX_CUSTMACRO; ++i, ++mptr) {
				mptr->szName[0] = L'\0';
				mptr->szFile[0] = L'\0';
				mptr->bReloadWhenExecute = false;
			}

			_tcscpy( macro.szMACROFOLDER, szIniFolder );	// マクロ用フォルダ

			macro.nMacroOnOpened = -1;		// オープン後自動実行マクロ番号
			macro.nMacroOnTypeChanged = -1;	// タイプ変更後自動実行マクロ番号
			macro.nMacroOnSave = -1;		// 保存前自動実行マクロ番号
			macro.nMacroCancelTimer = 10;	// マクロ停止ダイアログ表示待ち時間(秒)
		}

		// [ファイル名表示]タブ
		{
			CommonSetting_FileName& fileName = pShareData->common.fileName;

			fileName.bTransformShortPath = true;
			fileName.nTransformShortMaxWidth = 100; // 100'x'幅

			for (int i=0; i<MAX_TRANSFORM_FILENAME; ++i) {
				fileName.szTransformFileNameFrom[i][0] = _T('\0');
				fileName.szTransformFileNameTo[i][0] = _T('\0');
			}
			_tcscpy( fileName.szTransformFileNameFrom[0], _T("%DeskTop%\\") );
			_tcscpy( fileName.szTransformFileNameTo[0],   _T("デスクトップ\\") );
			_tcscpy( fileName.szTransformFileNameFrom[1], _T("%Personal%\\") );
			_tcscpy( fileName.szTransformFileNameTo[1],   _T("マイドキュメント\\") );
			_tcscpy( fileName.szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
			_tcscpy( fileName.szTransformFileNameTo[2],   _T("IEキャッシュ\\") );
			_tcscpy( fileName.szTransformFileNameFrom[3], _T("%TEMP%\\") );
			_tcscpy( fileName.szTransformFileNameTo[3],   _T("TEMP\\") );
			_tcscpy( fileName.szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
			_tcscpy( fileName.szTransformFileNameTo[4],   _T("共有デスクトップ\\") );
			_tcscpy( fileName.szTransformFileNameFrom[5], _T("%Common Documents%\\") );
			_tcscpy( fileName.szTransformFileNameTo[5],   _T("共有ドキュメント\\") );
			_tcscpy( fileName.szTransformFileNameFrom[6], _T("%AppData%\\") );
			_tcscpy( fileName.szTransformFileNameTo[6],   _T("アプリデータ\\") );
			fileName.nTransformFileNameArrNum = 7;
		}

		// [その他]タブ
		{
			CommonSetting_Others& others = pShareData->common.others;

			::SetRect( &others.rcOpenDialog, 0, 0, 0, 0 );		// 「開く」ダイアログのサイズと位置
			::SetRect( &others.rcCompareDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcDiffDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcFavoriteDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcTagJumpDialog, 0, 0, 0, 0 );
		}

		// [ステータスバー]タブ
		{
			CommonSetting_StatusBar& statusbar = pShareData->common.statusBar;

			// 表示文字コードの指定
			statusbar.bDispUniInSjis		= false;	// SJISで文字コード値をUnicodeで表示する
			statusbar.bDispUniInJis			= false;	// JISで文字コード値をUnicodeで表示する
			statusbar.bDispUniInEuc			= false;	// EUCで文字コード値をUnicodeで表示する
			statusbar.bDispUtf8Codepoint	= true;		// UTF-8をコードポイントで表示する
			statusbar.bDispSPCodepoint		= true;		// サロゲートペアをコードポイントで表示する
			statusbar.bDispSelCountByByte	= false;	// 選択文字数を文字単位ではなくバイト単位で表示する
		}

		// [プラグイン]タブ
		{
			CommonSetting_Plugin& plugin = pShareData->common.plugin;

			plugin.bEnablePlugin			= false;	// プラグインを使用する
			for (int nPlugin=0; nPlugin<MAX_PLUGIN; ++nPlugin) {
				plugin.pluginTable[nPlugin].szName[0] = L'\0';	// プラグイン名
				plugin.pluginTable[nPlugin].szId[0]	= L'\0';	// プラグインID
				plugin.pluginTable[nPlugin].state = PLS_NONE;	// プラグイン状態
			}
		}

		// [メインメニュー]タブ
		{
			DataProfile	profile;
			std::vector<std::wstring> data;
			profile.SetReadingMode();
			profile.ReadProfileRes( MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI), &data );

			ShareData_IO::IO_MainMenu( profile, &data, pShareData->common.mainMenu, false );
		}

		{
			InitTypeConfigs(*pShareData, *pvTypeSettings);
		}

		{
			/* printSettingArr[0]を設定して、残りの1〜7にコピーする。
				必要になるまで遅らせるために、Printに、ShareDataを操作する権限を与える。
			*/
			{
				TCHAR szSettingName[64];
				int i = 0;
				auto_sprintf( szSettingName, _T("印刷設定 %d"), i + 1 );
				Print::SettingInitialize( pShareData->printSettingArr[0], szSettingName );	//	初期化命令。
			}
			for (int i=1; i<MAX_PrintSettingARR; ++i) {
				pShareData->printSettingArr[i] = pShareData->printSettingArr[0];
				auto_sprintf( pShareData->printSettingArr[i].szPrintSettingName, _T("印刷設定 %d"), i + 1 );	// 印刷設定の名前
			}
		}

		{
			pShareData->searchKeywords.searchKeys.clear();
			pShareData->searchKeywords.replaceKeys.clear();
			pShareData->searchKeywords.grepFiles.clear();
			pShareData->searchKeywords.grepFiles.push_back(_T("*.*"));
			pShareData->searchKeywords.grepFolders.clear();

			// タグジャンプ機能追加
			pShareData->tagJump.tagJumpNum = 0;
			// タグジャンプの先頭
			pShareData->tagJump.tagJumpTop = 0;
			// キーワード指定タグジャンプのHistory保管
			pShareData->tagJump.aTagJumpKeywords.clear();
			pShareData->tagJump.bTagJumpICase = false;
			pShareData->tagJump.bTagJumpAnyWhere = false;

			pShareData->history.aExceptMRU.clear();

			_tcscpy( pShareData->history.szIMPORTFOLDER, szIniFolder );	// 設定インポート用フォルダ

			pShareData->history.aCommands.clear();
			pShareData->history.aCurDirs.clear();

			pShareData->nExecFlgOpt = 1;	// 外部コマンド実行の「標準出力を得る」

			pShareData->nDiffFlgOpt = 0;	// DIFF差分表示

			pShareData->szTagsCmdLine[0] = _T('\0');	// CTAGS
			pShareData->nTagsOpt = 0;	/* CTAGS */

			pShareData->bLineNumIsCRLF_ForJump = true;	// 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か
		}
	}else {
		// オブジェクトがすでに存在する場合
		// ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします
		pShareData = (DllSharedData*)::MapViewOfFile(
			hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		SetDllShareData(pShareData);

		SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Share);
		InitCharWidthCache(pShareData->common.view.lf);

		// サイズチェック追加
		if (pShareData->vStructureVersion != uShareDataVersion
			|| pShareData->nSize != sizeof(*pShareData)
		) {
			// この共有データ領域は使えない．
			// ハンドルを解放する
			SetDllShareData(nullptr);
			::UnmapViewOfFile(pShareData);
			pShareData = nullptr;
			return false;
		}

	}
	return true;
}

static
void ConvertLangString(wchar_t* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to)
{
	NativeW mem;
	mem.SetString(pBuf);
	mem.Replace(org.c_str(), to.c_str());
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = L'\0';
}

static
void ConvertLangString(char* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to)
{
	NativeA mem;
	mem.SetString(pBuf);
	mem.Replace_j(to_achar(org.c_str()), to_achar(to.c_str()));
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = '\0';
}

static
void ConvertLangValueImpl(
	wchar_t* pBuf,
	size_t chBufSize,
	int nStrId,
	std::vector<std::wstring>& values,
	int& index,
	bool setValues,
	bool bUpdate
	)
{
	if (setValues) {
		if (bUpdate) {
			values.push_back(std::wstring(LSW(nStrId)));
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString(pBuf, chBufSize, values[index], to);
	++index;
}

static
void ConvertLangValueImpl(
	char* pBuf,
	size_t chBufSize,
	int nStrId,
	std::vector<std::wstring>& values,
	int& index,
	bool setValues,
	bool bUpdate
	)
{
	if (setValues) {
		if (bUpdate) {
			values.push_back(std::wstring(LSW(nStrId)));
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString(pBuf, chBufSize, values[index], to);
	++index;
}

#define ConvertLangValue(buf, id)  ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, true);
#define ConvertLangValue2(buf, id) ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, false);


/*!
	国際化対応のための文字列を変更する

	1. 1回目呼び出し、setValuesをtrueにして、valuesに旧設定の言語文字列を読み込み
	2. SelectLang呼び出し
	3. 2回目呼び出し、valuesを使って新設定の言語に書き換え
*/
void ShareData::ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues)
{
	DllSharedData& shareData = *pShareData;
	int index = 0;
	int indexBackup;
	CommonSetting& common = shareData.common;
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_TAB_CAPTION_OUTPUT);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_TAB_CAPTION_GREP);
	indexBackup = index;
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_KEYMACRO);
	index = indexBackup;
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_KEYMACRO);
	index = indexBackup;
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_KEYMACRO);
	ConvertLangValue(common.format.szDateFormat, STR_DATA_FORMAT);
	ConvertLangValue(common.format.szTimeFormat, STR_TIME_FORMAT);
	indexBackup = index;
	for (int i=0; i<common.fileName.nTransformFileNameArrNum; ++i) {
		index = indexBackup;
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_COMDESKTOP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_COMDOC);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_DESKTOP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_MYDOC);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_IE);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_TEMP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_APPDATA);
		if (bSetValues) {
			break;
		}
	}
	indexBackup = index;
	for (int i=0; i<MAX_PrintSettingARR; ++i) {
		index = indexBackup;
		ConvertLangValue(shareData.printSettingArr[i].szPrintSettingName, STR_PRINT_SET_NAME);
		if (bSetValues) {
			break;
		}
	}
	assert(pvTypeSettings);
	indexBackup = index;
	ConvertLangValue(shareData.typeBasis.szTypeName, STR_TYPE_NAME_BASIS);
	for (size_t i=0; i<GetTypeSettings().size(); ++i) {
		index = indexBackup;
		TypeConfig& type = *(GetTypeSettings()[i]);
		ConvertLangValue2(type.szTypeName, STR_TYPE_NAME_BASIS);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_RICHTEXT);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_TEXT);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_DOS);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_ASM);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_INI);
		index = indexBackup;
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_BASIS);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_RICHTEXT);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_TEXT);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_DOS);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_ASM);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_INI);
		if (bSetValues) {
			break;
		}
	}
}

/*!
	@brief	指定ファイルが開かれているか調べる
	
	指定のファイルが開かれている場合は開いているウィンドウのハンドルを返す

	@retval	true すでに開いていた
	@retval	false 開いていなかった
*/
bool ShareData::IsPathOpened(const TCHAR* pszPath, HWND* phwndOwner)
{
	*phwndOwner = NULL;

	// 相対パスを絶対パスに変換
	// 変換しないとIsPathOpenedで正しい結果が得られず，
	// 同一ファイルを複数開くことがある．
	TCHAR szBuf[_MAX_PATH];
	if (GetLongFileName(pszPath, szBuf)) {
		pszPath = szBuf;
	}

	// 現在の編集ウィンドウの数を調べる
	if (AppNodeGroupHandle(0).GetEditorWindowsNum() == 0) {
		return false;
	}
	
	for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
		if (IsSakuraMainWindow(pShareData->nodes.pEditArr[i].hWnd)) {
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage(pShareData->nodes.pEditArr[i].hWnd, MYWM_GETFILEINFO, 1, 0);
			EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

			// 同一パスのファイルが既に開かれているか
			if (_tcsicmp(pfi->szPath, pszPath) == 0) {
				*phwndOwner = pShareData->nodes.pEditArr[i].hWnd;
				return true;
			}
		}
	}
	return false;
}

/*!
	@brief	指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認

	もしすでに開いていればアクティブにして、ウィンドウのハンドルを返す。
	さらに、文字コードが異なるときのワーニングも処理する。
	あちこちに散らばった多重オープン処理を集結させるのが目的。

	@retval	開かれている場合は開いているウィンドウのハンドル

	@note	CEditDoc::FileLoadに先立って実行されることもあるが、
			CEditDoc::FileLoadからも実行される必要があることに注意。
			(フォルダ指定の場合やCEditDoc::FileLoadが直接実行される場合もあるため)

	@retval	true すでに開いていた
	@retval	false 開いていなかった
*/
bool ShareData::ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode)
{
	if (IsPathOpened(pszPath, phwndOwner)) {
		
		// 文字コードの一致確認
		::SendMessage(*phwndOwner, MYWM_GETFILEINFO, 0, 0);
		EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
		if (nCharCode != CODE_AUTODETECT) {
			TCHAR szCpNameCur[100];
			CodePage::GetNameLong(szCpNameCur, pfi->nCharCode);
			TCHAR szCpNameNew[100];
			CodePage::GetNameLong(szCpNameNew, pfi->nCharCode);
			if (szCpNameCur[0] && szCpNameNew[0]) {
				if (nCharCode != pfi->nCharCode) {
					TopWarningMessage(*phwndOwner,
						LS(STR_ERR_CSHAREDATA20),
						pszPath,
						szCpNameCur,
						szCpNameNew
					);
				}
			}else {
				TopWarningMessage(*phwndOwner,
					LS(STR_ERR_CSHAREDATA21),
					pszPath,
					pfi->nCharCode,
					szCpNameCur[0] == NULL ? LS(STR_ERR_CSHAREDATA22) : szCpNameCur,
					nCharCode,
					szCpNameNew[0] == NULL ? LS(STR_ERR_CSHAREDATA22) : szCpNameNew
				);
			}
		}

		// 開いているウィンドウをアクティブにする
		ActivateFrameWindow(*phwndOwner);

		// MRUリストへの登録
		MruFile().Add(pfi);
		return true;
	}else {
		return false;
	}

}


/*!
	アウトプットウィンドウに出力(書式付)

	アウトプットウィンドウが無ければオープンする
	@param lpFmt [in] 書式指定文字列(wchar_t版)
*/
void ShareData::TraceOut(LPCTSTR lpFmt, ...)
{
	if (!OpenDebugWindow(hwndTraceOutSource, false)) {
		return;
	}
	
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(pShareData->workBuffer.GetWorkBuffer<wchar_t>(), 
		pShareData->workBuffer.GetWorkBufferCount<wchar_t>(),
		to_wchar(lpFmt), argList);
	va_end(argList);
	if (ret == -1) {
		// 切り詰められた
		ret = auto_strlen(pShareData->workBuffer.GetWorkBuffer<wchar_t>());
	}else if (ret < 0) {
		// 保護コード:受け側はwParam→size_tで符号なしのため
		ret = 0;
	}
	DWORD_PTR dwMsgResult;
	::SendMessageTimeout(pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, ret, 0,
		SMTO_NORMAL, 10000, &dwMsgResult);
}

/*!
	アウトプットウィンドウに出力(文字列指定)

	長い場合は分割して送る
	アウトプットウィンドウが無ければオープンする
	@param  pStr  出力する文字列
	@param  len   pStrの文字数(終端NULを含まない) -1で自動計算
*/
void ShareData::TraceOutString(const wchar_t* pStr, int len)
{
	if (!OpenDebugWindow(hwndTraceOutSource, false)) {
		return;
	}
	if (len == -1) {
		len = wcslen(pStr);
	}
	// workBufferぎりぎりでも問題ないけれど、念のため\0終端にするために余裕をとる
	// -1 より 8,4バイト境界のほうがコピーが早いはずなので、-4にする
	const int buffLen = (int)pShareData->workBuffer.GetWorkBufferCount<wchar_t>() - 4;
	wchar_t*  pOutBuffer = pShareData->workBuffer.GetWorkBuffer<wchar_t>();
	int outPos = 0;
	if (len == 0) {
		// 0のときは何も追加しないが、カーソル移動が発生する
		LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
		pOutBuffer[0] = L'\0';
		::SendMessage(pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, 0, 0);
	}else {
		while (outPos < len) {
			int outLen = buffLen;
			if (len - outPos < buffLen) {
				// 残り全部
				outLen = len - outPos;
			}
			// あまりが1文字以上ある
			if (outPos + outLen < len) {
				// CRLF(\r\n)とUTF-16が分離されないように
				if ((pStr[outPos + outLen - 1] == WCODE::CR && pStr[outPos + outLen] == WCODE::LF)
					|| (IsUtf16SurrogHi(pStr[outPos + outLen - 1]) && IsUtf16SurrogLow(pStr[outPos + outLen]))
				) {
					--outLen;
				}
			}
			LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
			wmemcpy(pOutBuffer, pStr + outPos, outLen);
			pOutBuffer[outLen] = L'\0';
			DWORD_PTR dwMsgResult;
			if (::SendMessageTimeout(pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, outLen, 0,
				SMTO_NORMAL, 10000, &dwMsgResult) == 0
			) {
				// エラーかタイムアウト
				break;
			}
			outPos += outLen;
		}
	}
}

/*
	デバッグウィンドウを表示
	@param hwnd 新規ウィンドウのときのデバッグウィンドウの所属グループ
	@param bAllwaysActive 表示済みウィンドウでもアクティブにする
	@return true:表示できた。またはすでに表示されている。
*/
bool ShareData::OpenDebugWindow(HWND hwnd, bool bAllwaysActive)
{
	bool ret = true;
	if (!pShareData->handles.hwndDebug
		|| !IsSakuraMainWindow(pShareData->handles.hwndDebug)
	) {
		// アウトプットウィンドウを作成元と同じグループに作成するために hwndTraceOutSource を使っています
		// （hwndTraceOutSource は CEditWnd::Create() で予め設定）
		// ちょっと不恰好だけど、TraceOut() の引数にいちいち起動元を指定するのも．．．
		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_NONE;
		loadInfo.bViewMode = false;
		ret = ControlTray::OpenNewEditor(
			NULL,
			hwnd,
			loadInfo,
			_T("-DEBUGMODE"),
			true
		);
		// 窓が出るまでウエイトをかけるように修正
		// アウトプットウィンドウが出来るまで5秒ぐらい待つ。
		// OpenNewEditorの同期機能を利用するように変更
		bAllwaysActive = true; // 新しく作ったときはactive
	}
	// 開いているウィンドウをアクティブにする
	if (ret && bAllwaysActive) {
		ActivateFrameWindow(pShareData->handles.hwndDebug);
	}
	return ret;
}

// iniファイルの保存先がユーザ別設定フォルダかどうか
bool ShareData::IsPrivateSettings(void) {
	return pShareData->fileNameManagement.iniFolder.bWritePrivate;
}


/*
	ShareData::CheckMRUandOPENFOLDERList
	MRUとOPENFOLDERリストの存在チェックなど
	存在しないファイルやフォルダはMRUやOPENFOLDERリストから削除する

	@note 現在は使われていないようだ。
*/
/*!	idxで指定したマクロファイル名（フルパス）を取得する．

	@param pszPath [in]	パス名の出力先．長さのみを知りたいときはNULLを入れる．
	@param idx [in]		マクロ番号
	@param nBufLen [in]	pszPathで指定されたバッファのバッファサイズ

	@retval >0 : パス名の長さ．
	@retval  0 : エラー，そのマクロは使えない，ファイル名が指定されていない．
	@retval <0 : バッファ不足．必要なバッファサイズは -(戻り値)+1
	
	@note idxは正確なものでなければならない。(内部で正当性チェックを行っていない)
*/
int ShareData::GetMacroFilename(int idx, TCHAR* pszPath, size_t nBufLen)
{
	if (idx != -1 && !pShareData->common.macro.macroTable[idx].IsEnabled()) {
		return 0;
	}
	const TCHAR* pszFile;

	if (idx == -1) {
		pszFile = _T("RecKey.mac");
	}else {
		pszFile = pShareData->common.macro.macroTable[idx].szFile;
	}
	if (pszFile[0] == _T('\0')) {	// ファイル名が無い
		if (pszPath) {
			pszPath[0] = _T('\0');
		}
		return 0;
	}
	const TCHAR* ptr = pszFile;
	size_t nLen = _tcslen(ptr);

	if (!_IS_REL_PATH(pszFile)	// 絶対パス
		|| pShareData->common.macro.szMACROFOLDER[0] == _T('\0')	// フォルダ指定なし
	) {
		if (!pszPath || nBufLen <= nLen) {
			return -(int)nLen;
		}
		_tcscpy(pszPath, pszFile);
		return (int)nLen;
	}else {	// フォルダ指定あり
		// 相対パス→絶対パス
		int nFolderSep = AddLastChar(pShareData->common.macro.szMACROFOLDER, _countof2(pShareData->common.macro.szMACROFOLDER), _T('\\'));
		TCHAR* pszDir;

		// フォルダも相対パスなら実行ファイルからのパス
		// 相対パスは設定ファイルからのパスを優先
		if (_IS_REL_PATH(pShareData->common.macro.szMACROFOLDER)) {
			TCHAR szDir[_MAX_PATH + _countof2(pShareData->common.macro.szMACROFOLDER)];
			GetInidirOrExedir(szDir, pShareData->common.macro.szMACROFOLDER);
			pszDir = szDir;
		}else {
			pszDir = pShareData->common.macro.szMACROFOLDER;
		}

		size_t nDirLen = _tcslen(pszDir);
		size_t nAllLen = nDirLen + nLen + (nFolderSep == -1 ? 1 : 0);
		if (!pszPath || nBufLen <= nAllLen) {
			return -(int)nAllLen;
		}

		_tcscpy(pszPath, pszDir);
		TCHAR* ptr = pszPath + nDirLen;
		if (nFolderSep == -1) {
			*ptr++ = _T('\\');
		}
		_tcscpy(ptr, pszFile);
		return (int)nAllLen;
	}

}

/*!	idxで指定したマクロのbReloadWhenExecuteを取得する。
	idxは正確なものでなければならない。
*/
bool ShareData::BeReloadWhenExecuteMacro(int idx)
{
	if (!pShareData->common.macro.macroTable[idx].IsEnabled()) {
		return false;
	}
	return pShareData->common.macro.macroTable[idx].bReloadWhenExecute;
}


/*!	@brief 共有メモリ初期化/ツールバー
	ツールバー関連の初期化処理
*/
void ShareData::InitToolButtons(DllSharedData& shareData)
{
	// ツールバーボタン構造体
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	// 新規作成
		25,		// ファイルを開く(DropDown)
		3,		// 上書き保存
		4,		// 名前を付けて保存
		0,

		33,	// 元に戻す(Undo)
		34,	// やり直し(Redo)
		0,

		87,	// 移動履歴: 前へ
		88,	// 移動履歴: 次へ
		0,

		225,	// 検索
		226,	// 次を検索
		227,	// 前を検索
		228,	// 置換
		229,	// 検索マークのクリア
		230,	// Grep
		232,	// アウトライン解析
		0,

		264,	// タイプ別設定一覧
		265,	// タイプ別設定
		266,	// 共通設定
		0,		// 次行のために追加
		346,	// コマンド一覧
	};

	// ツールバーアイコン数の最大値を超えないためのおまじない
	// 最大値を超えて定義しようとするとここでコンパイルエラーになります．
	char dummy[_countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1 : 0];
	dummy[0] = 0;

	memcpy_raw(
		shareData.common.toolBar.nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	// ツールバーボタンの数
	shareData.common.toolBar.nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	shareData.common.toolBar.bToolBarIsFlat = !IsVisualStyle();			// フラットツールバーにする／しない
	
}


/*!	@brief 共有メモリ初期化/ポップアップメニュー

	ポップアップメニューの初期化処理
*/
void ShareData::InitPopupMenu(DllSharedData& shareData)
{
	// カスタムメニュー 規定値
	
	CommonSetting_CustomMenu& menu = pShareData->common.customMenu;

	// 右クリックメニュー
	int n = 0;
	menu.nCustMenuItemFuncArr[0][n] = F_UNDO;
	menu.nCustMenuItemKeyArr [0][n] = 'U';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_REDO;
	menu.nCustMenuItemKeyArr [0][n] = 'R';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_CUT;
	menu.nCustMenuItemKeyArr [0][n] = 'T';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY;
	menu.nCustMenuItemKeyArr [0][n] = 'C';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PASTE;
	menu.nCustMenuItemKeyArr [0][n] = 'P';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_DELETE;
	menu.nCustMenuItemKeyArr [0][n] = 'D';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;
	menu.nCustMenuItemKeyArr [0][n] = 'L';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	menu.nCustMenuItemKeyArr [0][n] = 'H';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PASTEBOX;
	menu.nCustMenuItemKeyArr [0][n] = 'X';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	menu.nCustMenuItemKeyArr [0][n] = 'A';
	++n;

	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	menu.nCustMenuItemKeyArr [0][n] = 'G';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	menu.nCustMenuItemKeyArr [0][n] = 'B';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	menu.nCustMenuItemKeyArr [0][n] = '@';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	menu.nCustMenuItemKeyArr [0][n] = '.';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	menu.nCustMenuItemKeyArr [0][n] = '\\';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	menu.nCustMenuItemKeyArr [0][n] = 'F';
	++n;
	menu.nCustMenuItemNumArr[0] = n;

	// カスタムメニュー１
	menu.nCustMenuItemNumArr[1] = 7;
	menu.nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	menu.nCustMenuItemKeyArr [1][0] = 'O';
	menu.nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	menu.nCustMenuItemKeyArr [1][1] = 'S';
	menu.nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	menu.nCustMenuItemKeyArr [1][2] = 'N';
	menu.nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	menu.nCustMenuItemKeyArr [1][3] = 'L';
	menu.nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	menu.nCustMenuItemKeyArr [1][4] = 'U';
	menu.nCustMenuItemFuncArr[1][5] = F_0;
	menu.nCustMenuItemKeyArr [1][5] = '\0';
	menu.nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	menu.nCustMenuItemKeyArr [1][6] = 'C';

	// タブメニュー
	n = 0;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_GROUPCLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'G';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSEOTHER;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'O';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSELEFT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'H';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSERIGHT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'M';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	++n;
	// TODO: loop
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_1;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_2;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '2';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_3;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '3';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_4;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '4';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_5;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '5';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_6;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '6';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_7;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '7';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_8;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '8';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_9;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '9';
	++n;
	menu.nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}

// 言語選択後に共有メモリ内の文字列を更新する
void ShareData::RefreshString()
{

	RefreshKeyAssignString(*pShareData);
}

void ShareData::CreateTypeSettings()
{
	if (!pvTypeSettings) {
		pvTypeSettings = new std::vector<TypeConfig*>();
	}
}

std::vector<TypeConfig*>& ShareData::GetTypeSettings()
{
	return *pvTypeSettings;
}


void ShareData::InitFileTree(FileTree* setting)
{
	setting->bProject = true;
	for (size_t i=0; i<_countof(setting->items); ++i) {
		FileTreeItem& item = setting->items[i];
		item.eFileTreeItemType = FileTreeItemType::Grep;
		item.szTargetPath = _T("");
		item.szLabelName = _T("");
		item.szTargetPath = _T("");
		item.nDepth = 0;
		item.szTargetFile = _T("");
		item.bIgnoreHidden = true;
		item.bIgnoreReadOnly = false;
		item.bIgnoreSystem = false;
	}
	setting->nItemCount = 1;
	setting->items[0].szTargetPath = _T(".");
	setting->items[0].szTargetFile = _T("*.*");
}
