#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字コードセット種別
enum EncodingType {
	CODE_SJIS,						// SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	CODE_JIS,						// JIS				(MS-CP5022x(ISO-2022-JP-MS)ではない)
	CODE_EUC,						// EUC				(MS-CP51932, eucJP-ms(eucJP-open)ではない)
	CODE_UNICODE,					// Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						// UTF-8(UCS-2)
	CODE_UTF7,						// UTF-7(UCS-2)
	CODE_UNICODEBE,					// Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CESU8,						// CESU-8
	CODE_LATIN1,					// Latin1				(Latin1, 欧文, Windows-1252, Windows Codepage 1252 West European)
	CODE_CODEMAX,
	CODE_CPACP      = 90,
	CODE_CPOEM      = 91,
	CODE_AUTODETECT	= 99,			// 文字コード自動判別
	CODE_ERROR      = -1,			// エラー
	CODE_NONE       = -1,			// 未検出
	CODE_DEFAULT    = CODE_SJIS,	// デフォルトの文字コード
	/*
		- MS-CP50220 
			Unicode から cp50220 への変換時に、
			JIS X 0201 片仮名は JIS X 0208 の片仮名に置換される
		- MS-CP50221
			Unicode から cp50221 への変換時に、
			JIS X 0201 片仮名は、G0 集合への指示のエスケープシーケンス ESC (I を用いてエンコードされる
		- MS-CP50222
			Unicode から cp50222 への変換時に、
			JIS X 0201 片仮名は、SO/SI を用いてエンコードされる
		
		参考
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           判定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 有効な文字コードセットならtrue
bool IsValidCodeType(int code);

// 有効な文字コードセットならtrue。ただし、SJISは除く(ファイル一覧に文字コードを[]付きで表示のため)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code != CODE_SJIS;
}

inline bool IsValidCodePageEx(int code)
{
	return code == 12000
		|| code == 12001
		|| ::IsValidCodePage(code);
}

void InitCodeSet();
inline bool IsValidCodeOrCPType(int code)
{
	return IsValidCodeType(code) || code == CODE_CPACP || code == CODE_CPOEM || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}
inline bool IsValidCodeOrCPTypeExceptSJIS(int code)
{
	return IsValidCodeTypeExceptSJIS(code) || code == CODE_CPACP || code == CODE_CPOEM || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CodeTypeName {
public:
	CodeTypeName(EncodingType eCodeType) : eCodeType(eCodeType) { InitCodeSet(); }
	CodeTypeName(int eCodeType) : eCodeType((EncodingType)eCodeType) { InitCodeSet(); }
	EncodingType GetCode() const { return eCodeType; }
	LPCTSTR	Normal() const;
	LPCTSTR	Short() const;
	LPCTSTR	Bracket() const;
	bool	UseBom();
	bool	CanDefault();
	bool	IsBomDefOn();
private:
	EncodingType eCodeType;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      コンボボックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CodeTypesForCombobox {
public:
	CodeTypesForCombobox() { InitCodeSet(); }
	size_t			GetCount() const;
	EncodingType	GetCode(size_t nIndex) const;
	LPCTSTR		GetName(size_t nIndex) const;
};

