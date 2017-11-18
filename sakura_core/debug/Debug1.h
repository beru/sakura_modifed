#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#if defined(_DEBUG) || defined(USE_RELPRINT)
void DebugOutW(LPCWSTR lpFmt, ...);
void DebugOutA(LPCSTR lpFmt, ...);
#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#define MYTRACE DebugOutW
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
	#define DEBUG_TRACE DebugOutW
#elif (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__)
	#define DEBUG_TRACE(...)
#else
	// Not support C99 variable macro
	inline void DEBUG_TRACE(...) {}
#endif

// RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
	#define RELPRINT DebugOutW
#else
	#define RELPRINT   Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT

