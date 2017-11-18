#include "StdAfx.h"
#include "CommandLine.h"
#include "mem/Memory.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "debug/RunningTimer.h"
#include "charset/charcode.h"
#include "io/TextStream.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "env/SakuraEnvironment.h"

// コマンドラインオプション用定数
#define CMDLINEOPT_R			1002
#define CMDLINEOPT_NOWIN		1003
#define CMDLINEOPT_WRITEQUIT	1004
#define CMDLINEOPT_GREPMODE		1100
#define CMDLINEOPT_GREPDLG		1101
#define CMDLINEOPT_DEBUGMODE	1999
#define CMDLINEOPT_NOMOREOPT	1998
#define CMDLINEOPT_AT			11
#define CMDLINEOPT_X			1
#define CMDLINEOPT_Y			2
#define CMDLINEOPT_VX			3
#define CMDLINEOPT_VY			4
#define CMDLINEOPT_TYPE			5
#define CMDLINEOPT_CODE			6
#define CMDLINEOPT_SX			7
#define CMDLINEOPT_SY			8
#define CMDLINEOPT_WX			9
#define CMDLINEOPT_WY			10
#define CMDLINEOPT_GKEY			101
#define CMDLINEOPT_GFILE		102
#define CMDLINEOPT_GFOLDER		103
#define CMDLINEOPT_GOPT			104
#define CMDLINEOPT_GCODE		105
#define CMDLINEOPT_M			106
#define CMDLINEOPT_MTYPE		107
#define CMDLINEOPT_GREPR		108
#define CMDLINEOPT_GROUP		500
#define CMDLINEOPT_PROF			501
#define CMDLINEOPT_PROFMGR		502

