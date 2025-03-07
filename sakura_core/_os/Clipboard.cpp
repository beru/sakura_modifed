#include "StdAfx.h"
#include <ShellAPI.h>// HDROP
#include "Clipboard.h"
#include "doc/EditDoc.h"
#include "charset/CodeMediator.h"
#include "charset/CodeFactory.h"
#include "charset/ShiftJis.h"
#include "charset/Utf8.h"
#include "Eol.h"

// MinGW<=4.5.0のコンパイルエラー対策
#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Clipboard::Clipboard(HWND hwnd)
{
	hwnd = hwnd;
	bOpenResult = ::OpenClipboard(hwnd);
}

Clipboard::~Clipboard()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Clipboard::Empty()
{
	::EmptyClipboard();
}

void Clipboard::Close()
{
	if (bOpenResult) {
		::CloseClipboard();
		bOpenResult = FALSE;
	}
}

bool Clipboard::SetText(
	const wchar_t*	pData,			// コピーするUNICODE文字列
	size_t			nDataLen,		// pDataの長さ（文字単位）
	bool			bColumnSelect,
	bool			bLineSelect,
	UINT			uFormat
	)
{
	if (!bOpenResult) {
		return false;
	}

	/*
	// テキスト形式のデータ (CF_OEMTEXT)
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextLen + 1
	);
	if (hgClipText) {
		char* pszClip = GlobalLockChar(hgClipText);
		memcpy(pszClip, pszText, nTextLen);
		pszClip[nTextLen] = '\0';
		::GlobalUnlock(hgClipText);
		::SetClipboardData(CF_OEMTEXT, hgClipText);
	}
	*/

	// UNICODE形式のデータ (CF_UNICODETEXT)
	HGLOBAL hgClipText = NULL;
	bool bUnicodeText = (uFormat == (UINT)-1 || uFormat == CF_UNICODETEXT);
	while (bUnicodeText) {
		// 領域確保
		hgClipText = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			(nDataLen + 1) * sizeof(wchar_t)
		);
		if (!hgClipText) {
			break;
		}

		// 確保した領域にデータをコピー
		wchar_t* pszClip = GlobalLockWChar(hgClipText);
		wmemcpy(pszClip, pData, nDataLen);	// データ
		pszClip[nDataLen] = L'\0';				// 終端ヌル
		::GlobalUnlock(hgClipText);

		// クリップボードに設定
		::SetClipboardData(CF_UNICODETEXT, hgClipText);
		bUnicodeText = false;
	}
	// 1回しか通らない. breakでここまで飛ぶ

	// バイナリ形式のデータ
	// (int) 「データ」の長さ
	//「データ」
	HGLOBAL hgClipSakura = NULL;
	// サクラエディタ専用フォーマットを取得
	CLIPFORMAT	uFormatSakuraClip = Clipboard::GetSakuraFormat();
	bool bSakuraText = (uFormat == (UINT)-1 || uFormat == uFormatSakuraClip);
	while (bSakuraText) {
		if (uFormatSakuraClip == 0) {
			break;
		}

		// 領域確保
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(int) + (nDataLen + 1) * sizeof(wchar_t)
		);
		if (!hgClipSakura) {
			break;
		}

		// 確保した領域にデータをコピー
		BYTE* pClip = GlobalLockBYTE(hgClipSakura);
		*((int*)pClip) = (int)nDataLen; pClip += sizeof(int);									// データの長さ
		wmemcpy((wchar_t*)pClip, pData, nDataLen ); pClip += nDataLen * sizeof(wchar_t);	// データ
		*((wchar_t*)pClip) = L'\0'; pClip += sizeof(wchar_t);								// 終端Null
		::GlobalUnlock(hgClipSakura);

		// クリップボードに設定
		::SetClipboardData(uFormatSakuraClip, hgClipSakura);
		bSakuraText = false;
	}
	// 1回しか通らない. breakでここまで飛ぶ

	// 矩形選択を示すダミーデータ
	HGLOBAL hgClipMSDEVColumn = NULL;
	if (bColumnSelect) {
		UINT uFormat = ::RegisterClipboardFormat(_T("MSDEVColumnSelect"));
		if (uFormat != 0) {
			hgClipMSDEVColumn = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if (hgClipMSDEVColumn) {
				BYTE* pClip = GlobalLockBYTE(hgClipMSDEVColumn);
				pClip[0] = 0;
				::GlobalUnlock(hgClipMSDEVColumn);
				::SetClipboardData(uFormat, hgClipMSDEVColumn);
			}
		}
	}

	// 行選択を示すダミーデータ
	HGLOBAL hgClipMSDEVLine = NULL;		// VS2008 以前の形式
	if (bLineSelect) {
		UINT uFormat = ::RegisterClipboardFormat(_T("MSDEVLineSelect"));
		if (uFormat != 0) {
			hgClipMSDEVLine = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if (hgClipMSDEVLine) {
				BYTE* pClip = (BYTE*)::GlobalLock(hgClipMSDEVLine);
				pClip[0] = 0x01;
				::GlobalUnlock(hgClipMSDEVLine);
				::SetClipboardData(uFormat, hgClipMSDEVLine);
			}
		}
	}
	HGLOBAL hgClipMSDEVLine2 = NULL;	// VS2010 形式
	if (bLineSelect) {
		UINT uFormat = ::RegisterClipboardFormat(_T("VisualStudioEditorOperationsLineCutCopyClipboardTag"));
		if (uFormat != 0) {
			hgClipMSDEVLine2 = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if (hgClipMSDEVLine2) {
				BYTE* pClip = (BYTE*)::GlobalLock(hgClipMSDEVLine2);
				pClip[0] = 0x01;	// ※ ClipSpy で調べるとデータはこれとは違うが内容には無関係に動くっぽい
				::GlobalUnlock(hgClipMSDEVLine2);
				::SetClipboardData(uFormat, hgClipMSDEVLine2);
			}
		}
	}

	if (bColumnSelect && !hgClipMSDEVColumn) {
		return false;
	}
	if (bLineSelect && !(hgClipMSDEVLine && hgClipMSDEVLine2)) {
		return false;
	}
	if (!(hgClipText && hgClipSakura)) {
		return false;
	}
	return true;
}

