// stdafx.h : 標準のシステム インクルード ファイル、
//				または参照回数が多く、かつあまり変更されない
//				プロジェクト専用のインクルード ファイルを記述します。
//

#pragma once

// この位置にヘッダーを挿入してください
// #define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します
#ifndef STRICT
#define STRICT 1
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400

//#pragma warning(disable: 4786)
#pragma warning(disable: 4345)	//warning C4345: 動作変更 : 形式 () の初期化子で構築される POD 型のオブジェクトは既定初期化されます。
#pragma warning(disable: 4996)	//warning C4996: 'xxxx': This function or variable may be unsafe. Consider using wcscpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#pragma warning(disable: 4355)	//warning C4355: 'this' : ベース メンバ初期化リストで使用されました。

#pragma warning(error: 6244)
#pragma warning(error: 6246)

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#ifdef __MINGW32__
#include <_mingw.h>
#ifdef MINGW_HAS_SECURE_API
#undef MINGW_HAS_SECURE_API
#endif  // MINGW_HAS_SECURE_API
#endif  // __MINGW32__
// グローバル
#include "_main/global.h"

// ビルドオプション的なヘッダ
#include "config/build_config.h"

// 定数(プリコンパイル日付に依存)
#include "config/system_constants.h"	//システム定数
#include "config/app_constants.h"		//アプリケーション定数

// 高頻度API等
// #include <CommDlg.h> // WIN32_LEAN_AND_MEANでは必要。OpenFileDialg系
#include <CommCtrl.h> // コモンコントロール
#include <stdlib.h>  // _MAX_PATH
#include <wchar.h>

#ifndef SAKURA_PCH_MODE_MIN
#include <HtmlHelp.h>
#include <ObjIdl.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#endif // ifndef SAKURA_PCH_MODE_MIN

#if defined(__MINGW32__) && defined(_countof)
#define BUILD_OPT_NEW_HEADERS
#endif

// デバッグ
#include "debug/Debug1.h"
#include "debug/Debug2.h"
#include "debug/Debug3.h"
#include "debug/trace.h"

// シンプルでよく使うもの
#include "basis/primitive.h"
#include "util/std_macro.h"

// MFC互換
#include "basis/MyString.h"
#include "basis/MyRect.h"
#include "basis/MyPoint.h"
#include "basis/MySize.h"

// サクラエディタ固有型
#include "basis/SakuraBasis.h"

// よく使うヘッダ
#include "mem/NativeW.h"
#include "mem/NativeA.h"
#include "mem/NativeT.h"

#include "util/string_ex.h"
#include "util/MessageBoxF.h"
#include "SelectLang.h"
#include "String_define.h"

// その他
#define malloc_char (char*)malloc
#define GlobalLockChar  (char*)::GlobalLock
#define GlobalLockUChar (unsigned char*)::GlobalLock
#define GlobalLockWChar (wchar_t*)::GlobalLock
#define GlobalLockBYTE  (BYTE*)::GlobalLock

// APIラップ
#include "apiwrap/StdControl.h"
#include "apiwrap/CommonControl.h"
#include "apiwrap/StdApi.h"

// TCHARユーティリティ
#include "util/tchar_convert.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"

#ifndef SAKURA_PCH_MODE_MIN
#define SAKURA_PCH_MODE_DLLSHARE 1
#endif

#if defined(SAKURA_PCH_MODE_MAX) || defined(SAKURA_PCH_MODE_DLLSHARE)
#include "env/DllSharedData.h"
#endif

#ifdef SAKURA_PCH_MODE_MAX
#include "env/ShareData.h"
#include "_main/NormalProcess.h"
#include "_main/AppMode.h"
#include "window/EditWnd.h"
#include "EditApp.h"
#include "doc/DocReader.h"
#include "docplus/ModifyManager.h"
#include "docplus/DiffManager.h"
#include "docplus/BookmarkManager.h"
#include "ReadManager.h"
#include "WriteManager.h"
#include "SearchAgent.h"
//###########超仮
#include "uiparts/Graphics.h"
#endif // SAKURA_PCH_MODE_MAX

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

