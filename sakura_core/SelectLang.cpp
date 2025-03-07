#include "stdafx.h"
#include "SelectLang.h"
#include "util/os.h"
#include "util/module.h"
#include "_os/OsVersionInfo.h"

#include <new>

SelectLang::SelLangInfo* SelectLang::psLangInfo = nullptr;	// メッセージリソース用構造体
SelectLang::PSSelLangInfoList SelectLang::psLangInfoList;

/*!
	@brief デストラクタ

	@note 読み込んだメッセージリソースDLLを解放する
*/
SelectLang::~SelectLang(void)
{
	for (auto it=psLangInfoList.begin(); it!=psLangInfoList.end(); ++it) {
		if ((*it)->hInstance) {
			FreeLibrary((*it)->hInstance);
			(*it)->hInstance = NULL;
		}
		delete *it;
	}
	psLangInfoList.clear();
}

/*!
	@brief メッセージリソースDLLのインスタンスハンドルを返す

	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLをロードしていない場合exeのインスタンスハンドルが返る
*/
HINSTANCE SelectLang::getLangRsrcInstance(void)
{
	return psLangInfo ? psLangInfo->hInstance : GetModuleHandle(NULL);
}

/*!
	@brief メッセージリソースDLL未読み込み時のデフォルト言語の文字列を返す

	@retval デフォルト言語の文字列（"(Japanese)" または "(English(United States))"）

	@note アプリケーションリソースより読み込んだ "(Japanese)" または "(English(United States))"
*/
LPCTSTR SelectLang::getDefaultLangString(void)
{
	return psLangInfo->szLangName;
}

// 言語IDを返す
WORD SelectLang::getDefaultLangId(void)
{
	if (!psLangInfo) {
		return ::GetUserDefaultLangID();
	}
	return psLangInfo->wLangId;
}