bool Clipboard::SetHtmlText(const NativeW& memBUf)
{
	if (!bOpenResult) {
		return false;
	}

	NativeA memUtf8;
	Utf8().UnicodeToCode(memBUf, memUtf8._GetMemory());

	NativeA memHeader;
	char szFormat[32];
	size_t size = memUtf8.GetStringLength() + 134;
	memHeader.AppendString("Version:0.9\r\n");
	memHeader.AppendString("StartHTML:00000097\r\n");
	sprintf(szFormat, "EndHTML:%08d\r\n", size + 36);
	memHeader.AppendString(szFormat);
	memHeader.AppendString("StartFragment:00000134\r\n");
	sprintf(szFormat, "EndFragment:%08d\r\n", size);
	memHeader.AppendString(szFormat);
	memHeader.AppendString("<html><body>\r\n<!--StartFragment -->\r\n");
	NativeA memFooter;
	memFooter.AppendString("\r\n<!--EndFragment-->\r\n</body></html>\r\n");

	size_t nLen = memHeader.GetStringLength() + memUtf8.GetStringLength() + memFooter.GetStringLength();
	// 領域確保
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nLen + 1
	);
	if (!hgClipText) {
		return false;
	}

	// 確保した領域にデータをコピー
	char* pszClip = GlobalLockChar(hgClipText);
	memcpy_raw(pszClip, memHeader.GetStringPtr(), memHeader.GetStringLength());	// データ
	memcpy_raw(pszClip + memHeader.GetStringLength(), memUtf8.GetStringPtr(), memUtf8.GetStringLength());	// データ
	memcpy_raw(pszClip + memHeader.GetStringLength() + memUtf8.GetStringLength(), memFooter.GetStringPtr(), memFooter.GetStringLength());	// データ
	pszClip[nLen] = '\0';				// 終端ヌル
	::GlobalUnlock(hgClipText);

	// クリップボードに設定
	UINT uFormat = ::RegisterClipboardFormat(_T("HTML Format"));
	::SetClipboardData(uFormat, hgClipText);
	return true;
}

