/*
	_Tマクロ互換のテンプレート。
	ビルド種に関わらず、指定した型の文字定数を提供する。

	_T2(char,'A')
	_T2(wchar_t,'x')
	_T2(TCHAR,'/')
	のように使います。

	テンプレートてんこもりなので、コンパイルが重くなると思われます。
	インクルードは最小限に！

	2007.10.23 kobake 作成
*/

template <class CHAR_TYPE, int CHAR_VALUE>
CHAR_TYPE _TextTemplate();

// 文字定義マクロ
#define DEFINE_T2(CHAR_VALUE) \
template <> char _TextTemplate<char,CHAR_VALUE>() { return ATEXT(CHAR_VALUE); } \
template <> wchar_t _TextTemplate<wchar_t,CHAR_VALUE>() { return LTEXT(CHAR_VALUE); }

// 使用マクロ
#define _T2(CHAR_TYPE, CHAR_VALUE) _TextTemplate<CHAR_TYPE,CHAR_VALUE>()