/*!
	コマンドラインのチェックを行って、オプション番号と
	引数がある場合はその先頭アドレスを返す。
	CommandLine::ParseCommandLine()で使われる。

	@return オプションの番号。どれにも該当しないときは0を返す。
*/
int CommandLine::CheckCommandLine(
	LPTSTR	str,		// [in] 検証する文字列（先頭の-は含まない）
	TCHAR** arg,		// [out] 引数がある場合はその先頭へのポインタ
	int*	arglen		// [out] 引数の長さ
	)
{
	/*!
		コマンドラインオプション解析用構造体配列
	*/
	struct _CmdLineOpt {
		LPCTSTR opt;	// オプション文字列
		int len;		// オプションの文字列長（計算を省くため）
		int value;		// 変換後の値
		bool bLen0;
	};

	/*!
		コマンドラインオプション
		後ろに引数を取らないもの
	*/
	static const _CmdLineOpt _COptWoA[] = {
		{_T("R"),			1,	CMDLINEOPT_R, false},
		{_T("-"),			1,	CMDLINEOPT_NOMOREOPT, false},
		{_T("NOWIN"),		5,	CMDLINEOPT_NOWIN, false},
		{_T("WQ"),			2,	CMDLINEOPT_WRITEQUIT, false},
		{_T("GREPMODE"),	8,	CMDLINEOPT_GREPMODE, false},
		{_T("GREPDLG"),		7,	CMDLINEOPT_GREPDLG, false},
		{_T("DEBUGMODE"),	9,	CMDLINEOPT_DEBUGMODE, false},
		{_T("PROFMGR"),		7,	CMDLINEOPT_PROFMGR, false},
		{NULL, 0, 0}
	};

	/*!
		コマンドラインオプション
		後ろに引数を取るもの
	*/
	static const _CmdLineOpt _COptWithA[] = {
		{_T("@"),		1,			CMDLINEOPT_AT, false},
		{_T("X"),		1,			CMDLINEOPT_X, false},
		{_T("Y"),		1,			CMDLINEOPT_Y, false},
		{_T("VX"),		2,			CMDLINEOPT_VX, false},
		{_T("VY"),		2,			CMDLINEOPT_VY, false},
		{_T("SX"),		2,			CMDLINEOPT_SX, false},
		{_T("SY"),		2,			CMDLINEOPT_SY, false},
		{_T("WX"),		2,			CMDLINEOPT_WX, false},
		{_T("WY"),		2,			CMDLINEOPT_WY, false},
		{_T("CODE"),	4,			CMDLINEOPT_CODE, false},
		{_T("TYPE"),	4,			CMDLINEOPT_TYPE, false},
		{_T("GKEY"),	4,			CMDLINEOPT_GKEY, false},
		{_T("GREPR"),	5,			CMDLINEOPT_GREPR, true},
		{_T("GFILE"),	5,			CMDLINEOPT_GFILE, false},
		{_T("GFOLDER"),	7,			CMDLINEOPT_GFOLDER, false},
		{_T("GOPT"),	4,			CMDLINEOPT_GOPT, false},
		{_T("GCODE"),	5,			CMDLINEOPT_GCODE, false},
		{_T("GROUP"),	5,			CMDLINEOPT_GROUP, false},
		{_T("M"),		1,			CMDLINEOPT_M, false},
		{_T("MTYPE"),	5,			CMDLINEOPT_MTYPE, false},
		{_T("PROF"),	4,			CMDLINEOPT_PROF, true},
		{NULL, 0, 0}
	};

	int len = lstrlen(str);

	// 引数がある場合を先に確認
	for (const auto* ptr = _COptWithA; ptr->opt; ++ptr) {
		if (
			len >= ptr->len		// 長さが足りているか
			&& (str[ptr->len] == '=' || str[ptr->len] == ':')	// オプション部分の長さチェック
			&& auto_memicmp(str, ptr->opt, ptr->len) == 0		// 文字列の比較
		) {
			*arg = str + ptr->len + 1;				// 引数開始位置
			*arglen = len - ptr->len - 1;
			if (**arg == '"') {						// 引数先頭に"があれば削除
				(*arg)++;
				(*arglen)--;
				if (*arglen > 0 && (*arg)[(*arglen) - 1] == '"') {	// 引数末尾に"があれば削除
					(*arg)[(*arglen) - 1] = '\0';
					(*arglen)--;
				}
			}
			if (*arglen <= 0 && !(ptr->bLen0)) {
				return 0;		// 値なしはオプションとして認めない
			}
			return ptr->value;
		}
	}

	// 引数がない場合
	for (const auto* ptr = _COptWoA; ptr->opt; ++ptr) {
		if (
			len == ptr->len									// 長さチェック
			&& auto_memicmp(str, ptr->opt, ptr->len) == 0	// 文字列の比較
		) {
			*arglen = 0;
			return ptr->value;
		}
	}
	return 0;	// 該当無し
}