/*! テキストを取得する
	@param [out] pMemBuf 取得したテキストの格納先
	@param [in/out] pbColumnSelect 矩形選択形式
	@param [in/out] pbLineSelect 行選択形式
	@param [in] eol HDROP形式のときの改行コード
	@param [in] uGetFormat クリップボード形式
*/
bool Clipboard::GetText(NativeW* pMemBuf, bool* pbColumnSelect, bool* pbLineSelect, const Eol& eol, UINT uGetFormat)
{
	if (!bOpenResult) {
		return false;
	}
	if (pbColumnSelect) {
		*pbColumnSelect = false;
	}
	if (pbLineSelect) {
		*pbLineSelect = false;
	}

	// 矩形選択や行選択のデータがあれば取得
	if (pbColumnSelect || pbLineSelect) {
		UINT uFormat = 0;
		while ((uFormat = ::EnumClipboardFormats(uFormat)) != 0) {
			// check return value of GetClipboardFormatName
			TCHAR szFormatName[128];
			if (::GetClipboardFormatName(uFormat, szFormatName, _countof(szFormatName) - 1)) {
				if (pbColumnSelect && lstrcmpi(_T("MSDEVColumnSelect"), szFormatName) == 0) {
					*pbColumnSelect = true;
					break;
				}
				if (pbLineSelect && lstrcmpi(_T("MSDEVLineSelect"), szFormatName) == 0) {
					*pbLineSelect = true;
					break;
				}
				if (pbLineSelect && lstrcmpi(_T("VisualStudioEditorOperationsLineCutCopyClipboardTag"), szFormatName) == 0) {
					*pbLineSelect = true;
					break;
				}
			}
		}
	}

	// サクラ形式のデータがあれば取得
	CLIPFORMAT uFormatSakuraClip = Clipboard::GetSakuraFormat();
	if (
		(uGetFormat == -1 || uGetFormat == uFormatSakuraClip)
		&& ::IsClipboardFormatAvailable(uFormatSakuraClip)
	) {
		HGLOBAL hSakura = ::GetClipboardData(uFormatSakuraClip);
		if (hSakura) {
			BYTE* pData = (BYTE*)::GlobalLock(hSakura);
			size_t nLength        = *((int*)pData);
			const wchar_t* szData = (const wchar_t*)(pData + sizeof(int));
			pMemBuf->SetString(szData, nLength);
			::GlobalUnlock(hSakura);
			return true;
		}
	}

	// UNICODE形式のデータがあれば取得
	HGLOBAL hUnicode = NULL;
	if (uGetFormat == -1 || uGetFormat == CF_UNICODETEXT) {
		hUnicode = ::GetClipboardData(CF_UNICODETEXT);
	}
	if (hUnicode) {
		//DWORD nLen = GlobalSize(hUnicode);
		wchar_t* szData = GlobalLockWChar(hUnicode);
		pMemBuf->SetString(szData);
		::GlobalUnlock(hUnicode);
		return true;
	}

	// OEMTEXT形式のデータがあれば取得
	HGLOBAL hText = NULL;
	if (uGetFormat == -1 || uGetFormat == CF_OEMTEXT) {
		hText = ::GetClipboardData(CF_OEMTEXT);
	}
	if (hText) {
		char* szData = GlobalLockChar(hText);
		// SJIS→UNICODE
		Memory memSjis(szData, GlobalSize(hText));
		NativeW memUni;
		ShiftJis::SJISToUnicode(memSjis, &memUni);
		memSjis.Clean();
		// '\0'までを取得
		memUni._SetStringLength(auto_strlen(memUni.GetStringPtr()));
		memUni.swap(*pMemBuf);
		::GlobalUnlock(hText);
		return true;
	}

	// HDROP形式のデータがあれば取得
	if (
		(uGetFormat == -1 || uGetFormat == CF_HDROP)
		&& ::IsClipboardFormatAvailable(CF_HDROP)
	) {
		HDROP hDrop = (HDROP)::GetClipboardData(CF_HDROP);
		if (hDrop) {
			TCHAR sTmpPath[_MAX_PATH + 1] = {0};
			const UINT nMaxCnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			for (UINT nLoop=0; nLoop<nMaxCnt; ++nLoop) {
				DragQueryFile(hDrop, nLoop, sTmpPath, _countof(sTmpPath) - 1);
				// ANSI版に合わせて最終行にも改行コードをつける
				pMemBuf->AppendStringT(sTmpPath);
				if (nMaxCnt > 1) {
					pMemBuf->AppendString(eol.GetValue2());
				}
			}
			return true;
		}
	}
	return false;
}