/*!
	@brief 言語環境を初期化する
	
	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はexeのインスタンスハンドルが返る
	@note （LoadString()の引数としてそのまま使用するため）
	@note デフォルト言語の文字列の読み込みも行う
	@note プロセス毎にProcessFactoryの最初に1回だけ呼ばれる
*/
HINSTANCE SelectLang::InitializeLanguageEnvironment(void)
{
	SelLangInfo* psLangInfo;

	if (psLangInfoList.size() == 0) {
		// デフォルト情報を作成する
		psLangInfo = new SelLangInfo();
		psLangInfo->hInstance = GetModuleHandle(NULL);

		// 言語情報ダイアログで "System default" に表示する文字列を作成する
		::LoadString( GetModuleHandle(NULL), STR_SELLANG_NAME, psLangInfo->szLangName, _countof(psLangInfo->szLangName) );

		psLangInfoList.push_back(psLangInfo);
	}

	if (psLangInfo
		&& psLangInfo->hInstance
		&& psLangInfo->hInstance != GetModuleHandle(NULL)
	) {
		// 読み込み済みのDLLを解放する
		::FreeLibrary(psLangInfo->hInstance);
		psLangInfo->hInstance = NULL;
		psLangInfo = nullptr;
	}

	// カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CurrentDirectoryBackupPoint curDirBackup;
	ChangeCurrentDirectoryToExeDir();
// ★iniまたはexeフォルダとなるように改造が必要

	WIN32_FIND_DATA w32fd;
	TCHAR szPath[] = _T("sakura_lang_*.dll");			// サーチするメッセージリソースDLL
	HANDLE handle = FindFirstFile(szPath, &w32fd);
	BOOL result = (handle != INVALID_HANDLE_VALUE) ? TRUE : FALSE;

	while (result) {
		if (!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {		// フォルダでない
			// バッファに登録する。
			psLangInfo = new SelLangInfo();
			_tcscpy(psLangInfo->szDllName, w32fd.cFileName);
			psLangInfo->hInstance = SelectLang::LoadLangRsrcLibrary(*psLangInfo);

			if (psLangInfo->hInstance) {
				if (!psLangInfo->bValid) {
					// メッセージリソースDLLとしては無効
					::FreeLibrary(psLangInfo->hInstance);
					psLangInfo->hInstance = NULL;
					delete psLangInfo;
				}else {
					// 有効なメッセージリソースDLL
					// 一旦DLLを解放し、後でChangeLangで再読み込みする
					psLangInfoList.push_back(psLangInfo);
					::FreeLibrary(psLangInfo->hInstance);
					psLangInfo->hInstance = NULL;
				}
			}
		}

		result = FindNextFile(handle, &w32fd);
	}

	if (handle != INVALID_HANDLE_VALUE) {
		FindClose(handle);
		handle = INVALID_HANDLE_VALUE;
	}

	// この時点ではexeのインスタンスハンドルで起動し、共有メモリ初期化後にChangeLangする
	psLangInfo = *psLangInfoList.begin();

	return psLangInfo->hInstance;
}

/*!
	@brief メッセージリソースDLLをロードする
	@retval メッセージリソースDLLのインスタンスハンドル
	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はNULLが返る
*/
HINSTANCE SelectLang::LoadLangRsrcLibrary(SelLangInfo& lang)
{
	if (lang.szDllName[0] == _T('\0')) {
		return NULL;		// DLLが指定されていなければNULLを返す
	}

	int nCount;

	lang.bValid  = FALSE;
	lang.szLangName[0] = _T('\0');
	lang.wLangId = 0;

	HINSTANCE hInstance = LoadLibraryExedir(lang.szDllName);

	if (hInstance) {
		// 言語名を取得
		nCount = ::LoadString(hInstance, STR_SELLANG_NAME, lang.szLangName, _countof(lang.szLangName));

		if (nCount > 0) {
			// 言語IDを取得
			TCHAR szBuf[7];		// "0x" + 4桁 + 番兵
			nCount = ::LoadString(hInstance, STR_SELLANG_LANGID, szBuf, _countof(szBuf));
			szBuf[_countof(szBuf) - 1] = _T('\0');

			if (nCount > 0) {
				lang.wLangId = (WORD)_tcstoul(szBuf, NULL, 16);		// 言語IDを数値化

				if (lang.wLangId > 0) {
					lang.bValid = TRUE;		// メッセージリソースDLLとして有効
				}
			}
		}
	}

	return hInstance;
}


// 文字列リソース読み込み用グローバル
ResourceString::LoadStrBuffer ResourceString::aLoadStrBufferTemp[];	// 文字列読み込みバッファの配列（LoadString::LoadStringSt() が使用する）
int ResourceString::nDataTempArrayIndex = 0;							// 最後に使用したバッファのインデックス（LoadString::LoadStringSt() が使用する）


/*!
	@brief 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note 静的バッファ（aLoadStrBufferTemp[?]）に文字列リソースを読み込む。
	@note バッファは複数準備しているが、呼び出す毎に更新するのでバッファ個数を
	@note 超えて呼び出すと順次内容が失われていく。
	@note 呼び出し直後での使用や関数の引数などでの使用を想定しており、前回値を
	@note 取り出すことはできない。
	@note 使用例）::SetWindowText(hWnd, LoadString::LoadStringSt(STR_ERR_DLGSMCLR1));
	@note アプリケーション内の関数への引数とする場合、その関数が本関数を使用
	@note しているか意識する必要がある（上限を超えれば内容が更新されるため）
	@note 内容を保持したい場合は LoadString::LoadString() を使用する。
*/
LPCTSTR ResourceString::LoadStringSt(UINT uid)
{
	// 使用するバッファの現在位置を進める
	nDataTempArrayIndex = (nDataTempArrayIndex + 1) % _countof(aLoadStrBufferTemp);

	aLoadStrBufferTemp[nDataTempArrayIndex].Load(uid);

	return aLoadStrBufferTemp[nDataTempArrayIndex].GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note メンバ変数内に記憶されるため  LoadString::LoadStringSt() の様に
	@note 不用意に破壊されることはない。
	@note ただし、変数を準備する必要があるのが不便。
	@note 使用例）
	@note   LoadString cStr[2];
	@note   dlgInput1.DoModal(hInstance, hWnd,
	@note       cStr[0].LoadString(STR_ERR_DLGPRNST1),
	@note       cStr[1].LoadString(STR_ERR_DLGPRNST2),
	@note       sizeof(PrintSettingArr[m_nCurrentPrintSetting].szPrintSettingName) - 1, szWork) )
*/
LPCTSTR ResourceString::Load(UINT uid)
{
	loadStrBuffer.Load(uid);

	return loadStrBuffer.GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（読み込み実行部）

	@param[in] uid  リソースID

	@retval 読み込んだ文字数（TCHAR単位）

	@note メッセージリソースより文字列を読み込む。メッセージリソースDLLに指定の
	@note リソースが存在しない、またはメッセージリソースDLL自体が読み込まれて
	@note いない場合、内部リソースより文字列を読み込む。
	@note 最初は静的バッファに読み込むがバッファ不足となったらバッファを拡張
	@note して読み直す。
	@note 取得したバッファはデストラクタで解放する。
	@note ANSI版は2バイト文字の都合により（バッファ - 2）バイトまでしか読まない
	@note 場合があるので1バイト少ない値でバッファ拡張を判定する。
*/
size_t ResourceString::LoadStrBuffer::Load(UINT uid)
{
	if (!pszString) {
		// バッファポインタが設定されていない場合初期化する（普通はあり得ない）
		pszString = szString;					// 変数内に準備したバッファを接続
		nBufferSize = _countof(szString);		// 配列個数
		szString[nBufferSize - 1] = 0;
		nLength = _tcslen(szString);			// 文字数
	}

	HINSTANCE hRsrc = SelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル

	if (!hRsrc) {
		// メッセージリソースDLL読込処理前は内部リソースを使う
		hRsrc = ::GetModuleHandle(NULL);
	}

	size_t nRet = 0;

	while (1) {
		nRet = ::LoadString(hRsrc, uid, pszString, nBufferSize);

		// リソースが無い
		if (nRet == 0) {
			if (hRsrc != ::GetModuleHandle(NULL)) {
				hRsrc = ::GetModuleHandle(NULL);	// 内部リソースを使う
			}else {
				// 内部リソースからも読めなかったら諦める（普通はあり得ない）
				pszString[0] = 0;
				break;
			}
#ifdef UNICODE
		}else if (nRet >= nBufferSize - 1) {
#else
		}else if (nRet >= nBufferSize - 2) {		// ANSI版は1小さい長さで再読み込みを判定する
#endif
			// 読みきれなかった場合、バッファを拡張して読み直す
			size_t nTemp = nBufferSize + LOADSTR_ADD_SIZE;		// 拡張したサイズ
			LPTSTR pTemp;
			try {
				pTemp = new TCHAR[nTemp];
			}catch (std::bad_alloc) {
				// メモリ割り当て例外（例外の発生する環境の場合でも旧来の処理にする）
				pTemp = NULL;
			}

			if (pTemp) {
				if (pszString != szString) {
					delete[] pszString;
				}

				pszString = pTemp;
				nBufferSize = nTemp;
			}else {
				// メモリ取得に失敗した場合は直前の内容で諦める
				nRet = _tcslen(pszString);
				break;
			}
		}else {
			break;		// 文字列リソースが正常に取得できた
		}
	}

	nLength = nRet;	// 読み込んだ文字数

	return nRet;
}

void SelectLang::ChangeLang(TCHAR* pszDllName)
{
	// 言語を選択する
	for (size_t unIndex=0; unIndex<SelectLang::psLangInfoList.size(); ++unIndex) {
		SelectLang::SelLangInfo* psLangInfo = SelectLang::psLangInfoList.at(unIndex);
		if (_tcsncmp(pszDllName, psLangInfo->szDllName, MAX_PATH) == 0) {
			SelectLang::ChangeLang(unIndex);
			break;
		}
	}
}

HINSTANCE SelectLang::ChangeLang(UINT nIndex)
{
	if (psLangInfoList.size() <= nIndex
		|| psLangInfoList.at(nIndex) == psLangInfo
	) {
		return psLangInfo->hInstance;
	}

	SelLangInfo* psLangInfo = psLangInfoList.at(nIndex);
	if (psLangInfo->hInstance != GetModuleHandle(NULL)) {
		psLangInfo->hInstance = LoadLangRsrcLibrary(*psLangInfo);
		if (!psLangInfo->hInstance) {
			return psLangInfo->hInstance;
		}else if (!psLangInfo->bValid) {
			::FreeLibrary(psLangInfo->hInstance);
			psLangInfo->hInstance = NULL;
			return psLangInfo->hInstance;
		}
	}

	if (psLangInfo->hInstance != GetModuleHandle(NULL)) {
		::FreeLibrary(psLangInfo->hInstance);
		psLangInfo->hInstance = NULL;
	}
	SelectLang::psLangInfo = psLangInfo;

	// ロケールを設定
	// SetThreadUILanguageの呼び出しを試みる
	bool isSuccess = false;
	if (OsVersionInfo()._IsWinVista_or_later()) {
		HMODULE hDll = LoadLibrary(_T("kernel32"));
		if (hDll) {
			typedef short (CALLBACK* SetThreadUILanguageType)(LANGID);
			SetThreadUILanguageType _SetThreadUILanguage = (SetThreadUILanguageType)
					GetProcAddress(hDll, "SetThreadUILanguage");
			isSuccess = _SetThreadUILanguage && _SetThreadUILanguage(psLangInfo->wLangId);
			FreeLibrary(hDll);
		}
	}
	if (!isSuccess) {
		SetThreadLocale(MAKELCID(psLangInfo->wLangId, SORT_DEFAULT));
	}

	return psLangInfo->hInstance;
}