/*! コマンドラインの解析

	WinMain()から呼び出される。
	
	@note
	これが呼び出された時点では共有メモリの初期化が完了していないため，
	共有メモリにアクセスしてはならない．
*/
void CommandLine::ParseCommandLine(LPCTSTR pszCmdLineSrc, bool bResponse)
{
	MY_RUNNINGTIMER(runningTimer, "CommandLine::Parse");

	// 実行ファイル名をもとに漢字コードを固定する．
	{
		TCHAR exename[512];
		::GetModuleFileName(NULL, exename, _countof(exename));
		wchar_t wexename[512];
		auto_strcpy( wexename, to_wchar(exename) );

		size_t len = wcslen( wexename );
		for (int i=(int)len-1; 0<=i; --i) {
			if (wexename[i] == L'.' ) {
				wexename[i] = L'\0';
				int k = i - 1;
				for (; 0<k && WCODE::Is09(wexename[k]); --k) {}
				if (k < 0 || !WCODE::Is09(wexename[k])) {
					++k;
				}
				if (WCODE::Is09(wexename[k])) {
					EncodingType n = (EncodingType)_wtoi(&wexename[k]);
					if (IsValidCodeOrCPType(n)) {
						fi.nCharCode = n;
					}
				}
				break;
			}
		}
	}


	TCHAR	szPath[_MAX_PATH];
	bool	bFind = false;				// ファイル名発見フラグ
	bool	bParseOptDisabled = false;	// オプション解析を行わなず，ファイル名として扱う
	size_t	nPos;
	size_t	i = 0;
	if (pszCmdLineSrc[0] != _T('-')) {
		for (i=0; i<_countof(szPath); ++i) {
			if (pszCmdLineSrc[i] == _T(' ') || pszCmdLineSrc[i] == _T('\0')) {
				// ファイルの存在をチェック
				szPath[i] = _T('\0');	// 終端文字
				if (fexist(szPath)) {
					bFind = true;
					break;
				}
				if (pszCmdLineSrc[i] == _T('\0')) {
					break;
				}
			}
			szPath[i] = pszCmdLineSrc[i];
		}
	}
	if (bFind) {
		SakuraEnvironment::ResolvePath(szPath);
		_tcscpy(fi.szPath, szPath);	// ファイル名
		nPos = i + 1;
	}else {
		fi.szPath[0] = _T('\0');
		nPos = 0;
	}

	NativeT mResponseFile = _T("");
	assert(lstrlen(pszCmdLineSrc) + 1 != 0);
	std::vector<TCHAR> szCmdLineWork(lstrlen(pszCmdLineSrc) + 1);
	LPTSTR pszCmdLineWork = &szCmdLineWork[0];
	_tcscpy(pszCmdLineWork, pszCmdLineSrc);
	size_t nCmdLineWorkLen = lstrlen(pszCmdLineWork);
	LPTSTR pszToken = my_strtok<TCHAR>(pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" "));
	while (pszToken) {
		DEBUG_TRACE(_T("OPT=[%ts]\n"), pszToken);

		if (bParseOptDisabled || !(pszToken[0] == '-' || pszToken[0] == '"' && pszToken[1] == '-' )) {
			if (pszToken[0] == _T('\"')) {
				NativeT work;
				// 末尾のクォーテーションが無い場合を考慮して，
				// 最後がダブルクォートの場合のみ取り除く
				// ファイル名には使えない文字なのでファイル名に含まれている場合は考慮不要
				// またSHIFT-JISの2バイト目の考慮も不要
				// 引数がダブルクォート1つの場合に，その1つを最初と最後の1つずつと
				// 見間違えて，インデックス-1にアクセスしてしまうのを防ぐために長さをチェックする
				// ファイル名の後ろにあるOptionを解析するため，ループは継続
				int len = lstrlen(pszToken + 1);
				if (len > 0) {
					work.SetString(&pszToken[1], len - (pszToken[len] == _T('"') ? 1 : 0));
					work.Replace(_T("\"\""), _T("\""));
					_tcscpy_s(szPath, work.GetStringPtr());	// ファイル名
				}else {
					szPath[0] = _T('\0');
				}
			}else {
				_tcscpy_s(szPath, pszToken);		// ファイル名
			}

			// 不正なファイル名のままだとファイル保存時ダイアログが出なくなるので
			// 簡単なファイルチェックを行うように修正
			if (_tcsncmp_literal(szPath, _T("file:///")) == 0) {
				_tcscpy(szPath, &(szPath[8]));
			}
			size_t len = _tcslen(szPath);
			for (size_t i=0; i<len; ) {
				if (!TCODE::IsValidFilenameChar(szPath, i)) {
					TCHAR msg_str[_MAX_PATH + 1];
					_stprintf(
						msg_str,
						LS(STR_CMDLINE_PARSECMD1),
						szPath
					);
					MessageBox(NULL, msg_str, _T("FileNameError"), MB_OK);
					szPath[0] = _T('\0');
					break;
				}
				int nChars = t_max(1, int(NativeT::GetCharNext(szPath, len, szPath + i) - (szPath + i)));
				i += nChars;
			}

			if (szPath[0] != _T('\0')) {
				SakuraEnvironment::ResolvePath(szPath);
				if (fi.szPath[0] == _T('\0')) {
					_tcscpy(fi.szPath, szPath );
				}else {
					fileNames.push_back(szPath);
				}
			}
		}else {
			if (*pszToken == '"') {
				++pszToken;	// 先頭の"はスキップ
				size_t tokenlen = _tcslen(pszToken);
				if (pszToken[tokenlen - 1] == '"') {	// 末尾の"を取り除く
					pszToken[tokenlen - 1] = '\0';
				}
			}
			++pszToken;	// 先頭の'-'はskip
			TCHAR* arg = NULL;
			int nArgLen;
			switch (CheckCommandLine(pszToken, &arg, &nArgLen)) {
			case CMDLINEOPT_AT:
				mResponseFile.SetStringT(arg, nArgLen);
				break;
			case CMDLINEOPT_X: // X
				// 行桁指定を1開始にした
				fi.ptCursor.x = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_Y:	// Y
				fi.ptCursor.y = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				// 行桁指定を1開始にした
				fi.nViewLeftCol = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_VY:	// VY
				// 行桁指定を1開始にした
				fi.nViewTopLine = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_SX: // SX
				fi.nWindowSizeX = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_SY:	// SY
				fi.nWindowSizeY = AtoiOptionInt(arg) - 1;
				break;
			case CMDLINEOPT_WX: // WX
				fi.nWindowOriginX = AtoiOptionInt(arg);
				break;
			case CMDLINEOPT_WY:	// WY
				fi.nWindowOriginY = AtoiOptionInt(arg);
				break;
			case CMDLINEOPT_TYPE:	// TYPE
				// ファイルタイプの強制指定
				{
					_tcsncpy(fi.szDocType, arg, MAX_DOCTYPE_LEN);
					fi.szDocType[nArgLen < MAX_DOCTYPE_LEN ? nArgLen : MAX_DOCTYPE_LEN] = L'\0';
				}
				break;
			case CMDLINEOPT_CODE:	// CODE
				fi.nCharCode = (EncodingType)AtoiOptionInt(arg);
				break;
			case CMDLINEOPT_R:	// R
				bViewMode = true;
				break;
			case CMDLINEOPT_NOWIN:	// NOWIN
				bNoWindow = true;
				break;
			case CMDLINEOPT_WRITEQUIT:	// WRITEQUIT
				bWriteQuit = true;
				bNoWindow = true;	// -WQを指定されたら-NOWINも指定されたとして扱う
				break;
			case CMDLINEOPT_GREPMODE:	// GREPMODE
				bGrepMode = true;
				if (fi.szDocType[0] == _T('\0')) {
					auto_strcpy(fi.szDocType , _T("grepout"));
				}
				break;
			case CMDLINEOPT_GREPDLG:	// GREPDLG
				bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	// GKEY
				// 前後の""を取り除く
				gi.mGrepKey.SetStringT(arg,  lstrlen(arg));
				gi.mGrepKey.Replace(L"\"\"", L"\"");
				break;
			case CMDLINEOPT_GREPR:	//	GREPR
				//	前後の""を取り除く
				gi.mGrepRep.SetStringT( arg,  lstrlen( arg ) );
				gi.mGrepRep.Replace( L"\"\"", L"\"" );
				gi.bGrepReplace = true;
				break;
			case CMDLINEOPT_GFILE:	// GFILE
				// 前後の""を取り除く
				gi.mGrepFile.SetStringT(arg,  lstrlen(arg));
				gi.mGrepFile.Replace(_T("\"\""), _T("\""));
				break;
			case CMDLINEOPT_GFOLDER:	// GFOLDER
				gi.mGrepFolder.SetString(arg,  lstrlen(arg));
				gi.mGrepFolder.Replace(_T("\"\""), _T("\""));
				break;
			case CMDLINEOPT_GOPT:	// GOPT
				for (; *arg!='\0' ; ++arg) {
					switch (*arg) {
					case 'X':
						gi.bGrepCurFolder = true;	break;
					case 'U':
						gi.bGrepStdout = true;	break;
					case 'H':
						gi.bGrepHeader = false;	break;
					case 'S':
						// サブフォルダからも検索する
						gi.bGrepSubFolder = true;	break;
					case 'L':
						// 英大文字と英小文字を区別する
						gi.grepSearchOption.bLoHiCase = true;	break;
					case 'R':
						// 正規表現
						gi.grepSearchOption.bRegularExp = true;	break;
					case 'K':
						// 文字コード自動判別
						gi.charEncoding = CODE_AUTODETECT;	break;
					case 'P':
						// 結果出力：[行を出力]/該当部分/否マッチ行
						gi.nGrepOutputLineType = 1;	break;
					case 'N':
						// 結果出力：行を出力/該当部分/[否マッチ行]
						gi.nGrepOutputLineType = 2;	break;
					case 'W':
						// 単語単位で探す
						gi.grepSearchOption.bWordOnly = true;	break;
					case '1':
						// Grep: 出力形式
						gi.nGrepOutputStyle = 1;	break;
					case '2':
						// Grep: 出力形式
						gi.nGrepOutputStyle = 2;	break;
					case '3':
						// Grep: 出力形式
						gi.nGrepOutputStyle = 3;	break;
					case 'F':
						gi.bGrepOutputFileOnly = true;	break;
					case 'B':
						gi.bGrepOutputBaseFolder = true;	break;
					case 'D':
						gi.bGrepSeparateFolder = true;	break;
					case 'C':
						gi.bGrepPaste = true;	break;
					case 'O':
						gi.bGrepBackup = true;	break;
					}
				}
				break;
			case CMDLINEOPT_GCODE:
				gi.charEncoding = (EncodingType)AtoiOptionInt(arg);	break;
			case CMDLINEOPT_GROUP:	// GROUP	// 2007.06.26 ryoji
				nGroup = AtoiOptionInt(arg);
				break;
			case CMDLINEOPT_DEBUGMODE:
				bDebugMode = true;
				if (fi.szDocType[0] == _T('\0')) {
					auto_strcpy(fi.szDocType , _T("output"));
				}
				break;
			case CMDLINEOPT_NOMOREOPT:
				bParseOptDisabled = true;
				break;
			case CMDLINEOPT_M:
				mMacro.SetStringT(arg);
				mMacro.Replace(L"\"\"", L"\"");
				break;
			case CMDLINEOPT_MTYPE:
				mMacroType.SetStringT(arg);
				break;
			case CMDLINEOPT_PROF:
				mProfile.SetStringT( arg );
				bSetProfile = true;
				break;
			case CMDLINEOPT_PROFMGR:
				bProfileMgr = true;
				break;
			}
		}
		pszToken = my_strtok<TCHAR>(pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" "));
	}

	// レスポンスファイル解析
	if (mResponseFile.GetStringLength() && bResponse) {
		TextInputStream input(mResponseFile.GetStringPtr());
		if (!input.Good()) {
			return;
		}
		std::wstring responseData;
		while (input) {
			responseData += input.ReadLineW();
		}
		ParseCommandLine(to_tchar(responseData.c_str()), false);
	}

	return;
}

CommandLine::CommandLine()
{
	bGrepMode				= false;
	bGrepDlg				= false;
	bDebugMode			= false;
	bNoWindow				= false;
	bWriteQuit			= false;
	bProfileMgr			= false;
	bSetProfile			= false;
	gi.bGrepSubFolder		= false;
	gi.grepSearchOption.Reset();
	/*
	gi.sGrepSearchOption.bLoHiCase	= false;
	gi.bGrepRegularExp	= false;
	gi.bGrepWordOnly		= false;
	*/
	gi.bGrepCurFolder		= false;
	gi.bGrepStdout		= false;
	gi.bGrepHeader		= true;
	gi.charEncoding		= CODE_SJIS;
	gi.nGrepOutputLineType	= 0;
	gi.nGrepOutputStyle	= 1;
	gi.bGrepOutputFileOnly	= false;
	gi.bGrepOutputBaseFolder	= false;
	gi.bGrepSeparateFolder	= false;
	gi.bGrepReplace		= false;
	gi.bGrepPaste			= false;
	gi.bGrepBackup		= false;
	bViewMode				= false;
	nGroup				= -1;
	mProfile.SetString(L"");
}