struct SystemClipFormatNames {
	CLIPFORMAT nClipFormat;
	const wchar_t* pszName;
};

static const SystemClipFormatNames sClipFormatNames[] = {
	{CF_TEXT        ,L"CF_TEXT"},
	{CF_BITMAP      ,L"CF_BITMAP"},
	{CF_METAFILEPICT,L"CF_METAFILEPICT"},
	{CF_SYLK        ,L"CF_SYLK"},
	{CF_DIF         ,L"CF_DIF"},
	{CF_TIFF        ,L"CF_TIFF"},
	{CF_OEMTEXT     ,L"CF_OEMTEXT"},
	{CF_DIB         ,L"CF_DIB"},
	{CF_PALETTE     ,L"CF_PALETTE"},
	{CF_PENDATA     ,L"CF_PENDATA"},
	{CF_RIFF        ,L"CF_RIFF"},
	{CF_WAVE        ,L"CF_WAVE"},
	{CF_UNICODETEXT ,L"CF_UNICODETEXT"},
	{CF_ENHMETAFILE ,L"CF_ENHMETAFILE"},
	{CF_HDROP       ,L"CF_HDROP"},
	{CF_LOCALE      ,L"CF_LOCALE"},
	{CF_DIBV5       ,L"CF_DIBV5"},
};

static CLIPFORMAT GetClipFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = (CLIPFORMAT)-1;
	if (pFormatName[0] == L'\0') {
		return uFormat;
	}
	for (size_t i=0; i<_countof(sClipFormatNames); ++i) {
		if (wcsicmp(pFormatName, sClipFormatNames[i].pszName) == 0) {
			uFormat = sClipFormatNames[i].nClipFormat;
		}
	}
	if (uFormat == (CLIPFORMAT)-1) {
		bool bNumber = true;
		for (size_t i=0; pFormatName[i]; ++i) {
			if (!WCODE::Is09(pFormatName[i])) {
				bNumber = false;
			}
		}
		if (bNumber) {
			uFormat = _wtoi(pFormatName);
		}else {
			uFormat = ::RegisterClipboardFormat(to_tchar(pFormatName));
		}
	}
	return uFormat;
}

bool Clipboard::IsIncludeClipboradFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if (::IsClipboardFormatAvailable(uFormat)) {
		return true;
	}
	return false;
}

static int GetEndModeByMode(int nMode, int nEndMode)
{
	if (nEndMode == -1) {
		switch (nMode) {
		case -1:                nEndMode = 0; break;
		case CODE_AUTODETECT:   nEndMode = 0; break;
		case CODE_SJIS:         nEndMode = 1; break;
		case CODE_EUC:          nEndMode = 1; break;
		case CODE_LATIN1:       nEndMode = 1; break;
		case CODE_UNICODE:      nEndMode = 2; break;
		case CODE_UNICODEBE:    nEndMode = 2; break;
		case CODE_UTF8:         nEndMode = 1; break;
		case CODE_CESU8:        nEndMode = 1; break;
		case CODE_UTF7:         nEndMode = 1; break;
		default:                nEndMode = 0; break;
		}
	}
	return nEndMode;
}

bool Clipboard::SetClipboradByFormat(
	const StringRef& str,
	const wchar_t* pFormatName,
	int nMode,
	int nEndMode
	)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if (uFormat == (CLIPFORMAT)-1) {
		return false;
	}
	if (nMode == -2) {
		if (uFormat == CF_UNICODETEXT || uFormat == GetSakuraFormat()) {
			return SetText(str.GetPtr(), str.GetLength(), false, false, uFormat);
		}
		return false;
	}

	Memory memBuf;
	char* pBuf = NULL;
	size_t nTextByteLen = 0;
	if (nMode == -1) {
		// バイナリモード U+00 - U+ffを0x00 - 0xffにマッピング
		memBuf.AllocBuffer(str.GetLength());
		memBuf._SetRawLength(str.GetLength());
		pBuf = (char*)memBuf.GetRawPtr();
		size_t len = str.GetLength();
		const wchar_t* pMem = str.GetPtr();
		for (size_t i=0; i<len; ++i) {
			pBuf[i] = (unsigned char)pMem[i];
			if (0xff < pMem[i]) {
				return false;
			}
		}
		nTextByteLen = len;
 	}else {
		EncodingType eMode = (EncodingType)nMode;
		if (!IsValidCodeType(eMode)) {
			return false;
		}
		if (eMode == CODE_UNICODE) {
			pBuf = (char*)str.GetPtr();
			nTextByteLen = str.GetLength() * sizeof(wchar_t);
		}else {
			CodeBase* pCode = CodeFactory::CreateCodeBase(eMode, GetDllShareData().common.file.GetAutoMIMEdecode());
			if (pCode->UnicodeToCode(str, &memBuf) == CodeConvertResult::Failure) {
				return false;
			}
			delete pCode;
			pBuf = (char*)memBuf.GetRawPtr();
			nTextByteLen = memBuf.GetRawLength();
		}
 	}
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nulLen = 0;
	switch (nEndMode) {
	case 1: nulLen = 1; break;
	case 2: nulLen = 2; break;
	case 4: nulLen = 4; break;
	case 0: nulLen = 0; break;
	default: nulLen = 0; break;
	}
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextByteLen + nulLen
	);
	if (!hgClipText) {
		return false;
	}
	char* pszClip = GlobalLockChar(hgClipText);
	memcpy(pszClip, pBuf, nTextByteLen);
	if (nulLen) {
		memset(&pszClip[nTextByteLen], 0, nulLen);
	}
	::GlobalUnlock(hgClipText);
	::SetClipboardData(uFormat, hgClipText);

	return true;
}

static size_t GetLengthByMode(
	HGLOBAL hClipData,
	const BYTE* pData,
	int nMode,
	int nEndMode
	)
{
	size_t nMemLength = ::GlobalSize(hClipData);
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nLength;
	if (nEndMode == 1) {
		nLength = strnlen((const char*)pData, nMemLength);
	}else if (nEndMode == 2) {
		nLength = wcsnlen((const wchar_t*)pData, nMemLength / 2) * 2;
	}else if (nEndMode == 4) {
		const wchar32_t* pData32 = (const wchar32_t*)pData;
		const size_t len = nMemLength / 4;
		nLength = 0;
		while (pData32[nLength] != 0 && nLength < len) {
			++nLength;
		}
		nLength *= 4;
	}else {
		nLength = nMemLength;
	}
	return nLength;
}

