// printf系ラップ関数群
// 2007.09.20 kobake 作成。
//
// 重要な特徴として、独自のフィールド "%ts" および "%tc" を認識して処理する、という点があります。
// UNICODEビルドでは "%ts", "%tc" はそれぞれ "%ls", %lc" として認識され、
// ANSIビルドでは    "%ts", "%tc" はそれぞれ "%hs", %hc" として認識されます。
//
// "%s", "%c" は使用関数により型が変わり、char, wchar_t が混在するコーディング環境ではバグの元となりやすいので、
// できるだけ、上に記したような明示的な型指定をしたフィールドを用いてください。
//
// 注意：%10ts %.12ts のようなものは未サポート
//
// ++ ++ 改善案 ++ ++
//
// あくまでも標準ライブラリ動作を「ラップ」しているだけなので、
// そのラップ処理分、パフォーマンスは悪いです。
// 標準ライブラリに頼らずに全て自前で実装すれば、標準ライブラリ並みのパフォーマンスが得られるはずです。
//
// ちょっと関数名が分かりにくいので、もっと良い名前募集。
// 今のままだと、上記説明を読まなければ、_tsprintf とかと何が違うの？と思われちゃいそう。。。
//
// プロジェクト全体がTCHARに頼らないのであれば、これらの関数群は不要。
//
#pragma once

// vsprintf_sラップ
int tchar_vsprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v);
int tchar_vsprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v);

// vsprintfラップ
int tchar_vsprintf(char* buf, const char* format, va_list& v);
int tchar_vsprintf(wchar_t* buf, const wchar_t* format, va_list& v);

// vsnprintf_sラップ
int tchar_vsnprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v);
int tchar_vsnprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v);

// sprintf_sラップ
int tchar_sprintf_s(char* buf, size_t nBufCount, const char* format, ...);
int tchar_sprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, ...);

// sprintfラップ
int tchar_sprintf(char* buf, const char* format, ...);
int tchar_sprintf(wchar_t* buf, const wchar_t* format, ...);

// _snprintf_sラップ
int tchar_snprintf_s(char* buf, size_t count, const char* format, ...);
int tchar_snprintf_s(wchar_t* buf, size_t count, const wchar_t* format, ...);

