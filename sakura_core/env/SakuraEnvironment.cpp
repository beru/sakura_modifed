#include "StdAfx.h"
#include "SakuraEnvironment.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/FormatManager.h"
#include "env/FileNameManager.h"
#include "_main/AppMode.h"
#include "_main/CommandLine.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "print/PrintPreview.h"
#include "macro/SMacroMgr.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/string_ex2.h"
#include "util/module.h" // GetAppVersionInfo
#include "util/shell.h"
#include "util/window.h"

typedef std::wstring wstring;

EditWnd& SakuraEnvironment::GetMainWindow()
{
	return EditWnd::getInstance();
}

enum EExpParamName
{
	EExpParamName_none = -1,
	EExpParamName_begin = 0,
	EExpParamName_profile = 0,
	EExpParamName_end
};

struct ExpParamName
{
	const wchar_t* szName;
	int nLen;
};
static ExpParamName SExpParamNameTable[] = {
	{L"profile", 7},
	{NULL, 0}
};
wchar_t* ExParam_LongName( wchar_t* q, wchar_t* q_max, EExpParamName eLongParam );

/*!	$xの展開

	特殊文字は以下の通り
	@li $  $自身
	@li A  アプリ名
	@li F  開いているファイルのフルパス。名前がなければ(無題)。
	@li f  開いているファイルの名前（ファイル名+拡張子のみ）
	@li g  開いているファイルの名前（拡張子除く）
	@li /  開いているファイルの名前（フルパス。パスの区切りが/）
	@li N  開いているファイルの名前(簡易表示)
	@li n  無題の通し番号
	@li E  開いているファイルのあるフォルダの名前(簡易表示)
	@li e  開いているファイルのあるフォルダの名前
	@li B  タイプ別設定の名前
	@li b  開いているファイルの拡張子
	@li Q  印刷ページ設定の名前
	@li C  現在選択中のテキスト
	@li x  現在の物理桁位置(先頭からのバイト数1開始)
	@li y  現在の物理行位置(1開始)
	@li d  現在の日付(共通設定の日付書式)
	@li t  現在の時刻(共通設定の時刻書式)
	@li p  現在のページ
	@li P  総ページ
	@li D  ファイルのタイムスタンプ(共通設定の日付書式)
	@li T  ファイルのタイムスタンプ(共通設定の時刻書式)
	@li V  エディタのバージョン文字列
	@li h  Grep検索キーの先頭32byte
	@li S  サクラエディタのフルパス
	@li I  iniファイルのフルパス
	@li M  現在実行しているマクロファイルパス
	@li <profile> プロファイル名
*/
void SakuraEnvironment::ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen)
{
	const EditDoc* pDoc = EditDoc::GetInstance(0); //###

	const wstring	PRINT_PREVIEW_ONLY		= LSW(STR_PREVIEW_ONLY);	// L"(印刷Previewでのみ使用できます)";
	const size_t	PRINT_PREVIEW_ONLY_LEN	= PRINT_PREVIEW_ONLY.length();
	const wstring	NO_TITLE				= LSW(STR_NO_TITLE1);	// L"(無題)";
	const size_t	NO_TITLE_LEN			= NO_TITLE.length();
	const wstring	NOT_SAVED				= LSW(STR_NOT_SAVED);	// L"(保存されていません)";
	const size_t	NOT_SAVED_LEN			= NOT_SAVED.length();

	const wchar_t *p, *r;	// p：目的のバッファ。r：作業用のポインタ。
	wchar_t *q, *q_max;

	for (p=pszSource, q=pszBuffer, q_max=pszBuffer+nBufferLen; *p!='\0' && q<q_max;) {
		if (*p != '$') {
			*q++ = *p++;
			continue;
		}
		switch (*(++p)) {
		case L'$':	// $$ -> $
			*q++ = *p++;
			break;
		case L'A':	// アプリ名
			q = wcs_pushW(q, q_max - q, GSTR_APPNAME_W, wcslen(GSTR_APPNAME_W));
			++p;
			break;
		case L'F':	// 開いているファイルの名前（フルパス）
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
				++p;
			}else {
				r = to_wchar(pDoc->docFile.GetFilePath());
				q = wcs_pushW(q, q_max - q, r, wcslen(r));
				++p;
			}
			break;
		case L'f':	// 開いているファイルの名前（ファイル名+拡張子のみ）
			// ファイル名のみを渡すバージョン
			// ポインタを末尾に
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
				++p;
			}else {
				// ファイル名(パスなし)を取得。日本語対応
				// 万一\\が末尾にあってもその後ろには\0があるのでアクセス違反にはならない。
				q = wcs_pushT(q, q_max - q, pDoc->docFile.GetFileName());
				++p;
			}
			break;
		case L'g':	// 開いているファイルの名前（拡張子を除くファイル名のみ）
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
				++p;
			}else {
				// ポインタを末尾に
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(pDoc->docFile.GetFileName()); // ファイル名(パスなし)を取得。日本語対応
				end_of_path = dot_position = r + wcslen(r);
				// 後ろから.を探す
				for (--dot_position; dot_position>r && *dot_position!='.'; --dot_position)
					;
				// rと同じ場所まで行ってしまった⇔.が無かった
				if (dot_position == r)
					dot_position = end_of_path;

				q = wcs_pushW(q, q_max - q, r, dot_position - r);
				++p;
			}
			break;
		case L'/':	// 開いているファイルの名前（フルパス。パスの区切りが/）
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
				++p;
			}else {
				// パスの区切りとして'/'を使うバージョン
				for (r=to_wchar(pDoc->docFile.GetFilePath()); *r!=L'\0' && q<q_max; ++r, ++q) {
					if (*r == L'\\')
						*q = L'/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		case L'N':	// 開いているファイルの名前(簡易表示)
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
				++p;
			}else {
				TCHAR szText[1024];
				NONCLIENTMETRICS met;
				met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
				::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
				DCFont dcFont(met.lfCaptionFont, GetMainWindow().GetHwnd());
				FileNameManager::getInstance().GetTransformFileNameFast( pDoc->docFile.GetFilePath(), szText, _countof(szText)-1, dcFont.GetHDC(), true );
				q = wcs_pushT(q, q_max - q, szText);
				++p;
			}
			break;
		case L'n':
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				if (EditApp::getInstance().pGrepAgent->bGrepMode) {
				}else if (AppMode::getInstance().IsDebugMode()) {
				}else {
					wchar_t szText[10];
					const EditNode* node = AppNodeManager::getInstance().GetEditNode(GetMainWindow().GetHwnd());
					if (0 < node->nId) {
						swprintf(szText, L"%d", node->nId);
						q = wcs_pushW(q, q_max - q, szText);
					}
				}
			}
			++p;
			break;
		case L'E':	// 開いているファイルのあるフォルダの名前(簡易表示)
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
			}else {
				wchar_t buff[_MAX_PATH];		// \の処理をする為wchar_t
				wchar_t* pEnd;
				wchar_t* p;

				wcscpy_s(buff, _MAX_PATH, to_wchar(pDoc->docFile.GetFilePath()));
				pEnd = NULL;
				for (p=buff; *p!='\0'; ++p) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				if (pEnd) {
					// 最後の\の後で終端
					*(pEnd + 1) = '\0';
				}

				// 簡易表示に変換
				TCHAR szText[1024];
				NONCLIENTMETRICS met;
				met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
				::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
				DCFont dcFont(met.lfCaptionFont, GetMainWindow().GetHwnd());
				FileNameManager::getInstance().GetTransformFileNameFast( to_tchar(buff), szText, _countof(szText)-1, dcFont.GetHDC(), true );
				q = wcs_pushT(q, q_max - q, szText);
			}
			++p;
			break;
		case L'e':	// 開いているファイルのあるフォルダの名前
			if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
				q = wcs_pushW(q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN);
			}else {
				const wchar_t* pStr;
				const wchar_t* pEnd;
				const wchar_t* p;
				
				pStr = to_wchar(pDoc->docFile.GetFilePath());
				pEnd = pStr - auto_strlen(pStr) - 1;
				for (p=pStr; *p!='\0'; ++p) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				q = wcs_pushW(q, q_max - q, pStr, pEnd - pStr + 1);
			}
			++p;
			break;
		case L'B':	// タイプ別設定の名前
			{
				const TypeConfig& typeCongig = pDoc->docType.GetDocumentAttribute();
				if (typeCongig.nIdx > 0) {	// 基本は表示しない
					q = wcs_pushT(q, q_max - q, typeCongig.szTypeName);
				}
				++p;
			}
			break;
		case L'b':	// 開いているファイルの拡張子
			if (pDoc->docFile.GetFilePathClass().IsValidPath()) {
				// ポインタを末尾に
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(pDoc->docFile.GetFileName());
				end_of_path = dot_position = r + wcslen(r);
				// 後ろから.を探す
				while (--dot_position >= r && *dot_position != L'.')
					;
				// .を発見(拡張子有り)
				if (*dot_position == L'.') {
					q = wcs_pushW(q, q_max - q, dot_position +1, end_of_path - dot_position -1);
				}
			}
			++p;
			break;
		case L'Q':	// 印刷ページ設定の名前
			{
				PrintSetting* ps = &GetDllShareData().printSettingArr[
					 pDoc->docType.GetDocumentAttribute().nCurrentPrintSetting];
				q = wcs_pushT(q, q_max - q, ps->szPrintSettingName);
				++p;
			}
			break;
		case L'C':	// 現在選択中のテキスト
			{
				NativeW memCurText;
				GetMainWindow().GetActiveView().GetCurrentTextForSearch(memCurText);

				q = wcs_pushW(q, q_max - q, memCurText.GetStringPtr(), memCurText.GetStringLength());
				++p;
			}
			break;
		case L'x':	// 現在の物理桁位置(先頭からのバイト数1開始)
			{
				wchar_t szText[11];
				_itow(GetMainWindow().GetActiveView().GetCaret().GetCaretLogicPos().x + 1, szText, 10);
				q = wcs_pushW(q, q_max - q, szText);
				++p;
			}
			break;
		case L'y':	// 現在の物理行位置(1開始)
			{
				wchar_t szText[11];
				_itow(GetMainWindow().GetActiveView().GetCaret().GetCaretLogicPos().y + 1, szText, 10);
				q = wcs_pushW(q, q_max - q, szText);
				++p;
			}
			break;
		case L'd':	// 共通設定の日付書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime(&systime);
				FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);
				q = wcs_pushT(q, q_max - q, szText);
				++p;
			}
			break;
		case L't':	// 共通設定の時刻書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime(&systime);
				FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);
				q = wcs_pushT(q, q_max - q, szText);
				++p;
			}
			break;
		case L'p':	// 現在のページ
			{
				auto& editWnd = GetMainWindow();
				if (editWnd.pPrintPreview) {
					wchar_t szText[1024];
					_itow(editWnd.pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = wcs_pushW(q, q_max - q, szText, wcslen(szText));
					++p;
				}else {
					q = wcs_pushW(q, q_max - q, PRINT_PREVIEW_ONLY.c_str(), PRINT_PREVIEW_ONLY_LEN);
					++p;
				}
			}
			break;
		case L'P':	// 総ページ
			{
				auto& editWnd = GetMainWindow();
				if (editWnd.pPrintPreview) {
					wchar_t szText[1024];
					_itow(editWnd.pPrintPreview->GetAllPageNum(), szText, 10);
					q = wcs_pushW(q, q_max - q, szText);
					++p;
				}else {
					q = wcs_pushW(q, q_max - q, PRINT_PREVIEW_ONLY.c_str(), PRINT_PREVIEW_ONLY_LEN);
					++p;
				}
			}
			break;
		case L'D':	// タイムスタンプ
			if (!pDoc->docFile.IsFileTimeZero()) {
				TCHAR szText[1024];
				FormatManager().MyGetDateFormat(
					pDoc->docFile.GetFileSysTime(),
					szText,
					_countof(szText) - 1
				);
				q = wcs_pushT(q, q_max - q, szText);
				++p;
			}else {
				q = wcs_pushW(q, q_max - q, NOT_SAVED.c_str(), NOT_SAVED_LEN);
				++p;
			}
			break;
		case L'T':	// タイムスタンプ
			if (!pDoc->docFile.IsFileTimeZero()) {
				TCHAR szText[1024];
				FormatManager().MyGetTimeFormat(
					pDoc->docFile.GetFileSysTime(),
					szText,
					_countof(szText) - 1
				);
				q = wcs_pushT(q, q_max - q, szText);
				++p;
			}else {
				q = wcs_pushW(q, q_max - q, NOT_SAVED.c_str(), NOT_SAVED_LEN);
				++p;
			}
			break;
		case L'V':
			// Version number
			{
				wchar_t buf[28]; // 6(符号含むWORDの最大長) * 4 + 4(固定部分)
				// バージョン番号は、プロセスごとに取得する
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo(NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS);
				int len = auto_sprintf(buf, L"%d.%d.%d.%d",
					HIWORD(dwVersionMS),
					LOWORD(dwVersionMS),
					HIWORD(dwVersionLS),
					LOWORD(dwVersionLS)
				);
				q = wcs_pushW(q, q_max - q, buf, len);
				++p;
			}
			break;
		case L'h':
			// Grep Key文字列 MAX 32文字
			{
				NativeW memDes;
				// szGrepKey → memDes
				LimitStringLength(AppMode::getInstance().szGrepKey, wcslen(AppMode::getInstance().szGrepKey), (q_max - q > 32 ? 32 : q_max - q - 3), memDes);
				if ((int)wcslen(AppMode::getInstance().szGrepKey) > memDes.GetStringLength()) {
					memDes.AppendStringLiteral(L"...");
				}
				q = wcs_pushW(q, q_max - q, memDes.GetStringPtr(), memDes.GetStringLength());
				++p;
			}
			break;
		case L'S':
			// サクラエディタのフルパス
			{
				SFilePath szPath;
				::GetModuleFileName(NULL, szPath, _countof2(szPath));
				q = wcs_pushT(q, q_max - q, szPath);
				++p;
			}
			break;
		case 'I':
			// iniファイルのフルパス
			{
				TCHAR	szPath[_MAX_PATH + 1];
				std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
				FileNameManager::getInstance().GetIniFileName( szPath, strProfileName.c_str() );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'M':
			// 現在実行しているマクロファイルパスの取得
			{
				// 実行中マクロのインデックス番号 (INVALID_MACRO_IDX:無効 / STAND_KEYMACRO:標準マクロ)
				SMacroMgr* pSMacroMgr = EditApp::getInstance().pSMacroMgr;
				switch (pSMacroMgr->GetCurrentIdx()) {
				case INVALID_MACRO_IDX:
					break;
				case TEMP_KEYMACRO:
					q = wcs_pushT(q, q_max - q, pSMacroMgr->GetFile(TEMP_KEYMACRO));
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = GetDllShareData().common.macro.szKeyMacroFileName;
						q = wcs_pushT(q, q_max - q, pszMacroFilePath);
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = ShareData::getInstance().GetMacroFilename(pSMacroMgr->GetCurrentIdx(), szMacroFilePath, _countof(szMacroFilePath));
						if (0 < n) {
							q = wcs_pushT(q, q_max - q, szMacroFilePath);
						}
					}
					break;
				}
				++p;
			}
			break;
		// 条件分岐
		// ${cond:string1$:string2$:string3$}
		//	
		case L'{':	// 条件分岐
			{
				int cond = _ExParam_Evaluate(p + 1);
				while (*p != '?' && *p != '\0')
					++p;
				if (*p == '\0')
					break;
				p = _ExParam_SkipCond(p + 1, cond);
			}
			break;
		case L':':	// 条件分岐の中間
			// 条件分岐の末尾までSKIP
			p = _ExParam_SkipCond(p + 1, -1);
			break;
		case L'}':	// 条件分岐の末尾
			// 特にすることはない
			++p;
			break;
		case L'<':
			{
				// $<LongName>
				++p;
				const wchar_t *pBegin = p;
				while (*p != '>' && *p != '\0') {
					++p;
				}
				if (*p == '\0') {
					break;
				}
				int nParamNameIdx = EExpParamName_begin;
				for (; nParamNameIdx!=EExpParamName_end; ++nParamNameIdx) {
					if (SExpParamNameTable[nParamNameIdx].nLen == (p - pBegin)
						&& auto_strnicmp(SExpParamNameTable[nParamNameIdx].szName, pBegin, p - pBegin) == 0
					) {
						q = ExParam_LongName( q, q_max, static_cast<EExpParamName>(nParamNameIdx) );
						break;
					}
				}
				++p; // skip '>'
				break;
			}
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}