/*!
	指定のクリップボード形式で取得
	
	@param nMode -2:通常のサクラの処理, -1:バイナリモード, それ以外:文字コード
	@param nEndMode -1:文字コードに依存 0:GlobalSize 1:strlen 2:wcslen 4:wchar32_tの文字列
*/
bool Clipboard::GetClipboradByFormat(
	NativeW& mem,
	const wchar_t* pFormatName,
	int nMode,
	int nEndMode,
	const Eol& eol
	)
{
	mem.SetString(L"");
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if (uFormat == (CLIPFORMAT)-1) {
		return false;
	}
	if (!::IsClipboardFormatAvailable(uFormat)) {
		return false;
	}
	if (nMode == -2) {
		bool bret = false;
		if (GetDataType() != -1) {
			bret = GetText(&mem, NULL, NULL, eol, uFormat);
			if (!bret) {
				mem.SetString(L"");
			}
		}
		return bret;
	}
	HGLOBAL hClipData = ::GetClipboardData(uFormat);
	if (hClipData) {
		bool retVal = true;
		const BYTE* pData = (BYTE*)::GlobalLock(hClipData);
		if (!pData) {
			return false;
		}

		// 長さオプションの解釈
		size_t nLength = GetLengthByMode(hClipData, pData, nMode, nEndMode);

		// エンコードオプション
		if (nMode == -1) {
			// バイナリモード。1byteをU+00-U+ffにマッピング
			mem.AllocStringBuffer(nLength);
			mem._SetStringLength(nLength);
			wchar_t* pBuf = (wchar_t*)mem.GetStringPtr();
			for (size_t i=0; i<nLength; ++i) {
				pBuf[i] = (unsigned char)pData[i];
			}
		}else {
			EncodingType eMode = (EncodingType)nMode;
			if (!IsValidCodeType(eMode)) {
				// コード不明と99は自動判別
				EncodingType nBomCode = CodeMediator::DetectUnicodeBom((const char*)pData, nLength);
				if (nBomCode != CODE_NONE) {
					eMode = nBomCode;
				}else {
					const TypeConfig& type = EditDoc::GetInstance(0)->docType.GetDocumentAttribute();
					CodeMediator mediator(type.encoding);
					eMode = mediator.CheckKanjiCode((const char*)pData, nLength);
				}
				if (!IsValidCodeType(eMode)) {
					eMode = CODE_DEFAULT;
				}
				if (nEndMode == -1) {
					// nLength 再設定
					nLength = GetLengthByMode(hClipData, pData, eMode, nEndMode);
				}
			}
			if (eMode == CODE_UNICODE) {
				mem.SetString((wchar_t*)pData, nLength / sizeof(wchar_t));
			}else {
				Memory tmpMem;
				tmpMem.SetRawData(pData, nLength);
				if (tmpMem.GetRawPtr()) {
					CodeBase* pCode = CodeFactory::CreateCodeBase(eMode, GetDllShareData().common.file.GetAutoMIMEdecode());
					if (pCode->CodeToUnicode(tmpMem, &mem) == CodeConvertResult::Failure) {
						mem.SetString(L"");
						retVal = false;
					}
					delete pCode;
				}
			}
		}
		::GlobalUnlock(hClipData);
		return retVal;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// クリップボード内に、サクラエディタで扱えるデータがあればtrue
bool Clipboard::HasValidData()
{
	// 扱える形式が１つでもあればtrue
	if (::IsClipboardFormatAvailable(CF_OEMTEXT)) return true;
	if (::IsClipboardFormatAvailable(CF_UNICODETEXT)) return true;
	if (::IsClipboardFormatAvailable(GetSakuraFormat())) return true;
	if (::IsClipboardFormatAvailable(CF_HDROP)) return true;
	return false;
}

// サクラエディタ独自のクリップボードデータ形式
CLIPFORMAT Clipboard::GetSakuraFormat()
{
	/*
		UNICODE形式でクリップボードデータを保持するよう変更したため、
		以前のバージョンのクリップボードデータと競合しないように
		フォーマット名を変更
	*/
	return (CLIPFORMAT)::RegisterClipboardFormat(_T("SAKURAClipW"));
}

// クリップボードデータ形式(CF_UNICODETEXT等)の取得
int Clipboard::GetDataType()
{
	// 扱える形式が１つでもあればtrue
	// GetTextの取得順に
	if (::IsClipboardFormatAvailable(GetSakuraFormat())) return GetSakuraFormat();
	if (::IsClipboardFormatAvailable(CF_UNICODETEXT)) return CF_UNICODETEXT;
	if (::IsClipboardFormatAvailable(CF_OEMTEXT)) return CF_OEMTEXT;
	if (::IsClipboardFormatAvailable(CF_HDROP)) return CF_HDROP;
	return -1;
}

