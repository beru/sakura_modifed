#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// アプリ名。2007.09.21 kobake 整理
	#define _APP_NAME_(TYPE) TYPE("sakura")

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(デバッグ版)")
#else
	#define _APP_NAME_2_(TYPE) TYPE("")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) // 例:UNICODEデバッグ→_T("sakura(デバッグ版)")

#define GSTR_APPNAME    (_GSTR_APPNAME_(_T)   )
#define GSTR_APPNAME_A  (_GSTR_APPNAME_(ATEXT))
#define GSTR_APPNAME_W  (_GSTR_APPNAME_(LTEXT))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキストエリア                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta 最大値の定数化と値変更
const size_t LINESPACE_MAX = 128;
const size_t COLUMNSPACE_MAX = 64;

// Aug. 14, 2005 genta 定数定義追加
// 2007.09.07 kobake 定数名変更: MAXLINESIZE→MAXLINEKETAS
// 2007.09.07 kobake 定数名変更: MINLINESIZE→MINLINEKETAS
const size_t MAXLINEKETAS		= 10240;	// 1行の桁数の最大値
const size_t MINLINEKETAS		= 10;		// 1行の桁数の最小値

// 2014.08.02 定数定義追加 katze
const size_t LINENUMWIDTH_MIN = 2;
const size_t LINENUMWIDTH_MAX = 11;