/*! @brief 処理の読み飛ばし

	条件分岐の構文 ${cond:A0$:A1$:A2$:..$} において，
	指定した番号に対応する位置の先頭へのポインタを返す．
	指定番号に対応する物が無ければ$}の次のポインタを返す．

	${が登場した場合にはネストと考えて$}まで読み飛ばす．

	@param pszSource [in] スキャンを開始する文字列の先頭．cond:の次のアドレスを渡す．
	@param part [in] 移動する番号＝読み飛ばす$:の数．-1を与えると最後まで読み飛ばす．

	@return 移動後のポインタ．該当領域の先頭かあるいは$}の直後．
*/
const wchar_t* SakuraEnvironment::_ExParam_SkipCond(const wchar_t* pszSource, int part)
{
	if (part == 0) {
		return pszSource;
	}
	
	int nest = 0;	// 入れ子のレベル
	bool next = true;	// 継続フラグ
	const wchar_t* p;
	for (p=pszSource; next && *p!=L'\0'; ++p) {
		if (*p == L'$' && p[1] != L'\0') { // $が末尾なら無視
			switch (*(++p)) {
			case L'{':	// 入れ子の開始
				++nest;
				break;
			case L'}':
				if (nest == 0) {
					// 終了ポイントに達した
					next = false; 
				}else {
					// ネストレベルを下げる
					--nest;
				}
				break;
			case L':':
				if (nest == 0 && --part == 0) { // 入れ子でない場合のみ
					// 目的のポイント
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief 条件の評価

	@param pCond [in] 条件種別先頭．'?'までを条件と見なして評価する
	@return 評価の値

	@note
	ポインタの読み飛ばし作業は行わないので，'?'までの読み飛ばしは
	呼び出し側で別途行う必要がある．
*/
int SakuraEnvironment::_ExParam_Evaluate(const wchar_t* pCond)
{
	const EditDoc* pDoc = EditDoc::GetInstance(0); //###

	switch (*pCond) {
	case L'R': // $R ビューモードおよび読み取り専用属性
		if (AppMode::getInstance().IsViewMode()) {
			return 0; // ビューモード
		}else if (!EditDoc::GetInstance(0)->docLocker.IsDocWritable()) {
			return 1; // 上書き禁止
		}else {
			return 2; // 上記以外
		}
	case L'w': // $w Grepモード/Output Mode
		if (EditApp::getInstance().pGrepAgent->bGrepMode) {
			return 0;
		}else if (AppMode::getInstance().IsDebugMode()) {
			return 1;
		}else {
			return 2;
		}
	case L'M': // $M キーボードマクロの記録中
		if (GetDllShareData().flags.bRecordingKeyMacro
			&& GetDllShareData().flags.hwndRecordingKeyMacro == EditWnd::getInstance().GetHwnd() // ウィンドウ
		) {
			return 0;
		}else {
			return 1;
		}
	case L'U': // $U 更新
		if (pDoc->docEditor.IsModified()) {
			return 0;
		}else {
			return 1;
		}
	case L'N': // $N 新規/(無題)
		if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {
			return 0;
		}else {
			return 1;
		}
	case L'I': // $I アイコン化されているか
		if (::IsIconic(EditWnd::getInstance().GetHwnd())) {
			return 0;
		}else {
 			return 1;
 		}
	default:
		break;
	}
	return 0;
}

// 長い名前の設定
wchar_t* ExParam_LongName( wchar_t* q, wchar_t* q_max, EExpParamName eLongParam )
{
	switch (eLongParam) {
	case EExpParamName_profile:
		{
			LPCWSTR pszProf = CommandLine::getInstance().GetProfileName();
			q = wcs_pushW( q, q_max - q, pszProf );
		}
		break;
	default:
		assert( 0 );
		break;
	}
	return q;
}

/*!	@brief 初期フォルダ取得

	@param bControlProcess [in] trueのときはOPENDIALOGDIR_CUR->OPENDIALOGDIR_MRUに変更
	@return 初期フォルダ
*/
std::tstring SakuraEnvironment::GetDlgInitialDir(bool bControlProcess)
{
	EditDoc* pDoc = EditDoc::GetInstance(0); //######
	if (pDoc && pDoc->docFile.GetFilePathClass().IsValidPath()) {
		return to_tchar(pDoc->docFile.GetFilePathClass().GetDirPath().c_str());
	}

	EOpenDialogDir eOpenDialogDir = GetDllShareData().common.edit.eOpenDialogDir;
	if (bControlProcess && eOpenDialogDir == OPENDIALOGDIR_CUR) {
		eOpenDialogDir = OPENDIALOGDIR_MRU;
	}

	switch (eOpenDialogDir) {
	case OPENDIALOGDIR_CUR:
		{
			TCHAR szCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory(_countof(szCurDir), szCurDir);
			if (nCurDir == 0 || _MAX_PATH < nCurDir) {
				return _T("");
			}else {
				return szCurDir;
			}
		}
		break;
	case OPENDIALOGDIR_MRU:
		{
			const MruFolder mru;
			auto& vMRU = mru.GetPathList();
			size_t nCount = mru.Length();
			for (size_t i=0; i<nCount ; ++i) {
				DWORD attr = GetFileAttributes(vMRU[i]);
				if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
					return vMRU[i];
				}
			}

			TCHAR szCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory(_countof(szCurDir), szCurDir);
			if (nCurDir == 0 || _MAX_PATH < nCurDir) {
				return _T("");
			}else {
				return szCurDir;
			}
		}
		break;
	case OPENDIALOGDIR_SEL:
		{
			TCHAR szSelDir[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(GetDllShareData().common.edit.openDialogSelDir, szSelDir, _countof(szSelDir));
			return szSelDir;
		}
		break;
	default:
		assert(0);
		return _T("");
	}
}

void SakuraEnvironment::ResolvePath(TCHAR* pszPath)
{
	// pszPath -> pSrc
	TCHAR* pSrc = pszPath;

	// ショートカット(.lnk)の解決: pSrc -> szBuf -> pSrc
	TCHAR szBuf[_MAX_PATH];
	if (ResolveShortcutLink(NULL, pSrc, szBuf)) {
		pSrc = szBuf;
	}

	// ロングファイル名を取得する: pSrc -> szBuf2 -> pSrc
	TCHAR szBuf2[_MAX_PATH];
	if (::GetLongFileName(pSrc, szBuf2)) {
		pSrc = szBuf2;
	}

	// pSrc -> pszPath
	if (pSrc != pszPath) {
		_tcscpy_s(pszPath, _MAX_PATH, pSrc);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ウィンドウ管理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる
bool IsSakuraMainWindow(HWND hWnd)
{
	TCHAR szClassName[64];
	if (hWnd == NULL) {
		return false;
	}
	if (!::IsWindow(hWnd)) {
		return false;
	}
	if (::GetClassName(hWnd, szClassName, _countof(szClassName) - 1) == 0) {
		return false;
	}
	return (_tcscmp(GSTR_EDITWINDOWNAME, szClassName) == 0);
}

